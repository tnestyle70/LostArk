// Warlord recovered source programs. Carrier filtering preserves the selected VF.
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative2000(WARLORD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[1u];
    source[3] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[0u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[4] = WarlordNativeAppend(WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),WarlordNativePeriodic(((g_WarlordSourceMaterialTime.xxxx*g_WarlordSourceMaterialParameters[0u].yyyy)*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[5].x = ((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.5);
    source[5].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].z = ((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.100000001);
    source[5].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.100000001))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.100000001))));
    source[6].y = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.0))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.0))));
    source[6].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].w = (sign(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.5))*frac(abs(((g_WarlordSourceMaterialTime*(g_WarlordSourceMaterialParameters[0u].yyyy).x)*0.5))));
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
    // 10: mul_sat r0.xy, r0.xxxx, l(0.010000, 0.033333, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xxxx)*(float4(0.010000,0.033333,0.000000,0.000000)))).xy;
    // 11: log r0.zw, r0.xxxy
    r0.zw = (log2(r0.xxxy)).zw;
    // 12: lt r0.xy, r0.xyxx, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 13: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.250000, 0.125000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.250000,0.125000))).zw;
    // 14: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 15: add r0.zw, -r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.zzzw))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 16: movc r0.xy, r0.xyxx, l(1.000000,1.000000,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 17: mul r1.xyzw, v4.xyxy, cb0[5].yyww
    r1.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 18: mad r0.zw, r1.zzzw, l(0.000000, 0.000000, 3.000000, 0.500000), cb0[3].xxxy
    r0.zw = ((r1.zzzw)*(float4(0.000000,0.000000,3.000000,0.500000))+(source[3].xxxy)).zw;
    // 19: mul r1.xy, r1.xyxx, l(4.000000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(4.000000,0.500000,0.000000,0.000000))).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: mad r0.zw, r0.zzzz, cb0[6].zzzz, r1.xxxy
    r0.zw = ((r0.zzzz)*(source[6].zzzz)+(r1.xxxy)).zw;
    // 22: add r0.zw, r0.zzzw, cb0[4].xxxy
    r0.zw = ((r0.zzzw)+(source[4].xxxy)).zw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (WarlordNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 24: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 25: mad r0.y, r0.y, r0.z, r0.y
    r0.y = ((r0.yyyy)*(r0.zzzz)+(r0.yyyy)).y;
    // 26: mad r0.x, r0.x, r0.z, r0.y
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).x;
    // 27: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 28: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 29: mul r0.w, v4.y, l(0.500000)
    r0.w = ((v4.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 30: mad r1.x, -v4.y, l(0.500000), l(1.000000)
    r1.x = ((-(v4.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 32: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 33: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 34: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 35: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 36: mul r1.x, r1.x, l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 37: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 38: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 39: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative2001(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[8u];
    source[3] = g_WarlordSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[6].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
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
    source[9].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (-0.523599029*(g_WarlordSourceMaterialParameters[3u].xxxx).x);
    source[10].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[11].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
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
    // 55: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 56: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 57: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 58: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 59: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 60: mul r1.w, r1.w, cb0[11].z
    r1.w = ((r1.wwww)*(source[11].zzzz)).w;
    // 61: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 62: mul_sat r1.w, r1.w, cb0[11].w
    r1.w = (saturate((r1.wwww)*(source[11].wwww))).w;
    // 63: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 64: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 65: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 66: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 67: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 68: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 69: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 70: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative2002(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2] = WarlordNativeAppend(cos(((g_WarlordSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_WarlordSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = WarlordNativeAppend(sin(((g_WarlordSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_WarlordSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = WarlordNativeAppend((float4(1.0, 0.0, 0.0, 0.0)/g_WarlordSourceMaterialParameters[1u].zzzz),(float4(1.0, 0.0, 0.0, 0.0)/g_WarlordSourceMaterialParameters[1u].wwww),1u);
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[1u].xxxx,g_WarlordSourceMaterialParameters[1u].yyyy,1u);
    source[6].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[6].z = max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06);
    source[6].w = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06));
    source[7].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
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
    // 2: dp2 r1.x, cb0[2].xyxx, r0.xyxx
    r1.x = (dot((source[2].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.xyxx
    r1.y = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: mad r0.xy, cb0[4].xyxx, r1.xyxx, -cb0[5].xyxx
    r0.xy = ((source[4].xyxx)*(r1.xyxx)+(-(source[5].xyxx))).xy;
    // 5: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 6: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 7: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul_sat r0.x, r0.x, cb0[6].w
    r0.x = (saturate((r0.xxxx)*(source[6].wwww))).x;
    // 9: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 10: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 12: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 13: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 14: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 15: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 16: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 17: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 18: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 19: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 20: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative2003(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_WarlordSourceMaterialParameters[8u];
    source[3] = g_WarlordSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5] = WarlordNativeAppend(g_WarlordSourceMaterialParameters[2u].zzzz,g_WarlordSourceMaterialParameters[2u].wwww,1u);
    source[6].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[6].y = g_WarlordSourceMaterialTime;
    source[6].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (1.0*(g_WarlordSourceMaterialParameters[0u].zzzz).x);
    source[8].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[9].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (-0.523599029*(g_WarlordSourceMaterialParameters[3u].xxxx).x);
    source[10].y = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_WarlordSourceMaterialParameters[0u].wwww).x;
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
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
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
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
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
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
    // 48: mul r0.w, r0.w, cb0[11].y
    r0.w = ((r0.wwww)*(source[11].yyyy)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: mul_sat r0.w, r0.w, cb0[11].z
    r0.w = (saturate((r0.wwww)*(source[11].zzzz))).w;
    // 51: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 52: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 53: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 54: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 55: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 56: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 57: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative2004(WARLORD_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[1u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
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
    // 24: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 25: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 26: add r0.xy, r0.xxxx, l(-0.900000, -0.880000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)+(float4(-0.900000,-0.880000,0.000000,0.000000))).xy;
    // 27: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 28: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 29: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 30: mad r0.z, r0.z, l(0.159155), l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 31: mul r0.w, r0.z, l(30.000000)
    r0.w = ((r0.zzzz)*(float4(30.000000,30.000000,30.000000,30.000000))).w;
    // 32: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 34: mul r1.x, r0.z, l(30.000000)
    r1.x = ((r0.zzzz)*(float4(30.000000,30.000000,30.000000,30.000000))).x;
    // 35: add r0.z, r0.z, -v4.x
    r0.z = ((r0.zzzz)+(-(v4.xxxx))).z;
    // 36: mul_sat r0.xyz, r0.xyzx, l(200.000000, 200.000000, 1000.000000, 0.000000)
    r0.xyz = (saturate((r0.xyzx)*(float4(200.000000,200.000000,1000.000000,0.000000)))).xyz;
    // 37: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 38: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 39: mad r0.w, r0.w, l(4.000000), l(-0.100000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).w;
    // 40: mul_sat r0.w, r0.w, l(20.000000)
    r0.w = (saturate((r0.wwww)*(float4(20.000000,20.000000,20.000000,20.000000)))).w;
    // 41: add r0.xz, -r0.xxzx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = ((-(r0.xxzx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 42: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 43: mul r0.y, r0.w, r0.x
    r0.y = ((r0.wwww)*(r0.xxxx)).y;
    // 44: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 45: mad r0.x, r0.x, r0.w, -r0.y
    r0.x = ((r0.xxxx)*(r0.wwww)+(-(r0.yyyy))).x;
    // 46: mad r0.x, r0.z, r0.x, r0.y
    r0.x = ((r0.zzzz)*(r0.xxxx)+(r0.yyyy)).x;
    // 47: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 48: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 49: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 50: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 WarlordNative2005(WARLORD_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[7u];
    source[2] = g_WarlordSourceMaterialParameters[6u];
    source[3].x = (g_WarlordSourceMaterialParameters[5u].zzzz).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[3].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[4].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[4].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[4].w = (1.0*(g_WarlordSourceMaterialParameters[3u].wwww).x);
    source[5].x = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[5].z = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[6].w = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[7].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[7].y = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].zzzz).x);
    source[7].z = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[1u].zzzz).x));
    source[7].w = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[8].x = (1.0-(g_WarlordSourceMaterialParameters[1u].wwww).x);
    source[8].y = max((1.0-(g_WarlordSourceMaterialParameters[1u].wwww).x),9.99999975e-06);
    source[8].z = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[1u].wwww).x),9.99999975e-06));
    source[8].w = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[9].x = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[9].y = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[0u].xxxx).x));
    source[9].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x);
    source[10].x = max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06);
    source[10].y = (1.0/max((1.0-(g_WarlordSourceMaterialParameters[0u].yyyy).x),9.99999975e-06));
    source[10].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[10].w = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[11].x = (g_WarlordSourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
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
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 25: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 26: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 27: mul r0.z, v4.w, cb0[4].y
    r0.z = ((v4.wwww)*(source[4].yyyy)).z;
    // 28: add r0.w, -r0.x, l(0.500000)
    r0.w = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 29: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 30: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 31: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 32: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 33: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 34: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 35: mul r0.z, r0.z, cb0[4].w
    r0.z = ((r0.zzzz)*(source[4].wwww)).z;
    // 36: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 37: mad r0.y, r0.y, l(0.159155), r0.z
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(r0.zzzz)).y;
    // 38: add r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 39: mul r0.z, v4.z, cb0[5].x
    r0.z = ((v4.zzzz)*(source[5].xxxx)).z;
    // 40: add r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)+(r0.xxxx)).w;
    // 41: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 42: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 43: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 44: lt r0.w, r0.x, l(0.000000)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 45: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 46: add r0.z, r0.z, r0.y
    r0.z = ((r0.zzzz)+(r0.yyyy)).z;
    // 47: mul r0.w, cb0[3].x, cb0[3].y
    r0.w = ((source[3].xxxx)*(source[3].yyyy)).w;
    // 48: mul r1.xy, r0.wwww, cb0[5].yzyy
    r1.xy = ((r0.wwww)*(source[5].yzyy)).xy;
    // 49: mad r2.y, cb0[4].x, r0.z, r1.x
    r2.y = ((source[4].xxxx)*(r0.zzzz)+(r1.xxxx)).y;
    // 50: mul r0.z, r0.z, cb0[6].x
    r0.z = ((r0.zzzz)*(source[6].xxxx)).z;
    // 51: mad r3.y, r0.w, cb0[6].y, r0.z
    r3.y = ((r0.wwww)*(source[6].yyyy)+(r0.zzzz)).y;
    // 52: mad r3.x, cb0[5].w, r0.y, r1.y
    r3.x = ((source[5].wwww)*(r0.yyyy)+(r1.yyyy)).x;
    // 53: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 54: mad r2.x, r0.w, cb0[3].z, r0.y
    r2.x = ((r0.wwww)*(source[3].zzzz)+(r0.yyyy)).x;
    // 55: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t0.wxyz, s0, l(-1.000000)
    r0.yzw = (WarlordNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r3.xyxx, t1.xyzw, s1, l(-1.000000)
    r1.xyz = (WarlordNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 57: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 58: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: mad r0.yzw, -r0.yyzw, r1.xxyz, r1.wwww
    r0.yzw = ((-(r0.yyzw))*(r1.xxyz)+(r1.wwww)).yzw;
    // 60: mad r0.yzw, cb0[6].zzzz, r0.yyzw, r2.xxyz
    r0.yzw = ((source[6].zzzz)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 61: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 62: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 63: mul r0.yzw, r0.yyzw, cb0[6].wwww
    r0.yzw = ((r0.yyzw)*(source[6].wwww)).yzw;
    // 64: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 65: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 66: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 67: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 68: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 69: mad r1.x, -r0.x, cb0[7].z, l(1.000000)
    r1.x = ((-(r0.xxxx))*(source[7].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 70: mad r0.x, -r0.x, cb0[9].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[9].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 71: mul_sat r0.x, r0.x, cb0[10].y
    r0.x = (saturate((r0.xxxx)*(source[10].yyyy))).x;
    // 72: mul_sat r1.x, r1.x, cb0[8].z
    r1.x = (saturate((r1.xxxx)*(source[8].zzzz))).x;
    // 73: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 74: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 75: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 76: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 77: mul r0.x, r0.x, cb0[10].z
    r0.x = ((r0.xxxx)*(source[10].zzzz)).x;
    // 78: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 79: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 80: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 81: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 82: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 83: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 84: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 85: dp3 r1.y, v6.xyzx, v6.xyzx
    r1.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 86: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 87: mul r1.y, r1.y, v6.z
    r1.y = ((r1.yyyy)*(v6.zzzz)).y;
    // 88: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 89: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 90: mul r1.z, r1.z, cb0[11].y
    r1.z = ((r1.zzzz)*(source[11].yyyy)).z;
    // 91: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 92: mul_sat r1.z, r1.z, cb0[11].z
    r1.z = (saturate((r1.zzzz)*(source[11].zzzz))).z;
    // 93: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 94: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 95: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 96: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 97: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 98: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 99: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 WarlordNative2006(WARLORD_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[6u];
    source[2].x = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[2].y = (g_WarlordSourceMaterialParameters[4u].yyyy).x;
    source[2].z = g_WarlordSourceMaterialTime;
    source[2].w = (g_WarlordSourceMaterialParameters[3u].wwww).x;
    source[3].x = (g_WarlordSourceMaterialParameters[4u].xxxx).x;
    source[3].y = (g_WarlordSourceMaterialParameters[3u].xxxx).x;
    source[3].z = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[4].x = (g_WarlordSourceMaterialParameters[3u].yyyy).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_WarlordSourceMaterialParameters[4u].zzzz).x;
    source[4].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[5].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[4u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[5u].xxxx).x;
    source[6].y = (g_WarlordSourceMaterialParameters[3u].zzzz).x;
    source[6].z = max(9.99999975e-06,(g_WarlordSourceMaterialParameters[3u].zzzz).x);
    source[6].w = (1.0/max(9.99999975e-06,(g_WarlordSourceMaterialParameters[3u].zzzz).x));
    source[7].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
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
    // 24: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 25: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 26: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 27: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 28: mad r0.x, r0.z, l(0.159155), l(0.500000)
    r0.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 29: mul r0.zw, v2.xxxy, cb0[3].zzzw
    r0.zw = ((v2.xxxy)*(source[3].zzzw)).zw;
    // 30: mad r1.x, cb0[2].z, cb0[3].y, r0.z
    r1.x = ((source[2].zzzz)*(source[3].yyyy)+(r0.zzzz)).x;
    // 31: mad r1.y, cb0[2].z, cb0[4].x, r0.w
    r1.y = ((source[2].zzzz)*(source[4].xxxx)+(r0.wwww)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (WarlordNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 33: mul r0.w, v4.w, cb0[4].y
    r0.w = ((v4.wwww)*(source[4].yyyy)).w;
    // 34: mad r0.xy, r0.zzzz, r0.wwww, r0.xyxx
    r0.xy = ((r0.zzzz)*(r0.wwww)+(r0.xyxx)).xy;
    // 35: mad r0.zw, r0.zzzz, r0.wwww, v2.xxxy
    r0.zw = ((r0.zzzz)*(r0.wwww)+(v2.xxxy)).zw;
    // 36: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 37: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 38: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 39: mad r0.z, -r0.z, cb0[6].w, l(1.000000)
    r0.z = ((-(r0.zzzz))*(source[6].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 40: mul r0.w, r0.x, cb0[2].w
    r0.w = ((r0.xxxx)*(source[2].wwww)).w;
    // 41: mad r1.x, cb0[2].z, cb0[2].y, r0.w
    r1.x = ((source[2].zzzz)*(source[2].yyyy)+(r0.wwww)).x;
    // 42: mul r0.w, cb0[2].z, cb0[4].z
    r0.w = ((source[2].zzzz)*(source[4].zzzz)).w;
    // 43: mad r1.y, cb0[3].x, r0.y, r0.w
    r1.y = ((source[3].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 44: mul r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)*(source[5].xyxx)).xy;
    // 45: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s0, cb0[2].x
    r1.xyz = (WarlordNativeSample0((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 46: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 47: add r0.w, r1.z, r0.w
    r0.w = ((r1.zzzz)+(r0.wwww)).w;
    // 48: mad r1.x, cb0[2].z, cb0[4].w, r0.x
    r1.x = ((source[2].zzzz)*(source[4].wwww)+(r0.xxxx)).x;
    // 49: mad r1.y, cb0[2].z, cb0[5].z, r0.y
    r1.y = ((source[2].zzzz)*(source[5].zzzz)+(r0.yyyy)).y;
    // 50: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, cb0[2].x
    r1.xyz = (WarlordNativeSample2((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 51: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 52: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 53: mul r0.x, r0.x, l(0.330000)
    r0.x = ((r0.xxxx)*(float4(0.330000,0.330000,0.330000,0.330000))).x;
    // 54: mad r0.x, r0.w, l(0.330000), r0.x
    r0.x = ((r0.wwww)*(float4(0.330000,0.330000,0.330000,0.330000))+(r0.xxxx)).x;
    // 55: mul r0.x, r0.x, cb0[5].w
    r0.x = ((r0.xxxx)*(source[5].wwww)).x;
    // 56: mul_sat r0.x, r0.x, l(0.500000)
    r0.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 57: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 58: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 59: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 60: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 61: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 62: mul r0.yw, v4.yyyx, cb0[7].xxxz
    r0.yw = ((v4.yyyx)*(source[7].xxxz)).yw;
    // 63: mov_sat r0.y, r0.y
    r0.y = (saturate(r0.yyyy)).y;
    // 64: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 66: max r0.y, r0.y, l(0.000010)
    r0.y = (max(r0.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 67: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 68: mul_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)*(r0.zzzz))).y;
    // 69: mul_sat r0.z, r0.y, cb0[7].y
    r0.z = (saturate((r0.yyyy)*(source[7].yyyy))).z;
    // 70: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 71: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 72: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 73: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 75: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 76: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 77: mov_sat r0.z, v4.z
    r0.z = (saturate(v4.zzzz)).z;
    // 78: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 79: mad_sat r0.x, r0.x, r0.y, -r0.z
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(-(r0.zzzz)))).x;
    // 80: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 81: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 82: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 83: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 84: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 85: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 WarlordNative2007(WARLORD_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[2u];
    source[2].x = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[3].y = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[3].w = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x));
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = (WarlordNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r0.zw, -r0.xxxy, v2.xxxy
    r0.zw = ((-(r0.xxxy))+(v2.xxxy)).zw;
    // 3: mad r0.xy, v4.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((v4.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 4: mul r0.zw, r0.xxxy, cb0[2].zzzz
    r0.zw = ((r0.xxxy)*(source[2].zzzz)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (WarlordNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mul_sat r0.x, r0.x, cb0[2].x
    r0.x = (saturate((r0.xxxx)*(source[2].xxxx))).x;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t3.yxzw, s4, l(0.000000)
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 23: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 24: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 25: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 26: source device depth mapped to centimetre view depth; reconstruction at 28.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 28-31: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 32: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 33: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 34: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 35: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 36: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 37: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 39: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 40: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 WarlordNative2008(WARLORD_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier; source additive selection/highlight prefix is zero.
    float4 output=0.f;
    source[1] = g_WarlordSourceMaterialParameters[4u];
    source[2] = g_WarlordSourceMaterialParameters[3u];
    source[3].x = (g_WarlordSourceMaterialParameters[0u].wwww).x;
    source[3].y = g_WarlordSourceMaterialTime;
    source[3].z = (g_WarlordSourceMaterialParameters[0u].yyyy).x;
    source[3].w = (g_WarlordSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_WarlordSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_WarlordSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_WarlordSourceMaterialParameters[2u].yyyy).x;
    source[4].w = (g_WarlordSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_WarlordSourceMaterialParameters[1u].zzzz).x;
    source[5].y = (g_WarlordSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_WarlordSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_WarlordSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_WarlordSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x);
    source[6].z = (0.00999999978*(100.0-(g_WarlordSourceMaterialParameters[0u].xxxx).x));
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
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[6].z, l(1.000000)
    r0.y = ((-(source[6].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v2.xyxx, t1.zxyw, s1, l(0.000000)
    r0.yz = (WarlordNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 15: add r1.xyzw, -r0.yzyz, v2.xyxy
    r1.xyzw = ((-(r0.yzyz))+(v2.xyxy)).xyzw;
    // 16: mad r1.xyzw, v4.xxyy, r1.xyzw, r0.yzyz
    r1.xyzw = ((v4.xxyy)*(r1.xyzw)+(r0.yzyz)).xyzw;
    // 17: mul r0.yz, r1.xxyx, cb0[5].yyyy
    r0.yz = ((r1.xxyx)*(source[5].yyyy)).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.yxzw, s5, l(0.000000)
    r0.y = (WarlordNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 19: mul_sat r0.y, r0.y, cb0[5].z
    r0.y = (saturate((r0.yyyy)*(source[5].zzzz))).y;
    // 20: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 21: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 22: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s4, l(0.000000)
    r0.z = (WarlordNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 26: mul r1.xy, r1.zwzz, cb0[3].zwzz
    r1.xy = ((r1.zwzz)*(source[3].zwzz)).xy;
    // 27: mul_sat r0.z, r0.z, cb0[4].w
    r0.z = (saturate((r0.zzzz)*(source[4].wwww))).z;
    // 28: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 29: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 30: mul r0.w, r0.w, cb0[5].x
    r0.w = ((r0.wwww)*(source[5].xxxx)).w;
    // 31: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 32: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 33: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 34: mad r0.y, -r0.z, r0.y, r0.z
    r0.y = ((-(r0.zzzz))*(r0.yyyy)+(r0.zzzz)).y;
    // 35: mad r0.y, v4.z, r0.y, r0.w
    r0.y = ((v4.zzzz)*(r0.yyyy)+(r0.wwww)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t0.yzxw, s3, l(0.000000)
    r0.z = (WarlordNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 38: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 39: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 40: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 41: mad r2.x, cb0[3].y, cb0[3].x, r1.x
    r2.x = ((source[3].yyyy)*(source[3].xxxx)+(r1.xxxx)).x;
    // 42: mad r2.y, cb0[3].y, cb0[4].x, r1.y
    r2.y = ((source[3].yyyy)*(source[4].xxxx)+(r1.yyyy)).y;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t5.wxyz, s2, l(0.000000)
    r0.yzw = (WarlordNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 44: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 45: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 46: mad r0.yzw, cb0[4].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 47: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 48: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 49: mul r0.yzw, r0.yyzw, cb0[4].zzzz
    r0.yzw = ((r0.yyzw)*(source[4].zzzz)).yzw;
    // 50: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 51: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 52: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 53: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 54: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 55: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 56: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
