// Vehicle skeletal model-cue native programs (skinned carrier only).
// mn_pmstg_00-2_aa_mi: 5f33bef7c823444d8983ab12adf5b7bb; selected map 1eeff6dab024c9dec0da1cdb50741f0ff1405bcd71358f679f64c9397cd93e1a.
float4 ArtistNative3828(ARTIST_NATIVE_INPUT input)
{
    float4 source[26]; [unroll] for (uint i=0u; i<26u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.color.a,0.f,0.f,0.f);
    source[23]=float4(input.skyUpperColor,0.f);
    source[24]=float4(input.skyLowerColor,0.f);
    source[25]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_ArtistSourceMaterialParameters[13u];
    source[2] = g_ArtistSourceMaterialParameters[16u];
    source[3] = g_ArtistSourceMaterialParameters[3u];
    source[4] = g_ArtistSourceMaterialParameters[12u];
    source[5] = g_ArtistSourceMaterialParameters[5u];
    source[6] = g_ArtistSourceMaterialParameters[6u];
    source[7] = g_ArtistSourceMaterialParameters[7u];
    source[8] = g_ArtistSourceMaterialParameters[8u];
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[11] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[12] = g_ArtistSourceMaterialParameters[10u];
    source[13] = g_ArtistSourceMaterialParameters[9u];
    source[14] = g_ArtistSourceMaterialParameters[15u];
    source[15] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[16] = g_ArtistSourceMaterialParameters[11u];
    source[17] = g_ArtistSourceMaterialParameters[14u];
    source[18].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[18].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[18].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[19].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0)))).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[20].x = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[20].y = ((g_ArtistSourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[20].z = (((g_ArtistSourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))*((float4(1.5, 0.0, 0.0, 0.0)+sin(((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0)))).x;
    source[20].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[21].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[21].y = ((float4(1000.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[21].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[21].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    float4 passValues[5]; [unroll] for(uint passIndex=0u;passIndex<5u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override; only the original secondary MRT consumes it.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v8 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v9 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 2: mul r0.x, r0.x, l(0.125000)
    r0.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 3: mul r1.y, cb0[14].y, cb0[15].y
    r1.y = ((source[14].yyyy)*(source[15].yyyy)).y;
    // 4: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 5: mov r1.xw, l(0,0,0,0)
    r1.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 6: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 7: frc r0.z, cb0[14].x
    r0.z = (frac(source[14].xxxx)).z;
    // 8: add r0.w, -r0.z, cb0[14].x
    r0.w = ((-(r0.zzzz))+(source[14].xxxx)).w;
    // 9: mul r1.z, r0.w, l(0.125000)
    r1.z = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 10: add r0.xy, r0.xyxx, r1.zwzz
    r0.xy = ((r0.xyxx)+(r1.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyzw = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 12: mul r0.x, r0.z, r1.w
    r0.x = ((r0.zzzz)*(r1.wwww)).x;
    // 13: add r0.y, -cb0[14].w, l(1.000000)
    r0.y = ((-(source[14].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: mul r0.y, r0.y, cb0[18].z
    r0.y = ((r0.yyyy)*(source[18].zzzz)).y;
    // 15: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 16: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 17: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 18: mul r0.w, cb0[14].z, l(1.500000)
    r0.w = ((source[14].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 19: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 20: mad r0.y, r0.y, l(0.500000), cb0[14].z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[14].zzzz)).y;
    // 21: mul r1.xyz, r1.xyzx, r0.yyyy
    r1.xyz = ((r1.xyzx)*(r0.yyyy)).xyz;
    // 22: mad r2.xyz, cb0[12].wwww, cb0[12].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[12].wwww)*(source[12].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 23: mad r3.xyz, cb0[13].wwww, cb0[13].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[13].wwww)*(source[13].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 24: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 25: add r3.xyzw, -cb0[5].xyzw, cb0[6].xyzw
    r3.xyzw = ((-(source[5].xyzw))+(source[6].xyzw)).xyzw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 27: mad r3.xyzw, r4.xxxx, r3.xyzw, cb0[5].xyzw
    r3.xyzw = ((r4.xxxx)*(r3.xyzw)+(source[5].xyzw)).xyzw;
    // 28: add r5.xyzw, -r3.xyzw, cb0[7].xyzw
    r5.xyzw = ((-(r3.xyzw))+(source[7].xyzw)).xyzw;
    // 29: mad r3.xyzw, r4.yyyy, r5.xyzw, r3.xyzw
    r3.xyzw = ((r4.yyyy)*(r5.xyzw)+(r3.xyzw)).xyzw;
    // 30: add r5.xyzw, -r3.xyzw, cb0[8].xyzw
    r5.xyzw = ((-(r3.xyzw))+(source[8].xyzw)).xyzw;
    // 31: mad r3.xyzw, r4.zzzz, r5.xyzw, r3.xyzw
    r3.xyzw = ((r4.zzzz)*(r5.xyzw)+(r3.xyzw)).xyzw;
    // 32: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 34: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 35: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 36: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 37: mul r6.xyz, r0.yyyy, v6.xyzx
    r6.xyz = ((r0.yyyy)*(v6.xyzx)).xyz;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 39: mad r7.xy, r0.ywyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r0.ywyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r0.y, r7.xyxx, r7.xyxx
    r0.y = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).y;
    // 41: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 42: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 43: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 44: add r7.z, r0.y, l(0.000010)
    r7.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: dp3 r0.y, r7.xyzx, r6.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 46: max r0.w, r6.z, l(0.000000)
    r0.w = (max(r6.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 47: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 48: add r0.yw, -r0.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = ((-(r0.yyyw))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 49: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 50: mul r1.w, r1.w, l(0.800000)
    r1.w = ((r1.wwww)*(float4(0.800000,0.800000,0.800000,0.800000))).w;
    // 51: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 52: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: lt r2.w, |r0.y|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 54: movc r1.w, r2.w, l(1.000000), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r1.wwww)).w;
    // 55: add r6.xy, v4.xyxx, cb0[10].xyxx
    r6.xy = ((v4.xyxx)+(source[10].xyxx)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyz = (ArtistNativeSample3((r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 57: add r8.xy, v4.xyxx, cb0[11].xyxx
    r8.xy = ((v4.xyxx)+(source[11].xyxx)).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, r8.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyz = (ArtistNativeSample3((r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 59: add r6.xyzw, r6.xxyz, r8.xxyz
    r6.xyzw = ((r6.xxyz)+(r8.xxyz)).xyzw;
    // 60: mul r8.xyz, r6.yzwy, cb0[9].xyzx
    r8.xyz = ((r6.yzwy)*(source[9].xyzx)).xyz;
    // 61: mad r5.xyz, r1.wwww, r8.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 62: mul r8.xyz, r1.wwww, cb0[16].xyzx
    r8.xyz = ((r1.wwww)*(source[16].xyzx)).xyz;
    // 63: mul r1.w, |r0.y|, |r0.y|
    r1.w = ((abs(r0.yyyy))*(abs(r0.yyyy))).w;
    // 64: mul r0.y, |r0.y|, r1.w
    r0.y = ((abs(r0.yyyy))*(r1.wwww)).y;
    // 65: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 66: movc r0.y, r2.w, l(0), r0.y
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 67: mul r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)*(r1.wwww)).xyz;
    // 68: log r1.w, r0.y
    r1.w = (log2(r0.yyyy)).w;
    // 69: mul r1.w, r1.w, cb0[18].x
    r1.w = ((r1.wwww)*(source[18].xxxx)).w;
    // 70: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 71: mul r9.xyz, r1.wwww, cb0[4].xyzx
    r9.xyz = ((r1.wwww)*(source[4].xyzx)).xyz;
    // 72: mul r9.xyz, r9.xyzx, cb0[18].yyyy
    r9.xyz = ((r9.xyzx)*(source[18].yyyy)).xyz;
    // 73: lt r1.w, r0.y, l(0.000001)
    r1.w = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: movc r9.xyz, r1.wwww, l(0,0,0,0), r9.xyzx
    r9.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xyzx)).xyz;
    // 75: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 76: mad r4.xyz, r4.xyzx, r3.xyzx, -r5.xyzx
    r4.xyz = ((r4.xyzx)*(r3.xyzx)+(-(r5.xyzx))).xyz;
    // 77: mad r4.xyz, r4.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r5.xyzx)).xyz;
    // 78: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 79: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 81: mad r3.xyz, cb0[19].yyyy, r3.xyzx, r5.xyzx
    r3.xyz = ((source[19].yyyy)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 82: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 84: mad r3.xyz, cb0[19].zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((source[19].zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 85: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 86: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 87: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 88: add r2.xyz, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = ((r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 89: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 90: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 91: div r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 92: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 94: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 95: mad r3.xyz, r0.yyyy, cb0[3].xyzx, -cb0[3].xyzx
    r3.xyz = ((r0.yyyy)*(source[3].xyzx)+(-(source[3].xyzx))).xyz;
    // 96: mad r3.xyz, cb0[3].wwww, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((source[3].wwww)*(r3.xyzx)+(source[3].xyzx)).xyz;
    // 97: mul r0.x, r0.y, cb0[20].z
    r0.x = ((r0.yyyy)*(source[20].zzzz)).x;
    // 98: mad r4.xyz, r0.yyyy, cb0[2].xyzx, -cb0[2].xyzx
    r4.xyz = ((r0.yyyy)*(source[2].xyzx)+(-(source[2].xyzx))).xyz;
    // 99: mad r4.xyz, cb0[2].wwww, r4.xyzx, cb0[2].xyzx
    r4.xyz = ((source[2].wwww)*(r4.xyzx)+(source[2].xyzx)).xyz;
    // 100: mad r2.xyz, r0.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 101: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 102: mad r2.xyz, r8.xyzx, l(1.200000, 1.200000, 1.200000, 0.000000), r2.xyzx
    r2.xyz = ((r8.xyzx)*(float4(1.200000,1.200000,1.200000,0.000000))+(r2.xyzx)).xyz;
    // 103: add r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 104: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 105: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 106: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 107: dp3 r0.x, r7.xyzx, r7.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 108: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 109: mul r3.xyz, r0.xxxx, r7.xyzx
    r3.xyz = ((r0.xxxx)*(r7.xyzx)).xyz;
    // 110: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 111: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 112: mul r4.xyz, r0.xxxx, v7.xyzx
    r4.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 113: dp3 r0.x, r4.xyzx, r3.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 114: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 115: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 116: mul r4.xyz, r0.yyyy, cb0[24].xyzx
    r4.xyz = ((r0.yyyy)*(source[24].xyzx)).xyz;
    // 117: mad r4.xyz, r0.xxxx, cb0[23].xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(source[23].xyzx)+(r4.xyzx)).xyz;
    // 118: mul r4.xyz, r4.xyzx, cb0[25].wwww
    r4.xyz = ((r4.xyzx)*(source[25].wwww)).xyz;
    // 119: mad r2.xyz, r4.xyzx, r1.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 120: mul r4.xyz, r1.xyzx, r4.xyzx
    r4.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 122: mad r2.xyz, r1.xyzx, cb0[25].xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(source[25].xyzx)+(r2.xyzx)).xyz;
    // 124: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 125: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 126: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 127: mul r0.x, r0.x, l(0.300000)
    r0.x = ((r0.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))).x;
    // 128: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 129: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 130: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 131: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 132: mul r0.x, r0.x, cb0[20].w
    r0.x = ((r0.xxxx)*(source[20].wwww)).x;
    // 133: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 134: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: movc r0.x, r0.y, l(1.000000), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.xxxx)).x;
    // 136: add r0.x, -r6.x, r0.x
    r0.x = ((-(r6.xxxx))+(r0.xxxx)).x;
    // 137: mad r0.x, r0.x, l(0.800000), r6.x
    r0.x = ((r0.xxxx)*(float4(0.800000,0.800000,0.800000,0.800000))+(r6.xxxx)).x;
    // 138: mul r0.x, r0.x, r4.w
    r0.x = ((r0.xxxx)*(r4.wwww)).x;
    // 139: add r0.y, -cb0[21].x, cb0[21].y
    r0.y = ((-(source[21].xxxx))+(source[21].yyyy)).y;
    // 140: mad r0.y, r0.z, r0.y, cb0[21].x
    r0.y = ((r0.zzzz)*(r0.yyyy)+(source[21].xxxx)).y;
    // 141: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 142: mul_sat r0.x, r0.x, cb0[21].w
    r0.x = (saturate((r0.xxxx)*(source[21].wwww))).x;
    // 143: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
// mn_pmstg_00.mat.mn_pmstg_00-2_aa_mi: directional light PS 1a678e8069300a4892da665e175a9a1f; parameter rows reuse program 3828 packing.
float4 ArtistNative3828Light(ARTIST_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0]=float4(input.color.a,0.f,0.f,0.f);
    source[19]=float4(lightColor,1.f);
    source[1] = g_ArtistSourceMaterialParameters[13u];
    source[2] = g_ArtistSourceMaterialParameters[12u];
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = g_ArtistSourceMaterialParameters[7u];
    source[6] = g_ArtistSourceMaterialParameters[8u];
    source[7] = g_ArtistSourceMaterialParameters[4u];
    source[8] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ArtistSourceMaterialParameters[10u];
    source[11] = g_ArtistSourceMaterialParameters[9u];
    source[12] = g_ArtistSourceMaterialParameters[15u];
    source[13] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[14] = g_ArtistSourceMaterialParameters[14u];
    source[15].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[15].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[16].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0)))).x;
    source[16].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[17].x = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[17].y = ((g_ArtistSourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[17].z = (((g_ArtistSourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))*((float4(1.5, 0.0, 0.0, 0.0)+sin(((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0)))).x;
    source[17].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[18].y = ((float4(1000.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[18].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[18].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    float4 passValues[5]; [unroll] for(uint passIndex=0u;passIndex<5u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override, as program 3828.
    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override, as program 3828.
    float4 v2 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v3 = float4(tangentLight,0.f); // native tangent light vector
    float4 v5 = float4(input.tangentView,0.f); // native tangent camera vector
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
    float4 output=0.f;
    // 1: add r0.x, -cb0[12].w, l(1.000000)
    r0.x = ((-(source[12].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: mul r0.x, r0.x, cb0[15].z
    r0.x = ((r0.xxxx)*(source[15].zzzz)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: mul r0.y, cb0[12].z, l(1.500000)
    r0.y = ((source[12].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 7: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 8: mad r0.x, r0.x, l(0.500000), cb0[12].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[12].zzzz)).x;
    // 9: frc r0.y, v2.x
    r0.y = (frac(v2.xxxx)).y;
    // 10: mul r1.x, r0.y, l(0.125000)
    r1.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 11: mul r2.y, cb0[12].y, cb0[13].y
    r2.y = ((source[12].yyyy)*(source[13].yyyy)).y;
    // 12: mov r1.y, v2.y
    r1.y = (v2.yyyy).y;
    // 13: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 14: add r0.yz, r1.xxyx, r2.xxyx
    r0.yz = ((r1.xxyx)+(r2.xxyx)).yz;
    // 15: frc r0.w, cb0[12].x
    r0.w = (frac(source[12].xxxx)).w;
    // 16: add r1.x, -r0.w, cb0[12].x
    r1.x = ((-(r0.wwww))+(source[12].xxxx)).x;
    // 17: mul r2.z, r1.x, l(0.125000)
    r2.z = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 18: add r0.yz, r0.yyzy, r2.zzwz
    r0.yz = ((r0.yyzy)+(r2.zzwz)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r1.xyzw = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 20: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 21: mul r1.x, r0.w, r1.w
    r1.x = ((r0.wwww)*(r1.wwww)).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, v2.xyxx, t0.zxyw, s0, l(0.000000)
    r1.yz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 23: mad r2.xy, r1.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r1.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: dp2 r1.y, r2.xyxx, r2.xyxx
    r1.y = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 25: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 27: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 28: add r2.z, r1.y, l(0.000010)
    r2.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 29: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 30: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 31: mul r1.yzw, r1.yyyy, v5.xxyz
    r1.yzw = ((r1.yyyy)*(v5.xxyz)).yzw;
    // 32: dp3 r2.w, r2.xyzx, r1.yzwy
    r2.w = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 33: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: mul r3.x, |r2.w|, |r2.w|
    r3.x = ((abs(r2.wwww))*(abs(r2.wwww))).x;
    // 36: mul r3.x, |r2.w|, r3.x
    r3.x = ((abs(r2.wwww))*(r3.xxxx)).x;
    // 37: lt r3.y, |r2.w|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 38: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 39: mul r2.w, r2.w, l(0.800000)
    r2.w = ((r2.wwww)*(float4(0.800000,0.800000,0.800000,0.800000))).w;
    // 40: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 41: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: movc r2.w, r3.y, l(1.000000), r2.w
    r2.w = ((asuint(r3.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r2.wwww)).w;
    // 43: movc r3.x, r3.y, l(0), r3.x
    r3.x = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 44: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 45: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 46: mul r3.y, r3.y, cb0[15].x
    r3.y = ((r3.yyyy)*(source[15].xxxx)).y;
    // 47: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 48: mul r3.yzw, r3.yyyy, cb0[2].xxyz
    r3.yzw = ((r3.yyyy)*(source[2].xxyz)).yzw;
    // 49: mul r3.yzw, r3.yyzw, cb0[15].yyyy
    r3.yzw = ((r3.yyzw)*(source[15].yyyy)).yzw;
    // 50: movc r3.xyz, r3.xxxx, l(0,0,0,0), r3.yzwy
    r3.xyz = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yzwy)).xyz;
    // 51: add r4.xyzw, -cb0[3].xyzw, cb0[4].xyzw
    r4.xyzw = ((-(source[3].xyzw))+(source[4].xyzw)).xyzw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v2.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mad r4.xyzw, r5.xxxx, r4.xyzw, cb0[3].xyzw
    r4.xyzw = ((r5.xxxx)*(r4.xyzw)+(source[3].xyzw)).xyzw;
    // 54: add r6.xyzw, -r4.xyzw, cb0[5].xyzw
    r6.xyzw = ((-(r4.xyzw))+(source[5].xyzw)).xyzw;
    // 55: mad r4.xyzw, r5.yyyy, r6.xyzw, r4.xyzw
    r4.xyzw = ((r5.yyyy)*(r6.xyzw)+(r4.xyzw)).xyzw;
    // 56: add r6.xyzw, -r4.xyzw, cb0[6].xyzw
    r6.xyzw = ((-(r4.xyzw))+(source[6].xyzw)).xyzw;
    // 57: mad r4.xyzw, r5.zzzz, r6.xyzw, r4.xyzw
    r4.xyzw = ((r5.zzzz)*(r6.xyzw)+(r4.xyzw)).xyzw;
    // 58: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r5.xyzw = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 60: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 61: add r7.xy, v2.xyxx, cb0[8].xyxx
    r7.xy = ((v2.xyxx)+(source[8].xyxx)).xy;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t1.xyzw, s3, l(0.000000)
    r7.xyz = (ArtistNativeSample3((r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 63: add r8.xy, v2.xyxx, cb0[9].xyxx
    r8.xy = ((v2.xyxx)+(source[9].xyxx)).xy;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, r8.xyxx, t1.xyzw, s3, l(0.000000)
    r8.xyz = (ArtistNativeSample3((r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 65: add r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)+(r8.xyzx)).xyz;
    // 66: mul r8.xyz, r7.xyzx, cb0[7].xyzx
    r8.xyz = ((r7.xyzx)*(source[7].xyzx)).xyz;
    // 67: mad r6.xyz, r2.wwww, r8.xyzx, r6.xyzx
    r6.xyz = ((r2.wwww)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 68: add r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)+(r6.xyzx)).xyz;
    // 69: mad r5.xyz, r5.xyzx, r4.xyzx, -r3.xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(-(r3.xyzx))).xyz;
    // 70: mad r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r3.xyzx)).xyz;
    // 71: mul r5.xyz, r4.xyzx, r3.xyzx
    r5.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 72: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r3.xyz, -r3.xyzx, r4.xyzx, r2.wwww
    r3.xyz = ((-(r3.xyzx))*(r4.xyzx)+(r2.wwww)).xyz;
    // 74: mad r3.xyz, cb0[16].yyyy, r3.xyzx, r5.xyzx
    r3.xyz = ((source[16].yyyy)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 75: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 76: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 77: mad r3.xyz, cb0[16].zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 78: mad r4.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: mad r5.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 81: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 82: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 83: mad r0.xyz, r1.xxxx, r0.xyzx, r3.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 84: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 86: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 87: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 88: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 89: mul r2.xyz, r1.xxxx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // 90: dp3 r1.x, r2.xyzx, r1.yzwy
    r1.x = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 91: mul r3.xyz, r1.xxxx, r2.xyzx
    r3.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // 92: mad r1.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.yzwy
    r1.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.yzwy))).xyz;
    // 93: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 94: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: dp3 r2.w, v3.xyzx, v3.xyzx
    r2.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 96: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 97: mul r3.xyz, r2.wwww, v3.xyzx
    r3.xyz = ((r2.wwww)*(v3.xyzx)).xyz;
    // 98: dp3_sat r1.x, r1.xyzx, r3.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 99: dp3_sat r1.y, r2.xyzx, r3.xyzx
    r1.y = (saturate(dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx)).y;
    // 100: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 101: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 102: mul r1.x, r1.x, l(15.000000)
    r1.x = ((r1.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 103: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 104: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 105: mul r2.xyz, r7.xyzx, cb0[14].xyzx
    r2.xyz = ((r7.xyzx)*(source[14].xyzx)).xyz;
    // 106: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 107: mad r3.xyz, -r7.xyzx, cb0[14].xyzx, r1.zzzz
    r3.xyz = ((-(r7.xyzx))*(source[14].xyzx)+(r1.zzzz)).xyz;
    // 108: mad r2.xyz, cb0[16].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 109: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 110: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 111: mad r2.xyz, cb0[16].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 112: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 113: mul r2.xyz, r1.xxxx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // 114: lt r1.x, r1.y, l(0.000001)
    r1.x = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 115: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 116: mad r0.xyz, r0.xyzx, r1.xxxx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)+(r2.xyzx)).xyz;
    // 117: mul o0.xyz, r0.xyzx, cb0[19].xyzx
    output.xyz = ((r0.xyzx)*(source[19].xyzx)).xyz;
    // 118: log r0.x, |r1.w|
    r0.x = (log2(abs(r1.wwww))).x;
    // 119: lt r0.y, |r1.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 120: mul r0.x, r0.x, l(0.300000)
    r0.x = ((r0.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))).x;
    // 121: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 122: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 123: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 124: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 125: mul r0.x, r0.x, cb0[17].w
    r0.x = ((r0.xxxx)*(source[17].wwww)).x;
    // 126: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 127: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 128: movc r0.x, r0.y, l(1.000000), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.xxxx)).x;
    // 129: add r0.x, -r7.x, r0.x
    r0.x = ((-(r7.xxxx))+(r0.xxxx)).x;
    // 130: mad r0.x, r0.x, l(0.800000), r7.x
    r0.x = ((r0.xxxx)*(float4(0.800000,0.800000,0.800000,0.800000))+(r7.xxxx)).x;
    // 131: mul r0.x, r0.x, r5.w
    r0.x = ((r0.xxxx)*(r5.wwww)).x;
    // 132: add r0.y, -cb0[18].x, cb0[18].y
    r0.y = ((-(source[18].xxxx))+(source[18].yyyy)).y;
    // 133: mad r0.y, r0.w, r0.y, cb0[18].x
    r0.y = ((r0.wwww)*(r0.yyyy)+(source[18].xxxx)).y;
    // 134: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 135: mul_sat r0.x, r0.x, cb0[18].w
    r0.x = (saturate((r0.xxxx)*(source[18].wwww))).x;
    // 136: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 138: ret
    return output;
}
