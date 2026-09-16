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
