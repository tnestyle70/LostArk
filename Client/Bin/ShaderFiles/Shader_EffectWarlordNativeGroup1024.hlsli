// Warlord recovered source programs. Carrier filtering preserves the selected VF.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1024(WARLORD_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[7u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].wwww,g_WarlordSourceMaterialParameters[3u].zzzz,1u);
    source[3] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].yyyy,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[6].x = cos(((g_WarlordSourceMaterialParameters[1u].wwww).x*0.25));
    source[6].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[7].y = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[8].y = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[8].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[8].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[9].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[10].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[11].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
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
    r0.z = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mov r1.xz, r0.zzzz
    r1.xz = (r0.zzzz).xz;
    // 8: mul r2.xyzw, v2.xyxy, cb0[8].xyzw
    r2.xyzw = ((v2.xyxy)*(source[8].xyzw)).xyzw;
    // 9: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, -0.050000, -0.050000), r2.xxxy
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,-0.050000,-0.050000))+(r2.xxxy)).zw;
    // 10: mad r2.xy, v4.xxxx, l(0.080000, 3.000000, 0.000000, 0.000000), r2.zwzz
    r2.xy = ((v4.xxxx)*(float4(0.080000,3.000000,0.000000,0.000000))+(r2.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.x = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.xzyw, s0, l(0.000000)
    r0.z = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
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
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 26: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 27: mul r0.yz, v2.xxyx, cb0[11].zzwz
    r0.yz = ((v2.xxyx)*(source[11].zzwz)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1025(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[4u];
    source[2].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[2].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[2].w = g_WarlordSourceMaterialTime;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[4].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[4].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(source[3].xyxx)).xy;
    // 2: mad r1.x, cb0[2].w, cb0[2].z, r0.x
    r1.x = ((source[2].wwww)*(source[2].zzzz)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[2].w, cb0[3].z, r0.y
    r1.y = ((source[2].wwww)*(source[3].zzzz)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.yz, v2.xxyx, cb0[4].xxyx
    r0.yz = ((v2.xxyx)*(source[4].xxyx)).yz;
    // 6: mad r1.x, cb0[2].w, cb0[3].w, r0.y
    r1.x = ((source[2].wwww)*(source[3].wwww)+(r0.yyyy)).x;
    // 7: mad r1.y, cb0[2].w, cb0[4].z, r0.z
    r1.y = ((source[2].wwww)*(source[4].zzzz)+(r0.zzzz)).y;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 9: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 10: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 11: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)*(source[4].wwww)).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: mul r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)*(source[5].xxxx)).y;
    // 15: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 16: add r0.y, v2.x, l(-0.500000)
    r0.y = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 17: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 18: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 19: mul r0.z, r0.y, v2.y
    r0.z = ((r0.yyyy)*(v2.yyyy)).z;
    // 20: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 21: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 22: sqrt r0.z, v2.y
    r0.z = (sqrt(v2.yyyy)).z;
    // 23: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 24: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 25: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 27: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 28: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 29: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 30: mad_sat r0.x, r0.y, cb0[2].y, r0.x
    r0.x = (saturate((r0.yyyy)*(source[2].yyyy)+(r0.xxxx))).x;
    // 31: add r0.y, -v2.y, l(1.000000)
    r0.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: mul_sat r0.y, r0.y, cb0[5].y
    r0.y = (saturate((r0.yyyy)*(source[5].yyyy))).y;
    // 33: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 34: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 35: mul r0.z, r0.z, cb0[5].z
    r0.z = ((r0.zzzz)*(source[5].zzzz)).z;
    // 36: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 37: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 38: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 39: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 40: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 41: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 42: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 43: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 44: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 45: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 46: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 47: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 48: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 49: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 50: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1027(WARLORD_NATIVE_INPUT input)
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
    // 1: mul r0.xy, v2.xyxx, cb0[4].yzyy
    r0.xy = ((v2.xyxx)*(source[4].yzyy)).xy;
    // 2: mad r1.x, cb0[4].x, cb0[3].w, r0.x
    r1.x = ((source[4].xxxx)*(source[3].wwww)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[4].x, cb0[4].w, r0.y
    r1.y = ((source[4].xxxx)*(source[4].wwww)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, cb0[5].xxxx, r0.xyxx, v2.xyxx
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(v2.xyxx)).xy;
    // 6: mad r0.z, cb0[5].y, v4.z, l(-1.000000)
    r0.z = ((source[5].yyyy)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 7: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 8: mul r0.w, v4.z, cb0[5].y
    r0.w = ((v4.zzzz)*(source[5].yyyy)).w;
    // 9: mad r0.xy, r0.wwww, r0.xyxx, -r0.zzzz
    r0.xy = ((r0.wwww)*(r0.xyxx)+(-(r0.zzzz))).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: mul r0.yzw, r1.xxyz, cb0[5].zzzz
    r0.yzw = ((r1.xxyz)*(source[5].zzzz)).yzw;
    // 13: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 14: mad r1.xyz, -cb0[5].zzzz, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[5].zzzz))*(r1.xyzx)+(r1.wwww)).xyz;
    // 15: mad r0.yzw, cb0[5].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[5].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 16: mul r1.xyz, r0.yzwy, cb0[6].xxxx
    r1.xyz = ((r0.yzwy)*(source[6].xxxx)).xyz;
    // 17: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 18: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 19: mul r1.xyz, r1.xyzx, cb0[6].yyyy
    r1.xyz = ((r1.xyzx)*(source[6].yyyy)).xyz;
    // 20: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 21: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 22: add r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = ((r0.yyzw)+(r1.xxxx)).yzw;
    // 23: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t2.xyzw, s0, l(0.000000)
    r1.x = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: mul r1.x, r1.x, cb0[3].x
    r1.x = ((r1.xxxx)*(source[3].xxxx)).x;
    // 26: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 27: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r1.y, r1.y, cb0[3].y
    r1.y = ((r1.yyyy)*(source[3].yyyy)).y;
    // 29: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 30: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 31: mad r1.y, v4.x, l(2.000000), l(-1.000000)
    r1.y = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 32: add r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 33: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 34: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 35: mul r1.y, r1.y, cb0[3].z
    r1.y = ((r1.yyyy)*(source[3].zzzz)).y;
    // 36: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 37: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 39: ge r1.y, l(0.250000), r1.x
    r1.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r1.xxxx)) * 0xffffffffu)).y;
    // 40: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 41: movc r0.yzw, r1.yyyy, r2.xxyz, r0.yyzw
    r0.yzw = ((asuint(r1.yyyy) != 0u) ? (r2.xxyz) : (r0.yyzw)).yzw;
    // 42: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 43: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 44: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 45: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 46: mul r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)*(source[6].zzzz)).y;
    // 47: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 48: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 49: movc r0.x, r0.x, l(0), |r0.y|
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).x;
    // 50: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 51: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 52: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 53: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 54: mul r0.y, r1.x, r0.y
    r0.y = ((r1.xxxx)*(r0.yyyy)).y;
    // 55: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 56: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 57: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1028(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[4u];
    source[2] = g_WarlordSourceMaterialParameters[3u];
    source[3].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[5].y, l(1.000000)
    r0.y = ((-(source[5].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 15: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 16: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 17: mul r0.z, r0.z, cb0[3].y
    r0.z = ((r0.zzzz)*(source[3].yyyy)).z;
    // 18: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 19: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 20: mad r0.z, v4.x, l(2.000000), l(-1.000000)
    r0.z = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 21: add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // 22: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 23: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 24: mul r0.z, r0.z, cb0[3].z
    r0.z = ((r0.zzzz)*(source[3].zzzz)).z;
    // 25: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 26: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 28: mad r0.z, cb0[3].w, v4.z, l(-1.000000)
    r0.z = ((source[3].wwww)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 29: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 30: mul r0.w, v4.z, cb0[3].w
    r0.w = ((v4.zzzz)*(source[3].wwww)).w;
    // 31: mad r0.zw, r0.wwww, v2.xxxy, -r0.zzzz
    r0.zw = ((r0.wwww)*(v2.xxxy)+(-(r0.zzzz))).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t0.xyzw, s2, l(0.000000)
    r1.xyzw = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 33: log r0.z, |r1.w|
    r0.z = (log2(abs(r1.wwww))).z;
    // 34: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 37: lt r0.w, |r1.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 38: movc r0.z, r0.w, l(0), |r0.z|
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.zzzz))).z;
    // 39: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 40: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 41: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 42: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 43: mul r0.w, r0.y, r0.w
    r0.w = ((r0.yyyy)*(r0.wwww)).w;
    // 44: ge r0.y, l(0.250000), r0.y
    r0.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.yyyy)) * 0xffffffffu)).y;
    // 45: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 46: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 48: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 49: mul r0.xzw, r1.xxyz, cb0[4].xxxx
    r0.xzw = ((r1.xxyz)*(source[4].xxxx)).xzw;
    // 50: dp3 r1.w, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: mad r1.xyz, -cb0[4].xxxx, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[4].xxxx))*(r1.xyzx)+(r1.wwww)).xyz;
    // 52: mad r0.xzw, cb0[4].yyyy, r1.xxyz, r0.xxzw
    r0.xzw = ((source[4].yyyy)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 53: mul r1.xyz, r0.xzwx, cb0[4].zzzz
    r1.xyz = ((r0.xzwx)*(source[4].zzzz)).xyz;
    // 54: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 56: mul r1.xyz, r1.xyzx, cb0[4].wwww
    r1.xyz = ((r1.xyzx)*(source[4].wwww)).xyz;
    // 57: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 58: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 59: add r0.xzw, r0.xxzw, r1.xxxx
    r0.xzw = ((r0.xxzw)+(r1.xxxx)).xzw;
    // 60: mul r0.xzw, r0.xxzw, v3.xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)).xzw;
    // 61: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 62: movc r0.xyz, r0.yyyy, r1.xyzx, r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (r1.xyzx) : (r0.xzwx)).xyz;
    // 63: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 64: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1029(WARLORD_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (WarlordNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r0.zw, -r0.xxxy, v2.xxxy
    r0.zw = ((-(r0.xxxy))+(v2.xxxy)).zw;
    // 3: mad r0.xy, v4.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((v4.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 4: mul r0.zw, r0.xxxy, cb0[2].zzzz
    r0.zw = ((r0.xxxy)*(source[2].zzzz)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.wxyz, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
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
    // 25: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 26: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 27: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1030(WARLORD_NATIVE_INPUT input)
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
    // 5: mul_sat r0.x, r0.x, cb0[2].w
    r0.x = (saturate((r0.xxxx)*(source[2].wwww))).x;
    // 6: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 7: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 8: mad o0.xyz, cb0[1].xyzx, v5.wwww, v5.xyzx
    output.xyz = ((source[1].xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1031(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
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
    // 10: mul_sat r0.x, r0.x, l(0.034483)
    r0.x = (saturate((r0.xxxx)*(float4(0.034483,0.034483,0.034483,0.034483)))).x;
    // 11: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 12: add r1.y, r0.y, cb0[3].x
    r1.y = ((r0.yyyy)+(source[3].xxxx)).y;
    // 13: mul r1.x, v4.x, cb0[3].y
    r1.x = ((v4.xxxx)*(source[3].yyyy)).x;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, cb0[1].w
    r0.y = ((r0.yyyy)*(source[1].wwww)).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v4.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 17: mul_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)*(r0.yyyy))).y;
    // 18: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 19: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 20: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 21: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 22: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 23: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1032(WARLORD_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3].x = g_WarlordSourceMaterialTime;
    source[3].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[3].z = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[4].x = ((g_WarlordSourceMaterialParameters[3u].zzzz).x*g_WarlordSourceMaterialTime);
    source[4].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[5].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].xxxx).x);
    source[5].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[6].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[2u].xxxx).x);
    source[7].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].y = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[2u].zzzz).x);
    source[8].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
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
    r0.xzw = (WarlordNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xwyz).xzw;
    // 49: mul r2.y, r0.y, cb0[6].y
    r2.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s0, l(0.000000)
    r1.xyz = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1033(WARLORD_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[11u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[8u].wwww,g_WarlordSourceMaterialParameters[9u].xxxx,1u);
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].wwww,g_WarlordSourceMaterialParameters[4u].xxxx,1u);
    source[4] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_WarlordSourceMaterialParameters[10u];
    source[9].x = (-1.0*sin((((g_WarlordSourceMaterialParameters[4u].wwww).x*6.28000021)*0.25)));
    source[9].y = cos((((g_WarlordSourceMaterialParameters[4u].wwww).x*6.28000021)*0.25));
    source[9].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[9].w = g_WarlordSourceMaterialTime;
    source[10].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[10].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_WarlordSourceMaterialParameters[8u].yyyy).x;
    source[11].w = (g_WarlordSourceMaterialParameters[8u].zzzz).x;
    source[12].x = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_WarlordSourceMaterialParameters[7u].wwww).x;
    source[12].z = (g_WarlordSourceMaterialParameters[8u].xxxx).x;
    source[12].w = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[13].x = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[13].y = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[13].z = (g_WarlordSourceMaterialParameters[9u].xxxx).x;
    source[13].w = (g_WarlordSourceMaterialParameters[8u].wwww).x;
    source[14].x = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[14].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[14].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[14].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[15].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[15].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[15].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[16].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[16].z = ((g_WarlordSourceMaterialParameters[1u].wwww).x*0.25);
    source[16].w = sin(((g_WarlordSourceMaterialParameters[1u].wwww).x*0.25));
    source[17].x = (-1.0*sin(((g_WarlordSourceMaterialParameters[1u].wwww).x*0.25)));
    source[17].y = cos(((g_WarlordSourceMaterialParameters[1u].wwww).x*0.25));
    source[17].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[18].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[18].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[18].w = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[19].x = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[19].y = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[19].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[12].yzyy
    r0.xy = ((v2.xyxx)*(source[12].yzyy)).xy;
    // 2: mad r0.xy, cb0[9].wwww, cb0[12].xwxx, r0.xyxx
    r0.xy = ((source[9].wwww)*(source[12].xwxx)+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.xy, r0.xxxx, cb0[13].xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)*(source[13].xxxx)+(v2.xyxx)).xy;
    // 5: mul r0.xy, r0.xyxx, cb0[11].zwzz
    r0.xy = ((r0.xyxx)*(source[11].zwzz)).xy;
    // 6: mad r1.x, cb0[9].w, cb0[11].y, r0.x
    r1.x = ((source[9].wwww)*(source[11].yyyy)+(r0.xxxx)).x;
    // 7: mad r1.y, cb0[9].w, cb0[13].y, r0.y
    r1.y = ((source[9].wwww)*(source[13].yyyy)+(r0.yyyy)).y;
    // 8: add r0.xy, r1.xyxx, cb0[2].xyxx
    r0.xy = ((r1.xyxx)+(source[2].xyxx)).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: add r0.y, v4.z, cb0[14].x
    r0.y = ((v4.zzzz)+(source[14].xxxx)).y;
    // 11: mad r0.xy, r0.xxxx, r0.yyyy, cb0[3].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[3].xyxx)).xy;
    // 12: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 13: max r1.x, |r0.w|, |r0.z|
    r1.x = (max(abs(r0.wwww),abs(r0.zzzz))).x;
    // 14: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 15: min r1.y, |r0.w|, |r0.z|
    r1.y = (min(abs(r0.wwww),abs(r0.zzzz))).y;
    // 16: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 17: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 18: mad r1.z, r1.y, l(0.020835), l(-0.085133)
    r1.z = ((r1.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 19: mad r1.z, r1.y, r1.z, l(0.180141)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 20: mad r1.z, r1.y, r1.z, l(-0.330299)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 21: mad r1.y, r1.y, r1.z, l(0.999866)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 22: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 23: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 24: lt r1.w, |r0.w|, |r0.z|
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).w;
    // 25: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 26: mad r1.x, r1.x, r1.y, r1.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // 27: lt r1.y, r0.w, -r0.w
    r1.y = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).y;
    // 28: and r1.y, r1.y, l(0xc0490fdb)
    r1.y = (asfloat(asuint(r1.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 29: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 30: min r1.y, r0.w, r0.z
    r1.y = (min(r0.wwww,r0.zzzz)).y;
    // 31: lt r1.y, r1.y, -r1.y
    r1.y = (asfloat((uint4)((r1.yyyy)<(-(r1.yyyy))) * 0xffffffffu)).y;
    // 32: max r1.z, r0.w, r0.z
    r1.z = (max(r0.wwww,r0.zzzz)).z;
    // 33: ge r1.z, r1.z, -r1.z
    r1.z = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).z;
    // 34: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 35: movc r1.x, r1.y, -r1.x, r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (-(r1.xxxx)) : (r1.xxxx)).x;
    // 36: mad r1.x, r1.x, l(0.159155), l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 37: dp2 r1.z, r0.zwzz, r0.zwzz
    r1.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 38: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 39: log r1.w, r1.z
    r1.w = (log2(r1.zzzz)).w;
    // 40: mul r1.w, r1.w, v4.w
    r1.w = ((r1.wwww)*(v4.wwww)).w;
    // 41: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 42: lt r2.x, r1.z, l(0.000001)
    r2.x = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 43: mad r1.z, -r1.z, l(2.000000), l(1.000000)
    r1.z = ((-(r1.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 44: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: mul r1.z, r1.z, cb0[18].w
    r1.z = ((r1.zzzz)*(source[18].wwww)).z;
    // 46: max r1.z, r1.z, cb0[19].y
    r1.z = (max(r1.zzzz,source[19].yyyy)).z;
    // 47: min r1.z, r1.z, cb0[19].x
    r1.z = (min(r1.zzzz,source[19].xxxx)).z;
    // 48: movc r1.y, r2.x, l(0), r1.w
    r1.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 49: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 50: mad r2.x, cb0[9].w, cb0[9].z, r1.x
    r2.x = ((source[9].wwww)*(source[9].zzzz)+(r1.xxxx)).x;
    // 51: mad r2.y, cb0[9].w, cb0[10].z, r1.y
    r2.y = ((source[9].wwww)*(source[10].zzzz)+(r1.yyyy)).y;
    // 52: mul r1.x, v4.x, cb0[10].w
    r1.x = ((v4.xxxx)*(source[10].wwww)).x;
    // 53: mul r1.y, v4.x, cb0[11].x
    r1.y = ((v4.xxxx)*(source[11].xxxx)).y;
    // 54: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r1.xyxx
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 55: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 56: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 57: dp2 r1.x, cb0[4].xyxx, r0.xyxx
    r1.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 58: dp2 r1.y, cb0[5].xyxx, r0.xyxx
    r1.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 59: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 60: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(-1.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 61: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 62: mul r0.y, r0.y, cb0[14].w
    r0.y = ((r0.yyyy)*(source[14].wwww)).y;
    // 63: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 64: mul r0.y, r0.y, cb0[15].x
    r0.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 65: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 67: mad r0.y, r0.x, cb0[15].y, r0.y
    r0.y = ((r0.xxxx)*(source[15].yyyy)+(r0.yyyy)).y;
    // 68: dp2 r1.x, cb0[6].xyxx, r0.zwzz
    r1.x = (dot((source[6].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 69: dp2 r1.y, cb0[7].xyxx, r0.zwzz
    r1.y = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 70: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 71: mul r0.z, r0.z, cb0[15].w
    r0.z = ((r0.zzzz)*(source[15].wwww)).z;
    // 72: mul r0.w, r0.w, cb0[16].x
    r0.w = ((r0.wwww)*(source[16].xxxx)).w;
    // 73: mad r0.w, cb0[9].w, cb0[17].z, r0.w
    r0.w = ((source[9].wwww)*(source[17].zzzz)+(r0.wwww)).w;
    // 74: add r1.y, r0.w, cb0[18].x
    r1.y = ((r0.wwww)+(source[18].xxxx)).y;
    // 75: mad r0.z, cb0[9].w, cb0[15].z, r0.z
    r0.z = ((source[9].wwww)*(source[15].zzzz)+(r0.zzzz)).z;
    // 76: add r1.x, r0.z, cb0[17].w
    r1.x = ((r0.zzzz)+(source[17].wwww)).x;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t3.yzxw, s3, l(0.000000)
    r0.z = (WarlordNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 78: add r0.z, r0.z, l(0.100000)
    r0.z = ((r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 79: add r0.w, v4.y, l(-1.000000)
    r0.w = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 80: add_sat r0.z, -r0.w, r0.z
    r0.z = (saturate((-(r0.wwww))+(r0.zzzz))).z;
    // 81: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 82: mul r0.w, r0.w, cb0[18].z
    r0.w = ((r0.wwww)*(source[18].zzzz)).w;
    // 83: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 84: mul_sat r0.w, r0.w, cb0[18].y
    r0.w = (saturate((r0.wwww)*(source[18].yyyy))).w;
    // 85: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 86: mul r0.z, r0.z, cb0[18].y
    r0.z = ((r0.zzzz)*(source[18].yyyy)).z;
    // 87: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 88: mov_sat r1.x, r0.z
    r1.x = (saturate(r0.zzzz)).x;
    // 89: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 90: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 91: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 92: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 93: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 94: add r0.x, r0.w, r1.x
    r0.x = ((r0.wwww)+(r1.xxxx)).x;
    // 95: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 96: mad r0.xyz, r0.xxxx, cb0[8].xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(source[8].xyzx)+(r0.yyyy)).xyz;
    // 97: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 98: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1034(WARLORD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2] = g_WarlordSourceMaterialParameters[2u];
    source[3].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[4].w, l(1.000000)
    r0.y = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 15: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 16: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 17: mul r0.z, r0.z, cb0[3].y
    r0.z = ((r0.zzzz)*(source[3].yyyy)).z;
    // 18: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 19: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 20: mad r0.z, v4.x, l(2.000000), l(-1.000000)
    r0.z = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 21: add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // 22: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 23: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 24: mul r0.z, r0.z, cb0[3].z
    r0.z = ((r0.zzzz)*(source[3].zzzz)).z;
    // 25: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 26: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 28: mad r0.z, cb0[3].w, v4.z, l(-1.000000)
    r0.z = ((source[3].wwww)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 29: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 30: mul r0.w, v4.z, cb0[3].w
    r0.w = ((v4.zzzz)*(source[3].wwww)).w;
    // 31: mad r0.zw, r0.wwww, v2.xxxy, -r0.zzzz
    r0.zw = ((r0.wwww)*(v2.xxxy)+(-(r0.zzzz))).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t0.xyzw, s2, l(0.000000)
    r1.xyzw = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 33: log r0.z, |r1.w|
    r0.z = (log2(abs(r1.wwww))).z;
    // 34: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 37: lt r0.w, |r1.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 38: movc r0.z, r0.w, l(0), |r0.z|
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.zzzz))).z;
    // 39: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 40: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 41: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 42: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 43: mul r0.w, r0.y, r0.w
    r0.w = ((r0.yyyy)*(r0.wwww)).w;
    // 44: ge r0.y, l(0.250000), r0.y
    r0.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.yyyy)) * 0xffffffffu)).y;
    // 45: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 46: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 48: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 49: mul r0.xzw, r1.xxyz, cb0[4].xxxx
    r0.xzw = ((r1.xxyz)*(source[4].xxxx)).xzw;
    // 50: dp3 r1.w, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: mad r1.xyz, -cb0[4].xxxx, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[4].xxxx))*(r1.xyzx)+(r1.wwww)).xyz;
    // 52: mad r0.xzw, cb0[4].yyyy, r1.xxyz, r0.xxzw
    r0.xzw = ((source[4].yyyy)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 53: mul r0.xzw, r0.xxzw, v3.xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)).xzw;
    // 54: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 55: movc r0.xyz, r0.yyyy, r1.xyzx, r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (r1.xyzx) : (r0.xzwx)).xyz;
    // 56: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 57: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1036(WARLORD_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
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
    // 1: mul r0.xyz, v5.wwww, cb0[1].xyzx
    r0.xyz = ((v5.wwww)*(source[1].xyzx)).xyz;
    // 2: mul o0.xyz, r0.xyzx, cb0[0].xxxx
    output.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 3: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1037(WARLORD_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1038(WARLORD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = WarlordNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (-1.0*sin(((-0.523599029*(g_WarlordSourceMaterialParameters[2u].wwww).x)*1.0)));
    source[4].y = cos(((-0.523599029*(g_WarlordSourceMaterialParameters[2u].wwww).x)*1.0));
    source[4].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[4].w = clamp((g_WarlordSourceMaterialParameters[4u].zzzz).x,0.0,1.0);
    source[5].x = (1.0-clamp((g_WarlordSourceMaterialParameters[4u].zzzz).x,0.0,1.0));
    source[5].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[5].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].z = g_WarlordSourceMaterialTime;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[8].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[9].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[9].w = (100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[10].x = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x));
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
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 44: div r0.yw, v7.xxxy, v7.wwww
    r0.yw = ((v7.xxxy)/(v7.wwww)).yw;
    // 45: mad r0.yw, r0.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r0.yw = ((r0.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // Native 46: source device depth mapped to centimetre view depth; reconstruction at 48.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.ywyy).xy, 0.f).y * 100000.f;
    // Native 48-51: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 52: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 53: add r0.w, -cb0[10].x, l(1.000000)
    r0.w = ((-(source[10].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 55: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 56: div_sat r0.y, r0.y, r0.w
    r0.y = (saturate((r0.yyyy)/(r0.wwww))).y;
    // 57: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 58: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 59: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 60: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 61: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 62: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 64: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 65: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 66: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 67: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 68: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 69: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1039(WARLORD_NATIVE_INPUT input)
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (WarlordNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 3: add r0.xyzw, r0.wxyz, -r1.wxyz
    r0.xyzw = ((r0.wxyz)+(-(r1.wxyz))).xyzw;
    // 4: mad r0.xyzw, v0.wwww, r0.xyzw, r1.wxyz
    r0.xyzw = ((v0.wwww)*(r0.xyzw)+(r1.wxyz)).xyzw;
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
float4 WarlordNative1040(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].zzzz).x);
    source[2].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].zzzz).x));
    source[2].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mul_sat r0.x, v4.y, cb0[2].w
    r0.x = (saturate((v4.yyyy)*(source[2].wwww))).x;
    // 2: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 4: max r0.x, r0.x, l(0.000010)
    r0.x = (max(r0.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 5: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 6: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 7: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 8: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 9: mad r0.y, -r0.y, cb0[2].z, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[2].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 10: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 11: mad r0.x, r0.x, l(-2.000000), l(1.000000)
    r0.x = ((r0.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 14: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: mul r0.z, v4.x, cb0[3].x
    r0.z = ((v4.xxxx)*(source[3].xxxx)).z;
    // 16: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 17: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 18: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 19: mov_sat r0.y, v4.z
    r0.y = (saturate(v4.zzzz)).y;
    // 20: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 21: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 25: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 26: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 27: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1041(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
    source[2] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3].x = (sin(((g_WarlordSourceMaterialTime*2.0)*6.28318548))*2.0);
    source[3].y = abs((sin(((g_WarlordSourceMaterialTime*2.0)*6.28318548))*2.0));
    source[3].z = fmod(abs((sin(((g_WarlordSourceMaterialTime*2.0)*6.28318548))*2.0)),1.5);
    source[3].w = (fmod(abs((sin(((g_WarlordSourceMaterialTime*2.0)*6.28318548))*2.0)),1.5)+0.300000012);
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
    // 1: mad r0.xy, v2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 4: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 5: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 6: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 7: mad r0.xw, r0.xxxx, l(0.070000, 0.000000, 0.000000, 0.070000), v2.xxxy
    r0.xw = ((r0.xxxx)*(float4(0.070000,0.000000,0.000000,0.070000))+(v2.xxxy)).xw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t1.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 9: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 10: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 11: mul r0.y, r0.x, l(7.000000)
    r0.y = ((r0.xxxx)*(float4(7.000000,7.000000,7.000000,7.000000))).y;
    // 12: mov_sat r0.y, r0.y
    r0.y = (saturate(r0.yyyy)).y;
    // 13: mad_sat r0.x, -r0.x, l(5.000000), r0.y
    r0.x = (saturate((-(r0.xxxx))*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.yyyy))).x;
    // 14: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 15: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 16: mul r0.x, r0.x, cb0[3].w
    r0.x = ((r0.xxxx)*(source[3].wwww)).x;
    // 17: mul r0.x, r0.x, l(5.000000)
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 18: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 19: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1042(WARLORD_NATIVE_INPUT input)
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
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1043(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4].x = g_WarlordSourceMaterialTime;
    source[4].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 45: mad r1.xyzw, r0.xxxx, l(0.500000, 0.500000, 0.500000, 0.500000), r1.xwxw
    r1.xyzw = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xwxw)).xyzw;
    // 46: mad r1.xyzw, r0.zzzz, l(-0.300000, -0.700000, -0.010000, -0.500000), r1.xyzw
    r1.xyzw = ((r0.zzzz)*(float4(-0.300000,-0.700000,-0.010000,-0.500000))+(r1.xyzw)).xyzw;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t1.yxzw, s2, l(0.000000)
    r0.y = (WarlordNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
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
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
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
    r0.y = (WarlordNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 90: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 91: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1044(WARLORD_NATIVE_INPUT input)
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
float4 WarlordNative1045(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
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
    // 1: mul r0.x, v4.x, l(0.100000)
    r0.x = ((v4.xxxx)*(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 2: mov r0.y, l(0)
    r0.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 3: mad r0.xy, v2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mad r0.xy, r0.xxxx, l(0.100000, 0.100000, 0.000000, 0.000000), v2.xyxx
    r0.xy = ((r0.xxxx)*(float4(0.100000,0.100000,0.000000,0.000000))+(v2.xyxx)).xy;
    // 6: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 7: dp2 r1.x, cb0[2].xyxx, r0.xyxx
    r1.x = (dot((source[2].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 8: dp2 r1.y, cb0[3].xyxx, r0.xyxx
    r1.y = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 9: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 12: add r0.yz, -v2.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(v2.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 13: mul r0.yz, r0.yyzy, v2.xxyx
    r0.yz = ((r0.yyzy)*(v2.xxyx)).yz;
    // 14: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, l(15.000000)
    r0.y = ((r0.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 16: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 17: mul r0.y, |r0.y|, |r0.y|
    r0.y = ((abs(r0.yyyy))*(abs(r0.yyyy))).y;
    // 18: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 19: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 20: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 21: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 22: source device depth mapped to centimetre view depth; reconstruction at 24.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 24-27: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 28: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 29: mul_sat r0.y, r0.y, l(0.040000)
    r0.y = (saturate((r0.yyyy)*(float4(0.040000,0.040000,0.040000,0.040000)))).y;
    // 30: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 31: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 32: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 33: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1046(WARLORD_NATIVE_INPUT input)
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s3, l(0.000000)
    r0.w = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t3.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: log r0.y, |r0.w|
    r0.y = (log2(abs(r0.wwww))).y;
    // 25: lt r0.z, |r0.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 27: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 28: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 29: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 30: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 31: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 32: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 35: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 37: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 38: mul r0.xyz, r1.xyzx, cb0[3].wwww
    r0.xyz = ((r1.xyzx)*(source[3].wwww)).xyz;
    // 39: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: mad r1.xyz, -cb0[3].wwww, r1.xyzx, r0.wwww
    r1.xyz = ((-(source[3].wwww))*(r1.xyzx)+(r0.wwww)).xyz;
    // 41: mad r0.xyz, cb0[4].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 42: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 43: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1047(WARLORD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[7] = g_WarlordSourceMaterialParameters[5u];
    source[8] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].wwww,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[9] = g_WarlordSourceMaterialParameters[4u];
    source[10] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].wwww,g_WarlordSourceMaterialParameters[2u].xxxx,1u);
    source[11].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[11].y = g_WarlordSourceMaterialTime;
    source[11].z = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[2u].yyyy).x);
    source[11].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[12].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[12].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[12].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[13].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[13].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[13].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
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
    // 2: mul r1.xyz, r0.yyyy, cb0[5].xyzx
    r1.xyz = ((r0.yyyy)*(source[5].xyzx)).xyz;
    // 3: mad r0.xyw, cb0[4].xyxz, r0.xxxx, r1.xyxz
    r0.xyw = ((source[4].xyxz)*(r0.xxxx)+(r1.xyxz)).xyw;
    // 4: mad r0.xyz, cb0[6].xyzx, r0.zzzz, r0.xywx
    r0.xyz = ((source[6].xyzx)*(r0.zzzz)+(r0.xywx)).xyz;
    // 5: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 6: mad r0.x, cb0[1].x, r0.x, r0.y
    r0.x = ((source[1].xxxx)*(r0.xxxx)+(r0.yyyy)).x;
    // 7: mad r0.x, cb0[3].x, r0.z, r0.x
    r0.x = ((source[3].xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 8: mad r0.x, v4.x, r0.x, cb0[11].z
    r0.x = ((v4.xxxx)*(r0.xxxx)+(source[11].zzzz)).x;
    // 9: mul r1.x, r0.x, cb0[11].w
    r1.x = ((r0.xxxx)*(source[11].wwww)).x;
    // 10: mul r0.z, v2.y, v4.y
    r0.z = ((v2.yyyy)*(v4.yyyy)).z;
    // 11: mul r1.y, r0.z, cb0[12].x
    r1.y = ((r0.zzzz)*(source[12].xxxx)).y;
    // 12: mul r0.xy, r0.xzxx, cb0[13].zwzz
    r0.xy = ((r0.xzxx)*(source[13].zwzz)).xy;
    // 13: add r0.zw, r1.xxxy, cb0[8].xxxy
    r0.zw = ((r1.xxxy)+(source[8].xxxy)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s0, l(0.000000)
    r1.xyz = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 16: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 17: mad r1.xyz, cb0[12].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[12].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 18: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 19: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 20: mul r1.xyz, r1.xyzx, cb0[13].xxxx
    r1.xyz = ((r1.xyzx)*(source[13].xxxx)).xyz;
    // 21: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 22: mul r1.xyz, r1.xyzx, cb0[9].xyzx
    r1.xyz = ((r1.xyzx)*(source[9].xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, cb0[13].yyyy
    r1.xyz = ((r1.xyzx)*(source[13].yyyy)).xyz;
    // 24: mul r1.xyz, r1.xyzx, v3.xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)).xyz;
    // 25: add r0.z, -v2.x, l(1.000000)
    r0.z = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 26: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 27: mad r1.xyz, r0.zzzz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r0.zzzz)+(r1.xyzx)).xyz;
    // 28: add r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)+(source[7].xyzx)).xyz;
    // 29: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 30: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 1.500000, 1.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,1.500000,1.000000))).zw;
    // 31: add r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)+(source[10].xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 33: mad r0.xy, r0.zzzz, v4.wwww, r0.xyxx
    r0.xy = ((r0.zzzz)*(v4.wwww)+(r0.xyxx)).xy;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 36: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 37: mul r0.x, r0.x, v4.z
    r0.x = ((r0.xxxx)*(v4.zzzz)).x;
    // 38: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 39: mul_sat r0.x, r0.x, cb0[14].z
    r0.x = (saturate((r0.xxxx)*(source[14].zzzz))).x;
    // 40: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 41: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 42: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 43: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 44: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1048(WARLORD_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = WarlordNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = WarlordNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[4u].yyyy,g_WarlordSourceMaterialParameters[4u].zzzz,1u);
    source[7].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[7].z = g_WarlordSourceMaterialTime;
    source[7].w = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[8].x = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[8].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[8].z = (3.1400001*(g_WarlordSourceMaterialParameters[5u].yyyy).x);
    source[8].w = ((3.1400001*(g_WarlordSourceMaterialParameters[5u].yyyy).x)*0.25);
    source[9].x = sin(((3.1400001*(g_WarlordSourceMaterialParameters[5u].yyyy).x)*0.25));
    source[9].y = (-1.0*sin(((3.1400001*(g_WarlordSourceMaterialParameters[5u].yyyy).x)*0.25)));
    source[9].z = cos(((3.1400001*(g_WarlordSourceMaterialParameters[5u].yyyy).x)*0.25));
    source[9].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[10].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[10].w = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[11].x = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[12].x = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[12].y = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[12].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[13].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[13].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[13].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[14].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[15].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[15].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
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
    // 10: mul_sat r0.x, r0.x, l(0.007143)
    r0.x = (saturate((r0.xxxx)*(float4(0.007143,0.007143,0.007143,0.007143)))).x;
    // 11: mul_sat r0.x, r0.x, cb0[15].w
    r0.x = (saturate((r0.xxxx)*(source[15].wwww))).x;
    // 12: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 13: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 14: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 15: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 16: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 17: mul r0.z, r0.z, cb0[15].y
    r0.z = ((r0.zzzz)*(source[15].yyyy)).z;
    // 18: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 19: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 20: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 21: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 22: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 23: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 24: mul r1.x, r0.z, cb0[7].w
    r1.x = ((r0.zzzz)*(source[7].wwww)).x;
    // 25: mad r1.x, cb0[7].z, cb0[7].y, r1.x
    r1.x = ((source[7].zzzz)*(source[7].yyyy)+(r1.xxxx)).x;
    // 26: mul r1.z, r0.w, cb0[8].x
    r1.z = ((r0.wwww)*(source[8].xxxx)).z;
    // 27: mad r1.y, cb0[7].z, cb0[9].w, r1.z
    r1.y = ((source[7].zzzz)*(source[9].wwww)+(r1.zzzz)).y;
    // 28: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 29: add r1.z, cb0[3].w, cb0[12].x
    r1.z = ((source[3].wwww)+(source[12].xxxx)).z;
    // 30: add r1.z, r1.z, l(-1.000000)
    r1.z = ((r1.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 31: mul r2.xy, r0.zwzz, cb0[11].xyxx
    r2.xy = ((r0.zwzz)*(source[11].xyxx)).xy;
    // 32: mul r0.zw, r0.zzzw, cb0[14].xxxy
    r0.zw = ((r0.zzzw)*(source[14].xxxy)).zw;
    // 33: mad r1.w, cb0[7].z, cb0[10].w, r2.x
    r1.w = ((source[7].zzzz)*(source[10].wwww)+(r2.xxxx)).w;
    // 34: mad r2.x, cb0[7].z, cb0[11].z, r2.y
    r2.x = ((source[7].zzzz)*(source[11].zzzz)+(r2.yyyy)).x;
    // 35: mad r2.y, cb0[3].y, cb0[11].w, r2.x
    r2.y = ((source[3].yyyy)*(source[11].wwww)+(r2.xxxx)).y;
    // 36: mad r2.x, cb0[3].y, cb0[10].z, r1.w
    r2.x = ((source[3].yyyy)*(source[10].zzzz)+(r1.wwww)).x;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 38: mad r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((r1.zzzz)*(r2.xyxx)+(r1.xyxx)).xy;
    // 39: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 40: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 41: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyz = (WarlordNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xy = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 44: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: mul r3.xyzw, r1.xxyz, cb0[1].wxyz
    r3.xyzw = ((r1.xxyz)*(source[1].wxyz)).xyzw;
    // 46: mad r0.z, cb0[7].z, cb0[13].w, r0.z
    r0.z = ((source[7].zzzz)*(source[13].wwww)+(r0.zzzz)).z;
    // 47: mad r0.w, cb0[7].z, cb0[14].z, r0.w
    r0.w = ((source[7].zzzz)*(source[14].zzzz)+(r0.wwww)).w;
    // 48: mad r4.y, cb0[3].y, cb0[14].w, r0.w
    r4.y = ((source[3].yyyy)*(source[14].wwww)+(r0.wwww)).y;
    // 49: mad r4.x, cb0[3].y, cb0[13].z, r0.z
    r4.x = ((source[3].yyyy)*(source[13].zzzz)+(r0.zzzz)).x;
    // 50: add r0.zw, r4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 51: dp2 r4.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.zwzz
    r4.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).x;
    // 52: dp2 r4.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.zwzz
    r4.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).y;
    // 53: add r0.zw, r4.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r4.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 55: add r0.z, -r1.w, r0.z
    r0.z = ((-(r1.wwww))+(r0.zzzz)).z;
    // 56: mul_sat r0.z, r0.z, cb0[15].x
    r0.z = (saturate((r0.zzzz)*(source[15].xxxx))).z;
    // 57: mul r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)*(r3.xxxx)).z;
    // 58: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 59: mul_sat r0.y, r0.y, cb0[15].z
    r0.y = (saturate((r0.yyyy)*(source[15].zzzz))).y;
    // 60: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 61: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 62: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 63: mad r0.xyz, -cb0[1].xyzx, r1.xyzx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 64: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 65: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 71: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1049(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[2].x, l(1.000000)
    r0.y = ((-(source[2].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.zwzz, t0.xwyz, s2, l(0.000000)
    r0.y = (WarlordNativeSample1((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t0.xywz, s2, l(0.000000)
    r0.z = (WarlordNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).z;
    // 15: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 16: mad r0.y, v0.w, r0.y, r0.z
    r0.y = ((v0.wwww)*(r0.yyyy)+(r0.zzzz)).y;
    // 17: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 18: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 19: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 20: mul r0.xy, v2.xyxx, l(8.000000, 4.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(8.000000,4.000000,0.000000,0.000000))).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 22: mul r1.xyz, v3.xyzx, l(0.359551, 0.330854, 0.274713, 0.000000)
    r1.xyz = ((v3.xyzx)*(float4(0.359551,0.330854,0.274713,0.000000))).xyz;
    // 23: mad r0.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 24: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1050(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2] = g_WarlordSourceMaterialParameters[1u];
    source[3].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1051(WARLORD_NATIVE_INPUT input)
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
float4 WarlordNative1052(WARLORD_NATIVE_INPUT input)
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
float4 WarlordNative1053(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
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
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t0.xyzw, s1, l(-1.000000)
    r1.xyz = (WarlordNativeSample0((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
    // Native 51: source device depth mapped to centimetre view depth; reconstruction at 53.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 53-56: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1054(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[4u];
    source[2] = g_WarlordSourceMaterialParameters[3u];
    source[3].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[5].z, l(1.000000)
    r0.y = ((-(source[5].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mul_sat r0.y, r0.y, cb0[5].x
    r0.y = (saturate((r0.yyyy)*(source[5].xxxx))).y;
    // 16: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 17: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 21: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: mul r0.y, v2.x, cb0[3].w
    r0.y = ((v2.xxxx)*(source[3].wwww)).y;
    // 25: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 26: mad r1.x, r0.z, cb0[3].z, r0.y
    r1.x = ((r0.zzzz)*(source[3].zzzz)+(r0.yyyy)).x;
    // 27: mul r0.y, v2.y, cb0[4].x
    r0.y = ((v2.yyyy)*(source[4].xxxx)).y;
    // 28: mad r1.y, r0.z, cb0[4].y, r0.y
    r1.y = ((r0.zzzz)*(source[4].yyyy)+(r0.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t1.wxyz, s1, l(0.000000)
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 30: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 31: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 32: mad r0.yzw, cb0[4].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 33: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 34: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 35: mul r0.yzw, r0.yyzw, cb0[4].wwww
    r0.yzw = ((r0.yyzw)*(source[4].wwww)).yzw;
    // 36: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 37: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 38: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1055(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1056(WARLORD_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = input.dynamicParameter;
    source[4] = g_WarlordSourceMaterialParameters[5u];
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].yyyy,g_WarlordSourceMaterialParameters[2u].zzzz,1u);
    source[6].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[6].y = g_WarlordSourceMaterialTime;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[8].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[8].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[8].z = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[8].w = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].yyyy).x));
    source[9].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[9].y = (1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x);
    source[9].z = max((1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x),9.99999975e-06);
    source[9].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].zzzz).x),9.99999975e-06));
    source[10].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[10].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[10].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[11].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[12].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
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
    // 10: add r0.y, -cb0[11].z, l(1.000000)
    r0.y = ((-(source[11].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 16: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 17: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[11].w
    r0.z = ((r0.zzzz)*(source[11].wwww)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: mul_sat r0.z, r0.z, cb0[12].x
    r0.z = (saturate((r0.zzzz)*(source[12].xxxx))).z;
    // 21: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 22: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 24: max r0.w, |r0.z|, |r0.y|
    r0.w = (max(abs(r0.zzzz),abs(r0.yyyy))).w;
    // 25: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 26: min r1.x, |r0.z|, |r0.y|
    r1.x = (min(abs(r0.zzzz),abs(r0.yyyy))).x;
    // 27: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 28: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 29: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 30: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 31: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 32: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 33: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 34: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 35: lt r1.z, |r0.z|, |r0.y|
    r1.z = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.yyyy))) * 0xffffffffu)).z;
    // 36: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 37: mad r0.w, r0.w, r1.x, r1.y
    r0.w = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).w;
    // 38: lt r1.x, r0.z, -r0.z
    r1.x = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).x;
    // 39: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 40: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 41: min r1.x, r0.z, r0.y
    r1.x = (min(r0.zzzz,r0.yyyy)).x;
    // 42: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 43: max r1.y, r0.z, r0.y
    r1.y = (max(r0.zzzz,r0.yyyy)).y;
    // 44: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 45: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 46: ge r0.z, r1.y, -r1.y
    r0.z = (asfloat((uint4)((r1.yyyy)>=(-(r1.yyyy))) * 0xffffffffu)).z;
    // 47: and r0.z, r0.z, r1.x
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r1.xxxx))).z;
    // 48: movc r0.z, r0.z, -r0.w, r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (-(r0.wwww)) : (r0.wwww)).z;
    // 49: mad r0.z, r0.z, l(0.159155), l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 50: mul r1.x, r0.z, cb0[5].x
    r1.x = ((r0.zzzz)*(source[5].xxxx)).x;
    // 51: mul r0.z, r0.z, cb0[6].w
    r0.z = ((r0.zzzz)*(source[6].wwww)).z;
    // 52: mul r2.x, cb0[6].y, cb0[10].w
    r2.x = ((source[6].yyyy)*(source[10].wwww)).x;
    // 53: mov r1.z, l(-1.000000)
    r1.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 54: add r0.w, r0.y, r0.y
    r0.w = ((r0.yyyy)+(r0.yyyy)).w;
    // 55: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 56: mul r1.y, cb0[3].z, cb0[7].y
    r1.y = ((source[3].zzzz)*(source[7].yyyy)).y;
    // 57: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 58: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 59: lt r1.y, r0.y, l(0.000000)
    r1.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 60: mad r0.y, -r0.y, cb0[8].w, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[8].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 61: mul_sat r0.y, r0.y, cb0[9].w
    r0.y = (saturate((r0.yyyy)*(source[9].wwww))).y;
    // 62: movc r0.w, r1.y, l(0), r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 63: mad r2.y, r0.w, cb0[5].y, cb0[3].x
    r2.y = ((r0.wwww)*(source[5].yyyy)+(source[3].xxxx)).y;
    // 64: mul r0.w, r0.w, cb0[7].x
    r0.w = ((r0.wwww)*(source[7].xxxx)).w;
    // 65: add r1.xy, r1.xzxx, r2.xyxx
    r1.xy = ((r1.xzxx)+(r2.xyxx)).xy;
    // 66: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 67: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s2, l(-1.000000)
    r1.x = (WarlordNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 68: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 69: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 70: mul r1.y, r1.y, cb0[10].x
    r1.y = ((r1.yyyy)*(source[10].xxxx)).y;
    // 71: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 72: movc r0.y, r0.y, l(0), r1.y
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 73: mul r0.y, r1.x, r0.y
    r0.y = ((r1.xxxx)*(r0.yyyy)).y;
    // 74: mul_sat r0.y, r0.y, cb0[11].x
    r0.y = (saturate((r0.yyyy)*(source[11].xxxx))).y;
    // 75: log r1.x, r0.y
    r1.x = (log2(r0.yyyy)).x;
    // 76: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 77: mul r1.x, r1.x, cb0[11].y
    r1.x = ((r1.xxxx)*(source[11].yyyy)).x;
    // 78: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 79: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 80: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 81: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 82: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 83: mul r0.y, cb0[6].x, cb0[6].y
    r0.y = ((source[6].xxxx)*(source[6].yyyy)).y;
    // 84: mad r1.x, r0.y, cb0[6].z, r0.z
    r1.x = ((r0.yyyy)*(source[6].zzzz)+(r0.zzzz)).x;
    // 85: mad r1.y, r0.y, cb0[7].z, r0.w
    r1.y = ((r0.yyyy)*(source[7].zzzz)+(r0.wwww)).y;
    // 86: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t2.wxyz, s1, l(-1.000000)
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 87: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 88: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 89: mad r0.yzw, cb0[7].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 90: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 91: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 92: mul r0.yzw, r0.yyzw, cb0[8].xxxx
    r0.yzw = ((r0.yyzw)*(source[8].xxxx)).yzw;
    // 93: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 94: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 95: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 96: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 97: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 98: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 99: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1057(WARLORD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[4u];
    source[3] = g_WarlordSourceMaterialParameters[2u];
    source[4] = input.dynamicParameter;
    source[5] = WarlordNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[6] = WarlordNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7].x = (-1.0*sin(((-0.523599029*(g_WarlordSourceMaterialParameters[1u].xxxx).x)*1.0)));
    source[7].y = cos(((-0.523599029*(g_WarlordSourceMaterialParameters[1u].xxxx).x)*1.0));
    source[7].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[8].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[8].z = (0.0*(g_WarlordSourceMaterialParameters[0u].xxxx).x);
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
    // 1: add r0.x, cb0[4].y, l(-1.000000)
    r0.x = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: dp2 r1.x, cb0[5].xyxx, r0.yzyy
    r1.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 4: dp2 r1.y, cb0[6].xyxx, r0.yzyy
    r1.y = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 5: add r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 6: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 8: add r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)+(r0.zzzz)).z;
    // 9: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 11: mul r0.w, cb0[4].z, cb0[7].z
    r0.w = ((source[4].zzzz)*(source[7].zzzz)).w;
    // 12: mad_sat r0.y, r1.y, r0.w, l(0.500000)
    r0.y = (saturate((r1.yyyy)*(r0.wwww)+(float4(0.500000,0.500000,0.500000,0.500000)))).y;
    // 13: add r0.xy, r0.xyxx, l(0.500000, 0.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.000000,0.000000,0.000000))).xy;
    // 14: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(-1.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 15: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 16: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 22: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 23: add r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)+(source[8].zzzz)).x;
    // 24: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 25: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 26: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
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
float4 WarlordNative1058(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[4].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (1.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[5].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[5].w = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[6].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
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
    r0.x = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
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
    r0.yzw = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1059(WARLORD_NATIVE_INPUT input)
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
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s3, l(-1.000000)
    r1.xyz = (WarlordNativeSample2((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 48: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 49: add r0.w, r1.z, r0.w
    r0.w = ((r1.zzzz)+(r0.wwww)).w;
    // 50: mad r1.x, -r0.x, cb0[7].w, l(1.000000)
    r1.x = ((-(r0.xxxx))*(source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, -r0.x, cb0[9].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[9].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 53: mul_sat r1.x, r1.x, cb0[8].w
    r1.x = (saturate((r1.xxxx)*(source[8].wwww))).x;
    // 54: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 55: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 56: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 57: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 58: mul r0.x, r0.x, cb0[10].w
    r0.x = ((r0.xxxx)*(source[10].wwww)).x;
    // 59: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 60: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 61: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 62: mul r0.x, r0.x, cb0[11].w
    r0.x = ((r0.xxxx)*(source[11].wwww)).x;
    // 63: mul_sat r0.x, r0.x, l(0.333330)
    r0.x = (saturate((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330)))).x;
    // 64: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 65: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: mul r0.w, r0.w, cb0[12].x
    r0.w = ((r0.wwww)*(source[12].xxxx)).w;
    // 67: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 68: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 69: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 70: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 71: source device depth mapped to centimetre view depth; reconstruction at 73.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 73-76: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 77: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 78: add r1.x, -cb0[12].y, l(1.000000)
    r1.x = ((-(source[12].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: max r1.x, -r1.x, l(0.001000)
    r1.x = (max(-(r1.xxxx),float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 80: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 81: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 82: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 83: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 84: mad r0.x, cb0[4].y, cb0[4].x, r0.y
    r0.x = ((source[4].yyyy)*(source[4].xxxx)+(r0.yyyy)).x;
    // 85: mad r0.y, cb0[4].y, cb0[6].z, r0.z
    r0.y = ((source[4].yyyy)*(source[6].zzzz)+(r0.zzzz)).y;
    // 86: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(-1.000000)
    r0.xyz = (WarlordNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 87: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 88: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 89: mad r0.xyz, cb0[6].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[6].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 90: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 91: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 92: mul r0.xyz, r0.xyzx, cb0[7].xxxx
    r0.xyz = ((r0.xyzx)*(source[7].xxxx)).xyz;
    // 93: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 94: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 95: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 96: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 97: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1060(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].x = ((g_WarlordSourceMaterialParameters[0u].wwww).x*0.0);
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
    // 1: mov r0.y, l(0)
    r0.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 2: mul r0.x, v4.x, l(0.200000)
    r0.x = ((v4.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 3: mad r0.xyzw, cb0[2].yyzz, v2.xyxy, r0.yxxy
    r0.xyzw = ((source[2].yyzz)*(v2.xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t1.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 8: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 9: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 10: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 12: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 13: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 14: mad_sat r0.x, r0.x, cb0[2].w, cb0[3].x
    r0.x = (saturate((r0.xxxx)*(source[2].wwww)+(source[3].xxxx))).x;
    // 15: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 16: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 17: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 18: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1061(WARLORD_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].yyyy,g_WarlordSourceMaterialParameters[1u].zzzz,1u);
    source[5].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[5].y = g_WarlordSourceMaterialTime;
    source[5].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (-0.523599029*(g_WarlordSourceMaterialParameters[1u].wwww).x);
    source[7].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.zxyw, s1, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
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
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xzyw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
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
    // 39: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 40: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
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
float4 WarlordNative1062(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[4u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].yyyy,g_WarlordSourceMaterialParameters[2u].zzzz,1u);
    source[3].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[5].z = (1.0*(g_WarlordSourceMaterialParameters[0u].wwww).x);
    source[5].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[6].z = (-0.523599029*(g_WarlordSourceMaterialParameters[2u].wwww).x);
    source[6].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
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
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
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
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
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
    // 32: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 33: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 34: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 35: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 36: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1063(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[2].y = g_WarlordSourceMaterialTime;
    source[2].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[2].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[3].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[3].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (0.00249999994/(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[5].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s3, l(0.000000)
    r0.w = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t3.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: log r0.y, |r0.w|
    r0.y = (log2(abs(r0.wwww))).y;
    // 25: lt r0.z, |r0.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 27: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 28: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 29: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 30: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 31: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 32: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 35: mul_sat r0.z, v7.w, cb0[5].x
    r0.z = (saturate((v7.wwww)*(source[5].xxxx))).z;
    // 36: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 37: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 38: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 39: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 40: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 41: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 42: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 43: mul r0.w, v4.w, cb0[5].y
    r0.w = ((v4.wwww)*(source[5].yyyy)).w;
    // 44: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 45: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 46: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 49: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 50: mul r0.xyz, r1.xyzx, cb0[3].wwww
    r0.xyz = ((r1.xyzx)*(source[3].wwww)).xyz;
    // 51: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: mad r1.xyz, -cb0[3].wwww, r1.xyzx, r0.wwww
    r1.xyz = ((-(source[3].wwww))*(r1.xyzx)+(r0.wwww)).xyz;
    // 53: mad r0.xyz, cb0[4].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 54: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 55: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1064(WARLORD_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = WarlordNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = WarlordNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].wwww,g_WarlordSourceMaterialParameters[4u].xxxx,1u);
    source[7].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].z = g_WarlordSourceMaterialTime;
    source[7].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[8].z = (3.1400001*(g_WarlordSourceMaterialParameters[4u].wwww).x);
    source[8].w = ((3.1400001*(g_WarlordSourceMaterialParameters[4u].wwww).x)*0.25);
    source[9].x = sin(((3.1400001*(g_WarlordSourceMaterialParameters[4u].wwww).x)*0.25));
    source[9].y = (-1.0*sin(((3.1400001*(g_WarlordSourceMaterialParameters[4u].wwww).x)*0.25)));
    source[9].z = cos(((3.1400001*(g_WarlordSourceMaterialParameters[4u].wwww).x)*0.25));
    source[9].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[10].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[12].x = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[12].y = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[12].w = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[13].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[13].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[13].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[14].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[14].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[14].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[14].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[15].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
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
    // 10: mul_sat r0.x, r0.x, l(0.007143)
    r0.x = (saturate((r0.xxxx)*(float4(0.007143,0.007143,0.007143,0.007143)))).x;
    // 11: mul_sat r0.x, r0.x, cb0[15].z
    r0.x = (saturate((r0.xxxx)*(source[15].zzzz))).x;
    // 12: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 13: dp2 r1.x, cb0[4].xyxx, r0.yzyy
    r1.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 14: dp2 r1.y, cb0[5].xyxx, r0.yzyy
    r1.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 15: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 16: mul r0.w, r0.y, cb0[7].w
    r0.w = ((r0.yyyy)*(source[7].wwww)).w;
    // 17: mad r1.x, cb0[7].z, cb0[7].y, r0.w
    r1.x = ((source[7].zzzz)*(source[7].yyyy)+(r0.wwww)).x;
    // 18: mul r0.w, r0.z, cb0[8].x
    r0.w = ((r0.zzzz)*(source[8].xxxx)).w;
    // 19: mad r1.y, cb0[7].z, cb0[9].w, r0.w
    r1.y = ((source[7].zzzz)*(source[9].wwww)+(r0.wwww)).y;
    // 20: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 21: add r0.w, cb0[3].w, cb0[12].x
    r0.w = ((source[3].wwww)+(source[12].xxxx)).w;
    // 22: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 23: mul r1.zw, r0.yyyz, cb0[11].xxxy
    r1.zw = ((r0.yyyz)*(source[11].xxxy)).zw;
    // 24: mul r0.yz, r0.yyzy, cb0[14].xxyx
    r0.yz = ((r0.yyzy)*(source[14].xxyx)).yz;
    // 25: mad r1.z, cb0[7].z, cb0[10].w, r1.z
    r1.z = ((source[7].zzzz)*(source[10].wwww)+(r1.zzzz)).z;
    // 26: mad r1.w, cb0[7].z, cb0[11].z, r1.w
    r1.w = ((source[7].zzzz)*(source[11].zzzz)+(r1.wwww)).w;
    // 27: mad r2.y, cb0[3].y, cb0[11].w, r1.w
    r2.y = ((source[3].yyyy)*(source[11].wwww)+(r1.wwww)).y;
    // 28: mad r2.x, cb0[3].y, cb0[10].z, r1.z
    r2.x = ((source[3].yyyy)*(source[10].zzzz)+(r1.zzzz)).x;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r2.xyxx, t0.zwxy, s1, l(0.000000)
    r1.zw = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 30: mad r1.xy, r0.wwww, r1.zwzz, r1.xyxx
    r1.xy = ((r0.wwww)*(r1.zwzz)+(r1.xyxx)).xy;
    // 31: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 32: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 33: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyz = (WarlordNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xy = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 36: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: mul r3.xyzw, r1.xxyz, cb0[1].wxyz
    r3.xyzw = ((r1.xxyz)*(source[1].wxyz)).xyzw;
    // 38: mad r0.y, cb0[7].z, cb0[13].w, r0.y
    r0.y = ((source[7].zzzz)*(source[13].wwww)+(r0.yyyy)).y;
    // 39: mad r0.z, cb0[7].z, cb0[14].z, r0.z
    r0.z = ((source[7].zzzz)*(source[14].zzzz)+(r0.zzzz)).z;
    // 40: mad r4.y, cb0[3].y, cb0[14].w, r0.z
    r4.y = ((source[3].yyyy)*(source[14].wwww)+(r0.zzzz)).y;
    // 41: mad r4.x, cb0[3].y, cb0[13].z, r0.y
    r4.x = ((source[3].yyyy)*(source[13].zzzz)+(r0.yyyy)).x;
    // 42: add r0.yz, r4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 43: dp2 r4.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.yzyy
    r4.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 44: dp2 r4.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.yzyy
    r4.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).y;
    // 45: add r0.yz, r4.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r4.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 47: add r0.y, -r1.w, r0.y
    r0.y = ((-(r1.wwww))+(r0.yyyy)).y;
    // 48: mul_sat r0.y, r0.y, cb0[15].x
    r0.y = (saturate((r0.yyyy)*(source[15].xxxx))).y;
    // 49: mul r0.y, r0.y, r3.x
    r0.y = ((r0.yyyy)*(r3.xxxx)).y;
    // 50: mul_sat r0.y, r0.y, cb0[15].y
    r0.y = (saturate((r0.yyyy)*(source[15].yyyy))).y;
    // 51: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 52: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 53: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 54: mad r0.xyz, -cb0[1].xyzx, r1.xyzx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 55: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 56: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 57: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 58: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 59: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 60: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 61: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 62: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 63: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1065(WARLORD_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.zwzz, t0.xywz, s1, l(0.000000)
    r0.xyz = (WarlordNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyz;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v2.xyxx, t0.xywz, s1, l(0.000000)
    r1.xyz = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyz;
    // 3: add r0.xyz, r0.zxyz, -r1.zxyz
    r0.xyz = ((r0.zxyz)+(-(r1.zxyz))).xyz;
    // 4: mad r0.xyz, v0.wwww, r0.xyzx, r1.zxyz
    r0.xyz = ((v0.wwww)*(r0.xyzx)+(r1.zxyz)).xyz;
    // 5: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 6: mul r0.yz, r0.yyzy, v4.xxyx
    r0.yz = ((r0.yyzy)*(v4.xxyx)).yz;
    // 7: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 8: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 9: mul r0.x, |r0.x|, |r0.x|
    r0.x = ((abs(r0.xxxx))*(abs(r0.xxxx))).x;
    // 10: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 11: add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 12: mul r0.z, r0.x, r0.z
    r0.z = ((r0.xxxx)*(r0.zzzz)).z;
    // 13: mad r0.y, r0.z, l(0.020000), r0.y
    r0.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 14: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 15: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 16: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 17: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 18: source device depth mapped to centimetre view depth; reconstruction at 20.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 20-23: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 24: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 25: mul_sat r0.y, r0.y, l(0.012346)
    r0.y = (saturate((r0.yyyy)*(float4(0.012346,0.012346,0.012346,0.012346)))).y;
    // 26: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 27: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1066(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[2].z = g_WarlordSourceMaterialTime;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[3].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[3].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[4].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
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
    r0.z = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r0.x = (WarlordNativeSample2((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t1.yxzw, s0, cb0[2].x
    r0.y = (WarlordNativeSample0((r0.ywyy).xy, (source[2].xxxx).x, true).yxzw).y;
    // 48: mul_sat r0.y, r0.y, cb0[4].w
    r0.y = (saturate((r0.yyyy)*(source[4].wwww))).y;
    // 49: mov_sat r0.z, v4.y
    r0.z = (saturate(v4.yyyy)).z;
    // 50: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 52: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 53: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 54: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 55: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: mul r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)*(source[5].xxxx)).y;
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
float4 WarlordNative1067(WARLORD_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[12u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[8u].xxxx,g_WarlordSourceMaterialParameters[8u].yyyy,1u);
    source[3] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].xxxx,g_WarlordSourceMaterialParameters[2u].yyyy,1u);
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].zzzz,g_WarlordSourceMaterialParameters[3u].wwww,1u);
    source[7].x = cos(((g_WarlordSourceMaterialParameters[4u].yyyy).x*0.25));
    source[7].y = (g_WarlordSourceMaterialParameters[8u].zzzz).x;
    source[7].z = (g_WarlordSourceMaterialParameters[9u].xxxx).x;
    source[7].w = (g_WarlordSourceMaterialParameters[8u].yyyy).x;
    source[8].x = (g_WarlordSourceMaterialParameters[8u].xxxx).x;
    source[8].y = (g_WarlordSourceMaterialParameters[10u].zzzz).x;
    source[8].z = (g_WarlordSourceMaterialParameters[10u].wwww).x;
    source[8].w = (g_WarlordSourceMaterialParameters[10u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[11u].xxxx).x;
    source[9].y = (g_WarlordSourceMaterialParameters[11u].yyyy).x;
    source[9].z = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[9].w = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[10].x = (g_WarlordSourceMaterialParameters[8u].wwww).x;
    source[10].y = (g_WarlordSourceMaterialParameters[9u].yyyy).x;
    source[10].z = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[10].w = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[11].x = (g_WarlordSourceMaterialParameters[9u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[9u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[7u].wwww).x;
    source[11].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[12].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[12].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[12].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[13].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[13].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[14].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[14].y = (g_WarlordSourceMaterialParameters[10u].xxxx).x;
    source[14].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[14].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[15].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[15].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[16].z = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[16].w = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[17].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[17].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[17].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[17].w = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[18].x = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[18].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
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
    // 1: mul r0.x, v2.x, cb0[11].w
    r0.x = ((v2.xxxx)*(source[11].wwww)).x;
    // 2: mul r0.y, v2.y, cb0[12].x
    r0.y = ((v2.yyyy)*(source[12].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 4: mad r1.x, v4.z, cb0[12].w, r0.x
    r1.x = ((v4.zzzz)*(source[12].wwww)+(r0.xxxx)).x;
    // 5: mad r1.y, v4.z, cb0[13].x, r0.y
    r1.y = ((v4.zzzz)*(source[13].xxxx)+(r0.yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.yz, v2.xxyx, cb0[8].yyzy
    r0.yz = ((v2.xxyx)*(source[8].yyzy)).yz;
    // 8: mad r0.yz, v4.zzzz, cb0[9].xxyx, r0.yyzy
    r0.yz = ((v4.zzzz)*(source[9].xxyx)+(r0.yyzy)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mad r0.z, v2.y, cb0[8].z, cb0[8].w
    r0.z = ((v2.yyyy)*(source[8].zzzz)+(source[8].wwww)).z;
    // 11: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 12: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 13: mad r0.zw, v2.xxxy, cb0[13].yyyz, cb0[6].xxxy
    r0.zw = ((v2.xxxy)*(source[13].yyyz)+(source[6].xxxy)).zw;
    // 14: mad r0.zw, cb0[14].yyyy, r0.yyyy, r0.zzzw
    r0.zw = ((source[14].yyyy)*(r0.yyyy)+(r0.zzzw)).zw;
    // 15: add r1.xy, v4.xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r0.zw, r1.xxxx, cb0[14].zzzw, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[14].zzzw)+(r0.zzzw)).zw;
    // 17: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 18: dp2 r2.x, cb0[3].xyxx, r0.zwzz
    r2.x = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 19: dp2 r2.y, cb0[4].xyxx, r0.zwzz
    r2.y = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 20: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (WarlordNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 22: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 23: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 24: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 28: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r0.z, r0.z, cb0[15].y
    r0.z = ((r0.zzzz)*(source[15].yyyy)).z;
    // 31: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 32: mul_sat r0.z, r0.z, cb0[15].z
    r0.z = (saturate((r0.zzzz)*(source[15].zzzz))).z;
    // 33: mul r1.zw, v4.zzzz, cb0[17].zzzw
    r1.zw = ((v4.zzzz)*(source[17].zzzw)).zw;
    // 34: mad r1.zw, cb0[17].xxxy, v2.xxxy, r1.zzzw
    r1.zw = ((source[17].xxxy)*(v2.xxxy)+(r1.zzzw)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t3.yzwx, s4, l(0.000000)
    r0.w = (WarlordNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: mul r1.zw, v4.zzzz, cb0[16].zzzw
    r1.zw = ((v4.zzzz)*(source[16].zzzw)).zw;
    // 37: mad r1.zw, cb0[16].xxxy, v2.xxxy, r1.zzzw
    r1.zw = ((source[16].xxxy)*(v2.xxxy)+(r1.zzzw)).zw;
    // 38: mad r1.zw, r0.wwww, cb0[18].xxxx, r1.zzzw
    r1.zw = ((r0.wwww)*(source[18].xxxx)+(r1.zzzw)).zw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t4.yzwx, s5, l(0.000000)
    r0.w = (WarlordNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: add r0.w, -r1.y, r0.w
    r0.w = ((-(r1.yyyy))+(r0.wwww)).w;
    // 41: mul_sat r0.w, r0.w, cb0[18].y
    r0.w = (saturate((r0.wwww)*(source[18].yyyy))).w;
    // 42: add r1.y, -v2.y, l(1.000000)
    r1.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: mul r1.y, r1.y, v2.y
    r1.y = ((r1.yyyy)*(v2.yyyy)).y;
    // 44: mul_sat r1.y, r1.y, cb0[15].w
    r1.y = (saturate((r1.yyyy)*(source[15].wwww))).y;
    // 45: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 46: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 47: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 48: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 49: movc o0.w, r0.x, l(0), r0.z
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 50: mad r0.xz, v2.xxyx, cb0[7].yyzy, cb0[2].xxyx
    r0.xz = ((v2.xxyx)*(source[7].yyzy)+(source[2].xxyx)).xz;
    // 51: mad r0.xy, r0.yyyy, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xzxx
    r0.xy = ((r0.yyyy)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xzxx)).xy;
    // 52: mad r0.xy, r1.xxxx, cb0[9].zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[9].zwzz)+(r0.xyxx)).xy;
    // 53: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 54: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 55: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 56: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 58: mul r0.yz, v4.zzzz, cb0[10].zzwz
    r0.yz = ((v4.zzzz)*(source[10].zzwz)).yz;
    // 59: mad r0.yz, v2.xxyx, cb0[10].xxyx, r0.yyzy
    r0.yz = ((v2.xxyx)*(source[10].xxyx)+(r0.yyzy)).yz;
    // 60: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 61: dp2 r1.x, cb0[3].xyxx, r0.yzyy
    r1.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 62: dp2 r1.y, cb0[4].xyxx, r0.yzyy
    r1.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 63: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 65: add r0.y, -r0.x, r0.y
    r0.y = ((-(r0.xxxx))+(r0.yyyy)).y;
    // 66: mad r0.x, r0.y, l(0.500000), r0.x
    r0.x = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.xxxx)).x;
    // 67: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 68: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 69: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 70: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 72: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 73: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 74: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 75: mad r0.x, r0.y, cb0[11].z, r0.x
    r0.x = ((r0.yyyy)*(source[11].zzzz)+(r0.xxxx)).x;
    // 76: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 77: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1068(WARLORD_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mad r0.x, cb0[4].x, cb0[3].x, l(-1.000000)
    r0.x = ((source[4].xxxx)*(source[3].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, cb0[3].x, cb0[4].x
    r0.y = ((source[3].xxxx)*(source[4].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v4.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v4.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1069(WARLORD_NATIVE_INPUT input)
{
    float4 source[33]; [unroll] for (uint i=0u; i<33u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[14u];
    source[2] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[0u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[0u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[0u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[0u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[1u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[1u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[1u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[1u].wwww*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[7u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[7u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[7u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[7u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[5u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[5u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[5u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[5u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[6u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[6u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[13] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[6u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[6u].yyyy*g_WarlordSourceMaterialTime.xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[14] = g_WarlordSourceMaterialParameters[13u];
    source[15].x = (g_WarlordSourceMaterialParameters[10u].zzzz).x;
    source[15].y = (g_WarlordSourceMaterialParameters[10u].wwww).x;
    source[15].z = g_WarlordSourceMaterialTime;
    source[15].w = (g_WarlordSourceMaterialParameters[9u].zzzz).x;
    source[16].x = ((g_WarlordSourceMaterialParameters[9u].zzzz).x*g_WarlordSourceMaterialTime);
    source[16].y = (((g_WarlordSourceMaterialParameters[9u].zzzz).x*g_WarlordSourceMaterialTime)+1.0);
    source[16].z = (g_WarlordSourceMaterialParameters[9u].wwww).x;
    source[16].w = ((g_WarlordSourceMaterialParameters[9u].wwww).x*g_WarlordSourceMaterialTime);
    source[17].x = (((g_WarlordSourceMaterialParameters[9u].wwww).x*g_WarlordSourceMaterialTime)+1.0);
    source[17].y = (g_WarlordSourceMaterialParameters[9u].yyyy).x;
    source[17].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[17].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[18].x = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[18].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[18].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[18].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[19].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[19].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[19].z = ((g_WarlordSourceMaterialParameters[0u].wwww).x*g_WarlordSourceMaterialTime);
    source[19].w = (((g_WarlordSourceMaterialParameters[0u].wwww).x*g_WarlordSourceMaterialTime)*0.25);
    source[20].x = sin((((g_WarlordSourceMaterialParameters[0u].wwww).x*g_WarlordSourceMaterialTime)*0.25));
    source[20].y = (-1.0*sin((((g_WarlordSourceMaterialParameters[0u].wwww).x*g_WarlordSourceMaterialTime)*0.25)));
    source[20].z = cos((((g_WarlordSourceMaterialParameters[0u].wwww).x*g_WarlordSourceMaterialTime)*0.25));
    source[20].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[21].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[21].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[21].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[21].w = ((g_WarlordSourceMaterialParameters[1u].wwww).x*g_WarlordSourceMaterialTime);
    source[22].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[22].y = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[22].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[22].w = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[23].x = floor((g_WarlordSourceMaterialParameters[4u].zzzz).x);
    source[23].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[23].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[23].w = (g_WarlordSourceMaterialParameters[8u].yyyy).x;
    source[24].x = (1.0*(g_WarlordSourceMaterialParameters[8u].yyyy).x);
    source[24].y = (g_WarlordSourceMaterialParameters[7u].wwww).x;
    source[24].z = ((g_WarlordSourceMaterialParameters[7u].wwww).x*0.25);
    source[24].w = sin(((g_WarlordSourceMaterialParameters[7u].wwww).x*0.25));
    source[25].x = (-1.0*sin(((g_WarlordSourceMaterialParameters[7u].wwww).x*0.25)));
    source[25].y = cos(((g_WarlordSourceMaterialParameters[7u].wwww).x*0.25));
    source[25].z = (g_WarlordSourceMaterialParameters[8u].xxxx).x;
    source[25].w = (g_WarlordSourceMaterialParameters[8u].zzzz).x;
    source[26].x = (g_WarlordSourceMaterialParameters[8u].wwww).x;
    source[26].y = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[26].z = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[26].w = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[27].x = cos((((g_WarlordSourceMaterialParameters[5u].yyyy).x*g_WarlordSourceMaterialTime)*0.25));
    source[27].y = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[27].z = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[27].w = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[28].x = (-1.0*sin((((g_WarlordSourceMaterialParameters[6u].yyyy).x*g_WarlordSourceMaterialTime)*0.25)));
    source[28].y = cos((((g_WarlordSourceMaterialParameters[6u].yyyy).x*g_WarlordSourceMaterialTime)*0.25));
    source[28].z = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[28].w = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[29].x = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[29].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[29].z = floor((g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[29].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[30].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[30].y = (g_WarlordSourceMaterialParameters[11u].zzzz).x;
    source[30].z = (g_WarlordSourceMaterialParameters[10u].yyyy).x;
    source[30].w = (g_WarlordSourceMaterialParameters[10u].xxxx).x;
    source[31].x = (g_WarlordSourceMaterialParameters[12u].xxxx).x;
    source[31].y = (g_WarlordSourceMaterialParameters[12u].yyyy).x;
    source[31].z = (g_WarlordSourceMaterialParameters[11u].wwww).x;
    source[31].w = (g_WarlordSourceMaterialParameters[11u].yyyy).x;
    source[32].x = (g_WarlordSourceMaterialParameters[9u].xxxx).x;
    source[32].y = (g_WarlordSourceMaterialParameters[11u].xxxx).x;
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
    // 1: mul r0.x, v4.w, cb0[27].w
    r0.x = ((v4.wwww)*(source[27].wwww)).x;
    // 2: add r0.y, v4.y, cb0[26].z
    r0.y = ((v4.yyyy)+(source[26].zzzz)).y;
    // 3: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 4: mad r1.xy, cb0[25].zzzz, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[25].zzzz)*(r0.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mad r2.y, r1.y, cb0[26].x, r0.y
    r2.y = ((r1.yyyy)*(source[26].xxxx)+(r0.yyyy)).y;
    // 6: mad r2.x, r1.x, cb0[25].w, cb0[26].y
    r2.x = ((r1.xxxx)*(source[25].wwww)+(source[26].yyyy)).x;
    // 7: add r1.xy, r2.xyxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 8: dp2 r2.x, cb0[8].xyxx, r1.xyxx
    r2.x = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 9: dp2 r2.y, cb0[9].xyxx, r1.xyxx
    r2.y = (dot((source[9].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 10: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 11: dp2 r2.x, cb0[10].xyxx, r0.zwzz
    r2.x = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 12: dp2 r2.y, cb0[11].xyxx, r0.zwzz
    r2.y = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 13: add r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 14: mul r1.zw, r1.zzzw, cb0[27].yyyz
    r1.zw = ((r1.zzzw)*(source[27].yyyz)).zw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s6, l(0.000000)
    r1.zw = (WarlordNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 16: mad r0.xy, r0.xxxx, r1.zwzz, r1.xyxx
    r0.xy = ((r0.xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 17: dp2 r1.x, cb0[12].xyxx, r0.zwzz
    r1.x = (dot((source[12].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 18: dp2 r1.y, cb0[13].xyxx, r0.zwzz
    r1.y = (dot((source[13].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 19: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 20: mul r1.xy, r1.xyxx, cb0[28].zwzz
    r1.xy = ((r1.xyxx)*(source[28].zwzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t4.xyzw, s7, l(0.000000)
    r1.xy = (WarlordNativeSample6((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 22: mad r0.xy, cb0[29].xxxx, r1.xyxx, r0.xyxx
    r0.xy = ((source[29].xxxx)*(r1.xyxx)+(r0.xyxx)).xy;
    // 23: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s5, cb0[24].x
    r0.x = (WarlordNativeSample4((r0.xyxx).xy, (source[24].xxxx).x, true).xyzw).x;
    // 24: mad r0.x, cb0[29].z, -r0.x, r0.x
    r0.x = ((source[29].zzzz)*(-(r0.xxxx))+(r0.xxxx)).x;
    // 25: mul r0.x, r0.x, cb0[29].w
    r0.x = ((r0.xxxx)*(source[29].wwww)).x;
    // 26: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 27: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.y, r0.y, cb0[30].x
    r0.y = ((r0.yyyy)*(source[30].xxxx)).y;
    // 29: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 30: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 31: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 32: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 33: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 34: mul r2.x, r1.x, cb0[20].w
    r2.x = ((r1.xxxx)*(source[20].wwww)).x;
    // 35: mul r2.y, r1.y, cb0[21].x
    r2.y = ((r1.yyyy)*(source[21].xxxx)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s3, l(0.000000)
    r1.xy = (WarlordNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 37: add r0.y, v4.x, cb0[19].x
    r0.y = ((v4.xxxx)+(source[19].xxxx)).y;
    // 38: mad r1.zw, cb0[18].xxxx, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[18].xxxx)*(r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 39: mad r2.y, r1.w, cb0[18].z, r0.y
    r2.y = ((r1.wwww)*(source[18].zzzz)+(r0.yyyy)).y;
    // 40: mad r2.x, r1.z, cb0[18].y, cb0[18].w
    r2.x = ((r1.zzzz)*(source[18].yyyy)+(source[18].wwww)).x;
    // 41: add r1.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r1.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 42: dp2 r2.x, cb0[2].xyxx, r1.zwzz
    r2.x = (dot((source[2].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 43: dp2 r2.y, cb0[3].xyxx, r1.zwzz
    r2.y = (dot((source[3].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 44: add r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 45: mul r0.y, v4.w, cb0[21].y
    r0.y = ((v4.wwww)*(source[21].yyyy)).y;
    // 46: mad r1.xy, r0.yyyy, r1.xyxx, r1.zwzz
    r1.xy = ((r0.yyyy)*(r1.xyxx)+(r1.zwzz)).xy;
    // 47: dp2 r2.x, cb0[6].xyxx, r0.zwzz
    r2.x = (dot((source[6].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 48: dp2 r2.y, cb0[7].xyxx, r0.zwzz
    r2.y = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 49: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 50: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 52: mul r0.zw, r0.zzzw, cb0[22].xxxy
    r0.zw = ((r0.zzzw)*(source[22].xxxy)).zw;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s4, l(0.000000)
    r0.zw = (WarlordNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 54: mad r0.zw, cb0[22].zzzz, r0.zzzw, r1.xxxy
    r0.zw = ((source[22].zzzz)*(r0.zzzw)+(r1.xxxy)).zw;
    // 55: mul r1.x, v4.z, cb0[17].z
    r1.x = ((v4.zzzz)*(source[17].zzzz)).x;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, r1.x
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (r1.xxxx).x, true).yzxw).z;
    // 57: mad r0.z, cb0[23].x, -r0.z, r0.z
    r0.z = ((source[23].xxxx)*(-(r0.zzzz))+(r0.zzzz)).z;
    // 58: mul r0.z, r0.z, cb0[23].y
    r0.z = ((r0.zzzz)*(source[23].yyyy)).z;
    // 59: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 60: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 61: mul r0.z, r0.z, cb0[23].z
    r0.z = ((r0.zzzz)*(source[23].zzzz)).z;
    // 62: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 63: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 64: add r0.x, -r0.z, r0.x
    r0.x = ((-(r0.zzzz))+(r0.xxxx)).x;
    // 65: mad r0.x, cb0[30].y, r0.x, r0.z
    r0.x = ((source[30].yyyy)*(r0.xxxx)+(r0.zzzz)).x;
    // 66: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 67: mul r0.z, r0.z, cb0[31].x
    r0.z = ((r0.zzzz)*(source[31].xxxx)).z;
    // 68: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 69: mul r0.z, r0.z, cb0[31].y
    r0.z = ((r0.zzzz)*(source[31].yyyy)).z;
    // 70: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 71: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 72: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 73: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: mad r1.x, cb0[31].z, l(10.000000), l(10.000000)
    r1.x = ((source[31].zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 75: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 76: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 77: mul r0.w, r0.w, cb0[31].w
    r0.w = ((r0.wwww)*(source[31].wwww)).w;
    // 78: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 79: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 80: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 81: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 82: source device depth mapped to centimetre view depth; reconstruction at 84.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 84-87: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 88: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 89: add r0.w, -cb0[32].x, l(1.000000)
    r0.w = ((-(source[32].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 91: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 92: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 93: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 94: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 95: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 96: mad r1.x, v2.x, cb0[15].x, cb0[16].y
    r1.x = ((v2.xxxx)*(source[15].xxxx)+(source[16].yyyy)).x;
    // 97: mad r1.y, v2.y, cb0[15].y, cb0[17].x
    r1.y = ((v2.yyyy)*(source[15].yyyy)+(source[17].xxxx)).y;
    // 98: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t7.wxyz, s1, l(-1.000000)
    r0.yzw = (WarlordNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 99: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 100: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 101: mad r0.yzw, cb0[17].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[17].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 102: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 103: mul r0.xyz, r0.xyzx, cb0[30].zzzz
    r0.xyz = ((r0.xyzx)*(source[30].zzzz)).xyz;
    // 104: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 105: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 106: mul r0.xyz, r0.xyzx, cb0[30].wwww
    r0.xyz = ((r0.xyzx)*(source[30].wwww)).xyz;
    // 107: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 108: mul_sat r0.xyz, r0.xyzx, cb0[14].xyzx
    r0.xyz = (saturate((r0.xyzx)*(source[14].xyzx))).xyz;
    // 109: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 110: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1070(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
    source[2] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
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
    // 1: add r0.x, v2.y, -v3.w
    r0.x = ((v2.yyyy)+(-(v3.wwww))).x;
    // 2: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: add_sat r0.x, -r0.x, l(1.000000)
    r0.x = (saturate((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mad r0.yz, v2.xxyx, l(0.000000, 2.000000, 1.000000, 0.000000), cb0[2].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,2.000000,1.000000,0.000000))+(source[2].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.z, v4.z, l(0.100000)
    r0.z = ((v4.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 7: mad r0.zw, r0.zzzz, r0.yyyy, v2.xxxy
    r0.zw = ((r0.zzzz)*(r0.yyyy)+(v2.xxxy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 9: dp2 r0.y, r0.zzzz, r0.yyyy
    r0.y = (dot((r0.zzzz).xy,(r0.yyyy).xy).xxxx).y;
    // 10: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 11: mul_sat r0.y, r0.z, v4.y
    r0.y = (saturate((r0.zzzz)*(v4.yyyy))).y;
    // 12: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 13: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 14: log r0.x, |r0.z|
    r0.x = (log2(abs(r0.zzzz))).x;
    // 15: lt r0.y, |r0.z|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 16: mul r0.x, r0.x, v4.x
    r0.x = ((r0.xxxx)*(v4.xxxx)).x;
    // 17: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 18: mul r0.xzw, r0.xxxx, v3.xxyz
    r0.xzw = ((r0.xxxx)*(v3.xxyz)).xzw;
    // 19: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 20: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1071(WARLORD_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
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
    // 10: mul_sat r0.x, r0.x, l(0.020000)
    r0.x = (saturate((r0.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000)))).x;
    // 11: mov r1.x, l(0)
    r1.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 12: mov r1.y, v4.x
    r1.y = (v4.xxxx).y;
    // 13: add r0.yz, r1.xxyx, v2.xxyx
    r0.yz = ((r1.xxyx)+(v2.xxyx)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.xwyz, s2, l(0.000000)
    r0.y = (WarlordNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 15: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 16: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 17: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 18: mul r0.xy, v2.xyxx, l(6.000000, 6.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(6.000000,6.000000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: mul r0.xyz, r0.xxxx, v3.xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = (WarlordNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 22: mul r1.xyz, r0.wwww, v3.xyzx
    r1.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // 23: mad r0.xyz, r0.xyzx, l(0.200000, 0.200000, 0.200000, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(0.200000,0.200000,0.200000,0.000000))+(r1.xyzx)).xyz;
    // 24: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 26: mul r1.xyz, r1.xyzx, l(1.500000, 1.500000, 1.500000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(1.500000,1.500000,1.500000,0.000000))).xyz;
    // 27: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 28: mad r0.xyz, v4.yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((v4.yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 29: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 30: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1072(WARLORD_NATIVE_INPUT input)
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
float4 WarlordNative1073(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[0u];
    source[2] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.119999997, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(-0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[3] = WarlordNativeAppend(WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),WarlordNativePeriodic((g_WarlordSourceMaterialTime.xxxx*float4(0.0599999987, 0.0, 0.0, 0.0))),1u);
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
    // 10: mul_sat r0.x, r0.x, l(0.010000)
    r0.x = (saturate((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000)))).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s3, l(0.000000)
    r0.y = (WarlordNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 12: mul_sat r0.y, r0.y, l(2.500000)
    r0.y = (saturate((r0.yyyy)*(float4(2.500000,2.500000,2.500000,2.500000)))).y;
    // 13: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 14: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 15: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 16: add r0.xy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((v2.xyxx)+(source[2].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 18: add r0.yz, v2.xxyx, cb0[3].xxyx
    r0.yz = ((v2.xxyx)+(source[3].xxyx)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.xyzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 20: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 21: mad r0.xy, r0.xxxx, l(0.100000, 0.100000, 0.000000, 0.000000), v2.xyxx
    r0.xy = ((r0.xxxx)*(float4(0.100000,0.100000,0.000000,0.000000))+(v2.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 24: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1074(WARLORD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[11u];
    source[3] = g_WarlordSourceMaterialParameters[8u];
    source[4] = g_WarlordSourceMaterialParameters[9u];
    source[5] = input.dynamicParameter;
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[4u].zzzz,g_WarlordSourceMaterialParameters[4u].wwww,1u);
    source[7].x = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[7].y = g_WarlordSourceMaterialTime;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (1.0*(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[9].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].x = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[12].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[12].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[12].z = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[12].w = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[13].x = (-0.523599029*(g_WarlordSourceMaterialParameters[5u].xxxx).x);
    source[13].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[14].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[14].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[5u].wwww).x;
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
    // 10: add r0.y, -cb0[14].y, l(1.000000)
    r0.y = ((-(source[14].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 16: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 17: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[14].z
    r0.z = ((r0.zzzz)*(source[14].zzzz)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: mul_sat r0.z, r0.z, cb0[14].w
    r0.z = (saturate((r0.zzzz)*(source[14].wwww))).z;
    // 21: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 22: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: add r0.y, cb0[5].y, l(-1.000000)
    r0.y = ((source[5].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 24: mad r0.z, cb0[7].y, cb0[12].w, cb0[13].x
    r0.z = ((source[7].yyyy)*(source[12].wwww)+(source[13].xxxx)).z;
    // 25: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 26: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 27: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 28: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 29: mul r0.zw, v4.xxxy, cb0[8].zzzw
    r0.zw = ((v4.xxxy)*(source[8].zzzw)).zw;
    // 30: mul r1.x, cb0[7].x, cb0[7].y
    r1.x = ((source[7].xxxx)*(source[7].yyyy)).x;
    // 31: mad r2.x, r1.x, cb0[8].y, r0.z
    r2.x = ((r1.xxxx)*(source[8].yyyy)+(r0.zzzz)).x;
    // 32: mad r2.y, r1.x, cb0[9].x, r0.w
    r2.y = ((r1.xxxx)*(source[9].xxxx)+(r0.wwww)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 34: mad r0.zw, cb0[9].zzzz, r0.zzzw, v4.xxxy
    r0.zw = ((source[9].zzzz)*(r0.zzzw)+(v4.xxxy)).zw;
    // 35: add r1.yz, r0.zzwz, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.zzwz)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 36: dp2 r1.w, r3.zyzz, r1.yzyy
    r1.w = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).w;
    // 37: dp2 r1.y, r3.yxyy, r1.yzyy
    r1.y = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).y;
    // 38: mul r2.x, r1.y, cb0[6].x
    r2.x = ((r1.yyyy)*(source[6].xxxx)).x;
    // 39: mad r2.z, r1.w, cb0[6].y, r0.y
    r2.z = ((r1.wwww)*(source[6].yyyy)+(r0.yyyy)).z;
    // 40: add r1.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.yzyy, t2.xzyw, s5, l(0.000000)
    r0.y = (WarlordNativeSample4((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 42: mul r1.y, r0.z, cb0[11].w
    r1.y = ((r0.zzzz)*(source[11].wwww)).y;
    // 43: mad r2.x, r1.x, cb0[11].z, r1.y
    r2.x = ((r1.xxxx)*(source[11].zzzz)+(r1.yyyy)).x;
    // 44: mul r1.y, r0.w, cb0[12].x
    r1.y = ((r0.wwww)*(source[12].xxxx)).y;
    // 45: mad r0.zw, v4.yyyx, cb0[3].xxxy, r0.zzzw
    r0.zw = ((v4.yyyx)*(source[3].xxxy)+(r0.zzzw)).zw;
    // 46: mad r2.y, r1.x, cb0[12].y, r1.y
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r1.yyyy)).y;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.xzyw, s4, l(0.000000)
    r1.y = (WarlordNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 48: add r1.z, -cb0[5].x, l(1.000000)
    r1.z = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 49: mad r0.y, r1.y, r0.y, -r1.z
    r0.y = ((r1.yyyy)*(r0.yyyy)+(-(r1.zzzz))).y;
    // 50: mul_sat r0.y, r0.y, cb0[13].w
    r0.y = (saturate((r0.yyyy)*(source[13].wwww))).y;
    // 51: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 52: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 53: mul r1.y, r1.y, cb0[14].x
    r1.y = ((r1.yyyy)*(source[14].xxxx)).y;
    // 54: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 55: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 56: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 59: mul r0.y, r0.z, cb0[7].w
    r0.y = ((r0.zzzz)*(source[7].wwww)).y;
    // 60: mad r2.x, r1.x, cb0[7].z, r0.y
    r2.x = ((r1.xxxx)*(source[7].zzzz)+(r0.yyyy)).x;
    // 61: mul r0.y, r1.x, cb0[9].w
    r0.y = ((r1.xxxx)*(source[9].wwww)).y;
    // 62: mad r2.y, cb0[8].x, r0.w, r0.y
    r2.y = ((source[8].xxxx)*(r0.wwww)+(r0.yyyy)).y;
    // 63: mul r0.yz, r0.zzwz, cb0[10].yyzy
    r0.yz = ((r0.zzwz)*(source[10].yyzy)).yz;
    // 64: mad r0.yz, r1.xxxx, cb0[10].xxwx, r0.yyzy
    r0.yz = ((r1.xxxx)*(source[10].xxwx)+(r0.yyzy)).yz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s3, l(0.000000)
    r0.yzw = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t4.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 67: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 68: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 69: mad r0.yzw, -r1.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r1.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 70: mad r0.yzw, cb0[11].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[11].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 71: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 72: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 73: mul r0.yzw, r0.yyzw, cb0[11].yyyy
    r0.yzw = ((r0.yyzw)*(source[11].yyyy)).yzw;
    // 74: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 75: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 76: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 77: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 78: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 79: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 80: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1075(WARLORD_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[11u];
    source[3] = g_WarlordSourceMaterialParameters[8u];
    source[4] = g_WarlordSourceMaterialParameters[9u];
    source[5] = input.dynamicParameter;
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[4u].zzzz,g_WarlordSourceMaterialParameters[4u].wwww,1u);
    source[7].x = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[7].y = g_WarlordSourceMaterialTime;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (1.0*(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[9].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].x = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[11].y = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[12].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[12].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[12].z = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[12].w = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[13].x = (-0.523599029*(g_WarlordSourceMaterialParameters[5u].xxxx).x);
    source[13].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[14].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[14].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[5u].wwww).x;
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
    // 10: add r0.y, -cb0[14].y, l(1.000000)
    r0.y = ((-(source[14].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 16: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 17: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[14].z
    r0.z = ((r0.zzzz)*(source[14].zzzz)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: mul_sat r0.z, r0.z, cb0[14].w
    r0.z = (saturate((r0.zzzz)*(source[14].wwww))).z;
    // 21: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 22: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: add r0.y, cb0[5].y, l(-1.000000)
    r0.y = ((source[5].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 24: mad r0.z, cb0[7].y, cb0[12].w, cb0[13].x
    r0.z = ((source[7].yyyy)*(source[12].wwww)+(source[13].xxxx)).z;
    // 25: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 26: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 27: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 28: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 29: mul r0.zw, v4.xxxy, cb0[8].zzzw
    r0.zw = ((v4.xxxy)*(source[8].zzzw)).zw;
    // 30: mul r1.x, cb0[7].x, cb0[7].y
    r1.x = ((source[7].xxxx)*(source[7].yyyy)).x;
    // 31: mad r2.x, r1.x, cb0[8].y, r0.z
    r2.x = ((r1.xxxx)*(source[8].yyyy)+(r0.zzzz)).x;
    // 32: mad r2.y, r1.x, cb0[9].x, r0.w
    r2.y = ((r1.xxxx)*(source[9].xxxx)+(r0.wwww)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 34: mad r0.zw, cb0[9].zzzz, r0.zzzw, v4.xxxy
    r0.zw = ((source[9].zzzz)*(r0.zzzw)+(v4.xxxy)).zw;
    // 35: add r1.yz, r0.zzwz, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.zzwz)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 36: dp2 r1.w, r3.zyzz, r1.yzyy
    r1.w = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).w;
    // 37: dp2 r1.y, r3.yxyy, r1.yzyy
    r1.y = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).y;
    // 38: mul r2.x, r1.y, cb0[6].x
    r2.x = ((r1.yyyy)*(source[6].xxxx)).x;
    // 39: mad r2.z, r1.w, cb0[6].y, r0.y
    r2.z = ((r1.wwww)*(source[6].yyyy)+(r0.yyyy)).z;
    // 40: add r1.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.yzyy, t2.xzyw, s5, l(0.000000)
    r0.y = (WarlordNativeSample4((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 42: mul r1.y, r0.z, cb0[11].w
    r1.y = ((r0.zzzz)*(source[11].wwww)).y;
    // 43: mad r2.x, r1.x, cb0[11].z, r1.y
    r2.x = ((r1.xxxx)*(source[11].zzzz)+(r1.yyyy)).x;
    // 44: mul r1.y, r0.w, cb0[12].x
    r1.y = ((r0.wwww)*(source[12].xxxx)).y;
    // 45: mad r0.zw, v4.yyyx, cb0[3].xxxy, r0.zzzw
    r0.zw = ((v4.yyyx)*(source[3].xxxy)+(r0.zzzw)).zw;
    // 46: mad r2.y, r1.x, cb0[12].y, r1.y
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r1.yyyy)).y;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.xzyw, s4, l(0.000000)
    r1.y = (WarlordNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 48: add r1.z, -cb0[5].x, l(1.000000)
    r1.z = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 49: mad r0.y, r1.y, r0.y, -r1.z
    r0.y = ((r1.yyyy)*(r0.yyyy)+(-(r1.zzzz))).y;
    // 50: mul_sat r0.y, r0.y, cb0[13].w
    r0.y = (saturate((r0.yyyy)*(source[13].wwww))).y;
    // 51: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 52: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 53: mul r1.y, r1.y, cb0[14].x
    r1.y = ((r1.yyyy)*(source[14].xxxx)).y;
    // 54: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 55: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 56: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 59: mul r0.y, r0.z, cb0[7].w
    r0.y = ((r0.zzzz)*(source[7].wwww)).y;
    // 60: mad r2.x, r1.x, cb0[7].z, r0.y
    r2.x = ((r1.xxxx)*(source[7].zzzz)+(r0.yyyy)).x;
    // 61: mul r0.y, r1.x, cb0[9].w
    r0.y = ((r1.xxxx)*(source[9].wwww)).y;
    // 62: mad r2.y, cb0[8].x, r0.w, r0.y
    r2.y = ((source[8].xxxx)*(r0.wwww)+(r0.yyyy)).y;
    // 63: mul r0.yz, r0.zzwz, cb0[10].yyzy
    r0.yz = ((r0.zzwz)*(source[10].yyzy)).yz;
    // 64: mad r0.yz, r1.xxxx, cb0[10].xxwx, r0.yyzy
    r0.yz = ((r1.xxxx)*(source[10].xxwx)+(r0.yyzy)).yz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s3, l(0.000000)
    r0.yzw = (WarlordNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t4.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 67: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 68: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 69: mad r0.yzw, -r1.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r1.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 70: mad r0.yzw, cb0[11].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[11].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 71: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 72: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 73: mul r0.yzw, r0.yyzw, cb0[11].yyyy
    r0.yzw = ((r0.yyzw)*(source[11].yyyy)).yzw;
    // 74: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 75: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 76: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 77: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 78: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 79: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 80: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1076(WARLORD_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[7u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[3u].wwww,g_WarlordSourceMaterialParameters[4u].xxxx,1u);
    source[4] = WarlordNativeAppend((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[3u].yyyy),(g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = WarlordNativeAppend((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[0u].xxxx),(g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[0u].yyyy),1u);
    source[7] = WarlordNativeAppend((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].xxxx),(g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].yyyy),1u);
    source[8] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[5u].zzzz,g_WarlordSourceMaterialParameters[5u].wwww,1u);
    source[9] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[12].x = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[12].y = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[12].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[14].y = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[15].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_WarlordSourceMaterialParameters[2u].wwww).x;
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
    // 1: mov r0.y, cb0[11].z
    r0.y = (source[11].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[3].xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[3].xxxy)+(source[4].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (WarlordNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    // 16: lt r0.z, |cb0[12].z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[12].zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 17: mul r0.w, |cb0[12].z|, |cb0[12].z|
    r0.w = ((abs(source[12].zzzz))*(abs(source[12].zzzz))).w;
    // 18: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 19: mul r1.y, r0.z, v4.y
    r1.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 20: mul r1.x, v4.x, cb0[12].w
    r1.x = ((v4.xxxx)*(source[12].wwww)).x;
    // 21: mad r0.zw, cb0[5].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[5].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 22: add r0.zw, r0.zzzw, cb0[7].xxxy
    r0.zw = ((r0.zzzw)+(source[7].xxxy)).zw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t3.xyzw, s2, l(0.000000)
    r1.xyz = (WarlordNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: lt r0.z, |cb0[11].x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[11].xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 25: mul r0.w, |cb0[11].x|, |cb0[11].x|
    r0.w = ((abs(source[11].xxxx))*(abs(source[11].xxxx))).w;
    // 26: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 27: mul r2.y, r0.z, v4.y
    r2.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 28: mul r2.x, v4.x, cb0[11].y
    r2.x = ((v4.xxxx)*(source[11].yyyy)).x;
    // 29: mad r0.xy, cb0[5].xxxx, r0.xyxx, r2.xyxx
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 30: add r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)+(source[5].wwww)).y;
    // 31: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 33: mul r2.xyz, r1.xyzx, r0.xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 34: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: mad r0.yzw, -r0.xxyz, r1.xxyz, r0.wwww
    r0.yzw = ((-(r0.xxyz))*(r1.xxyz)+(r0.wwww)).yzw;
    // 36: mad r0.yzw, cb0[13].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[13].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 37: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 38: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 39: mul r0.yzw, r0.yyzw, cb0[13].yyyy
    r0.yzw = ((r0.yyzw)*(source[13].yyyy)).yzw;
    // 40: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 41: mul r0.yzw, r0.yyzw, cb0[13].zzzz
    r0.yzw = ((r0.yyzw)*(source[13].zzzz)).yzw;
    // 42: mad r0.yzw, cb0[1].xxyz, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)*(r0.yyzw)+(source[2].xxyz)).yzw;
    // 43: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 44: mad r0.yz, v4.xxyx, cb0[8].xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)*(source[8].xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 45: dp2 r1.x, cb0[9].xyxx, r0.yzyy
    r1.x = (dot((source[9].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 46: dp2 r1.y, cb0[10].xyxx, r0.yzyy
    r1.y = (dot((source[10].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 47: add r0.yz, r1.xxyx, cb0[5].zzyz
    r0.yz = ((r1.xxyx)+(source[5].zzyz)).yz;
    // 48: add r0.yz, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 49: mad r0.xy, r0.xxxx, cb0[14].zzzz, r0.yzyy
    r0.xy = ((r0.xxxx)*(source[14].zzzz)+(r0.yzyy)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 52: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 54: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 55: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 59: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 60: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 61: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 62: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, cb0[15].y
    r0.x = (saturate((r0.xxxx)*(source[15].yyyy))).x;
    // 64: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 65: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1077(WARLORD_NATIVE_INPUT input)
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1078(WARLORD_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[9u];
    source[3] = g_WarlordSourceMaterialParameters[6u];
    source[4] = g_WarlordSourceMaterialParameters[7u];
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].zzzz,g_WarlordSourceMaterialParameters[1u].wwww,1u);
    source[6] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].wwww,g_WarlordSourceMaterialParameters[3u].yyyy,1u);
    source[7] = WarlordNativeAppend((g_WarlordSourceMaterialParameters[2u].zzzz*g_WarlordSourceMaterialTime.xxxx),(g_WarlordSourceMaterialParameters[3u].xxxx*g_WarlordSourceMaterialTime.xxxx),1u);
    source[8] = input.dynamicParameter;
    source[9] = WarlordNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_WarlordSourceMaterialParameters[5u].xxxx*g_WarlordSourceMaterialTime.xxxx),1u);
    source[10] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[4u].yyyy,g_WarlordSourceMaterialParameters[4u].zzzz,1u);
    source[13] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[14] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[15].x = ((g_WarlordSourceMaterialParameters[3u].xxxx).x*g_WarlordSourceMaterialTime);
    source[15].y = ((g_WarlordSourceMaterialParameters[2u].zzzz).x*g_WarlordSourceMaterialTime);
    source[15].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[16].y = ((g_WarlordSourceMaterialParameters[5u].xxxx).x*g_WarlordSourceMaterialTime);
    source[16].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[16].w = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[17].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[17].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[17].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[17].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[18].x = cos(((g_WarlordSourceMaterialParameters[3u].zzzz).x*0.25));
    source[18].y = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[18].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[18].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[19].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[19].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[19].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
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
    // 51: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 52: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 53: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 54: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 55: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 56: mul r0.z, r0.z, cb0[19].y
    r0.z = ((r0.zzzz)*(source[19].yyyy)).z;
    // 57: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 58: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 59: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 60: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1079(WARLORD_NATIVE_INPUT input)
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
    source[4].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (100.0-(g_WarlordSourceMaterialParameters[1u].yyyy).x);
    source[5].y = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[1u].yyyy).x));
    source[5].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
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
    // 12: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 13: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 14: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 15: source device depth mapped to centimetre view depth; reconstruction at 17.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 17-20: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 21: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 22: add r0.z, -cb0[5].y, l(1.000000)
    r0.z = ((-(source[5].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 23: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 24: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 25: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 26: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
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
    // 32: mul r0.w, r0.w, cb0[5].z
    r0.w = ((r0.wwww)*(source[5].zzzz)).w;
    // 33: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 34: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 35: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 36: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 37: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 38: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 39: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 40: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 41: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 42: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1080(WARLORD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2].x = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[2].y = g_WarlordSourceMaterialTime;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
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
    // 6: mul r0.y, r0.y, cb0[4].x
    r0.y = ((r0.yyyy)*(source[4].xxxx)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul_sat r0.y, r0.y, cb0[4].y
    r0.y = (saturate((r0.yyyy)*(source[4].yyyy))).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: mul r0.y, v2.x, cb0[2].w
    r0.y = ((v2.xxxx)*(source[2].wwww)).y;
    // 11: mul r0.z, cb0[2].x, cb0[2].y
    r0.z = ((source[2].xxxx)*(source[2].yyyy)).z;
    // 12: mad r1.x, r0.z, cb0[2].z, r0.y
    r1.x = ((r0.zzzz)*(source[2].zzzz)+(r0.yyyy)).x;
    // 13: mul r0.y, v2.y, cb0[3].x
    r0.y = ((v2.yyyy)*(source[3].xxxx)).y;
    // 14: mad r1.y, r0.z, cb0[3].y, r0.y
    r1.y = ((r0.zzzz)*(source[3].yyyy)+(r0.yyyy)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (WarlordNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 16: add r0.z, -v4.x, l(1.000000)
    r0.z = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 17: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 18: mul_sat r0.y, r0.y, cb0[3].z
    r0.y = (saturate((r0.yyyy)*(source[3].zzzz))).y;
    // 19: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 20: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 21: mul r0.z, r0.z, cb0[3].w
    r0.z = ((r0.zzzz)*(source[3].wwww)).z;
    // 22: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 23: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 24: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 25: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 26: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 27: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 28: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 29: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 30: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1081(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[5u];
    source[2] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].wwww,g_WarlordSourceMaterialParameters[2u].xxxx,1u);
    source[3] = g_WarlordSourceMaterialParameters[4u];
    source[4].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[4].y = g_WarlordSourceMaterialTime;
    source[4].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[6].x = (-0.523599029*(g_WarlordSourceMaterialParameters[2u].yyyy).x);
    source[6].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
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
    // 1: add r0.x, v4.y, l(-1.000000)
    r0.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[4].y, cb0[5].w, cb0[6].x
    r0.y = ((source[4].yyyy)*(source[5].wwww)+(source[6].xxxx)).y;
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
    // 10: mad r0.x, r0.y, cb0[2].x, r0.x
    r0.x = ((r0.yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 11: mul r0.z, r0.w, cb0[2].y
    r0.z = ((r0.wwww)*(source[2].yyyy)).z;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (WarlordNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: mul r0.w, v2.x, cb0[4].w
    r0.w = ((v2.xxxx)*(source[4].wwww)).w;
    // 15: mul r1.x, cb0[4].x, cb0[4].y
    r1.x = ((source[4].xxxx)*(source[4].yyyy)).x;
    // 16: mad r2.x, r1.x, cb0[4].z, r0.w
    r2.x = ((r1.xxxx)*(source[4].zzzz)+(r0.wwww)).x;
    // 17: mul r0.w, v2.y, cb0[5].x
    r0.w = ((v2.yyyy)*(source[5].xxxx)).w;
    // 18: mad r2.y, r1.x, cb0[5].y, r0.w
    r2.y = ((r1.xxxx)*(source[5].yyyy)+(r0.wwww)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 21: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 22: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 23: mad r0.xyz, cb0[6].wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((source[6].wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 24: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 26: mul r0.xyz, r0.xyzx, cb0[7].xxxx
    r0.xyz = ((r0.xyzx)*(source[7].xxxx)).xyz;
    // 27: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 28: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 29: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 30: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 32: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 33: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 34: source device depth mapped to centimetre view depth; reconstruction at 36.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 36-39: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 40: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 41: add r1.x, -cb0[7].w, l(1.000000)
    r1.x = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 42: max r1.x, -r1.x, l(0.001000)
    r1.x = (max(-(r1.xxxx),float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 43: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 44: add r1.x, -v4.x, l(1.000000)
    r1.x = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: mul_sat r1.x, r1.x, cb0[7].y
    r1.x = (saturate((r1.xxxx)*(source[7].yyyy))).x;
    // 47: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 48: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 49: mul r1.y, r1.y, cb0[7].z
    r1.y = ((r1.yyyy)*(source[7].zzzz)).y;
    // 50: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 51: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 52: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 53: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 54: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 55: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 56: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1082(WARLORD_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2] = g_WarlordSourceMaterialParameters[5u];
    source[3] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].wwww,g_WarlordSourceMaterialParameters[3u].xxxx,1u);
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].zzzz,g_WarlordSourceMaterialParameters[1u].xxxx,1u);
    source[5] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[0u].wwww,g_WarlordSourceMaterialParameters[1u].yyyy,1u);
    source[8] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[2u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[10].z = g_WarlordSourceMaterialTime;
    source[10].w = (g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[11].x = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x)*0.0))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[1u].zzzz).x)*0.0))));
    source[12].x = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[2u].yyyy).x)*0.0))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[2u].yyyy).x)*0.0))));
    source[12].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[13].z = (100.0-(g_WarlordSourceMaterialParameters[3u].zzzz).x);
    source[13].w = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[3u].zzzz).x));
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
    // 1: mad r0.xy, cb0[3].xyxx, v2.xyxx, v4.xyxx
    r0.xy = ((source[3].xyxx)*(v2.xyxx)+(v4.xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (WarlordNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, cb0[11].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[11].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 4: mad r1.x, cb0[4].x, r0.x, cb0[5].x
    r1.x = ((source[4].xxxx)*(r0.xxxx)+(source[5].xxxx)).x;
    // 5: mad r1.y, cb0[4].y, r0.y, cb0[6].y
    r1.y = ((source[4].yyyy)*(r0.yyyy)+(source[6].yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mad r1.x, cb0[7].x, r0.x, cb0[8].x
    r1.x = ((source[7].xxxx)*(r0.xxxx)+(source[8].xxxx)).x;
    // 8: mad r1.y, cb0[7].y, r0.y, cb0[9].y
    r1.y = ((source[7].yyyy)*(r0.yyyy)+(source[9].yyyy)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1083(WARLORD_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
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
    source[5].z = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[5].w = clamp((g_WarlordSourceMaterialParameters[5u].zzzz).x,0.0,1.0);
    source[6].x = (1.0-clamp((g_WarlordSourceMaterialParameters[5u].zzzz).x,0.0,1.0));
    source[6].y = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[6].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[7].x = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[7].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[9].x = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
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
    r0.xz = (WarlordNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).xz;
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
    r0.w = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 21: mad r2.x, cb0[4].y, cb0[7].w, r0.x
    r2.x = ((source[4].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 22: mad r2.y, cb0[4].y, cb0[8].z, r0.z
    r2.y = ((source[4].yyyy)*(source[8].zzzz)+(r0.zzzz)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (WarlordNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER)
float4 WarlordNative1084(WARLORD_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[3u];
    source[2].x = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[2].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
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
    // 7: sample_indexable(texture2d)(float,float,float,float) r2.z, r0.zwzz, t1.xyzw, s1 (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r0.zwzz).xy).xyzw).z;
    // 8: mul r0.zw, r1.yyyy, cb0[2].zzzw
    r0.zw = ((r1.yyyy)*(source[2].zzzw)).zw;
    // 9: mad r1.xyzw, r0.zzww, r1.xzxz, r0.xyxy
    r1.xyzw = ((r0.zzww)*(r1.xzxz)+(r0.xyxy)).xyzw;
    // Native 10: source device depth mapped to centimetre view depth; reconstruction at 20.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // 12: sample_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t1.xyzw, s1 (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r1.xyxx).xy).xyzw).x;
    // 13: sample_indexable(texture2d)(float,float,float,float) r2.y, r1.zwzz, t1.xyzw, s1 (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r1.zwzz).xy).xyzw).y;
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
    // Native 20-23: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
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
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1085(WARLORD_NATIVE_INPUT input)
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
float4 WarlordNative1086(WARLORD_NATIVE_INPUT input)
{
    float4 source[23]; [unroll] for (uint i=0u; i<23u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[12u];
    source[2] = WarlordNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_WarlordSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[4u].zzzz,g_WarlordSourceMaterialParameters[4u].wwww,1u);
    source[5] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = WarlordNativeAppend(cos((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = WarlordNativeAppend(sin((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_WarlordSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = g_WarlordSourceMaterialParameters[11u];
    source[10].x = (-1.0*sin((((g_WarlordSourceMaterialParameters[5u].zzzz).x*6.28000021)*0.25)));
    source[10].y = cos((((g_WarlordSourceMaterialParameters[5u].zzzz).x*6.28000021)*0.25));
    source[10].z = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[10].w = g_WarlordSourceMaterialTime;
    source[11].x = (g_WarlordSourceMaterialParameters[5u].wwww).x;
    source[11].y = (g_WarlordSourceMaterialParameters[6u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].w = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[12].x = cos(((3.1400001*(g_WarlordSourceMaterialParameters[5u].zzzz).x)*0.25));
    source[12].y = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_WarlordSourceMaterialParameters[9u].yyyy).x;
    source[12].w = (g_WarlordSourceMaterialParameters[10u].yyyy).x;
    source[13].x = (g_WarlordSourceMaterialParameters[10u].zzzz).x;
    source[13].y = (g_WarlordSourceMaterialParameters[8u].wwww).x;
    source[13].z = (g_WarlordSourceMaterialParameters[9u].wwww).x;
    source[13].w = (g_WarlordSourceMaterialParameters[10u].xxxx).x;
    source[14].x = (g_WarlordSourceMaterialParameters[9u].xxxx).x;
    source[14].y = (g_WarlordSourceMaterialParameters[8u].yyyy).x;
    source[14].z = (g_WarlordSourceMaterialParameters[9u].zzzz).x;
    source[14].w = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[15].x = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[15].y = (g_WarlordSourceMaterialParameters[8u].zzzz).x;
    source[15].z = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[15].w = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[16].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[16].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[17].x = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[17].y = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[17].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[17].w = ((g_WarlordSourceMaterialParameters[2u].xxxx).x*0.25);
    source[18].x = sin(((g_WarlordSourceMaterialParameters[2u].xxxx).x*0.25));
    source[18].y = (-1.0*sin(((g_WarlordSourceMaterialParameters[2u].xxxx).x*0.25)));
    source[18].z = cos(((g_WarlordSourceMaterialParameters[2u].xxxx).x*0.25));
    source[18].w = (g_WarlordSourceMaterialParameters[6u].zzzz).x;
    source[19].x = (g_WarlordSourceMaterialParameters[7u].xxxx).x;
    source[19].y = (g_WarlordSourceMaterialParameters[7u].yyyy).x;
    source[19].z = (g_WarlordSourceMaterialParameters[6u].wwww).x;
    source[19].w = (g_WarlordSourceMaterialParameters[6u].yyyy).x;
    source[20].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[20].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[20].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[20].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[21].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[21].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[21].z = (g_WarlordSourceMaterialParameters[7u].zzzz).x;
    source[21].w = (g_WarlordSourceMaterialParameters[7u].wwww).x;
    source[22].x = (g_WarlordSourceMaterialParameters[8u].xxxx).x;
    source[22].y = (g_WarlordSourceMaterialParameters[3u].wwww).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[19].xyxx
    r0.xy = ((v2.xyxx)*(source[19].xyxx)).xy;
    // 2: mad r1.x, cb0[10].w, cb0[18].w, r0.x
    r1.x = ((source[10].wwww)*(source[18].wwww)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[10].w, cb0[19].z, r0.y
    r1.y = ((source[10].wwww)*(source[19].zzzz)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: dp2 r1.x, cb0[7].xyxx, r0.yzyy
    r1.x = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 7: dp2 r1.y, cb0[8].xyxx, r0.yzyy
    r1.y = (dot((source[8].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 8: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: mad r0.xw, r0.xxxx, cb0[19].wwww, r1.xxxy
    r0.xw = ((r0.xxxx)*(source[19].wwww)+(r1.xxxy)).xw;
    // 10: mul r0.xw, r0.xxxw, cb0[17].xxxy
    r0.xw = ((r0.xxxw)*(source[17].xxxy)).xw;
    // 11: mad r1.x, cb0[10].w, cb0[16].w, r0.x
    r1.x = ((source[10].wwww)*(source[16].wwww)+(r0.xxxx)).x;
    // 12: mad r1.y, cb0[10].w, cb0[20].x, r0.w
    r1.y = ((source[10].wwww)*(source[20].xxxx)+(r0.wwww)).y;
    // 13: add r0.xw, r1.xxxy, cb0[20].yyyz
    r0.xw = ((r1.xxxy)+(source[20].yyyz)).xw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t4.xyzw, s4, l(0.000000)
    r0.x = (WarlordNativeSample4((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 15: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 16: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 17: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 18: mul r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)*(source[20].wwww)).w;
    // 19: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 20: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 21: add r0.w, v4.y, l(-1.000000)
    r0.w = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 22: add_sat r0.x, -r0.w, r0.x
    r0.x = (saturate((-(r0.wwww))+(r0.xxxx))).x;
    // 23: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 24: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 25: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 26: mul_sat r0.w, r0.w, cb0[21].x
    r0.w = (saturate((r0.wwww)*(source[21].xxxx))).w;
    // 27: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.x, r0.x, cb0[21].x
    r0.x = ((r0.xxxx)*(source[21].xxxx)).x;
    // 29: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 30: mov_sat r1.x, r0.x
    r1.x = (saturate(r0.xxxx)).x;
    // 31: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 32: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: mul r1.xyz, r0.wwww, cb0[9].xyzx
    r1.xyz = ((r0.wwww)*(source[9].xyzx)).xyz;
    // 34: mul r2.xy, v2.xyxx, cb0[13].zwzz
    r2.xy = ((v2.xyxx)*(source[13].zwzz)).xy;
    // 35: mad r3.x, cb0[10].w, cb0[13].y, r2.x
    r3.x = ((source[10].wwww)*(source[13].yyyy)+(r2.xxxx)).x;
    // 36: mad r3.y, cb0[10].w, cb0[14].x, r2.y
    r3.y = ((source[10].wwww)*(source[14].xxxx)+(r2.yyyy)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (WarlordNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 38: mad r2.xy, r0.wwww, cb0[14].yyyy, v2.xyxx
    r2.xy = ((r0.wwww)*(source[14].yyyy)+(v2.xyxx)).xy;
    // 39: mul r0.w, cb0[10].w, cb0[12].z
    r0.w = ((source[10].wwww)*(source[12].zzzz)).w;
    // 40: mad r3.x, cb0[12].w, r2.x, r0.w
    r3.x = ((source[12].wwww)*(r2.xxxx)+(r0.wwww)).x;
    // 41: mul r0.w, cb0[10].w, cb0[14].z
    r0.w = ((source[10].wwww)*(source[14].zzzz)).w;
    // 42: mad r3.y, cb0[13].x, r2.y, r0.w
    r3.y = ((source[13].xxxx)*(r2.yyyy)+(r0.wwww)).y;
    // 43: mul r2.x, v4.w, cb0[14].w
    r2.x = ((v4.wwww)*(source[14].wwww)).x;
    // 44: mul r2.y, v4.w, cb0[15].x
    r2.y = ((v4.wwww)*(source[15].xxxx)).y;
    // 45: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (WarlordNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 47: add r1.w, v4.z, cb0[15].y
    r1.w = ((v4.zzzz)+(source[15].yyyy)).w;
    // 48: mad r2.xy, r0.wwww, r1.wwww, cb0[4].xyxx
    r2.xy = ((r0.wwww)*(r1.wwww)+(source[4].xyxx)).xy;
    // 49: dp2 r3.x, cb0[2].xyxx, r0.yzyy
    r3.x = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 50: dp2 r3.y, cb0[3].xyxx, r0.yzyy
    r3.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 51: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 52: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 53: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 55: mul r0.y, r0.y, cb0[21].z
    r0.y = ((r0.yyyy)*(source[21].zzzz)).y;
    // 56: max r0.y, r0.y, cb0[22].x
    r0.y = (max(r0.yyyy,source[22].xxxx)).y;
    // 57: min r0.y, r0.y, cb0[21].w
    r0.y = (min(r0.yyyy,source[21].wwww)).y;
    // 58: mad r0.zw, cb0[11].zzzw, v4.xxxx, r3.xxxy
    r0.zw = ((source[11].zzzw)*(v4.xxxx)+(r3.xxxy)).zw;
    // 59: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 60: mul r0.zw, r0.zzzw, cb0[11].xxxy
    r0.zw = ((r0.zzzw)*(source[11].xxxy)).zw;
    // 61: mad r3.x, cb0[10].w, cb0[10].z, r0.z
    r3.x = ((source[10].wwww)*(source[10].zzzz)+(r0.zzzz)).x;
    // 62: mad r3.y, cb0[10].w, cb0[12].y, r0.w
    r3.y = ((source[10].wwww)*(source[12].yyyy)+(r0.wwww)).y;
    // 63: add r0.zw, r2.xxxy, r3.xxxy
    r0.zw = ((r2.xxxy)+(r3.xxxy)).zw;
    // 64: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 65: dp2 r2.x, cb0[5].xyxx, r0.zwzz
    r2.x = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 66: dp2 r2.y, cb0[6].xyxx, r0.zwzz
    r2.y = (dot((source[6].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 67: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 68: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s0, l(-1.000000)
    r0.z = (WarlordNativeSample0((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 69: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 70: mul r0.w, r0.w, cb0[16].x
    r0.w = ((r0.wwww)*(source[16].xxxx)).w;
    // 71: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 72: mul r0.w, r0.w, cb0[16].y
    r0.w = ((r0.wwww)*(source[16].yyyy)).w;
    // 73: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 75: mad r0.w, r0.z, cb0[16].z, r0.w
    r0.w = ((r0.zzzz)*(source[16].zzzz)+(r0.wwww)).w;
    // 76: mad r1.xyz, r0.zzzz, r1.xyzx, r0.wwww
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r0.wwww)).xyz;
    // 77: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 78: mul r0.x, r0.x, cb0[22].y
    r0.x = ((r0.xxxx)*(source[22].yyyy)).x;
    // 79: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 80: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 81: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 82: mad r0.yzw, r1.xxyz, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r1.xxyz)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 83: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 84: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 85: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative1087(WARLORD_NATIVE_INPUT input)
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
