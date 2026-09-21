// Original Kouku material programs 4352..4415; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_skull_02_20_tr: 456fc57bd455014b93b97e18c9390a4f; selected map 45f9f31688792848e09af99f5a64410bef1bac4520e1f2c9707f5c31e5b7fdc4.
float4 ArtistNative4352(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[8u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[7u].yyyy,g_ArtistSourceMaterialParameters[7u].zzzz,1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].yyyy,g_ArtistSourceMaterialParameters[4u].zzzz,1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[10].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[10].z = ((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[10].w = (((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))).x;
    source[11].x = (((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[12].x = (((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[12].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0)))).x;
    source[13].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].w = ((g_ArtistSourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))).x;
    source[14].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[15].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[15].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[15].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[17].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].xxxx))).x;
    source[17].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[17].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[17].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[18].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[18].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[19].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 1: mad r0.xy, v2.xyxx, cb0[11].yzyy, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[11].yzyy)+(source[2].xyxx)).xy;
    // 2: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, v2.xxyx, cb0[12].yyzy, cb0[4].xxyx
    r0.yz = ((v2.xxyx)*(source[12].yyzy)+(source[4].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, cb0[13].y
    r0.x = ((r0.xxxx)*(source[13].yyyy)).x;
    // 8: mad r0.yz, v2.xxyx, cb0[14].xxyx, cb0[5].xxyx
    r0.yz = ((v2.xxyx)*(source[14].xxyx)+(source[5].xxyx)).yz;
    // 9: mad r1.xy, cb0[15].xxxx, r0.xxxx, r0.yzyy
    r1.xy = ((source[15].xxxx)*(r0.xxxx)+(r0.yzyy)).xy;
    // 10: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 11: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: dp2 r2.x, cb0[7].xyxx, r1.xyxx
    r2.x = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 13: dp2 r2.y, cb0[8].xyxx, r1.xyxx
    r2.y = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 14: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: mul r1.x, r0.x, cb0[15].x
    r1.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 17: mad r0.yz, r1.xxxx, cb0[15].wwww, r0.yyzy
    r0.yz = ((r1.xxxx)*(source[15].wwww)+(r0.yyzy)).yz;
    // 18: add r0.yz, r0.yyzy, cb0[6].xxyx
    r0.yz = ((r0.yyzy)+(source[6].xxyx)).yz;
    // 19: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 20: dp2 r1.x, cb0[7].xyxx, r0.yzyy
    r1.x = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 21: dp2 r1.y, cb0[8].xyxx, r0.yzyy
    r1.y = (dot((source[8].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 22: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 24: mul r0.z, r0.y, r0.w
    r0.z = ((r0.yyyy)*(r0.wwww)).z;
    // 25: mad r0.w, r0.w, r0.y, r0.w
    r0.w = ((r0.wwww)*(r0.yyyy)+(r0.wwww)).w;
    // 26: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 27: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 28: mul r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 29: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 30: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 31: mul r0.z, r0.z, r0.x
    r0.z = ((r0.zzzz)*(r0.xxxx)).z;
    // 32: mul r0.x, r0.x, l(0.300000)
    r0.x = ((r0.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))).x;
    // 33: mad r1.xy, v2.xyxx, l(1.000000, 0.820000, 0.000000, 0.000000), r0.xxxx
    r1.xy = ((v2.xyxx)*(float4(1.000000,0.820000,0.000000,0.000000))+(r0.xxxx)).xy;
    // 34: add r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)+(source[9].xyxx)).xy;
    // 35: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 36: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 37: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 38: mad r0.x, -r0.x, cb0[17].x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[17].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mul_sat r0.x, r0.x, cb0[18].x
    r0.x = (saturate((r0.xxxx)*(source[18].xxxx))).x;
    // 40: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 41: mul_sat r0.x, r0.x, cb0[18].y
    r0.x = (saturate((r0.xxxx)*(source[18].yyyy))).x;
    // 42: mad r0.x, r0.y, cb0[16].w, r0.x
    r0.x = ((r0.yyyy)*(source[16].wwww)+(r0.xxxx)).x;
    // 43: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 44: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 45: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 47: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 48: mul r0.y, r0.y, cb0[16].y
    r0.y = ((r0.yyyy)*(source[16].yyyy)).y;
    // 49: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 50: mad r0.y, r0.w, cb0[16].z, r0.y
    r0.y = ((r0.wwww)*(source[16].zzzz)+(r0.yyyy)).y;
    // 51: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 52: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 53: mul r0.y, v2.x, cb0[18].w
    r0.y = ((v2.xxxx)*(source[18].wwww)).y;
    // 54: mad r1.x, cb0[10].x, cb0[18].z, r0.y
    r1.x = ((source[10].xxxx)*(source[18].zzzz)+(r0.yyyy)).x;
    // 55: mul r0.y, v2.y, cb0[19].x
    r0.y = ((v2.yyyy)*(source[19].xxxx)).y;
    // 56: mad r1.y, cb0[10].x, cb0[19].y, r0.y
    r1.y = ((source[10].xxxx)*(source[19].yyyy)+(r0.yyyy)).y;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 58: add r0.z, v4.x, cb0[19].z
    r0.z = ((v4.xxxx)+(source[19].zzzz)).z;
    // 59: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 60: mul_sat r0.y, r0.y, cb0[19].w
    r0.y = (saturate((r0.yyyy)*(source[19].wwww))).y;
    // 61: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 62: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_spritetransition_01_09_tr: 7f4a593e6dd7de48bac58e2b543cfdd8; selected map 7980e705b5b3eb5abeb43828cc5df7ffef5deb156d8b4f9d0bc99b1122763cb6.
float4 ArtistNative4353(ARTIST_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].zzzz,g_ArtistSourceMaterialParameters[5u].wwww,1u);
    source[3] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].zzzz),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].wwww),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ArtistSourceMaterialParameters[6u];
    source[11] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[13] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].xxxx,g_ArtistSourceMaterialParameters[3u].yyyy,1u);
    source[14] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[15] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[16] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[17] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[18].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[18].w = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[19].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[19].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[20].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[20].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[20].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[21].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[21].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[21].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[21].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
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
    // 2: mul r0.y, v4.y, l(0.050000)
    r0.y = ((v4.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 3: add r0.xy, r0.xyxx, v2.xyxx
    r0.xy = ((r0.xyxx)+(v2.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.xy, r0.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))).xy;
    // 7: mad r0.zw, cb0[2].xxxy, v2.xxxy, r0.xxxy
    r0.zw = ((source[2].xxxy)*(v2.xxxy)+(r0.xxxy)).zw;
    // 8: mad r0.xy, cb0[6].xyxx, v2.xyxx, r0.xyxx
    r0.xy = ((source[6].xyxx)*(v2.xyxx)+(r0.xyxx)).xy;
    // 9: add r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)+(source[7].xyxx)).xy;
    // 10: add r0.zw, r0.zzzw, cb0[3].xxxy
    r0.zw = ((r0.zzzw)+(source[3].xxxy)).zw;
    // 11: add r0.xyzw, r0.xyzw, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((r0.xyzw)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 12: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 13: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 14: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 16: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 17: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 18: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 19: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 20: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 21: add r0.w, v4.x, cb0[18].y
    r0.w = ((v4.xxxx)+(source[18].yyyy)).w;
    // 22: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 23: dp2 r1.x, cb0[8].xyxx, r0.xyxx
    r1.x = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 24: dp2 r1.y, cb0[9].xyxx, r0.xyxx
    r1.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 25: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 28: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 29: mul r0.x, r0.x, cb0[19].w
    r0.x = ((r0.xxxx)*(source[19].wwww)).x;
    // 30: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 31: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 32: add r0.y, v4.z, cb0[20].x
    r0.y = ((v4.zzzz)+(source[20].xxxx)).y;
    // 33: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 34: add r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)+(r0.zzzz)).x;
    // 35: mul_sat r0.xy, r0.xxxx, l(10.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xxxx)*(float4(10.000000,8.000000,0.000000,0.000000)))).xy;
    // 36: add r0.y, -r0.y, r0.x
    r0.y = ((-(r0.yyyy))+(r0.xxxx)).y;
    // 37: mul r0.y, r0.y, cb0[20].y
    r0.y = ((r0.yyyy)*(source[20].yyyy)).y;
    // 38: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 39: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 40: mul r0.z, r0.z, cb0[20].z
    r0.z = ((r0.zzzz)*(source[20].zzzz)).z;
    // 41: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 42: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 43: mul r1.xyz, v3.xyzx, cb0[10].xyzx
    r1.xyz = ((v3.xyzx)*(source[10].xyzx)).xyz;
    // 44: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), cb0[11].xxxy
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(source[11].xxxy)).zw;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 46: mul r0.w, |r0.z|, |r0.z|
    r0.w = ((abs(r0.zzzz))*(abs(r0.zzzz))).w;
    // 47: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 49: movc r1.xyz, r0.zzzz, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 50: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 51: mad r0.yzw, v3.xxyz, l(0.000000, 0.010000, 0.010000, 0.010000), r0.yyzw
    r0.yzw = ((v3.xxyz)*(float4(0.000000,0.010000,0.010000,0.010000))+(r0.yyzw)).yzw;
    // 52: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 53: add r0.yzw, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 54: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 55: mad r0.yz, v2.xxyx, cb0[14].xxyx, cb0[15].xxyx
    r0.yz = ((v2.xxyx)*(source[14].xxyx)+(source[15].xxyx)).yz;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 57: mad r0.zw, v2.xxxy, cb0[12].xxxy, cb0[13].xxxy
    r0.zw = ((v2.xxxy)*(source[12].xxxy)+(source[13].xxxy)).zw;
    // 58: mad r0.yz, r0.yyyy, cb0[21].zzzz, r0.zzwz
    r0.yz = ((r0.yyyy)*(source[21].zzzz)+(r0.zzwz)).yz;
    // 59: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 60: dp2 r1.x, cb0[16].xyxx, r0.yzyy
    r1.x = (dot((source[16].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 61: dp2 r1.y, cb0[17].xyxx, r0.yzyy
    r1.y = (dot((source[17].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 62: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 64: mul r0.y, r0.y, cb0[21].w
    r0.y = ((r0.yyyy)*(source[21].wwww)).y;
    // 65: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 66: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 67: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif
