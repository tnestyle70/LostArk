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
// sk_admg_00_mi_dead: 79c838f04b955f4e86b5b185a9599f1f; selected map a83346c0684637c27c9595d859e8b713513c26aeb49b2a8063bc28f6bb0dd63f.
float4 ArtistNative3831(ARTIST_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing scene adapter: source world origin is absolute; camera is converted to source cm.
    source[0]=0.f;
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[1].w=input.color.a;
    source[29]=float4(input.skyUpperColor,0.f);
    source[30]=float4(input.skyLowerColor,0.f);
    source[31]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_ArtistSourceMaterialParameters[23u];
    source[3] = g_ArtistSourceMaterialParameters[24u];
    source[4] = g_ArtistSourceMaterialParameters[12u];
    source[5] = g_ArtistSourceMaterialParameters[13u];
    source[6] = g_ArtistSourceMaterialParameters[14u];
    source[7] = g_ArtistSourceMaterialParameters[15u];
    source[8] = g_ArtistSourceMaterialParameters[17u];
    source[9] = g_ArtistSourceMaterialParameters[16u];
    source[10] = g_ArtistSourceMaterialParameters[27u];
    source[11] = g_ArtistSourceMaterialParameters[21u];
    source[12] = g_ArtistSourceMaterialParameters[22u];
    source[13] = g_ArtistSourceMaterialParameters[26u];
    source[14] = g_ArtistSourceMaterialParameters[10u];
    source[15] = g_ArtistSourceMaterialParameters[11u];
    source[16] = g_ArtistSourceMaterialParameters[20u];
    source[17] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[18] = g_ArtistSourceMaterialParameters[25u];
    source[19] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[20] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[21].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[21].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[21].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[21].w = ((g_ArtistSourceMaterialParameters[6u].xxxx*g_ArtistSourceMaterialParameters[8u].zzzz)).x;
    source[22].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[22].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[22].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[22].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[23].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[23].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[23].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[23].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[24].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[24].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[24].z = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[24].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[25].x = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[25].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[25].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[25].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[26].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[26].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[26].z = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[26].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[27].x = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[27].y = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[27].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[27].w = (sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[28].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))))).x;
    source[28].y = (cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[28].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[28].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[28].zzzz
    r0.xy = ((v4.xyxx)*(source[28].zzzz)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.xyzw, s6, l(0.000000)
    r0.x = (ArtistNativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[28].w
    r0.x = ((r0.xxxx)+(-(source[28].wwww))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 8: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 9: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 10: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 11: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 13: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 14: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 15: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 16: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 17: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 18: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 19: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 20: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 21: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 22: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 25: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 27: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 29: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 30: add r1.w, -cb0[22].y, cb0[22].x
    r1.w = ((-(source[22].yyyy))+(source[22].xxxx)).w;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 32: mad r1.w, r3.w, r1.w, cb0[22].y
    r1.w = ((r3.wwww)*(r1.wwww)+(source[22].yyyy)).w;
    // 33: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 34: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 35: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 36: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 37: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 38: add r1.w, -r0.w, cb0[23].y
    r1.w = ((-(r0.wwww))+(source[23].yyyy)).w;
    // 39: mad r0.w, r3.w, r1.w, r0.w
    r0.w = ((r3.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 40: mul r0.w, r0.w, cb0[23].z
    r0.w = ((r0.wwww)*(source[23].zzzz)).w;
    // 41: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 42: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 43: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 44: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 46: mad r4.xyzw, r0.ywyw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r0.ywyw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 47: dp2 r0.y, r4.zwzz, r4.zwzz
    r0.y = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).y;
    // 48: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 50: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 51: add r5.z, r0.y, l(0.000010)
    r5.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: mul r5.xy, r4.xyxx, cb0[21].xxxx
    r5.xy = ((r4.xyxx)*(source[21].xxxx)).xy;
    // 53: mad r4.xy, cb0[21].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[21].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 54: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 55: mad r4.xyz, r3.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 56: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 57: mad r5.xyz, cb0[23].xxxx, r5.xyzx, r4.xyzx
    r5.xyz = ((source[23].xxxx)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 58: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 59: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 60: div r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)/(r0.yyyy)).xyz;
    // 61: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 64: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 65: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 66: mul r7.xyz, r0.yyyy, v0.xyzx
    r7.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 67: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 68: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 69: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 70: dp3 r9.y, r8.xyzx, r5.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 71: dp3 r9.x, r7.xyzx, r5.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 72: dp3 r9.z, r6.xyzx, r5.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 73: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 74: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 75: mul r5.xyz, r0.yyyy, v5.xyzx
    r5.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 76: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 77: dp3 r11.y, r8.xyzx, r5.xyzx
    r11.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 78: dp3 r11.x, r7.xyzx, r5.xyzx
    r11.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 79: dp3 r11.z, r6.xyzx, r5.xyzx
    r11.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 80: dp3 r0.y, r9.xyzx, r11.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 81: mul r9.xyz, r9.xyzx, r0.yyyy
    r9.xyz = ((r9.xyzx)*(r0.yyyy)).xyz;
    // 82: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 83: mov r9.w, -r9.x
    r9.w = (-(r9.xxxx)).w;
    // 84: dp2 r0.y, r9.ywyy, r9.ywyy
    r0.y = (dot((r9.ywyy).xy,(r9.ywyy).xy).xxxx).y;
    // 85: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 86: div r0.yw, r9.yyyw, r0.yyyy
    r0.yw = ((r9.yyyw)/(r0.yyyy)).yw;
    // 87: mad r1.w, -r9.z, l(0.250000), l(0.250000)
    r1.w = ((-(r9.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 88: add r2.w, r9.z, l(1.000000)
    r2.w = ((r9.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 90: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 91: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = (ArtistNativeSample4((r0.ywyy).xy, (r0.xxxx).x, true).xywz).xyw;
    // 92: log r9.xyz, r0.xywx
    r9.xyz = (log2(r0.xywx)).xyz;
    // 93: rcp r1.w, cb0[23].w
    r1.w = (1.0/(source[23].wwww)).w;
    // 94: mul r11.xyz, r9.xyzx, r1.wwww
    r11.xyz = ((r9.xyzx)*(r1.wwww)).xyz;
    // 95: mul r9.xyz, r9.xyzx, cb0[23].wwww
    r9.xyz = ((r9.xyzx)*(source[23].wwww)).xyz;
    // 96: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 97: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 98: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 99: mad r9.xyz, r9.xyzx, cb0[23].wwww, r11.xyzx
    r9.xyz = ((r9.xyzx)*(source[23].wwww)+(r11.xyzx)).xyz;
    // 100: add r0.xyw, r0.xyxw, r9.xyxz
    r0.xyw = ((r0.xyxw)+(r9.xyxz)).xyw;
    // 101: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 102: add r1.w, cb0[23].w, l(1.000000)
    r1.w = ((source[23].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 104: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 105: add r9.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r9.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 106: mad r9.xyz, r2.wwww, r9.xyzx, cb0[11].xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(source[11].xyzx)).xyz;
    // 107: mul r0.xyw, r0.xxxx, r9.xyxz
    r0.xyw = ((r0.xxxx)*(r9.xyxz)).xyw;
    // 108: mul r0.xyw, r0.xyxw, cb0[24].xxxx
    r0.xyw = ((r0.xyxw)*(source[24].xxxx)).xyw;
    // 109: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: add r9.xyz, -r2.xyzx, r1.wwww
    r9.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 111: mad r2.yzw, cb0[22].zzzz, r9.xxyz, r2.xxyz
    r2.yzw = ((source[22].zzzz)*(r9.xxyz)+(r2.xxyz)).yzw;
    // 112: dp3 r1.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: add r9.xyz, -r2.yzwy, r1.wwww
    r9.xyz = ((-(r2.yzwy))+(r1.wwww)).xyz;
    // 114: mad r2.yzw, cb0[22].wwww, r9.xxyz, r2.yyzw
    r2.yzw = ((source[22].wwww)*(r9.xxyz)+(r2.yyzw)).yzw;
    // 115: dp3 r1.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: add r9.xyz, -r2.yzwy, r1.wwww
    r9.xyz = ((-(r2.yzwy))+(r1.wwww)).xyz;
    // 117: mul r9.xyz, r9.xyzx, cb0[24].yyyy
    r9.xyz = ((r9.xyzx)*(source[24].yyyy)).xyz;
    // 118: add r1.w, r3.y, r3.x
    r1.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 119: add r1.w, r3.z, r1.w
    r1.w = ((r3.zzzz)+(r1.wwww)).w;
    // 120: add_sat r1.w, r3.w, r1.w
    r1.w = (saturate((r3.wwww)+(r1.wwww))).w;
    // 121: mad r2.yzw, r1.wwww, r9.xxyz, r2.yyzw
    r2.yzw = ((r1.wwww)*(r9.xxyz)+(r2.yyzw)).yzw;
    // 122: add r9.xyz, -r2.yzwy, r2.xxxx
    r9.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 123: mad r2.xyz, r3.wwww, r9.xyzx, r2.yzwy
    r2.xyz = ((r3.wwww)*(r9.xyzx)+(r2.yzwy)).xyz;
    // 124: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 125: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 126: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 127: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 128: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 130: add r2.w, -cb0[25].z, cb0[25].y
    r2.w = ((-(source[25].zzzz))+(source[25].yyyy)).w;
    // 131: mad r2.w, r3.w, r2.w, cb0[25].z
    r2.w = ((r3.wwww)*(r2.wwww)+(source[25].zzzz)).w;
    // 132: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 133: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 134: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 137: div r2.w, cb0[25].w, r2.w
    r2.w = ((source[25].wwww)/(r2.wwww)).w;
    // 138: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 139: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 140: div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // 141: dp3 r4.w, r4.xyzx, r5.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 142: mul_sat r5.w, r4.w, cb0[24].z
    r5.w = (saturate((r4.wwww)*(source[24].zzzz))).w;
    // 143: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mul_sat r6.w, r5.z, cb0[24].z
    r6.w = (saturate((r5.zzzz)*(source[24].zzzz))).w;
    // 146: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: add_sat r6.w, r6.w, -cb0[24].w
    r6.w = (saturate((r6.wwww)+(-(source[24].wwww)))).w;
    // 148: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 149: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 150: mul r7.w, r7.w, cb0[25].x
    r7.w = ((r7.wwww)*(source[25].xxxx)).w;
    // 151: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 152: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 153: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 154: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 155: mul r9.xyz, r0.xywx, r2.wwww
    r9.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 156: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 157: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 158: mad r1.xyz, cb0[22].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 159: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 160: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 161: mad r1.xyz, cb0[22].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 162: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 163: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 164: mad r11.xyz, r3.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 165: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 166: mad r11.xyz, r3.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 167: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 168: mad r3.xyz, r3.zzzz, r12.xyzx, r11.xyzx
    r3.xyz = ((r3.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 169: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: add r11.xyz, -r3.xyzx, r2.wwww
    r11.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 171: mad r3.xyz, cb0[22].zzzz, r11.xyzx, r3.xyzx
    r3.xyz = ((source[22].zzzz)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 172: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 173: add r11.xyz, -r3.xyzx, r2.wwww
    r11.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 174: mad r3.xyz, cb0[22].wwww, r11.xyzx, r3.xyzx
    r3.xyz = ((source[22].wwww)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 175: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 176: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 178: mul r12.xyz, r3.xyzx, r11.xyzx
    r12.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 179: mad r3.xyz, -r3.xyzx, r11.xyzx, cb0[10].xyzx
    r3.xyz = ((-(r3.xyzx))*(r11.xyzx)+(source[10].xyzx)).xyz;
    // 180: mad r3.xyz, r3.wwww, r3.xyzx, r12.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r12.xyzx)).xyz;
    // 181: mul r12.xyz, r1.xyzx, r3.xyzx
    r12.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 182: mad r1.xyz, r3.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r3.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 183: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 184: mad r2.xyz, r2.xyzx, r9.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r0.xywx))).xyz;
    // 185: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: mul r2.w, r2.w, cb0[26].x
    r2.w = ((r2.wwww)*(source[26].xxxx)).w;
    // 187: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 188: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 189: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 190: mul r3.xyz, r0.xywx, r2.yyyy
    r3.xyz = ((r0.xywx)*(r2.yyyy)).xyz;
    // 191: dp3 r2.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 192: mad r2.yzw, -r2.yyyy, r0.xxyw, r2.zzzz
    r2.yzw = ((-(r2.yyyy))*(r0.xxyw)+(r2.zzzz)).yzw;
    // 193: mad r2.yzw, cb0[22].zzzz, r2.yyzw, r3.xxyz
    r2.yzw = ((source[22].zzzz)*(r2.yyzw)+(r3.xxyz)).yzw;
    // 194: dp3 r3.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 195: add r3.xyz, -r2.yzwy, r3.xxxx
    r3.xyz = ((-(r2.yzwy))+(r3.xxxx)).xyz;
    // 196: mad r2.yzw, cb0[22].wwww, r3.xxyz, r2.yyzw
    r2.yzw = ((source[22].wwww)*(r3.xxyz)+(r2.yyzw)).yzw;
    // 197: dp3 r3.x, r1.xyzx, r1.xyzx
    r3.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 198: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 199: div r1.xyz, r1.xyzx, r3.xxxx
    r1.xyz = ((r1.xyzx)/(r3.xxxx)).xyz;
    // 200: dp3 r3.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 201: add r3.xyz, -r1.xyzx, r3.xxxx
    r3.xyz = ((-(r1.xyzx))+(r3.xxxx)).xyz;
    // 202: add r1.xyz, r1.xyzx, -r3.xyzx
    r1.xyz = ((r1.xyzx)+(-(r3.xyzx))).xyz;
    // 203: mul r3.xyz, cb0[15].xyzx, cb0[26].zzzz
    r3.xyz = ((source[15].xyzx)*(source[26].zzzz)).xyz;
    // 204: mul r3.xyz, r3.xyzx, cb0[27].yyyy
    r3.xyz = ((r3.xyzx)*(source[27].yyyy)).xyz;
    // 205: mul r3.xyz, r3.xyzx, r5.wwww
    r3.xyz = ((r3.xyzx)*(r5.wwww)).xyz;
    // 206: mad r9.xyz, r5.wwww, cb0[14].xyzx, -cb0[14].xyzx
    r9.xyz = ((r5.wwww)*(source[14].xyzx)+(-(source[14].xyzx))).xyz;
    // 207: add r3.w, r5.w, l(-1.000000)
    r3.w = ((r5.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 208: mad r3.w, cb0[13].w, r3.w, l(1.000000)
    r3.w = ((source[13].wwww)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: mad r9.xyz, cb0[14].wwww, r9.xyzx, cb0[14].xyzx
    r9.xyz = ((source[14].wwww)*(r9.xyzx)+(source[14].xyzx)).xyz;
    // 210: mad r1.xyz, r1.xyzx, r3.xyzx, r9.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(r9.xyzx)).xyz;
    // 211: mad r1.xyz, r3.wwww, cb0[13].xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(source[13].xyzx)+(r1.xyzx)).xyz;
    // 212: mad r1.xyz, r2.yzwy, r11.xyzx, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 213: add r2.y, -|r5.z|, l(1.000000)
    r2.y = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 214: mul r2.y, r4.w, r2.y
    r2.y = ((r4.wwww)*(r2.yyyy)).y;
    // 215: log r2.z, |r2.y|
    r2.z = (log2(abs(r2.yyyy))).z;
    // 216: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 217: mul r2.z, r2.z, l(1.500000)
    r2.z = ((r2.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 218: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 219: mul r3.xyz, r2.zzzz, cb0[16].xyzx
    r3.xyz = ((r2.zzzz)*(source[16].xyzx)).xyz;
    // 220: movc r2.yzw, r2.yyyy, l(0,0,0,0), r3.xxyz
    r2.yzw = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 221: add r1.xyz, r1.xyzx, r2.yzwy
    r1.xyz = ((r1.xyzx)+(r2.yzwy)).xyz;
    // 222: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 223: dp3 r2.y, r10.xyzx, r10.xyzx
    r2.y = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 224: sqrt r2.z, r2.y
    r2.z = (sqrt(r2.yyyy)).z;
    // 225: div r3.xyz, r10.xyzx, r2.zzzz
    r3.xyz = ((r10.xyzx)/(r2.zzzz)).xyz;
    // 226: dp3 r2.z, r3.xyzx, r5.xyzx
    r2.z = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 227: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 228: mul r2.w, |r2.z|, |r2.z|
    r2.w = ((abs(r2.zzzz))*(abs(r2.zzzz))).w;
    // 229: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 230: mul r2.w, r2.w, |r2.z|
    r2.w = ((r2.wwww)*(abs(r2.zzzz))).w;
    // 231: lt r2.z, |r2.z|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 232: movc r2.z, r2.z, l(0), r2.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 233: add r2.w, r2.z, l(-0.027778)
    r2.w = ((r2.zzzz)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 234: mad r2.z, r2.z, r2.w, l(0.027778)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).z;
    // 235: div_sat r2.y, r2.z, r2.y
    r2.y = (saturate((r2.zzzz)/(r2.yyyy))).y;
    // 236: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 237: mul r0.z, r0.z, r2.y
    r0.z = ((r0.zzzz)*(r2.yyyy)).z;
    // 238: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 239: mad r0.xyz, r1.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 240: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 241: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 242: mad r0.xyz, cb0[22].zzzz, r2.yzwy, r0.xyzx
    r0.xyz = ((source[22].zzzz)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 243: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 244: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 245: mad r0.xyz, cb0[22].wwww, r2.yzwy, r0.xyzx
    r0.xyz = ((source[22].wwww)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 246: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 247: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 248: mul r0.w, r0.w, cb0[26].w
    r0.w = ((r0.wwww)*(source[26].wwww)).w;
    // 249: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 250: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 251: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 252: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 253: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 254: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 255: add r1.w, -r2.x, cb0[3].x
    r1.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 256: mul r3.z, r1.w, l(0.125000)
    r3.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 257: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 258: mul r5.x, r1.w, l(0.125000)
    r5.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 259: mul r3.y, cb0[3].y, cb0[17].y
    r3.y = ((source[3].yyyy)*(source[17].yyyy)).y;
    // 260: mov r5.y, v4.y
    r5.y = (v4.yyyy).y;
    // 261: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 262: add r2.yz, r3.xxyx, r5.xxyx
    r2.yz = ((r3.xxyx)+(r5.xxyx)).yz;
    // 263: add r2.yz, r2.yyzy, r3.zzwz
    r2.yz = ((r2.yyzy)+(r3.zzwz)).yz;
    // 264: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.yzyy, t6.xyzw, s5, l(0.000000)
    r3.xyzw = (ArtistNativeSample5((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 265: mul r2.yzw, r0.wwww, r3.xxyz
    r2.yzw = ((r0.wwww)*(r3.xxyz)).yzw;
    // 266: mul r0.w, r2.x, r3.w
    r0.w = ((r2.xxxx)*(r3.wwww)).w;
    // 267: mad r2.xyz, r2.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 268: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 269: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 270: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 271: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 272: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 273: mad r2.xy, cb0[18].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[18].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 274: mul r0.w, cb0[18].y, cb0[26].w
    r0.w = ((source[18].yyyy)*(source[26].wwww)).w;
    // 275: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 276: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 277: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 278: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 279: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 280: mul r1.w, cb0[18].x, l(0.001000)
    r1.w = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 281: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 282: mad r2.xy, r1.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 283: dp2 r1.w, cb0[19].xyxx, r2.xyxx
    r1.w = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 284: dp2 r2.y, cb0[20].xyxx, r2.xyxx
    r2.y = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 285: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 286: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r2.xyzw = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 288: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 289: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 290: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 291: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 292: mad r3.xyz, cb0[18].zzzz, r2.xyzx, -r0.xyzx
    r3.xyz = ((source[18].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 293: mul r2.xyz, r2.xyzx, cb0[18].zzzz
    r2.xyz = ((r2.xyzx)*(source[18].zzzz)).xyz;
    // 294: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 295: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 296: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 297: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 298: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 299: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 300: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 301: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 302: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 303: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 304: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 305: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 306: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 307: mul r3.yzw, r3.yyyy, cb0[30].xxyz
    r3.yzw = ((r3.yyyy)*(source[30].xxyz)).yzw;
    // 308: mad r3.xyz, r3.xxxx, cb0[29].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[29].xyzx)+(r3.yzwy)).xyz;
    // 309: mul r3.xyz, r3.xyzx, cb0[31].wwww
    r3.xyz = ((r3.xyzx)*(source[31].wwww)).xyz;
    // 310: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 311: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 313: mad o0.xyz, r0.xyzx, cb0[31].xyzx, r1.xyzx
    output.xyz = ((r0.xyzx)*(source[31].xyzx)+(r1.xyzx)).xyz;
    // 315: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
// mn_admg_00.mat.sk_admg_00_mi_dead: directional light PS f6c4280d30bd9b49b78aa82a10f24741; parameter rows reuse program 3831 packing.
float4 ArtistNative3831Light(ARTIST_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0]=0.f;
    source[1]=float4(input.sourceCameraPosition,input.color.a);
    source[28]=float4(lightColor,1.f);
    source[29].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[2] = g_ArtistSourceMaterialParameters[24u];
    source[3] = g_ArtistSourceMaterialParameters[12u];
    source[4] = g_ArtistSourceMaterialParameters[13u];
    source[5] = g_ArtistSourceMaterialParameters[14u];
    source[6] = g_ArtistSourceMaterialParameters[15u];
    source[7] = g_ArtistSourceMaterialParameters[17u];
    source[8] = g_ArtistSourceMaterialParameters[16u];
    source[9] = g_ArtistSourceMaterialParameters[27u];
    source[10] = g_ArtistSourceMaterialParameters[21u];
    source[11] = g_ArtistSourceMaterialParameters[22u];
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[13] = g_ArtistSourceMaterialParameters[25u];
    source[14] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[15] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[16] = float4(1.17105305, 0.66533798, 0.546052992, 1.0);
    source[17].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[17].w = ((g_ArtistSourceMaterialParameters[6u].xxxx*g_ArtistSourceMaterialParameters[8u].zzzz)).x;
    source[18].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[18].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[19].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[20].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[20].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[20].z = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[20].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[21].x = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[21].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[21].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[21].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[22].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[22].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[22].z = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[22].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[23].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))))).x;
    source[23].y = (cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[23].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[23].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[24].x = (float4(0.5, 0.5, 0.5, 0.5)).x;
    source[24].y = (float4(0.5, -0.5, -0.5, -0.5)).x;
    source[24].z = (float4(1.0, 1.0, 1.0, 1.0)).x;
    source[24].w = (float4(1.0, 1.0, 1.0, 1.0)).x;
    source[25].x = (float4(0.920000017, 0.920000017, 0.920000017, 0.920000017)).x;
    source[25].y = (float4(0.200000003, 0.200000003, 0.200000003, 0.200000003)).x;
    source[25].z = (float4(15.0, 15.0, 15.0, 15.0)).x;
    source[25].w = (float4(5.0, 5.0, 5.0, 5.0)).x;
    source[26].x = (float4(0.5, 0.5, 0.5, 0.5)).x;
    source[26].y = (float4(0.150000006, 0.150000006, 0.150000006, 0.150000006)).x;
    source[26].z = (float4(0.0, 0.0, 0.0, 0.0)).x;
    source[26].w = (float4(3.3499999, 3.3499999, 3.3499999, 3.3499999)).x;
    source[27].x = (float4(6.0, 6.0, 6.0, 6.0)).x;
    source[27].y = (float4(10.0, 10.0, 10.0, 10.0)).x;
    float4 passValues[5]; [unroll] for(uint passIndex=0u;passIndex<5u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override, as program 3831.
    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override, as program 3831.
    float4 v0 = float4(input.sourceBasisX,0.f); // native tangent basis row 0
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native tangent basis row 1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(tangentLight,1.f); // native tangent light vector
    float4 v7 = float4(input.tangentView,1.f); // native tangent camera vector
    float4 v8 = float4(input.sourceWorldPosition,1.f); // native source world position
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    float4 output=0.f;
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
    // 7: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 8: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[29].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[29].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t8.xyzw, s0
    r5.xyz = (float4(1.f,1.f,1.f,1.f).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyzw = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 31: mul r9.xy, v4.xyxx, cb0[23].zzzz
    r9.xy = ((v4.xyxx)*(source[23].zzzz)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r9.xyxx, t4.yzwx, s7, l(0.000000)
    r1.w = (ArtistNativeSample6((r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 33: add r1.w, r1.w, -cb0[23].w
    r1.w = ((r1.wwww)+(-(source[23].wwww))).w;
    // 34: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 35: mul_sat r1.w, r1.w, r8.w
    r1.w = (saturate((r1.wwww)*(r8.wwww))).w;
    // 36: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 37: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 38: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 40: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 41: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 42: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 44: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 45: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 46: mul r10.xy, r9.xyxx, cb0[17].xxxx
    r10.xy = ((r9.xyxx)*(source[17].xxxx)).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 48: mad r9.xy, cb0[17].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[17].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 49: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 50: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 51: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 52: mad r12.xyz, cb0[19].xxxx, r10.xyzx, r9.xyzx
    r12.xyz = ((source[19].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 53: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 54: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 55: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 56: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 57: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 58: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 59: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 60: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 61: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 62: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 63: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 64: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 65: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 66: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 67: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 68: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 69: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 70: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 72: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: add r1.x, -cb0[18].y, cb0[18].x
    r1.x = ((-(source[18].yyyy))+(source[18].xxxx)).x;
    // 74: mad r1.x, r11.w, r1.x, cb0[18].y
    r1.x = ((r11.wwww)*(r1.xxxx)+(source[18].yyyy)).x;
    // 75: lt r1.y, |r0.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 76: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 77: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 78: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 79: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 80: movc r0.z, r1.y, l(0), r0.z
    r0.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 81: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 82: add r1.y, -r1.x, cb0[19].y
    r1.y = ((-(r1.xxxx))+(source[19].yyyy)).y;
    // 83: mad r1.x, r11.w, r1.y, r1.x
    r1.x = ((r11.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 84: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 85: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 86: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 87: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 88: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 89: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 90: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 91: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 92: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 93: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 94: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 95: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 96: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 97: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 98: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 99: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 100: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = (ArtistNativeSample4((r0.xyxx).xy, (r1.xxxx).x, true).xywz).xyw;
    // 101: rcp r0.x, cb0[19].w
    r0.x = (1.0/(source[19].wwww)).x;
    // 102: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 103: mul r12.xyz, r4.xyzx, cb0[19].wwww
    r12.xyz = ((r4.xyzx)*(source[19].wwww)).xyz;
    // 104: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 105: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 106: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 107: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 108: mad r4.xyz, r12.xyzx, cb0[19].wwww, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[19].wwww)+(r4.xyzx)).xyz;
    // 109: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 110: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 111: add r0.x, cb0[19].w, l(1.000000)
    r0.x = ((source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 113: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 114: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 115: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 116: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 117: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 118: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 119: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 120: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 121: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 122: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 123: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 124: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 125: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 126: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 127: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 128: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 129: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 131: mul r12.xyz, r1.xywx, r4.xxxx
    r12.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 132: mul r13.xyz, r12.xyzx, cb0[24].yyyy
    r13.xyz = ((r12.xyzx)*(source[24].yyyy)).xyz;
    // 133: mul r4.z, r11.w, l(0.500000)
    r4.z = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 134: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 136: mul r4.w, r0.z, r4.w
    r4.w = ((r0.zzzz)*(r4.wwww)).w;
    // 137: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 138: mad r9.xyz, r4.zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((r4.zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 139: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 140: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 141: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 142: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 143: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 144: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: dp3 r6.w, cb0[16].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[16].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 147: add r7.xyz, r6.wwww, -cb0[16].xyzx
    r7.xyz = ((r6.wwww)+(-(source[16].xyzx))).xyz;
    // 148: mad r7.xyz, r5.wwww, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[16].xyzx)).xyz;
    // 149: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 150: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 151: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 152: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 153: mad r7.xyz, r7.xyzx, r4.zzzz, r5.wwww
    r7.xyz = ((r7.xyzx)*(r4.zzzz)+(r5.wwww)).xyz;
    // 154: add_sat r4.z, r11.w, cb0[24].z
    r4.z = (saturate((r11.wwww)+(source[24].zzzz))).z;
    // 155: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 157: mul_sat r6.xy, r6.xzxx, cb0[20].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[20].zzzz))).xy;
    // 158: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 159: add_sat r6.y, r6.y, -cb0[20].w
    r6.y = (saturate((r6.yyyy)+(-(source[20].wwww)))).y;
    // 160: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 161: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 162: mul r6.y, r6.y, cb0[21].x
    r6.y = ((r6.yyyy)*(source[21].xxxx)).y;
    // 163: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 164: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 165: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 166: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 168: mad r7.w, r2.w, l(2.000000), -r4.x
    r7.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).w;
    // 169: mad r6.yzw, r6.yyzw, r7.wwww, r4.xxxx
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r4.xxxx)).yzw;
    // 170: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 171: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 172: mul r7.w, r0.z, r0.z
    r7.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 173: mul r8.w, r7.w, cb0[24].w
    r8.w = ((r7.wwww)*(source[24].wwww)).w;
    // 174: mad r0.z, -r7.w, cb0[24].w, r0.z
    r0.z = ((-(r7.wwww))*(source[24].wwww)+(r0.zzzz)).z;
    // 175: mad r0.z, r11.w, r0.z, r8.w
    r0.z = ((r11.wwww)*(r0.zzzz)+(r8.wwww)).z;
    // 176: mad r6.yzw, r4.zzzz, r6.yyzw, -r7.xxyz
    r6.yzw = ((r4.zzzz)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 177: mad r6.yzw, r0.zzzz, r6.yyzw, r7.xxyz
    r6.yzw = ((r0.zzzz)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 178: sqrt r0.z, r5.w
    r0.z = (sqrt(r5.wwww)).z;
    // 179: mul r5.xyz, r5.xyzx, r0.zzzz
    r5.xyz = ((r5.xyzx)*(r0.zzzz)).xyz;
    // 180: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 181: mad r6.yzw, -cb0[24].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[24].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 182: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 183: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 184: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 185: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 186: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 187: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 188: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 189: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 190: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 192: mad r7.xyz, cb0[18].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 193: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 194: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 195: mad r7.xyz, cb0[18].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 196: mad r10.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 197: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 199: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 200: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[9].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[9].xyzx)).xyz;
    // 201: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 202: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 203: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 204: mad r8.xyz, cb0[18].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 205: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 206: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 207: mad r8.xyz, cb0[18].wwww, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].wwww)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 208: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 209: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 210: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 211: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 212: add r1.yzw, -cb0[10].xxyz, cb0[11].xxyz
    r1.yzw = ((-(source[10].xxyz))+(source[11].xxyz)).yzw;
    // 213: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[10].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[10].xyzx)).xyz;
    // 214: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 215: mul r1.xyz, r1.xyzx, cb0[20].xxxx
    r1.xyz = ((r1.xyzx)*(source[20].xxxx)).xyz;
    // 216: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 217: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 218: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 219: mad r14.xyz, cb0[18].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[18].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 220: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 221: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 222: mad r14.xyz, cb0[18].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[18].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 223: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 224: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 225: mul r15.xyz, r15.xyzx, cb0[20].yyyy
    r15.xyz = ((r15.xyzx)*(source[20].yyyy)).xyz;
    // 226: add r0.z, r11.y, r11.x
    r0.z = ((r11.yyyy)+(r11.xxxx)).z;
    // 227: add r0.z, r11.z, r0.z
    r0.z = ((r11.zzzz)+(r0.zzzz)).z;
    // 228: add_sat r0.z, r11.w, r0.z
    r0.z = (saturate((r11.wwww)+(r0.zzzz))).z;
    // 229: mad r11.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r11.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 230: add r2.xyz, r2.xxxx, -r11.xyzx
    r2.xyz = ((r2.xxxx)+(-(r11.xyzx))).xyz;
    // 231: mad r2.xyz, r11.wwww, r2.xyzx, r11.xyzx
    r2.xyz = ((r11.wwww)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 232: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 233: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 234: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 235: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 236: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 237: add r1.w, -cb0[21].z, cb0[21].y
    r1.w = ((-(source[21].zzzz))+(source[21].yyyy)).w;
    // 238: mad r1.w, r11.w, r1.w, cb0[21].z
    r1.w = ((r11.wwww)*(r1.wwww)+(source[21].zzzz)).w;
    // 239: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 240: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 241: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 242: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 243: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 244: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 245: div r1.w, cb0[21].w, r1.w
    r1.w = ((source[21].wwww)/(r1.wwww)).w;
    // 246: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 247: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 248: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 249: mul r1.w, r1.w, cb0[22].x
    r1.w = ((r1.wwww)*(source[22].xxxx)).w;
    // 250: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 251: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 252: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 253: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 254: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 255: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 256: mad r1.xyz, cb0[18].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 257: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 259: mad r1.xyz, cb0[18].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 260: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 261: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 262: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 263: mul r4.z, r4.z, cb0[22].w
    r4.z = ((r4.zzzz)*(source[22].wwww)).z;
    // 264: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 265: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 266: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 267: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 268: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 269: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 270: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 271: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 272: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 273: mul r10.y, cb0[2].y, cb0[12].y
    r10.y = ((source[2].yyyy)*(source[12].yyyy)).y;
    // 274: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 275: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 276: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 277: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 278: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t6.xyzw, s6, l(0.000000)
    r10.xyzw = (ArtistNativeSample5((r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 280: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 281: mul r1.w, r4.z, r10.w
    r1.w = ((r4.zzzz)*(r10.wwww)).w;
    // 282: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 283: mad r1.xyz, r1.wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 284: mul r1.w, cb0[13].y, cb0[22].w
    r1.w = ((source[13].yyyy)*(source[22].wwww)).w;
    // 285: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 286: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 287: mul r10.y, r1.w, l(0.020000)
    r10.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 288: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 289: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 290: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 291: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 292: mul r3.z, cb0[13].x, l(0.001000)
    r3.z = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 293: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 294: mad r3.xy, r3.zzzz, r3.xyxx, r10.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r10.xyxx)).xy;
    // 295: dp2 r3.z, cb0[14].xyxx, r3.xyxx
    r3.z = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 296: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 297: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 298: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 299: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = (ArtistNativeSample5((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 300: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 301: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 302: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 303: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 304: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 305: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 306: mul r10.xyz, r3.xyzx, cb0[13].zzzz
    r10.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 307: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 308: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 309: mad r3.xyz, cb0[13].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 310: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 311: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 312: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 313: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 314: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 315: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 316: mul r0.z, r2.w, cb0[25].x
    r0.z = ((r2.wwww)*(source[25].xxxx)).z;
    // 317: mul r0.w, r11.w, r0.z
    r0.w = ((r11.wwww)*(r0.zzzz)).w;
    // 318: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 319: min r0.w, r0.w, cb0[25].x
    r0.w = (min(r0.wwww,source[25].xxxx)).w;
    // 320: add r1.w, -cb0[25].w, cb0[25].z
    r1.w = ((-(source[25].wwww))+(source[25].zzzz)).w;
    // 321: mad r1.w, cb0[25].y, r1.w, cb0[25].w
    r1.w = ((source[25].yyyy)*(r1.wwww)+(source[25].wwww)).w;
    // 322: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 323: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 324: mad r1.w, r11.w, r1.w, l(1.000000)
    r1.w = ((r11.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 325: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 326: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 327: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 328: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 329: movc r0.y, r2.w, l(0), r0.w
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 330: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r5.xyz = (ArtistNativeSample7((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 331: add r0.y, -cb0[26].z, cb0[26].y
    r0.y = ((-(source[26].zzzz))+(source[26].yyyy)).y;
    // 332: mad r0.y, cb0[26].x, r0.y, cb0[26].z
    r0.y = ((source[26].xxxx)*(r0.yyyy)+(source[26].zzzz)).y;
    // 333: mul r0.y, r0.y, r11.w
    r0.y = ((r0.yyyy)*(r11.wwww)).y;
    // 334: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t7.xwyz, s8, l(0.000000)
    r0.xzw = (ArtistNativeSample7((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 335: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 336: add r0.w, -cb0[26].w, l(2.000000)
    r0.w = ((-(source[26].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 337: mad r0.w, r4.x, r0.w, cb0[26].w
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[26].wwww)).w;
    // 338: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 339: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 340: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 341: mul r0.xyz, r0.xyzx, cb0[27].xxxx
    r0.xyz = ((r0.xyzx)*(source[27].xxxx)).xyz;
    // 342: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 343: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 344: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 345: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 346: mul r0.xyz, r0.xyzx, cb0[27].yyyy
    r0.xyz = ((r0.xyzx)*(source[27].yyyy)).xyz;
    // 347: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 348: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 349: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 350: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 351: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 352: mul o0.xyz, r0.xyzx, cb0[28].xyzx
    output.xyz = ((r0.xyzx)*(source[28].xyzx)).xyz;
    // 353: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 355: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 356: ret
    return output;
}
// sk_admg_00_hair_mi_dead: 10215f76a54bc242a767938c5056e6fd; selected map 086561700738e25278d48bfe1443e5675270d8c64905097b6b41e26ac9ec1e5d.
float4 ArtistNative3832(ARTIST_NATIVE_INPUT input)
{
    float4 source[27]; [unroll] for (uint i=0u; i<27u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing scene adapter: source world origin is absolute; camera is converted to source cm.
    source[0]=0.f;
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[1].w=input.color.a;
    source[24]=float4(input.skyUpperColor,0.f);
    source[25]=float4(input.skyLowerColor,0.f);
    source[26]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_ArtistSourceMaterialParameters[16u];
    source[3] = g_ArtistSourceMaterialParameters[17u];
    source[4] = g_ArtistSourceMaterialParameters[9u];
    source[5] = g_ArtistSourceMaterialParameters[11u];
    source[6] = g_ArtistSourceMaterialParameters[10u];
    source[7] = g_ArtistSourceMaterialParameters[14u];
    source[8] = g_ArtistSourceMaterialParameters[15u];
    source[9] = g_ArtistSourceMaterialParameters[19u];
    source[10] = g_ArtistSourceMaterialParameters[7u];
    source[11] = g_ArtistSourceMaterialParameters[8u];
    source[12] = g_ArtistSourceMaterialParameters[13u];
    source[13] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[14] = g_ArtistSourceMaterialParameters[18u];
    source[15] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[16] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[17].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[17].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[17].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[18].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[18].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[19].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[19].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[19].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[20].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[20].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[20].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[21].x = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[21].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[21].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[21].w = ((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[22].x = (((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[22].y = (sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[22].z = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[22].w = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[23].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[23].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[23].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[23].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].yyyy)).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[23].xxxx
    r0.xy = ((v4.xyxx)*(source[23].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s6, l(0.000000)
    r0.x = (ArtistNativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[23].y
    r0.x = ((r0.xxxx)+(-(source[23].yyyy))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 8: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 9: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 10: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 11: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 13: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 14: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 15: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 16: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 17: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 18: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 19: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 20: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 21: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 22: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 25: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 27: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 29: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 30: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 31: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 32: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 34: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 35: mul r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)*(source[18].yyyy)).w;
    // 36: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 37: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 38: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 39: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 41: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 42: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 43: mul r3.xy, r0.ywyy, cb0[17].xxxx
    r3.xy = ((r0.ywyy)*(source[17].xxxx)).xy;
    // 44: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 45: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 46: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 47: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 49: mad r4.xyz, cb0[17].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[17].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 50: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 51: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 52: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 53: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 54: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 55: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 56: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 57: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 60: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 61: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 62: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 63: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 64: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 65: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 68: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 69: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 70: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 71: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 72: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 73: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 74: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 75: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 76: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 77: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 78: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 79: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 80: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 82: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t4.xywz, s3, r0.x
    r0.xyw = (ArtistNativeSample3((r0.ywyy).xy, (r0.xxxx).x, true).xywz).xyw;
    // 84: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 85: rcp r1.w, cb0[18].z
    r1.w = (1.0/(source[18].zzzz)).w;
    // 86: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 87: mul r6.xyz, r6.xyzx, cb0[18].zzzz
    r6.xyz = ((r6.xyzx)*(source[18].zzzz)).xyz;
    // 88: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 89: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 90: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 91: mad r6.xyz, r6.xyzx, cb0[18].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[18].zzzz)+(r10.xyzx)).xyz;
    // 92: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 93: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 94: add r1.w, cb0[18].z, l(1.000000)
    r1.w = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 96: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 97: add r6.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r6.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 98: mad r6.xyz, r2.wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 99: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 100: mul r0.xyw, r0.xyxw, cb0[18].wwww
    r0.xyw = ((r0.xyxw)*(source[18].wwww)).xyw;
    // 101: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: add r6.xyz, -r2.xyzx, r1.wwww
    r6.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 103: mad r2.xyz, cb0[17].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[17].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 104: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r6.xyz, -r2.xyzx, r1.wwww
    r6.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 106: mad r2.xyz, cb0[17].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[17].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 107: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 108: add r6.xyz, -r2.xyzx, r1.wwww
    r6.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 109: mul r6.xyz, r6.xyzx, cb0[19].xxxx
    r6.xyz = ((r6.xyzx)*(source[19].xxxx)).xyz;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r10.xyzw = (ArtistNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 111: add r1.w, r10.y, r10.x
    r1.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 112: add r1.w, r10.z, r1.w
    r1.w = ((r10.zzzz)+(r1.wwww)).w;
    // 113: add_sat r1.w, r10.w, r1.w
    r1.w = (saturate((r10.wwww)+(r1.wwww))).w;
    // 114: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 115: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 116: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 117: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 118: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 119: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 120: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 121: mul r1.w, r1.w, cb0[20].x
    r1.w = ((r1.wwww)*(source[20].xxxx)).w;
    // 122: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 123: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 126: div r2.w, cb0[20].y, r2.w
    r2.w = ((source[20].yyyy)/(r2.wwww)).w;
    // 127: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 128: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 129: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 130: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 131: mul_sat r4.w, r3.w, cb0[19].y
    r4.w = (saturate((r3.wwww)*(source[19].yyyy))).w;
    // 132: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: mul_sat r5.w, r4.z, cb0[19].y
    r5.w = (saturate((r4.zzzz)*(source[19].yyyy))).w;
    // 135: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: add_sat r5.w, r5.w, -cb0[19].z
    r5.w = (saturate((r5.wwww)+(-(source[19].zzzz)))).w;
    // 137: log r6.x, r5.w
    r6.x = (log2(r5.wwww)).x;
    // 138: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: mul r6.x, r6.x, cb0[19].w
    r6.x = ((r6.xxxx)*(source[19].wwww)).x;
    // 140: exp r6.x, r6.x
    r6.x = (exp2(r6.xxxx)).x;
    // 141: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 142: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 143: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 144: mul r6.xyz, r0.xywx, r2.wwww
    r6.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 145: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 146: add r10.xyz, -r1.xyzx, r2.wwww
    r10.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 147: mad r1.xyz, cb0[17].yyyy, r10.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 148: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: add r10.xyz, -r1.xyzx, r2.wwww
    r10.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 150: mad r1.xyz, cb0[17].zzzz, r10.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 151: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 152: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 154: mad r10.xyz, cb0[17].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 155: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 157: mad r10.xyz, cb0[17].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 158: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 161: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 162: mul r12.xyz, r1.xyzx, r10.xyzx
    r12.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 163: mad r1.xyz, r10.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 164: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 165: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 166: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 168: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 169: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 170: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: mul r6.xyz, r0.xywx, r2.yyyy
    r6.xyz = ((r0.xywx)*(r2.yyyy)).xyz;
    // 172: dp3 r2.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 173: mad r2.yzw, -r2.yyyy, r0.xxyw, r2.zzzz
    r2.yzw = ((-(r2.yyyy))*(r0.xxyw)+(r2.zzzz)).yzw;
    // 174: mad r2.yzw, cb0[17].yyyy, r2.yyzw, r6.xxyz
    r2.yzw = ((source[17].yyyy)*(r2.yyzw)+(r6.xxyz)).yzw;
    // 175: dp3 r5.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: add r6.xyz, -r2.yzwy, r5.wwww
    r6.xyz = ((-(r2.yzwy))+(r5.wwww)).xyz;
    // 177: mad r2.yzw, cb0[17].zzzz, r6.xxyz, r2.yyzw
    r2.yzw = ((source[17].zzzz)*(r6.xxyz)+(r2.yyzw)).yzw;
    // 178: dp3 r5.w, r1.xyzx, r1.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 179: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 180: div r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)/(r5.wwww)).xyz;
    // 181: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 182: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 183: add r1.xyz, r1.xyzx, -r6.xyzx
    r1.xyz = ((r1.xyzx)+(-(r6.xyzx))).xyz;
    // 184: mul r6.xyz, cb0[11].xyzx, cb0[21].xxxx
    r6.xyz = ((source[11].xyzx)*(source[21].xxxx)).xyz;
    // 185: mul r6.xyz, r6.xyzx, cb0[22].wwww
    r6.xyz = ((r6.xyzx)*(source[22].wwww)).xyz;
    // 186: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 187: mad r10.xyz, r4.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r10.xyz = ((r4.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 188: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 189: mad r4.w, cb0[9].w, r4.w, l(1.000000)
    r4.w = ((source[9].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: mad r10.xyz, cb0[10].wwww, r10.xyzx, cb0[10].xyzx
    r10.xyz = ((source[10].wwww)*(r10.xyzx)+(source[10].xyzx)).xyz;
    // 191: mad r1.xyz, r1.xyzx, r6.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(r10.xyzx)).xyz;
    // 192: mad r1.xyz, r4.wwww, cb0[9].xyzx, r1.xyzx
    r1.xyz = ((r4.wwww)*(source[9].xyzx)+(r1.xyzx)).xyz;
    // 193: mad r1.xyz, r2.yzwy, r11.xyzx, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 194: add r2.y, -|r4.z|, l(1.000000)
    r2.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 195: mul r2.y, r3.w, r2.y
    r2.y = ((r3.wwww)*(r2.yyyy)).y;
    // 196: log r2.z, |r2.y|
    r2.z = (log2(abs(r2.yyyy))).z;
    // 197: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 198: mul r2.z, r2.z, l(1.500000)
    r2.z = ((r2.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 199: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 200: mul r6.xyz, r2.zzzz, cb0[12].xyzx
    r6.xyz = ((r2.zzzz)*(source[12].xyzx)).xyz;
    // 201: movc r2.yzw, r2.yyyy, l(0,0,0,0), r6.xxyz
    r2.yzw = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxyz)).yzw;
    // 202: add r1.xyz, r1.xyzx, r2.yzwy
    r1.xyz = ((r1.xyzx)+(r2.yzwy)).xyz;
    // 203: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 204: dp3 r2.y, r9.xyzx, r9.xyzx
    r2.y = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 205: sqrt r2.z, r2.y
    r2.z = (sqrt(r2.yyyy)).z;
    // 206: div r6.xyz, r9.xyzx, r2.zzzz
    r6.xyz = ((r9.xyzx)/(r2.zzzz)).xyz;
    // 207: dp3 r2.z, r6.xyzx, r4.xyzx
    r2.z = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 208: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 209: mul r2.w, |r2.z|, |r2.z|
    r2.w = ((abs(r2.zzzz))*(abs(r2.zzzz))).w;
    // 210: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 211: mul r2.w, r2.w, |r2.z|
    r2.w = ((r2.wwww)*(abs(r2.zzzz))).w;
    // 212: lt r2.z, |r2.z|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 213: movc r2.z, r2.z, l(0), r2.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 214: add r2.w, r2.z, l(-0.027778)
    r2.w = ((r2.zzzz)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 215: mad r2.z, r2.z, r2.w, l(0.027778)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).z;
    // 216: div_sat r2.y, r2.z, r2.y
    r2.y = (saturate((r2.zzzz)/(r2.yyyy))).y;
    // 217: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 218: mul r0.z, r0.z, r2.y
    r0.z = ((r0.zzzz)*(r2.yyyy)).z;
    // 219: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 220: mad r0.xyz, r1.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 221: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 222: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 223: mad r0.xyz, cb0[17].yyyy, r2.yzwy, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 224: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 225: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 226: mad r0.xyz, cb0[17].zzzz, r2.yzwy, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 227: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 228: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 229: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 230: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 231: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 232: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 233: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 234: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 235: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 236: add r1.w, -r2.x, cb0[3].x
    r1.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 237: mul r4.z, r1.w, l(0.125000)
    r4.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 239: mul r6.x, r1.w, l(0.125000)
    r6.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 240: mul r4.y, cb0[3].y, cb0[13].y
    r4.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 241: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 242: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 243: add r2.yz, r4.xxyx, r6.xxyx
    r2.yz = ((r4.xxyx)+(r6.xxyx)).yz;
    // 244: add r2.yz, r2.yyzy, r4.zzwz
    r2.yz = ((r2.yyzy)+(r4.zzwz)).yz;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.yzyy, t6.xyzw, s5, l(0.000000)
    r4.xyzw = (ArtistNativeSample5((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 246: mul r2.yzw, r0.wwww, r4.xxyz
    r2.yzw = ((r0.wwww)*(r4.xxyz)).yzw;
    // 247: mul r0.w, r2.x, r4.w
    r0.w = ((r2.xxxx)*(r4.wwww)).w;
    // 248: mad r2.xyz, r2.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 249: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 250: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 251: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 252: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 253: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 254: mad r2.xy, cb0[14].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[14].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 255: mul r0.w, cb0[14].y, cb0[21].y
    r0.w = ((source[14].yyyy)*(source[21].yyyy)).w;
    // 256: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 257: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 258: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 259: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 260: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 261: mul r1.w, cb0[14].x, l(0.001000)
    r1.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 262: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 263: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 264: dp2 r1.w, cb0[15].xyxx, r2.xyxx
    r1.w = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 265: dp2 r2.y, cb0[16].xyxx, r2.xyxx
    r2.y = (dot((source[16].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 266: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 267: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 268: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r2.xyzw = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 269: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 270: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 271: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 272: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 273: mad r4.xyz, cb0[14].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[14].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 274: mul r2.xyz, r2.xyzx, cb0[14].zzzz
    r2.xyz = ((r2.xyzx)*(source[14].zzzz)).xyz;
    // 275: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 276: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 277: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 278: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 279: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 280: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 281: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 282: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 283: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 284: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 285: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 286: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 287: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 288: mul r3.yzw, r3.yyyy, cb0[25].xxyz
    r3.yzw = ((r3.yyyy)*(source[25].xxyz)).yzw;
    // 289: mad r3.xyz, r3.xxxx, cb0[24].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[24].xyzx)+(r3.yzwy)).xyz;
    // 290: mul r3.xyz, r3.xyzx, cb0[26].wwww
    r3.xyz = ((r3.xyzx)*(source[26].wwww)).xyz;
    // 291: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 292: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 294: mad o0.xyz, r0.xyzx, cb0[26].xyzx, r1.xyzx
    output.xyz = ((r0.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 296: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
// mn_admg_00.mat.sk_admg_00_hair_mi_dead: directional light PS 6b368ae945f9814ba34651c9880020b0; parameter rows reuse program 3832 packing.
float4 ArtistNative3832Light(ARTIST_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0]=0.f;
    source[1]=float4(input.sourceCameraPosition,input.color.a);
    source[20]=float4(lightColor,1.f);
    source[21].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[2] = g_ArtistSourceMaterialParameters[17u];
    source[3] = g_ArtistSourceMaterialParameters[9u];
    source[4] = g_ArtistSourceMaterialParameters[11u];
    source[5] = g_ArtistSourceMaterialParameters[10u];
    source[6] = g_ArtistSourceMaterialParameters[14u];
    source[7] = g_ArtistSourceMaterialParameters[15u];
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[9] = g_ArtistSourceMaterialParameters[18u];
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[16].x = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[16].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[16].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[16].w = ((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[17].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[17].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].yyyy)).x;
    source[18].x = (float4(1.0, 1.0, 1.0, 1.0)).x;
    source[18].y = (float4(1.0, 1.0, 1.0, 1.0)).x;
    source[18].z = (float4(0.920000017, 0.920000017, 0.920000017, 0.920000017)).x;
    source[18].w = (float4(3.3499999, 3.3499999, 3.3499999, 3.3499999)).x;
    source[19].x = (float4(6.0, 6.0, 6.0, 6.0)).x;
    source[19].y = (float4(10.0, 10.0, 10.0, 10.0)).x;
    float4 passValues[5]; [unroll] for(uint passIndex=0u;passIndex<5u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override, as program 3832.
    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override, as program 3832.
    float4 v0 = float4(input.sourceBasisX,0.f); // native tangent basis row 0
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native tangent basis row 1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(tangentLight,1.f); // native tangent light vector
    float4 v7 = float4(input.tangentView,1.f); // native tangent camera vector
    float4 v8 = float4(input.sourceWorldPosition,1.f); // native source world position
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    float4 output=0.f;
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
    // 7: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 8: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t8.xyzw, s0
    r5.xyz = (float4(1.f,1.f,1.f,1.f).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r8.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 31: mul r9.xy, v4.xyxx, cb0[17].xxxx
    r9.xy = ((v4.xyxx)*(source[17].xxxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r9.xyxx, t2.yzwx, s7, l(0.000000)
    r1.w = (ArtistNativeSample6((r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 33: add r1.w, r1.w, -cb0[17].y
    r1.w = ((r1.wwww)+(-(source[17].yyyy))).w;
    // 34: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 35: mul_sat r1.w, r1.w, r8.w
    r1.w = (saturate((r1.wwww)*(r8.wwww))).w;
    // 36: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 37: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 38: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 40: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: mul r10.xy, r9.xyxx, cb0[12].xxxx
    r10.xy = ((r9.xyxx)*(source[12].xxxx)).xy;
    // 42: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 43: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 46: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 47: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r9.xyz, cb0[12].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[12].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 49: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 53: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 54: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 60: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 61: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 62: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 65: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 66: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 68: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 71: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 72: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 73: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 75: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 76: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 77: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 78: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 79: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 80: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 81: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 82: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 83: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 84: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 85: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 86: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 87: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 88: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 89: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 90: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 91: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s4, r1.x
    r1.xyw = (ArtistNativeSample3((r0.xyxx).xy, (r1.xxxx).x, true).xywz).xyw;
    // 93: rcp r0.x, cb0[13].z
    r0.x = (1.0/(source[13].zzzz)).x;
    // 94: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 95: mul r9.xyz, r4.xyzx, cb0[13].zzzz
    r9.xyz = ((r4.xyzx)*(source[13].zzzz)).xyz;
    // 96: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 97: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 98: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 99: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 100: mad r4.xyz, r9.xyzx, cb0[13].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[13].zzzz)+(r4.xyzx)).xyz;
    // 101: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 102: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 103: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 105: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 106: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 107: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 108: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 109: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 110: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 112: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 113: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 114: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 115: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 116: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 117: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 118: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 119: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 120: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 121: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 123: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 124: mul r11.xyz, r9.xyzx, cb0[17].wwww
    r11.xyz = ((r9.xyzx)*(source[17].wwww)).xyz;
    // 125: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 126: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 127: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 128: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 129: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 130: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 132: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 133: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 134: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 136: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 137: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 138: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 139: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 140: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 141: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 142: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 143: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 144: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 145: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 147: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 148: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 149: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 151: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 152: mul r0.z, r0.z, cb0[18].y
    r0.z = ((r0.zzzz)*(source[18].yyyy)).z;
    // 153: mad r6.y, cb0[18].x, r6.y, -r4.z
    r6.y = ((source[18].xxxx)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 154: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 155: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 156: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 157: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 158: mad r6.yzw, -cb0[17].wwww, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[17].wwww))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 159: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 160: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 161: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 162: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 163: mad r7.xyz, cb0[12].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 164: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 165: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 166: mad r7.xyz, cb0[12].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 167: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 170: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 171: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 172: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 173: mad r8.xyz, cb0[12].yyyy, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].yyyy)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 174: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 175: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 176: mad r8.xyz, cb0[12].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 177: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 178: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 181: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 182: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 183: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 184: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 185: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 186: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 187: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 188: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 189: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 191: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 192: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 194: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 195: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r14.xyzw = (ArtistNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 196: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 197: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 198: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 199: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 200: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 201: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 202: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 203: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 204: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 205: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 206: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 207: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 208: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 209: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 211: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 212: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 213: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 214: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 215: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 216: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 217: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 218: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 219: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 220: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 221: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 222: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 223: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 224: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 225: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 226: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 227: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 228: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r4.z, r4.z, cb0[16].y
    r4.z = ((r4.zzzz)*(source[16].yyyy)).z;
    // 230: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 231: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 232: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 234: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 235: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 236: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 237: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 239: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 240: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 241: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 242: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 243: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 244: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = (ArtistNativeSample5((r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 246: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 247: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 248: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 249: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r1.w, cb0[9].y, cb0[16].y
    r1.w = ((source[9].yyyy)*(source[16].yyyy)).w;
    // 251: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 252: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 253: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 254: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 255: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 256: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 257: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 258: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 259: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 260: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 261: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 262: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 263: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 264: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 265: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = (ArtistNativeSample5((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 266: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 267: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 268: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 269: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 271: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 272: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 273: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 275: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 276: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 277: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 278: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 279: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 280: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 281: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 282: mul r0.y, r2.w, cb0[18].z
    r0.y = ((r2.wwww)*(source[18].zzzz)).y;
    // 283: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = (ArtistNativeSample7((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 284: add r0.w, -cb0[18].w, l(2.000000)
    r0.w = ((-(source[18].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 285: mad r0.w, r4.x, r0.w, cb0[18].w
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].wwww)).w;
    // 286: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 287: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 288: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 289: mul r0.xyz, r0.xyzx, cb0[19].xxxx
    r0.xyz = ((r0.xyzx)*(source[19].xxxx)).xyz;
    // 290: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 291: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 292: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 293: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 294: mul r0.xyz, r0.xyzx, cb0[19].yyyy
    r0.xyz = ((r0.xyzx)*(source[19].yyyy)).xyz;
    // 295: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 296: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 297: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 298: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 299: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 300: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 301: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 303: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 304: ret
    return output;
}
// sk_admg_00_mi_dead: 0536b29c3525994fa5ccf365c7edc279; selected map aab7dcd9405cbc7826e8a997938d268b07141e1ea74cbb81ed41aef6b9e0838c.
float4 ArtistNative3833(ARTIST_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing scene adapter: source world origin is absolute; camera is converted to source cm.
    source[0]=0.f;
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[1].w=input.color.a;
    source[27]=float4(input.skyUpperColor,0.f);
    source[28]=float4(input.skyLowerColor,0.f);
    source[29]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_ArtistSourceMaterialParameters[19u];
    source[3] = g_ArtistSourceMaterialParameters[20u];
    source[4] = g_ArtistSourceMaterialParameters[9u];
    source[5] = g_ArtistSourceMaterialParameters[10u];
    source[6] = g_ArtistSourceMaterialParameters[11u];
    source[7] = g_ArtistSourceMaterialParameters[12u];
    source[8] = g_ArtistSourceMaterialParameters[14u];
    source[9] = g_ArtistSourceMaterialParameters[13u];
    source[10] = g_ArtistSourceMaterialParameters[17u];
    source[11] = g_ArtistSourceMaterialParameters[18u];
    source[12] = g_ArtistSourceMaterialParameters[22u];
    source[13] = g_ArtistSourceMaterialParameters[7u];
    source[14] = g_ArtistSourceMaterialParameters[8u];
    source[15] = g_ArtistSourceMaterialParameters[16u];
    source[16] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[17] = g_ArtistSourceMaterialParameters[21u];
    source[18] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[19] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[20].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[20].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[20].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[20].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[21].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[21].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[21].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[21].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[22].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[22].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[22].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[22].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[23].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[23].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[23].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[23].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[24].x = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[24].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[24].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[24].w = ((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[25].x = (((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[25].y = (sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[25].z = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[25].w = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[26].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[26].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[26].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[26].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].yyyy)).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[26].xxxx
    r0.xy = ((v4.xyxx)*(source[26].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s6, l(0.000000)
    r0.x = (ArtistNativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[26].y
    r0.x = ((r0.xxxx)+(-(source[26].yyyy))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 8: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 9: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 10: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 11: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 13: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 14: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 15: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 16: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 17: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 18: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 19: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 20: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 21: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 22: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 25: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 27: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 29: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 30: mul r0.w, r0.w, cb0[21].x
    r0.w = ((r0.wwww)*(source[21].xxxx)).w;
    // 31: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 32: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 34: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 35: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 36: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 37: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 38: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 39: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 41: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 42: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 43: mul r3.xy, r0.ywyy, cb0[20].xxxx
    r3.xy = ((r0.ywyy)*(source[20].xxxx)).xy;
    // 44: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 45: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 46: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 47: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 49: mad r4.xyz, cb0[20].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 50: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 51: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 52: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 53: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 54: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 55: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 56: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 57: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 60: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 61: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 62: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 63: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 64: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 65: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 68: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 69: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 70: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 71: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 72: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 73: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 74: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 75: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 76: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 77: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 78: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 79: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 80: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 82: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = (ArtistNativeSample4((r0.ywyy).xy, (r0.xxxx).x, true).xywz).xyw;
    // 84: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 85: rcp r1.w, cb0[21].z
    r1.w = (1.0/(source[21].zzzz)).w;
    // 86: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 87: mul r6.xyz, r6.xyzx, cb0[21].zzzz
    r6.xyz = ((r6.xyzx)*(source[21].zzzz)).xyz;
    // 88: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 89: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 90: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 91: mad r6.xyz, r6.xyzx, cb0[21].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[21].zzzz)+(r10.xyzx)).xyz;
    // 92: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 93: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 94: add r1.w, cb0[21].z, l(1.000000)
    r1.w = ((source[21].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 96: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 97: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 98: mad r6.xyz, r2.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 99: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 100: mul r0.xyw, r0.xyxw, cb0[21].wwww
    r0.xyw = ((r0.xyxw)*(source[21].wwww)).xyw;
    // 101: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 102: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 103: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 104: dp3 r1.w, r3.xyzx, r4.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 105: mul_sat r2.w, r1.w, cb0[22].y
    r2.w = (saturate((r1.wwww)*(source[22].yyyy))).w;
    // 106: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: mul_sat r3.w, r4.z, cb0[22].y
    r3.w = (saturate((r4.zzzz)*(source[22].yyyy))).w;
    // 109: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: add_sat r3.w, r3.w, -cb0[22].z
    r3.w = (saturate((r3.wwww)+(-(source[22].zzzz)))).w;
    // 111: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 112: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: mul r4.w, r4.w, cb0[22].w
    r4.w = ((r4.wwww)*(source[22].wwww)).w;
    // 114: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 115: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 116: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 117: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 118: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 119: mad r2.xyz, cb0[20].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 120: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 121: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 122: mad r2.xyz, cb0[20].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 123: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 124: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 125: mul r6.xyz, r6.xyzx, cb0[22].xxxx
    r6.xyz = ((r6.xyzx)*(source[22].xxxx)).xyz;
    // 126: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r10.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 127: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 128: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 129: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 130: mad r2.xyz, r3.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 131: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 132: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 133: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 134: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 135: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 136: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 137: mul r3.w, r3.w, cb0[23].x
    r3.w = ((r3.wwww)*(source[23].xxxx)).w;
    // 138: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 139: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 142: div r4.w, cb0[23].y, r4.w
    r4.w = ((source[23].yyyy)/(r4.wwww)).w;
    // 143: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 144: mul r6.xyz, r0.xywx, r4.wwww
    r6.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 145: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 146: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 147: mad r1.xyz, cb0[20].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 148: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 150: mad r1.xyz, cb0[20].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 151: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 152: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 153: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 154: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 155: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 156: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r10.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r10.xywx))).xyz;
    // 157: mad r10.xyz, r10.zzzz, r11.xyzx, r10.xywx
    r10.xyz = ((r10.zzzz)*(r11.xyzx)+(r10.xywx)).xyz;
    // 158: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 160: mad r10.xyz, cb0[20].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 161: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 163: mad r10.xyz, cb0[20].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 164: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 166: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 167: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 168: mul r12.xyz, r1.xyzx, r10.xyzx
    r12.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 169: mad r1.xyz, r10.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 170: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 171: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 172: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: mul r4.w, r4.w, cb0[23].z
    r4.w = ((r4.wwww)*(source[23].zzzz)).w;
    // 174: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 175: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 176: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 177: mul r6.xyz, r0.xywx, r2.yyyy
    r6.xyz = ((r0.xywx)*(r2.yyyy)).xyz;
    // 178: dp3 r2.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: mad r10.xyz, -r2.yyyy, r0.xywx, r2.zzzz
    r10.xyz = ((-(r2.yyyy))*(r0.xywx)+(r2.zzzz)).xyz;
    // 180: mad r6.xyz, cb0[20].yyyy, r10.xyzx, r6.xyzx
    r6.xyz = ((source[20].yyyy)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 181: dp3 r2.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 182: add r10.xyz, -r6.xyzx, r2.yyyy
    r10.xyz = ((-(r6.xyzx))+(r2.yyyy)).xyz;
    // 183: mad r6.xyz, cb0[20].zzzz, r10.xyzx, r6.xyzx
    r6.xyz = ((source[20].zzzz)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 184: dp3 r2.y, r1.xyzx, r1.xyzx
    r2.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 185: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 186: div r1.xyz, r1.xyzx, r2.yyyy
    r1.xyz = ((r1.xyzx)/(r2.yyyy)).xyz;
    // 187: dp3 r2.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 188: add r10.xyz, -r1.xyzx, r2.yyyy
    r10.xyz = ((-(r1.xyzx))+(r2.yyyy)).xyz;
    // 189: add r1.xyz, r1.xyzx, -r10.xyzx
    r1.xyz = ((r1.xyzx)+(-(r10.xyzx))).xyz;
    // 190: mul r10.xyz, cb0[14].xyzx, cb0[24].xxxx
    r10.xyz = ((source[14].xyzx)*(source[24].xxxx)).xyz;
    // 191: mul r10.xyz, r10.xyzx, cb0[25].wwww
    r10.xyz = ((r10.xyzx)*(source[25].wwww)).xyz;
    // 192: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 193: mad r13.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r13.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 194: add r2.y, r2.w, l(-1.000000)
    r2.y = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 195: mad r2.y, cb0[12].w, r2.y, l(1.000000)
    r2.y = ((source[12].wwww)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 196: mad r13.xyz, cb0[13].wwww, r13.xyzx, cb0[13].xyzx
    r13.xyz = ((source[13].wwww)*(r13.xyzx)+(source[13].xyzx)).xyz;
    // 197: mad r1.xyz, r1.xyzx, r10.xyzx, r13.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)+(r13.xyzx)).xyz;
    // 198: mad r1.xyz, r2.yyyy, cb0[12].xyzx, r1.xyzx
    r1.xyz = ((r2.yyyy)*(source[12].xyzx)+(r1.xyzx)).xyz;
    // 199: mad r1.xyz, r6.xyzx, r11.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 200: add r2.y, -|r4.z|, l(1.000000)
    r2.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 201: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 202: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 203: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 204: mul r2.y, r2.y, l(1.500000)
    r2.y = ((r2.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 205: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 206: mul r2.yzw, r2.yyyy, cb0[15].xxyz
    r2.yzw = ((r2.yyyy)*(source[15].xxyz)).yzw;
    // 207: movc r2.yzw, r1.wwww, l(0,0,0,0), r2.yyzw
    r2.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyzw)).yzw;
    // 208: add r1.xyz, r1.xyzx, r2.yzwy
    r1.xyz = ((r1.xyzx)+(r2.yzwy)).xyz;
    // 209: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 210: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 211: sqrt r2.y, r1.w
    r2.y = (sqrt(r1.wwww)).y;
    // 212: div r2.yzw, r9.xxyz, r2.yyyy
    r2.yzw = ((r9.xxyz)/(r2.yyyy)).yzw;
    // 213: dp3 r2.y, r2.yzwy, r4.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).y;
    // 214: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 215: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 216: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 217: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 218: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 219: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 220: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 221: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 222: div_sat r1.w, r2.y, r1.w
    r1.w = (saturate((r2.yyyy)/(r1.wwww))).w;
    // 223: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 224: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 225: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 226: mad r0.xyz, r3.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 227: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 228: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 229: mad r0.xyz, cb0[20].yyyy, r2.yzwy, r0.xyzx
    r0.xyz = ((source[20].yyyy)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 230: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 231: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 232: mad r0.xyz, cb0[20].zzzz, r2.yzwy, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 233: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 234: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 235: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 236: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 237: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 238: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 240: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 241: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 242: add r1.w, -r2.x, cb0[3].x
    r1.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 243: mul r4.z, r1.w, l(0.125000)
    r4.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 244: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 245: mul r6.x, r1.w, l(0.125000)
    r6.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 246: mul r4.y, cb0[3].y, cb0[16].y
    r4.y = ((source[3].yyyy)*(source[16].yyyy)).y;
    // 247: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 248: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 249: add r2.yz, r4.xxyx, r6.xxyx
    r2.yz = ((r4.xxyx)+(r6.xxyx)).yz;
    // 250: add r2.yz, r2.yyzy, r4.zzwz
    r2.yz = ((r2.yyzy)+(r4.zzwz)).yz;
    // 251: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.yzyy, t6.xyzw, s5, l(0.000000)
    r4.xyzw = (ArtistNativeSample5((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 252: mul r2.yzw, r0.wwww, r4.xxyz
    r2.yzw = ((r0.wwww)*(r4.xxyz)).yzw;
    // 253: mul r0.w, r2.x, r4.w
    r0.w = ((r2.xxxx)*(r4.wwww)).w;
    // 254: mad r2.xyz, r2.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 255: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 256: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 257: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 258: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 259: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 260: mad r2.xy, cb0[17].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 261: mul r0.w, cb0[17].y, cb0[24].y
    r0.w = ((source[17].yyyy)*(source[24].yyyy)).w;
    // 262: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 263: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 264: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 265: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 266: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 267: mul r1.w, cb0[17].x, l(0.001000)
    r1.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 268: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 269: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 270: dp2 r1.w, cb0[18].xyxx, r2.xyxx
    r1.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 271: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 272: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 273: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 274: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r2.xyzw = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 275: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 276: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 277: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 278: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 279: mad r4.xyz, cb0[17].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[17].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 280: mul r2.xyz, r2.xyzx, cb0[17].zzzz
    r2.xyz = ((r2.xyzx)*(source[17].zzzz)).xyz;
    // 281: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 282: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 283: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 284: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 285: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 286: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 287: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 288: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 289: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 290: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 291: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 292: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 293: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 294: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 295: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 296: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 297: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 298: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 300: mad o0.xyz, r0.xyzx, cb0[29].xyzx, r1.xyzx
    output.xyz = ((r0.xyzx)*(source[29].xyzx)+(r1.xyzx)).xyz;
    // 302: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
// wp_mn_admg_00.mat.sk_admg_00_mi_dead: directional light PS 44e7cf7fc42cab4e944849e49027a76d; parameter rows reuse program 3833 packing.
float4 ArtistNative3833Light(ARTIST_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
    source[0]=0.f;
    source[1]=float4(input.sourceCameraPosition,input.color.a);
    source[23]=float4(lightColor,1.f);
    source[24].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[2] = g_ArtistSourceMaterialParameters[20u];
    source[3] = g_ArtistSourceMaterialParameters[9u];
    source[4] = g_ArtistSourceMaterialParameters[10u];
    source[5] = g_ArtistSourceMaterialParameters[11u];
    source[6] = g_ArtistSourceMaterialParameters[12u];
    source[7] = g_ArtistSourceMaterialParameters[14u];
    source[8] = g_ArtistSourceMaterialParameters[13u];
    source[9] = g_ArtistSourceMaterialParameters[17u];
    source[10] = g_ArtistSourceMaterialParameters[18u];
    source[11] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[12] = g_ArtistSourceMaterialParameters[21u];
    source[13] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[14] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[15].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[16].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[16].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[16].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[17].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[18].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[18].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].x = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[19].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[19].w = ((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[20].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[20].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[20].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].yyyy)).x;
    source[21].x = (float4(1.0, 1.0, 1.0, 1.0)).x;
    source[21].y = (float4(1.0, 1.0, 1.0, 1.0)).x;
    source[21].z = (float4(0.920000017, 0.920000017, 0.920000017, 0.920000017)).x;
    source[21].w = (float4(3.3499999, 3.3499999, 3.3499999, 3.3499999)).x;
    source[22].x = (float4(6.0, 6.0, 6.0, 6.0)).x;
    source[22].y = (float4(10.0, 10.0, 10.0, 10.0)).x;
    float4 passValues[5]; [unroll] for(uint passIndex=0u;passIndex<5u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override, as program 3833.
    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override, as program 3833.
    float4 v0 = float4(input.sourceBasisX,0.f); // native tangent basis row 0
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native tangent basis row 1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(tangentLight,1.f); // native tangent light vector
    float4 v7 = float4(input.tangentView,1.f); // native tangent camera vector
    float4 v8 = float4(input.sourceWorldPosition,1.f); // native source world position
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    float4 output=0.f;
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
    // 7: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 8: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t8.xyzw, s0
    r5.xyz = (float4(1.f,1.f,1.f,1.f).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r8.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 31: mul r9.xy, v4.xyxx, cb0[20].xxxx
    r9.xy = ((v4.xyxx)*(source[20].xxxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r9.xyxx, t2.yzwx, s7, l(0.000000)
    r1.w = (ArtistNativeSample6((r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 33: add r1.w, r1.w, -cb0[20].y
    r1.w = ((r1.wwww)+(-(source[20].yyyy))).w;
    // 34: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 35: mul_sat r1.w, r1.w, r8.w
    r1.w = (saturate((r1.wwww)*(r8.wwww))).w;
    // 36: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 37: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 38: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) clip(-1.f);
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 40: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: mul r10.xy, r9.xyxx, cb0[15].xxxx
    r10.xy = ((r9.xyxx)*(source[15].xxxx)).xy;
    // 42: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 43: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 46: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 47: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r9.xyz, cb0[15].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[15].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 49: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 53: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 54: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 60: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 61: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 62: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 65: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 66: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 68: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 71: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 72: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 73: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 75: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 76: mul r1.x, r1.x, cb0[16].y
    r1.x = ((r1.xxxx)*(source[16].yyyy)).x;
    // 77: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 78: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 79: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 80: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 81: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 82: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 83: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 84: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 85: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 86: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 87: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 88: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 89: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 90: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 91: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = (ArtistNativeSample4((r0.xyxx).xy, (r1.xxxx).x, true).xywz).xyw;
    // 93: rcp r0.x, cb0[16].z
    r0.x = (1.0/(source[16].zzzz)).x;
    // 94: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 95: mul r9.xyz, r4.xyzx, cb0[16].zzzz
    r9.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 96: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 97: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 98: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 99: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 100: mad r4.xyz, r9.xyzx, cb0[16].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[16].zzzz)+(r4.xyzx)).xyz;
    // 101: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 102: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 103: add r0.x, cb0[16].z, l(1.000000)
    r0.x = ((source[16].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 105: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 106: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 107: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 108: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 109: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 110: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 112: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 113: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 114: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 115: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 116: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 117: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 118: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 119: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 120: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 121: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 123: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 124: mul r11.xyz, r9.xyzx, cb0[20].wwww
    r11.xyz = ((r9.xyzx)*(source[20].wwww)).xyz;
    // 125: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 126: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 127: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 128: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 129: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 130: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 132: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 133: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 134: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 136: mul_sat r6.xy, r6.xzxx, cb0[17].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[17].yyyy))).xy;
    // 137: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 138: add_sat r6.y, r6.y, -cb0[17].z
    r6.y = (saturate((r6.yyyy)+(-(source[17].zzzz)))).y;
    // 139: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 140: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 141: mul r6.y, r6.y, cb0[17].w
    r6.y = ((r6.yyyy)*(source[17].wwww)).y;
    // 142: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 143: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 144: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 145: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 147: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 148: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 149: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 151: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 152: mul r0.z, r0.z, cb0[21].y
    r0.z = ((r0.zzzz)*(source[21].yyyy)).z;
    // 153: mad r6.y, cb0[21].x, r6.y, -r4.z
    r6.y = ((source[21].xxxx)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 154: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 155: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 156: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 157: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 158: mad r6.yzw, -cb0[20].wwww, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[20].wwww))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 159: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 160: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 161: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r9.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 162: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 163: mad r7.xyz, r9.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 164: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 165: mad r7.xyz, r9.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 166: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 167: mad r7.xyz, r9.zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 168: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 169: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 170: mad r7.xyz, cb0[15].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 171: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 172: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 173: mad r7.xyz, cb0[15].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 174: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 175: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 176: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 177: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 178: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 180: mad r8.xyz, cb0[15].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 181: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 182: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 183: mad r8.xyz, cb0[15].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 184: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 185: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 186: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 188: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 189: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 190: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 191: mul r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = ((r1.xyzx)*(source[16].wwww)).xyz;
    // 192: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 193: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 194: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 195: mad r2.xyz, cb0[15].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 196: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 197: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 198: mad r2.xyz, cb0[15].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 199: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 200: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 201: mul r14.xyz, r14.xyzx, cb0[17].xxxx
    r14.xyz = ((r14.xyzx)*(source[17].xxxx)).xyz;
    // 202: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 203: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 204: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 205: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 206: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 207: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 208: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 209: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 210: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 211: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 212: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 213: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 214: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 215: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 216: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 217: div r1.w, cb0[18].y, r1.w
    r1.w = ((source[18].yyyy)/(r1.wwww)).w;
    // 218: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 219: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 220: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 221: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 222: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 223: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 224: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 225: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 226: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 227: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 228: mad r1.xyz, cb0[15].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 229: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 230: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 231: mad r1.xyz, cb0[15].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 232: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 233: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 234: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 235: mul r4.z, r4.z, cb0[19].y
    r4.z = ((r4.zzzz)*(source[19].yyyy)).z;
    // 236: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 237: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 238: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 239: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 240: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 241: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 242: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 243: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 244: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 245: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 246: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 247: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 248: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 249: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 250: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 251: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = (ArtistNativeSample5((r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 252: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 253: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 254: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 255: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 256: mul r1.w, cb0[12].y, cb0[19].y
    r1.w = ((source[12].yyyy)*(source[19].yyyy)).w;
    // 257: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 258: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 259: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 260: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 261: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 262: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 263: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 264: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 265: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 266: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 267: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 268: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 269: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 270: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 271: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = (ArtistNativeSample5((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 272: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 273: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 274: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 275: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 276: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 277: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 278: mul r9.xyz, r3.xyzx, cb0[12].zzzz
    r9.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 279: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 280: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 281: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 282: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 283: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 284: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 285: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 286: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 287: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 288: mul r0.y, r2.w, cb0[21].z
    r0.y = ((r2.wwww)*(source[21].zzzz)).y;
    // 289: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = (ArtistNativeSample7((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 290: add r0.w, -cb0[21].w, l(2.000000)
    r0.w = ((-(source[21].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 291: mad r0.w, r4.x, r0.w, cb0[21].w
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[21].wwww)).w;
    // 292: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 293: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 294: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 295: mul r0.xyz, r0.xyzx, cb0[22].xxxx
    r0.xyz = ((r0.xyzx)*(source[22].xxxx)).xyz;
    // 296: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 297: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 298: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 299: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 300: mul r0.xyz, r0.xyzx, cb0[22].yyyy
    r0.xyz = ((r0.xyzx)*(source[22].yyyy)).xyz;
    // 301: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 302: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 303: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 304: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 305: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 306: mul o0.xyz, r0.xyzx, cb0[23].xyzx
    output.xyz = ((r0.xyzx)*(source[23].xyzx)).xyz;
    // 307: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 309: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 310: ret
    return output;
}
// Vehicle model-cue light dispatch.
float4 Shade_VehicleModelNativeLight(uint profile, ARTIST_NATIVE_INPUT input, float3 tangentLight, float3 lightColor)
{
    switch (profile)
    {
    case 3828u: return ArtistNative3828Light(input, tangentLight, lightColor);
    case 3831u: return ArtistNative3831Light(input, tangentLight, lightColor);
    case 3832u: return ArtistNative3832Light(input, tangentLight, lightColor);
    case 3833u: return ArtistNative3833Light(input, tangentLight, lightColor);
    default: return 0.f;
    }
}
