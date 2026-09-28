// Original Kouku material programs 4928..4991; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_pa_trail_03_01_tr: 1946dbc9412ed54793435682a2c4fd1b; selected map 9ac0b942df1b47467d9f5e2ac2e1b67fab5086c7d944b608589a0521cf39c9f2.
float4 ArtistNative4928(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[9].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    r1.y = (ArtistNativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r0.w = (ArtistNativeSample2((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r1.x = ((r1.xxxx)*(float4(0.666667,0.666667,0.666667,0.666667))).x;
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
    r0.z = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4928Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[1] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[2] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[3].x = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[5].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[6].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[6].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 1: add r0.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
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
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: mul r1.y, r0.z, cb0[4].y
    r1.y = ((r0.zzzz)*(source[4].yyyy)).y;
    // 30: mul r1.x, r0.z, cb0[3].w
    r1.x = ((r0.zzzz)*(source[3].wwww)).x;
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
    // 37: mul r1.zw, r0.zzzz, cb0[4].xxxz
    r1.zw = ((r0.zzzz)*(source[4].xxxz)).zw;
    // 38: mad r0.zw, v3.zzzz, l(0.000000, 0.000000, 0.800000, -0.400000), r1.yyyw
    r0.zw = ((v3.zzzz)*(float4(0.000000,0.000000,0.800000,-0.400000))+(r1.yyyw)).zw;
    // 39: mad r1.yw, v3.zzzz, l(0.000000, 0.500000, 0.000000, -0.200000), r1.yyyw
    r1.yw = ((v3.zzzz)*(float4(0.000000,0.500000,0.000000,-0.200000))+(r1.yyyw)).yw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.ywyy, t0.yxzw, s1, l(0.000000)
    r1.y = (ArtistNativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: add r0.z, r1.y, r0.z
    r0.z = ((r1.yyyy)+(r0.zzzz)).z;
    // 43: mul r0.w, r1.y, cb0[4].w
    r0.w = ((r1.yyyy)*(source[4].wwww)).w;
    // 44: mad r1.yz, r0.wwww, v3.wwww, r1.xxzx
    r1.yz = ((r0.wwww)*(v3.wwww)+(r1.xxzx)).yz;
    // 45: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 46: mad r1.xw, v3.zzzz, l(0.400000, 0.000000, 0.000000, -0.600000), r1.yyyz
    r1.xw = ((v3.zzzz)*(float4(0.400000,0.000000,0.000000,-0.600000))+(r1.yyyz)).xw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xwxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample2((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 48: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 49: mad r0.z, r0.z, l(0.500000), r0.w
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).z;
    // 50: mad r0.w, -r0.x, l(2.000000), l(1.000000)
    r0.w = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: mad r0.x, -r0.x, cb0[5].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[5].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[6].y
    r0.x = (saturate((r0.xxxx)*(source[6].yyyy))).x;
    // 53: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 54: mul r0.w, r0.w, l(0.666667)
    r0.w = ((r0.wwww)*(float4(0.666667,0.666667,0.666667,0.666667))).w;
    // 55: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 56: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 57: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 58: add r1.xw, v3.xxxy, l(-1.000000, 0.000000, 0.000000, -1.000000)
    r1.xw = ((v3.xxxy)+(float4(-1.000000,0.000000,0.000000,-1.000000))).xw;
    // 59: mad r0.y, r0.z, r0.y, -r1.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(-(r1.wwww))).y;
    // 60: mad r1.xy, r1.xxxx, l(0.500000, -0.400000, 0.000000, 0.000000), r1.yzyy
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.400000,0.000000,0.000000))+(r1.yzyy)).xy;
    // 61: add r1.xy, r1.xyxx, cb0[0].xyxx
    r1.xy = ((r1.xyxx)+(source[0].xyxx)).xy;
    // 62: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 64: dp2 r2.x, cb0[1].xyxx, r1.xyxx
    r2.x = (dot((source[1].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[2].xyxx, r1.xyxx
    r2.y = (dot((source[2].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 66: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 68: mul_sat r0.w, r0.w, l(20.000000)
    r0.w = (saturate((r0.wwww)*(float4(20.000000,20.000000,20.000000,20.000000)))).w;
    // 69: mad_sat r0.x, r0.w, r0.y, -r0.x
    r0.x = (saturate((r0.wwww)*(r0.yyyy)+(-(r0.xxxx)))).x;
    // 70: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 71: mul r0.x, r0.x, v2.w
    r0.x = ((r0.xxxx)*(v2.wwww)).x;
    // 72: mul r0.x, r0.x, cb0[6].w
    r0.x = ((r0.xxxx)*(source[6].wwww)).x;
    // 73: mad r0.xyzw, r0.xxxx, l(20.000000, -20.000000, 20.000000, -20.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(20.000000,-20.000000,20.000000,-20.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 74: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 75: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 76: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 77: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 78: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 79: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 80: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 81: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 82: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 83: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 84: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 85: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 86: source device depth mapped to centimetre view depth; reconstruction at 88.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 88-91: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 92: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 93: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 94: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 95: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 96: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_me_ri_05_1_ad: cea52c684c0d964ab7aaa172d57ae94a; selected map 726fed7095f2c66ae6c9866f9f3df9cca9b7ca91d6a42b56f3752d5c7fde3d89.
float4 ArtistNative4929(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, cb0[4].x, l(-1.000000)
    r0.x = ((source[4].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mad r0.xy, cb0[4].xxxx, v4.xyxx, -r0.xxxx
    r0.xy = ((source[4].xxxx)*(v4.xyxx)+(-(r0.xxxx))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 5: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 6: mul r0.w, r0.w, cb0[4].w
    r0.w = ((r0.wwww)*(source[4].wwww)).w;
    // 7: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 8: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 9: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: movc r0.w, r1.x, l(0), |r0.w|
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.wwww))).w;
    // 11: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 12: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 13: mul r1.x, r1.x, cb0[3].y
    r1.x = ((r1.xxxx)*(source[3].yyyy)).x;
    // 14: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 15: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 17: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 18: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 19: mul r1.xyz, r0.xyzx, cb0[4].yyyy
    r1.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 20: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyz, -cb0[4].yyyy, r0.xyzx, r1.wwww
    r0.xyz = ((-(source[4].yyyy))*(r0.xyzx)+(r1.wwww)).xyz;
    // 22: mad r0.xyz, cb0[4].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 24: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 25: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 26: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4929Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_pa_turbpa_01_3_tr: 142d7eeccb6dec4b8b4233dcf6db51a2; selected map 1f8ae9aed7bae93344c781be239cb610ad315e63de228050a9f4abce3bea8db8.
float4 ArtistNative4930(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[8].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 7: mul r1.xyzw, v2.xyxy, cb0[2].xyxy
    r1.xyzw = ((v2.xyxy)*(source[2].xyxy)).xyzw;
    // 8: mul r1.xyzw, r1.xyzw, l(0.500000, 0.500000, 0.300000, 0.300000)
    r1.xyzw = ((r1.xyzw)*(float4(0.500000,0.500000,0.300000,0.300000))).xyzw;
    // 9: mad r1.xyzw, v4.yyyy, l(0.100000, -0.100000, -0.070000, 0.100000), r1.xyzw
    r1.xyzw = ((v4.yyyy)*(float4(0.100000,-0.100000,-0.070000,0.100000))+(r1.xyzw)).xyzw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.xyzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 12: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 13: mad r0.x, r0.x, r0.y, r0.z
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.zzzz)).x;
    // 14: mul r0.y, r0.x, v4.x
    r0.y = ((r0.xxxx)*(v4.xxxx)).y;
    // 15: mad r0.yz, v2.xxyx, cb0[7].xxyx, r0.yyyy
    r0.yz = ((v2.xxyx)*(source[7].xxyx)+(r0.yyyy)).yz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s3, l(0.000000)
    r0.yzw = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 17: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 18: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 19: mul r0.yzw, r0.yyzw, cb0[9].wwww
    r0.yzw = ((r0.yyzw)*(source[9].wwww)).yzw;
    // 20: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 21: mul r1.xy, v2.xyxx, cb0[5].xyxx
    r1.xy = ((v2.xyxx)*(source[5].xyxx)).xy;
    // 22: mad r1.xy, r0.xxxx, cb0[8].yyyy, r1.xyxx
    r1.xy = ((r0.xxxx)*(source[8].yyyy)+(r1.xyxx)).xy;
    // 23: mad r1.zw, r0.xxxx, cb0[11].zzzz, v2.xxxy
    r1.zw = ((r0.xxxx)*(source[11].zzzz)+(v2.xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.zwzz, t3.xyzw, s5, l(0.000000)
    r0.x = (ArtistNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: add r1.zw, r1.xxxy, cb0[6].xxxy
    r1.zw = ((r1.xxxy)+(source[6].xxxy)).zw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 28: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 29: mul r1.x, r1.x, cb0[11].x
    r1.x = ((r1.xxxx)*(source[11].xxxx)).x;
    // 30: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 31: mul r1.x, r1.x, cb0[11].y
    r1.x = ((r1.xxxx)*(source[11].yyyy)).x;
    // 32: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 33: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 34: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t4.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 36: mad r0.xyz, cb0[10].xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 37: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 38: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 39: mad r0.xyz, cb0[10].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[10].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 40: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 41: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 42: mul r0.xyz, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 43: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 44: mul r0.xyz, r0.xyzx, cb0[10].wwww
    r0.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 45: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 46: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative4931(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_pa_ring_01_1_ts_tr: 0c82fede9116814d8f31e43c95489cc4; selected map 90b32c9c46d704d7eb6cb62c9c89cfa4b73c3e14435a908fe0c41a05ddb5384a.
float4 ArtistNative4932(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[5].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[6].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[6].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.xy, v2.xyxx, l(0.800000, 0.500000, 0.000000, 0.000000), cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(float4(0.800000,0.500000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.y = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4932Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[4].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[5].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[5].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v4 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v1.xyxx, l(0.800000, 0.500000, 0.000000, 0.000000), cb0[2].xyxx
    r0.xy = ((v1.xyxx)*(float4(0.800000,0.500000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: max r0.xyzw, |r0.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 4: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 5: mul r0.xyzw, r0.xyzw, l(1.200000, 1.200000, 1.200000, 1.200000)
    r0.xyzw = ((r0.xyzw)*(float4(1.200000,1.200000,1.200000,1.200000))).xyzw;
    // 6: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 7: mul r0.xyzw, r0.xyzw, cb0[4].yyyy
    r0.xyzw = ((r0.xyzw)*(source[4].yyyy)).xyzw;
    // 8: add r1.xy, v1.xyxx, cb0[1].xyxx
    r1.xy = ((v1.xyxx)+(source[1].xyxx)).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xy = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mul r1.xyzw, r1.xyxy, cb0[4].yyyy
    r1.xyzw = ((r1.xyxy)*(source[4].yyyy)).xyzw;
    // 11: mul r0.xyzw, r0.xyzw, r1.xyzw
    r0.xyzw = ((r0.xyzw)*(r1.xyzw)).xyzw;
    // 12: add r1.xy, v4.xyxx, cb0[0].xyxx
    r1.xy = ((v4.xyxx)+(source[0].xyxx)).xy;
    // 13: mul r1.xy, r1.xyxx, cb0[3].yzyy
    r1.xy = ((r1.xyxx)*(source[3].yzyy)).xy;
    // 14: mul r1.xy, r1.xyxx, l(0.003000, 0.003000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.003000,0.003000,0.000000,0.000000))).xy;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 16: mul r1.xyzw, r1.xyxy, cb0[3].wwww
    r1.xyzw = ((r1.xyxy)*(source[3].wwww)).xyzw;
    // 17: mul r1.xyzw, r1.xyzw, l(0.300000, 0.300000, 0.300000, 0.300000)
    r1.xyzw = ((r1.xyzw)*(float4(0.300000,0.300000,0.300000,0.300000))).xyzw;
    // 18: mul r0.xyzw, r0.xyzw, r1.zwzw
    r0.xyzw = ((r0.xyzw)*(r1.zwzw)).xyzw;
    // 19: mad r0.xyzw, r0.xyzw, l(0.200000, 0.200000, 0.200000, 0.200000), r1.xyzw
    r0.xyzw = ((r0.xyzw)*(float4(0.200000,0.200000,0.200000,0.200000))+(r1.xyzw)).xyzw;
    // 20: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 21: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 22: mul r0.xyzw, r0.xyzw, cb0[5].zzzz
    r0.xyzw = ((r0.xyzw)*(source[5].zzzz)).xyzw;
    // 23: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 24: add r1.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 25: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 26: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 27: mad r1.x, -r1.x, l(2.000000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 29: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 30: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: log r1.z, r1.x
    r1.z = (log2(r1.xxxx)).z;
    // 32: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 33: mul r1.z, r1.z, cb0[5].w
    r1.z = ((r1.zzzz)*(source[5].wwww)).z;
    // 34: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 35: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 36: mul_sat r1.y, r1.y, cb0[6].x
    r1.y = (saturate((r1.yyyy)*(source[6].xxxx))).y;
    // 37: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 38: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 39: min r0.xyzw, r0.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r0.xyzw = (min(r0.xyzw,float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = (ArtistNativeSample0((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 41: max r1.xyzw, |r1.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 42: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 43: mul r1.xyzw, r1.xyzw, cb0[3].xxxx
    r1.xyzw = ((r1.xyzw)*(source[3].xxxx)).xyzw;
    // 44: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 45: mad r0.xyzw, r1.xyzw, l(2.000000, 2.000000, 2.000000, 2.000000), r0.xyzw
    r0.xyzw = ((r1.xyzw)*(float4(2.000000,2.000000,2.000000,2.000000))+(r0.xyzw)).xyzw;
    // 46: mul r0.xyzw, r0.xyzw, cb0[6].yyyy
    r0.xyzw = ((r0.xyzw)*(source[6].yyyy)).xyzw;
    // 47: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 48: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 49: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 50: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 51: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 52: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 53: mul r1.xyz, v4.yyyy, cb1[1].xywx
    r1.xyz = ((v4.yyyy)*(projection[1].xywx)).xyz;
    // 54: mad r1.xyz, cb1[0].xywx, v4.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v4.xxxx)+(r1.xyzx)).xyz;
    // 55: mad r1.xyz, cb1[2].xywx, v4.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v4.zzzz)+(r1.xyzx)).xyz;
    // 56: mad r1.xyz, cb1[3].xywx, v4.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v4.wwww)+(r1.xyzx)).xyz;
    // 57: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 58: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 59: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 60: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 61: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 62: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 63: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 64: source device depth mapped to centimetre view depth; reconstruction at 66.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 66-69: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 70: ge r0.x, r1.z, r0.x
    r0.x = (asfloat((uint4)((r1.zzzz)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 71: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 72: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 73: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 74: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_trail_01_09_tr: 045a98e64142ef478d38469879c97683; selected map fa3742d03f927ffa4570423e2a265db32224f07f574302f3a47a6350069a36bf.
float4 ArtistNative4933(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[9].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[9].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    r0.w = (ArtistNativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.zwzz, t0.yxzw, s0, l(0.000000)
    r1.y = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r1.x = (ArtistNativeSample2((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r1.z = ((r1.zzzz)*(float4(0.666667,0.666667,0.666667,0.666667))).z;
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
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4933Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[1] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[2] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[3].x = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[6].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[7].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 1: add r0.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
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
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    // 35: mul r1.yzw, r0.zzww, cb0[4].yyxz
    r1.yzw = ((r0.zzww)*(source[4].yyxz)).yzw;
    // 36: mul r2.y, r0.w, cb0[5].y
    r2.y = ((r0.wwww)*(source[5].yyyy)).y;
    // 37: mad r2.zw, v3.zzzz, l(0.000000, 0.000000, 0.800000, -0.400000), r1.yyyw
    r2.zw = ((v3.zzzz)*(float4(0.000000,0.000000,0.800000,-0.400000))+(r1.yyyw)).zw;
    // 38: mad r1.yw, v3.zzzz, l(0.000000, 0.500000, 0.000000, -0.200000), r1.yyyw
    r1.yw = ((v3.zzzz)*(float4(0.000000,0.500000,0.000000,-0.200000))+(r1.yyyw)).yw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.ywyy, t0.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.zwzz, t0.yxzw, s1, l(0.000000)
    r1.y = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: add r1.y, r0.w, r1.y
    r1.y = ((r0.wwww)+(r1.yyyy)).y;
    // 42: mul r0.w, r0.w, cb0[4].w
    r0.w = ((r0.wwww)*(source[4].wwww)).w;
    // 43: mul r2.x, r0.z, cb0[5].x
    r2.x = ((r0.zzzz)*(source[5].xxxx)).x;
    // 44: mul r1.x, r0.z, cb0[3].w
    r1.x = ((r0.zzzz)*(source[3].wwww)).x;
    // 45: mad r2.xy, r0.wwww, v3.wwww, r2.xyxx
    r2.xy = ((r0.wwww)*(v3.wwww)+(r2.xyxx)).xy;
    // 46: mad r0.zw, r0.wwww, v3.wwww, r1.xxxz
    r0.zw = ((r0.wwww)*(v3.wwww)+(r1.xxxz)).zw;
    // 47: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 48: mad r1.xz, v3.zzzz, l(0.400000, 0.000000, -0.600000, 0.000000), r2.xxyx
    r1.xz = ((v3.zzzz)*(float4(0.400000,0.000000,-0.600000,0.000000))+(r2.xxyx)).xz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xzxx, t2.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 50: add r1.y, -r1.x, r1.y
    r1.y = ((-(r1.xxxx))+(r1.yyyy)).y;
    // 51: mad r1.x, r1.y, l(0.500000), r1.x
    r1.x = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xxxx)).x;
    // 52: mad r1.y, -r0.x, l(2.000000), l(1.000000)
    r1.y = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: mad r0.x, -r0.x, cb0[6].w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[6].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 55: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 56: mul r1.y, r1.y, l(0.666667)
    r1.y = ((r1.yyyy)*(float4(0.666667,0.666667,0.666667,0.666667))).y;
    // 57: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 58: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 59: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 60: add r1.yz, v3.xxyx, l(0.000000, -1.000000, -1.000000, 0.000000)
    r1.yz = ((v3.xxyx)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 61: mad r0.y, r1.x, r0.y, -r1.z
    r0.y = ((r1.xxxx)*(r0.yyyy)+(-(r1.zzzz))).y;
    // 62: mad r0.zw, r1.yyyy, l(0.000000, 0.000000, 0.500000, -0.400000), r0.zzzw
    r0.zw = ((r1.yyyy)*(float4(0.000000,0.000000,0.500000,-0.400000))+(r0.zzzw)).zw;
    // 63: add r0.zw, r0.zzzw, cb0[0].xxxy
    r0.zw = ((r0.zzzw)+(source[0].xxxy)).zw;
    // 64: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 65: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 66: dp2 r2.x, cb0[1].xyxx, r0.zwzz
    r2.x = (dot((source[1].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 67: dp2 r2.y, cb0[2].xyxx, r0.zwzz
    r2.y = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 68: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 70: mul_sat r0.z, r0.z, l(20.000000)
    r0.z = (saturate((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000)))).z;
    // 71: mad_sat r0.x, r0.z, r0.y, -r0.x
    r0.x = (saturate((r0.zzzz)*(r0.yyyy)+(-(r0.xxxx)))).x;
    // 72: mul r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)*(r0.xxxx)).x;
    // 73: mul r0.x, r0.x, v2.w
    r0.x = ((r0.xxxx)*(v2.wwww)).x;
    // 74: mul r0.x, r0.x, cb0[8].y
    r0.x = ((r0.xxxx)*(source[8].yyyy)).x;
    // 75: mad r0.xyzw, r0.xxxx, l(20.000000, -20.000000, 20.000000, -20.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(20.000000,-20.000000,20.000000,-20.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 76: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 77: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 78: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 79: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 80: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 81: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 82: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 83: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 84: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 85: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 86: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 87: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 88: source device depth mapped to centimetre view depth; reconstruction at 90.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 90-93: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 94: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 95: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 96: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 97: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 98: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_dist_05_ad: 0fc154ee4cdbcc408e07e6b3989115cd; selected map 51630230004172d119766ea88ac7bbd7786f285ef0fc919f13cb27502021606a.
float4 ArtistNative4934(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
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
    float4 r0=0.f;
    // 1: mul r0.xyz, v5.wwww, cb0[1].xyzx
    r0.xyz = ((v5.wwww)*(source[1].xyzx)).xyz;
    // 2: mul o0.xyz, r0.xyzx, cb0[0].xxxx
    output.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 3: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4934Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[1].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[1].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[1].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[1].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = ((float4(1.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[2].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: mul r0.zw, r0.xxxy, cb0[1].wwww
    r0.zw = ((r0.xxxy)*(source[1].wwww)).zw;
    // 3: max r1.x, |r0.w|, |r0.z|
    r1.x = (max(abs(r0.wwww),abs(r0.zzzz))).x;
    // 4: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 5: min r1.y, |r0.w|, |r0.z|
    r1.y = (min(abs(r0.wwww),abs(r0.zzzz))).y;
    // 6: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 7: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 8: mad r1.z, r1.y, l(0.020835), l(-0.085133)
    r1.z = ((r1.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 9: mad r1.z, r1.y, r1.z, l(0.180141)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 10: mad r1.z, r1.y, r1.z, l(-0.330299)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 11: mad r1.y, r1.y, r1.z, l(0.999866)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 12: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 13: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 14: lt r1.w, |r0.w|, |r0.z|
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).w;
    // 15: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 16: mad r1.x, r1.x, r1.y, r1.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // 17: lt r1.y, r0.w, -r0.w
    r1.y = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).y;
    // 18: and r1.y, r1.y, l(0xc0490fdb)
    r1.y = (asfloat(asuint(r1.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 19: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 20: min r1.y, r0.w, r0.z
    r1.y = (min(r0.wwww,r0.zzzz)).y;
    // 21: lt r1.y, r1.y, -r1.y
    r1.y = (asfloat((uint4)((r1.yyyy)<(-(r1.yyyy))) * 0xffffffffu)).y;
    // 22: max r1.z, r0.w, r0.z
    r1.z = (max(r0.wwww,r0.zzzz)).z;
    // 23: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 24: ge r0.w, r1.z, -r1.z
    r0.w = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).w;
    // 25: and r0.w, r0.w, r1.y
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.yyyy))).w;
    // 26: movc r0.w, r0.w, -r1.x, r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (-(r1.xxxx)) : (r1.xxxx)).w;
    // 27: mul r1.x, cb0[2].y, l(3.141592)
    r1.x = ((source[2].yyyy)*(float4(3.141592,3.141592,3.141592,3.141592))).x;
    // 28: div r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)/(r1.xxxx)).w;
    // 29: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mul r1.x, r0.w, l(0.500000)
    r1.x = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 31: lt r0.w, r0.z, l(0.000001)
    r0.w = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 32: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 33: movc r1.y, r0.w, l(0), r0.z
    r1.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 34: add r0.zw, r1.xxxy, cb0[0].xxxy
    r0.zw = ((r1.xxxy)+(source[0].xxxy)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 36: mad r1.xy, v3.xxxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.200000, 0.170000, 0.000000, 0.000000)
    r1.xy = ((v3.xxxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.200000,0.170000,0.000000,0.000000))).xy;
    // 37: max r1.xy, r1.xyxx, l(0.000010, 0.000010, 0.000000, 0.000000)
    r1.xy = (max(r1.xyxx,float4(0.000010,0.000010,0.000000,0.000000))).xy;
    // 38: div r1.xy, l(1.000000, 1.000000, 1.000000, 1.000000), r1.xyxx
    r1.xy = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xyxx)).xy;
    // 39: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 40: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 41: mad r1.xy, -r0.wwww, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r0.wwww))*(r1.xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 42: mad r0.w, -r0.w, l(2.000000), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: add_sat r0.w, r0.w, r0.w
    r0.w = (saturate((r0.wwww)+(r0.wwww))).w;
    // 44: mul_sat r1.x, r1.x, l(5.000000)
    r1.x = (saturate((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 45: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 46: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 47: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 48: div r1.x, l(0.500000), v3.x
    r1.x = ((float4(0.500000,0.500000,0.500000,0.500000))/(v3.xxxx)).x;
    // 49: mad r0.xy, r1.xxxx, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xxxx)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 52: mul r0.y, r1.x, v3.z
    r0.y = ((r1.xxxx)*(v3.zzzz)).y;
    // 53: mad r0.z, r0.z, r1.y, r0.y
    r0.z = ((r0.zzzz)*(r1.yyyy)+(r0.yyyy)).z;
    // 54: mad r0.x, -v3.w, r0.x, r0.y
    r0.x = ((-(v3.wwww))*(r0.xxxx)+(r0.yyyy)).x;
    // 55: add r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)+(r0.zzzz)).x;
    // 56: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 57: mul r0.x, r0.x, -v3.z
    r0.x = ((r0.xxxx)*(-(v3.zzzz))).x;
    // 58: mul r0.x, r0.x, v3.y
    r0.x = ((r0.xxxx)*(v3.yyyy)).x;
    // 59: div r0.y, l(1536.000000), v4.z
    r0.y = ((float4(1536.000000,1536.000000,1536.000000,1536.000000))/(v4.zzzz)).y;
    // 60: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 61: mul r0.yw, r0.xxxx, cb0[3].xxxx
    r0.yw = ((r0.xxxx)*(source[3].xxxx)).yw;
    // 62: mov r0.xz, cb0[3].xxxx
    r0.xz = (source[3].xxxx).xz;
    // 63: mad r0.xyzw, r0.xyzw, l(0.000000, -2.000000, 0.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(0.000000,-2.000000,0.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 64: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 65: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 66: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 67: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 68: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 69: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 70: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 71: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 72: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 73: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 74: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 75: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 76: source device depth mapped to centimetre view depth; reconstruction at 78.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 78-81: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 82: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 83: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 84: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 85: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 86: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_c_pa_shield_02_ad: 54d5c5ba7e1d3e45a382bddff3f1d86d; selected map a03723bd3ae35475a6326089a41822e8710f322ea48a8b532e6a330ce2ebf10e.
float4 ArtistNative4935(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = g_ArtistSourceMaterialParameters[1u];
    source[8].x = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[8].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0)))).x;
    source[8].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].w = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.300000012, 0.0, 0.0, 0.0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: mul r0.zw, r0.xxxy, cb0[8].wwww
    r0.zw = ((r0.xxxy)*(source[8].wwww)).zw;
    // 3: max r1.x, |r0.w|, |r0.z|
    r1.x = (max(abs(r0.wwww),abs(r0.zzzz))).x;
    // 4: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 5: min r1.y, |r0.w|, |r0.z|
    r1.y = (min(abs(r0.wwww),abs(r0.zzzz))).y;
    // 6: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 7: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 8: mad r1.z, r1.y, l(0.020835), l(-0.085133)
    r1.z = ((r1.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 9: mad r1.z, r1.y, r1.z, l(0.180141)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 10: mad r1.z, r1.y, r1.z, l(-0.330299)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 11: mad r1.y, r1.y, r1.z, l(0.999866)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 12: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 13: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 14: lt r1.w, |r0.w|, |r0.z|
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).w;
    // 15: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 16: mad r1.x, r1.x, r1.y, r1.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // 17: lt r1.y, r0.w, -r0.w
    r1.y = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).y;
    // 18: and r1.y, r1.y, l(0xc0490fdb)
    r1.y = (asfloat(asuint(r1.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 19: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 20: min r1.y, r0.w, r0.z
    r1.y = (min(r0.wwww,r0.zzzz)).y;
    // 21: lt r1.y, r1.y, -r1.y
    r1.y = (asfloat((uint4)((r1.yyyy)<(-(r1.yyyy))) * 0xffffffffu)).y;
    // 22: max r1.z, r0.w, r0.z
    r1.z = (max(r0.wwww,r0.zzzz)).z;
    // 23: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 24: ge r0.w, r1.z, -r1.z
    r0.w = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).w;
    // 25: and r0.w, r0.w, r1.y
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.yyyy))).w;
    // 26: movc r0.w, r0.w, -r1.x, r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (-(r1.xxxx)) : (r1.xxxx)).w;
    // 27: mul r1.x, cb0[9].y, l(3.141592)
    r1.x = ((source[9].yyyy)*(float4(3.141592,3.141592,3.141592,3.141592))).x;
    // 28: div r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)/(r1.xxxx)).w;
    // 29: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mul r1.x, r0.w, l(0.500000)
    r1.x = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 31: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 32: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 33: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 34: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 35: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 36: add r1.y, r0.z, cb0[9].w
    r1.y = ((r0.zzzz)+(source[9].wwww)).y;
    // 37: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 38: mul r0.zw, r0.xxxy, r0.xxxy
    r0.zw = ((r0.xxxy)*(r0.xxxy)).zw;
    // 39: mul r2.xy, r0.zwzz, l(4.000000, 4.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 40: lt r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.zw = (asfloat((uint4)((r0.zzzw)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).zw;
    // 41: log r2.xy, r2.xyxx
    r2.xy = (log2(r2.xyxx)).xy;
    // 42: mul r2.xy, r2.xyxx, l(10.000000, 10.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(10.000000,10.000000,0.000000,0.000000))).xy;
    // 43: exp r2.xy, r2.xyxx
    r2.xy = (exp2(r2.xyxx)).xy;
    // 44: movc r0.zw, r0.zzzw, l(0,0,0,0), r2.xxxy
    r0.zw = ((asuint(r0.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxy)).zw;
    // 45: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 46: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 47: mul_sat r1.xyz, r1.xyzx, r0.zzzz
    r1.xyz = (saturate((r1.xyzx)*(r0.zzzz))).xyz;
    // 48: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 49: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 50: mad r0.w, -r0.w, l(20.000000), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(20.000000,20.000000,20.000000,20.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 52: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 54: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 55: mad r2.xy, v4.xyxx, l(1.750000, 1.000000, 0.000000, 0.000000), cb0[3].xyxx
    r2.xy = ((v4.xyxx)*(float4(1.750000,1.000000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 57: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 58: mad r2.xy, v4.xyxx, l(3.000000, 2.000000, 0.000000, 0.000000), r0.wwww
    r2.xy = ((v4.xyxx)*(float4(3.000000,2.000000,0.000000,0.000000))+(r0.wwww)).xy;
    // 59: mad r2.zw, v4.xxxy, l(0.000000, 0.000000, 5.000000, 3.000000), r0.wwww
    r2.zw = ((v4.xxxy)*(float4(0.000000,0.000000,5.000000,3.000000))+(r0.wwww)).zw;
    // 60: add r2.zw, r2.zzzw, cb0[4].xxxy
    r2.zw = ((r2.zzzw)+(source[4].xxxy)).zw;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.zwzz, t1.xyzw, s1, l(0.000000)
    r3.xyz = (ArtistNativeSample1((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 62: mul r3.xyz, r3.xyzx, l(40.000000, 40.000000, 40.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(40.000000,40.000000,40.000000,0.000000))).xyz;
    // 63: add r2.xy, r2.xyxx, cb0[6].xyxx
    r2.xy = ((r2.xyxx)+(source[6].xyxx)).xy;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 65: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 66: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 67: mul r2.xyz, r2.xyzx, cb0[7].xyzx
    r2.xyz = ((r2.xyzx)*(source[7].xyzx)).xyz;
    // 68: dp2 r0.w, l(0.707107, -0.707107, 0.000000, 0.000000), r0.xyxx
    r0.w = (dot((float4(0.707107,-0.707107,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).w;
    // 69: dp2 r0.x, l(0.707107, 0.707107, 0.000000, 0.000000), r0.xyxx
    r0.x = (dot((float4(0.707107,0.707107,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).x;
    // 70: add r0.x, |r0.w|, |r0.x|
    r0.x = ((abs(r0.wwww))+(abs(r0.xxxx))).x;
    // 71: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 72: mul_sat r0.x, r0.x, l(1.650000)
    r0.x = (saturate((r0.xxxx)*(float4(1.650000,1.650000,1.650000,1.650000)))).x;
    // 73: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 74: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: mul r0.y, r0.y, l(15.000000)
    r0.y = ((r0.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 76: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 77: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 78: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 79: mul r4.xyz, r0.zzzz, cb0[7].xyzx
    r4.xyz = ((r0.zzzz)*(source[7].xyzx)).xyz;
    // 80: mul r0.y, r0.z, cb0[1].w
    r0.y = ((r0.zzzz)*(source[1].wwww)).y;
    // 81: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 82: mul r4.xyz, r4.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))).xyz;
    // 83: mad_sat r0.xzw, r0.xxxx, r2.xxyz, r4.xxyz
    r0.xzw = (saturate((r0.xxxx)*(r2.xxyz)+(r4.xxyz))).xzw;
    // 84: mad r0.xzw, r1.xxyz, l(0.700000, 0.000000, 0.700000, 0.700000), r0.xxzw
    r0.xzw = ((r1.xxyz)*(float4(0.700000,0.000000,0.700000,0.700000))+(r0.xxzw)).xzw;
    // 85: mad r1.xy, v4.xyxx, l(0.250000, 0.250000, 0.000000, 0.000000), l(0.375000, 0.375000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)*(float4(0.250000,0.250000,0.000000,0.000000))+(float4(0.375000,0.375000,0.000000,0.000000))).xy;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 87: mul r1.x, r1.x, l(5.000000)
    r1.x = ((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 88: round_pi r1.x, r1.x
    r1.x = (ceil(r1.xxxx)).x;
    // 89: mad r1.x, r1.x, l(0.200000), -cb0[5].x
    r1.x = ((r1.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))+(-(source[5].xxxx))).x;
    // 90: round_pi_sat r1.x, r1.x
    r1.x = (saturate(ceil(r1.xxxx))).x;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r1.y, v4.xyxx, t2.xwyz, s2, l(0.000000)
    r1.y = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 92: mul r1.z, |r1.y|, |r1.y|
    r1.z = ((abs(r1.yyyy))*(abs(r1.yyyy))).z;
    // 93: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 94: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 95: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 96: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 97: mad r0.xzw, r1.xxxx, r3.xxyz, r0.xxzw
    r0.xzw = ((r1.xxxx)*(r3.xxyz)+(r0.xxzw)).xzw;
    // 98: mad r0.xzw, r0.xxzw, cb0[1].xxyz, cb0[2].xxyz
    r0.xzw = ((r0.xxzw)*(source[1].xxyz)+(source[2].xxyz)).xzw;
    // 99: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 100: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 101: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4935Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 3: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 4: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 5: log r0.zw, r0.zzzw
    r0.zw = (log2(r0.zzzw)).zw;
    // 6: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 10.000000, 10.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,10.000000,10.000000))).zw;
    // 7: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 8: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 9: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 10: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 11: add r0.yz, v2.xxyx, cb0[1].xxyx
    r0.yz = ((v2.xxyx)+(source[1].xxyx)).yz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 13: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), cb0[0].xxxy
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(source[0].xxxy)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 16: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 17: mul r0.y, r0.y, l(10.000000)
    r0.y = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 18: mul r0.yw, r0.xxxx, r0.yyyy
    r0.yw = ((r0.xxxx)*(r0.yyyy)).yw;
    // 19: mov r0.xz, l(2.000000,0,2.000000,0)
    r0.xz = (float4(2.000000,asfloat(0u),2.000000,asfloat(0u))).xz;
    // 20: mad r0.xyzw, l(0.000000, -2.000000, 0.000000, -2.000000), r0.xyzw, l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((float4(0.000000,-2.000000,0.000000,-2.000000))*(r0.xyzw)+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 21: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 22: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 23: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 24: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 25: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 26: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 27: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 28: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 29: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 30: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 31: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 32: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 33: source device depth mapped to centimetre view depth; reconstruction at 35.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 35-38: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 39: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 40: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 41: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 42: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 43: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_f_pa_wind_05_tr: bd8398a6efa91243bb7de7f4bed27970; selected map b2ae4f2aac6beb7fbe8393c856ec9810b7bff830a2d04ee479916d7f6c017df9.
float4 ArtistNative4936(ARTIST_NATIVE_INPUT input)
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
// fx_a_pa_db_01_2_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative4937(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_f_pa_ri_01_3_ad: 22fe1d84b183a24ba061efbe70ff88fe; selected map dcb4af1b0c51e126ebfcd50d3245e9fb4ef45276b0b47937288bd666ab550511.
float4 ArtistNative4938(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    { const float4 sourceAngle = r1.xxxx; r1.x = (sin(sourceAngle)).x; }
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_05_04_tr: 1166c98a352b934594aa881b9923e0f3; selected map c2d8d01196bc1ad3c52eed22b3401991f762707ca1c0e1467fc60f3a974cf0b1.
float4 ArtistNative4940(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_f_me_fall_01_3_tr: f398dd49b20c6e499c0f2b9948422958; selected map 8aa338d577f2b3da598e24febe07e8631dcfc6a41feca2e4b433eaeb1accf193.
float4 ArtistNative4941(ARTIST_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].xxxx,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-1.65999997, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-2.0, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].xxxx,1u);
    source[9] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-2.0, 0.0, 0.0, 0.0))),1u);
    source[10] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[11] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-3.0, 0.0, 0.0, 0.0))),1u);
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].yyyy,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[13] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(-0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[14] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(0.300000012, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[15] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[16] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-1.37, 0.0, 0.0, 0.0))),1u);
    source[17].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[17].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-2.0, 0.0, 0.0, 0.0)))).x;
    source[17].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[17].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[18].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-1.37, 0.0, 0.0, 0.0)))).x;
    source[18].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[18].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[18].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v4.xyxx, cb0[8].xyxx, cb0[9].xyxx
    r0.xy = ((v4.xyxx)*(source[8].xyxx)+(source[9].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v4.xxyx, cb0[10].xxyx, cb0[11].xxyx
    r0.yz = ((v4.xxyx)*(source[10].xxyx)+(source[11].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: mad_sat r0.x, r0.x, r0.y, l(0.120000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(0.120000,0.120000,0.120000,0.120000)))).x;
    // 6: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 7: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 8: mul r0.y, r0.y, l(1.150000)
    r0.y = ((r0.yyyy)*(float4(1.150000,1.150000,1.150000,1.150000))).y;
    // 9: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 10: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 11: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 12: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 13: mad r0.yz, cb0[4].xxyx, v4.xxyx, cb0[5].xxyx
    r0.yz = ((source[4].xxyx)*(v4.xxyx)+(source[5].xxyx)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mad r0.zw, v4.xxxy, cb0[6].xxxy, cb0[7].xxxy
    r0.zw = ((v4.xxxy)*(source[6].xxxy)+(source[7].xxxy)).zw;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 17: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 18: mad r0.x, r0.y, cb0[17].w, r0.x
    r0.x = ((r0.yyyy)*(source[17].wwww)+(r0.xxxx)).x;
    // 19: mul r0.xyz, r0.xxxx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)).xyz;
    // 20: mad r0.xyz, cb0[3].xyzx, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[3].xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 22: mad r0.xy, v4.xyxx, cb0[12].xyxx, cb0[13].xyxx
    r0.xy = ((v4.xyxx)*(source[12].xyxx)+(source[13].xyxx)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 24: mad r0.yz, v4.xxyx, cb0[12].xxyx, cb0[14].xxyx
    r0.yz = ((v4.xxyx)*(source[12].xxyx)+(source[14].xxyx)).yz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 26: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 27: mad r0.yz, v4.xxyx, cb0[15].xxyx, cb0[16].xxyx
    r0.yz = ((v4.xxyx)*(source[15].xxyx)+(source[16].xxyx)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 30: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 31: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 32: mul r0.y, r0.y, cb0[18].z
    r0.y = ((r0.yyyy)*(source[18].zzzz)).y;
    // 33: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 34: mul r0.y, r0.y, cb0[18].w
    r0.y = ((r0.yyyy)*(source[18].wwww)).y;
    // 35: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 36: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 37: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4941Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].yyyy,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(-0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(0.300000012, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-1.37, 0.0, 0.0, 0.0))),1u);
    source[7] = input.dynamicParameter;
    source[8].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-1.37, 0.0, 0.0, 0.0)))).x;
    source[8].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xyz, -cb0[0].xyzx, cb0[1].xyzx
    r0.xyz = ((-(source[0].xyzx))+(source[1].xyzx)).xyz;
    // 2: dp3 r0.x, r0.xyzx, r0.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(-0.000500)
    r0.x = ((r0.xxxx)+(float4(-0.000500,-0.000500,-0.000500,-0.000500))).x;
    // 6: mul_sat r0.x, r0.x, l(750.000000)
    r0.x = (saturate((r0.xxxx)*(float4(750.000000,750.000000,750.000000,750.000000)))).x;
    // 7: mul r0.x, r0.x, cb0[9].x
    r0.x = ((r0.xxxx)*(source[9].xxxx)).x;
    // 8: mad r0.yz, v2.xxyx, cb0[2].xxyx, cb0[3].xxyx
    r0.yz = ((v2.xxyx)*(source[2].xxyx)+(source[3].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mad r0.zw, v2.xxxy, cb0[2].xxxy, cb0[4].xxxy
    r0.zw = ((v2.xxxy)*(source[2].xxxy)+(source[4].xxxy)).zw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 12: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 13: mad r0.zw, v2.xxxy, cb0[5].xxxy, cb0[6].xxxy
    r0.zw = ((v2.xxxy)*(source[5].xxxy)+(source[6].xxxy)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: mul_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)*(r0.yyyy))).y;
    // 16: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 17: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[8].z
    r0.z = ((r0.zzzz)*(source[8].zzzz)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 21: mul r0.x, r0.x, cb0[7].x
    r0.x = ((r0.xxxx)*(source[7].xxxx)).x;
    // 22: mad r1.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 23: movc r0.xyzw, r0.yyyy, l(-1.000000,1.000000,-1.000000,1.000000), r1.xyzw
    r0.xyzw = ((asuint(r0.yyyy) != 0u) ? (float4(-1.000000,1.000000,-1.000000,1.000000)) : (r1.xyzw)).xyzw;
    // 24: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 25: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 26: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 27: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 28: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 29: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 30: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 31: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 32: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 33: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 34: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 35: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 36: source device depth mapped to centimetre view depth; reconstruction at 38.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 38-41: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 42: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 43: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 44: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 45: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 46: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_b_pa_water_01_ad: 54768850dad1e64b9f8d95faebca3c42; selected map e790e57055400b20b192bcbb689f0c99d7d98ba205ce619d8c809197e31e9730.
float4 ArtistNative4942(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v2.xyxx, l(1.000000, 7.000000, 0.000000, 0.000000), cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(float4(1.000000,7.000000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 5: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 6: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: add r0.w, r0.w, l(0.000010)
    r0.w = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 9: mad r1.xy, v2.xyxx, l(1.400000, 10.000000, 0.000000, 0.000000), cb0[3].xyxx
    r1.xy = ((v2.xyxx)*(float4(1.400000,10.000000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 11: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 13: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 16: add r1.z, r0.w, r1.w
    r1.z = ((r0.wwww)+(r1.wwww)).z;
    // 17: mov r0.z, l(0.000010)
    r0.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // 18: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 19: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 22: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 25: dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 26: mul r0.xy, r0.zzzz, r0.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)).xy;
    // 27: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r1.xyxx))).xy;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t4.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r0.zzzz, t5.xyzw, s4, l(0.000000)
    r1.xy = (ArtistNativeSample3((r0.zzzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 30: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), r0.zzzz
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(r0.zzzz)).zw;
    // 31: add r0.xy, r0.zwzz, r0.xyxx
    r0.xy = ((r0.zwzz)+(r0.xyxx)).xy;
    // 32: add r0.xy, r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t6.xyzw, s5, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 34: mul r0.y, |r0.x|, |r0.x|
    r0.y = ((abs(r0.xxxx))*(abs(r0.xxxx))).y;
    // 35: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 36: mul r0.y, r0.y, l(1000.000000)
    r0.y = ((r0.yyyy)*(float4(1000.000000,1000.000000,1000.000000,1000.000000))).y;
    // 37: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v2.xyxx, t7.wxyz, s6, l(0.000000)
    r0.yzw = (ArtistNativeSample5((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 39: add r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)+(r0.xxxx)).xyz;
    // 40: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 41: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 42: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 43: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 44: source device depth mapped to centimetre view depth; reconstruction at 46.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 46-49: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 50: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 51: mul_sat r0.w, r0.w, l(0.020833)
    r0.w = (saturate((r0.wwww)*(float4(0.020833,0.020833,0.020833,0.020833)))).w;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t2.xyzw, s7, l(0.000000)
    r1.x = (ArtistNativeSample6((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 53: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 54: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 55: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 56: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 57: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4942Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0199999996, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v1.xyxx, l(1.000000, 7.000000, 0.000000, 0.000000), cb0[0].xyxx
    r0.xy = ((v1.xyxx)*(float4(1.000000,7.000000,0.000000,0.000000))+(source[0].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xyzw, r0.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r0.xyzw = ((r0.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 4: mad r1.xy, v1.xyxx, l(1.400000, 10.000000, 0.000000, 0.000000), cb0[1].xyxx
    r1.xy = ((v1.xyxx)*(float4(1.400000,10.000000,0.000000,0.000000))+(source[1].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xyzw, r1.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), r0.xyzw
    r0.xyzw = ((r1.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(r0.xyzw)).xyzw;
    // 7: add r0.xyzw, r0.xyzw, l(-1.000000, -1.000000, -1.000000, -1.000000)
    r0.xyzw = ((r0.xyzw)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample6((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 9: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 10: mul r0.xyzw, r0.xyzw, v3.xxxx
    r0.xyzw = ((r0.xyzw)*(v3.xxxx)).xyzw;
    // 11: mad r0.xyzw, r0.xyzw, l(4.000000, -4.000000, 4.000000, -4.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(4.000000,-4.000000,4.000000,-4.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 12: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 13: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 14: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 15: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 16: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 17: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 18: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 20: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 21: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 22: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 23: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 24: source device depth mapped to centimetre view depth; reconstruction at 26.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 26-29: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 30: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 31: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 32: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 33: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 34: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_05_07_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative4943(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_rgbline_01_ad: 5ea6abf36654c94eae8b523062def76d; selected map ddfa6780fd232822c32a57bef702012872510d7c744aec89366ef16885fdc498.
float4 ArtistNative4944(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4944Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_chromaring_01_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative4945(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_spla_05_01_tr: 11925c9c04a9ad45ae523ecc70566462; selected map 7d7457f0a5af899f6e250b68199b59c5bb9747ed63cfaa80ea44010289b7701f.
float4 ArtistNative4946(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = g_ArtistSourceMaterialParameters[5u];
    source[6] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[9].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)).x;
    source[9].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[12].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0))).x;
    source[12].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[12].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[13].z = ((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)).x;
    source[13].w = (((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[14].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[10].zwzz
    r0.xy = ((v2.xyxx)*(source[10].zwzz)).xy;
    // 2: mad r1.x, cb0[9].y, cb0[10].y, r0.x
    r1.x = ((source[9].yyyy)*(source[10].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[9].y, cb0[11].x, r0.y
    r1.y = ((source[9].yyyy)*(source[11].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.y, v4.z, cb0[11].y
    r0.y = ((v4.zzzz)*(source[11].yyyy)).y;
    // 6: mad r0.xy, r0.xxxx, r0.yyyy, v2.xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(v2.xyxx)).xy;
    // 7: mul r0.z, r0.x, cb0[14].w
    r0.z = ((r0.xxxx)*(source[14].wwww)).z;
    // 8: mad r1.x, cb0[9].y, cb0[14].z, r0.z
    r1.x = ((source[9].yyyy)*(source[14].zzzz)+(r0.zzzz)).x;
    // 9: mul r0.z, r0.y, cb0[15].x
    r0.z = ((r0.yyyy)*(source[15].xxxx)).z;
    // 10: mad r1.y, cb0[9].y, cb0[15].y, r0.z
    r1.y = ((source[9].yyyy)*(source[15].yyyy)+(r0.zzzz)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 12: mov_sat r0.w, v4.y
    r0.w = (saturate(v4.yyyy)).w;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: add r1.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 15: dp2 r2.x, cb0[6].xyxx, r1.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 16: dp2 r2.y, cb0[7].xyxx, r1.xyxx
    r2.y = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 17: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 18: mul r1.xy, r1.xyxx, cb0[8].xyxx
    r1.xy = ((r1.xyxx)*(source[8].xyxx)).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.zxyw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 20: mad r0.z, r1.x, r0.z, -r0.w
    r0.z = ((r1.xxxx)*(r0.zzzz)+(-(r0.wwww))).z;
    // 21: add r0.w, -r1.x, cb0[14].y
    r0.w = ((-(r1.xxxx))+(source[14].yyyy)).w;
    // 22: add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 23: mul_sat r0.z, r0.z, cb0[15].z
    r0.z = (saturate((r0.zzzz)*(source[15].zzzz))).z;
    // 24: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 25: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r1.x, r1.x, cb0[15].w
    r1.x = ((r1.xxxx)*(source[15].wwww)).x;
    // 27: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 28: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 29: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 30: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 31: movc o0.w, r0.z, l(0), r0.w
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 32: mad r0.zw, cb0[11].zzzz, r0.xxxy, cb0[2].xxxy
    r0.zw = ((source[11].zzzz)*(r0.xxxy)+(source[2].xxxy)).zw;
    // 33: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t3.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 36: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 37: mul r0.x, r0.x, cb0[12].w
    r0.x = ((r0.xxxx)*(source[12].wwww)).x;
    // 38: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 40: mul r0.x, r0.x, cb0[13].x
    r0.x = ((r0.xxxx)*(source[13].xxxx)).x;
    // 41: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 42: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 43: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 44: add r0.xyz, r0.xyzx, cb0[5].xyzx
    r0.xyz = ((r0.xyzx)+(source[5].xyzx)).xyz;
    // 45: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 46: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4946Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)).x;
    source[3].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[4].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xy, v1.xyxx, cb0[4].zwzz
    r0.xy = ((v1.xyxx)*(source[4].zwzz)).xy;
    // 2: mad r1.x, cb0[3].y, cb0[4].y, r0.x
    r1.x = ((source[3].yyyy)*(source[4].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[3].y, cb0[5].x, r0.y
    r1.y = ((source[3].yyyy)*(source[5].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.y, v3.z, cb0[5].y
    r0.y = ((v3.zzzz)*(source[5].yyyy)).y;
    // 6: mad r0.xy, r0.xxxx, r0.yyyy, v1.xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(v1.xyxx)).xy;
    // 7: mul r0.z, r0.x, cb0[6].w
    r0.z = ((r0.xxxx)*(source[6].wwww)).z;
    // 8: mad r1.x, cb0[3].y, cb0[6].z, r0.z
    r1.x = ((source[3].yyyy)*(source[6].zzzz)+(r0.zzzz)).x;
    // 9: mul r0.z, r0.y, cb0[7].x
    r0.z = ((r0.yyyy)*(source[7].xxxx)).z;
    // 10: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 11: mad r1.y, cb0[3].y, cb0[7].y, r0.z
    r1.y = ((source[3].yyyy)*(source[7].yyyy)+(r0.zzzz)).y;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: dp2 r1.x, cb0[0].xyxx, r0.xyxx
    r1.x = (dot((source[0].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 14: dp2 r1.y, cb0[1].xyxx, r0.xyxx
    r1.y = (dot((source[1].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 15: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.zxyw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 18: mov_sat r0.y, v3.y
    r0.y = (saturate(v3.yyyy)).y;
    // 19: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 20: mad r0.y, r0.x, r0.z, -r0.y
    r0.y = ((r0.xxxx)*(r0.zzzz)+(-(r0.yyyy))).y;
    // 21: add r0.x, -r0.x, cb0[6].y
    r0.x = ((-(r0.xxxx))+(source[6].yyyy)).x;
    // 22: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 23: mul_sat r0.y, r0.y, cb0[7].z
    r0.y = (saturate((r0.yyyy)*(source[7].zzzz))).y;
    // 24: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 25: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 27: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 28: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 29: mul r0.z, v3.w, cb0[8].x
    r0.z = ((v3.wwww)*(source[8].xxxx)).z;
    // 30: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 31: mad r1.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 32: movc r0.xyzw, r0.yyyy, l(-1.000000,1.000000,-1.000000,1.000000), r1.xyzw
    r0.xyzw = ((asuint(r0.yyyy) != 0u) ? (float4(-1.000000,1.000000,-1.000000,1.000000)) : (r1.xyzw)).xyzw;
    // 33: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 34: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 35: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 36: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 37: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 38: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 39: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 40: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 41: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 42: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 43: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 44: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 45: source device depth mapped to centimetre view depth; reconstruction at 47.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 47-50: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 51: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 52: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 53: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 54: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 55: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_c_pa_lightflare_01_ddt_4_ad: 62d22e79ebe9a24b98406751d77a5ac9; selected map 059656cc63615f570a09b85d568328904dd0a9354e3275e24447a0a5b6966d25.
float4 ArtistNative4947(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[4].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].z = (((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].w = (sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[5].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[5].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    r0.yzw = (ArtistNativeSample0((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_g_pa_colorring_half_01_ad: d2f98fa3ff9867468d6951becfbcf7e7; selected map 5651218fc865f2d1162a0637698d7183bb98441305f2a03f0e35e99d2ff77bfb.
float4 ArtistNative4948(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[9u];
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[5u].xxxx,1u);
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[5].x = ((g_ArtistSourceMaterialParameters[4u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].w = ((g_ArtistSourceMaterialParameters[0u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[7].x = (((g_ArtistSourceMaterialParameters[0u].xxxx*g_ArtistSourceMaterialTime.xxxx)*g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[7].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[8].x = ((g_ArtistSourceMaterialParameters[6u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[8].y = (((g_ArtistSourceMaterialParameters[6u].zzzz*g_ArtistSourceMaterialTime.xxxx)*g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[8].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].y = ((g_ArtistSourceMaterialParameters[2u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].z = ((float4(0.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
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
    // 27: mul r0.y, r0.y, cb0[4].z
    r0.y = ((r0.yyyy)*(source[4].zzzz)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[4].wwww
    r0.z = (dot((r0.zzzz).xy,(source[4].wwww).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r1.x, r0.y, cb0[5].x
    r1.x = ((r0.yyyy)+(source[5].xxxx)).x;
    // 32: mul r2.x, r1.x, cb0[9].z
    r2.x = ((r1.xxxx)*(source[9].zzzz)).x;
    // 33: mul r3.x, r1.x, cb0[7].y
    r3.x = ((r1.xxxx)*(source[7].yyyy)).x;
    // 34: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 35: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 36: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 38: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 39: mul r0.y, r0.y, cb0[5].z
    r0.y = ((r0.yyyy)*(source[5].zzzz)).y;
    // 40: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 41: add r0.y, v4.x, cb0[5].w
    r0.y = ((v4.xxxx)+(source[5].wwww)).y;
    // 42: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 43: mad r3.y, r1.y, cb0[7].z, cb0[8].y
    r3.y = ((r1.yyyy)*(source[7].zzzz)+(source[8].yyyy)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r3.xyxx, t0.xyzw, s1, l(-1.000000)
    r0.xy = (ArtistNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 45: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 46: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 47: mul r0.xy, r0.xyxx, cb0[8].zzzz
    r0.xy = ((r0.xyxx)*(source[8].zzzz)).xy;
    // 48: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 49: mad r2.y, r1.y, cb0[9].w, cb0[10].y
    r2.y = ((r1.yyyy)*(source[9].wwww)+(source[10].yyyy)).y;
    // 50: mad r0.zw, cb0[8].wwww, r0.xxxy, r2.xxxy
    r0.zw = ((source[8].wwww)*(r0.xxxy)+(r2.xxxy)).zw;
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
    // 57: mul r2.xyz, r2.xyzx, cb0[10].zzzz
    r2.xyz = ((r2.xyzx)*(source[10].zzzz)).xyz;
    // 58: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 59: mul r3.xyz, r2.xyzx, cb0[10].wwww
    r3.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // 60: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 61: mad r2.xyz, -cb0[10].wwww, r2.xyzx, r0.zzzz
    r2.xyz = ((-(source[10].wwww))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 62: mad r2.xyz, cb0[11].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[11].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 63: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 64: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[3].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[3].xxxy)).zw;
    // 65: mul r3.x, r1.x, cb0[6].x
    r3.x = ((r1.xxxx)*(source[6].xxxx)).x;
    // 66: mad r3.y, r1.y, cb0[6].y, cb0[7].x
    r3.y = ((r1.yyyy)*(source[6].yyyy)+(source[7].xxxx)).y;
    // 67: mad r1.xy, cb0[8].wwww, r0.xyxx, r3.xyxx
    r1.xy = ((source[8].wwww)*(r0.xyxx)+(r3.xyxx)).xy;
    // 68: mad r0.xy, cb0[8].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[8].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s3, l(-1.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 70: mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 71: mul r1.xy, r1.xyxx, l(1.500000, 0.250000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(1.500000,0.250000,0.000000,0.000000))).xy;
    // 72: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 73: mul r1.xyz, r0.xyzx, cb0[11].wwww
    r1.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 74: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 75: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 76: mul r1.xyz, r1.xyzx, cb0[12].xxxx
    r1.xyz = ((r1.xyzx)*(source[12].xxxx)).xyz;
    // 77: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 78: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 79: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 80: mul r1.w, r1.w, cb0[9].x
    r1.w = ((r1.wwww)*(source[9].xxxx)).w;
    // 81: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 82: mul r1.w, r1.w, cb0[9].y
    r1.w = ((r1.wwww)*(source[9].yyyy)).w;
    // 83: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 84: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 85: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 86: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 87: mad r1.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 88: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 89: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 91: mul r1.x, r1.x, cb0[12].z
    r1.x = ((r1.xxxx)*(source[12].zzzz)).x;
    // 92: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 93: mul r1.x, r1.x, cb0[12].w
    r1.x = ((r1.xxxx)*(source[12].wwww)).x;
    // 94: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 95: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 97: log r1.y, r0.w
    r1.y = (log2(r0.wwww)).y;
    // 98: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 99: mul r1.y, r1.y, cb0[12].y
    r1.y = ((r1.yyyy)*(source[12].yyyy)).y;
    // 100: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 101: movc r0.w, r0.w, l(0), r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).w;
    // 102: mul_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)*(r0.wwww))).w;
    // 103: mul r0.w, r0.w, cb0[13].x
    r0.w = ((r0.wwww)*(source[13].xxxx)).w;
    // 104: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 105: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 106: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 107: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4948Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[8u];
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[5u].xxxx,1u);
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[4u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].w = ((g_ArtistSourceMaterialParameters[0u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[5].x = (((g_ArtistSourceMaterialParameters[0u].xxxx*g_ArtistSourceMaterialTime.xxxx)*g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[5].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[6].x = ((g_ArtistSourceMaterialParameters[6u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[6].y = (((g_ArtistSourceMaterialParameters[6u].zzzz*g_ArtistSourceMaterialTime.xxxx)*g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[6].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].y = ((g_ArtistSourceMaterialParameters[2u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[8].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].z = ((float4(0.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 29: mul r0.yz, r0.yyzy, cb0[2].zzwz
    r0.yz = ((r0.yyzy)*(source[2].zzwz)).yz;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r1.x, r0.y, cb0[3].x
    r1.x = ((r0.yyyy)+(source[3].xxxx)).x;
    // 32: mul r2.x, r1.x, cb0[7].z
    r2.x = ((r1.xxxx)*(source[7].zzzz)).x;
    // 33: mul r3.x, r1.x, cb0[5].y
    r3.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 34: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 35: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 36: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
    // 38: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 39: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 40: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 41: add r0.y, v3.x, cb0[3].w
    r0.y = ((v3.xxxx)+(source[3].wwww)).y;
    // 42: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 43: mad r3.y, r1.y, cb0[5].z, cb0[6].y
    r3.y = ((r1.yyyy)*(source[5].zzzz)+(source[6].yyyy)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r3.xyxx, t0.xyzw, s2, l(-1.000000)
    r0.xy = (ArtistNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 45: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 46: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 47: mul r0.xy, r0.xyxx, cb0[6].zzzz
    r0.xy = ((r0.xyxx)*(source[6].zzzz)).xy;
    // 48: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 49: mad r2.y, r1.y, cb0[7].w, cb0[8].y
    r2.y = ((r1.yyyy)*(source[7].wwww)+(source[8].yyyy)).y;
    // 50: mad r0.zw, cb0[6].wwww, r0.xxxy, r2.xxxy
    r0.zw = ((source[6].wwww)*(r0.xxxy)+(r2.xxxy)).zw;
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
    // 57: mul r2.xyz, r2.xyzx, cb0[8].zzzz
    r2.xyz = ((r2.xyzx)*(source[8].zzzz)).xyz;
    // 58: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 59: mul r3.xyz, r2.xyzx, cb0[8].wwww
    r3.xyz = ((r2.xyzx)*(source[8].wwww)).xyz;
    // 60: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 61: mad r2.xyzw, -cb0[8].wwww, r2.xyxy, r0.zzzz
    r2.xyzw = ((-(source[8].wwww))*(r2.xyxy)+(r0.zzzz)).xyzw;
    // 62: mad r2.xyzw, cb0[9].xxxx, r2.xyzw, r3.xyxy
    r2.xyzw = ((source[9].xxxx)*(r2.xyzw)+(r3.xyxy)).xyzw;
    // 63: mul r2.xyzw, r2.xyzw, cb0[0].xyxy
    r2.xyzw = ((r2.xyzw)*(source[0].xyxy)).xyzw;
    // 64: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[1].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[1].xxxy)).zw;
    // 65: mul r3.x, r1.x, cb0[4].x
    r3.x = ((r1.xxxx)*(source[4].xxxx)).x;
    // 66: mad r3.y, r1.y, cb0[4].y, cb0[5].x
    r3.y = ((r1.yyyy)*(source[4].yyyy)+(source[5].xxxx)).y;
    // 67: mad r1.xy, cb0[6].wwww, r0.xyxx, r3.xyxx
    r1.xy = ((source[6].wwww)*(r0.xyxx)+(r3.xyxx)).xy;
    // 68: mad r0.xy, cb0[6].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s4, l(-1.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 70: mul r0.xyzw, r2.xyzw, r0.xxxx
    r0.xyzw = ((r2.xyzw)*(r0.xxxx)).xyzw;
    // 71: mul r1.xy, r1.xyxx, l(1.500000, 0.250000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(1.500000,0.250000,0.000000,0.000000))).xy;
    // 72: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s1, l(-1.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 73: mul r2.xyzw, r0.xyzw, cb0[9].wwww
    r2.xyzw = ((r0.xyzw)*(source[9].wwww)).xyzw;
    // 74: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 75: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 76: mul r2.xyzw, r2.xyzw, cb0[10].xxxx
    r2.xyzw = ((r2.xyzw)*(source[10].xxxx)).xyzw;
    // 77: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 78: log r0.x, |r1.x|
    r0.x = (log2(abs(r1.xxxx))).x;
    // 79: lt r0.y, |r1.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: mul r0.x, r0.x, cb0[7].x
    r0.x = ((r0.xxxx)*(source[7].xxxx)).x;
    // 81: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 82: mul r0.x, r0.x, cb0[7].y
    r0.x = ((r0.xxxx)*(source[7].yyyy)).x;
    // 83: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 84: mad r0.xyzw, r0.xxxx, r0.zwzw, r2.xyzw
    r0.xyzw = ((r0.xxxx)*(r0.zwzw)+(r2.xyzw)).xyzw;
    // 85: mul r0.xyzw, r0.xyzw, cb0[11].zzzz
    r0.xyzw = ((r0.xyzw)*(source[11].zzzz)).xyzw;
    // 86: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 87: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 88: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 89: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 90: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 91: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 92: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 93: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 94: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 95: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 96: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 97: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 98: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 99: source device depth mapped to centimetre view depth; reconstruction at 101.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 101-104: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 105: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 106: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 107: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 108: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 109: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_gl_01_4_ad: b0d551a53864c9408d2c03215f9ef88f; selected map 4dd93e1b4ffae10c2db3ed0c4c9132c6db631aa6aa2fdbffcc7ea3e1b5d89ab4.
float4 ArtistNative4949(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    r1.xyzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4949Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_me_fd_02_2_ts_tr: 6f7d48813f113e4f9e74351373f7d1e3; selected map ca7bd339e01a327c65bba9fc0db7e33c191e6200b995be7f47c35be66d8957e1.
float4 ArtistNative4950(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f;
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
    // 28: mul r1.xy, r0.yyyy, cb0[4].wyww
    r1.xy = ((r0.yyyy)*(source[4].wyww)).xy;
    // 29: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 30: mul r0.yz, r0.yyyy, l(0.000000, 0.200000, 0.400000, 0.000000)
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.200000,0.400000,0.000000))).yz;
    // 31: exp r0.yz, r0.yyzy
    r0.yz = (exp2(r0.yyzy)).yz;
    // 32: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 33: mul r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)*(source[5].xxxx)).y;
    // 34: lt r0.w, r0.x, l(0.000001)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 36: mad r2.xy, -r0.xxxx, l(4.000000, 5.000000, 0.000000, 0.000000), l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r0.xxxx))*(float4(4.000000,5.000000,0.000000,0.000000))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 37: mul r2.xy, r2.xyxx, l(0.400000, 0.200000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(0.400000,0.200000,0.000000,0.000000))).xy;
    // 38: max r2.xy, r2.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (max(r2.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 39: movc r1.zw, r0.wwww, l(0,0,0,0), r0.zzzy
    r1.zw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzy)).zw;
    // 40: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 41: mul r0.z, cb0[3].x, cb0[4].x
    r0.z = ((source[3].xxxx)*(source[4].xxxx)).z;
    // 42: mov r0.y, -r0.z
    r0.y = (-(r0.zzzz)).y;
    // 43: add r0.xy, r1.yzyy, r0.xyxx
    r0.xy = ((r1.yzyy)+(r0.xyxx)).xy;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 45: mad r1.xyzw, r0.xxxx, l(0.500000, 0.500000, 0.500000, 0.500000), r1.xwxw
    r1.xyzw = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xwxw)).xyzw;
    // 46: mad r1.xyzw, r0.zzzz, l(-0.300000, -0.700000, -0.010000, -0.500000), r1.xyzw
    r1.xyzw = ((r0.zzzz)*(float4(-0.300000,-0.700000,-0.010000,-0.500000))+(r1.xyzw)).xyzw;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
    // 49: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 50: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 51: add_sat r0.z, -r2.y, r0.x
    r0.z = (saturate((-(r2.yyyy))+(r0.xxxx))).z;
    // 52: mul r0.z, r0.z, l(0.200000)
    r0.z = ((r0.zzzz)*(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 53: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 1.200000, 1.200000), r0.zzzz
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,1.200000,1.200000))+(r0.zzzz)).zw;
    // 54: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.100000, -0.100000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.100000,-0.100000))).zw;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.xyzw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
    // 56: mul r0.xw, r0.zzzz, r0.xxxy
    r0.xw = ((r0.zzzz)*(r0.xxxy)).xw;
    // 57: mul r0.z, |r0.w|, |r0.w|
    r0.z = ((abs(r0.wwww))*(abs(r0.wwww))).z;
    // 58: mul r0.z, r0.z, |r0.w|
    r0.z = ((r0.zzzz)*(abs(r0.wwww))).z;
    // 59: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 60: mul r0.z, r0.z, l(2000.000000)
    r0.z = ((r0.zzzz)*(float4(2000.000000,2000.000000,2000.000000,2000.000000))).z;
    // 61: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 62: mad r0.x, r0.x, l(3.000000), r0.z
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(r0.zzzz)).x;
    // 63: add_sat r0.z, -r2.y, r0.y
    r0.z = (saturate((-(r2.yyyy))+(r0.yyyy))).z;
    // 64: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 65: mad r0.xzw, r0.xxxx, cb0[1].xxyz, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[1].xxyz)+(source[2].xxyz)).xzw;
    // 66: mad o0.xyz, r0.xzwx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xzwx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 67: mul r0.x, r0.y, l(10.000000)
    r0.x = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 68: mad r0.yz, r0.yyyy, l(0.000000, 0.600000, 0.600000, 0.000000), v4.xxyx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.600000,0.600000,0.000000))+(v4.xxyx)).yz;
    // 69: add r0.yz, r0.yyzy, l(0.000000, -0.530000, -0.520000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.530000,-0.520000,0.000000))).yz;
    // 70: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 71: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 72: mad r0.y, -r0.y, l(2.857143), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.857143,2.857143,2.857143,2.857143))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 73: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 74: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 75: mul r0.y, r0.y, l(20.000000)
    r0.y = ((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000))).y;
    // 76: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 77: mad r0.x, r2.x, l(10.000000), r0.x
    r0.x = ((r2.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 78: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 79: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 80: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 81: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 82: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 83: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 84: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r0.z, r0.z, l(1.050000)
    r0.z = ((r0.zzzz)*(float4(1.050000,1.050000,1.050000,1.050000))).z;
    // 86: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 87: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 88: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 89: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 90: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 91: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_me_fd_02_3_ts_tr: 6f7d48813f113e4f9e74351373f7d1e3; selected map ca7bd339e01a327c65bba9fc0db7e33c191e6200b995be7f47c35be66d8957e1.
float4 ArtistNative4951(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f;
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
    // 28: mul r1.xy, r0.yyyy, cb0[4].wyww
    r1.xy = ((r0.yyyy)*(source[4].wyww)).xy;
    // 29: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 30: mul r0.yz, r0.yyyy, l(0.000000, 0.200000, 0.400000, 0.000000)
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.200000,0.400000,0.000000))).yz;
    // 31: exp r0.yz, r0.yyzy
    r0.yz = (exp2(r0.yyzy)).yz;
    // 32: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 33: mul r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)*(source[5].xxxx)).y;
    // 34: lt r0.w, r0.x, l(0.000001)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 36: mad r2.xy, -r0.xxxx, l(4.000000, 5.000000, 0.000000, 0.000000), l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r0.xxxx))*(float4(4.000000,5.000000,0.000000,0.000000))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 37: mul r2.xy, r2.xyxx, l(0.400000, 0.200000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(0.400000,0.200000,0.000000,0.000000))).xy;
    // 38: max r2.xy, r2.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (max(r2.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 39: movc r1.zw, r0.wwww, l(0,0,0,0), r0.zzzy
    r1.zw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzy)).zw;
    // 40: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 41: mul r0.z, cb0[3].x, cb0[4].x
    r0.z = ((source[3].xxxx)*(source[4].xxxx)).z;
    // 42: mov r0.y, -r0.z
    r0.y = (-(r0.zzzz)).y;
    // 43: add r0.xy, r1.yzyy, r0.xyxx
    r0.xy = ((r1.yzyy)+(r0.xyxx)).xy;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 45: mad r1.xyzw, r0.xxxx, l(0.500000, 0.500000, 0.500000, 0.500000), r1.xwxw
    r1.xyzw = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xwxw)).xyzw;
    // 46: mad r1.xyzw, r0.zzzz, l(-0.300000, -0.700000, -0.010000, -0.500000), r1.xyzw
    r1.xyzw = ((r0.zzzz)*(float4(-0.300000,-0.700000,-0.010000,-0.500000))+(r1.xyzw)).xyzw;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
    // 49: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 50: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 51: add_sat r0.z, -r2.y, r0.x
    r0.z = (saturate((-(r2.yyyy))+(r0.xxxx))).z;
    // 52: mul r0.z, r0.z, l(0.200000)
    r0.z = ((r0.zzzz)*(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 53: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 1.200000, 1.200000), r0.zzzz
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,1.200000,1.200000))+(r0.zzzz)).zw;
    // 54: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.100000, -0.100000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.100000,-0.100000))).zw;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.xyzw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
    // 56: mul r0.xw, r0.zzzz, r0.xxxy
    r0.xw = ((r0.zzzz)*(r0.xxxy)).xw;
    // 57: mul r0.z, |r0.w|, |r0.w|
    r0.z = ((abs(r0.wwww))*(abs(r0.wwww))).z;
    // 58: mul r0.z, r0.z, |r0.w|
    r0.z = ((r0.zzzz)*(abs(r0.wwww))).z;
    // 59: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 60: mul r0.z, r0.z, l(2000.000000)
    r0.z = ((r0.zzzz)*(float4(2000.000000,2000.000000,2000.000000,2000.000000))).z;
    // 61: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 62: mad r0.x, r0.x, l(3.000000), r0.z
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(r0.zzzz)).x;
    // 63: add_sat r0.z, -r2.y, r0.y
    r0.z = (saturate((-(r2.yyyy))+(r0.yyyy))).z;
    // 64: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 65: mad r0.xzw, r0.xxxx, cb0[1].xxyz, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[1].xxyz)+(source[2].xxyz)).xzw;
    // 66: mad o0.xyz, r0.xzwx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xzwx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 67: mul r0.x, r0.y, l(10.000000)
    r0.x = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 68: mad r0.yz, r0.yyyy, l(0.000000, 0.600000, 0.600000, 0.000000), v4.xxyx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.600000,0.600000,0.000000))+(v4.xxyx)).yz;
    // 69: add r0.yz, r0.yyzy, l(0.000000, -0.530000, -0.520000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.530000,-0.520000,0.000000))).yz;
    // 70: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 71: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 72: mad r0.y, -r0.y, l(2.857143), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.857143,2.857143,2.857143,2.857143))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 73: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 74: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 75: mul r0.y, r0.y, l(20.000000)
    r0.y = ((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000))).y;
    // 76: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 77: mad r0.x, r2.x, l(10.000000), r0.x
    r0.x = ((r2.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 78: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 79: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 80: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 81: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 82: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 83: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 84: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r0.z, r0.z, l(1.050000)
    r0.z = ((r0.zzzz)*(float4(1.050000,1.050000,1.050000,1.050000))).z;
    // 86: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 87: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 88: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 89: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 90: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 91: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_gl_01_9_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ArtistNative4952(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_bubblesq_01_tr: 5ee6df364f5a90479ebaa628d7fcf54d; selected map 127eb09ef9912ad892b985544d5d5c748fd4a8d7254dc399a9913c5f2613f758.
float4 ArtistNative4953(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww),1u);
    source[3] = ArtistNativeAppend(float4(1.0, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 2: mov r0.y, v4.x
    r0.y = (v4.xxxx).y;
    // 3: mad r0.xy, v2.xyxx, l(0.250000, 0.250000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(0.250000,0.250000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, v2.xyxx, t2.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 5: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 6: mad r0.xy, cb0[4].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[4].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 7: add r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)+(source[2].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 10: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 11: mad r0.xyz, cb0[5].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 12: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 13: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 14: mul r0.xyz, r0.xyzx, cb0[5].yyyy
    r0.xyz = ((r0.xyzx)*(source[5].yyyy)).xyz;
    // 15: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, cb0[5].zzzz
    r0.xyz = ((r0.xyzx)*(source[5].zzzz)).xyz;
    // 17: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 18: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 19: mul r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(source[3].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 21: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 22: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 23: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 24: mul r0.x, r0.x, cb0[6].x
    r0.x = ((r0.xxxx)*(source[6].xxxx)).x;
    // 25: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 27: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 28: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 29: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif
