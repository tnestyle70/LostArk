// Warlord recovered source programs. Carrier filtering preserves the selected VF.
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1088(WARLORD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[2u];
    source[3] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.300000012, 0.0, 0.0, 0.0))),1u);
    source[4] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[5] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.699999988, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[7].y = g_WarlordSourceMaterialTime;
    source[7].z = (g_WarlordSourceMaterialTime*0.0);
    source[7].w = (g_WarlordSourceMaterialTime*-0.300000012);
    source[8].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialTime*-0.699999988);
    source[8].z = (sign((g_WarlordSourceMaterialTime*-0.699999988))*frac(abs((g_WarlordSourceMaterialTime*-0.699999988))));
    source[8].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[9].x = (g_WarlordSourceMaterialTime*1.0);
    source[9].y = (sign((g_WarlordSourceMaterialTime*1.0))*frac(abs((g_WarlordSourceMaterialTime*1.0))));
    source[9].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mad r0.xy, v4.xyxx, l(6.000000, 2.500000, 0.000000, 0.000000), cb0[4].xyxx
    r0.xy = ((v4.xyxx)*(float4(6.000000,2.500000,0.000000,0.000000))+(source[4].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = (WarlordNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    // 9: mad r1.xy, v4.xyxx, l(12.000000, 5.000000, 0.000000, 0.000000), cb0[5].xyxx
    r1.xy = ((v4.xyxx)*(float4(12.000000,5.000000,0.000000,0.000000))+(source[5].xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s3, l(0.000000)
    r1.xy = (WarlordNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    // 19: mad r0.xyz, r0.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(-0.000000, -0.000000, -1.000000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = (WarlordNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 21: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 23: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 26: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: mul r2.xyz, r1.xyzx, l(0.750000, 0.750000, 0.750000, 0.000000)
    r2.xyz = ((r1.xyzx)*(float4(0.750000,0.750000,0.750000,0.000000))).xyz;
    // 28: mov r2.w, l(0)
    r2.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 29: mad r0.xyz, cb0[8].wwww, r0.xyzx, r2.wwzw
    r0.xyz = ((source[8].wwww)*(r0.xyzx)+(r2.wwzw)).xyz;
    // 30: mov r2.z, l(1.000000)
    r2.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 31: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 32: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 33: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 34: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 35: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 36: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 37: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 38: dp3_sat r0.x, r0.xyzx, r2.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // 39: mad r0.yz, v4.xxyx, l(0.000000, 2.000000, 2.000000, 0.000000), cb0[6].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,2.000000,2.000000,0.000000))+(source[6].xxyx)).yz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (WarlordNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: add r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)+(source[9].zzzz)).y;
    // 42: round_ni_sat r0.y, r0.y
    r0.y = (saturate(floor(r0.yyyy))).y;
    // 43: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 44: mul r0.x, r0.x, cb0[9].w
    r0.x = ((r0.xxxx)*(source[9].wwww)).x;
    // 45: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 46: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 47: mul r0.yz, r1.xxyx, l(0.000000, 0.155000, 0.155000, 0.000000)
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.155000,0.155000,0.000000))).yz;
    // 48: mad r1.xy, r1.xyxx, l(0.155000, 0.155000, 0.000000, 0.000000), v4.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.155000,0.155000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 49: add r1.xy, r1.xyxx, cb0[3].xyxx
    r1.xy = ((r1.xyxx)+(source[3].xyxx)).xy;
    // 50: mad r0.yz, r2.xxyx, l(0.000000, -0.550000, -0.550000, 0.000000), r0.yyzy
    r0.yz = ((r2.xxyx)*(float4(0.000000,-0.550000,-0.550000,0.000000))+(r0.yyzy)).yz;
    // 51: add r0.yz, r0.yyzy, cb0[4].xxyx
    r0.yz = ((r0.yyzy)+(source[4].xxyx)).yz;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r1.xyxx, t4.xyzw, s2, l(0.000000)
    r2.y = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 54: add r1.xyzw, r1.xyxy, l(0.015000, 0.015000, -0.015000, -0.015000)
    r1.xyzw = ((r1.xyxy)+(float4(0.015000,0.015000,-0.015000,-0.015000))).xyzw;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t4.xyzw, s2, l(0.000000)
    r2.x = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r1.zwzz, t4.xyzw, s2, l(0.000000)
    r2.z = (WarlordNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
    // 57: mad r0.yzw, r2.xxyz, l(0.000000, 5.000000, 5.000000, 5.000000), r0.yyyy
    r0.yzw = ((r2.xxyz)*(float4(0.000000,5.000000,5.000000,5.000000))+(r0.yyyy)).yzw;
    // 58: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 59: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 60: mad r0.yzw, cb0[8].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[8].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t3.xyzw, s0, l(0.000000)
    r1.xyz = (WarlordNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 62: mul r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, cb0[7].xxxx
    r1.xyz = ((r1.xyzx)*(source[7].xxxx)).xyz;
    // 64: mad r0.yzw, r1.xxyz, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r1.xxyz)*(r0.yyzw)+(source[2].xxyz)).yzw;
    // 65: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 66: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 67: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1089(WARLORD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[2].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x));
    source[2].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0);
    source[3].y = (1.0-((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0));
    source[3].z = max((1.0-((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0)),9.99999975e-06);
    source[3].w = (1.0/max((1.0-((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0)),9.99999975e-06));
    source[4].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_WarlordSourceMaterialParameters[0u].zzzz).x*1.0);
    source[4].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
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
    // 13: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 14: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 15: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1090(WARLORD_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
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
    // 10: mul_sat r0.x, r0.x, l(0.033333)
    r0.x = (saturate((r0.xxxx)*(float4(0.033333,0.033333,0.033333,0.033333)))).x;
    // 11: mul r1.xyzw, v2.xyxy, l(0.500000, 0.500000, 0.500000, 0.500000)
    r1.xyzw = ((v2.xyxy)*(float4(0.500000,0.500000,0.500000,0.500000))).xyzw;
    // 12: mad r1.xyzw, v4.xxxx, l(0.085000, 0.050000, 0.010000, -0.040000), r1.xyzw
    r1.xyzw = ((v4.xxxx)*(float4(0.085000,0.050000,0.010000,-0.040000))+(r1.xyzw)).xyzw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 16: mad r0.yz, r0.yyyy, v4.yyyy, v2.xxyx
    r0.yz = ((r0.yyyy)*(v4.yyyy)+(v2.xxyx)).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s2, l(0.000000)
    r0.yzw = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 18: mul r1.x, r0.y, v3.w
    r1.x = ((r0.yyyy)*(v3.wwww)).x;
    // 19: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 20: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 21: mul_sat r0.y, r1.x, l(5.000000)
    r0.y = (saturate((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).y;
    // 22: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 23: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1091(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialParameters[1u].zzzz*g_WarlordSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialParameters[1u].zzzz*g_WarlordSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[3].w = ((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime);
    source[4].x = (((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime)*1.0);
    source[4].y = (((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime)*0.0);
    source[4].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (sign((((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime)*0.0))*frac(abs((((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime)*0.0))));
    source[5].x = (sign((((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime)*1.0))*frac(abs((((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime)*1.0))));
    source[5].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].w = ((g_WarlordSourceMaterialParameters[0u].xxxx).x*1.0);
    source[7].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
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
    // 30: mad r1.x, cb0[4].z, r0.y, cb0[2].x
    r1.x = ((source[4].zzzz)*(r0.yyyy)+(source[2].xxxx)).x;
    // 31: mul r0.y, v2.x, cb0[5].w
    r0.y = ((v2.xxxx)*(source[5].wwww)).y;
    // 32: mad r2.x, cb0[3].y, cb0[5].z, r0.y
    r2.x = ((source[3].yyyy)*(source[5].zzzz)+(r0.yyyy)).x;
    // 33: mul r0.y, v2.y, cb0[6].x
    r0.y = ((v2.yyyy)*(source[6].xxxx)).y;
    // 34: mad r2.y, cb0[3].y, cb0[6].y, r0.y
    r2.y = ((source[3].yyyy)*(source[6].yyyy)+(r0.yyyy)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 36: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 37: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 38: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 39: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 40: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 41: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 42: add r0.z, v4.x, l(-1.000000)
    r0.z = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 43: add r1.y, r0.x, r0.z
    r1.y = ((r0.xxxx)+(r0.zzzz)).y;
    // 44: mad r0.xz, r0.yyyy, cb0[6].wwww, r1.xxyx
    r0.xz = ((r0.yyyy)*(source[6].wwww)+(r1.xxyx)).xz;
    // 45: mad r0.yw, r0.yyyy, cb0[6].wwww, v2.xxxy
    r0.yw = ((r0.yyyy)*(source[6].wwww)+(v2.xxxy)).yw;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t2.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample2((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t1.xyzw, s0, cb0[3].x
    r0.x = (WarlordNativeSample0((r0.xzxx).xy, (source[3].xxxx).x, true).xyzw).x;
    // 48: mul_sat r0.x, r0.x, cb0[7].x
    r0.x = (saturate((r0.xxxx)*(source[7].xxxx))).x;
    // 49: mov_sat r0.z, v4.y
    r0.z = (saturate(v4.yyyy)).z;
    // 50: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: add r0.x, -r0.z, r0.x
    r0.x = ((-(r0.zzzz))+(r0.xxxx)).x;
    // 52: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 53: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 54: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 55: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 57: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 58: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 59: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 60: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 61: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 62: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1092(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].zzzz,g_WarlordSourceMaterialParameters[3u].wwww,1u);
    source[3] = WarlordNativeAppend(cos((g_WarlordSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = WarlordNativeAppend(sin((g_WarlordSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].yyyy,g_WarlordSourceMaterialParameters[2u].zzzz,1u);
    source[6] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].zzzz,g_WarlordSourceMaterialParameters[0u].wwww,1u);
    source[8].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[9].x = (sign((g_WarlordSourceMaterialTime*0.100000001))*frac(abs((g_WarlordSourceMaterialTime*0.100000001))));
    source[9].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[10].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[11].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    // 10: add r0.y, -cb0[11].w, l(1.000000)
    r0.y = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 14: dp2 r1.x, cb0[3].xyxx, r0.yzyy
    r1.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 15: dp2 r1.y, cb0[4].xyxx, r0.yzyy
    r1.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 16: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 18: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 19: mul r1.xyzw, v2.xyxy, cb0[2].xyxy
    r1.xyzw = ((v2.xyxy)*(source[2].xyxy)).xyzw;
    // 20: mul r1.xyzw, r1.xyzw, l(0.500000, 0.500000, 0.300000, 0.300000)
    r1.xyzw = ((r1.xyzw)*(float4(0.500000,0.500000,0.300000,0.300000))).xyzw;
    // 21: mad r1.xyzw, v4.yyyy, l(0.100000, -0.100000, -0.070000, 0.100000), r1.xyzw
    r1.xyzw = ((v4.yyyy)*(float4(0.100000,-0.100000,-0.070000,0.100000))+(r1.xyzw)).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.xzyw, s1, l(0.000000)
    r0.z = (WarlordNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 24: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 25: mad r0.y, r0.y, r0.z, r0.w
    r0.y = ((r0.yyyy)*(r0.zzzz)+(r0.wwww)).y;
    // 26: mul r0.zw, v2.xxxy, cb0[5].xxxy
    r0.zw = ((v2.xxxy)*(source[5].xxxy)).zw;
    // 27: mad r0.zw, r0.yyyy, cb0[8].yyyy, r0.zzzw
    r0.zw = ((r0.yyyy)*(source[8].yyyy)+(r0.zzzw)).zw;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.zwzz, t2.xyzw, s5, l(0.000000)
    r1.x = (WarlordNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 29: add r0.zw, r0.zzzw, cb0[6].xxxy
    r0.zw = ((r0.zzzw)+(source[6].xxxy)).zw;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r0.zwzz, t5.wxyz, s3, l(0.000000)
    r1.yzw = (WarlordNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 31: max r0.z, |r1.x|, l(0.000001)
    r0.z = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 32: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 33: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 34: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 35: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 36: mad r2.xy, r0.yyyy, cb0[11].zzzz, v2.xyxx
    r2.xy = ((r0.yyyy)*(source[11].zzzz)+(v2.xyxx)).xy;
    // 37: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 38: mad r0.yw, v2.xxxy, cb0[7].xxxy, r0.yyyy
    r0.yw = ((v2.xxxy)*(source[7].xxxy)+(r0.yyyy)).yw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.ywyy, t6.xyzw, s4, l(0.000000)
    r3.xyz = (WarlordNativeSample3((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 40: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 41: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 42: mul r3.xyz, r3.xyzx, cb0[9].wwww
    r3.xyz = ((r3.xyzx)*(source[9].wwww)).xyz;
    // 43: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 44: mad r1.xyz, cb0[10].xxxx, r3.xyzx, r1.yzwy
    r1.xyz = ((source[10].xxxx)*(r3.xyzx)+(r1.yzwy)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t3.yxzw, s6, l(0.000000)
    r0.y = (WarlordNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 46: mul_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)*(r0.zzzz))).y;
    // 47: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 48: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 49: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 50: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 51: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 52: mad r0.xyz, cb0[10].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[10].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 53: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 54: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 55: mul r0.xyz, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 56: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 57: mul r0.xyz, r0.xyzx, cb0[10].wwww
    r0.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 58: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 59: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1093(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[2].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x));
    source[2].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0);
    source[3].y = (1.0-((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0));
    source[3].z = max((1.0-((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0)),9.99999975e-06);
    source[3].w = (1.0/max((1.0-((g_WarlordSourceMaterialParameters[0u].yyyy).x*1.0)),9.99999975e-06));
    source[4].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_WarlordSourceMaterialParameters[0u].zzzz).x*1.0);
    source[4].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].x = (100.0-(g_WarlordSourceMaterialParameters[1u].xxxx).x);
    source[5].y = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[1u].xxxx).x));
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1094(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[5].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[7].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
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
    r0.x = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
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
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1095(WARLORD_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[10u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[5u].xxxx,g_WarlordSourceMaterialParameters[5u].yyyy,1u);
    source[4] = input.dynamicParameter;
    source[5] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].zzzz,g_WarlordSourceMaterialParameters[1u].wwww,1u);
    source[8].x = cos(((g_WarlordSourceMaterialParameters[2u].xxxx).x*0.25));
    source[8].y = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[8].w = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[9].z = (g_WarlordSourceMaterialParameters[7u].wwww).x;
    source[9].w = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[10].x = (g_WarlordSourceMaterialParameters[8u].xxxx).x;
    source[10].y = (g_WarlordSourceMaterialParameters[8u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[11].y = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[11].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].w = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[12].x = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[12].y = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[12].z = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[12].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[13].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[13].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[13].w = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[14].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[14].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[14].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[15].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[15].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[16].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
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
    // 1: mul r0.x, v4.x, cb0[12].w
    r0.x = ((v4.xxxx)*(source[12].wwww)).x;
    // 2: mul r0.y, v4.y, cb0[13].x
    r0.y = ((v4.yyyy)*(source[13].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)+(source[7].xyxx)).xy;
    // 4: mul r0.zw, v4.xxxy, cb0[9].yyyz
    r0.zw = ((v4.xxxy)*(source[9].yyyz)).zw;
    // 5: mad r0.zw, cb0[4].zzzz, cb0[10].xxxy, r0.zzzw
    r0.zw = ((source[4].zzzz)*(source[10].xxxy)+(r0.zzzw)).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mad r0.w, v4.y, cb0[9].z, cb0[9].w
    r0.w = ((v4.yyyy)*(source[9].zzzz)+(source[9].wwww)).w;
    // 8: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 9: mul r0.z, r0.z, cb0[4].y
    r0.z = ((r0.zzzz)*(source[4].yyyy)).z;
    // 10: mad r0.xy, cb0[13].wwww, r0.zzzz, r0.xyxx
    r0.xy = ((source[13].wwww)*(r0.zzzz)+(r0.xyxx)).xy;
    // 11: add r1.xy, cb0[4].xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[4].xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: mad r0.xy, r1.xxxx, cb0[14].xyxx, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[14].xyxx)+(r0.xyxx)).xy;
    // 13: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 14: dp2 r2.x, cb0[5].xyxx, r0.xyxx
    r2.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 15: dp2 r2.y, cb0[6].xyxx, r0.xyxx
    r2.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 16: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 18: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 19: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 20: mul r0.y, r0.y, cb0[14].z
    r0.y = ((r0.yyyy)*(source[14].zzzz)).y;
    // 21: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 22: mul_sat r0.y, r0.y, cb0[14].w
    r0.y = (saturate((r0.yyyy)*(source[14].wwww))).y;
    // 23: mul r1.zw, v4.xxxy, cb0[15].yyyz
    r1.zw = ((v4.xxxy)*(source[15].yyyz)).zw;
    // 24: mad r2.x, cb0[4].z, cb0[15].w, r1.z
    r2.x = ((source[4].zzzz)*(source[15].wwww)+(r1.zzzz)).x;
    // 25: mad r2.y, cb0[4].z, cb0[16].x, r1.w
    r2.y = ((source[4].zzzz)*(source[16].xxxx)+(r1.wwww)).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (WarlordNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 27: add r0.w, -r1.y, r0.w
    r0.w = ((-(r1.yyyy))+(r0.wwww)).w;
    // 28: mul_sat r0.w, r0.w, cb0[16].y
    r0.w = (saturate((r0.wwww)*(source[16].yyyy))).w;
    // 29: add r1.y, -v4.y, l(1.000000)
    r1.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 30: mul r1.y, r1.y, v4.y
    r1.y = ((r1.yyyy)*(v4.yyyy)).y;
    // 31: mul_sat r1.y, r1.y, cb0[15].x
    r1.y = (saturate((r1.yyyy)*(source[15].xxxx))).y;
    // 32: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 33: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 34: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 35: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 36: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 37: mad r0.xy, v4.xyxx, cb0[8].yzyy, cb0[3].xyxx
    r0.xy = ((v4.xyxx)*(source[8].yzyy)+(source[3].xyxx)).xy;
    // 38: mad r0.xy, r0.zzzz, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r0.zzzz)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 39: mad r0.xy, r1.xxxx, cb0[10].zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[10].zwzz)+(r0.xyxx)).xy;
    // 40: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 41: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 42: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 43: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 45: mul r0.yz, cb0[4].zzzz, cb0[11].zzwz
    r0.yz = ((source[4].zzzz)*(source[11].zzwz)).yz;
    // 46: mad r0.yz, v4.xxyx, cb0[11].xxyx, r0.yyzy
    r0.yz = ((v4.xxyx)*(source[11].xxyx)+(r0.yyzy)).yz;
    // 47: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 48: dp2 r1.x, cb0[5].xyxx, r0.yzyy
    r1.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 49: dp2 r1.y, cb0[6].xyxx, r0.yzyy
    r1.y = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 50: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 52: add r0.y, -r0.x, r0.y
    r0.y = ((-(r0.xxxx))+(r0.yyyy)).y;
    // 53: mad r0.x, r0.y, l(0.500000), r0.x
    r0.x = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.xxxx)).x;
    // 54: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 55: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 56: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 57: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 58: mul r0.z, r0.z, cb0[12].x
    r0.z = ((r0.zzzz)*(source[12].xxxx)).z;
    // 59: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 60: mul r0.z, r0.z, cb0[12].y
    r0.z = ((r0.zzzz)*(source[12].yyyy)).z;
    // 61: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 62: mad r0.x, r0.y, cb0[12].z, r0.x
    r0.x = ((r0.yyyy)*(source[12].zzzz)+(r0.xxxx)).x;
    // 63: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 64: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1096(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyz = (WarlordNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyz = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 3: add r0.xyzw, r0.xxyz, -r1.xxyz
    r0.xyzw = ((r0.xxyz)+(-(r1.xxyz))).xyzw;
    // 4: mad r0.xyzw, v0.wwww, r0.xyzw, r1.xxyz
    r0.xyzw = ((v0.wwww)*(r0.xyzw)+(r1.xxyz)).xyzw;
    // 5: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 6: mul r1.x, r1.x, cb0[2].z
    r1.x = ((r1.xxxx)*(source[2].zzzz)).x;
    // 7: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 8: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 9: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: movc r0.x, r0.x, l(0), |r1.x|
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).x;
    // 11: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 12: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 14: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 15: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 16: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 17: movc o0.w, r0.x, l(0), r1.x
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 18: mul r1.xyz, r0.yzwy, cb0[2].xxxx
    r1.xyz = ((r0.yzwy)*(source[2].xxxx)).xyz;
    // 19: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 20: mad r0.xyz, -cb0[2].xxxx, r0.yzwy, r0.xxxx
    r0.xyz = ((-(source[2].xxxx))*(r0.yzwy)+(r0.xxxx)).xyz;
    // 21: mad r0.xyz, cb0[2].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[2].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 22: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 23: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1097(WARLORD_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[9u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[7u].zzzz,g_WarlordSourceMaterialParameters[7u].wwww,1u);
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].wwww,g_WarlordSourceMaterialParameters[4u].xxxx,1u);
    source[4] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_WarlordSourceMaterialParameters[8u];
    source[9].x = (-1.0*sin((((g_WarlordSourceMaterialParameters[4u].wwww).x*6.28000021)*0.25)));
    source[9].y = cos((((g_WarlordSourceMaterialParameters[4u].wwww).x*6.28000021)*0.25));
    source[9].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[9].w = g_WarlordSourceMaterialTime;
    source[10].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[10].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[11].w = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[12].x = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_WarlordSourceMaterialParameters[7u].wwww).x;
    source[12].z = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[12].w = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[13].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[14].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[14].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[15].x = cos(((g_WarlordSourceMaterialParameters[1u].wwww).x*0.25));
    source[15].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[15].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[16].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[16].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[16].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[16].w = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[17].x = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
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
    r0.w = (WarlordNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.w = (WarlordNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
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
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1098(WARLORD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = g_WarlordSourceMaterialParameters[4u];
    source[3].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].x = g_WarlordSourceMaterialTime;
    source[4].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t2.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    // 16: ge r0.y, l(0.250000), r0.x
    r0.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.xxxx)) * 0xffffffffu)).y;
    // 17: mul r0.zw, v2.xxxy, cb0[4].yyyz
    r0.zw = ((v2.xxxy)*(source[4].yyyz)).zw;
    // 18: mad r1.x, cb0[4].x, cb0[3].w, r0.z
    r1.x = ((source[4].xxxx)*(source[3].wwww)+(r0.zzzz)).x;
    // 19: mad r1.y, cb0[4].x, cb0[4].w, r0.w
    r1.y = ((source[4].xxxx)*(source[4].wwww)+(r0.wwww)).y;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 21: mad r0.zw, cb0[5].xxxx, r0.zzzw, v2.xxxy
    r0.zw = ((source[5].xxxx)*(r0.zzzw)+(v2.xxxy)).zw;
    // 22: mad r1.x, cb0[5].y, v4.z, l(-1.000000)
    r1.x = ((source[5].yyyy)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 23: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 24: mul r1.y, v4.z, cb0[5].y
    r1.y = ((v4.zzzz)*(source[5].yyyy)).y;
    // 25: mad r0.zw, r1.yyyy, r0.zzzw, -r1.xxxx
    r0.zw = ((r1.yyyy)*(r0.zzzw)+(-(r1.xxxx))).zw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t1.xyzw, s2, l(0.000000)
    r1.xyzw = (WarlordNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 27: mul r2.xyz, r1.xyzx, cb0[5].zzzz
    r2.xyz = ((r1.xyzx)*(source[5].zzzz)).xyz;
    // 28: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 29: mad r1.xyz, -cb0[5].zzzz, r1.xyzx, r0.zzzz
    r1.xyz = ((-(source[5].zzzz))*(r1.xyzx)+(r0.zzzz)).xyz;
    // 30: mad r1.xyz, cb0[5].wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((source[5].wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 31: mul r2.xyz, r1.xyzx, cb0[6].xxxx
    r2.xyz = ((r1.xyzx)*(source[6].xxxx)).xyz;
    // 32: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 33: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 34: mul r2.xyz, r2.xyzx, cb0[6].yyyy
    r2.xyz = ((r2.xyzx)*(source[6].yyyy)).xyz;
    // 35: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 36: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 37: add r1.xyz, r1.xyzx, r0.zzzz
    r1.xyz = ((r1.xyzx)+(r0.zzzz)).xyz;
    // 38: mul r1.xyz, r1.xyzx, v3.xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)).xyz;
    // 39: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 40: movc r0.yzw, r0.yyyy, r2.xxyz, r1.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (r2.xxyz) : (r1.xxyz)).yzw;
    // 41: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 42: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 43: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 44: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 45: mul r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)*(source[6].zzzz)).y;
    // 46: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 47: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 48: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 49: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 50: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 52: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 53: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 54: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 55: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 56: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1099(WARLORD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = input.dynamicParameter;
    source[4] = g_WarlordSourceMaterialParameters[5u];
    source[5].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[5].z = g_WarlordSourceMaterialTime;
    source[5].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[8].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[10].y = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x));
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 10: add r0.y, -cb0[10].y, l(1.000000)
    r0.y = ((-(source[10].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: max r0.w, |r0.z|, |r0.y|
    r0.w = (max(abs(r0.zzzz),abs(r0.yyyy))).w;
    // 16: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 17: min r1.x, |r0.z|, |r0.y|
    r1.x = (min(abs(r0.zzzz),abs(r0.yyyy))).x;
    // 18: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 19: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 20: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 21: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 22: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 23: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 24: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 25: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 26: lt r1.z, |r0.z|, |r0.y|
    r1.z = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.yyyy))) * 0xffffffffu)).z;
    // 27: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 28: mad r0.w, r0.w, r1.x, r1.y
    r0.w = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).w;
    // 29: lt r1.x, r0.z, -r0.z
    r1.x = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).x;
    // 30: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 31: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 32: min r1.x, r0.z, r0.y
    r1.x = (min(r0.zzzz,r0.yyyy)).x;
    // 33: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 34: max r1.y, r0.z, r0.y
    r1.y = (max(r0.zzzz,r0.yyyy)).y;
    // 35: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 36: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 37: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 38: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: ge r0.z, r1.y, -r1.y
    r0.z = (asfloat((uint4)((r1.yyyy)>=(-(r1.yyyy))) * 0xffffffffu)).z;
    // 40: and r0.z, r0.z, r1.x
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r1.xxxx))).z;
    // 41: movc r0.z, r0.z, -r0.w, r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (-(r0.wwww)) : (r0.wwww)).z;
    // 42: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 43: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 44: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[3].z
    r0.z = ((r0.zzzz)*(source[3].zzzz)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: movc r1.y, r0.y, l(0), r0.z
    r1.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 48: mul r0.yz, r1.xxyx, cb0[8].zzwz
    r0.yz = ((r1.xxyx)*(source[8].zzwz)).yz;
    // 49: mul r0.w, r1.x, cb0[5].w
    r0.w = ((r1.xxxx)*(source[5].wwww)).w;
    // 50: mad r2.x, cb0[5].z, cb0[5].y, r0.w
    r2.x = ((source[5].zzzz)*(source[5].yyyy)+(r0.wwww)).x;
    // 51: mad r3.x, cb0[5].z, cb0[8].y, r0.y
    r3.x = ((source[5].zzzz)*(source[8].yyyy)+(r0.yyyy)).x;
    // 52: mad r3.y, cb0[5].z, cb0[9].x, r0.z
    r3.y = ((source[5].zzzz)*(source[9].xxxx)+(r0.zzzz)).y;
    // 53: mul r0.y, v2.y, cb0[7].x
    r0.y = ((v2.yyyy)*(source[7].xxxx)).y;
    // 54: mad r4.y, cb0[5].z, cb0[7].y, r0.y
    r4.y = ((source[5].zzzz)*(source[7].yyyy)+(r0.yyyy)).y;
    // 55: mul r0.yz, cb0[5].zzzz, cb0[6].zzyz
    r0.yz = ((source[5].zzzz)*(source[6].zzyz)).yz;
    // 56: mad r4.x, cb0[6].w, v2.x, r0.y
    r4.x = ((source[6].wwww)*(v2.xxxx)+(r0.yyyy)).x;
    // 57: mad r2.y, cb0[6].x, r1.y, r0.z
    r2.y = ((source[6].xxxx)*(r1.yyyy)+(r0.zzzz)).y;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r4.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample1((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 59: mul r0.z, cb0[3].x, cb0[7].z
    r0.z = ((source[3].xxxx)*(source[7].zzzz)).z;
    // 60: mad r1.xy, r0.yyyy, r0.zzzz, r3.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(r3.xyxx)).xy;
    // 61: mad r0.yz, r0.yyyy, r0.zzzz, r2.xxyx
    r0.yz = ((r0.yyyy)*(r0.zzzz)+(r2.xxyx)).yz;
    // 62: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s1, cb0[5].x
    r0.yzw = (WarlordNativeSample0((r0.yzyy).xy, (source[5].xxxx).x, true).wxyz).yzw;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s3, cb0[5].x
    r1.x = (WarlordNativeSample2((r1.xyxx).xy, (source[5].xxxx).x, true).xyzw).x;
    // 64: mul_sat r1.x, r1.x, cb0[9].y
    r1.x = (saturate((r1.xxxx)*(source[9].yyyy))).x;
    // 65: mov_sat r1.y, cb0[3].y
    r1.y = (saturate(source[3].yyyy)).y;
    // 66: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 68: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r1.y, v2.xyxx, t2.yxzw, s4, l(0.000000)
    r1.y = (WarlordNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 70: mul_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)*(r1.xxxx))).x;
    // 71: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 72: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 73: mul r1.y, r1.y, cb0[9].z
    r1.y = ((r1.yyyy)*(source[9].zzzz)).y;
    // 74: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 75: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 76: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 77: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 78: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 79: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 80: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 81: mad r0.yzw, cb0[7].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 82: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 83: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 84: mul r0.yzw, r0.yyzw, cb0[8].xxxx
    r0.yzw = ((r0.yyzw)*(source[8].xxxx)).yzw;
    // 85: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 86: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 87: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 88: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 89: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 90: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 91: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1100(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    float4 r0=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (WarlordNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r0.zw, -r0.xxxy, v2.xxxy
    r0.zw = ((-(r0.xxxy))+(v2.xxxy)).zw;
    // 3: mad r0.xy, v4.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((v4.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 4: mul r0.zw, r0.xxxy, cb0[2].zzzz
    r0.zw = ((r0.xxxy)*(source[2].zzzz)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.yxzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 6: mul_sat r0.x, r0.x, cb0[2].x
    r0.x = (saturate((r0.xxxx)*(source[2].xxxx))).x;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t3.yxzw, s3, l(0.000000)
    r0.y = (WarlordNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 8: mul_sat r0.y, r0.y, cb0[2].w
    r0.y = (saturate((r0.yyyy)*(source[2].wwww))).y;
    // 9: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 10: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 11: mul r0.z, r0.z, cb0[3].x
    r0.z = ((r0.zzzz)*(source[3].xxxx)).z;
    // 12: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 13: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, cb0[2].y
    r0.z = ((r0.zzzz)*(source[2].yyyy)).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: mul r0.z, r0.y, r0.x
    r0.z = ((r0.yyyy)*(r0.xxxx)).z;
    // 20: mad r0.x, -r0.x, r0.y, r0.x
    r0.x = ((-(r0.xxxx))*(r0.yyyy)+(r0.xxxx)).x;
    // 21: mad r0.x, v4.z, r0.x, r0.z
    r0.x = ((v4.zzzz)*(r0.xxxx)+(r0.zzzz)).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 23: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 24: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 25: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 26: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 27: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 28: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 29: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1101(WARLORD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = g_WarlordSourceMaterialParameters[4u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].xxxx,g_WarlordSourceMaterialParameters[2u].yyyy,1u);
    source[4].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[4].y = g_WarlordSourceMaterialTime;
    source[4].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].z = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[6].w = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x));
    source[7].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[7].z = max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06);
    source[7].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06));
    source[8].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[8].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
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
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 28: mul r1.x, r0.y, cb0[3].x
    r1.x = ((r0.yyyy)*(source[3].xxxx)).x;
    // 29: mul r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)*(source[4].wwww)).y;
    // 30: mul r2.x, cb0[4].y, cb0[8].w
    r2.x = ((source[4].yyyy)*(source[8].wwww)).x;
    // 31: mov r1.z, l(-1.000000)
    r1.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 32: add r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 33: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 34: mul r0.w, v4.z, cb0[5].y
    r0.w = ((v4.zzzz)*(source[5].yyyy)).w;
    // 35: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 36: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 37: lt r0.w, r0.x, l(0.000000)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 38: mad r0.x, -r0.x, cb0[6].w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[6].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 40: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 41: mad r2.y, r0.z, cb0[3].y, v4.x
    r2.y = ((r0.zzzz)*(source[3].yyyy)+(v4.xxxx)).y;
    // 42: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 43: add r1.xy, r1.xzxx, r2.xyxx
    r1.xy = ((r1.xzxx)+(r2.xyxx)).xy;
    // 44: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 45: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s1, l(-1.000000)
    r0.w = (WarlordNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 46: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 47: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 48: mul r1.x, r1.x, cb0[8].x
    r1.x = ((r1.xxxx)*(source[8].xxxx)).x;
    // 49: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 50: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 51: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 52: mul_sat r0.x, r0.x, cb0[9].x
    r0.x = (saturate((r0.xxxx)*(source[9].xxxx))).x;
    // 53: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 54: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 56: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 57: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 58: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 59: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 60: mul r0.w, cb0[4].x, cb0[4].y
    r0.w = ((source[4].xxxx)*(source[4].yyyy)).w;
    // 61: mad r1.x, r0.w, cb0[4].z, r0.y
    r1.x = ((r0.wwww)*(source[4].zzzz)+(r0.yyyy)).x;
    // 62: mad r1.y, r0.w, cb0[5].z, r0.z
    r1.y = ((r0.wwww)*(source[5].zzzz)+(r0.zzzz)).y;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t1.wxyz, s0, l(-1.000000)
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 64: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 65: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 66: mad r0.yzw, cb0[5].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[5].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 67: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 68: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 69: mul r0.yzw, r0.yyzw, cb0[6].xxxx
    r0.yzw = ((r0.yyzw)*(source[6].xxxx)).yzw;
    // 70: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 71: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 72: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 73: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 74: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 75: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 76: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1102(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (1.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[2].z = max((1.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x),9.99999975e-06);
    source[2].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x),9.99999975e-06));
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
    // 6: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: min r0.x, r0.x, v4.x
    r0.x = (min(r0.xxxx,v4.xxxx)).x;
    // 9: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 10: mad o0.xyz, cb0[1].xyzx, v5.wwww, v5.xyzx
    output.xyz = ((source[1].xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1103(WARLORD_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[4] = g_WarlordSourceMaterialParameters[8u];
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[5u].xxxx,g_WarlordSourceMaterialParameters[5u].yyyy,1u);
    source[6] = WarlordNativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = WarlordNativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].xxxx,g_WarlordSourceMaterialParameters[2u].yyyy,1u);
    source[9] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].zzzz,g_WarlordSourceMaterialParameters[3u].wwww,1u);
    source[10].x = (-1.0*sin(((6.28000021*(g_WarlordSourceMaterialParameters[5u].zzzz).x)*0.25)));
    source[10].y = cos(((6.28000021*(g_WarlordSourceMaterialParameters[5u].zzzz).x)*0.25));
    source[10].z = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[11].w = g_WarlordSourceMaterialTime;
    source[12].x = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[12].z = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[13].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[14].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[15].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[15].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[15].w = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[16].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[16].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[16].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[17].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    r0.z = (WarlordNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r0.w = (WarlordNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.xy = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r1.xyz = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1104(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = g_WarlordSourceMaterialParameters[5u];
    source[4] = input.dynamicParameter;
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[6].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[6].y = g_WarlordSourceMaterialTime;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].z = (1.0*(g_WarlordSourceMaterialParameters[0u].wwww).x);
    source[8].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (-0.523599029*(g_WarlordSourceMaterialParameters[3u].xxxx).x);
    source[10].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[11].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[7].zwzz
    r0.xy = ((v4.xyxx)*(source[7].zwzz)).xy;
    // 2: mul r0.z, cb0[6].x, cb0[6].y
    r0.z = ((source[6].xxxx)*(source[6].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[7].y, r0.x
    r1.x = ((r0.zzzz)*(source[7].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[8].x, r0.y
    r1.y = ((r0.zzzz)*(source[8].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[8].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[8].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[6].w
    r0.w = ((r0.xxxx)*(source[6].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[6].z, r0.w
    r1.x = ((r0.zzzz)*(source[6].zzzz)+(r0.wwww)).x;
    // 9: mul r0.z, r0.z, cb0[8].w
    r0.z = ((r0.zzzz)*(source[8].wwww)).z;
    // 10: mad r1.y, cb0[7].x, r0.y, r0.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 14: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 15: mad r1.xyz, cb0[9].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[9].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 16: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 18: mul r1.xyz, r1.xyzx, cb0[9].yyyy
    r1.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 19: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 20: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 21: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 22: mad r1.xyz, r1.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 24: mad r0.z, cb0[6].y, cb0[9].w, cb0[10].x
    r0.z = ((source[6].yyyy)*(source[9].wwww)+(source[10].xxxx)).z;
    // 25: sincos r2.x, r3.x, r0.z
    r2.x = (sin(r0.zzzz)).x; r3.x = (cos(r0.zzzz)).x;
    // 26: mov r4.x, -r2.x
    r4.x = (-(r2.xxxx)).x;
    // 27: mov r4.y, r3.x
    r4.y = (r3.xxxx).y;
    // 28: mov r4.z, r2.x
    r4.z = (r2.xxxx).z;
    // 29: dp2 r0.z, r4.zyzz, r0.xyxx
    r0.z = (dot((r4.zyzz).xy,(r0.xyxx).xy).xxxx).z;
    // 30: dp2 r0.x, r4.yxyy, r0.xyxx
    r0.x = (dot((r4.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 31: mul r2.z, r0.z, cb0[5].y
    r2.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 32: add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 33: mad r2.x, r0.x, cb0[5].x, r0.y
    r2.x = ((r0.xxxx)*(source[5].xxxx)+(r0.yyyy)).x;
    // 34: add r0.xy, r2.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 36: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 39: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 40: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 41: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 42: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 43: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 44: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 45: source device depth mapped to centimetre view depth; reconstruction at 47.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 47-50: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 51: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 52: add r0.w, -cb0[11].y, l(1.000000)
    r0.w = ((-(source[11].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 54: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 55: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 56: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 59: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 60: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1105(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = g_WarlordSourceMaterialParameters[5u];
    source[4] = input.dynamicParameter;
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[6].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[6].y = g_WarlordSourceMaterialTime;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].z = (1.0*(g_WarlordSourceMaterialParameters[0u].wwww).x);
    source[8].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (-0.523599029*(g_WarlordSourceMaterialParameters[3u].xxxx).x);
    source[10].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[11].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[7].zwzz
    r0.xy = ((v4.xyxx)*(source[7].zwzz)).xy;
    // 2: mul r0.z, cb0[6].x, cb0[6].y
    r0.z = ((source[6].xxxx)*(source[6].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[7].y, r0.x
    r1.x = ((r0.zzzz)*(source[7].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[8].x, r0.y
    r1.y = ((r0.zzzz)*(source[8].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[8].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[8].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[6].w
    r0.w = ((r0.xxxx)*(source[6].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[6].z, r0.w
    r1.x = ((r0.zzzz)*(source[6].zzzz)+(r0.wwww)).x;
    // 9: mul r0.z, r0.z, cb0[8].w
    r0.z = ((r0.zzzz)*(source[8].wwww)).z;
    // 10: mad r1.y, cb0[7].x, r0.y, r0.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 14: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 15: mad r1.xyz, cb0[9].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[9].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 16: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 18: mul r1.xyz, r1.xyzx, cb0[9].yyyy
    r1.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 19: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 20: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 21: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 22: mad r1.xyz, r1.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 24: mad r0.z, cb0[6].y, cb0[9].w, cb0[10].x
    r0.z = ((source[6].yyyy)*(source[9].wwww)+(source[10].xxxx)).z;
    // 25: sincos r2.x, r3.x, r0.z
    r2.x = (sin(r0.zzzz)).x; r3.x = (cos(r0.zzzz)).x;
    // 26: mov r4.x, -r2.x
    r4.x = (-(r2.xxxx)).x;
    // 27: mov r4.y, r3.x
    r4.y = (r3.xxxx).y;
    // 28: mov r4.z, r2.x
    r4.z = (r2.xxxx).z;
    // 29: dp2 r0.z, r4.zyzz, r0.xyxx
    r0.z = (dot((r4.zyzz).xy,(r0.xyxx).xy).xxxx).z;
    // 30: dp2 r0.x, r4.yxyy, r0.xyxx
    r0.x = (dot((r4.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 31: mul r2.z, r0.z, cb0[5].y
    r2.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 32: add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 33: mad r2.x, r0.x, cb0[5].x, r0.y
    r2.x = ((r0.xxxx)*(source[5].xxxx)+(r0.yyyy)).x;
    // 34: add r0.xy, r2.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 36: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 39: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 40: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 41: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 42: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 43: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 44: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 45: source device depth mapped to centimetre view depth; reconstruction at 47.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 47-50: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 51: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 52: add r0.w, -cb0[11].y, l(1.000000)
    r0.w = ((-(source[11].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 54: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 55: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 56: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 59: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 60: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1106(WARLORD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].yyyy,g_WarlordSourceMaterialParameters[3u].zzzz,1u);
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].wwww,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[4] = WarlordNativeAppend((g_WarlordSourceMaterialParameters[0u].yyyy*g_WarlordSourceMaterialTime.xxxx),(g_WarlordSourceMaterialParameters[0u].zzzz*g_WarlordSourceMaterialTime.xxxx),1u);
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].wwww,g_WarlordSourceMaterialParameters[2u].xxxx,1u);
    source[6] = WarlordNativeAppend((g_WarlordSourceMaterialParameters[1u].yyyy*g_WarlordSourceMaterialTime.xxxx),(g_WarlordSourceMaterialParameters[1u].zzzz*g_WarlordSourceMaterialTime.xxxx),1u);
    source[7] = WarlordNativeAppend((g_WarlordSourceMaterialParameters[2u].yyyy*g_WarlordSourceMaterialTime.xxxx),(g_WarlordSourceMaterialParameters[2u].zzzz*g_WarlordSourceMaterialTime.xxxx),1u);
    source[8].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].y = ((g_WarlordSourceMaterialParameters[1u].zzzz).x*g_WarlordSourceMaterialTime);
    source[8].z = ((g_WarlordSourceMaterialParameters[1u].yyyy).x*g_WarlordSourceMaterialTime);
    source[8].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
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
    float4 r0=0.f;
    // 1: mad r0.xy, v2.xyxx, cb0[3].xyxx, v4.yyyy
    r0.xy = ((v2.xyxx)*(source[3].xyxx)+(v4.yyyy)).xy;
    // 2: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, v2.xxyx, cb0[5].xxyx, v4.yyyy
    r0.yz = ((v2.xxyx)*(source[5].xxyx)+(v4.yyyy)).yz;
    // 5: add r0.yz, r0.yyzy, cb0[6].xxyx
    r0.yz = ((r0.yyzy)+(source[6].xxyx)).yz;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.xyzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 7: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 8: mad r0.yz, cb0[8].wwww, r0.xxxx, v2.xxyx
    r0.yz = ((source[8].wwww)*(r0.xxxx)+(v2.xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t3.yzwx, s4, l(0.000000)
    r0.w = (WarlordNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s3, l(0.000000)
    r0.y = (WarlordNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 11: add r0.z, -r0.y, r0.w
    r0.z = ((-(r0.yyyy))+(r0.wwww)).z;
    // 12: mad r0.y, v4.x, r0.z, r0.y
    r0.y = ((v4.xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 13: max r0.y, |r0.y|, l(0.000001)
    r0.y = (max(abs(r0.yyyy),float4(0.000001,0.000001,0.000001,0.000001))).y;
    // 14: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 16: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 17: mul r0.y, r0.y, cb0[9].w
    r0.y = ((r0.yyyy)*(source[9].wwww)).y;
    // 18: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 19: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 20: mad r0.zw, v2.xxxy, cb0[2].xxxy, v4.yyyy
    r0.zw = ((v2.xxxy)*(source[2].xxxy)+(v4.yyyy)).zw;
    // 21: mad r0.xz, cb0[8].wwww, r0.xxxx, r0.zzwz
    r0.xz = ((source[8].wwww)*(r0.xxxx)+(r0.zzwz)).xz;
    // 22: add r0.xz, r0.xxzx, cb0[7].xxyx
    r0.xz = ((r0.xxzx)+(source[7].xxyx)).xz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t4.xwyz, s2, l(0.000000)
    r0.xzw = (WarlordNativeSample2((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 25: log r0.xzw, r0.xxzw
    r0.xzw = (log2(r0.xxzw)).xzw;
    // 26: mul r0.xzw, r0.xxzw, cb0[9].xxxx
    r0.xzw = ((r0.xxzw)*(source[9].xxxx)).xzw;
    // 27: exp r0.xzw, r0.xxzw
    r0.xzw = (exp2(r0.xxzw)).xzw;
    // 28: mul r0.xzw, r0.xxzw, cb0[9].yyyy
    r0.xzw = ((r0.xxzw)*(source[9].yyyy)).xzw;
    // 29: mad r0.xzw, r0.xxzw, v3.xxyz, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)+(source[1].xxyz)).xzw;
    // 30: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 31: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 32: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1107(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].z = g_WarlordSourceMaterialTime;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].wwww).x);
    source[4].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].wwww).x));
    source[4].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
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
    // 27: mad r0.y, r0.y, l(0.159155), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 28: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 29: mad r1.x, cb0[2].z, cb0[2].y, r0.y
    r1.x = ((source[2].zzzz)*(source[2].yyyy)+(r0.yyyy)).x;
    // 30: mul r0.y, r0.x, cb0[3].x
    r0.y = ((r0.xxxx)*(source[3].xxxx)).y;
    // 31: mad r0.x, -r0.x, cb0[4].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[4].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 32: mul r0.y, r0.y, l(2.000000)
    r0.y = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))).y;
    // 33: mad r1.y, cb0[2].z, cb0[3].y, r0.y
    r1.y = ((source[2].zzzz)*(source[3].yyyy)+(r0.yyyy)).y;
    // 34: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s0, cb0[2].x
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (source[2].xxxx).x, true).wxyz).yzw;
    // 35: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 36: add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 38: mul_sat r0.y, r0.y, l(0.330000)
    r0.y = (saturate((r0.yyyy)*(float4(0.330000,0.330000,0.330000,0.330000)))).y;
    // 39: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 40: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 41: mul r0.z, r0.z, cb0[3].w
    r0.z = ((r0.zzzz)*(source[3].wwww)).z;
    // 42: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 43: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 44: mov_sat r0.z, v4.z
    r0.z = (saturate(v4.zzzz)).z;
    // 45: mul_sat r0.w, v4.y, cb0[4].w
    r0.w = (saturate((v4.yyyy)*(source[4].wwww))).w;
    // 46: add r0.zw, -r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.zzzw))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 47: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: max r0.w, r0.w, l(0.000010)
    r0.w = (max(r0.wwww,float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 49: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 50: mul_sat r0.x, r0.w, r0.x
    r0.x = (saturate((r0.wwww)*(r0.xxxx))).x;
    // 51: mad r0.x, r0.x, l(-2.000000), l(1.000000)
    r0.x = ((r0.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 54: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: mul r1.x, v4.x, cb0[5].x
    r1.x = ((v4.xxxx)*(source[5].xxxx)).x;
    // 56: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 57: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 58: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 59: mad_sat r0.x, r0.y, r0.x, -r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz)))).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 62: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 63: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 64: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 65: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1108(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1109(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].y = g_WarlordSourceMaterialTime;
    source[2].z = (g_WarlordSourceMaterialTime*4.0);
    source[2].w = ((g_WarlordSourceMaterialTime*4.0)*6.28318548);
    source[3].x = sin(((g_WarlordSourceMaterialTime*4.0)*6.28318548));
    source[3].y = (1.0+sin(((g_WarlordSourceMaterialTime*4.0)*6.28318548)));
    source[3].z = ((1.0+sin(((g_WarlordSourceMaterialTime*4.0)*6.28318548)))*0.5);
    source[3].w = ((g_WarlordSourceMaterialTime*4.0)*4.58626699);
    source[4].x = sin(((g_WarlordSourceMaterialTime*4.0)*4.58626699));
    source[4].y = (1.0+sin(((g_WarlordSourceMaterialTime*4.0)*4.58626699)));
    source[4].z = ((1.0+sin(((g_WarlordSourceMaterialTime*4.0)*4.58626699)))*0.5);
    source[4].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1110(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
    source[2].x = g_WarlordSourceMaterialTime;
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
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: max r0.y, r0.x, l(0.000000)
    r0.y = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 6: mul_sat r0.x, r0.x, l(2.500000)
    r0.x = (saturate((r0.xxxx)*(float4(2.500000,2.500000,2.500000,2.500000)))).x;
    // 7: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 8: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 9: mad r0.x, cb0[2].x, l(-0.700000), r0.y
    r0.x = ((source[2].xxxx)*(float4(-0.700000,-0.700000,-0.700000,-0.700000))+(r0.yyyy)).x;
    // 10: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 12: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xxxx, t0.xwyz, s0, l(0.000000)
    r0.xzw = (WarlordNativeSample0((r0.xxxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 14: mul r0.xzw, r0.xxzw, r0.xxzw
    r0.xzw = ((r0.xxzw)*(r0.xxzw)).xzw;
    // 15: mad r0.xyz, r0.yyyy, r0.xzwx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 16: mul r0.xyz, r0.xyzx, v3.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)).xyz;
    // 17: mad r0.xyz, r0.xyzx, l(1.500000, 1.500000, 1.500000, 0.000000), cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(float4(1.500000,1.500000,1.500000,0.000000))+(source[1].xyzx)).xyz;
    // 18: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1111(WARLORD_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[8u];
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = WarlordNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = WarlordNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[5].y = g_WarlordSourceMaterialTime;
    source[5].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[9].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[9].w = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].wwww).x);
    source[10].x = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].wwww).x));
    source[10].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (1.0-(g_WarlordSourceMaterialParameters[2u].xxxx).x);
    source[10].w = max((1.0-(g_WarlordSourceMaterialParameters[2u].xxxx).x),9.99999975e-06);
    source[11].x = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[2u].xxxx).x),9.99999975e-06));
    source[11].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[11].z = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[11].w = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x));
    source[12].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[12].y = (1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[12].z = max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06);
    source[12].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06));
    source[13].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[13].z = (-0.523599029*(g_WarlordSourceMaterialParameters[5u].xxxx).x);
    source[13].w = ((-0.523599029*(g_WarlordSourceMaterialParameters[5u].xxxx).x)*1.0);
    source[14].x = sin(((-0.523599029*(g_WarlordSourceMaterialParameters[5u].xxxx).x)*1.0));
    source[14].y = (-1.0*sin(((-0.523599029*(g_WarlordSourceMaterialParameters[5u].xxxx).x)*1.0)));
    source[14].z = cos(((-0.523599029*(g_WarlordSourceMaterialParameters[5u].xxxx).x)*1.0));
    source[14].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[15].y = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
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
    // 26: mul r0.z, v4.y, cb0[7].z
    r0.z = ((v4.yyyy)*(source[7].zzzz)).z;
    // 27: mul r0.w, v2.x, cb0[6].w
    r0.w = ((v2.xxxx)*(source[6].wwww)).w;
    // 28: mul r1.z, cb0[5].x, cb0[5].y
    r1.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 29: mad r2.x, r1.z, cb0[6].z, r0.w
    r2.x = ((r1.zzzz)*(source[6].zzzz)+(r0.wwww)).x;
    // 30: mul r2.zw, r1.zzzz, cb0[7].yyyw
    r2.zw = ((r1.zzzz)*(source[7].yyyw)).zw;
    // 31: mad r2.y, cb0[7].x, v2.y, r2.z
    r2.y = ((source[7].xxxx)*(v2.yyyy)+(r2.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 33: mul r0.w, v4.z, cb0[6].y
    r0.w = ((v4.zzzz)*(source[6].yyyy)).w;
    // 34: dp2 r1.w, r0.xyxx, r0.xyxx
    r1.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 35: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 36: add r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)+(r1.wwww)).z;
    // 37: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 38: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 39: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 40: lt r2.z, r1.w, l(0.000000)
    r2.z = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 41: movc r1.y, r2.z, l(0), r0.w
    r1.y = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 42: mad r0.zw, r0.zzzz, r2.xxxy, r1.xxxy
    r0.zw = ((r0.zzzz)*(r2.xxxy)+(r1.xxxy)).zw;
    // 43: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 44: mad r1.x, r1.z, cb0[5].z, r1.x
    r1.x = ((r1.zzzz)*(source[5].zzzz)+(r1.xxxx)).x;
    // 45: mad r1.y, cb0[6].x, r0.w, r2.w
    r1.y = ((source[6].xxxx)*(r0.wwww)+(r2.wwww)).y;
    // 46: mul r0.zw, r0.zzzw, cb0[8].yyyz
    r0.zw = ((r0.zzzw)*(source[8].yyyz)).zw;
    // 47: mad r0.zw, r1.zzzz, cb0[8].xxxw, r0.zzzw
    r0.zw = ((r1.zzzz)*(source[8].xxxw)+(r0.zzzw)).zw;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s2, l(-1.000000)
    r2.xyz = (WarlordNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s0, l(-1.000000)
    r1.xyz = (WarlordNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 50: mul r3.xyz, r2.xyzx, r1.xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 51: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 52: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 53: mad r1.xyz, cb0[9].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[9].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 54: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 56: mul r1.xyz, r1.xyzx, cb0[9].yyyy
    r1.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 57: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 58: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 59: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 60: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 61: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 62: dp2 r2.x, cb0[3].xyxx, r0.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 63: dp2 r2.y, cb0[4].xyxx, r0.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 64: mad r0.xy, cb0[14].wwww, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[14].wwww)*(r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 66: mad r0.y, -r1.w, cb0[10].x, l(1.000000)
    r0.y = ((-(r1.wwww))*(source[10].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: mad r0.z, -r1.w, cb0[11].w, l(1.000000)
    r0.z = ((-(r1.wwww))*(source[11].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 68: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 69: mul_sat r0.y, r0.y, cb0[11].x
    r0.y = (saturate((r0.yyyy)*(source[11].xxxx))).y;
    // 70: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 71: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 72: lt r0.z, r0.y, l(0.000001)
    r0.z = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 73: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 74: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 75: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 76: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 77: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 78: mul_sat r0.x, r0.x, cb0[15].x
    r0.x = (saturate((r0.xxxx)*(source[15].xxxx))).x;
    // 79: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 81: mul r0.x, r0.x, cb0[15].y
    r0.x = ((r0.xxxx)*(source[15].yyyy)).x;
    // 82: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 83: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 84: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 85: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 86: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1112(WARLORD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = cos(((g_WarlordSourceMaterialParameters[1u].yyyy).x*0.25));
    source[4].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
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
    r0.zw = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.y = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r1.xyz = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1114(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[3].z = g_WarlordSourceMaterialTime;
    source[3].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[4].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 29: mad r1.x, r0.y, l(0.159155), l(0.500000)
    r1.x = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 30: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 31: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 32: mul r0.z, r0.y, v4.z
    r0.z = ((r0.yyyy)*(v4.zzzz)).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: movc r1.y, r0.x, l(0), r0.z
    r1.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 35: mul r0.zw, r1.xxxy, cb0[6].zzzw
    r0.zw = ((r1.xxxy)*(source[6].zzzw)).zw;
    // 36: mul r1.x, r1.x, cb0[3].w
    r1.x = ((r1.xxxx)*(source[3].wwww)).x;
    // 37: mad r2.x, cb0[3].z, cb0[3].y, r1.x
    r2.x = ((source[3].zzzz)*(source[3].yyyy)+(r1.xxxx)).x;
    // 38: mad r3.x, cb0[3].z, cb0[6].y, r0.z
    r3.x = ((source[3].zzzz)*(source[6].yyyy)+(r0.zzzz)).x;
    // 39: mad r3.y, cb0[3].z, cb0[7].x, r0.w
    r3.y = ((source[3].zzzz)*(source[7].xxxx)+(r0.wwww)).y;
    // 40: mul r0.z, v2.y, cb0[5].x
    r0.z = ((v2.yyyy)*(source[5].xxxx)).z;
    // 41: mad r4.y, cb0[3].z, cb0[5].y, r0.z
    r4.y = ((source[3].zzzz)*(source[5].yyyy)+(r0.zzzz)).y;
    // 42: mul r0.zw, cb0[3].zzzz, cb0[4].zzzy
    r0.zw = ((source[3].zzzz)*(source[4].zzzy)).zw;
    // 43: mad r4.x, cb0[4].w, v2.x, r0.z
    r4.x = ((source[4].wwww)*(v2.xxxx)+(r0.zzzz)).x;
    // 44: mad r2.y, cb0[4].x, r1.y, r0.w
    r2.y = ((source[4].xxxx)*(r1.yyyy)+(r0.wwww)).y;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r4.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample1((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 46: mul r0.w, v4.x, cb0[5].z
    r0.w = ((v4.xxxx)*(source[5].zzzz)).w;
    // 47: mad r1.xy, r0.zzzz, r0.wwww, r3.xyxx
    r1.xy = ((r0.zzzz)*(r0.wwww)+(r3.xyxx)).xy;
    // 48: mad r0.zw, r0.zzzz, r0.wwww, r2.xxxy
    r0.zw = ((r0.zzzz)*(r0.wwww)+(r2.xxxy)).zw;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s0, cb0[3].x
    r2.xyz = (WarlordNativeSample0((r0.zwzz).xy, (source[3].xxxx).x, true).xyzw).xyz;
    // 50: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, cb0[3].x
    r0.z = (WarlordNativeSample2((r1.xyxx).xy, (source[3].xxxx).x, true).yzxw).z;
    // 51: mul_sat r0.z, r0.z, cb0[7].y
    r0.z = (saturate((r0.zzzz)*(source[7].yyyy))).z;
    // 52: mov_sat r0.w, v4.y
    r0.w = (saturate(v4.yyyy)).w;
    // 53: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 55: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (WarlordNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 57: mul_sat r0.z, r0.w, r0.z
    r0.z = (saturate((r0.wwww)*(r0.zzzz))).z;
    // 58: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 59: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 60: mul r0.w, r0.w, cb0[7].z
    r0.w = ((r0.wwww)*(source[7].zzzz)).w;
    // 61: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 62: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 63: mul r0.w, v4.w, cb0[7].w
    r0.w = ((v4.wwww)*(source[7].wwww)).w;
    // 64: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 65: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 66: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 67: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 68: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 69: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 70: dp3 r0.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 71: add r0.yzw, -r2.xxyz, r0.yyyy
    r0.yzw = ((-(r2.xxyz))+(r0.yyyy)).yzw;
    // 72: mad r0.yzw, cb0[5].wwww, r0.yyzw, r2.xxyz
    r0.yzw = ((source[5].wwww)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 73: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 74: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 75: mul r0.yzw, r0.yyzw, cb0[6].xxxx
    r0.yzw = ((r0.yyzw)*(source[6].xxxx)).yzw;
    // 76: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 77: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 78: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1115(WARLORD_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[7u];
    source[2] = g_WarlordSourceMaterialParameters[6u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[4u].xxxx,g_WarlordSourceMaterialParameters[4u].yyyy,1u);
    source[4].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[4].y = g_WarlordSourceMaterialTime;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[5].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[2u].xxxx).x);
    source[7].w = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[2u].xxxx).x));
    source[8].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (1.0-(g_WarlordSourceMaterialParameters[2u].yyyy).x);
    source[8].z = max((1.0-(g_WarlordSourceMaterialParameters[2u].yyyy).x),9.99999975e-06);
    source[8].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[2u].yyyy).x),9.99999975e-06));
    source[9].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[9].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[9].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x));
    source[9].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[10].x = (1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x);
    source[10].y = max((1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x),9.99999975e-06);
    source[10].z = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x),9.99999975e-06));
    source[10].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[11].w = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[12].x = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[12].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 28: mul r0.yz, v2.xxyx, cb0[5].zzwz
    r0.yz = ((v2.xxyx)*(source[5].zzwz)).yz;
    // 29: mad r2.x, cb0[4].y, cb0[5].y, r0.y
    r2.x = ((source[4].yyyy)*(source[5].yyyy)+(r0.yyyy)).x;
    // 30: mad r2.y, cb0[4].y, cb0[6].x, r0.z
    r2.y = ((source[4].yyyy)*(source[6].xxxx)+(r0.zzzz)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r2.xyxx, t0.zxyw, s2, l(0.000000)
    r0.yz = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 32: mul r0.w, v4.y, cb0[6].y
    r0.w = ((v4.yyyy)*(source[6].yyyy)).w;
    // 33: mul r1.z, v4.z, cb0[5].x
    r1.z = ((v4.zzzz)*(source[5].xxxx)).z;
    // 34: add r1.w, r0.x, r0.x
    r1.w = ((r0.xxxx)+(r0.xxxx)).w;
    // 35: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 36: mul r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)*(r1.zzzz)).z;
    // 37: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 38: lt r1.w, r0.x, l(0.000000)
    r1.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 39: movc r1.y, r1.w, l(0), r1.z
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 40: mad r0.yz, r0.wwww, r0.yyzy, r1.xxyx
    r0.yz = ((r0.wwww)*(r0.yyzy)+(r1.xxyx)).yz;
    // 41: mul r1.x, r0.y, cb0[3].x
    r1.x = ((r0.yyyy)*(source[3].xxxx)).x;
    // 42: mad r2.y, r0.z, cb0[3].y, v4.x
    r2.y = ((r0.zzzz)*(source[3].yyyy)+(v4.xxxx)).y;
    // 43: mul r0.yz, r0.yyzy, cb0[4].zzwz
    r0.yz = ((r0.yyzy)*(source[4].zzwz)).yz;
    // 44: mul r2.x, cb0[4].y, cb0[11].z
    r2.x = ((source[4].yyyy)*(source[11].zzzz)).x;
    // 45: mov r1.z, l(-1.000000)
    r1.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 46: add r1.xy, r1.xzxx, r2.xyxx
    r1.xy = ((r1.xzxx)+(r2.xyxx)).xy;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.xywz, s3, l(-1.000000)
    r0.w = (WarlordNativeSample2((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xywz).w;
    // 48: mad r1.x, -r0.x, cb0[7].w, l(1.000000)
    r1.x = ((-(r0.xxxx))*(source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 49: mad r0.x, -r0.x, cb0[9].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[9].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 51: mul_sat r1.x, r1.x, cb0[8].w
    r1.x = (saturate((r1.xxxx)*(source[8].wwww))).x;
    // 52: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 54: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 56: mul r0.x, r0.x, cb0[10].w
    r0.x = ((r0.xxxx)*(source[10].wwww)).x;
    // 57: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 58: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 59: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 60: mul_sat r0.x, r0.x, cb0[11].w
    r0.x = (saturate((r0.xxxx)*(source[11].wwww))).x;
    // 61: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 62: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 63: mul r0.w, r0.w, cb0[12].x
    r0.w = ((r0.wwww)*(source[12].xxxx)).w;
    // 64: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 65: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 66: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 67: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 68: source device depth mapped to centimetre view depth; reconstruction at 70.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 70-73: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 74: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 75: add r1.x, -cb0[12].y, l(1.000000)
    r1.x = ((-(source[12].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 76: max r1.x, -r1.x, l(0.001000)
    r1.x = (max(-(r1.xxxx),float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 77: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 78: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 79: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 80: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 81: mad r0.x, cb0[4].y, cb0[4].x, r0.y
    r0.x = ((source[4].yyyy)*(source[4].xxxx)+(r0.yyyy)).x;
    // 82: mad r0.y, cb0[4].y, cb0[6].z, r0.z
    r0.y = ((source[4].yyyy)*(source[6].zzzz)+(r0.zzzz)).y;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(-1.000000)
    r0.xyz = (WarlordNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 84: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 86: mad r0.xyz, cb0[6].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[6].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 87: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 88: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 89: mul r0.xyz, r0.xyzx, cb0[7].xxxx
    r0.xyz = ((r0.xyzx)*(source[7].xxxx)).xyz;
    // 90: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 91: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 92: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 93: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 94: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1116(WARLORD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = g_WarlordSourceMaterialParameters[5u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[5].y = g_WarlordSourceMaterialTime;
    source[5].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[9].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[9].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
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
    // 10: add r0.y, -cb0[9].z, l(1.000000)
    r0.y = ((-(source[9].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: mul r0.zw, v4.xxxy, cb0[7].yyyz
    r0.zw = ((v4.xxxy)*(source[7].yyyz)).zw;
    // 15: mul r1.x, cb0[5].x, cb0[5].y
    r1.x = ((source[5].xxxx)*(source[5].yyyy)).x;
    // 16: mad r0.zw, r1.xxxx, cb0[7].xxxw, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[7].xxxw)+(r0.zzzw)).zw;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 18: mul r1.yz, v4.xxyx, cb0[8].yyzy
    r1.yz = ((v4.xxyx)*(source[8].yyzy)).yz;
    // 19: mad r1.yz, r1.xxxx, cb0[8].xxwx, r1.yyzy
    r1.yz = ((r1.xxxx)*(source[8].xxwx)+(r1.yyzy)).yz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.yzyy, t1.yzwx, s3, l(0.000000)
    r0.w = (WarlordNativeSample2((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 21: mad r0.y, r0.z, r0.w, -r0.y
    r0.y = ((r0.zzzz)*(r0.wwww)+(-(r0.yyyy))).y;
    // 22: mul_sat r0.y, r0.y, cb0[9].x
    r0.y = (saturate((r0.yyyy)*(source[9].xxxx))).y;
    // 23: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 24: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: mul r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)*(source[9].yyyy)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 28: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 29: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 30: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 31: mul r0.y, v4.x, cb0[5].w
    r0.y = ((v4.xxxx)*(source[5].wwww)).y;
    // 32: mad r2.x, r1.x, cb0[5].z, r0.y
    r2.x = ((r1.xxxx)*(source[5].zzzz)+(r0.yyyy)).x;
    // 33: mul r0.y, v4.y, cb0[6].x
    r0.y = ((v4.yyyy)*(source[6].xxxx)).y;
    // 34: mad r2.y, r1.x, cb0[6].y, r0.y
    r2.y = ((r1.xxxx)*(source[6].yyyy)+(r0.yyyy)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t3.wxyz, s1, l(0.000000)
    r0.yzw = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 36: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 37: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 38: mad r0.yzw, cb0[6].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[6].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 39: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 40: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 41: mul r0.yzw, r0.yyzw, cb0[6].wwww
    r0.yzw = ((r0.yyzw)*(source[6].wwww)).yzw;
    // 42: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 43: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 44: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 45: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 46: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 47: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 48: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1117(WARLORD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[9u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].zzzz,g_WarlordSourceMaterialParameters[1u].wwww,1u);
    source[4] = g_WarlordSourceMaterialParameters[7u];
    source[5] = input.dynamicParameter;
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].zzzz,g_WarlordSourceMaterialParameters[3u].wwww,1u);
    source[7] = g_WarlordSourceMaterialParameters[6u];
    source[8].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[8].w = g_WarlordSourceMaterialTime;
    source[9].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[9].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[9].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[10].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[10].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].z = (-0.523599029*(g_WarlordSourceMaterialParameters[4u].xxxx).x);
    source[11].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[12].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[12].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[13].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[13].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[13].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[13].w = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[14].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
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
    // 10: add r0.y, -cb0[13].z, l(1.000000)
    r0.y = ((-(source[13].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.yzw, r0.yyyy, v6.xxyz
    r0.yzw = ((r0.yyyy)*(v6.xxyz)).yzw;
    // 16: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 17: mul r1.x, r1.x, cb0[13].w
    r1.x = ((r1.xxxx)*(source[13].wwww)).x;
    // 18: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 19: mul_sat r1.x, r1.x, cb0[14].x
    r1.x = (saturate((r1.xxxx)*(source[14].xxxx))).x;
    // 20: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 21: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 22: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: add r0.w, cb0[5].y, l(-1.000000)
    r0.w = ((source[5].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 24: mad r1.x, cb0[8].w, cb0[11].y, cb0[11].z
    r1.x = ((source[8].wwww)*(source[11].yyyy)+(source[11].zzzz)).x;
    // 25: sincos r1.x, r2.x, r1.x
    r1.x = (sin(r1.xxxx)).x; r2.x = (cos(r1.xxxx)).x;
    // 26: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 27: add r1.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 28: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 29: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 30: dp2 r1.x, r3.zyzz, r1.yzyy
    r1.x = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).x;
    // 31: dp2 r1.y, r3.yxyy, r1.yzyy
    r1.y = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).y;
    // 32: mad r2.x, r1.y, cb0[6].x, r0.w
    r2.x = ((r1.yyyy)*(source[6].xxxx)+(r0.wwww)).x;
    // 33: mul r2.z, r1.x, cb0[6].y
    r2.z = ((r1.xxxx)*(source[6].yyyy)).z;
    // 34: add r1.xy, r2.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: mul r1.xy, v4.xyxx, cb0[10].yzyy
    r1.xy = ((v4.xyxx)*(source[10].yzyy)).xy;
    // 37: mul r1.z, cb0[8].z, cb0[8].w
    r1.z = ((source[8].zzzz)*(source[8].wwww)).z;
    // 38: mad r1.xy, r1.zzzz, cb0[10].xwxx, r1.xyxx
    r1.xy = ((r1.zzzz)*(source[10].xwxx)+(r1.xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 40: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 41: add r1.x, -cb0[5].x, l(1.000000)
    r1.x = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 42: mul r1.yw, v4.xxxy, cb0[9].yyyz
    r1.yw = ((v4.xxxy)*(source[9].yyyz)).yw;
    // 43: mad r1.yz, r1.zzzz, cb0[9].xxwx, r1.yywy
    r1.yz = ((r1.zzzz)*(source[9].xxwx)+(r1.yywy)).yz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t0.yxzw, s1, l(0.000000)
    r1.y = (WarlordNativeSample0((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 45: mad r1.x, r1.y, r0.w, -r1.x
    r1.x = ((r1.yyyy)*(r0.wwww)+(-(r1.xxxx))).x;
    // 46: mul_sat r1.x, r1.x, cb0[13].x
    r1.x = (saturate((r1.xxxx)*(source[13].xxxx))).x;
    // 47: log r1.z, r1.x
    r1.z = (log2(r1.xxxx)).z;
    // 48: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 49: mul r1.z, r1.z, cb0[13].y
    r1.z = ((r1.zzzz)*(source[13].yyyy)).z;
    // 50: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 51: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 52: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 53: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 54: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 55: mul r1.xzw, cb0[4].xxyz, cb0[4].wwww
    r1.xzw = ((source[4].xxyz)*(source[4].wwww)).xzw;
    // 56: mad r2.xy, r1.yyyy, r0.wwww, r1.xzxx
    r2.xy = ((r1.yyyy)*(r0.wwww)+(r1.xzxx)).xy;
    // 57: mul r2.xy, r2.xyxx, cb0[12].yyyy
    r2.xy = ((r2.xyxx)*(source[12].yyyy)).xy;
    // 58: mad r0.yz, r0.yyzy, cb0[3].xxyx, r2.xxyx
    r0.yz = ((r0.yyzy)*(source[3].xxyx)+(r2.xxyx)).yz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s4, l(0.000000)
    r0.yzw = (WarlordNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 60: dp3 r1.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 61: add r2.xyz, -r0.yzwy, r1.yyyy
    r2.xyz = ((-(r0.yzwy))+(r1.yyyy)).xyz;
    // 62: mad r0.yzw, cb0[12].zzzz, r2.xxyz, r0.yyzw
    r0.yzw = ((source[12].zzzz)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 63: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 64: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 65: mul r0.yzw, r0.yyzw, cb0[12].wwww
    r0.yzw = ((r0.yyzw)*(source[12].wwww)).yzw;
    // 66: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 67: mul r2.xyz, cb0[7].xyzx, cb0[7].wwww
    r2.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 68: mad r0.yzw, r0.yyzw, r2.xxyz, r1.xxzw
    r0.yzw = ((r0.yyzw)*(r2.xxyz)+(r1.xxzw)).yzw;
    // 69: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 70: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 71: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 72: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1118(WARLORD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[5].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (1.0*(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[5].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[7].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[8].x, l(1.000000)
    r0.y = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: mul r0.yz, v2.xxyx, cb0[4].zzwz
    r0.yz = ((v2.xxyx)*(source[4].zzwz)).yz;
    // 14: mul r0.w, cb0[3].x, cb0[3].y
    r0.w = ((source[3].xxxx)*(source[3].yyyy)).w;
    // 15: mad r1.x, r0.w, cb0[4].y, r0.y
    r1.x = ((r0.wwww)*(source[4].yyyy)+(r0.yyyy)).x;
    // 16: mad r1.y, r0.w, cb0[5].x, r0.z
    r1.y = ((r0.wwww)*(source[5].xxxx)+(r0.zzzz)).y;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r1.xyxx, t0.zxyw, s1, l(0.000000)
    r0.yz = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 18: mad r0.yz, cb0[5].zzzz, r0.yyzy, v2.xxyx
    r0.yz = ((source[5].zzzz)*(r0.yyzy)+(v2.xxyx)).yz;
    // 19: mul r1.x, r0.y, cb0[6].w
    r1.x = ((r0.yyyy)*(source[6].wwww)).x;
    // 20: mad r1.x, r0.w, cb0[6].z, r1.x
    r1.x = ((r0.wwww)*(source[6].zzzz)+(r1.xxxx)).x;
    // 21: mul r1.z, r0.z, cb0[7].x
    r1.z = ((r0.zzzz)*(source[7].xxxx)).z;
    // 22: mad r1.y, r0.w, cb0[7].y, r1.z
    r1.y = ((r0.wwww)*(source[7].yyyy)+(r1.zzzz)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.wxyz, s3, l(0.000000)
    r1.x = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 24: add r1.y, -v4.x, l(1.000000)
    r1.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 25: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 26: mul_sat r1.x, r1.x, cb0[7].z
    r1.x = (saturate((r1.xxxx)*(source[7].zzzz))).x;
    // 27: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 28: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r1.y, r1.y, cb0[7].w
    r1.y = ((r1.yyyy)*(source[7].wwww)).y;
    // 30: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 31: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 32: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 33: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 34: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 35: mul r0.x, r0.y, cb0[3].w
    r0.x = ((r0.yyyy)*(source[3].wwww)).x;
    // 36: mad r0.x, r0.w, cb0[3].z, r0.x
    r0.x = ((r0.wwww)*(source[3].zzzz)+(r0.xxxx)).x;
    // 37: mul r0.w, r0.w, cb0[5].w
    r0.w = ((r0.wwww)*(source[5].wwww)).w;
    // 38: mad r0.y, cb0[4].x, r0.z, r0.w
    r0.y = ((source[4].xxxx)*(r0.zzzz)+(r0.wwww)).y;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r0.xyz, cb0[6].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[6].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 43: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 45: mul r0.xyz, r0.xyzx, cb0[6].yyyy
    r0.xyz = ((r0.xyzx)*(source[6].yyyy)).xyz;
    // 46: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 47: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 48: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 49: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 50: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1119(WARLORD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2] = WarlordNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].zzzz,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[6].x = (-1.0*sin(((0.523599029*(g_WarlordSourceMaterialParameters[0u].xxxx).x)*1.0)));
    source[6].y = cos(((0.523599029*(g_WarlordSourceMaterialParameters[0u].xxxx).x)*1.0));
    source[6].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].x = ((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].yyyy).x)*0.100000001);
    source[7].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[7].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
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
    // 1: mul r0.x, v4.x, cb0[7].z
    r0.x = ((v4.xxxx)*(source[7].zzzz)).x;
    // 2: add r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 3: dp2 r0.y, cb0[3].xyxx, r1.zwzz
    r0.y = (dot((source[3].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 4: dp4 r0.z, cb0[2].xyxy, r1.xyzw
    r0.z = (dot((source[2].xyxy).xyzw,(r1.xyzw).xyzw).xxxx).z;
    // 5: add r1.y, r0.y, l(0.500000)
    r1.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 6: add r0.y, r1.y, v4.y
    r0.y = ((r1.yyyy)+(v4.yyyy)).y;
    // 7: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
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
    // 18: mad_sat r1.x, r0.x, cb0[7].w, l(0.500000)
    r1.x = (saturate((r0.xxxx)*(source[7].wwww)+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 19: add r0.xz, r1.xxyx, cb0[4].xxyx
    r0.xz = ((r1.xxyx)+(source[4].xxyx)).xz;
    // 20: mul_sat r1.x, r1.y, cb0[8].x
    r1.x = (saturate((r1.yyyy)*(source[8].xxxx))).x;
    // 21: mul r0.xz, r0.xxzx, cb0[5].xxyx
    r0.xz = ((r0.xxzx)*(source[5].xxyx)).xz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t0.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 23: add r0.z, -v4.z, l(1.000000)
    r0.z = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 24: add_sat r0.x, -r0.z, r0.x
    r0.x = (saturate((-(r0.zzzz))+(r0.xxxx))).x;
    // 25: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 26: log r0.z, r1.x
    r0.z = (log2(r1.xxxx)).z;
    // 27: lt r0.w, r1.x, l(0.000001)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: mul r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)*(source[8].yyyy)).z;
    // 29: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 30: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 31: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 32: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 33: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 34: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 35: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 36: mul r0.z, r0.z, cb0[6].z
    r0.z = ((r0.zzzz)*(source[6].zzzz)).z;
    // 37: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 38: mad r0.z, r0.z, cb0[6].w, l(1.000000)
    r0.z = ((r0.zzzz)*(source[6].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 39: movc r0.y, r0.y, l(1.000000), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.zzzz)).y;
    // 40: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 41: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 42: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 43: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1120(WARLORD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = g_WarlordSourceMaterialParameters[3u];
    source[3] = g_WarlordSourceMaterialParameters[4u];
    source[4] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = WarlordNativeAppend(cos((((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[9] = WarlordNativeAppend(sin((((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_WarlordSourceMaterialParameters[2u].yyyy+g_WarlordSourceMaterialParameters[2u].zzzz)+g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = cos(((g_WarlordSourceMaterialParameters[2u].yyyy).x*1.0));
    source[10].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].w = ((g_WarlordSourceMaterialParameters[2u].yyyy).x+(g_WarlordSourceMaterialParameters[2u].zzzz).x);
    source[11].x = cos(((((g_WarlordSourceMaterialParameters[2u].yyyy).x+(g_WarlordSourceMaterialParameters[2u].zzzz).x)+(g_WarlordSourceMaterialParameters[2u].zzzz).x)*1.0));
    source[11].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[11].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[12].y = ((g_WarlordSourceMaterialParameters[1u].zzzz).x*3.0);
    source[12].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[12].w = ((g_WarlordSourceMaterialParameters[1u].wwww).x*20.0);
    source[13].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[13].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[13].w = (100.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[14].x = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x));
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
    r1.xyzw = ((r0.xxxx)*(float4(1.330000,1.330000,1.768900,1.768900))).xyzw;
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
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: dp2 r0.y, cb0[7].xyxx, r1.xyxx
    r0.y = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 11: mad r2.y, v4.x, l(0.020000), r0.y
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 12: dp2 r2.x, cb0[6].xyxx, r1.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 13: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.z = ((-(r0.zzzz))*(float4(1.428571,1.428571,1.428571,1.428571))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
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
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1121(WARLORD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].yyyy,g_WarlordSourceMaterialParameters[2u].zzzz,1u);
    source[5].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[5].y = g_WarlordSourceMaterialTime;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[8].x = (-0.523599029*(g_WarlordSourceMaterialParameters[2u].wwww).x);
    source[8].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 2: mad r0.y, cb0[5].y, cb0[7].w, cb0[8].x
    r0.y = ((source[5].yyyy)*(source[7].wwww)+(source[8].xxxx)).y;
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
    // 10: mad r0.x, r0.y, cb0[4].x, r0.x
    r0.x = ((r0.yyyy)*(source[4].xxxx)+(r0.xxxx)).x;
    // 11: mul r0.z, r0.w, cb0[4].y
    r0.z = ((r0.wwww)*(source[4].yyyy)).z;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.zxyw, s3, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 14: mul r0.y, v4.y, cb0[7].x
    r0.y = ((v4.yyyy)*(source[7].xxxx)).y;
    // 15: mul r0.z, cb0[5].x, cb0[5].y
    r0.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 16: mad r1.w, r0.z, cb0[7].y, r0.y
    r1.w = ((r0.zzzz)*(source[7].yyyy)+(r0.yyyy)).w;
    // 17: mul r0.yw, v4.yyyx, cb0[6].xxxw
    r0.yw = ((v4.yyyx)*(source[6].xxxw)).yw;
    // 18: mad r1.yz, r0.zzzz, cb0[6].yyzy, r0.yywy
    r1.yz = ((r0.zzzz)*(source[6].yyzy)+(r0.yywy)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t1.xzyw, s2, l(0.000000)
    r0.y = (WarlordNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: mul r0.y, v4.x, cb0[5].w
    r0.y = ((v4.xxxx)*(source[5].wwww)).y;
    // 22: mad r1.x, r0.z, cb0[5].z, r0.y
    r1.x = ((r0.zzzz)*(source[5].zzzz)+(r0.yyyy)).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xzyw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 24: add r0.z, -cb0[3].x, l(1.000000)
    r0.z = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: mad r0.x, r0.y, r0.x, -r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 26: mul_sat r0.x, r0.x, cb0[8].w
    r0.x = (saturate((r0.xxxx)*(source[8].wwww))).x;
    // 27: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 28: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.y, r0.y, cb0[9].x
    r0.y = ((r0.yyyy)*(source[9].xxxx)).y;
    // 30: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 31: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 32: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 33: source device depth mapped to centimetre view depth; reconstruction at 35.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 35-38: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 39: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 40: add r0.w, -cb0[9].y, l(1.000000)
    r0.w = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 42: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 45: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 46: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 47: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 48: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 49: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 50: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)

// Exact source a7880c2699099d408e9b6be6a6ce1089; directional/no-static-shadow light PS.
float3 WarlordNative1122Directional(WARLORD_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)
{
    float4 source[20];
    [unroll] for (uint i=0u;i<20u;++i) source[i]=0.f;
    float4 output=0.f;
    float4 diffuse=0.f;
    source[18]=float4(lightColor,1.f);
    source[19].x=0.f; // No source shadow texture in the effect pass; use native no-shadow branch.
    source[2] = g_WarlordSourceMaterialParameters[14u];
    source[3] = g_WarlordSourceMaterialParameters[4u];
    source[4] = g_WarlordSourceMaterialParameters[6u];
    source[5] = g_WarlordSourceMaterialParameters[7u];
    source[6] = g_WarlordSourceMaterialParameters[16u];
    source[7] = g_WarlordSourceMaterialParameters[9u];
    source[8] = g_WarlordSourceMaterialParameters[8u];
    source[9] = input.dynamicParameter;
    source[10] = g_WarlordSourceMaterialParameters[13u];
    source[11] = WarlordNativeAppend(g_WarlordSourceMaterialTime.xxxx,g_WarlordSourceMaterialTime.xxxx,1u);
    source[12] = g_WarlordSourceMaterialParameters[15u];
    source[13].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[13].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[13].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[13].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[14].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[14].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[15].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[16].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[16].y = clamp((g_WarlordSourceMaterialParameters[2u].xxxx).x,0.0,1.0);
    source[16].z = g_WarlordSourceMaterialTime;
    source[16].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[17].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[17].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f)};
    float4 v4=float4(input.uv,0.f,0.f);
    float4 v5=float4(tangentLight,1.f);
    float4 v6=float4(input.tangentUp,0.f);
    float4 v7=float4(input.tangentView,1.f);
    float4 v8=float4(0.f,0.f,0.f,1.f);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 4: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = (WarlordNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r3.xy, r2.xyxx, cb0[13].xxxx
    r3.xy = ((r2.xyxx)*(source[13].xxxx)).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = (WarlordNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 22: mul r4.xy, v4.xyxx, cb0[15].xxxx
    r4.xy = ((v4.xyxx)*(source[15].xxxx)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.xyxx, t2.yzwx, s5, l(0.000000)
    r1.w = (WarlordNativeSample4((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 24: mov_sat r2.w, cb0[9].x
    r2.w = (saturate(source[9].xxxx)).w;
    // 25: add r2.w, -r2.w, cb0[15].y
    r2.w = ((-(r2.wwww))+(source[15].yyyy)).w;
    // 26: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: add r1.w, r1.w, -r2.w
    r1.w = ((r1.wwww)+(-(r2.wwww))).w;
    // 28: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 29: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 30: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 31: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 32: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 33: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[19].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[19].xxxx)) * 0xffffffffu)).w;
    // 34: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 35: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 36: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native dynamic-shadow branch cannot execute (source[19].x == 0).
    r4.xyz = (float4(1.f,1.f,1.f,1.f)).xyz;
    // 38: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 39: else
    } else {
    // 40: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 41: endif
    }
    // 42: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = (WarlordNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 44: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 45: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 46: mul r1.w, r6.x, cb0[15].z
    r1.w = ((r6.xxxx)*(source[15].zzzz)).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 49: add_sat r1.w, r1.w, cb0[15].w
    r1.w = (saturate((r1.wwww)+(source[15].wwww))).w;
    // 50: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 52: mul r2.w, r6.y, cb0[13].y
    r2.w = ((r6.yyyy)*(source[13].yyyy)).w;
    // 53: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 54: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 56: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 57: max r7.xyw, r6.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r6.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 58: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 59: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 60: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 61: add r6.xyw, -r7.xyxw, r6.xyxw
    r6.xyw = ((-(r7.xyxw))+(r6.xyxw)).xyw;
    // 62: mad r6.xyw, r2.wwww, r6.xyxw, r7.xyxw
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r7.xyxw)).xyw;
    // 63: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 64: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 65: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 66: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 67: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 68: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 69: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xy = (WarlordNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 71: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 72: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 73: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 74: max r9.xzw, r7.xxyw, l(0.002170, 0.000000, 0.002170, 0.002170)
    r9.xzw = (max(r7.xxyw,float4(0.002170,0.000000,0.002170,0.002170))).xzw;
    // 75: min r9.xzw, r9.xxzw, l(100.000000, 0.000000, 100.000000, 100.000000)
    r9.xzw = (min(r9.xxzw,float4(100.000000,0.000000,100.000000,100.000000))).xzw;
    // 76: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 77: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 78: add r7.xyw, -r9.xzxw, r7.xyxw
    r7.xyw = ((-(r9.xzxw))+(r7.xyxw)).xyw;
    // 79: mad r7.xyw, r2.wwww, r7.xyxw, r9.xzxw
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xzxw)).xyw;
    // 80: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 81: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 82: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 84: mad r6.xyw, cb0[13].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[13].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 85: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 86: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 87: mad r6.xyw, cb0[14].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[14].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 88: mad r7.xyw, cb0[7].wwww, cb0[7].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[7].wwww)*(source[7].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 89: mad r9.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 90: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 91: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 92: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r9.xyz, -r3.xyzx, r3.wwww
    r9.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 94: mad r3.xyz, cb0[13].wwww, r9.xyzx, r3.xyzx
    r3.xyz = ((source[13].wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 95: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r9.xyz, -r3.xyzx, r3.wwww
    r9.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 97: mad r3.xyz, cb0[14].xxxx, r9.xyzx, r3.xyzx
    r3.xyz = ((source[14].xxxx)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 98: mul r9.xyz, r3.xyzx, r6.xywx
    r9.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 99: dp3 r3.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 101: mad r3.xyz, cb0[13].wwww, r3.xyzx, r9.xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(r9.xyzx)).xyz;
    // 102: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 104: mad r3.xyz, cb0[14].xxxx, r6.xywx, r3.xyzx
    r3.xyz = ((source[14].xxxx)*(r6.xywx)+(r3.xyzx)).xyz;
    // 105: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 106: mul r3.w, cb0[6].z, l(1.500000)
    r3.w = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 107: add r4.w, -cb0[6].w, l(1.000000)
    r4.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: mul r4.w, r4.w, cb0[16].z
    r4.w = ((r4.wwww)*(source[16].zzzz)).w;
    // 109: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 110: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 111: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 113: mad r3.w, r3.w, l(0.500000), cb0[6].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 114: frc r4.w, cb0[6].x
    r4.w = (frac(source[6].xxxx)).w;
    // 115: add r5.w, -r4.w, cb0[6].x
    r5.w = ((-(r4.wwww))+(source[6].xxxx)).w;
    // 116: mul r9.z, r5.w, l(0.125000)
    r9.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 117: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 118: mul r9.y, cb0[6].y, cb0[11].y
    r9.y = ((source[6].yyyy)*(source[11].yyyy)).y;
    // 119: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 120: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 121: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 122: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 123: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 124: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = (WarlordNativeSample5((r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 125: mul r6.xyw, r3.wwww, r9.xyxz
    r6.xyw = ((r3.wwww)*(r9.xyxz)).xyw;
    // 126: mul r3.w, r4.w, r9.w
    r3.w = ((r4.wwww)*(r9.wwww)).w;
    // 127: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 128: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 129: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 130: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 131: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 132: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 133: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 134: mov_sat r1.w, cb0[16].w
    r1.w = (saturate(source[16].wwww)).w;
    // 135: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 136: mul r3.w, r6.z, cb0[17].x
    r3.w = ((r6.zzzz)*(source[17].xxxx)).w;
    // 137: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 138: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 140: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 141: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 143: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 144: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 145: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 146: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 147: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 148: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 149: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 151: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 152: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 155: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 156: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 158: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 159: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 160: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 161: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 162: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 164: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 165: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 166: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 167: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 168: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 169: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 170: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 171: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 172: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 173: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 174: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 175: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 176: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 178: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 179: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 180: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 181: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 182: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 183: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 184: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 185: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 186: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 187: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 188: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 189: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 190: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 191: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 192: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 193: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 194: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 195: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 196: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 197: mul_sat r6.xyz, cb0[12].xyzx, cb0[12].wwww
    r6.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 198: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 199: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 200: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 201: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 202: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 203: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 204: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 205: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 206: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 207: mul r0.x, r0.x, cb0[17].y
    r0.x = ((r0.xxxx)*(source[17].yyyy)).x;
    // 208: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 209: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 210: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 211: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 212: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 213: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 214: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 215: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output.rgb;
}


// Exact source 5bb199a91c36704d905fd13e15b315af; material-owned emissive/diffuse slice; IBL is deliberately unconnected.
float4 WarlordNative1122(WARLORD_NATIVE_INPUT input)
{
    float4 source[35];
    [unroll] for (uint i=0u;i<35u;++i) source[i]=0.f;
    float4 output=0.f;
    float4 diffuse=0.f;
    source[2] = g_WarlordSourceMaterialParameters[14u];
    source[3] = g_WarlordSourceMaterialParameters[4u];
    source[4] = g_WarlordSourceMaterialParameters[6u];
    source[5] = g_WarlordSourceMaterialParameters[7u];
    source[6] = g_WarlordSourceMaterialParameters[16u];
    source[7] = g_WarlordSourceMaterialParameters[9u];
    source[8] = g_WarlordSourceMaterialParameters[8u];
    source[9] = g_WarlordSourceMaterialParameters[17u];
    source[10] = g_WarlordSourceMaterialParameters[5u];
    source[11] = g_WarlordSourceMaterialParameters[11u];
    source[12] = input.dynamicParameter;
    source[13] = g_WarlordSourceMaterialParameters[10u];
    source[14] = g_WarlordSourceMaterialParameters[13u];
    source[15] = WarlordNativeAppend(g_WarlordSourceMaterialTime.xxxx,g_WarlordSourceMaterialTime.xxxx,1u);
    source[16] = g_WarlordSourceMaterialParameters[15u];
    source[17].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[17].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[17].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[18].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[18].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[18].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[18].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[19].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[19].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[19].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[19].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[20].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[20].y = clamp((g_WarlordSourceMaterialParameters[2u].xxxx).x,0.0,1.0);
    source[20].z = g_WarlordSourceMaterialTime;
    source[20].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[21].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[21].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f)};
    float4 v4=float4(input.uv,0.f,0.f);
    float4 v5=float4(input.tangentView,1.f);
    float4 v6=float4(input.tangentUp,0.f);
    float4 v7=float4(input.tangentView,1.f);
    float4 v8=float4(0.f,0.f,0.f,1.f);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: mov_sat r0.x, cb0[12].x
    r0.x = (saturate(source[12].xxxx)).x;
    // 2: add r0.x, -r0.x, cb0[19].y
    r0.x = ((-(r0.xxxx))+(source[19].yyyy)).x;
    // 3: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 4: mul r0.yz, v4.xxyx, cb0[19].xxxx
    r0.yz = ((v4.xxyx)*(source[19].xxxx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (WarlordNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: add r0.z, -r0.x, r0.y
    r0.z = ((-(r0.xxxx))+(r0.yyyy)).z;
    // 7: mad r0.x, r0.x, l(-1.100000), r0.y
    r0.x = ((r0.xxxx)*(float4(-1.100000,-1.100000,-1.100000,-1.100000))+(r0.yyyy)).x;
    // 8: round_pi_sat r0.xy, r0.xzxx
    r0.xy = (saturate(ceil(r0.xzxx))).xy;
    // 9: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = (WarlordNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 11: mul_sat r0.y, r0.y, r1.w
    r0.y = (saturate((r0.yyyy)*(r1.wwww))).y;
    // 12: add r0.y, r0.y, l(-0.333300)
    r0.y = ((r0.yyyy)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 13: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 14: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) clip(-1.f);
    // 16: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 17: add r0.yzw, -r1.xxyz, r0.yyyy
    r0.yzw = ((-(r1.xxyz))+(r0.yyyy)).yzw;
    // 18: mad r0.yzw, cb0[17].wwww, r0.yyzw, r1.xxyz
    r0.yzw = ((source[17].wwww)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 19: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 21: mad r0.yzw, cb0[18].xxxx, r2.xxyz, r0.yyzw
    r0.yzw = ((source[18].xxxx)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 22: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 23: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 24: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 25: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 26: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 27: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = (WarlordNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 29: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 30: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 31: mul r1.w, r5.y, cb0[17].y
    r1.w = ((r5.yyyy)*(source[17].yyyy)).w;
    // 32: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 33: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: movc r1.w, r4.y, l(0), r1.w
    r1.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 35: mad r2.xyz, r1.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 36: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 37: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 38: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 39: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 40: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 41: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 42: mad r3.xyz, r1.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 43: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r6.xyz = (WarlordNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 45: mad r2.xyz, r6.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r6.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 46: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 47: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 48: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 49: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 50: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 51: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 52: mad r3.xyz, r1.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 54: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 55: mad r2.xyz, r6.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 56: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 58: mad r3.xyz, cb0[17].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[17].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 59: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 60: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r2.xyz, -r3.xyzx, r1.wwww
    r2.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 62: mad r2.xyz, cb0[18].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[18].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 63: mad r3.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 64: mad r6.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r6.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 65: mul r3.xyz, r3.xyzx, r6.xywx
    r3.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 66: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 67: mul r6.xyw, r0.yzyw, r2.xyxz
    r6.xyw = ((r0.yzyw)*(r2.xyxz)).xyw;
    // 68: dp3 r1.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 69: mad r0.yzw, -r2.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r2.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 70: mad r0.yzw, cb0[17].wwww, r0.yyzw, r6.xxyw
    r0.yzw = ((source[17].wwww)*(r0.yyzw)+(r6.xxyw)).yzw;
    // 71: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 72: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 73: mad r0.yzw, cb0[18].xxxx, r2.xxyz, r0.yyzw
    r0.yzw = ((source[18].xxxx)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 74: mul r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)*(r0.yyzw)).yzw;
    // 75: add r1.w, -cb0[6].w, l(1.000000)
    r1.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul r1.w, r1.w, cb0[20].z
    r1.w = ((r1.wwww)*(source[20].zzzz)).w;
    // 77: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 78: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 79: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: mul r2.x, cb0[6].z, l(1.500000)
    r2.x = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 81: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 82: mad r1.w, r1.w, l(0.500000), cb0[6].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 83: frc r2.x, v4.x
    r2.x = (frac(v4.xxxx)).x;
    // 84: mul r2.x, r2.x, l(0.125000)
    r2.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 85: mul r3.y, cb0[6].y, cb0[15].y
    r3.y = ((source[6].yyyy)*(source[15].yyyy)).y;
    // 86: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 87: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 88: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 89: frc r2.z, cb0[6].x
    r2.z = (frac(source[6].xxxx)).z;
    // 90: add r2.w, -r2.z, cb0[6].x
    r2.w = ((-(r2.zzzz))+(source[6].xxxx)).w;
    // 91: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 92: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 93: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r3.xyzw = (WarlordNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 94: mul r2.xyw, r1.wwww, r3.xyxz
    r2.xyw = ((r1.wwww)*(r3.xyxz)).xyw;
    // 95: mul r1.w, r2.z, r3.w
    r1.w = ((r2.zzzz)*(r3.wwww)).w;
    // 96: mad r2.xyz, r2.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.yzwy
    r2.xyz = ((r2.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.yzwy))).xyz;
    // 97: mad r0.yzw, r1.wwww, r2.xxyz, r0.yyzw
    r0.yzw = ((r1.wwww)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 98: mul r1.w, r5.x, cb0[19].z
    r1.w = ((r5.xxxx)*(source[19].zzzz)).w;
    // 105: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 106: movc r1.w, r4.x, l(0), r1.w
    r1.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 107: add_sat r1.w, r1.w, cb0[19].w
    r1.w = (saturate((r1.wwww)+(source[19].wwww))).w;
    // 108: add r2.x, -r1.w, l(1.000000)
    r2.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 109: mul r2.xyz, r2.xxxx, cb0[14].xyzx
    r2.xyz = ((r2.xxxx)*(source[14].xyzx)).xyz;
    // 110: mul r3.xyz, r0.yzwy, r2.xyzx
    r3.xyz = ((r0.yzwy)*(r2.xyzx)).xyz;
    // 111: mad r0.yzw, -r2.xxyz, r0.yyzw, r0.yyzw
    r0.yzw = ((-(r2.xxyz))*(r0.yyzw)+(r0.yyzw)).yzw;
    // 112: mad r0.yzw, r1.wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 113: add r2.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 114: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 115: mad_sat r2.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 129: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = (WarlordNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 130: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 131: dp2 r3.w, r4.xyxx, r4.xyxx
    r3.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 132: mul r4.xy, r4.xyxx, cb0[17].xxxx
    r4.xy = ((r4.xyxx)*(source[17].xxxx)).xy;
    // 133: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 135: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 136: add r4.z, r3.w, l(0.000010)
    r4.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 137: dp3 r3.w, r4.xyzx, r4.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 138: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 139: div r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)/(r3.wwww)).xyz;
    // 143: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 144: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 145: mul r6.xyw, r3.wwww, v5.xyxz
    r6.xyw = ((r3.wwww)*(v5.xyxz)).xyw;
    // 302: dp3 r1.w, r4.xyzx, r6.xywx
    r1.w = (dot((r4.xyzx).xyz,(r6.xywx).xyz).xxxx).w;
    // 303: mul_sat r2.w, r1.w, cb0[18].y
    r2.w = (saturate((r1.wwww)*(source[18].yyyy))).w;
    // 304: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 305: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 306: mul_sat r3.x, r6.w, cb0[18].y
    r3.x = (saturate((r6.wwww)*(source[18].yyyy))).x;
    // 307: add r3.y, -|r6.w|, l(1.000000)
    r3.y = ((-(abs(r6.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 308: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 309: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 310: add_sat r3.x, r3.x, -cb0[18].z
    r3.x = (saturate((r3.xxxx)+(-(source[18].zzzz)))).x;
    // 311: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 312: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 313: mul r3.y, r3.y, cb0[18].w
    r3.y = ((r3.yyyy)*(source[18].wwww)).y;
    // 314: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 315: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 316: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 317: add r3.x, -r6.z, r2.w
    r3.x = ((-(r6.zzzz))+(r2.wwww)).x;
    // 318: mad r4.xyz, r2.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r4.xyz = ((r2.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 319: mad r4.xyz, cb0[10].wwww, r4.xyzx, cb0[10].xyzx
    r4.xyz = ((source[10].wwww)*(r4.xyzx)+(source[10].xyzx)).xyz;
    // 320: mad r2.w, cb0[9].w, r3.x, r6.z
    r2.w = ((source[9].wwww)*(r3.xxxx)+(r6.zzzz)).w;
    // 321: mad r3.xyz, r2.wwww, cb0[9].xyzx, r4.xyzx
    r3.xyz = ((r2.wwww)*(source[9].xyzx)+(r4.xyzx)).xyz;
    // 322: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 323: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 324: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 325: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 326: mul r4.xyz, r2.wwww, cb0[11].xyzx
    r4.xyz = ((r2.wwww)*(source[11].xyzx)).xyz;
    // 327: movc r4.xyz, r1.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 328: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 329: mad r3.xyz, r0.xxxx, cb0[13].xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(source[13].xyzx)+(r3.xyzx)).xyz;
    // 330: mad r1.xyz, cb0[17].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[17].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 331: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 332: mov o0.xyz, r1.xyzx
    output.xyz = (r1.xyzx).xyz;
    // 333: mov o3.xyz, r2.xyzx
    diffuse.xyz = (r2.xyzx).xyz;
    // Existing SourceCharacter composition: scene ambient multiplies resolved diffuse.
    float3 radiance=output.rgb+diffuse.rgb*g_WarlordAmbient.rgb;
    [loop] for(uint light=0u;light<g_WarlordGuardianLightCount;++light)
    {
        float3 tangentLight, lightColor; float attenuation;
        if (WarlordGuardianLight(light,input,tangentLight,lightColor,attenuation))
            radiance+=WarlordNative1122Directional(input,tangentLight,lightColor)*attenuation;
    }
    return float4(radiance,1.f);
}


// Exact source 6a9554452ea3d2469eef6e627bc6cf3f; directional/no-static-shadow light PS.
float3 WarlordNative1123Directional(WARLORD_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)
{
    float4 source[20];
    [unroll] for (uint i=0u;i<20u;++i) source[i]=0.f;
    float4 output=0.f;
    float4 diffuse=0.f;
    source[18]=float4(lightColor,1.f);
    source[19].x=0.f; // No source shadow texture in the effect pass; use native no-shadow branch.
    source[2] = g_WarlordSourceMaterialParameters[18u];
    source[3] = g_WarlordSourceMaterialParameters[5u];
    source[4] = g_WarlordSourceMaterialParameters[7u];
    source[5] = g_WarlordSourceMaterialParameters[8u];
    source[6] = g_WarlordSourceMaterialParameters[20u];
    source[7] = g_WarlordSourceMaterialParameters[11u];
    source[8] = g_WarlordSourceMaterialParameters[10u];
    source[9] = input.dynamicParameter;
    source[10] = g_WarlordSourceMaterialParameters[17u];
    source[11] = WarlordNativeAppend(g_WarlordSourceMaterialTime.xxxx,g_WarlordSourceMaterialTime.xxxx,1u);
    source[12] = g_WarlordSourceMaterialParameters[19u];
    source[13].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[13].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[14].x = g_WarlordSourceMaterialTime;
    source[14].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[14].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[15].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[15].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[16].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[16].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[16].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[16].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[17].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[17].y = clamp((g_WarlordSourceMaterialParameters[2u].wwww).x,0.0,1.0);
    source[17].z = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[17].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f)};
    float4 v4=float4(input.uv,0.f,0.f);
    float4 v5=float4(tangentLight,1.f);
    float4 v6=float4(input.tangentUp,0.f);
    float4 v7=float4(input.tangentView,1.f);
    float4 v8=float4(0.f,0.f,0.f,1.f);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 4: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = (WarlordNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r3.xy, r2.xyxx, cb0[13].xxxx
    r3.xy = ((r2.xyxx)*(source[13].xxxx)).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = (WarlordNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 22: mul r4.xy, v4.xyxx, cb0[16].xxxx
    r4.xy = ((v4.xyxx)*(source[16].xxxx)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.xyxx, t2.yzwx, s5, l(0.000000)
    r1.w = (WarlordNativeSample6((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 24: mov_sat r2.w, cb0[9].x
    r2.w = (saturate(source[9].xxxx)).w;
    // 25: add r2.w, -r2.w, cb0[16].y
    r2.w = ((-(r2.wwww))+(source[16].yyyy)).w;
    // 26: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: add r1.w, r1.w, -r2.w
    r1.w = ((r1.wwww)+(-(r2.wwww))).w;
    // 28: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 29: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 30: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 31: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 32: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 33: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[19].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[19].xxxx)) * 0xffffffffu)).w;
    // 34: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 35: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 36: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native dynamic-shadow branch cannot execute (source[19].x == 0).
    r4.xyz = (float4(1.f,1.f,1.f,1.f)).xyz;
    // 38: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 39: else
    } else {
    // 40: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 41: endif
    }
    // 42: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = (WarlordNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 44: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 45: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 46: mul r1.w, r6.x, cb0[16].z
    r1.w = ((r6.xxxx)*(source[16].zzzz)).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 49: add_sat r1.w, r1.w, cb0[16].w
    r1.w = (saturate((r1.wwww)+(source[16].wwww))).w;
    // 50: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 52: mul r2.w, r6.y, cb0[13].y
    r2.w = ((r6.yyyy)*(source[13].yyyy)).w;
    // 53: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 54: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 56: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 57: max r7.xyw, r6.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r6.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 58: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 59: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 60: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 61: add r6.xyw, -r7.xyxw, r6.xyxw
    r6.xyw = ((-(r7.xyxw))+(r6.xyxw)).xyw;
    // 62: mad r6.xyw, r2.wwww, r6.xyxw, r7.xyxw
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r7.xyxw)).xyw;
    // 63: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 64: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 65: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 66: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 67: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 68: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 69: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xy = (WarlordNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 71: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 72: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 73: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 74: max r9.xzw, r7.xxyw, l(0.002170, 0.000000, 0.002170, 0.002170)
    r9.xzw = (max(r7.xxyw,float4(0.002170,0.000000,0.002170,0.002170))).xzw;
    // 75: min r9.xzw, r9.xxzw, l(100.000000, 0.000000, 100.000000, 100.000000)
    r9.xzw = (min(r9.xxzw,float4(100.000000,0.000000,100.000000,100.000000))).xzw;
    // 76: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 77: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 78: add r7.xyw, -r9.xzxw, r7.xyxw
    r7.xyw = ((-(r9.xzxw))+(r7.xyxw)).xyw;
    // 79: mad r7.xyw, r2.wwww, r7.xyxw, r9.xzxw
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xzxw)).xyw;
    // 80: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 81: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 82: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 84: mad r6.xyw, cb0[15].zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 85: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 86: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 87: mad r6.xyw, cb0[15].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 88: mad r7.xyw, cb0[7].wwww, cb0[7].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[7].wwww)*(source[7].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 89: mad r9.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 90: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 91: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 92: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r9.xyz, -r3.xyzx, r3.wwww
    r9.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 94: mad r3.xyz, cb0[15].zzzz, r9.xyzx, r3.xyzx
    r3.xyz = ((source[15].zzzz)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 95: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r9.xyz, -r3.xyzx, r3.wwww
    r9.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 97: mad r3.xyz, cb0[15].wwww, r9.xyzx, r3.xyzx
    r3.xyz = ((source[15].wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 98: mul r9.xyz, r3.xyzx, r6.xywx
    r9.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 99: dp3 r3.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 101: mad r3.xyz, cb0[15].zzzz, r3.xyzx, r9.xyzx
    r3.xyz = ((source[15].zzzz)*(r3.xyzx)+(r9.xyzx)).xyz;
    // 102: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 104: mad r3.xyz, cb0[15].wwww, r6.xywx, r3.xyzx
    r3.xyz = ((source[15].wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 105: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 106: mul r3.w, cb0[6].z, l(1.500000)
    r3.w = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 107: add r4.w, -cb0[6].w, l(1.000000)
    r4.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: mul r4.w, r4.w, cb0[14].x
    r4.w = ((r4.wwww)*(source[14].xxxx)).w;
    // 109: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 110: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 111: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 113: mad r3.w, r3.w, l(0.500000), cb0[6].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 114: frc r4.w, cb0[6].x
    r4.w = (frac(source[6].xxxx)).w;
    // 115: add r5.w, -r4.w, cb0[6].x
    r5.w = ((-(r4.wwww))+(source[6].xxxx)).w;
    // 116: mul r9.z, r5.w, l(0.125000)
    r9.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 117: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 118: mul r9.y, cb0[6].y, cb0[11].y
    r9.y = ((source[6].yyyy)*(source[11].yyyy)).y;
    // 119: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 120: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 121: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 122: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 123: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 124: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = (WarlordNativeSample7((r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 125: mul r6.xyw, r3.wwww, r9.xyxz
    r6.xyw = ((r3.wwww)*(r9.xyxz)).xyw;
    // 126: mul r3.w, r4.w, r9.w
    r3.w = ((r4.wwww)*(r9.wwww)).w;
    // 127: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 128: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 129: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 130: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 131: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 132: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 133: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 134: mul_sat r1.w, r2.w, cb2[3].w
    r1.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 135: mul r2.w, r6.z, cb0[17].z
    r2.w = ((r6.zzzz)*(source[17].zzzz)).w;
    // 136: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 137: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: movc r2.w, r7.z, l(0), r2.w
    r2.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 139: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 140: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 142: dp3 r3.w, r5.xyzx, r5.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 143: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 144: mul r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 145: dp3_sat r3.w, r2.xyzx, r5.xyzx
    r3.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 146: dp3 r4.w, r2.xyzx, r0.xyzx
    r4.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 147: add r4.w, |r4.w|, l(0.000010)
    r4.w = ((abs(r4.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 148: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: dp3_sat r5.w, r2.xyzx, r1.xyzx
    r5.w = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).w;
    // 150: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 151: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 154: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 155: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: mad r5.xyz, -r3.xyzx, r1.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r1.wwww)+(r3.xyzx)).xyz;
    // 157: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 158: mul r6.x, r2.w, r2.w
    r6.x = ((r2.wwww)*(r2.wwww)).x;
    // 159: mul r6.y, r6.x, r6.x
    r6.y = ((r6.xxxx)*(r6.xxxx)).y;
    // 160: mad r6.z, r3.w, r6.y, -r3.w
    r6.z = ((r3.wwww)*(r6.yyyy)+(-(r3.wwww))).z;
    // 161: mad r3.w, r6.z, r3.w, l(1.000000)
    r3.w = ((r6.zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 163: mul r3.w, r3.w, l(3.141593)
    r3.w = ((r3.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 164: div r3.w, r6.y, r3.w
    r3.w = ((r6.yyyy)/(r3.wwww)).w;
    // 165: mad r6.y, -r2.w, r2.w, l(1.000000)
    r6.y = ((-(r2.wwww))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 166: mad r6.z, r4.w, r6.y, r6.x
    r6.z = ((r4.wwww)*(r6.yyyy)+(r6.xxxx)).z;
    // 167: mad r6.x, r5.w, r6.y, r6.x
    r6.x = ((r5.wwww)*(r6.yyyy)+(r6.xxxx)).x;
    // 168: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 169: mad r4.w, r5.w, r6.z, r4.w
    r4.w = ((r5.wwww)*(r6.zzzz)+(r4.wwww)).w;
    // 170: rcp r4.w, r4.w
    r4.w = (1.0/(r4.wwww)).w;
    // 171: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 172: add r6.xyz, r3.xyzx, l(-0.040000, -0.040000, -0.040000, 0.000000)
    r6.xyz = ((r3.xyzx)+(float4(-0.040000,-0.040000,-0.040000,0.000000))).xyz;
    // 173: mad r6.xyz, r1.wwww, r6.xyzx, l(0.040000, 0.040000, 0.040000, 0.000000)
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(float4(0.040000,0.040000,0.040000,0.000000))).xyz;
    // 174: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mul r4.w, r0.w, r0.w
    r4.w = ((r0.wwww)*(r0.wwww)).w;
    // 176: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 177: mul r0.w, r0.w, r4.w
    r0.w = ((r0.wwww)*(r4.wwww)).w;
    // 178: mul r4.w, r6.y, l(50.000000)
    r4.w = ((r6.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000))).w;
    // 179: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: mul r4.w, r0.w, r4.w
    r4.w = ((r0.wwww)*(r4.wwww)).w;
    // 181: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 182: max r7.xyz, r6.xyzx, r2.wwww
    r7.xyz = (max(r6.xyzx,r2.wwww)).xyz;
    // 183: add r7.xyz, -r6.xyzx, r7.xyzx
    r7.xyz = ((-(r6.xyzx))+(r7.xyzx)).xyz;
    // 184: mad r6.xyz, -r0.wwww, r6.xyzx, r6.xyzx
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(r6.xyzx)).xyz;
    // 185: mad r6.xyz, r4.wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((r4.wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 186: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 187: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 188: mul r2.w, r3.w, l(0.500000)
    r2.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 189: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 190: min r0.w, r0.w, r2.w
    r0.w = (min(r0.wwww,r2.wwww)).w;
    // 191: mul r7.xyz, r6.xyzx, r0.wwww
    r7.xyz = ((r6.xyzx)*(r0.wwww)).xyz;
    // 192: add r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mad r5.xyz, r5.xyzx, r6.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 194: mul r5.xyz, r5.wwww, r5.xyzx
    r5.xyz = ((r5.wwww)*(r5.xyzx)).xyz;
    // 195: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 196: mul_sat r6.xyz, cb0[12].xyzx, cb0[12].wwww
    r6.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 197: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 198: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 199: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 200: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 201: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 202: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 203: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 204: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 205: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 206: mul r0.x, r0.x, cb0[17].w
    r0.x = ((r0.xxxx)*(source[17].wwww)).x;
    // 207: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 208: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 209: add r0.w, -r1.w, l(1.000000)
    r0.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 211: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 212: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 213: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 214: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output.rgb;
}

// Exact source 6fc176effef5c5499251c1098803727b; material-owned emissive/diffuse slice; IBL is deliberately unconnected.
float4 WarlordNative1123(WARLORD_NATIVE_INPUT input)
{
    float4 source[38];
    [unroll] for (uint i=0u;i<38u;++i) source[i]=0.f;
    float4 output=0.f;
    float4 diffuse=0.f;
    source[2] = g_WarlordSourceMaterialParameters[18u];
    source[3] = g_WarlordSourceMaterialParameters[5u];
    source[4] = g_WarlordSourceMaterialParameters[7u];
    source[5] = g_WarlordSourceMaterialParameters[8u];
    source[6] = g_WarlordSourceMaterialParameters[20u];
    source[7] = g_WarlordSourceMaterialParameters[9u];
    source[8] = g_WarlordSourceMaterialParameters[14u];
    source[9] = g_WarlordSourceMaterialParameters[13u];
    source[10] = g_WarlordSourceMaterialParameters[11u];
    source[11] = g_WarlordSourceMaterialParameters[10u];
    source[12] = g_WarlordSourceMaterialParameters[21u];
    source[13] = g_WarlordSourceMaterialParameters[6u];
    source[14] = g_WarlordSourceMaterialParameters[15u];
    source[15] = input.dynamicParameter;
    source[16] = g_WarlordSourceMaterialParameters[12u];
    source[17] = g_WarlordSourceMaterialParameters[17u];
    source[18] = WarlordNativeAppend(g_WarlordSourceMaterialTime.xxxx,g_WarlordSourceMaterialTime.xxxx,1u);
    source[19] = g_WarlordSourceMaterialParameters[19u];
    source[20].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[20].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[20].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[20].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[21].x = g_WarlordSourceMaterialTime;
    source[21].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[21].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[21].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[22].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[22].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[22].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[22].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[23].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[23].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[23].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[23].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[24].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[24].y = clamp((g_WarlordSourceMaterialParameters[2u].wwww).x,0.0,1.0);
    source[24].z = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[24].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f)};
    float4 v4=float4(input.uv,0.f,0.f);
    float4 v5=float4(input.tangentView,1.f);
    float4 v6=float4(input.tangentUp,0.f);
    float4 v7=float4(input.tangentView,1.f);
    float4 v8=float4(0.f,0.f,0.f,1.f);
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: mov_sat r0.x, cb0[15].x
    r0.x = (saturate(source[15].xxxx)).x;
    // 2: add r0.x, -r0.x, cb0[23].y
    r0.x = ((-(r0.xxxx))+(source[23].yyyy)).x;
    // 3: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 4: mul r0.yz, v4.xxyx, cb0[23].xxxx
    r0.yz = ((v4.xxyx)*(source[23].xxxx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s6, l(0.000000)
    r0.y = (WarlordNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: add r0.z, -r0.x, r0.y
    r0.z = ((-(r0.xxxx))+(r0.yyyy)).z;
    // 7: mad r0.x, r0.x, l(-1.100000), r0.y
    r0.x = ((r0.xxxx)*(float4(-1.100000,-1.100000,-1.100000,-1.100000))+(r0.yyyy)).x;
    // 8: round_pi_sat r0.xy, r0.xzxx
    r0.xy = (saturate(ceil(r0.xzxx))).xy;
    // 9: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = (WarlordNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 11: mul_sat r0.y, r0.y, r1.w
    r0.y = (saturate((r0.yyyy)*(r1.wwww))).y;
    // 12: add r0.y, r0.y, l(-0.333300)
    r0.y = ((r0.yyyy)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 13: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 14: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) clip(-1.f);
    // 16: add r0.y, -cb0[6].w, l(1.000000)
    r0.y = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 17: mul r0.y, r0.y, cb0[21].x
    r0.y = ((r0.yyyy)*(source[21].xxxx)).y;
    // 18: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 19: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 20: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 21: mul r0.z, cb0[6].z, l(1.500000)
    r0.z = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 22: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 23: mad r0.y, r0.y, l(0.500000), cb0[6].z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).y;
    // 24: frc r0.z, v4.x
    r0.z = (frac(v4.xxxx)).z;
    // 25: mul r2.x, r0.z, l(0.125000)
    r2.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 26: mul r3.y, cb0[6].y, cb0[18].y
    r3.y = ((source[6].yyyy)*(source[18].yyyy)).y;
    // 27: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 28: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 29: add r0.zw, r2.xxxy, r3.xxxy
    r0.zw = ((r2.xxxy)+(r3.xxxy)).zw;
    // 30: frc r1.w, cb0[6].x
    r1.w = (frac(source[6].xxxx)).w;
    // 31: add r2.x, -r1.w, cb0[6].x
    r2.x = ((-(r1.wwww))+(source[6].xxxx)).x;
    // 32: mul r3.z, r2.x, l(0.125000)
    r3.z = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 33: add r0.zw, r0.zzzw, r3.zzzw
    r0.zw = ((r0.zzzw)+(r3.zzzw)).zw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.zwzz, t5.xyzw, s7, l(0.000000)
    r2.xyzw = (WarlordNativeSample7((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 35: mul r0.yzw, r0.yyyy, r2.xxyz
    r0.yzw = ((r0.yyyy)*(r2.xxyz)).yzw;
    // 36: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 37: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 38: add r2.xyz, -r1.xyzx, r2.xxxx
    r2.xyz = ((-(r1.xyzx))+(r2.xxxx)).xyz;
    // 39: mad r2.xyz, cb0[22].zzzz, r2.xyzx, r1.xyzx
    r2.xyz = ((source[22].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 40: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 42: mad r2.xyz, cb0[22].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[22].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 43: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 44: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 45: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 46: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 47: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 48: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = (WarlordNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 50: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 51: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 52: mul r2.w, r6.y, cb0[20].y
    r2.w = ((r6.yyyy)*(source[20].yyyy)).w;
    // 53: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 54: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: movc r2.w, r5.y, l(0), r2.w
    r2.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 56: mad r3.xyz, r2.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 57: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 58: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 59: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 60: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 61: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 62: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 63: mad r4.xyz, r2.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 64: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r7.xyz = (WarlordNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 66: mad r3.xyz, r7.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r7.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 67: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 68: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 69: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 70: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 71: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 72: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 73: mad r4.xyz, r2.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 75: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 76: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 77: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 78: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 79: mad r4.xyz, cb0[22].zzzz, r4.xyzx, r3.xyzx
    r4.xyz = ((source[22].zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 80: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 81: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r3.xyz, -r4.xyzx, r2.wwww
    r3.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 83: mad r3.xyz, cb0[22].wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((source[22].wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 84: mad r4.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mad r7.xyw, cb0[11].wwww, cb0[11].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[11].wwww)*(source[11].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 86: mul r4.xyz, r4.xyzx, r7.xywx
    r4.xyz = ((r4.xyzx)*(r7.xywx)).xyz;
    // 87: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 88: mul r7.xyw, r2.xyxz, r3.xyxz
    r7.xyw = ((r2.xyxz)*(r3.xyxz)).xyw;
    // 89: dp3 r2.w, r7.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: mad r2.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 91: mad r2.xyz, cb0[22].zzzz, r2.xyzx, r7.xywx
    r2.xyz = ((source[22].zzzz)*(r2.xyzx)+(r7.xywx)).xyz;
    // 92: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 94: mad r2.xyz, cb0[22].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[22].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 95: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 96: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r2.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r2.xxyz))).yzw;
    // 97: mad r0.yzw, r1.wwww, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 98: mul r1.w, r6.x, cb0[23].z
    r1.w = ((r6.xxxx)*(source[23].zzzz)).w;
    // 105: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 106: movc r1.w, r5.x, l(0), r1.w
    r1.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 107: add_sat r1.w, r1.w, cb0[23].w
    r1.w = (saturate((r1.wwww)+(source[23].wwww))).w;
    // 108: add r2.x, -r1.w, l(1.000000)
    r2.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 109: mul r2.xyz, r2.xxxx, cb0[17].xyzx
    r2.xyz = ((r2.xxxx)*(source[17].xyzx)).xyz;
    // 110: mul r3.xyz, r0.yzwy, r2.xyzx
    r3.xyz = ((r0.yzwy)*(r2.xyzx)).xyz;
    // 111: mad r0.yzw, -r2.xxyz, r0.yyzw, r0.yyzw
    r0.yzw = ((-(r2.xxyz))*(r0.yyzw)+(r0.yyzw)).yzw;
    // 112: mad r0.yzw, r1.wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 113: add r2.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 114: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 115: mad_sat r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = (saturate((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 127: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = (WarlordNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 128: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 129: dp2 r3.w, r5.xyxx, r5.xyxx
    r3.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 130: mul r5.xy, r5.xyxx, cb0[20].xxxx
    r5.xy = ((r5.xyxx)*(source[20].xxxx)).xy;
    // 131: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 133: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 134: add r5.z, r3.w, l(0.000010)
    r5.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 135: dp3 r3.w, r5.xyzx, r5.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 136: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 137: div r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)/(r3.wwww)).xyz;
    // 141: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 142: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 143: mul r7.xyw, r3.wwww, v5.xyxz
    r7.xyw = ((r3.wwww)*(v5.xyxz)).xyw;
    // 300: dp3 r1.w, r5.xyzx, r7.xywx
    r1.w = (dot((r5.xyzx).xyz,(r7.xywx).xyz).xxxx).w;
    // 301: mul_sat r2.w, r1.w, cb0[21].w
    r2.w = (saturate((r1.wwww)*(source[21].wwww))).w;
    // 302: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 303: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 304: mul_sat r3.x, r7.w, cb0[21].w
    r3.x = (saturate((r7.wwww)*(source[21].wwww))).x;
    // 305: add r3.y, -|r7.w|, l(1.000000)
    r3.y = ((-(abs(r7.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 306: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 307: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 308: add_sat r3.x, r3.x, -cb0[22].x
    r3.x = (saturate((r3.xxxx)+(-(source[22].xxxx)))).x;
    // 309: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 310: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 311: mul r3.y, r3.y, cb0[22].y
    r3.y = ((r3.yyyy)*(source[22].yyyy)).y;
    // 312: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 313: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 314: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 315: add r3.x, -r7.z, r2.w
    r3.x = ((-(r7.zzzz))+(r2.wwww)).x;
    // 316: mad r3.x, cb0[12].w, r3.x, r7.z
    r3.x = ((source[12].wwww)*(r3.xxxx)+(r7.zzzz)).x;
    // 317: mad r5.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r5.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 318: mad r5.xyz, cb0[13].wwww, r5.xyzx, cb0[13].xyzx
    r5.xyz = ((source[13].wwww)*(r5.xyzx)+(source[13].xyzx)).xyz;
    // 319: mad r3.xyz, r3.xxxx, cb0[12].xyzx, r5.xyzx
    r3.xyz = ((r3.xxxx)*(source[12].xyzx)+(r5.xyzx)).xyz;
    // 320: mul r5.xy, v4.xyxx, cb0[8].zzzz
    r5.xy = ((v4.xyxx)*(source[8].zzzz)).xy;
    // 321: mul r5.zw, cb0[8].xxxy, cb0[21].xxxx
    r5.zw = ((source[8].xxxy)*(source[21].xxxx)).zw;
    // 322: mad r5.xy, r5.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r5.xyxx
    r5.xy = ((r5.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r5.xyxx)).xy;
    // 323: mad r5.zw, cb0[8].zzzz, v4.xxxy, r5.zzzw
    r5.zw = ((source[8].zzzz)*(v4.xxxy)+(r5.zzzw)).zw;
    // 324: sample_b_indexable(texture2d)(float,float,float,float) r5.x, r5.xyxx, t7.xyzw, s5, l(0.000000)
    r5.x = (WarlordNativeSample5((r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 325: mad r5.xy, r5.xxxx, cb0[21].yyyy, r5.zwzz
    r5.xy = ((r5.xxxx)*(source[21].yyyy)+(r5.zwzz)).xy;
    // 326: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t7.xyzw, s5, l(0.000000)
    r5.xyz = (WarlordNativeSample5((r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 327: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r7.xyzw = (WarlordNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 328: mul r5.xyz, r5.xyzx, r7.wwww
    r5.xyz = ((r5.xyzx)*(r7.wwww)).xyz;
    // 329: mul r8.xyz, cb0[9].xyzx, cb0[21].zzzz
    r8.xyz = ((source[9].xyzx)*(source[21].zzzz)).xyz;
    // 330: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 331: mad r8.xyz, r2.wwww, r5.xyzx, -r5.xyzx
    r8.xyz = ((r2.wwww)*(r5.xyzx)+(-(r5.xyzx))).xyz;
    // 332: mad r5.xyz, cb0[9].wwww, r8.xyzx, r5.xyzx
    r5.xyz = ((source[9].wwww)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 333: mul r8.xyz, cb0[7].xyzx, cb0[20].wwww
    r8.xyz = ((source[7].xyzx)*(source[20].wwww)).xyz;
    // 334: mad r5.xyz, r7.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 335: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 336: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 337: mad r5.xyz, cb0[22].zzzz, r7.xyzx, r5.xyzx
    r5.xyz = ((source[22].zzzz)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 338: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 339: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 340: mad r5.xyz, cb0[22].wwww, r7.xyzx, r5.xyzx
    r5.xyz = ((source[22].wwww)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 341: mad r3.xyz, r5.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 342: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 343: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 344: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 345: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 346: mul r4.xyz, r2.wwww, cb0[14].xyzx
    r4.xyz = ((r2.wwww)*(source[14].xyzx)).xyz;
    // 347: movc r4.xyz, r1.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 348: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 349: mad r3.xyz, r0.xxxx, cb0[16].xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(source[16].xyzx)+(r3.xyzx)).xyz;
    // 350: mad r1.xyz, cb0[20].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[20].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 351: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 352: mov o0.xyz, r1.xyzx
    output.xyz = (r1.xyzx).xyz;
    // 353: mov o3.xyz, r0.yzwy
    diffuse.xyz = (r0.yzwy).xyz;
    // Existing SourceCharacter composition: scene ambient multiplies resolved diffuse.
    float3 radiance=output.rgb+diffuse.rgb*g_WarlordAmbient.rgb;
    [loop] for(uint light=0u;light<g_WarlordGuardianLightCount;++light)
    {
        float3 tangentLight, lightColor; float attenuation;
        if (WarlordGuardianLight(light,input,tangentLight,lightColor,attenuation))
            radiance+=WarlordNative1123Directional(input,tangentLight,lightColor)*attenuation;
    }
    return float4(radiance,1.f);
}


float4 WarlordNative1124(WARLORD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0]=float4(1.f,1.f,1.f,1.f); // Native mesh RGB multiplier and dither opacity prefix; neutral color and fully present coverage.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[9u];
    source[2] = WarlordNativeAppend(WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].wwww,g_WarlordSourceMaterialParameters[3u].wwww,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].wwww,g_WarlordSourceMaterialParameters[1u].zzzz,1u);
    source[4] = g_WarlordSourceMaterialParameters[6u];
    source[5] = input.dynamicParameter;
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].yyyy,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[7] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].zzzz,g_WarlordSourceMaterialParameters[1u].yyyy,1u);
    source[8] = g_WarlordSourceMaterialParameters[7u];
    source[9] = g_WarlordSourceMaterialParameters[5u];
    source[10].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[10].y = g_WarlordSourceMaterialTime;
    source[10].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[10].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[12].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[12].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[12].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[12].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[13].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[13].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[13].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[14].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[14].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[14].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((v6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 2: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 3: mov_sat r0.z, cb0[0].w
    r0.z = (saturate(source[0].wwww)).z;
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
    // 12: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 13: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 14: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 16: mul r0.xy, v4.xyxx, cb0[7].xyxx
    r0.xy = ((v4.xyxx)*(source[7].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 18: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 19: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 20: mad r0.xyz, cb0[13].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[13].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 21: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 22: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 23: mul r0.xyz, r0.xyzx, cb0[13].zzzz
    r0.xyz = ((r0.xyzx)*(source[13].zzzz)).xyz;
    // 24: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 25: mul r1.xy, v4.xyxx, cb0[10].zwzz
    r1.xy = ((v4.xyxx)*(source[10].zwzz)).xy;
    // 26: mad r2.x, cb0[10].y, cb0[10].x, r1.x
    r2.x = ((source[10].yyyy)*(source[10].xxxx)+(r1.xxxx)).x;
    // 27: mad r2.y, cb0[10].y, cb0[11].x, r1.y
    r2.y = ((source[10].yyyy)*(source[11].xxxx)+(r1.yyyy)).y;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 29: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 31: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 34: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 35: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 36: mul r2.xyz, cb0[8].xyzx, cb0[8].wwww
    r2.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 37: mad r3.xy, r0.xyxx, r2.xyxx, r1.xyxx
    r3.xy = ((r0.xyxx)*(r2.xyxx)+(r1.xyxx)).xy;
    // 38: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 39: mul r2.xy, r3.xyxx, cb0[13].wwww
    r2.xy = ((r3.xyxx)*(source[13].wwww)).xy;
    // 40: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 41: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 42: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 43: mad r2.xy, r3.xyxx, cb0[6].xyxx, r2.xyxx
    r2.xy = ((r3.xyxx)*(source[6].xyxx)+(r2.xyxx)).xy;
    // 44: add r0.w, -|r3.z|, l(1.000000)
    r0.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = (WarlordNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 47: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 48: mad r2.xyz, cb0[14].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[14].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 49: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 50: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 51: mul r2.xyz, r2.xyzx, cb0[14].yyyy
    r2.xyz = ((r2.xyzx)*(source[14].yyyy)).xyz;
    // 52: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 53: mul r3.xyz, cb0[9].xyzx, cb0[9].wwww
    r3.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 54: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 55: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 56: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 57: mad r2.w, v1.z, r1.w, l(1.000000)
    r2.w = ((v1.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mul r3.xyz, r1.wwww, v1.xyzx
    r3.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 59: mul_sat r1.w, r2.w, l(0.500000)
    r1.w = (saturate((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 60: mad r1.w, r1.w, l(0.950000), l(0.050000)
    r1.w = ((r1.wwww)*(float4(0.950000,0.950000,0.950000,0.950000))+(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 61: dp3 r1.w, r1.xyzx, r1.wwww
    r1.w = (dot((r1.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 62: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 63: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 64: mul r2.w, r2.w, cb0[14].z
    r2.w = ((r2.wwww)*(source[14].zzzz)).w;
    // 65: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 66: mul r2.w, r2.w, cb0[14].w
    r2.w = ((r2.wwww)*(source[14].wwww)).w;
    // 67: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 68: mad r0.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 69: mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 70: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 71: mad r2.xy, r2.xyxx, cb0[3].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(source[3].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.xzwy, s1, l(0.000000)
    r1.w = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzwy).w;
    // 73: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 74: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 75: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 76: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 77: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 78: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 79: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 80: mul r2.xyz, r2.xyzx, cb0[5].xxxx
    r2.xyz = ((r2.xyzx)*(source[5].xxxx)).xyz;
    // 81: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 82: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 83: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1125(WARLORD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[2].y = g_WarlordSourceMaterialTime;
    source[2].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[2].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[3].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[4].z, l(1.000000)
    r0.y = ((-(source[4].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: mul r0.yz, v2.xxyx, cb0[2].zzwz
    r0.yz = ((v2.xxyx)*(source[2].zzwz)).yz;
    // 14: mad r1.x, cb0[2].y, cb0[2].x, r0.y
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.yyyy)).x;
    // 15: mad r1.y, cb0[2].y, cb0[3].x, r0.z
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.zzzz)).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r1.xyxx, t0.zxyw, s1, l(0.000000)
    r0.yz = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 17: mad r0.yz, cb0[3].yyyy, r0.yyzy, v2.xxyx
    r0.yz = ((source[3].yyyy)*(r0.yyzy)+(v2.xxyx)).yz;
    // 18: mad r0.w, cb0[3].z, v4.x, l(-1.000000)
    r0.w = ((source[3].zzzz)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 19: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 20: mul r1.x, v4.x, cb0[3].z
    r1.x = ((v4.xxxx)*(source[3].zzzz)).x;
    // 21: mad r0.yz, r1.xxxx, r0.yyzy, -r0.wwww
    r0.yz = ((r1.xxxx)*(r0.yyzy)+(-(r0.wwww))).yz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t1.xyzw, s2, l(0.000000)
    r1.xyzw = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 23: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 24: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 27: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 28: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 29: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 30: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 32: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 33: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 34: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 36: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 37: mul r0.xyz, r1.xyzx, cb0[3].wwww
    r0.xyz = ((r1.xyzx)*(source[3].wwww)).xyz;
    // 38: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: mad r1.xyz, -cb0[3].wwww, r1.xyzx, r0.wwww
    r1.xyz = ((-(source[3].wwww))*(r1.xyzx)+(r0.wwww)).xyz;
    // 40: mad r0.xyz, cb0[4].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 41: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 42: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1126(WARLORD_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
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
    float4 r0=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1127(WARLORD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = cos(((g_WarlordSourceMaterialParameters[1u].yyyy).x*0.25));
    source[4].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
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
    r0.zw = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.y = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r1.xyz = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1128(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    r1.xyzw = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1129(WARLORD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[2].y = g_WarlordSourceMaterialTime;
    source[2].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[2].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[3].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[2].zwzz
    r0.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 2: mad r1.x, cb0[2].y, cb0[2].x, r0.x
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[2].y, cb0[3].x, r0.y
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.xyz = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[3].wwww
    r1.xyz = ((r0.xyzx)*(source[3].wwww)).xyz;
    // 12: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: mad r0.yzw, -cb0[3].wwww, r0.xxyz, r0.wwww
    r0.yzw = ((-(source[3].wwww))*(r0.xxyz)+(r0.wwww)).yzw;
    // 14: mad r0.yzw, cb0[4].xxxx, r0.yyzw, r1.xxyz
    r0.yzw = ((source[4].xxxx)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 15: mul r1.xyz, r0.yzwy, cb0[4].yyyy
    r1.xyz = ((r0.yzwy)*(source[4].yyyy)).xyz;
    // 16: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 18: mul r1.xyz, r1.xyzx, cb0[4].zzzz
    r1.xyz = ((r1.xyzx)*(source[4].zzzz)).xyz;
    // 19: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 20: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 21: add r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = ((r0.yyzw)+(r1.xxxx)).yzw;
    // 22: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 23: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 24: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 25: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 26: mul r1.x, r1.x, cb0[4].w
    r1.x = ((r1.xxxx)*(source[4].wwww)).x;
    // 27: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 28: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 29: movc r0.x, r0.x, l(0), |r1.x|
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).x;
    // 30: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 32: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 33: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 34: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 35: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 36: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 37: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1130(WARLORD_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[9u];
    source[3] = g_WarlordSourceMaterialParameters[6u];
    source[4] = g_WarlordSourceMaterialParameters[7u];
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].yyyy,g_WarlordSourceMaterialParameters[1u].zzzz,1u);
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[3u].xxxx,1u);
    source[7] = WarlordNativeAppend((g_WarlordSourceMaterialParameters[2u].yyyy*g_WarlordSourceMaterialTime.xxxx),(g_WarlordSourceMaterialParameters[2u].wwww*g_WarlordSourceMaterialTime.xxxx),1u);
    source[8] = input.dynamicParameter;
    source[9] = WarlordNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_WarlordSourceMaterialParameters[4u].wwww*g_WarlordSourceMaterialTime.xxxx),1u);
    source[10] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[4u].xxxx,g_WarlordSourceMaterialParameters[4u].yyyy,1u);
    source[13] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[14] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[15].x = ((g_WarlordSourceMaterialParameters[2u].wwww).x*g_WarlordSourceMaterialTime);
    source[15].y = ((g_WarlordSourceMaterialParameters[2u].yyyy).x*g_WarlordSourceMaterialTime);
    source[15].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[15].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[16].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[16].y = ((g_WarlordSourceMaterialParameters[4u].wwww).x*g_WarlordSourceMaterialTime);
    source[16].z = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[16].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[17].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[17].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[17].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[18].x = cos(((g_WarlordSourceMaterialParameters[3u].yyyy).x*0.25));
    source[18].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[18].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[18].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[19].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[19].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, v4.y, cb0[9].y
    r0.x = ((v4.yyyy)+(source[9].yyyy)).x;
    // 2: mul r0.x, r0.x, cb0[16].z
    r0.x = ((r0.xxxx)*(source[16].zzzz)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
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
    r0.zw = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.xyz = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    // 38: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 39: mul r0.x, v4.x, cb0[12].x
    r0.x = ((v4.xxxx)*(source[12].xxxx)).x;
    // 40: add r0.x, r0.x, cb0[18].w
    r0.x = ((r0.xxxx)+(source[18].wwww)).x;
    // 41: mad r0.z, v4.y, cb0[12].y, cb0[8].y
    r0.z = ((v4.yyyy)*(source[12].yyyy)+(source[8].yyyy)).z;
    // 42: add r0.xy, r0.xzxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 43: dp2 r1.x, cb0[13].xyxx, r0.xyxx
    r1.x = (dot((source[13].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 44: dp2 r1.y, cb0[14].xyxx, r0.xyxx
    r1.y = (dot((source[14].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 45: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: mul_sat r0.x, r0.x, cb0[19].x
    r0.x = (saturate((r0.xxxx)*(source[19].xxxx))).x;
    // 50: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 51: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1131(WARLORD_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
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
    // 7: mad r0.z, -r0.y, l(2.000000), l(1.000000)
    r0.z = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: mad r0.y, -r0.y, l(1.923077), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(1.923077,1.923077,1.923077,1.923077))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 9: mul_sat r0.y, r0.y, l(1.428571)
    r0.y = (saturate((r0.yyyy)*(float4(1.428571,1.428571,1.428571,1.428571)))).y;
    // 10: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 11: add_sat r0.z, r0.z, r0.z
    r0.z = (saturate((r0.zzzz)+(r0.zzzz))).z;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1132(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[3].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (1.0*(g_WarlordSourceMaterialParameters[1u].xxxx).x);
    source[5].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (-0.523599029*(g_WarlordSourceMaterialParameters[3u].xxxx).x);
    source[6].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[7].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[4].zwzz
    r0.xy = ((v2.xyxx)*(source[4].zwzz)).xy;
    // 2: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[4].y, r0.x
    r1.x = ((r0.zzzz)*(source[4].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[5].x, r0.y
    r1.y = ((r0.zzzz)*(source[5].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[5].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[5].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[3].w
    r0.w = ((r0.xxxx)*(source[3].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[3].z, r0.w
    r1.x = ((r0.zzzz)*(source[3].zzzz)+(r0.wwww)).x;
    // 9: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 10: mad r1.y, cb0[4].x, r0.y, r0.z
    r1.y = ((source[4].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: mad r0.w, cb0[3].y, cb0[6].y, cb0[6].z
    r0.w = ((source[3].yyyy)*(source[6].yyyy)+(source[6].zzzz)).w;
    // 14: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 15: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 16: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 17: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 18: dp2 r0.w, r3.zyzz, r0.xyxx
    r0.w = (dot((r3.zyzz).xy,(r0.xyxx).xy).xxxx).w;
    // 19: dp2 r0.x, r3.yxyy, r0.xyxx
    r0.x = (dot((r3.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 20: mul r1.z, r0.w, cb0[2].y
    r1.z = ((r0.wwww)*(source[2].yyyy)).z;
    // 21: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 22: mad r1.x, r0.x, cb0[2].x, r0.y
    r1.x = ((r0.xxxx)*(source[2].xxxx)+(r0.yyyy)).x;
    // 23: add r0.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mad r0.x, r0.z, r0.x, -r0.y
    r0.x = ((r0.zzzz)*(r0.xxxx)+(-(r0.yyyy))).x;
    // 27: mul_sat r0.x, r0.x, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy))).x;
    // 28: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)*(source[7].zzzz)).y;
    // 31: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
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
    // 41: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 43: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 44: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 45: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 46: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 47: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 48: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 49: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1134(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[7u];
    source[2] = g_WarlordSourceMaterialParameters[6u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].xxxx,g_WarlordSourceMaterialParameters[3u].yyyy,1u);
    source[5].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[5].y = g_WarlordSourceMaterialTime;
    source[5].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].z = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[7].w = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].zzzz).x));
    source[8].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].y = (1.0-(g_WarlordSourceMaterialParameters[1u].wwww).x);
    source[8].z = max((1.0-(g_WarlordSourceMaterialParameters[1u].wwww).x),9.99999975e-06);
    source[8].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[1u].wwww).x),9.99999975e-06));
    source[9].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[9].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[9].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x));
    source[9].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[10].x = (1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[10].y = max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06);
    source[10].z = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06));
    source[10].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[11].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
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
    // 28: mul r0.yz, cb0[4].xxyx, cb0[5].yyyy
    r0.yz = ((source[4].xxyx)*(source[5].yyyy)).yz;
    // 29: mul r0.w, v4.z, cb0[6].y
    r0.w = ((v4.zzzz)*(source[6].yyyy)).w;
    // 30: add r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 31: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 32: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 33: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 34: lt r1.z, r0.x, l(0.000000)
    r1.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 35: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 36: mad r0.yz, r1.xxyx, cb0[3].xxyx, r0.yyzy
    r0.yz = ((r1.xxyx)*(source[3].xxyx)+(r0.yyzy)).yz;
    // 37: mul r0.w, r1.x, cb0[5].w
    r0.w = ((r1.xxxx)*(source[5].wwww)).w;
    // 38: mul r1.x, r1.y, cb0[6].x
    r1.x = ((r1.yyyy)*(source[6].xxxx)).x;
    // 39: mul r0.yz, r0.yyzy, cb0[5].xxxx
    r0.yz = ((r0.yyzy)*(source[5].xxxx)).yz;
    // 40: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(-1.000000)
    r0.y = (WarlordNativeSample1((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 41: mad r0.z, -r0.x, cb0[7].w, l(1.000000)
    r0.z = ((-(r0.xxxx))*(source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: mad r0.x, -r0.x, cb0[9].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[9].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 44: mul_sat r0.z, r0.z, cb0[8].w
    r0.z = (saturate((r0.zzzz)*(source[8].wwww))).z;
    // 45: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 47: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 49: mul r0.x, r0.x, cb0[10].w
    r0.x = ((r0.xxxx)*(source[10].wwww)).x;
    // 50: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 51: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 52: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 53: mul_sat r0.x, r0.x, cb0[11].x
    r0.x = (saturate((r0.xxxx)*(source[11].xxxx))).x;
    // 54: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 55: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 57: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 58: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 59: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 60: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 61: log r1.y, |r0.z|
    r1.y = (log2(abs(r0.zzzz))).y;
    // 62: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 63: mul r1.y, r1.y, cb0[11].z
    r1.y = ((r1.yyyy)*(source[11].zzzz)).y;
    // 64: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 65: mul_sat r1.y, r1.y, cb0[11].w
    r1.y = (saturate((r1.yyyy)*(source[11].wwww))).y;
    // 66: movc r0.z, r0.z, l(0), r1.y
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).z;
    // 67: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 68: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 69: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 70: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 71: mul r0.y, cb0[5].x, cb0[5].y
    r0.y = ((source[5].xxxx)*(source[5].yyyy)).y;
    // 72: mad r2.x, r0.y, cb0[5].z, r0.w
    r2.x = ((r0.yyyy)*(source[5].zzzz)+(r0.wwww)).x;
    // 73: mad r2.y, r0.y, cb0[6].z, r1.x
    r2.y = ((r0.yyyy)*(source[6].zzzz)+(r1.xxxx)).y;
    // 74: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t1.wxyz, s0, l(-1.000000)
    r0.yzw = (WarlordNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 75: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 76: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 77: mad r0.yzw, cb0[6].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[6].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 78: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 79: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 80: mul r0.yzw, r0.yyzw, cb0[7].xxxx
    r0.yzw = ((r0.yyzw)*(source[7].xxxx)).yzw;
    // 81: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 82: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 83: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 84: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 85: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 86: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1135(WARLORD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].xxxx,g_WarlordSourceMaterialParameters[1u].yyyy,1u);
    source[5].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[5].y = g_WarlordSourceMaterialTime;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].x = (-0.523599029*(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[7].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1136(WARLORD_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[11u];
    source[2] = g_WarlordSourceMaterialParameters[9u];
    source[3] = g_WarlordSourceMaterialParameters[10u];
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[6u].yyyy,g_WarlordSourceMaterialParameters[6u].zzzz,1u);
    source[5].x = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[5].y = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[5].z = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[5].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_WarlordSourceMaterialParameters[8u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[8u].wwww).x;
    source[7].z = (g_WarlordSourceMaterialParameters[7u].wwww).x;
    source[7].w = (g_WarlordSourceMaterialParameters[8u].xxxx).x;
    source[8].x = (g_WarlordSourceMaterialParameters[8u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[9].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[10].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[11].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[11].w = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[12].x = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[12].y = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[12].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[12].w = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[13].y = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[13].z = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[13].w = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
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
    r0.xy = (WarlordNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
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
    r2.xyz = (WarlordNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
    r3.xyz = (WarlordNativeSample3((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
    r0.z = (WarlordNativeSample4((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
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
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1137(WARLORD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].wwww,g_WarlordSourceMaterialParameters[3u].xxxx,1u);
    source[4].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].wwww).x);
    source[4].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].wwww).x));
    source[4].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].x = (1.0-(g_WarlordSourceMaterialParameters[1u].xxxx).x);
    source[5].y = max((1.0-(g_WarlordSourceMaterialParameters[1u].xxxx).x),9.99999975e-06);
    source[5].z = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[1u].xxxx).x),9.99999975e-06));
    source[5].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].x = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[6].y = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x));
    source[6].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[7].x = max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06);
    source[7].y = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06));
    source[7].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[8].y = g_WarlordSourceMaterialTime;
    source[8].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[9].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[9].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
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
    // 28: mul r0.y, v2.x, cb0[8].w
    r0.y = ((v2.xxxx)*(source[8].wwww)).y;
    // 29: mul r0.z, cb0[8].x, cb0[8].y
    r0.z = ((source[8].xxxx)*(source[8].yyyy)).z;
    // 30: mad r2.x, r0.z, cb0[8].z, r0.y
    r2.x = ((r0.zzzz)*(source[8].zzzz)+(r0.yyyy)).x;
    // 31: mul r0.y, v2.y, cb0[9].x
    r0.y = ((v2.yyyy)*(source[9].xxxx)).y;
    // 32: mad r2.y, r0.z, cb0[9].y, r0.y
    r2.y = ((r0.zzzz)*(source[9].yyyy)+(r0.yyyy)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r2.xyxx, t0.zxyw, s1, l(0.000000)
    r0.yz = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 34: mul r0.w, v4.y, cb0[9].z
    r0.w = ((v4.yyyy)*(source[9].zzzz)).w;
    // 35: mul r1.z, v4.z, cb0[7].w
    r1.z = ((v4.zzzz)*(source[7].wwww)).z;
    // 36: add r1.w, r0.x, r0.x
    r1.w = ((r0.xxxx)+(r0.xxxx)).w;
    // 37: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 38: mul r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)*(r1.zzzz)).z;
    // 39: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 40: lt r1.w, r0.x, l(0.000000)
    r1.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 41: movc r1.y, r1.w, l(0), r1.z
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 42: mad r0.yz, r0.wwww, r0.yyzy, r1.xxyx
    r0.yz = ((r0.wwww)*(r0.yyzy)+(r1.xxyx)).yz;
    // 43: mul r1.x, r0.y, cb0[3].x
    r1.x = ((r0.yyyy)*(source[3].xxxx)).x;
    // 44: mad r2.y, r0.z, cb0[3].y, v4.x
    r2.y = ((r0.zzzz)*(source[3].yyyy)+(v4.xxxx)).y;
    // 45: mul r2.x, cb0[8].y, cb0[10].y
    r2.x = ((source[8].yyyy)*(source[10].yyyy)).x;
    // 46: mov r1.z, l(-1.000000)
    r1.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 47: add r0.yz, r1.xxzx, r2.xxyx
    r0.yz = ((r1.xxzx)+(r2.xxyx)).yz;
    // 48: mul r0.yz, r0.yyzy, cb0[8].xxxx
    r0.yz = ((r0.yyzy)*(source[8].xxxx)).yz;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(-1.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 50: mad r0.z, -r0.x, cb0[4].z, l(1.000000)
    r0.z = ((-(r0.xxxx))*(source[4].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: mad r0.x, -r0.x, cb0[6].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[6].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy))).x;
    // 53: mul_sat r0.z, r0.z, cb0[5].z
    r0.z = (saturate((r0.zzzz)*(source[5].zzzz))).z;
    // 54: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 55: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 56: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 57: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 58: mul r0.x, r0.x, cb0[7].z
    r0.x = ((r0.xxxx)*(source[7].zzzz)).x;
    // 59: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 60: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 61: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 62: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 63: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 64: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 65: mul r0.y, r0.y, cb0[10].w
    r0.y = ((r0.yyyy)*(source[10].wwww)).y;
    // 66: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 67: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 68: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 69: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 70: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 71: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 72: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 73: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 74: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1138(WARLORD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[4u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[3].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[3].zwzz
    r0.xy = ((v2.xyxx)*(source[3].zwzz)).xy;
    // 2: mad r1.x, cb0[3].y, cb0[3].x, r0.x
    r1.x = ((source[3].yyyy)*(source[3].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[3].y, cb0[4].x, r0.y
    r1.y = ((source[3].yyyy)*(source[4].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.zw, cb0[3].yyyy, cb0[2].xxxy, v2.xxxy
    r0.zw = ((source[3].yyyy)*(source[2].xxxy)+(v2.xxxy)).zw;
    // 6: mad r0.xy, cb0[4].yyyy, r0.xyxx, r0.zwzz
    r0.xy = ((source[4].yyyy)*(r0.xyxx)+(r0.zwzz)).xy;
    // 7: add r0.z, cb0[5].x, l(-1.000000)
    r0.z = ((source[5].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 8: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 9: mad r0.xy, cb0[5].xxxx, r0.xyxx, -r0.zzzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(-(r0.zzzz))).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t3.xywz, s1, l(0.000000)
    r0.xyw = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
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
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t2.xyzw, s3, l(0.000000)
    r1.x = (WarlordNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 24: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 25: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 26: mul r1.xyz, r0.xywx, cb0[5].yyyy
    r1.xyz = ((r0.xywx)*(source[5].yyyy)).xyz;
    // 27: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 28: mad r0.xyw, -cb0[5].yyyy, r0.xyxw, r1.wwww
    r0.xyw = ((-(source[5].yyyy))*(r0.xyxw)+(r1.wwww)).xyw;
    // 29: mad r0.xyw, cb0[5].zzzz, r0.xyxw, r1.xyxz
    r0.xyw = ((source[5].zzzz)*(r0.xyxw)+(r1.xyxz)).xyw;
    // 30: mul r1.xyz, r0.xywx, cb0[5].wwww
    r1.xyz = ((r0.xywx)*(source[5].wwww)).xyz;
    // 31: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 32: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[6].xxxx
    r1.xyz = ((r1.xyzx)*(source[6].xxxx)).xyz;
    // 34: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 35: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 36: add r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)+(r1.xxxx)).xyw;
    // 37: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 38: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 39: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 40: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1139(WARLORD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = WarlordNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].y = g_WarlordSourceMaterialTime;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].x = (-1.0*sin(((-0.523599029*(g_WarlordSourceMaterialParameters[2u].zzzz).x)*1.0)));
    source[5].y = cos(((-0.523599029*(g_WarlordSourceMaterialParameters[2u].zzzz).x)*1.0));
    source[5].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[6].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[6].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[8].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[8].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[9].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[9].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f) /* Native VS fog varying: project no-fog adapter RGB=0, transmission=1. */; // native texcoord4
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
    // 5: mul r0.z, r0.x, cb0[5].w
    r0.z = ((r0.xxxx)*(source[5].wwww)).z;
    // 6: mad r1.x, cb0[4].y, cb0[5].z, r0.z
    r1.x = ((source[4].yyyy)*(source[5].zzzz)+(r0.zzzz)).x;
    // 7: mul r0.z, cb0[4].y, cb0[6].y
    r0.z = ((source[4].yyyy)*(source[6].yyyy)).z;
    // 8: mad r1.y, cb0[6].x, r0.y, r0.z
    r1.y = ((source[6].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 10: mad r0.zw, cb0[6].zzzz, r0.zzzw, r0.xxxy
    r0.zw = ((source[6].zzzz)*(r0.zzzw)+(r0.xxxy)).zw;
    // 11: mul r1.xy, r0.zwzz, cb0[4].zwzz
    r1.xy = ((r0.zwzz)*(source[4].zwzz)).xy;
    // 12: mul r0.zw, r0.zzzw, cb0[7].yyyz
    r0.zw = ((r0.zzzw)*(source[7].yyyz)).zw;
    // 13: mad r0.zw, cb0[4].yyyy, cb0[7].xxxw, r0.zzzw
    r0.zw = ((source[4].yyyy)*(source[7].xxxw)+(r0.zzzw)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: mad r2.x, cb0[4].y, cb0[4].x, r1.x
    r2.x = ((source[4].yyyy)*(source[4].xxxx)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[4].y, cb0[6].w, r1.y
    r2.y = ((source[4].yyyy)*(source[6].wwww)+(r1.yyyy)).y;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: add r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)+(r0.wwww)).z;
    // 19: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[8].x
    r0.w = ((r0.wwww)*(source[8].xxxx)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 25: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 26: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 27: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 28: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 29: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 30: lt r1.x, |r0.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 32: add r1.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 33: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 34: mul_sat r0.y, r1.y, cb0[9].z
    r0.y = (saturate((r1.yyyy)*(source[9].zzzz))).y;
    // 35: mul r1.x, r0.x, l(4.000000)
    r1.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 36: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 37: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 38: mul r1.x, r1.x, cb0[8].z
    r1.x = ((r1.xxxx)*(source[8].zzzz)).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: mul r1.x, r1.x, cb0[8].w
    r1.x = ((r1.xxxx)*(source[8].wwww)).x;
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
    // 46: mul r0.z, r0.z, cb0[9].w
    r0.z = ((r0.zzzz)*(source[9].wwww)).z;
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
    // 52: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1140(WARLORD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2] = g_WarlordSourceMaterialParameters[2u];
    source[3].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[3].xxxx
    r0.xy = ((v2.xyxx)*(source[3].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 3: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 4: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 5: mad r0.xyz, cb0[3].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[3].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 6: mul r0.w, r0.w, cb0[3].z
    r0.w = ((r0.wwww)*(source[3].zzzz)).w;
    // 7: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 8: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 10: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 11: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 12: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 13: mul r0.x, r0.x, cb0[3].w
    r0.x = ((r0.xxxx)*(source[3].wwww)).x;
    // 14: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 15: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 16: mul r0.yz, v2.xxyx, cb0[4].xxxx
    r0.yz = ((v2.xxyx)*(source[4].xxxx)).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 18: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 19: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 20: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 21: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1141(WARLORD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = g_WarlordSourceMaterialParameters[4u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].yyyy,g_WarlordSourceMaterialParameters[0u].zzzz,1u);
    source[4] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_WarlordSourceMaterialParameters[3u];
    source[6].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].x = ((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x)*0.0);
    source[7].y = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x)*0.0))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x)*0.0))));
    source[7].z = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x)*1.0))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x)*1.0))));
    source[7].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[8].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[8].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
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
    // 2: mad r0.xy, r0.xyxx, cb0[3].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(source[3].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.yxzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 4: mul r0.yz, v2.xxyx, cb0[6].xxxx
    r0.yz = ((v2.xxyx)*(source[6].xxxx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mad r0.yz, r1.xxyx, l(0.000000, 0.200000, 0.200000, 0.000000), v2.xxyx
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.200000,0.200000,0.000000))+(v2.xxyx)).yz;
    // 7: add r0.yz, r0.yyzy, cb0[4].xxyx
    r0.yz = ((r0.yyzy)+(source[4].xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 9: add r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)+(r0.xxxx)).y;
    // 10: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 11: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 12: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, cb0[7].w
    r0.y = ((r0.yyyy)*(source[7].wwww)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 16: mul r0.yzw, r0.yyyy, r2.xxyz
    r0.yzw = ((r0.yyyy)*(r2.xxyz)).yzw;
    // 17: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 18: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 19: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 21: mad r1.xyz, cb0[6].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[6].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 22: mul r0.w, r1.w, cb0[8].x
    r0.w = ((r1.wwww)*(source[8].xxxx)).w;
    // 23: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 24: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 25: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 26: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 27: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 28: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 29: mul r0.x, r0.x, cb0[8].y
    r0.x = ((r0.xxxx)*(source[8].yyyy)).x;
    // 30: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 31: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 32: mul r0.yz, v2.xxyx, cb0[8].zzzz
    r0.yz = ((v2.xxyx)*(source[8].zzzz)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 34: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 35: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 36: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 37: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 39: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1142(WARLORD_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.0149999997, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.0209999997, 0.0, 0.0, 0.0))),1u);
    source[4] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.0109999999, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[5] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialParameters[0u].zzzz*g_WarlordSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialParameters[0u].zzzz*g_WarlordSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))),1u);
    source[6].x = (0.00100000005*(g_WarlordSourceMaterialParameters[3u].zzzz).x);
    source[6].y = (sign((g_WarlordSourceMaterialTime*-0.0209999997))*frac(abs((g_WarlordSourceMaterialTime*-0.0209999997))));
    source[6].z = (sign((g_WarlordSourceMaterialTime*-0.0149999997))*frac(abs((g_WarlordSourceMaterialTime*-0.0149999997))));
    source[6].w = (g_WarlordSourceMaterialTime*-0.0109999999);
    source[7].x = (g_WarlordSourceMaterialTime*0.0199999996);
    source[7].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[7].z = (0.00100000005*(g_WarlordSourceMaterialParameters[3u].wwww).x);
    source[7].w = (sign((g_WarlordSourceMaterialTime*0.0199999996))*frac(abs((g_WarlordSourceMaterialTime*0.0199999996))));
    source[8].x = (sign((g_WarlordSourceMaterialTime*-0.0109999999))*frac(abs((g_WarlordSourceMaterialTime*-0.0109999999))));
    source[8].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[8].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[8].w = (0.00100000005*(g_WarlordSourceMaterialParameters[4u].xxxx).x);
    source[9].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[9].z = ((g_WarlordSourceMaterialParameters[0u].zzzz).x*g_WarlordSourceMaterialTime);
    source[9].w = (((g_WarlordSourceMaterialParameters[0u].zzzz).x*g_WarlordSourceMaterialTime)*0.0);
    source[10].x = (((g_WarlordSourceMaterialParameters[0u].zzzz).x*g_WarlordSourceMaterialTime)*-0.200000003);
    source[10].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (sign((((g_WarlordSourceMaterialParameters[0u].zzzz).x*g_WarlordSourceMaterialTime)*-0.200000003))*frac(abs((((g_WarlordSourceMaterialParameters[0u].zzzz).x*g_WarlordSourceMaterialTime)*-0.200000003))));
    source[11].x = (sign((((g_WarlordSourceMaterialParameters[0u].zzzz).x*g_WarlordSourceMaterialTime)*0.0))*frac(abs((((g_WarlordSourceMaterialParameters[0u].zzzz).x*g_WarlordSourceMaterialTime)*0.0))));
    source[11].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[11].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[12].x = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[12].y = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x));
    source[12].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[12].w = (-1.0*(g_WarlordSourceMaterialParameters[1u].wwww).x);
    source[13].x = (1.0-(-1.0*(g_WarlordSourceMaterialParameters[1u].wwww).x));
    source[13].y = max((1.0-(-1.0*(g_WarlordSourceMaterialParameters[1u].wwww).x)),9.99999975e-06);
    source[13].z = (1.0/max((1.0-(-1.0*(g_WarlordSourceMaterialParameters[1u].wwww).x)),9.99999975e-06));
    source[13].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[14].x = (0.00100000005*(g_WarlordSourceMaterialParameters[3u].yyyy).x);
    source[14].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[14].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[15].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    r1.xw = (WarlordNativeSample0((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzwy).xw;
    // 50: mad r2.xy, cb0[7].zzzz, r1.yzyy, cb0[4].xyxx
    r2.xy = ((source[7].zzzz)*(r1.yzyy)+(source[4].xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 52: mul r1.xw, r1.xxxw, r2.xxxy
    r1.xw = ((r1.xxxw)*(r2.xxxy)).xw;
    // 53: mad r0.zw, cb0[9].xxxx, r1.xxxw, r0.zzzw
    r0.zw = ((source[9].xxxx)*(r1.xxxw)+(r0.zzzw)).zw;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 55: mul r0.w, r0.z, cb0[11].y
    r0.w = ((r0.zzzz)*(source[11].yyyy)).w;
    // 56: mad r2.yz, cb0[8].wwww, r1.yyzy, r0.wwww
    r2.yz = ((source[8].wwww)*(r1.yyzy)+(r0.wwww)).yz;
    // 57: mul r1.yz, r1.yyzy, cb0[14].xxxx
    r1.yz = ((r1.yyzy)*(source[14].xxxx)).yz;
    // 58: mad r1.yz, r1.xxwx, l(0.000000, 0.200000, 0.200000, 0.000000), r1.yyzy
    r1.yz = ((r1.xxwx)*(float4(0.000000,0.200000,0.200000,0.000000))+(r1.yyzy)).yz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r1.yzyy, t3.xyzw, s3, l(0.000000)
    r4.xyz = (WarlordNativeSample3((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r2.yzyy, t2.zxyw, s2, l(0.000000)
    r1.yz = (WarlordNativeSample2((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
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
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1143(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0]=float4(1.f,1.f,1.f,1.f); // Native mesh RGB multiplier and dither opacity prefix: neutral color and fully present coverage.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[7u];
    source[2] = WarlordNativeAppend(WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].zzzz,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].zzzz,g_WarlordSourceMaterialParameters[1u].yyyy,1u);
    source[4] = g_WarlordSourceMaterialParameters[4u];
    source[5] = input.dynamicParameter;
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].yyyy,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[7] = g_WarlordSourceMaterialParameters[5u];
    source[8].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].y = g_WarlordSourceMaterialTime;
    source[8].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[8].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[9].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[9].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[10].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[11].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((v6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 2: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 3: mov_sat r0.z, cb0[0].w
    r0.z = (saturate(source[0].wwww)).z;
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
    // 12: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 13: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 14: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 16: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 17: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 18: mul r0.x, r0.x, v5.z
    r0.x = ((r0.xxxx)*(v5.zzzz)).x;
    // 19: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 21: mad r0.yz, r0.yyzy, cb0[3].xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)*(source[3].xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.xyzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 23: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 24: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 25: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 26: mul r0.y, r0.y, cb0[10].x
    r0.y = ((r0.yyyy)*(source[10].xxxx)).y;
    // 27: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 28: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 29: mul r0.yz, v4.xxyx, cb0[6].xxyx
    r0.yz = ((v4.xxyx)*(source[6].xxyx)).yz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s2, l(0.000000)
    r0.yzw = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 31: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 32: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 33: mad r0.yzw, cb0[10].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[10].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 34: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 35: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 36: mul r0.yzw, r0.yyzw, cb0[11].xxxx
    r0.yzw = ((r0.yyzw)*(source[11].xxxx)).yzw;
    // 37: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 38: mul r1.xyz, cb0[7].xyzx, cb0[7].wwww
    r1.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 39: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 40: mul r1.xy, v4.xyxx, cb0[8].zwzz
    r1.xy = ((v4.xyxx)*(source[8].zwzz)).xy;
    // 41: mad r2.x, cb0[8].y, cb0[8].x, r1.x
    r2.x = ((source[8].yyyy)*(source[8].xxxx)+(r1.xxxx)).x;
    // 42: mad r2.y, cb0[8].y, cb0[9].x, r1.y
    r2.y = ((source[8].yyyy)*(source[9].xxxx)+(r1.yyyy)).y;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 44: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 46: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: add r1.z, r1.w, l(0.000010)
    r1.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 50: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 51: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 52: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 53: mad r2.x, v1.z, r1.w, l(1.000000)
    r2.x = ((v1.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul r2.yzw, r1.wwww, v1.xxyz
    r2.yzw = ((r1.wwww)*(v1.xxyz)).yzw;
    // 55: mul_sat r1.w, r2.x, l(0.500000)
    r1.w = (saturate((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 56: mad r1.w, r1.w, l(0.950000), l(0.050000)
    r1.w = ((r1.wwww)*(float4(0.950000,0.950000,0.950000,0.950000))+(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 57: dp3 r1.w, r1.xyzx, r1.wwww
    r1.w = (dot((r1.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 58: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 59: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 60: mul r2.x, r2.x, cb0[11].y
    r2.x = ((r2.xxxx)*(source[11].yyyy)).x;
    // 61: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 62: mul r2.x, r2.x, cb0[11].z
    r2.x = ((r2.xxxx)*(source[11].zzzz)).x;
    // 63: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 64: mul r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = ((r0.yyzw)*(r1.wwww)).yzw;
    // 65: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 66: mul r3.xyz, r3.xyzx, cb0[5].xxxx
    r3.xyz = ((r3.xyzx)*(source[5].xxxx)).xyz;
    // 67: mad r0.xyz, r0.xxxx, r3.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 68: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 69: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1144(WARLORD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[7u];
    source[2] = g_WarlordSourceMaterialParameters[6u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].zzzz,g_WarlordSourceMaterialParameters[3u].wwww,1u);
    source[4].x = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[4].y = g_WarlordSourceMaterialTime;
    source[4].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (1.0*(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[6].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[8].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[8].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[9].x = (-0.523599029*(g_WarlordSourceMaterialParameters[4u].xxxx).x);
    source[9].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[10].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[10].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
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
    // 10: add r0.y, -cb0[10].y, l(1.000000)
    r0.y = ((-(source[10].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 14: mad r0.z, cb0[4].y, cb0[8].w, cb0[9].x
    r0.z = ((source[4].yyyy)*(source[8].wwww)+(source[9].xxxx)).z;
    // 15: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 16: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 17: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 18: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 19: mul r0.zw, v2.xxxy, cb0[5].zzzw
    r0.zw = ((v2.xxxy)*(source[5].zzzw)).zw;
    // 20: mul r1.x, cb0[4].x, cb0[4].y
    r1.x = ((source[4].xxxx)*(source[4].yyyy)).x;
    // 21: mad r2.x, r1.x, cb0[5].y, r0.z
    r2.x = ((r1.xxxx)*(source[5].yyyy)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[6].x, r0.w
    r2.y = ((r1.xxxx)*(source[6].xxxx)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 24: mad r0.zw, cb0[6].zzzz, r0.zzzw, v2.xxxy
    r0.zw = ((source[6].zzzz)*(r0.zzzw)+(v2.xxxy)).zw;
    // 25: add r1.yz, r0.zzwz, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.zzwz)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 26: dp2 r1.w, r3.yxyy, r1.yzyy
    r1.w = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).w;
    // 27: dp2 r1.y, r3.zyzz, r1.yzyy
    r1.y = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).y;
    // 28: mul r2.z, r1.y, cb0[3].y
    r2.z = ((r1.yyyy)*(source[3].yyyy)).z;
    // 29: mad r2.x, r1.w, cb0[3].x, r0.y
    r2.x = ((r1.wwww)*(source[3].xxxx)+(r0.yyyy)).x;
    // 30: add r1.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t2.wxyz, s4, l(0.000000)
    r1.yzw = (WarlordNativeSample3((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 32: mul r0.y, r0.z, cb0[7].w
    r0.y = ((r0.zzzz)*(source[7].wwww)).y;
    // 33: mad r2.x, r1.x, cb0[7].z, r0.y
    r2.x = ((r1.xxxx)*(source[7].zzzz)+(r0.yyyy)).x;
    // 34: mul r0.y, r0.w, cb0[8].x
    r0.y = ((r0.wwww)*(source[8].xxxx)).y;
    // 35: mad r2.y, r1.x, cb0[8].y, r0.y
    r2.y = ((r1.xxxx)*(source[8].yyyy)+(r0.yyyy)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s3, l(0.000000)
    r2.xyz = (WarlordNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 37: mul r1.yz, r1.yyzy, r2.xxyx
    r1.yz = ((r1.yyzy)*(r2.xxyx)).yz;
    // 38: add r0.y, r1.z, r1.y
    r0.y = ((r1.zzzz)+(r1.yyyy)).y;
    // 39: mad r0.y, r2.z, r1.w, r0.y
    r0.y = ((r2.zzzz)*(r1.wwww)+(r0.yyyy)).y;
    // 40: add r1.y, -v4.x, l(1.000000)
    r1.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: mad r0.y, r0.y, l(0.333330), -r1.y
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))+(-(r1.yyyy))).y;
    // 42: mul_sat r0.y, r0.y, cb0[9].w
    r0.y = (saturate((r0.yyyy)*(source[9].wwww))).y;
    // 43: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 44: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r1.y, r1.y, cb0[10].x
    r1.y = ((r1.yyyy)*(source[10].xxxx)).y;
    // 46: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 47: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 48: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 49: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 50: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 51: mul r0.y, r0.z, cb0[4].w
    r0.y = ((r0.zzzz)*(source[4].wwww)).y;
    // 52: mad r2.x, r1.x, cb0[4].z, r0.y
    r2.x = ((r1.xxxx)*(source[4].zzzz)+(r0.yyyy)).x;
    // 53: mul r0.y, r1.x, cb0[6].w
    r0.y = ((r1.xxxx)*(source[6].wwww)).y;
    // 54: mad r2.y, cb0[5].x, r0.w, r0.y
    r2.y = ((source[5].xxxx)*(r0.wwww)+(r0.yyyy)).y;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t4.wxyz, s2, l(0.000000)
    r0.yzw = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 56: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 57: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 58: mad r0.yzw, cb0[7].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 59: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 60: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 61: mul r0.yzw, r0.yyzw, cb0[7].yyyy
    r0.yzw = ((r0.yyzw)*(source[7].yyyy)).yzw;
    // 62: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 63: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 64: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 65: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 66: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 67: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 68: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1145(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    r1.xyzw = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
    // 29: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 31: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 32: mul r0.xyz, r1.xyzx, cb0[2].yyyy
    r0.xyz = ((r1.xyzx)*(source[2].yyyy)).xyz;
    // 33: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: mad r1.xyz, -cb0[2].yyyy, r1.xyzx, r0.wwww
    r1.xyz = ((-(source[2].yyyy))*(r1.xyzx)+(r0.wwww)).xyz;
    // 35: mad r0.xyz, cb0[2].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[2].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 36: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 37: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1146(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].y = g_WarlordSourceMaterialTime;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = ((g_WarlordSourceMaterialParameters[0u].zzzz).x*3.5);
    source[3].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xyzw, v2.xyxy, l(0.500000, 0.500000, 0.700000, 0.700000)
    r0.xyzw = ((v2.xyxy)*(float4(0.500000,0.500000,0.700000,0.700000))).xyzw;
    // 2: mad r0.xy, v4.wwww, l(0.100000, -0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v4.wwww)*(float4(0.100000,-0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, r0.xxxx, l(0.000000, 0.400000, 0.400000, 0.000000), r0.zzwz
    r0.yz = ((r0.xxxx)*(float4(0.000000,0.400000,0.400000,0.000000))+(r0.zzwz)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s4, l(0.000000)
    r0.w = (WarlordNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: add r0.z, r0.w, r0.w
    r0.z = ((r0.wwww)+(r0.wwww)).z;
    // 8: mul r0.w, r0.x, l(0.200000)
    r0.w = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 9: add r1.xy, r0.xxxx, v2.xyxx
    r1.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t5.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.xy, v2.xyxx, l(0.950000, 0.950000, 0.000000, 0.000000), r0.wwww
    r1.xy = ((v2.xyxx)*(float4(0.950000,0.950000,0.000000,0.000000))+(r0.wwww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (WarlordNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r1.z = (WarlordNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.x = (WarlordNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1147(WARLORD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = g_WarlordSourceMaterialParameters[4u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].zzzz,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[4] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_WarlordSourceMaterialParameters[3u];
    source[6].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[2u].xxxx).x)*1.0))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[2u].xxxx).x)*1.0))));
    source[7].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 2: add r0.x, r0.x, -cb0[6].y
    r0.x = ((r0.xxxx)+(-(source[6].yyyy))).x;
    // 3: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 4: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 5: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 6: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 7: mul r0.yz, r0.yyyy, v6.xxyx
    r0.yz = ((r0.yyyy)*(v6.xxyx)).yz;
    // 8: mul r1.xy, r0.yzyy, r0.xxxx
    r1.xy = ((r0.yzyy)*(r0.xxxx)).xy;
    // 9: mad_sat r0.xy, r0.xxxx, r0.yzyy, v2.xyxx
    r0.xy = (saturate((r0.xxxx)*(r0.yzyy)+(v2.xyxx))).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mul r0.x, r0.x, cb0[7].z
    r0.x = ((r0.xxxx)*(source[7].zzzz)).x;
    // 12: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 13: mad r0.yz, r0.yyzy, cb0[3].xxyx, r1.xxyx
    r0.yz = ((r0.yyzy)*(source[3].xxyx)+(r1.xxyx)).yz;
    // 14: mad r1.xy, cb0[6].xxxx, v2.xyxx, r1.xyxx
    r1.xy = ((source[6].xxxx)*(v2.xyxx)+(r1.xyxx)).xy;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 16: add r0.yz, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.xyzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 18: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.200000, 0.200000), v2.xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.200000,0.200000))+(v2.xxxy)).zw;
    // 19: add r0.zw, r0.zzzw, cb0[4].xxxy
    r0.zw = ((r0.zzzw)+(source[4].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: add r0.z, r0.z, r0.y
    r0.z = ((r0.zzzz)+(r0.yyyy)).z;
    // 22: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 23: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 24: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: mul r0.z, r0.z, cb0[7].y
    r0.z = ((r0.zzzz)*(source[7].yyyy)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 28: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 29: mul r2.xyz, r2.xyzx, v3.xyzx
    r2.xyz = ((r2.xyzx)*(v3.xyzx)).xyz;
    // 30: movc r0.yzw, r0.yyyy, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 31: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 33: mad r1.xyz, cb0[6].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[6].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 34: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 35: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 36: mad r0.yzw, r1.xxyz, v3.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(v3.xxyz)+(r0.yyzw)).yzw;
    // 37: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 38: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 39: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 40: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 41: mul r0.y, r0.y, cb0[7].w
    r0.y = ((r0.yyyy)*(source[7].wwww)).y;
    // 42: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 43: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 44: mul r0.yz, v2.xxyx, cb0[8].xxxx
    r0.yz = ((v2.xxyx)*(source[8].xxxx)).yz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (WarlordNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 46: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 47: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 48: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 49: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 50: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 51: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1148(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2] = g_WarlordSourceMaterialParameters[2u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].wwww,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].zzzz,g_WarlordSourceMaterialParameters[1u].wwww,1u);
    source[5].x = g_WarlordSourceMaterialTime;
    source[5].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
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
    r0.xy = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1149(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.x, cb0[2].x, v4.x, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.x, cb0[2].x
    r0.y = ((v4.xxxx)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 6: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 7: mul r0.w, r0.w, cb0[2].w
    r0.w = ((r0.wwww)*(source[2].wwww)).w;
    // 8: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 9: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 10: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: movc r0.w, r1.x, l(0), |r0.w|
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.wwww))).w;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1150(WARLORD_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[8u];
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = WarlordNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = WarlordNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].y = g_WarlordSourceMaterialTime;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (-1.0*sin(((-0.523599029*(g_WarlordSourceMaterialParameters[2u].wwww).x)*1.0)));
    source[6].y = cos(((-0.523599029*(g_WarlordSourceMaterialParameters[2u].wwww).x)*1.0));
    source[6].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[7].x = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[7].y = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[7].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[8].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[9].z = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[9].w = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[10].x = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[10].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[10].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[11].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[12].y = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x));
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
    // 5: add r0.zw, -r0.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 6: mul_sat r0.z, r0.x, r0.z
    r0.z = (saturate((r0.xxxx)*(r0.zzzz))).z;
    // 7: mul_sat r0.w, r0.w, cb0[11].y
    r0.w = (saturate((r0.wwww)*(source[11].yyyy))).w;
    // 8: mul r1.x, r0.z, l(4.000000)
    r1.x = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 9: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 10: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 11: mul r1.x, r1.x, cb0[10].y
    r1.x = ((r1.xxxx)*(source[10].yyyy)).x;
    // 12: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 13: mul r1.x, r1.x, cb0[10].z
    r1.x = ((r1.xxxx)*(source[10].zzzz)).x;
    // 14: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 15: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 16: mul r1.x, r1.x, cb0[10].w
    r1.x = ((r1.xxxx)*(source[10].wwww)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[11].x
    r1.x = ((r1.xxxx)*(source[11].xxxx)).x;
    // 19: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 21: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 22: mul r1.x, r0.x, cb0[6].w
    r1.x = ((r0.xxxx)*(source[6].wwww)).x;
    // 23: mad r1.x, cb0[5].y, cb0[6].z, r1.x
    r1.x = ((source[5].yyyy)*(source[6].zzzz)+(r1.xxxx)).x;
    // 24: mul r1.z, cb0[5].y, cb0[7].y
    r1.z = ((source[5].yyyy)*(source[7].yyyy)).z;
    // 25: mad r1.y, cb0[7].x, r0.y, r1.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r1.zzzz)).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 27: mad r0.xy, cb0[7].zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((source[7].zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 28: mul r1.xy, r0.xyxx, cb0[8].yzyy
    r1.xy = ((r0.xyxx)*(source[8].yzyy)).xy;
    // 29: mul r0.xy, r0.xyxx, cb0[5].zwzz
    r0.xy = ((r0.xyxx)*(source[5].zwzz)).xy;
    // 30: mad r1.xy, cb0[5].yyyy, cb0[8].xwxx, r1.xyxx
    r1.xy = ((source[5].yyyy)*(source[8].xwxx)+(r1.xyxx)).xy;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyz = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: mad r2.x, cb0[5].y, cb0[5].x, r0.x
    r2.x = ((source[5].yyyy)*(source[5].xxxx)+(r0.xxxx)).x;
    // 33: mad r2.y, cb0[5].y, cb0[7].w, r0.y
    r2.y = ((source[5].yyyy)*(source[7].wwww)+(r0.yyyy)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyz = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 35: add r1.xyzw, r1.xxyz, r2.xxyz
    r1.xyzw = ((r1.xxyz)+(r2.xxyz)).xyzw;
    // 36: mul r2.xyzw, r1.xyzw, l(0.500000, 0.500000, 0.500000, 0.500000)
    r2.xyzw = ((r1.xyzw)*(float4(0.500000,0.500000,0.500000,0.500000))).xyzw;
    // 37: log r0.x, |r2.x|
    r0.x = (log2(abs(r2.xxxx))).x;
    // 38: mul r0.x, r0.x, cb0[9].w
    r0.x = ((r0.xxxx)*(source[9].wwww)).x;
    // 39: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 40: mul r0.x, r0.x, cb0[10].x
    r0.x = ((r0.xxxx)*(source[10].xxxx)).x;
    // 41: lt r0.y, |r2.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 42: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 43: mul_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)*(r0.xxxx))).x;
    // 44: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 45: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 46: source device depth mapped to centimetre view depth; reconstruction at 48.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 48-51: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 52: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 53: add r0.z, -cb0[12].y, l(1.000000)
    r0.z = ((-(source[12].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 54: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 55: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 56: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 57: log r0.z, r0.w
    r0.z = (log2(r0.wwww)).z;
    // 58: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 59: mul r0.z, r0.z, cb0[11].z
    r0.z = ((r0.zzzz)*(source[11].zzzz)).z;
    // 60: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 61: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 62: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 64: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 65: movc o0.w, r0.w, l(0), r0.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 66: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 67: mad r0.xyz, -r1.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r0.xxxx
    r0.xyz = ((-(r1.yzwy))*(float4(0.500000,0.500000,0.500000,0.000000))+(r0.xxxx)).xyz;
    // 68: mad r0.xyz, cb0[9].xxxx, r0.xyzx, r2.yzwy
    r0.xyz = ((source[9].xxxx)*(r0.xyzx)+(r2.yzwy)).xyz;
    // 69: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 70: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 71: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 72: mul r0.xyz, r0.xyzx, cb0[9].zzzz
    r0.xyz = ((r0.xyzx)*(source[9].zzzz)).xyz;
    // 73: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 74: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 75: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 76: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 77: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1151(WARLORD_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[8u];
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3].x = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[3].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[4].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[4].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[5].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[7].y = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].w = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[2u].xxxx).x);
    source[8].x = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[2u].xxxx).x));
    source[8].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (1.0-(g_WarlordSourceMaterialParameters[2u].yyyy).x);
    source[8].w = max((1.0-(g_WarlordSourceMaterialParameters[2u].yyyy).x),9.99999975e-06);
    source[9].x = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[2u].yyyy).x),9.99999975e-06));
    source[9].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[9].z = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[9].w = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x));
    source[10].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[10].y = (1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x);
    source[10].z = max((1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x),9.99999975e-06);
    source[10].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x),9.99999975e-06));
    source[11].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[11].y = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
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
    r2.xy = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.yzw = (WarlordNativeSample2((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(-1.000000)
    r1.xyz = (WarlordNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
#endif
