// Original Kouku material programs 2624..2687; native IDs and expressions are unchanged.
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2638(ARTIST_NATIVE_INPUT input)
{
    float4 source[28]; [unroll] for (uint i=0u; i<28u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=0.f; // Absolute source world position needs no pre-view translation.
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[2]=float4(input.sourceActorPosition,0.f); // Original actor-position pulse seed.
    source[25]=float4(input.skyUpperColor,0.f);
    source[26]=float4(input.skyLowerColor,0.f);
    source[27]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[16u];
    source[4] = g_ArtistSourceMaterialParameters[13u];
    source[5] = g_ArtistSourceMaterialParameters[10u];
    source[6] = g_ArtistSourceMaterialParameters[12u];
    source[7] = g_ArtistSourceMaterialParameters[11u];
    source[8] = g_ArtistSourceMaterialParameters[19u];
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10] = g_ArtistSourceMaterialParameters[5u];
    source[11] = g_ArtistSourceMaterialParameters[6u];
    source[12] = g_ArtistSourceMaterialParameters[7u];
    source[13] = g_ArtistSourceMaterialParameters[8u];
    source[14] = g_ArtistSourceMaterialParameters[9u];
    source[15] = g_ArtistSourceMaterialParameters[17u];
    source[16] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[17] = g_ArtistSourceMaterialParameters[18u];
    source[18] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[19] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[20] = g_ArtistSourceMaterialParameters[14u];
    source[21].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[21].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[21].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[21].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[22].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[22].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[22].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[22].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[23].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[23].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[23].z = ((g_ArtistSourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[23].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[24].x = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[24].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[24].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[24].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s5, l(0.000000)
    r0.xyzw = (ArtistNativeSample6((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[20].xyzw
    r1.xyzw = ((r0.xyzw)*(source[20].xyzw)).xyzw;
    // 3: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 4: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 5: add r0.yz, r1.yywy, r1.xxzx
    r0.yz = ((r1.yywy)+(r1.xxzx)).yz;
    // 6: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 7: add r0.y, r0.y, l(-1.000000)
    r0.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 8: mad_sat r0.x, r0.x, r0.y, l(1.000000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 10: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 11: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 12: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 13: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 14: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 15: add r0.xyzw, -cb0[11].xyzw, cb0[12].xyzw
    r0.xyzw = ((-(source[11].xyzw))+(source[12].xyzw)).xyzw;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 17: mad r0.xyzw, r2.xxxx, r0.xyzw, cb0[11].xyzw
    r0.xyzw = ((r2.xxxx)*(r0.xyzw)+(source[11].xyzw)).xyzw;
    // 18: add r3.xyzw, -r0.xyzw, cb0[13].xyzw
    r3.xyzw = ((-(r0.xyzw))+(source[13].xyzw)).xyzw;
    // 19: mad r0.xyzw, r2.yyyy, r3.xyzw, r0.xyzw
    r0.xyzw = ((r2.yyyy)*(r3.xyzw)+(r0.xyzw)).xyzw;
    // 20: add r3.xyzw, -r0.xyzw, cb0[14].xyzw
    r3.xyzw = ((-(r0.xyzw))+(source[14].xyzw)).xyzw;
    // 21: mad r0.xyzw, r2.zzzz, r3.xyzw, r0.xyzw
    r0.xyzw = ((r2.zzzz)*(r3.xyzw)+(r0.xyzw)).xyzw;
    // 22: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 23: mul r0.xyz, r0.xyzx, cb0[24].zzzz
    r0.xyz = ((r0.xyzx)*(source[24].zzzz)).xyz;
    // 24: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 27: mad r0.xyz, cb0[22].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[22].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 28: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 30: mad r0.xyz, cb0[22].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[22].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 31: mad r1.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: mad r2.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 34: add r0.w, -cb0[15].w, l(1.000000)
    r0.w = ((-(source[15].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: mul r0.w, r0.w, cb0[21].w
    r0.w = ((r0.wwww)*(source[21].wwww)).w;
    // 36: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 37: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 38: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r1.w, cb0[15].z, l(1.500000)
    r1.w = ((source[15].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 40: mul r0.xyzw, r0.xyzw, r1.xyzw
    r0.xyzw = ((r0.xyzw)*(r1.xyzw)).xyzw;
    // 41: mad r0.w, r0.w, l(0.500000), cb0[15].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[15].zzzz)).w;
    // 42: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 43: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 44: mul r3.y, cb0[15].y, cb0[16].y
    r3.y = ((source[15].yyyy)*(source[16].yyyy)).y;
    // 45: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 46: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 47: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 48: frc r1.w, cb0[15].x
    r1.w = (frac(source[15].xxxx)).w;
    // 49: add r2.z, -r1.w, cb0[15].x
    r2.z = ((-(r1.wwww))+(source[15].xxxx)).z;
    // 50: mul r3.z, r2.z, l(0.125000)
    r3.z = ((r2.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 51: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = (ArtistNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 53: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 54: mul r0.w, r1.w, r2.w
    r0.w = ((r1.wwww)*(r2.wwww)).w;
    // 55: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 56: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 57: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 58: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 59: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 60: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 61: mad r2.xy, cb0[17].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 62: mul r0.w, cb0[17].y, cb0[21].w
    r0.w = ((source[17].yyyy)*(source[21].wwww)).w;
    // 63: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 64: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 65: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 66: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 68: mul r1.w, cb0[17].x, l(0.001000)
    r1.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 69: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 70: mad r2.xy, r1.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 71: dp2 r1.w, cb0[18].xyxx, r2.xyxx
    r1.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 72: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 73: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 74: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = (ArtistNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 76: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 77: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 78: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 79: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 80: mad r3.xyz, cb0[17].zzzz, r2.xyzx, -r0.xyzx
    r3.xyz = ((source[17].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 81: mul r2.xyz, r2.xyzx, cb0[17].zzzz
    r2.xyz = ((r2.xyzx)*(source[17].zzzz)).xyz;
    // 82: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 84: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 85: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 86: add r0.w, cb0[2].y, cb0[2].x
    r0.w = ((source[2].yyyy)+(source[2].xxxx)).w;
    // 87: add r0.w, r0.w, cb0[2].z
    r0.w = ((r0.wwww)+(source[2].zzzz)).w;
    // 88: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 89: mad r0.w, cb0[21].z, cb0[21].w, r0.w
    r0.w = ((source[21].zzzz)*(source[21].wwww)+(r0.wwww)).w;
    // 90: mul r1.w, r0.w, l(3.524534)
    r1.w = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 91: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 92: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 93: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 94: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 95: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: mad r0.w, r0.w, l(0.500000), cb0[21].y
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[21].yyyy)).w;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t5.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 98: mul r3.xyz, cb0[5].xyzx, cb0[21].xxxx
    r3.xyz = ((source[5].xyzx)*(source[21].xxxx)).xyz;
    // 99: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 100: mul r3.xyz, r0.wwww, r2.xyzx
    r3.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 101: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: mad r2.xyz, -r0.wwww, r2.xyzx, r1.wwww
    r2.xyz = ((-(r0.wwww))*(r2.xyzx)+(r1.wwww)).xyz;
    // 103: mad r2.xyz, cb0[22].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[22].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 104: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 106: mad r2.xyz, cb0[22].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 107: mul r3.xyz, cb0[10].xyzx, cb0[23].zzzz
    r3.xyz = ((source[10].xyzx)*(source[23].zzzz)).xyz;
    // 108: mul r3.xyz, r3.xyzx, cb0[24].xxxx
    r3.xyz = ((r3.xyzx)*(source[24].xxxx)).xyz;
    // 109: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 110: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 111: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 112: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 114: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 115: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 116: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 117: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 118: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 119: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 120: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 121: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 122: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 123: mul r6.xyz, r0.wwww, v5.xyzx
    r6.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 124: dp3 r0.w, r4.xyzx, r6.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 125: add r1.w, -|r6.z|, l(1.000000)
    r1.w = ((-(abs(r6.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 128: mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 129: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 130: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 131: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 132: mul r3.xyz, r3.xyzx, cb0[24].yyyy
    r3.xyz = ((r3.xyzx)*(source[24].yyyy)).xyz;
    // 133: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 134: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 135: mad_sat r1.w, r0.w, cb0[22].z, -cb0[22].w
    r1.w = (saturate((r0.wwww)*(source[22].zzzz)+(-(source[22].wwww)))).w;
    // 136: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 137: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 138: mul r2.w, r2.w, cb0[23].x
    r2.w = ((r2.wwww)*(source[23].xxxx)).w;
    // 139: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 140: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 141: mad r4.xyz, r1.wwww, cb0[9].xyzx, -cb0[9].xyzx
    r4.xyz = ((r1.wwww)*(source[9].xyzx)+(-(source[9].xyzx))).xyz;
    // 142: mad r6.xyz, r1.wwww, cb0[8].xyzx, -cb0[8].xyzx
    r6.xyz = ((r1.wwww)*(source[8].xyzx)+(-(source[8].xyzx))).xyz;
    // 143: mad r6.xyz, cb0[8].wwww, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((source[8].wwww)*(r6.xyzx)+(source[8].xyzx)).xyz;
    // 144: mad r4.xyz, cb0[9].wwww, r4.xyzx, cb0[9].xyzx
    r4.xyz = ((source[9].wwww)*(r4.xyzx)+(source[9].xyzx)).xyz;
    // 145: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 146: mad r3.xyz, cb0[23].yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((source[23].yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 147: add r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)+(r6.xyzx)).xyz;
    // 148: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 149: mad r1.xyz, r2.xyzx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 150: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 151: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 152: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 153: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 154: mul r2.xyz, r1.wwww, cb0[4].xyzx
    r2.xyz = ((r1.wwww)*(source[4].xyzx)).xyz;
    // 155: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 156: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 157: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 158: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 159: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 160: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 161: dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 162: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 163: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 164: mul r2.yzw, r2.yyyy, cb0[26].xxyz
    r2.yzw = ((r2.yyyy)*(source[26].xxyz)).yzw;
    // 165: mad r2.xyz, r2.xxxx, cb0[25].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[25].xyzx)+(r2.yzwy)).xyz;
    // 166: mul r2.xyz, r2.xyzx, cb0[27].wwww
    r2.xyz = ((r2.xyzx)*(source[27].wwww)).xyz;
    // 167: mad r1.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 168: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 170: mad o0.xyz, r0.xyzx, cb0[27].xyzx, r1.xyzx
    output.xyz = ((r0.xyzx)*(source[27].xyzx)+(r1.xyzx)).xyz;
    // 172: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_105_ts_tr: 6869e1c5604f9a4f94e674cfa30cbbda; selected map 23cd2609dd36e6c6eef53dbe66f7a312c34b9f8eab1d7a385d03520d26b5826e.
float4 ArtistNative2624(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = g_ArtistSourceMaterialParameters[5u];
    source[6] = input.dynamicParameter;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[8].yzyy
    r0.xy = ((v4.xyxx)*(source[8].yzyy)).xy;
    // 2: mul r0.z, cb0[7].z, cb0[7].w
    r0.z = ((source[7].zzzz)*(source[7].wwww)).z;
    // 3: mad r0.xy, r0.zzzz, cb0[8].xwxx, r0.xyxx
    r0.xy = ((r0.zzzz)*(source[8].xwxx)+(r0.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t1.xywz, s0, l(0.000000)
    r0.xyw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 5: dp3 r1.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 6: add r1.xyz, -r0.xywx, r1.xxxx
    r1.xyz = ((-(r0.xywx))+(r1.xxxx)).xyz;
    // 7: mad r0.xyw, cb0[9].xxxx, r1.xyxz, r0.xyxw
    r0.xyw = ((source[9].xxxx)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 8: max r0.xyw, |r0.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r0.xyw = (max(abs(r0.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 9: log r0.xyw, r0.xyxw
    r0.xyw = (log2(r0.xyxw)).xyw;
    // 10: mul r0.xyw, r0.xyxw, cb0[9].yyyy
    r0.xyw = ((r0.xyxw)*(source[9].yyyy)).xyw;
    // 11: exp r0.xyw, r0.xyxw
    r0.xyw = (exp2(r0.xyxw)).xyw;
    // 12: mul r1.x, v4.x, cb0[9].w
    r1.x = ((v4.xxxx)*(source[9].wwww)).x;
    // 13: mad r1.x, r0.z, cb0[9].z, r1.x
    r1.x = ((r0.zzzz)*(source[9].zzzz)+(r1.xxxx)).x;
    // 14: mul r1.z, v4.y, cb0[10].x
    r1.z = ((v4.yyyy)*(source[10].xxxx)).z;
    // 15: mad r1.y, r0.z, cb0[10].y, r1.z
    r1.y = ((r0.zzzz)*(source[10].yyyy)+(r1.zzzz)).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.xywz, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).z;
    // 17: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 18: mad r2.xy, r0.xyxx, r1.xyxx, r0.zzzz
    r2.xy = ((r0.xyxx)*(r1.xyxx)+(r0.zzzz)).xy;
    // 19: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 20: mul r1.xy, r2.xyxx, cb0[10].zzzz
    r1.xy = ((r2.xyxx)*(source[10].zzzz)).xy;
    // 21: dp3 r1.z, v6.xyzx, v6.xyzx
    r1.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 22: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 23: mul r2.xyz, r1.zzzz, v6.xyzx
    r2.xyz = ((r1.zzzz)*(v6.xyzx)).xyz;
    // 24: mad r1.xy, r2.xyxx, cb0[3].xyxx, r1.xyxx
    r1.xy = ((r2.xyxx)*(source[3].xyxx)+(r1.xyxx)).xy;
    // 25: add r1.z, -|r2.z|, l(1.000000)
    r1.z = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.xyw, r1.xyxx, t2.xywz, s2, l(0.000000)
    r1.xyw = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 27: dp3 r2.x, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 28: add r2.xyz, -r1.xywx, r2.xxxx
    r2.xyz = ((-(r1.xywx))+(r2.xxxx)).xyz;
    // 29: mad r1.xyw, cb0[10].wwww, r2.xyxz, r1.xyxw
    r1.xyw = ((source[10].wwww)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 30: max r1.xyw, |r1.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r1.xyw = (max(abs(r1.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 31: log r1.xyw, r1.xyxw
    r1.xyw = (log2(r1.xyxw)).xyw;
    // 32: mul r1.xyw, r1.xyxw, cb0[11].xxxx
    r1.xyw = ((r1.xyxw)*(source[11].xxxx)).xyw;
    // 33: exp r1.xyw, r1.xyxw
    r1.xyw = (exp2(r1.xyxw)).xyw;
    // 34: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 35: mul r1.xyw, r1.xyxw, r2.xyxz
    r1.xyw = ((r1.xyxw)*(r2.xyxz)).xyw;
    // 36: log r2.x, |r1.z|
    r2.x = (log2(abs(r1.zzzz))).x;
    // 37: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 38: mul r2.x, r2.x, cb0[11].y
    r2.x = ((r2.xxxx)*(source[11].yyyy)).x;
    // 39: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 40: mul r2.x, r2.x, cb0[11].z
    r2.x = ((r2.xxxx)*(source[11].zzzz)).x;
    // 41: movc r1.z, r1.z, l(0), r2.x
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).z;
    // 42: mad r0.xyw, r1.zzzz, r1.xyxw, r0.xyxw
    r0.xyw = ((r1.zzzz)*(r1.xyxw)+(r0.xyxw)).xyw;
    // 43: mad r0.xyw, r0.xyxw, cb0[1].xyxz, cb0[2].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)+(source[2].xyxz)).xyw;
    // 44: mad o0.xyz, r0.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 45: add r0.x, -cb0[6].x, l(1.000000)
    r0.x = ((-(source[6].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: add r0.x, -r0.x, r0.z
    r0.x = ((-(r0.xxxx))+(r0.zzzz)).x;
    // 47: mul_sat r0.x, r0.x, cb0[11].w
    r0.x = (saturate((r0.xxxx)*(source[11].wwww))).x;
    // 48: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 49: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 50: mul r0.y, r0.y, cb0[12].x
    r0.y = ((r0.yyyy)*(source[12].xxxx)).y;
    // 51: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 52: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 53: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 54: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2624Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_master_01_103_tr: 2b3cb2df48b4d744aa4c26cae927ef12; selected map 41368368e92fcfbc92a6fcb4735e7512bddffc69ffd63a12c8103c2543147a17.
float4 ArtistNative2625(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
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
    // 2: mad r0.y, cb0[4].y, cb0[6].y, cb0[6].z
    r0.y = ((source[4].yyyy)*(source[6].yyyy)+(source[6].zzzz)).y;
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 16: mul_sat r0.x, r0.x, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)*(source[7].zzzz)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 22: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 23: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 24: mul r0.x, v2.x, cb0[4].w
    r0.x = ((v2.xxxx)*(source[4].wwww)).x;
    // 25: mul r0.y, cb0[4].x, cb0[4].y
    r0.y = ((source[4].xxxx)*(source[4].yyyy)).y;
    // 26: mad r1.x, r0.y, cb0[4].z, r0.x
    r1.x = ((r0.yyyy)*(source[4].zzzz)+(r0.xxxx)).x;
    // 27: mul r0.x, v2.y, cb0[5].x
    r0.x = ((v2.yyyy)*(source[5].xxxx)).x;
    // 28: mad r1.y, r0.y, cb0[5].y, r0.x
    r1.y = ((r0.yyyy)*(source[5].yyyy)+(r0.xxxx)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 31: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 32: mad r0.xyz, cb0[5].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 33: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 34: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 35: mul r0.xyz, r0.xyzx, cb0[5].wwww
    r0.xyz = ((r0.xyzx)*(source[5].wwww)).xyz;
    // 36: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 37: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 38: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 39: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2625Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_g_pa_symbolblur_03_tr: fb5674018c37d4478820f1d08d9c3d34; selected map 1787558e474ffbb86f3e325310df9ea162e8d840ddf4ae2179b95d674d1366ad.
float4 ArtistNative2626(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = g_ArtistSourceMaterialParameters[4u];
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ArtistSourceMaterialParameters[8u];
    source[7].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[11].x = ((float4(0.100000001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].z = ((float4(-1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[9].zwzz
    r0.xy = ((v2.xyxx)*(source[9].zwzz)).xy;
    // 2: mad r1.x, cb0[8].x, cb0[9].y, r0.x
    r1.x = ((source[8].xxxx)*(source[9].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[8].x, cb0[10].x, r0.y
    r1.y = ((source[8].xxxx)*(source[10].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.xy, r0.xyxx, cb0[10].yyyy
    r0.xy = ((r0.xyxx)*(source[10].yyyy)).xy;
    // 7: mul r0.zw, v2.xxxy, cb0[8].yyyz
    r0.zw = ((v2.xxxy)*(source[8].yyyz)).zw;
    // 8: mad r1.x, cb0[8].x, cb0[7].w, r0.z
    r1.x = ((source[8].xxxx)*(source[7].wwww)+(r0.zzzz)).x;
    // 9: mad r1.y, cb0[8].x, cb0[8].w, r0.w
    r1.y = ((source[8].xxxx)*(source[8].wwww)+(r0.wwww)).y;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 11: mad r0.xy, cb0[9].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[9].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 12: mul r0.z, v4.x, cb0[7].y
    r0.z = ((v4.xxxx)*(source[7].yyyy)).z;
    // 13: mad r0.w, -cb0[7].y, v4.x, v2.y
    r0.w = ((-(source[7].yyyy))*(v4.xxxx)+(v2.yyyy)).w;
    // 14: mad r0.z, cb0[7].z, r0.w, r0.z
    r0.z = ((source[7].zzzz)*(r0.wwww)+(r0.zzzz)).z;
    // 15: round_pi r0.w, r0.w
    r0.w = (ceil(r0.wwww)).w;
    // 16: add r1.x, -r0.z, v2.y
    r1.x = ((-(r0.zzzz))+(v2.yyyy)).x;
    // 17: mad r1.y, r0.w, r1.x, r0.z
    r1.y = ((r0.wwww)*(r1.xxxx)+(r0.zzzz)).y;
    // 18: mov r1.x, v2.x
    r1.x = (v2.xxxx).x;
    // 19: mad r0.xy, cb0[10].zzzz, r0.xyxx, r1.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(r1.xyxx)).xy;
    // 20: add r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)+(source[2].xyxx)).xy;
    // 21: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.300000, 0.050000), l(0.000000, 0.000000, 1.300000, 1.000000)
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,-0.300000,0.050000))+(float4(0.000000,0.000000,1.300000,1.000000))).zw;
    // 22: mul r0.xy, r0.xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(r0.zwzz)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: mul r1.xyz, r0.xyzx, cb0[3].xyzx
    r1.xyz = ((r0.xyzx)*(source[3].xyzx)).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 26: mad r2.xyz, cb0[4].xyzx, r2.xyzx, -r1.xyzx
    r2.xyz = ((source[4].xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 27: mad r0.xyz, -r0.xyzx, cb0[3].xyzx, r2.wwww
    r0.xyz = ((-(r0.xyzx))*(source[3].xyzx)+(r2.wwww)).xyz;
    // 28: add r3.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 29: dp2 r0.w, l(0.000001, -1.000000, 0.000000, 0.000000), r3.xyxx
    r0.w = (dot((float4(0.000001,-1.000000,0.000000,0.000000)).xy,(r3.xyxx).xy).xxxx).w;
    // 30: add r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)+(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 31: add r1.w, r0.w, -v4.x
    r1.w = ((r0.wwww)+(-(v4.xxxx))).w;
    // 32: mul_sat r1.w, r1.w, l(7.000000)
    r1.w = (saturate((r1.wwww)*(float4(7.000000,7.000000,7.000000,7.000000)))).w;
    // 33: add r3.xy, v2.xyxx, cb0[5].xyxx
    r3.xy = ((v2.xyxx)+(source[5].xyxx)).xy;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyz = (ArtistNativeSample4((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 35: mad_sat r4.xyz, r1.wwww, r3.xyzx, r1.wwww
    r4.xyz = (saturate((r1.wwww)*(r3.xyzx)+(r1.wwww))).xyz;
    // 36: mad r2.xyz, r4.xyzx, r2.xyzx, r1.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 37: mad r0.xyz, r4.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 38: add r1.xy, v4.xxxx, cb0[11].zxzz
    r1.xy = ((v4.xxxx)+(source[11].zxzz)).xy;
    // 39: add r1.xy, r0.wwww, -r1.xyxx
    r1.xy = ((r0.wwww)+(-(r1.xyxx))).xy;
    // 40: mul_sat r0.w, r1.y, l(0.500000)
    r0.w = (saturate((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 41: add_sat r1.x, r1.x, r1.x
    r1.x = (saturate((r1.xxxx)+(r1.xxxx))).x;
    // 42: mad r1.xyz, r1.xxxx, r3.xyzx, r1.xxxx
    r1.xyz = ((r1.xxxx)*(r3.xyzx)+(r1.xxxx)).xyz;
    // 43: mad_sat r3.xyz, r0.wwww, r3.xyzx, r0.wwww
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx)+(r0.wwww))).xyz;
    // 44: add r3.xyz, -r4.xyzx, r3.xyzx
    r3.xyz = ((-(r4.xyzx))+(r3.xyzx)).xyz;
    // 45: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 46: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 47: mul r4.xyz, r3.xyzx, r3.xyzx
    r4.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 48: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 49: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 50: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 51: mad r2.xyz, r3.xyzx, cb0[6].xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(source[6].xyzx)+(r2.xyzx)).xyz;
    // 52: mul r3.xyz, r1.xyzx, r1.xyzx
    r3.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 53: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 54: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 56: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 57: dp3_sat r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (saturate(dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 58: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 59: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 60: mad r0.xyz, v3.xyzx, r2.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r2.xyzx)+(source[1].xyzx)).xyz;
    // 61: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_brokenglass_01_tr: b704e932ad66a6419217a18d5be491e8; selected map 6d94349367ead5b37ba481d26f925794c2e9c5a4e570aef9ca166f9200636c8a.
float4 ArtistNative2627(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[4].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0)))).x;
    source[4].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[4].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[7]; [unroll] for(uint passIndex=0u;passIndex<7u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    uint viewportWidth, viewportHeight; g_EffectSceneDepthTexture.GetDimensions(viewportWidth,viewportHeight);
    passValues[6]=float4(max(float2(viewportWidth,viewportHeight),1.f),0.f,0.f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mov r0.yw, l(0,1.000000,0,0)
    r0.yw = (float4(asfloat(0u),1.000000,asfloat(0u),asfloat(0u))).yw;
    // 2: div r0.x, cb2[6].x, cb2[6].y
    r0.x = ((passValues[6].xxxx)/(passValues[6].yyyy)).x;
    // 3: mul r1.xy, r0.xyxx, v4.xyxx
    r1.xy = ((r0.xyxx)*(v4.xyxx)).xy;
    // 4: mad r1.xy, -r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), r1.xyxx
    r1.xy = ((-(r0.xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 5: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 6: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 7: mad r1.x, -r1.x, l(2.500000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(2.500000,2.500000,2.500000,2.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 9: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 10: mul r0.z, r0.x, l(0.500000)
    r0.z = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 11: mad r0.xy, v4.xyxx, r0.xyxx, -r0.zwzz
    r0.xy = ((v4.xyxx)*(r0.xyxx)+(-(r0.zwzz))).xy;
    // 12: mad r0.zw, r0.xxxx, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.xxxx)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r0.wyww, t2.zxyw, s0, l(0.000000)
    r1.yz = (ArtistNativeSample0((r0.wyww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zyzz, t1.xyzw, s3, l(0.000000)
    r2.xyz = (ArtistNativeSample3((r0.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: add r0.xy, r0.xyxx, l(0.500000, 0.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.000000,0.000000,0.000000))).xy;
    // 16: mad r0.zw, r1.yyyz, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r1.yyyz)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 17: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.700000, 0.700000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.700000,0.700000))).zw;
    // 18: mad r0.zw, r0.xxxy, l(0.000000, 0.000000, 3.000000, 3.000000), r0.zzzw
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,3.000000,3.000000))+(r0.zzzw)).zw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r0.xyxx, t0.wxyz, s2, l(0.000000)
    r1.yzw = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 20: add r0.xy, r0.zwzz, cb0[2].xyxx
    r0.xy = ((r0.zwzz)+(source[2].xyxx)).xy;
    // 21: add r0.zw, r0.zzzw, cb0[3].xxxy
    r0.zw = ((r0.zzzw)+(source[3].xxxy)).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t3.xyzw, s1, l(0.000000)
    r3.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 25: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 27: mad r0.xyz, r3.xyzx, l(0.700000, 0.700000, 0.700000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.700000,0.700000,0.700000,0.000000))+(r0.xyzx)).xyz;
    // 28: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 29: mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // 30: mul r3.xyz, r1.xxxx, r0.xyzx
    r3.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 31: mul r3.xyz, r3.xyzx, l(4.200000, 1.400000, 0.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(4.200000,1.400000,0.000000,0.000000))).xyz;
    // 32: mad r0.xyz, r0.xyzx, l(1.800000, 2.600000, 2.000000, 0.000000), r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(1.800000,2.600000,2.000000,0.000000))+(r3.xyzx)).xyz;
    // 33: add_sat r2.xyz, r2.xyzx, r1.yzwy
    r2.xyz = (saturate((r2.xyzx)+(r1.yzwy))).xyz;
    // 34: add r0.w, r2.y, r2.x
    r0.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 35: add r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)+(r0.wwww)).w;
    // 36: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 37: add r1.x, r1.z, r1.y
    r1.x = ((r1.zzzz)+(r1.yyyy)).x;
    // 38: add r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)+(r1.xxxx)).x;
    // 39: mul r1.x, r1.x, l(0.333330)
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 40: lt r1.y, l(0.900000), cb0[4].w
    r1.y = (asfloat((uint4)((float4(0.900000,0.900000,0.900000,0.900000))<(source[4].wwww)) * 0xffffffffu)).y;
    // 41: movc r0.w, r1.y, r0.w, r1.x
    r0.w = ((asuint(r1.yyyy) != 0u) ? (r0.wwww) : (r1.xxxx)).w;
    // 42: ge r1.y, cb0[4].w, l(0.900000)
    r1.y = (asfloat((uint4)((source[4].wwww)>=(float4(0.900000,0.900000,0.900000,0.900000))) * 0xffffffffu)).y;
    // 43: movc r0.w, r1.y, r0.w, r1.x
    r0.w = ((asuint(r1.yyyy) != 0u) ? (r0.wwww) : (r1.xxxx)).w;
    // 44: mov_sat r1.x, r0.w
    r1.x = (saturate(r0.wwww)).x;
    // 45: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 46: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 47: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul_sat r0.w, r0.w, cb0[5].x
    r0.w = (saturate((r0.wwww)*(source[5].xxxx))).w;
    // 49: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 50: lt r0.w, r1.x, l(0.000001)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 51: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 52: mul r1.x, r1.x, l(15.000000)
    r1.x = ((r1.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 53: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 54: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 55: add r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)+(r0.xyzx)).xyz;
    // 56: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 57: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif
