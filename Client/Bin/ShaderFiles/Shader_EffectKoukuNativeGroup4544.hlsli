// Original Kouku material programs 4544..4607; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_de_master_01_24_tr: b319111a4ef50d40a6b4b960d6c835c7; selected map b488a3fbf8b59d6601e8a8cb2b5c06528adb732d7f9074fff9d64c1f9e5e5432.
float4 ArtistNative4553(ARTIST_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[16]=float4(input.skyUpperColor,0.f);
    source[17]=float4(input.skyLowerColor,0.f);
    source[18]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[7u];
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[6] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].zzzz,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[8] = (g_ArtistSourceMaterialTime.xxxx*ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,float4(0.0, 0.0, 0.0, 0.0),1u));
    source[9] = g_ArtistSourceMaterialParameters[5u];
    source[10].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
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
    float4 v7 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v8 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v9 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 12: add r0.x, r0.x, -cb0[10].y
    r0.x = ((r0.xxxx)+(-(source[10].yyyy))).x;
    // 13: mul r0.x, r0.x, cb0[10].z
    r0.x = ((r0.xxxx)*(source[10].zzzz)).x;
    // 14: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 15: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 16: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 17: mul r0.yz, r0.yyyy, v6.xxyx
    r0.yz = ((r0.yyyy)*(v6.xxyx)).yz;
    // 18: mul r1.xy, r0.yzyy, r0.xxxx
    r1.xy = ((r0.yzyy)*(r0.xxxx)).xy;
    // 19: mad r1.zw, cb0[11].xxxx, v4.xxxy, r1.xxxy
    r1.zw = ((source[11].xxxx)*(v4.xxxy)+(r1.xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t5.xyzw, s3, l(0.000000)
    r2.xyz = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: add r0.w, r2.y, r2.x
    r0.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 22: add r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)+(r0.wwww)).w;
    // 23: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 24: log r1.z, |r0.w|
    r1.z = (log2(abs(r0.wwww))).z;
    // 25: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 26: mul r1.z, r1.z, cb0[11].y
    r1.z = ((r1.zzzz)*(source[11].yyyy)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: mul_sat r1.z, r1.z, cb0[11].z
    r1.z = (saturate((r1.zzzz)*(source[11].zzzz))).z;
    // 29: movc r0.w, r0.w, l(0), r1.z
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).w;
    // 30: mad r2.xyzw, cb0[10].xxww, v4.xyxy, r1.xyxy
    r2.xyzw = ((source[10].xxww)*(v4.xyxy)+(r1.xyxy)).xyzw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.zwzz, t4.xyzw, s2, l(0.000000)
    r3.xyz = (ArtistNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 33: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 34: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 35: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 36: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 37: mad r3.xyz, cb0[11].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[11].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 38: mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.200000, 0.200000), v4.xxxy
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,0.200000,0.200000))+(v4.xxxy)).zw;
    // 39: add r1.zw, r1.zzzw, cb0[8].xxxy
    r1.zw = ((r1.zzzw)+(source[8].xxxy)).zw;
    // 40: mad r1.zw, r0.xxxx, r0.yyyz, r1.zzzw
    r1.zw = ((r0.xxxx)*(r0.yyyz)+(r1.zzzw)).zw;
    // 41: mad_sat r0.xy, r0.xxxx, r0.yzyy, v4.xyxx
    r0.xy = (saturate((r0.xxxx)*(r0.yzyy)+(v4.xyxx))).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s6, l(0.000000)
    r0.x = (ArtistNativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 43: mul r0.x, r0.x, cb0[12].w
    r0.x = ((r0.xxxx)*(source[12].wwww)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t3.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 45: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 46: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 47: mad r0.zw, v4.xxxy, cb0[5].xxxy, r1.xxxy
    r0.zw = ((v4.xxxy)*(source[5].xxxy)+(r1.xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t6.zwxy, s4, l(0.000000)
    r0.zw = (ArtistNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 49: mad r3.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 50: dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 51: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 53: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 54: add r3.z, r0.z, l(0.000010)
    r3.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 55: dp3 r0.z, r3.xyzx, r3.xyzx
    r0.z = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 56: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 57: div r0.z, r3.z, r0.z
    r0.z = ((r3.zzzz)/(r0.zzzz)).z;
    // 58: mul r0.z, r0.z, cb0[6].z
    r0.z = ((r0.zzzz)*(source[6].zzzz)).z;
    // 59: max r0.z, r0.z, l(0.150000)
    r0.z = (max(r0.zzzz,float4(0.150000,0.150000,0.150000,0.150000))).z;
    // 60: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 61: mul r2.xyz, r2.xyzx, r0.zzzz
    r2.xyz = ((r2.xyzx)*(r0.zzzz)).xyz;
    // 62: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 63: mad r0.zw, r0.zzzw, cb0[7].xxxy, r1.xxxy
    r0.zw = ((r0.zzzw)*(source[7].xxxy)+(r1.xxxy)).zw;
    // 64: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t7.xzyw, s5, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 66: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 68: mul r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 69: mul r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 70: add r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)+(r0.zzzz)).w;
    // 71: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 72: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 73: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 74: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 75: mul r0.y, r0.y, cb0[12].z
    r0.y = ((r0.yyyy)*(source[12].zzzz)).y;
    // 76: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 77: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 78: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 80: movc r0.yzw, r0.zzzz, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 81: mad r0.yzw, r1.xxyz, l(0.000000, 0.150000, 0.150000, 0.150000), r0.yyzw
    r0.yzw = ((r1.xxyz)*(float4(0.000000,0.150000,0.150000,0.150000))+(r0.yyzw)).yzw;
    // 82: mul r1.xyz, r1.xyzx, cb2[3].wwww
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)).xyz;
    // 83: mad r1.xyz, r1.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 84: add r0.yzw, r0.yyzw, cb0[3].xxyz
    r0.yzw = ((r0.yyzw)+(source[3].xxyz)).yzw;
    // 85: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 86: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 87: mul r1.w, r1.w, v7.z
    r1.w = ((r1.wwww)*(v7.zzzz)).w;
    // 88: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 89: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 90: mul r2.yzw, r2.yyyy, cb0[17].xxyz
    r2.yzw = ((r2.yyyy)*(source[17].xxyz)).yzw;
    // 91: mad r2.xyz, r2.xxxx, cb0[16].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[16].xyzx)+(r2.yzwy)).xyz;
    // 92: mul r2.xyz, r2.xyzx, cb0[18].wwww
    r2.xyz = ((r2.xyzx)*(source[18].wwww)).xyz;
    // 93: mad r0.yzw, r2.xxyz, r1.xxyz, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 94: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 96: mad r0.yzw, r1.xxyz, cb0[18].xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(source[18].xxyz)+(r0.yyzw)).yzw;
    // 98: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 99: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 100: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 101: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 102: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 103: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 104: mul r0.yz, v4.xxyx, cb0[13].yyyy
    r0.yz = ((v4.xxyx)*(source[13].yyyy)).yz;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s7, l(0.000000)
    r0.y = (ArtistNativeSample7((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 106: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 107: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 108: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 109: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 110: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 111: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 112: add r0.y, -|v4.w|, cb0[0].y
    r0.y = ((-(abs(v4.wwww)))+(source[0].yyyy)).y;
    // 113: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 114: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 115: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 116: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_decmaster_01_04_tr: 58a9c7c9b8b98d4f85b571086bbb87dd; selected map 5554197888b620f30249fcbfdc2fa35f1a65571b5d578d74814138015ff0e79b.
float4 ArtistNative4554(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ArtistSourceMaterialParameters[3u];
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
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
    // 8: mad r1.xy, r0.xxxx, r0.yzyy, v2.xyxx
    r1.xy = ((r0.xxxx)*(r0.yzyy)+(v2.xyxx)).xy;
    // 9: mul r0.xy, r0.yzyy, r0.xxxx
    r0.xy = ((r0.yzyy)*(r0.xxxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t3.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 11: mad r1.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 13: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 14: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 15: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 16: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 18: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 19: div r0.z, r1.z, r0.z
    r0.z = ((r1.zzzz)/(r0.zzzz)).z;
    // 20: max r0.z, r0.z, l(0.150000)
    r0.z = (max(r0.zzzz,float4(0.150000,0.150000,0.150000,0.150000))).z;
    // 21: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 22: mad r1.xy, cb0[6].xxxx, v2.xyxx, r0.xyxx
    r1.xy = ((source[6].xxxx)*(v2.xyxx)+(r0.xyxx)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 24: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 25: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 26: mad r2.xyz, cb0[6].wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((source[6].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 27: mul r3.xyz, cb0[2].xyzx, cb0[2].wwww
    r3.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 28: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 29: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 30: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 31: mad r0.xy, r0.zwzz, cb0[3].xyxx, r0.xyxx
    r0.xy = ((r0.zwzz)*(source[3].xyxx)+(r0.xyxx)).xy;
    // 32: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.yxzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 34: mad r0.yz, r1.xxyx, l(0.000000, 0.200000, 0.200000, 0.000000), v2.xxyx
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.200000,0.200000,0.000000))+(v2.xxyx)).yz;
    // 35: mul r0.w, r1.w, cb0[7].z
    r0.w = ((r1.wwww)*(source[7].zzzz)).w;
    // 36: add r0.yz, r0.yyzy, cb0[4].xxyx
    r0.yz = ((r0.yyzy)+(source[4].xxyx)).yz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 38: add r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)+(r0.xxxx)).y;
    // 39: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 40: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 41: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 42: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 43: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 44: mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 45: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 46: mul r1.xyz, r1.xyzx, v3.xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)).xyz;
    // 47: movc r0.xyz, r0.xxxx, l(0,0,0,0), r1.xyzx
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 48: mad r0.xyz, r2.xyzx, v3.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(v3.xyzx)+(r0.xyzx)).xyz;
    // 49: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 50: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 51: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 52: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 53: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 54: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 55: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 56: mul r0.yz, v2.xxyx, cb0[8].xxxx
    r0.yz = ((v2.xxyx)*(source[8].xxxx)).yz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 58: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 59: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 61: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 62: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 63: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_sqc_01_04_dt_ad: daf7abe8ec128b438eded6bec25a2245; selected map 1d30749a26a87bece711be04d4d389569b66551530883ea6ebb3d9fe2f50a762.
float4 ArtistNative4555(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[2].z, l(1.000000)
    r0.y = ((-(source[2].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 14: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: sample_l_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s1, cb0[2].x
    r1.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyzw;
    // 16: add_sat r0.y, -r0.y, r1.w
    r0.y = (saturate((-(r0.yyyy))+(r1.wwww))).y;
    // 17: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 18: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 19: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 20: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 21: add r0.yzw, -r1.xxyz, r0.yyyy
    r0.yzw = ((-(r1.xxyz))+(r0.yyyy)).yzw;
    // 22: mad r0.yzw, cb0[2].yyyy, r0.yyzw, r1.xxyz
    r0.yzw = ((source[2].yyyy)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 23: mad r0.yzw, v3.xxyz, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 24: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 25: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 26: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4555Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_06_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative4556(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz),1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].zzzz,g_ArtistSourceMaterialParameters[5u].wwww,1u);
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[12].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
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
    // 1: mov r0.y, cb0[11].z
    r0.y = (source[11].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[3].xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[3].xxxy)+(source[4].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    // 49: mad r0.xy, r0.xxxx, cb0[14].wwww, r0.yzyy
    r0.xy = ((r0.xxxx)*(source[14].wwww)+(r0.yzyy)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 52: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 54: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 55: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 59: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 60: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 61: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 62: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 63: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 64: mul r0.z, r0.z, cb0[13].w
    r0.z = ((r0.zzzz)*(source[13].wwww)).z;
    // 65: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 66: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 67: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 68: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 69: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 70: mul r0.x, r0.x, cb0[15].y
    r0.x = ((r0.xxxx)*(source[15].yyyy)).x;
    // 71: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 72: mul_sat r0.x, r0.x, cb0[15].z
    r0.x = (saturate((r0.xxxx)*(source[15].zzzz))).x;
    // 73: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 74: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4556Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_27_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative4557(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz),1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].zzzz,g_ArtistSourceMaterialParameters[5u].wwww,1u);
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[12].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
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
    // 1: mov r0.y, cb0[11].z
    r0.y = (source[11].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[3].xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[3].xxxy)+(source[4].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    // 49: mad r0.xy, r0.xxxx, cb0[14].wwww, r0.yzyy
    r0.xy = ((r0.xxxx)*(source[14].wwww)+(r0.yzyy)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 52: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 54: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 55: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 59: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 60: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 61: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 62: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 63: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 64: mul r0.z, r0.z, cb0[13].w
    r0.z = ((r0.zzzz)*(source[13].wwww)).z;
    // 65: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 66: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 67: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 68: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 69: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 70: mul r0.x, r0.x, cb0[15].y
    r0.x = ((r0.xxxx)*(source[15].yyyy)).x;
    // 71: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 72: mul_sat r0.x, r0.x, cb0[15].z
    r0.x = (saturate((r0.xxxx)*(source[15].zzzz))).x;
    // 73: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 74: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4557Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_50_tr: 9765660da7e1414994a02fe197e8e364; selected map 63f30317cf581b133c3d9d7bb73869eefa79ea7a22b9497e57433c216f8431cc.
float4 ArtistNative4558(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww),1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz),1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].wwww,g_ArtistSourceMaterialParameters[6u].xxxx,1u);
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[12].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos((g_ArtistSourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    // 1: mov r0.y, cb0[11].z
    r0.y = (source[11].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[3].xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[3].xxxy)+(source[4].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    // 49: mad r0.xy, r0.xxxx, cb0[14].wwww, r0.yzyy
    r0.xy = ((r0.xxxx)*(source[14].wwww)+(r0.yzyy)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 52: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 54: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 55: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 59: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 60: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 61: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 62: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 63: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 64: mul r0.z, r0.z, cb0[13].w
    r0.z = ((r0.zzzz)*(source[13].wwww)).z;
    // 65: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 66: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 67: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 68: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 69: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 70: mul r0.x, r0.x, cb0[15].y
    r0.x = ((r0.xxxx)*(source[15].yyyy)).x;
    // 71: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 72: mul_sat r0.x, r0.x, cb0[15].z
    r0.x = (saturate((r0.xxxx)*(source[15].zzzz))).x;
    // 73: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 74: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4558Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[2] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww),1u);
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz),1u);
    source[5] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz),1u);
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[7].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mov r0.y, cb0[6].z
    r0.y = (source[6].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v2.xxxy, cb0[1].xxxy, cb0[2].xxxy
    r0.zw = ((v2.xxxy)*(source[1].xxxy)+(source[2].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    // 16: lt r0.z, |cb0[6].x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[6].xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 17: mul r0.w, |cb0[6].x|, |cb0[6].x|
    r0.w = ((abs(source[6].xxxx))*(abs(source[6].xxxx))).w;
    // 18: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 19: mul r1.y, r0.z, v2.y
    r1.y = ((r0.zzzz)*(v2.yyyy)).y;
    // 20: mul r1.x, v2.x, cb0[6].y
    r1.x = ((v2.xxxx)*(source[6].yyyy)).x;
    // 21: mad r0.zw, cb0[3].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[3].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 22: add r0.w, r0.w, cb0[3].w
    r0.w = ((r0.wwww)+(source[3].wwww)).w;
    // 23: add r0.zw, r0.zzzw, cb0[4].xxxy
    r0.zw = ((r0.zzzw)+(source[4].xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t1.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 25: lt r0.z, |cb0[7].z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[7].zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.w, |cb0[7].z|, |cb0[7].z|
    r0.w = ((abs(source[7].zzzz))*(abs(source[7].zzzz))).w;
    // 27: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 28: mul r2.y, r0.z, v2.y
    r2.y = ((r0.zzzz)*(v2.yyyy)).y;
    // 29: mul r2.x, v2.x, cb0[7].w
    r2.x = ((v2.xxxx)*(source[7].wwww)).x;
    // 30: mad r0.xy, cb0[3].xxxx, r0.xyxx, r2.xyxx
    r0.xy = ((source[3].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 31: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 33: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 34: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 35: mad r0.xyzw, -r1.xyxy, r0.xyxy, r0.zzzz
    r0.xyzw = ((-(r1.xyxy))*(r0.xyxy)+(r0.zzzz)).xyzw;
    // 36: mad r0.xyzw, cb0[8].xxxx, r0.xyzw, r2.xyxy
    r0.xyzw = ((source[8].xxxx)*(r0.xyzw)+(r2.xyxy)).xyzw;
    // 37: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 38: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 39: mul r0.xyzw, r0.xyzw, cb0[8].yyyy
    r0.xyzw = ((r0.xyzw)*(source[8].yyyy)).xyzw;
    // 40: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 41: mul r0.xyzw, r0.xyzw, cb0[8].zzzz
    r0.xyzw = ((r0.xyzw)*(source[8].zzzz)).xyzw;
    // 42: mul r0.xyzw, r0.xyzw, cb0[0].xyxy
    r0.xyzw = ((r0.xyzw)*(source[0].xyxy)).xyzw;
    // 43: mul r0.xyzw, r0.xyzw, cb0[9].wwww
    r0.xyzw = ((r0.xyzw)*(source[9].wwww)).xyzw;
    // 44: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
    // 45: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 46: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 47: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 48: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 49: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 50: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 51: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 52: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 53: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 54: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 55: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 56: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 57: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 58: source device depth mapped to centimetre view depth; reconstruction at 60.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 60-63: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 64: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 65: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 66: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 67: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 68: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_simplestep_01_03_tr: 4494f5cfc8e2b84f9d9bd59f211d9804; selected map 997eb6cc3812f2d6c09c7d94f7cb73504c873de8b39b21f0a8df926848f8e406.
float4 ArtistNative4559(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[4].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = ((g_ArtistSourceMaterialParameters[0u].wwww*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 4: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 6: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 7: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 8: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 9: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 10: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 11: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 12: add r1.xy, -v2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((-(v2.xyxx))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: mul r0.zw, r0.zzzz, r1.xxxy
    r0.zw = ((r0.zzzz)*(r1.xxxy)).zw;
    // 14: movc r0.yz, r0.yyyy, l(0,0,0,0), r0.zzwz
    r0.yz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzwz)).yz;
    // 15: add r0.yz, r0.yyzy, v2.xxyx
    r0.yz = ((r0.yyzy)+(v2.xxyx)).yz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 17: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 18: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 19: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: mul r0.z, r0.z, cb0[6].y
    r0.z = ((r0.zzzz)*(source[6].yyyy)).z;
    // 21: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 22: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 23: add r0.z, -v3.w, cb0[6].z
    r0.z = ((-(v3.wwww))+(source[6].zzzz)).z;
    // 24: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: ge r0.z, r0.z, r0.y
    r0.z = (asfloat((uint4)((r0.zzzz)>=(r0.yyyy)) * 0xffffffffu)).z;
    // 26: movc r0.z, r0.z, l(0), l(1.000000)
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: add r0.w, r0.z, r0.y
    r0.w = ((r0.zzzz)+(r0.yyyy)).w;
    // 28: mul r0.z, r0.z, r0.y
    r0.z = ((r0.zzzz)*(r0.yyyy)).z;
    // 29: mul r1.xyz, r0.yyyy, v3.xyzx
    r1.xyz = ((r0.yyyy)*(v3.xyzx)).xyz;
    // 30: mul r0.y, r0.z, r0.w
    r0.y = ((r0.zzzz)*(r0.wwww)).y;
    // 31: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 32: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 33: mad r0.w, cb0[6].w, l(10.000000), l(10.000000)
    r0.w = ((source[6].wwww)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 34: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, cb0[7].x
    r0.z = ((r0.zzzz)*(source[7].xxxx)).z;
    // 37: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 38: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 39: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 40: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 41: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 42: source device depth mapped to centimetre view depth; reconstruction at 44.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 44-47: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 48: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 49: add r0.z, -cb0[7].y, l(1.000000)
    r0.z = ((-(source[7].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 51: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 52: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 53: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 54: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 55: add r0.xy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((v2.xyxx)+(source[2].xyxx)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 57: mad r2.xy, v2.xyxx, l(2.000000, 1.000000, 0.000000, 0.000000), cb0[3].xyxx
    r2.xy = ((v2.xyxx)*(float4(2.000000,1.000000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 59: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 60: mul r0.xyz, r0.xyzx, cb0[4].yyyy
    r0.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 61: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 62: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 63: mul r0.xyz, r0.xyzx, cb0[4].zzzz
    r0.xyz = ((r0.xyzx)*(source[4].zzzz)).xyz;
    // 64: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 65: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 66: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 67: mad r0.xyz, cb0[4].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[4].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 68: mad r0.xyz, r0.xyzx, r1.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 69: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 70: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_master_01_16_ad: c9cf79250d787c4b8faa16cdb042f143; selected map 13e5eb199290fad9b403839d0b82b6413cb6d1816b19beac682507fa4b8ad99c.
float4 ArtistNative4560(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].xxxx)).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: add r0.x, cb0[3].y, l(-1.000000)
    r0.x = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[5].y, cb0[9].y, cb0[9].z
    r0.y = ((source[5].yyyy)*(source[9].yyyy)+(source[9].zzzz)).y;
    // 3: sincos r1.x, r2.x, r0.y
    { const float4 sourceAngle = r0.yyyy; r1.x = (sin(sourceAngle)).x; r2.x = (cos(sourceAngle)).x; }
    // 4: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 5: mul r0.yz, v4.xxyx, cb0[6].zzwz
    r0.yz = ((v4.xxyx)*(source[6].zzwz)).yz;
    // 6: mul r0.w, cb0[5].x, cb0[5].y
    r0.w = ((source[5].xxxx)*(source[5].yyyy)).w;
    // 7: mad r4.x, r0.w, cb0[6].y, r0.y
    r4.x = ((r0.wwww)*(source[6].yyyy)+(r0.yyyy)).x;
    // 8: mad r4.y, r0.w, cb0[7].x, r0.z
    r4.y = ((r0.wwww)*(source[7].xxxx)+(r0.zzzz)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.yz = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 10: mad r0.yz, cb0[7].zzzz, r0.yyzy, v4.xxyx
    r0.yz = ((source[7].zzzz)*(r0.yyzy)+(v4.xxyx)).yz;
    // 11: add r1.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 12: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 13: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 14: dp2 r1.x, r3.zyzz, r1.yzyy
    r1.x = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).x;
    // 15: dp2 r1.y, r3.yxyy, r1.yzyy
    r1.y = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).y;
    // 16: mad r2.x, r1.y, cb0[4].x, r0.x
    r2.x = ((r1.yyyy)*(source[4].xxxx)+(r0.xxxx)).x;
    // 17: mul r2.z, r1.x, cb0[4].y
    r2.z = ((r1.xxxx)*(source[4].yyyy)).z;
    // 18: add r1.xy, r2.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.zxyw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 20: mul r1.xy, r0.yzyy, cb0[8].yzyy
    r1.xy = ((r0.yzyy)*(source[8].yzyy)).xy;
    // 21: mad r1.xy, r0.wwww, cb0[8].xwxx, r1.xyxx
    r1.xy = ((r0.wwww)*(source[8].xwxx)+(r1.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.zxyw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 23: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 24: mul r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)*(source[5].wwww)).y;
    // 25: mad r1.x, r0.w, cb0[5].z, r0.y
    r1.x = ((r0.wwww)*(source[5].zzzz)+(r0.yyyy)).x;
    // 26: mul r0.y, r0.w, cb0[7].w
    r0.y = ((r0.wwww)*(source[7].wwww)).y;
    // 27: mad r1.y, cb0[6].x, r0.z, r0.y
    r1.y = ((source[6].xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.xzyw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 29: add r0.z, -cb0[3].x, l(1.000000)
    r0.z = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: mad r0.x, r0.y, r0.x, -r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 31: mul_sat r0.x, r0.x, cb0[10].y
    r0.x = (saturate((r0.xxxx)*(source[10].yyyy))).x;
    // 32: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 33: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 34: mul r0.y, r0.y, cb0[10].z
    r0.y = ((r0.yyyy)*(source[10].zzzz)).y;
    // 35: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 36: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 37: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 38: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 39: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 40: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 41: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 42: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4560Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0] = input.dynamicParameter;
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[2].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].xxxx)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.x, cb0[0].y, l(-1.000000)
    r0.x = ((source[0].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[2].y, cb0[6].y, cb0[6].z
    r0.y = ((source[2].yyyy)*(source[6].yyyy)+(source[6].zzzz)).y;
    // 3: sincos r1.x, r2.x, r0.y
    { const float4 sourceAngle = r0.yyyy; r1.x = (sin(sourceAngle)).x; r2.x = (cos(sourceAngle)).x; }
    // 4: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 5: mul r0.yz, v2.xxyx, cb0[3].zzwz
    r0.yz = ((v2.xxyx)*(source[3].zzwz)).yz;
    // 6: mul r0.w, cb0[2].x, cb0[2].y
    r0.w = ((source[2].xxxx)*(source[2].yyyy)).w;
    // 7: mad r4.x, r0.w, cb0[3].y, r0.y
    r4.x = ((r0.wwww)*(source[3].yyyy)+(r0.yyyy)).x;
    // 8: mad r4.y, r0.w, cb0[4].x, r0.z
    r4.y = ((r0.wwww)*(source[4].xxxx)+(r0.zzzz)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r4.xyxx, t0.zxyw, s1, l(0.000000)
    r0.yz = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 10: mad r0.yz, cb0[4].zzzz, r0.yyzy, v2.xxyx
    r0.yz = ((source[4].zzzz)*(r0.yyzy)+(v2.xxyx)).yz;
    // 11: add r1.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 12: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 13: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 14: dp2 r1.x, r3.zyzz, r1.yzyy
    r1.x = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).x;
    // 15: dp2 r1.y, r3.yxyy, r1.yzyy
    r1.y = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).y;
    // 16: mad r2.x, r1.y, cb0[1].x, r0.x
    r2.x = ((r1.yyyy)*(source[1].xxxx)+(r0.xxxx)).x;
    // 17: mul r2.z, r1.x, cb0[1].y
    r2.z = ((r1.xxxx)*(source[1].yyyy)).z;
    // 18: add r1.xy, r2.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.zxyw, s4, l(0.000000)
    r0.x = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 20: mul r1.xy, r0.yzyy, cb0[5].yzyy
    r1.xy = ((r0.yzyy)*(source[5].yzyy)).xy;
    // 21: mad r1.xy, r0.wwww, cb0[5].xwxx, r1.xyxx
    r1.xy = ((r0.wwww)*(source[5].xwxx)+(r1.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.zxyw, s3, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 23: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 24: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 25: mad r1.x, r0.w, cb0[2].z, r0.y
    r1.x = ((r0.wwww)*(source[2].zzzz)+(r0.yyyy)).x;
    // 26: mul r0.y, r0.w, cb0[4].w
    r0.y = ((r0.wwww)*(source[4].wwww)).y;
    // 27: mad r1.y, cb0[3].x, r0.z, r0.y
    r1.y = ((source[3].xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.xzyw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 29: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 30: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 31: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 32: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
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
    // 38: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
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
    // 51: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
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
// fx_d_pa_fire_01_08_tr: 768f1d9b30825c4bb1aa3ddd53be6aac; selected map 66ea45cff0db51fa4f2f7888db1c26ff6ef11b25ce01d97d4f77fed5a7534760.
float4 ArtistNative4561(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4561Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_blacklineaura_01_14_tr: 49770286d8a73d4197f56452cd319275; selected map d13061f001acb31979af624603759d6e97ccc872f96216b28d0f2c24ce18b2de.
float4 ArtistNative4562(ARTIST_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[17u];
    source[2] = g_ArtistSourceMaterialParameters[15u];
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = g_ArtistSourceMaterialParameters[16u];
    source[10].x = (cos((g_ArtistSourceMaterialParameters[3u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[11].y = ((g_ArtistSourceMaterialParameters[5u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[11].w = ((g_ArtistSourceMaterialParameters[6u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[13].x = ((g_ArtistSourceMaterialParameters[7u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[13].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].z = ((g_ArtistSourceMaterialParameters[7u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[13].w = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
    source[16].z = ((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[16].w = (sin((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[17].y = (cos((g_ArtistSourceMaterialParameters[11u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[17].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[18].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[18].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[18].z = (g_ArtistSourceMaterialParameters[13u].zzzz).x;
    source[18].w = (g_ArtistSourceMaterialParameters[13u].wwww).x;
    source[19].x = (g_ArtistSourceMaterialParameters[14u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[11u].wwww).x;
    source[19].z = (g_ArtistSourceMaterialParameters[12u].xxxx).x;
    source[19].w = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[20].x = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[20].y = ((g_ArtistSourceMaterialParameters[10u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[20].z = (((g_ArtistSourceMaterialParameters[10u].zzzz*g_ArtistSourceMaterialTime.xxxx)+g_ArtistSourceMaterialParameters[10u].xxxx)).x;
    source[20].w = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[21].x = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[21].y = ((g_ArtistSourceMaterialParameters[10u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[21].z = (((g_ArtistSourceMaterialParameters[10u].wwww*g_ArtistSourceMaterialTime.xxxx)+g_ArtistSourceMaterialParameters[10u].yyyy)).x;
    source[21].w = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[22].x = (g_ArtistSourceMaterialParameters[11u].zzzz).x;
    source[22].y = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[22].z = ((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[22].w = (sin((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[23].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[23].y = (cos((g_ArtistSourceMaterialParameters[9u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[23].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[23].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[24].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[24].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[24].z = (g_ArtistSourceMaterialParameters[12u].wwww).x;
    source[24].w = (g_ArtistSourceMaterialParameters[13u].xxxx).x;
    source[25].x = (g_ArtistSourceMaterialParameters[13u].yyyy).x;
    source[25].y = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[25].z = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[25].w = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[26].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[26].y = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[26].z = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[26].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[27].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[27].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[27].z = (g_ArtistSourceMaterialParameters[12u].zzzz).x;
    source[27].w = (g_ArtistSourceMaterialParameters[12u].yyyy).x;
    source[28].x = (g_ArtistSourceMaterialParameters[14u].yyyy).x;
    source[28].y = (g_ArtistSourceMaterialParameters[14u].zzzz).x;
    source[28].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[28].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[29].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[29].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[29].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[29].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mad r0.xy, v4.wwww, cb0[14].yzyy, v2.xyxx
    r0.xy = ((v4.wwww)*(source[14].yzyy)+(v2.xyxx)).xy;
    // 2: mul r1.x, r0.x, cb0[14].w
    r1.x = ((r0.xxxx)*(source[14].wwww)).x;
    // 3: mul r1.y, r0.y, cb0[15].x
    r1.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 4: add r0.xy, r1.xyxx, cb0[15].yzyy
    r0.xy = ((r1.xyxx)+(source[15].yzyy)).xy;
    // 5: mad r0.zw, v2.xxxy, cb0[12].yyyz, cb0[13].xxxz
    r0.zw = ((v2.xxxy)*(source[12].yyyz)+(source[13].xxxz)).zw;
    // 6: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(-1.000000)
    r0.zw = (ArtistNativeSample1((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zwxy).zw;
    // 7: mul r0.zw, r0.zzzw, cb0[13].wwww
    r0.zw = ((r0.zzzw)*(source[13].wwww)).zw;
    // 8: mad r1.xy, v2.xyxx, cb0[10].yzyy, cb0[11].ywyy
    r1.xy = ((v2.xyxx)*(source[10].yzyy)+(source[11].ywyy)).xy;
    // 9: sample_l_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s2, l(-1.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 10: mad r0.zw, cb0[12].xxxx, r1.xxxy, r0.zzzw
    r0.zw = ((source[12].xxxx)*(r1.xxxy)+(r0.zzzw)).zw;
    // 11: mad r0.xy, cb0[14].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[14].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 12: mul r0.zw, r0.zzzw, v4.yyyy
    r0.zw = ((r0.zzzw)*(v4.yyyy)).zw;
    // 13: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 1.010000, 1.010000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,1.010000,1.010000))).zw;
    // 14: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 15: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 16: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 17: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 18: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t5.xyzw, s1, l(-1.000000)
    r1.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 19: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 20: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 21: mul r1.xyz, r1.xyzx, cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(source[15].wwww)).xyz;
    // 22: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, cb0[16].xxxx
    r1.xyz = ((r1.xyzx)*(source[16].xxxx)).xyz;
    // 24: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 25: mad r2.x, r0.x, cb0[17].w, l(0.500000)
    r2.x = ((r0.xxxx)*(source[17].wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: mad r2.y, r0.y, cb0[18].y, l(0.500000)
    r2.y = ((r0.yyyy)*(source[18].yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 27: mad r2.xy, v4.zzzz, cb0[18].zwzz, r2.xyxx
    r2.xy = ((v4.zzzz)*(source[18].zwzz)+(r2.xyxx)).xy;
    // 28: mad r2.xy, cb0[19].xxxx, r0.zwzz, r2.xyxx
    r2.xy = ((source[19].xxxx)*(r0.zwzz)+(r2.xyxx)).xy;
    // 29: mad r3.x, r2.x, cb0[19].y, cb0[20].z
    r3.x = ((r2.xxxx)*(source[19].yyyy)+(source[20].zzzz)).x;
    // 30: mad r3.y, r2.y, cb0[19].z, cb0[21].z
    r3.y = ((r2.yyyy)*(source[19].zzzz)+(source[21].zzzz)).y;
    // 31: add r2.xy, r3.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 32: dp2 r3.x, cb0[5].xyxx, r2.xyxx
    r3.x = (dot((source[5].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 33: dp2 r3.y, cb0[6].xyxx, r2.xyxx
    r3.y = (dot((source[6].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 34: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t2.yzwx, s3, l(-1.000000)
    r1.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 36: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 37: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 38: mul r2.x, r2.x, cb0[21].w
    r2.x = ((r2.xxxx)*(source[21].wwww)).x;
    // 39: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 40: mul r2.x, r2.x, cb0[22].x
    r2.x = ((r2.xxxx)*(source[22].xxxx)).x;
    // 41: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 42: mad r2.x, r0.x, cb0[23].w, l(0.500000)
    r2.x = ((r0.xxxx)*(source[23].wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 43: mad r2.y, r0.y, cb0[24].y, l(0.500000)
    r2.y = ((r0.yyyy)*(source[24].yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 44: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 45: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 46: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mad r2.xy, v4.xxxx, cb0[24].zwzz, r2.xyxx
    r2.xy = ((v4.xxxx)*(source[24].zwzz)+(r2.xyxx)).xy;
    // 48: mad r0.yz, cb0[25].xxxx, r0.zzwz, r2.xxyx
    r0.yz = ((source[25].xxxx)*(r0.zzwz)+(r2.xxyx)).yz;
    // 49: mad r2.x, r0.y, cb0[25].y, cb0[25].w
    r2.x = ((r0.yyyy)*(source[25].yyyy)+(source[25].wwww)).x;
    // 50: mad r2.y, r0.z, cb0[25].z, cb0[26].x
    r2.y = ((r0.zzzz)*(source[25].zzzz)+(source[26].xxxx)).y;
    // 51: add r0.yz, r2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 52: dp2 r2.x, cb0[7].xyxx, r0.yzyy
    r2.x = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 53: dp2 r2.y, cb0[8].xyxx, r0.yzyy
    r2.y = (dot((source[8].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 54: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 55: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s4, l(-1.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 56: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 57: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 58: mul r0.y, r0.y, cb0[26].y
    r0.y = ((r0.yyyy)*(source[26].yyyy)).y;
    // 59: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 60: mul r0.y, r0.y, cb0[26].z
    r0.y = ((r0.yyyy)*(source[26].zzzz)).y;
    // 61: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 62: add r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)+(r1.wwww)).z;
    // 63: mul r2.xyz, r1.xyzx, r0.zzzz
    r2.xyz = ((r1.xyzx)*(r0.zzzz)).xyz;
    // 64: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: mad r1.xyz, -r0.zzzz, r1.xyzx, r0.wwww
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(r0.wwww)).xyz;
    // 66: mad r1.xyz, cb0[26].wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((source[26].wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 67: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 68: mad r0.z, r0.y, r1.w, r0.z
    r0.z = ((r0.yyyy)*(r1.wwww)+(r0.zzzz)).z;
    // 69: mad r0.y, -r0.y, r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 70: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 71: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 72: mov_sat r0.z, r0.z
    r0.z = (saturate(r0.zzzz)).z;
    // 73: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 74: mul r0.y, r0.y, cb0[27].x
    r0.y = ((r0.yyyy)*(source[27].xxxx)).y;
    // 75: lt r0.w, |r0.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 76: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 77: mul r0.y, r0.y, cb0[27].y
    r0.y = ((r0.yyyy)*(source[27].yyyy)).y;
    // 78: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 79: mul r2.xyz, r0.yyyy, cb0[9].xyzx
    r2.xyz = ((r0.yyyy)*(source[9].xyzx)).xyz;
    // 80: mul r2.xyz, r2.xyzx, v3.xyzx
    r2.xyz = ((r2.xyzx)*(v3.xyzx)).xyz;
    // 81: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 82: mad r1.xyz, cb0[2].xyzx, r1.xyzx, r2.xyzx
    r1.xyz = ((source[2].xyzx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 83: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 84: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 85: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 86: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 87: mad r0.w, cb0[27].z, l(10.000000), l(10.000000)
    r0.w = ((source[27].zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 88: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 89: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 90: mul r0.y, r0.y, cb0[27].w
    r0.y = ((r0.yyyy)*(source[27].wwww)).y;
    // 91: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 92: max r0.x, r0.x, cb0[28].y
    r0.x = (max(r0.xxxx,source[28].yyyy)).x;
    // 93: min r0.x, r0.x, cb0[28].x
    r0.x = (min(r0.xxxx,source[28].xxxx)).x;
    // 94: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 95: mul r0.y, v2.x, cb0[28].w
    r0.y = ((v2.xxxx)*(source[28].wwww)).y;
    // 96: mad r1.x, cb0[10].w, cb0[28].z, r0.y
    r1.x = ((source[10].wwww)*(source[28].zzzz)+(r0.yyyy)).x;
    // 97: mul r0.y, v2.y, cb0[29].x
    r0.y = ((v2.yyyy)*(source[29].xxxx)).y;
    // 98: mad r1.y, cb0[10].w, cb0[29].y, r0.y
    r1.y = ((source[10].wwww)*(source[29].yyyy)+(r0.yyyy)).y;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t3.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 100: add r0.y, r0.y, l(0.200000)
    r0.y = ((r0.yyyy)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 101: add r0.z, -v3.w, l(1.000000)
    r0.z = ((-(v3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 102: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 103: mul_sat r0.y, r0.y, cb0[29].z
    r0.y = (saturate((r0.yyyy)*(source[29].zzzz))).y;
    // 104: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 105: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 106: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 107: source device depth mapped to centimetre view depth; reconstruction at 109.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 109-112: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 113: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 114: add r0.z, -cb0[29].w, l(1.000000)
    r0.z = ((-(source[29].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 115: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 116: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 117: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 118: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 119: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_skintrail_03_10_tr: ea2513c856c4684b96fad45fd0cfbd8f; selected map 1224087d6ae1c1983876b5d3d93b7928859246dd52c2c810aea096759f9af486.
float4 ArtistNative4563(ARTIST_NATIVE_INPUT input)
{
    float4 source[29]; [unroll] for (uint i=0u; i<29u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[16u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = g_ArtistSourceMaterialParameters[15u];
    source[5] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[8u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[8u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[8u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[8u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = g_ArtistSourceMaterialParameters[14u];
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10].x = (cos((g_ArtistSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[13u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[13u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[11].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[13u].xxxx)).x;
    source[11].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[13u].xxxx)*float4(-1.0, 0.0, 0.0, 0.0))).x;
    source[12].x = (g_ArtistSourceMaterialParameters[13u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[13].y = ((g_ArtistSourceMaterialParameters[5u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[13].z = (((g_ArtistSourceMaterialParameters[5u].wwww*g_ArtistSourceMaterialTime.xxxx)+g_ArtistSourceMaterialParameters[5u].yyyy)).x;
    source[13].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].y = ((g_ArtistSourceMaterialParameters[6u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[14].z = (((g_ArtistSourceMaterialParameters[6u].xxxx*g_ArtistSourceMaterialTime.xxxx)+g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[14].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].w = ((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[16].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[16].y = ((g_ArtistSourceMaterialParameters[2u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[16].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[17].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[17].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[7u].wwww)).x;
    source[18].x = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[18].y = ((g_ArtistSourceMaterialParameters[7u].yyyy+(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[7u].wwww))).x;
    source[18].z = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[18].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[8u].xxxx)).x;
    source[19].x = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[19].y = ((g_ArtistSourceMaterialParameters[7u].zzzz+(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[8u].xxxx))).x;
    source[19].z = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[20].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[20].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[20].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[21].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[21].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[21].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[21].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[22].x = (g_ArtistSourceMaterialParameters[13u].yyyy).x;
    source[22].y = (g_ArtistSourceMaterialParameters[12u].xxxx).x;
    source[22].z = ((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[22].w = (sin((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[23].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[23].y = (cos((g_ArtistSourceMaterialParameters[12u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[23].z = (g_ArtistSourceMaterialParameters[12u].zzzz).x;
    source[23].w = (g_ArtistSourceMaterialParameters[12u].wwww).x;
    source[24].x = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[24].y = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
    source[24].z = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[24].w = (g_ArtistSourceMaterialParameters[11u].zzzz).x;
    source[25].x = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[25].y = (g_ArtistSourceMaterialParameters[12u].yyyy).x;
    source[25].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[25].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[26].x = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[26].y = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[26].z = (g_ArtistSourceMaterialParameters[11u].wwww).x;
    source[26].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[27].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[27].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[27].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[27].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[28].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
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
    // 1: mad r0.x, cb0[10].w, v2.x, cb0[11].w
    r0.x = ((source[10].wwww)*(v2.xxxx)+(source[11].wwww)).x;
    // 2: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 3: sincos r0.x, null, r0.x
    { const float4 sourceAngle = r0.xxxx; r0.x = (sin(sourceAngle)).x; }
    // 4: mad r0.x, r0.x, v4.x, l(-0.500000)
    r0.x = ((r0.xxxx)*(v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 5: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: mul r1.xy, r0.yzyy, cb0[10].yzyy
    r1.xy = ((r0.yzyy)*(source[10].yzyy)).xy;
    // 7: mad r0.x, r0.x, cb0[12].x, r1.y
    r0.x = ((r0.xxxx)*(source[12].xxxx)+(r1.yyyy)).x;
    // 8: add r0.x, r0.x, l(0.250000)
    r0.x = ((r0.xxxx)+(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 9: mul r1.z, r0.x, l(0.200000)
    r1.z = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 10: mad r0.x, r0.y, cb0[12].y, cb0[13].z
    r0.x = ((r0.yyyy)*(source[12].yyyy)+(source[13].zzzz)).x;
    // 11: mad r0.y, r0.z, cb0[12].z, cb0[14].z
    r0.y = ((r0.zzzz)*(source[12].zzzz)+(source[14].zzzz)).y;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 13: mad r0.x, r0.x, l(2.000000), l(-1.000000)
    r0.x = ((r0.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 14: mul r0.y, v4.z, cb0[14].w
    r0.y = ((v4.zzzz)*(source[14].wwww)).y;
    // 15: mad r0.zw, r0.xxxx, r0.yyyy, r1.xxxz
    r0.zw = ((r0.xxxx)*(r0.yyyy)+(r1.xxxz)).zw;
    // 16: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 17: mad r1.x, r0.z, cb0[17].x, cb0[18].y
    r1.x = ((r0.zzzz)*(source[17].xxxx)+(source[18].yyyy)).x;
    // 18: mad r1.y, r0.w, cb0[17].y, cb0[19].y
    r1.y = ((r0.wwww)*(source[17].yyyy)+(source[19].yyyy)).y;
    // 19: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 20: dp2 r2.x, cb0[5].xyxx, r1.xyxx
    r2.x = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 21: dp2 r2.y, cb0[6].xyxx, r1.xyxx
    r2.y = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 22: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: mul r1.xyz, r1.xyzx, cb0[19].zzzz
    r1.xyz = ((r1.xyzx)*(source[19].zzzz)).xyz;
    // 25: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 26: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 27: mul r1.xyz, r1.xyzx, cb0[19].wwww
    r1.xyz = ((r1.xyzx)*(source[19].wwww)).xyz;
    // 28: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 29: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 30: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 31: lt r1.y, r1.x, l(0.000003)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000003,0.000003,0.000003,0.000003))) * 0xffffffffu)).y;
    // 32: mul r1.x, r1.x, l(0.333330)
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 33: rsq r1.z, r1.x
    r1.z = (rsqrt(r1.xxxx)).z;
    // 34: div r1.z, l(1.000000, 1.000000, 1.000000, 1.000000), r1.z
    r1.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.zzzz)).z;
    // 35: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 36: dp3 r1.z, v6.xyzx, v6.xyzx
    r1.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 37: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 38: mul r1.zw, r1.zzzz, v6.xxxy
    r1.zw = ((r1.zzzz)*(v6.xxxy)).zw;
    // 39: mul r2.x, cb0[20].w, l(-0.500000)
    r2.x = ((source[20].wwww)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 40: mad r2.x, cb0[20].w, cb0[21].x, r2.x
    r2.x = ((source[20].wwww)*(source[21].xxxx)+(r2.xxxx)).x;
    // 41: mad r1.zw, r2.xxxx, r1.zzzw, r0.zzzw
    r1.zw = ((r2.xxxx)*(r1.zzzw)+(r0.zzzw)).zw;
    // 42: mul r1.zw, r1.zzzw, cb0[20].yyyz
    r1.zw = ((r1.zzzw)*(source[20].yyyz)).zw;
    // 43: mad r2.x, cb0[11].y, cb0[20].x, r1.z
    r2.x = ((source[11].yyyy)*(source[20].xxxx)+(r1.zzzz)).x;
    // 44: mad r2.y, cb0[11].y, cb0[21].y, r1.w
    r2.y = ((source[11].yyyy)*(source[21].yyyy)+(r1.wwww)).y;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t6.xyzw, s3, l(0.000000)
    r2.xyz = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: mul r2.xyz, r2.xyzx, cb0[21].zzzz
    r2.xyz = ((r2.xyzx)*(source[21].zzzz)).xyz;
    // 47: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 48: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 49: mul r2.xyz, r2.xyzx, cb0[21].wwww
    r2.xyz = ((r2.xyzx)*(source[21].wwww)).xyz;
    // 50: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 51: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 52: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 53: mad r2.xyz, cb0[22].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[22].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 54: mul r2.xyz, r2.xyzx, cb0[7].xyzx
    r2.xyz = ((r2.xyzx)*(source[7].xyzx)).xyz;
    // 55: mul r1.yzw, r1.yyyy, r2.xxyz
    r1.yzw = ((r1.yyyy)*(r2.xxyz)).yzw;
    // 56: mad r2.x, r0.z, cb0[15].x, cb0[15].w
    r2.x = ((r0.zzzz)*(source[15].xxxx)+(source[15].wwww)).x;
    // 57: mad r2.y, r0.w, cb0[15].y, cb0[16].y
    r2.y = ((r0.wwww)*(source[15].yyyy)+(source[16].yyyy)).y;
    // 58: add r0.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 59: dp2 r2.x, cb0[2].xyxx, r0.zwzz
    r2.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 60: dp2 r2.y, cb0[3].xyxx, r0.zwzz
    r2.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 61: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t5.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 63: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 64: add r3.xyz, -r2.xyzx, r0.zzzz
    r3.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 65: mad r2.xyz, cb0[16].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 66: mad r1.yzw, r2.xxyz, cb0[4].xxyz, r1.yyzw
    r1.yzw = ((r2.xxyz)*(source[4].xxyz)+(r1.yyzw)).yzw;
    // 67: mad r1.yzw, r1.yyzw, v3.xxyz, cb0[1].xxyz
    r1.yzw = ((r1.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 68: mad o0.xyz, r1.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r1.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 69: mad r0.zw, v4.wwww, cb0[24].xxxz, cb0[24].yyyw
    r0.zw = ((v4.wwww)*(source[24].xxxz)+(source[24].yyyw)).zw;
    // 70: mad r0.zw, v2.xxxy, cb0[23].zzzw, r0.zzzw
    r0.zw = ((v2.xxxy)*(source[23].zzzw)+(r0.zzzw)).zw;
    // 71: mad r0.xy, r0.xxxx, r0.yyyy, r0.zwzz
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(r0.zwzz)).xy;
    // 72: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 73: dp2 r2.x, cb0[8].xyxx, r0.xyxx
    r2.x = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 74: dp2 r2.y, cb0[9].xyxx, r0.xyxx
    r2.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 75: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 77: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 78: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 79: mul r0.x, r0.x, cb0[25].x
    r0.x = ((r0.xxxx)*(source[25].xxxx)).x;
    // 80: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 81: mul r0.x, r0.x, cb0[25].y
    r0.x = ((r0.xxxx)*(source[25].yyyy)).x;
    // 82: mul r0.x, r0.x, l(0.999990)
    r0.x = ((r0.xxxx)*(float4(0.999990,0.999990,0.999990,0.999990))).x;
    // 83: movc r0.x, r0.y, l(0), |r0.x|
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.xxxx))).x;
    // 84: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 86: add r0.z, v4.y, l(-0.300000)
    r0.z = ((v4.yyyy)+(float4(-0.300000,-0.300000,-0.300000,-0.300000))).z;
    // 87: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 88: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 89: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 90: mul r0.yz, v2.xxyx, cb0[25].zzwz
    r0.yz = ((v2.xxyx)*(source[25].zzwz)).yz;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 92: mul r0.y, r0.y, cb0[26].x
    r0.y = ((r0.yyyy)*(source[26].xxxx)).y;
    // 93: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 94: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 95: mul r0.z, r0.z, cb0[26].y
    r0.z = ((r0.zzzz)*(source[26].yyyy)).z;
    // 96: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 97: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 98: mul r0.y, r1.x, r0.y
    r0.y = ((r1.xxxx)*(r0.yyyy)).y;
    // 99: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 100: mad r0.z, v2.x, l(2.000000), l(-1.000000)
    r0.z = ((v2.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 101: add r0.z, -|r0.z|, l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 102: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 103: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 104: mul r0.w, r0.w, cb0[26].z
    r0.w = ((r0.wwww)*(source[26].zzzz)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 107: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 108: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 109: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 110: mul r0.yz, v2.xxyx, cb0[27].xxyx
    r0.yz = ((v2.xxyx)*(source[27].xxyx)).yz;
    // 111: mad r1.x, cb0[11].y, cb0[26].w, r0.y
    r1.x = ((source[11].yyyy)*(source[26].wwww)+(r0.yyyy)).x;
    // 112: mad r1.y, cb0[11].y, cb0[27].z, r0.z
    r1.y = ((source[11].yyyy)*(source[27].zzzz)+(r0.zzzz)).y;
    // 113: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t4.yxzw, s6, l(0.000000)
    r0.y = (ArtistNativeSample6((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 114: add r0.y, r0.y, l(0.200000)
    r0.y = ((r0.yyyy)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 115: add r0.z, v3.w, -cb0[27].w
    r0.z = ((v3.wwww)+(-(source[27].wwww))).z;
    // 116: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 117: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 118: mul_sat r0.y, r0.y, cb0[28].x
    r0.y = (saturate((r0.yyyy)*(source[28].xxxx))).y;
    // 119: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 120: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_afterburn_01_03_tr: 0b9775c22cefd74793c28efbfd13d24f; selected map f9fb0de4ec8cd6ee81b1a73642843514f1f79e81cc2e5a492c99f758a8f4a8a5.
float4 ArtistNative4564(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((((g_ArtistSourceMaterialTime.xxxx+float4(0.0, 0.0, 0.0, 0.0))*g_ArtistSourceMaterialParameters[2u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((((g_ArtistSourceMaterialTime.xxxx+float4(0.0, 0.0, 0.0, 0.0))*g_ArtistSourceMaterialParameters[2u].wwww)*float4(0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[8] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(cos((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[11].x = (sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[11].z = (cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].x = (cos((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].y = ((g_ArtistSourceMaterialParameters[1u].zzzz*float4(3.0, 0.0, 0.0, 0.0))).x;
    source[13].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].w = ((g_ArtistSourceMaterialParameters[1u].wwww*float4(20.0, 0.0, 0.0, 0.0))).x;
    source[14].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[14].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[15].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy))).x;
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
    // 1: mul r0.x, v4.w, cb0[11].w
    r0.x = ((v4.wwww)*(source[11].wwww)).x;
    // 2: mad r0.yz, r0.xxxx, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.xxxx)*(v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: mul r1.xyzw, r0.xxxx, l(1.330000, 1.330000, 1.768900, 1.768900)
    r1.xyzw = ((r0.xxxx)*(float4(1.330000,1.330000,1.768900,1.768900))).xyzw;
    // 4: mad r1.xyzw, r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((r1.xyzw)*(v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 5: dp2 r2.x, cb0[4].xyxx, r0.yzyy
    r2.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 6: dp2 r2.y, cb0[5].xyxx, r0.yzyy
    r2.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 7: add r0.xy, r2.xyxx, cb0[6].xyxx
    r0.xy = ((r2.xyxx)+(source[6].xyxx)).xy;
    // 8: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: dp2 r2.x, cb0[7].xyxx, r1.xyxx
    r2.x = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 11: dp2 r2.y, cb0[8].xyxx, r1.xyxx
    r2.y = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 12: add r0.yz, r2.xxyx, cb0[6].xxyx
    r0.yz = ((r2.xxyx)+(source[6].xxyx)).yz;
    // 13: add r0.yz, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, l(0.333300)
    r0.y = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))).y;
    // 16: mad r0.x, r0.x, l(0.333300), r0.y
    r0.x = ((r0.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).x;
    // 17: dp2 r1.x, cb0[9].xyxx, r1.zwzz
    r1.x = (dot((source[9].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 18: dp2 r1.y, cb0[10].xyxx, r1.zwzz
    r1.y = (dot((source[10].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 19: add r0.yz, r1.xxyx, cb0[6].xxyx
    r0.yz = ((r1.xxyx)+(source[6].xxyx)).yz;
    // 20: add r0.yz, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 22: mad r0.x, r0.y, l(0.333300), r0.x
    r0.x = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.xxxx)).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[13].w
    r0.y = ((r0.yyyy)*(source[13].wwww)).y;
    // 27: mov_sat r0.z, v4.z
    r0.z = (saturate(v4.zzzz)).z;
    // 28: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 30: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 31: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 32: movc r0.y, r0.z, l(-0.000000), -r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 33: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 34: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 35: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 36: mad r0.z, -r0.z, l(1.428571), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(1.428571,1.428571,1.428571,1.428571))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 38: add r0.w, r0.z, r0.x
    r0.w = ((r0.zzzz)+(r0.xxxx)).w;
    // 39: mul_sat r0.x, r0.x, cb0[12].y
    r0.x = (saturate((r0.xxxx)*(source[12].yyyy))).x;
    // 40: add r0.w, r0.w, -cb0[12].w
    r0.w = ((r0.wwww)+(-(source[12].wwww))).w;
    // 41: mad r0.y, r0.w, r0.z, r0.y
    r0.y = ((r0.wwww)*(r0.zzzz)+(r0.yyyy)).y;
    // 42: mul_sat r0.y, r0.y, cb0[14].x
    r0.y = (saturate((r0.yyyy)*(source[14].xxxx))).y;
    // 43: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 44: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[14].y
    r0.z = ((r0.zzzz)*(source[14].yyyy)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 48: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 49: source device depth mapped to centimetre view depth; reconstruction at 51.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 51-54: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 55: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 56: add r1.x, -cb0[15].x, l(1.000000)
    r1.x = ((-(source[15].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 58: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 59: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 60: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 61: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 62: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 63: movc o0.w, r0.y, l(0), r0.z
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 64: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 65: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: mul r0.y, r0.y, cb0[12].z
    r0.y = ((r0.yyyy)*(source[12].zzzz)).y;
    // 67: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 68: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 69: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 70: mad r1.xyz, cb0[3].wwww, cb0[3].xyzx, -r0.yzwy
    r1.xyz = ((source[3].wwww)*(source[3].xyzx)+(-(r0.yzwy))).xyz;
    // 71: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 72: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 73: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_afterburn_01_04_tr: 126117e21d2ff5458735ba17961ce55e; selected map b7b8d7bb4427f7c6a43f804016bf00f794df87d1105bae8f5bcfb2fc1876d5ed.
float4 ArtistNative4565(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(cos((((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin((((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = g_ArtistSourceMaterialParameters[6u];
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10] = g_ArtistSourceMaterialParameters[5u];
    source[11].x = (cos((g_ArtistSourceMaterialParameters[2u].wwww*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].w = ((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[12].x = (cos((((g_ArtistSourceMaterialParameters[2u].wwww+g_ArtistSourceMaterialParameters[3u].xxxx)+g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].x = ((g_ArtistSourceMaterialParameters[2u].xxxx*float4(3.0, 0.0, 0.0, 0.0))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[14].z = ((g_ArtistSourceMaterialParameters[2u].yyyy*float4(20.0, 0.0, 0.0, 0.0))).x;
    source[14].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[15].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[15].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy))).x;
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
    // 1: mul r0.x, v4.w, cb0[11].y
    r0.x = ((v4.wwww)*(source[11].yyyy)).x;
    // 2: mad r0.yz, r0.xxxx, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.xxxx)*(v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: mul r1.xyzw, r0.xxxx, l(1.330000, 1.330000, 1.768900, 1.768900)
    r1.xyzw = ((r0.xxxx)*(float4(1.330000,1.330000,1.768900,1.768900))).xyzw;
    // 4: mad r1.xyzw, r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((r1.xyzw)*(v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 5: dp2 r0.x, cb0[3].xyxx, r0.yzyy
    r0.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 6: dp2 r2.x, cb0[2].xyxx, r0.yzyy
    r2.x = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 7: mad r2.y, v4.x, l(0.020000), r0.x
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.xxxx)).y;
    // 8: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: dp2 r0.y, cb0[5].xyxx, r1.xyxx
    r0.y = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 12: mad r3.y, v4.x, l(0.020000), r0.y
    r3.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 13: dp2 r3.x, cb0[4].xyxx, r1.xyxx
    r3.x = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 14: add r0.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.yzyy, t2.xyzw, s1, l(0.000000)
    r3.xyz = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 17: mul r0.y, r0.y, l(0.333300)
    r0.y = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))).y;
    // 18: mad r0.x, r0.x, l(0.333300), r0.y
    r0.x = ((r0.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).x;
    // 19: mul r0.yzw, r3.xxyz, l(0.000000, 0.333300, 0.333300, 0.333300)
    r0.yzw = ((r3.xxyz)*(float4(0.000000,0.333300,0.333300,0.333300))).yzw;
    // 20: mad r0.yzw, r2.xxyz, l(0.000000, 0.333300, 0.333300, 0.333300), r0.yyzw
    r0.yzw = ((r2.xxyz)*(float4(0.000000,0.333300,0.333300,0.333300))+(r0.yyzw)).yzw;
    // 21: dp2 r1.x, cb0[7].xyxx, r1.zwzz
    r1.x = (dot((source[7].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 22: dp2 r2.x, cb0[6].xyxx, r1.zwzz
    r2.x = (dot((source[6].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 23: mad r2.y, v4.x, l(0.020000), r1.x
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r1.xxxx)).y;
    // 24: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: mad r0.x, r1.x, l(0.333300), r0.x
    r0.x = ((r1.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.xxxx)).x;
    // 28: mad r0.yzw, r2.xxyz, l(0.000000, 0.333300, 0.333300, 0.333300), r0.yyzw
    r0.yzw = ((r2.xxyz)*(float4(0.000000,0.333300,0.333300,0.333300))+(r0.yyzw)).yzw;
    // 29: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 30: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 31: mad r0.yzw, cb0[12].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[12].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 32: add r1.x, -r0.x, l(1.000000)
    r1.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: add r1.y, -r1.x, l(1.000000)
    r1.y = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: add r1.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 35: dp2 r1.z, r1.zwzz, r1.zwzz
    r1.z = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).z;
    // 36: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 37: mad r1.z, -r1.z, l(1.428571), l(1.000000)
    r1.z = ((-(r1.zzzz))*(float4(1.428571,1.428571,1.428571,1.428571))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 39: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 40: mul r0.yzw, r0.yyzw, r1.yyyy
    r0.yzw = ((r0.yyzw)*(r1.yyyy)).yzw;
    // 41: mul r0.yzw, r0.yyzw, cb0[12].zzzz
    r0.yzw = ((r0.yyzw)*(source[12].zzzz)).yzw;
    // 42: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 43: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 44: mul r0.yzw, r0.yyzw, cb0[12].wwww
    r0.yzw = ((r0.yyzw)*(source[12].wwww)).yzw;
    // 45: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 46: mul_sat r1.y, r1.x, cb0[13].x
    r1.y = (saturate((r1.xxxx)*(source[13].xxxx))).y;
    // 47: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 48: add r1.x, r1.x, -cb0[13].z
    r1.x = ((r1.xxxx)+(-(source[13].zzzz))).x;
    // 49: log r1.w, r1.y
    r1.w = (log2(r1.yyyy)).w;
    // 50: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: mul r1.w, r1.w, cb0[13].y
    r1.w = ((r1.wwww)*(source[13].yyyy)).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: movc r1.y, r1.y, l(0), r1.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 54: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 55: mad r3.xyz, cb0[10].wwww, cb0[10].xyzx, -r2.xyzx
    r3.xyz = ((source[10].wwww)*(source[10].xyzx)+(-(r2.xyzx))).xyz;
    // 56: mad r2.xyz, r1.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 57: mul r3.xyz, cb0[8].xyzx, cb0[8].wwww
    r3.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 58: mul r3.xyz, r3.xyzx, v4.yyyy
    r3.xyz = ((r3.xyzx)*(v4.yyyy)).xyz;
    // 59: mad r0.yzw, r0.yyzw, r3.xxyz, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)+(r2.xxyz)).yzw;
    // 60: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 61: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 62: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 63: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 64: mul r0.y, r0.y, cb0[14].x
    r0.y = ((r0.yyyy)*(source[14].xxxx)).y;
    // 65: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 66: mul r0.y, r0.y, cb0[14].z
    r0.y = ((r0.yyyy)*(source[14].zzzz)).y;
    // 67: mov_sat r0.z, v4.z
    r0.z = (saturate(v4.zzzz)).z;
    // 68: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 70: movc r0.x, r0.x, l(-0.000000), -r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).x;
    // 71: mad r0.x, r1.x, r1.z, r0.x
    r0.x = ((r1.xxxx)*(r1.zzzz)+(r0.xxxx)).x;
    // 72: mul_sat r0.x, r0.x, cb0[14].w
    r0.x = (saturate((r0.xxxx)*(source[14].wwww))).x;
    // 73: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 75: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 76: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 77: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 78: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 79: source device depth mapped to centimetre view depth; reconstruction at 81.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 81-84: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 85: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 86: add r0.w, -cb0[15].w, l(1.000000)
    r0.w = ((-(source[15].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 87: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 88: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 89: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 90: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 91: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 92: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 93: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_de_dragondecal_tr_01_01: 225313f3418d1644afd364c8fc464a7d; selected map 2d78a54aef03bb8c10552c18795c38564542adbd804d6d0a275741a3c889433a.
float4 ArtistNative4566(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = g_ArtistSourceMaterialParameters[1u];
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[7].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))).x;
    source[7].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0)))).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 5: add r1.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.xy, r1.xyxx, v4.xyxx
    r0.xy = ((r1.xyxx)*(v4.xyxx)).xy;
    // 7: lt r1.xyzw, r0.xyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyzw = (asfloat((uint4)((r0.xyzw)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xyzw;
    // 8: or r0.zw, r1.zzzw, r1.xxxy
    r0.zw = (asfloat(asuint(r1.zzzw) | asuint(r1.xxxy))).zw;
    // 9: or r0.z, r0.w, r0.z
    r0.z = (asfloat(asuint(r0.wwww) | asuint(r0.zzzz))).z;
    // 10: discard_nz r0.z
    if ((asuint(r0.zzzz)).x != 0u) clip(-1.f);
    // 11: mul r0.z, |r0.x|, |r0.x|
    r0.z = ((abs(r0.xxxx))*(abs(r0.xxxx))).z;
    // 12: mul r0.z, r0.z, |r0.x|
    r0.z = ((r0.zzzz)*(abs(r0.xxxx))).z;
    // 13: mad r0.z, -r0.z, l(60.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(60.000000,60.000000,60.000000,60.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 14: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 15: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: movc r0.x, r0.x, l(1.000000), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.zzzz)).x;
    // 17: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 0.300000, 0.300000), cb0[6].xxxy
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,0.300000,0.300000))+(source[6].xxxy)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 19: mul r0.xz, r0.zzwz, r0.xxxx
    r0.xz = ((r0.zzwz)*(r0.xxxx)).xz;
    // 20: add r1.xy, cb0[1].yzyy, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[1].yzyy)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mul r0.xz, r0.xxzx, r1.yyyy
    r0.xz = ((r0.xxzx)*(r1.yyyy)).xz;
    // 22: mad r1.yz, v4.xxyx, l(0.000000, 0.600000, 0.300000, 0.000000), cb0[5].xxyx
    r1.yz = ((v4.xxyx)*(float4(0.000000,0.600000,0.300000,0.000000))+(source[5].xxyx)).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r1.yzyy, t0.zxyw, s0, l(0.000000)
    r1.yz = (ArtistNativeSample0((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 24: mad r1.yz, r1.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 25: mov r2.x, l(0)
    r2.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 26: mov r2.y, cb0[1].x
    r2.y = (source[1].xxxx).y;
    // 27: add r2.zw, r2.xxxy, v4.xxxy
    r2.zw = ((r2.xxxy)+(v4.xxxy)).zw;
    // 28: log r0.w, |r2.w|
    r0.w = (log2(abs(r2.wwww))).w;
    // 29: mul r0.w, r0.w, l(0.450000)
    r0.w = ((r0.wwww)*(float4(0.450000,0.450000,0.450000,0.450000))).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: lt r1.w, |r2.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 33: movc r0.w, r1.w, l(-1.000000), -r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (-(r0.wwww))).w;
    // 34: add r1.w, -r2.z, l(1.000000)
    r1.w = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: mad_sat r0.w, r2.z, r1.w, r0.w
    r0.w = (saturate((r2.zzzz)*(r1.wwww)+(r0.wwww))).w;
    // 36: mul r1.yz, r1.yyzy, r0.wwww
    r1.yz = ((r1.yyzy)*(r0.wwww)).yz;
    // 37: mad r0.xz, r1.xxxx, r1.yyzy, r0.xxzx
    r0.xz = ((r1.xxxx)*(r1.yyzy)+(r0.xxzx)).xz;
    // 38: add r0.xz, r0.xxzx, v4.xxyx
    r0.xz = ((r0.xxzx)+(v4.xxyx)).xz;
    // 39: add r0.xz, r2.xxyx, r0.xxzx
    r0.xz = ((r2.xxyx)+(r0.xxzx)).xz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 41: mul r0.y, r0.x, r0.y
    r0.y = ((r0.xxxx)*(r0.yyyy)).y;
    // 42: mad r0.xzw, r0.xxxx, cb0[4].xxyz, cb0[3].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)+(source[3].xxyz)).xzw;
    // 43: mad o0.xyz, r0.xzwx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xzwx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 44: mul r0.x, r0.y, cb0[7].w
    r0.x = ((r0.yyyy)*(source[7].wwww)).x;
    // 45: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 46: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 47: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 48: add r0.y, -|v4.w|, cb0[0].y
    r0.y = ((-(abs(v4.wwww)))+(source[0].yyyy)).y;
    // 49: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 50: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 51: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 52: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_de_crack_01_05_tr: 3e2151eaa9254c408a93249c1647aaa6; selected map 7dc0be4ed5e9f68156a830cbde31725e4d7d255aa3acbe69e31d5dbb0f960f25.
float4 ArtistNative4544(ARTIST_NATIVE_INPUT input)
{
    float4 source[23]; [unroll] for (uint i=0u; i<23u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[20]=float4(input.skyUpperColor,0.f);
    source[21]=float4(input.skyLowerColor,0.f);
    source[22]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[9u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[6] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].zzzz,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[7] = g_ArtistSourceMaterialParameters[6u];
    source[8] = g_ArtistSourceMaterialParameters[8u];
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[10] = (g_ArtistSourceMaterialTime.xxxx*ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,float4(0.0, 0.0, 0.0, 0.0),1u));
    source[11] = g_ArtistSourceMaterialParameters[7u];
    source[12].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[15].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[16].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[17].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
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
    float4 v7 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v8 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v9 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 12: add r0.x, r0.x, -cb0[12].w
    r0.x = ((r0.xxxx)+(-(source[12].wwww))).x;
    // 13: mul r0.x, r0.x, cb0[13].x
    r0.x = ((r0.xxxx)*(source[13].xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 15: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 16: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 17: mul r0.yz, r0.yyyy, v6.xxyx
    r0.yz = ((r0.yyyy)*(v6.xxyx)).yz;
    // 18: mul r1.xy, r0.yzyy, r0.xxxx
    r1.xy = ((r0.yzyy)*(r0.xxxx)).xy;
    // 19: mad r1.zw, cb0[14].xxxx, v4.xxxy, r1.xxxy
    r1.zw = ((source[14].xxxx)*(v4.xxxy)+(r1.xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t6.xyzw, s4, l(0.000000)
    r2.xyz = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: add r0.w, r2.y, r2.x
    r0.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 22: add r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)+(r0.wwww)).w;
    // 23: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 24: log r1.z, |r0.w|
    r1.z = (log2(abs(r0.wwww))).z;
    // 25: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 26: mul r1.z, r1.z, cb0[14].y
    r1.z = ((r1.zzzz)*(source[14].yyyy)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: mul_sat r1.z, r1.z, cb0[14].z
    r1.z = (saturate((r1.zzzz)*(source[14].zzzz))).z;
    // 29: movc r0.w, r0.w, l(0), r1.z
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).w;
    // 30: mad r2.xyzw, cb0[13].zzww, v4.xyxy, r1.xyxy
    r2.xyzw = ((source[13].zzww)*(v4.xyxy)+(r1.xyxy)).xyzw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.zwzz, t5.xyzw, s3, l(0.000000)
    r3.xyz = (ArtistNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 33: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 34: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 35: mad r1.zw, v4.xxxy, cb0[5].xxxy, r1.xxxy
    r1.zw = ((v4.xxxy)*(source[5].xxxy)+(r1.xxxy)).zw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s1, l(0.000000)
    r1.zw = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 37: mad r3.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 38: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 39: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 42: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 44: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 45: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 46: mad r1.zw, r3.xxxy, cb0[6].xxxy, r2.xxxy
    r1.zw = ((r3.xxxy)*(source[6].xxxy)+(r2.xxxy)).zw;
    // 47: mul r0.w, r3.z, cb0[6].z
    r0.w = ((r3.zzzz)*(source[6].zzzz)).w;
    // 48: max r0.w, r0.w, l(0.150000)
    r0.w = (max(r0.wwww,float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 49: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mul r1.zw, r1.zzzw, cb0[14].wwww
    r1.zw = ((r1.zzzw)*(source[14].wwww)).zw;
    // 51: mad r1.zw, -r0.yyyz, cb0[4].xxxy, r1.zzzw
    r1.zw = ((-(r0.yyyz))*(source[4].xxxy)+(r1.zzzw)).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r1.zwzz, t7.xyzw, s5, l(0.000000)
    r3.xyz = (ArtistNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: dp3 r1.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 54: add r4.xyz, -r3.xyzx, r1.zzzz
    r4.xyz = ((-(r3.xyzx))+(r1.zzzz)).xyz;
    // 55: mad r3.xyz, cb0[15].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[15].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 56: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 57: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 58: mul r3.xyz, r3.xyzx, cb0[15].yyyy
    r3.xyz = ((r3.xyzx)*(source[15].yyyy)).xyz;
    // 59: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 60: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 61: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 62: mul r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = ((r3.xyzx)*(r1.zzzz)).xyz;
    // 63: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 64: add r4.xyz, -r2.xyzx, r1.zzzz
    r4.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 65: mad r4.xyz, cb0[15].zzzz, r4.xyzx, r2.xyzx
    r4.xyz = ((source[15].zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 66: mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.200000, 0.200000), v4.xxxy
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,0.200000,0.200000))+(v4.xxxy)).zw;
    // 67: add r1.zw, r1.zzzw, cb0[10].xxxy
    r1.zw = ((r1.zzzw)+(source[10].xxxy)).zw;
    // 68: mad r1.zw, r0.xxxx, r0.yyyz, r1.zzzw
    r1.zw = ((r0.xxxx)*(r0.yyyz)+(r1.zzzw)).zw;
    // 69: mad_sat r0.xy, r0.xxxx, r0.yzyy, v4.xyxx
    r0.xy = (saturate((r0.xxxx)*(r0.yzyy)+(v4.xyxx))).xy;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s7, l(0.000000)
    r0.x = (ArtistNativeSample7((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 71: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t4.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 73: mul r2.xyz, cb0[8].xyzx, cb0[8].wwww
    r2.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 74: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 75: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 76: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 77: mad r0.zw, r0.zzzw, cb0[9].xxxy, r1.xxxy
    r0.zw = ((r0.zzzw)*(source[9].xxxy)+(r1.xxxy)).zw;
    // 78: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 79: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t8.xzyw, s6, l(0.000000)
    r0.z = (ArtistNativeSample6((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 80: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 82: mul r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 83: mul r2.xyz, cb0[7].xyzx, cb0[7].wwww
    r2.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 84: mad r1.xyz, r3.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 85: mul r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 86: add r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)+(r0.zzzz)).w;
    // 87: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 88: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 89: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 90: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 91: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 92: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 93: mul r2.xyz, cb0[11].xyzx, cb0[11].wwww
    r2.xyz = ((source[11].xyzx)*(source[11].wwww)).xyz;
    // 94: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 95: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 96: movc r0.yzw, r0.zzzz, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 97: mad r0.yzw, r1.xxyz, l(0.000000, 0.150000, 0.150000, 0.150000), r0.yyzw
    r0.yzw = ((r1.xxyz)*(float4(0.000000,0.150000,0.150000,0.150000))+(r0.yyzw)).yzw;
    // 98: mul r1.xyz, r1.xyzx, cb2[3].wwww
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)).xyz;
    // 99: mad r1.xyz, r1.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 100: add r0.yzw, r0.yyzw, cb0[3].xxyz
    r0.yzw = ((r0.yyzw)+(source[3].xxyz)).yzw;
    // 101: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 102: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 103: mul r1.w, r1.w, v7.z
    r1.w = ((r1.wwww)*(v7.zzzz)).w;
    // 104: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 105: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 106: mul r2.yzw, r2.yyyy, cb0[21].xxyz
    r2.yzw = ((r2.yyyy)*(source[21].xxyz)).yzw;
    // 107: mad r2.xyz, r2.xxxx, cb0[20].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[20].xyzx)+(r2.yzwy)).xyz;
    // 108: mul r2.xyz, r2.xyzx, cb0[22].wwww
    r2.xyz = ((r2.xyzx)*(source[22].wwww)).xyz;
    // 109: mad r0.yzw, r2.xxyz, r1.xxyz, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 110: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 112: mad r0.yzw, r1.xxyz, cb0[22].xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(source[22].xxyz)+(r0.yyzw)).yzw;
    // 114: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 115: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 116: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 117: mul r0.y, r0.y, cb0[17].y
    r0.y = ((r0.yyyy)*(source[17].yyyy)).y;
    // 118: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 119: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 120: mul r0.yz, v4.xxyx, cb0[17].zzzz
    r0.yz = ((v4.xxyx)*(source[17].zzzz)).yz;
    // 121: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s8, l(0.000000)
    r0.y = (ArtistNativeSample8((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 122: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 123: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 124: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 125: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 126: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 127: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 128: add r0.y, -|v4.w|, cb0[0].y
    r0.y = ((-(abs(v4.wwww)))+(source[0].yyyy)).y;
    // 129: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 130: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 131: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 132: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_ring_11_08_ad: f2591e8eac94ce488574ad6dd2c88561; selected map 9ebdc7594c0b87978105287d88e81fc3cdfd2ccc23e3423a329a5fc7ae445829.
float4 ArtistNative4545(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].wwww))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 11: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 12: mul_sat r0.y, r0.y, cb0[3].x
    r0.y = (saturate((r0.yyyy)*(source[3].xxxx))).y;
    // 13: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 14: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, v4.x, cb0[3].y
    r0.z = ((v4.xxxx)*(source[3].yyyy)).z;
    // 17: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 18: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 19: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 20: mov_sat r0.y, v4.z
    r0.y = (saturate(v4.zzzz)).y;
    // 21: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 22: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 23: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 24: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4545Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_m_pa_spritewave_01_72_tr: 40698a3240b4464ca6513ff6ae962795; selected map 946eb1f9af03b5c82386453a9e12dfbfdcf0a94a4fed055064bb185570312ef5.
float4 ArtistNative4546(ARTIST_NATIVE_INPUT input)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[13u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].zzzz,g_ArtistSourceMaterialParameters[5u].wwww,1u);
    source[7] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = g_ArtistSourceMaterialParameters[11u];
    source[12].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[12].y = (cos(((g_ArtistSourceMaterialParameters[6u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[14].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[17].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[17].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[18].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[18].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[19].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[19].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[20].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[20].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[20].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[20].w = ((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[21].x = (sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[21].z = (cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[22].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[22].y = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[22].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[22].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[23].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[23].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[23].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[23].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[24].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[24].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[24].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[15].zwzz
    r0.xy = ((v4.xyxx)*(source[15].zwzz)).xy;
    // 2: mad r1.x, cb0[12].w, cb0[15].y, r0.x
    r1.x = ((source[12].wwww)*(source[15].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[12].w, cb0[16].x, r0.y
    r1.y = ((source[12].wwww)*(source[16].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mad r0.xy, r0.xxxx, cb0[16].yyyy, v4.xyxx
    r0.xy = ((r0.xxxx)*(source[16].yyyy)+(v4.xyxx)).xy;
    // 6: mul r0.z, cb0[12].w, cb0[14].z
    r0.z = ((source[12].wwww)*(source[14].zzzz)).z;
    // 7: mad r1.x, cb0[14].w, r0.x, r0.z
    r1.x = ((source[14].wwww)*(r0.xxxx)+(r0.zzzz)).x;
    // 8: mul r0.x, cb0[12].w, cb0[16].z
    r0.x = ((source[12].wwww)*(source[16].zzzz)).x;
    // 9: mad r1.y, cb0[15].x, r0.y, r0.x
    r1.y = ((source[15].xxxx)*(r0.yyyy)+(r0.xxxx)).y;
    // 10: mul r0.x, cb0[3].w, cb0[16].w
    r0.x = ((source[3].wwww)*(source[16].wwww)).x;
    // 11: mul r0.y, cb0[3].w, cb0[17].x
    r0.y = ((source[3].wwww)*(source[17].xxxx)).y;
    // 12: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: add r0.y, cb0[3].z, cb0[17].y
    r0.y = ((source[3].zzzz)+(source[17].yyyy)).y;
    // 15: mad r0.xy, r0.xxxx, r0.yyyy, cb0[6].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[6].xyxx)).xy;
    // 16: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 17: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 18: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 19: mad r1.xy, cb0[13].zwzz, cb0[3].xxxx, r1.xyxx
    r1.xy = ((source[13].zwzz)*(source[3].xxxx)+(r1.xyxx)).xy;
    // 20: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r1.xy, r1.xyxx, cb0[13].xyxx
    r1.xy = ((r1.xyxx)*(source[13].xyxx)).xy;
    // 22: mad r2.x, cb0[12].w, cb0[12].z, r1.x
    r2.x = ((source[12].wwww)*(source[12].zzzz)+(r1.xxxx)).x;
    // 23: mad r2.y, cb0[12].w, cb0[14].y, r1.y
    r2.y = ((source[12].wwww)*(source[14].yyyy)+(r1.yyyy)).y;
    // 24: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 25: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 26: dp2 r1.x, cb0[7].xyxx, r0.xyxx
    r1.x = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 27: dp2 r1.y, cb0[8].xyxx, r0.xyxx
    r1.y = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 28: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 29: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(-1.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 30: mul r1.xy, v4.xyxx, cb0[18].yzyy
    r1.xy = ((v4.xyxx)*(source[18].yzyy)).xy;
    // 31: mad r1.xy, cb0[12].wwww, cb0[18].xwxx, r1.xyxx
    r1.xy = ((source[12].wwww)*(source[18].xwxx)+(r1.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t5.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 33: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 34: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 35: mul r1.x, r1.x, cb0[19].x
    r1.x = ((r1.xxxx)*(source[19].xxxx)).x;
    // 36: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 37: mul r1.x, r1.x, cb0[19].y
    r1.x = ((r1.xxxx)*(source[19].yyyy)).x;
    // 38: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 40: mad r0.y, r0.y, cb0[19].z, r1.x
    r0.y = ((r0.yyyy)*(source[19].zzzz)+(r1.xxxx)).y;
    // 41: dp2 r1.x, cb0[9].xyxx, r0.zwzz
    r1.x = (dot((source[9].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 42: dp2 r1.y, cb0[10].xyxx, r0.zwzz
    r1.y = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 43: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 44: mul r1.xy, v4.xyxx, cb0[22].xyxx
    r1.xy = ((v4.xyxx)*(source[22].xyxx)).xy;
    // 45: mad r2.x, cb0[12].w, cb0[21].w, r1.x
    r2.x = ((source[12].wwww)*(source[21].wwww)+(r1.xxxx)).x;
    // 46: mad r2.y, cb0[12].w, cb0[22].z, r1.y
    r2.y = ((source[12].wwww)*(source[22].zzzz)+(r1.yyyy)).y;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t3.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: mad r0.zw, r1.xxxx, cb0[22].wwww, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[22].wwww)+(r0.zzzw)).zw;
    // 49: mul r0.zw, r0.zzzw, cb0[20].xxxy
    r0.zw = ((r0.zzzw)*(source[20].xxxy)).zw;
    // 50: mad r1.x, cb0[12].w, cb0[19].w, r0.z
    r1.x = ((source[12].wwww)*(source[19].wwww)+(r0.zzzz)).x;
    // 51: mad r1.y, cb0[12].w, cb0[23].x, r0.w
    r1.y = ((source[12].wwww)*(source[23].xxxx)+(r0.wwww)).y;
    // 52: add r0.zw, r1.xxxy, cb0[23].yyyz
    r0.zw = ((r1.xxxy)+(source[23].yyyz)).zw;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t4.yzxw, s5, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 54: add r0.z, r0.z, l(0.100000)
    r0.z = ((r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 55: add r0.w, cb0[3].y, l(-1.000000)
    r0.w = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 56: add_sat r0.z, -r0.w, r0.z
    r0.z = (saturate((-(r0.wwww))+(r0.zzzz))).z;
    // 57: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 58: mul r0.w, r0.w, cb0[24].x
    r0.w = ((r0.wwww)*(source[24].xxxx)).w;
    // 59: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 60: mul_sat r0.w, r0.w, cb0[23].w
    r0.w = (saturate((r0.wwww)*(source[23].wwww))).w;
    // 61: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 62: mul r0.z, r0.z, cb0[23].w
    r0.z = ((r0.zzzz)*(source[23].wwww)).z;
    // 63: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 64: mov_sat r1.x, r0.z
    r1.x = (saturate(r0.zzzz)).x;
    // 65: mul r0.z, r0.z, r0.x
    r0.z = ((r0.zzzz)*(r0.xxxx)).z;
    // 66: mul r0.z, r0.z, cb0[24].z
    r0.z = ((r0.zzzz)*(source[24].zzzz)).z;
    // 67: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 68: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 69: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 70: mul r1.xyz, r0.wwww, cb0[11].xyzx
    r1.xyz = ((r0.wwww)*(source[11].xyzx)).xyz;
    // 71: mad r0.xyw, r0.xxxx, r1.xyxz, r0.yyyy
    r0.xyw = ((r0.xxxx)*(r1.xyxz)+(r0.yyyy)).xyw;
    // 72: mad r0.xyw, r0.xyxw, cb0[1].xyxz, cb0[2].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)+(source[2].xyxz)).xyw;
    // 73: mad o0.xyz, r0.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 74: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 75: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 76: mul r0.x, r0.x, v6.z
    r0.x = ((r0.xxxx)*(v6.zzzz)).x;
    // 77: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 78: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 79: mul r0.y, r0.y, cb0[24].y
    r0.y = ((r0.yyyy)*(source[24].yyyy)).y;
    // 80: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 81: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 82: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 83: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_pa_ringmaster_01_18_ad: d4aba8890548634891660e68e46864cb; selected map 249944e8c32a34a0765a356803afc238df00a0ca5a711139a571b284bbfae3f7.
float4 ArtistNative4547(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[8u];
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[2u].xxxx)).x;
    source[8].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[2u].xxxx))).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].yyyy)).x;
    source[8].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[9].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[10].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[10].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    r2.xy = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.yzw = (ArtistNativeSample2((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(-1.000000)
    r1.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 50: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 51: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: mad r0.yzw, -r1.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r1.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 53: mad r0.yzw, cb0[7].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[7].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
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
    // 90: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 91: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 92: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 93: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 94: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 95: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4547Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_e_pa_fd_04_1_tr: a40cba34e2e4df4a88cb86692b690da3; selected map 075338ae49df4f3c7cf9d0e65f2455cc6070f44a45cb22739d8a2b49d724cf80.
float4 ArtistNative4548(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[4].w, l(1.000000)
    r0.y = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r1.xyzw = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4548Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_x_pa_turbulence_03_01_dt_tr: fcfce1b733b91947965e8ecf70840952; selected map 20ea86277028a24f58aa72c453cc3c152f90f35ce82df9274ebe98f91c16faa0.
float4 ArtistNative4549(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[3].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r0.zw, -r0.xxxy, v2.xxxy
    r0.zw = ((-(r0.xxxy))+(v2.xxxy)).zw;
    // 3: mad r0.xy, v4.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((v4.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 4: mul r0.zw, r0.xxxy, cb0[2].zzzz
    r0.zw = ((r0.xxxy)*(source[2].zzzz)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.wxyz, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 6: mul_sat r0.x, r0.x, cb0[2].x
    r0.x = (saturate((r0.xxxx)*(source[2].xxxx))).x;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t3.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.y = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_pa_spritetransition_01_4_tr: 28af9ecbaa26d040820b16dea7d6fbdd; selected map 7b903d701d364ecd68b56da9eb8569e4558885ac9e5c7d5cfbb0fd6bf5a89843.
float4 ArtistNative4550(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[3].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].z = ((float4(-0.100000001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].y = ((float4(-1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].wwww)).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: add r0.x, v4.x, cb0[5].z
    r0.x = ((v4.xxxx)+(source[5].zzzz)).x;
    // 2: add r0.y, cb0[2].x, l(-1.000000)
    r0.y = ((source[2].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 3: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 4: mad r0.yz, cb0[2].xxxx, v2.xxyx, -r0.yyyy
    r0.yz = ((source[2].xxxx)*(v2.xxyx)+(-(r0.yyyy))).yz;
    // 5: add r1.xy, r0.yzyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.yzyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: mad r0.w, r0.w, l(2.000000), l(1.000000)
    r0.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: add r0.x, -r0.x, r0.w
    r0.x = ((-(r0.xxxx))+(r0.wwww)).x;
    // 10: add_sat r0.x, r0.x, r0.x
    r0.x = (saturate((r0.xxxx)+(r0.xxxx))).x;
    // 11: mul r1.xy, r0.yzyy, cb0[4].wwww
    r1.xy = ((r0.yzyy)*(source[4].wwww)).xy;
    // 12: mad r2.x, cb0[2].z, cb0[4].z, r1.x
    r2.x = ((source[2].zzzz)*(source[4].zzzz)+(r1.xxxx)).x;
    // 13: mad r2.y, cb0[2].z, cb0[5].x, r1.y
    r2.y = ((source[2].zzzz)*(source[5].xxxx)+(r1.yyyy)).y;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: mad r2.xyz, r0.xxxx, r1.xyzx, r0.xxxx
    r2.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.xxxx)).xyz;
    // 16: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: mul r3.xyz, r2.xyzx, r2.xyzx
    r3.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 18: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 19: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 20: min r2.xyz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 21: add r0.x, r0.w, -v4.x
    r0.x = ((r0.wwww)+(-(v4.xxxx))).x;
    // 22: add_sat r0.x, r0.x, r0.x
    r0.x = (saturate((r0.xxxx)+(r0.xxxx))).x;
    // 23: mad r3.xyz, r0.xxxx, r1.xyzx, r0.xxxx
    r3.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.xxxx)).xyz;
    // 24: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: mul r4.xyz, r3.xyzx, r3.xyzx
    r4.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 26: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 27: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 28: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 29: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 30: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 31: mul r4.xyz, r2.xyzx, r2.xyzx
    r4.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 32: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 33: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 34: mul r4.xy, r0.yzyy, cb0[2].wwww
    r4.xy = ((r0.yzyy)*(source[2].wwww)).xy;
    // 35: mad r5.x, cb0[2].z, cb0[2].y, r4.x
    r5.x = ((source[2].zzzz)*(source[2].yyyy)+(r4.xxxx)).x;
    // 36: mad r5.y, cb0[2].z, cb0[3].x, r4.y
    r5.y = ((source[2].zzzz)*(source[3].xxxx)+(r4.yyyy)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r5.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = (ArtistNativeSample0((r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 38: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 39: mad r0.xy, cb0[3].yyyy, r4.xyxx, r0.yzyy
    r0.xy = ((source[3].yyyy)*(r4.xyxx)+(r0.yzyy)).xy;
    // 40: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 41: mad r0.xy, r0.xyxx, cb0[3].zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(source[3].zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: dp3 r1.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: add r4.xyz, -r0.xyzx, r1.wwww
    r4.xyz = ((-(r0.xyzx))+(r1.wwww)).xyz;
    // 45: mad r0.xyz, cb0[4].xxxx, r4.xyzx, r0.xyzx
    r0.xyz = ((source[4].xxxx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 46: mul r0.xyz, r0.xyzx, cb0[4].yyyy
    r0.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 47: mad r0.xyz, r3.xyzx, -r0.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(-(r0.xyzx))+(r0.xyzx)).xyz;
    // 48: mul r2.xyz, r2.xyzx, r0.xyzx
    r2.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 49: mad r0.xyz, cb0[5].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[5].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 50: add r1.w, v4.x, cb0[6].y
    r1.w = ((v4.xxxx)+(source[6].yyyy)).w;
    // 51: add r0.w, r0.w, -r1.w
    r0.w = ((r0.wwww)+(-(r1.wwww))).w;
    // 52: add_sat r0.w, r0.w, r0.w
    r0.w = (saturate((r0.wwww)+(r0.wwww))).w;
    // 53: mad r1.xyz, r0.wwww, r1.xyzx, r0.wwww
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r0.wwww)).xyz;
    // 54: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: mul r2.xyz, r1.xyzx, r1.xyzx
    r2.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 57: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 58: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 59: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 60: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 62: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 63: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 64: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 65: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 66: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 67: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 68: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 69: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4550Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_sqc_01_11_dt_tr: 0d047bda09fdab409b233d70ee0f2dd8; selected map fdf5be0aacb0bd0229e4e2c301d24d4b9bee7d3a6b6d174dfc1579d638d2bf30.
float4 ArtistNative4551(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[4].y, l(1.000000)
    r0.y = ((-(source[4].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 14: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: sample_l_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s1, cb0[3].x
    r1.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (source[3].xxxx).x, true).xyzw).xyzw;
    // 16: add_sat r0.y, -r0.y, r1.w
    r0.y = (saturate((-(r0.yyyy))+(r1.wwww))).y;
    // 17: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 18: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 19: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 20: mul r0.x, v2.y, cb0[2].y
    r0.x = ((v2.yyyy)*(source[2].yyyy)).x;
    // 21: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 22: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mad r0.xyz, cb0[3].wwww, r0.xxxx, r1.xyzx
    r0.xyz = ((source[3].wwww)*(r0.xxxx)+(r1.xyzx)).xyz;
    // 24: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 25: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 26: mad r0.xyz, cb0[4].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 27: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 28: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_ph_06_msk: 3b5374b1c393a046a6cef2bbd15fe775; selected map 9dd4d9856e72e97e347b28ec3de376c62617d95ed3a39e2a93b3fc71a306ac13.
float4 ArtistNative4552(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].zzzz,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[4] = g_ArtistSourceMaterialParameters[4u];
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[7] = g_ArtistSourceMaterialParameters[5u];
    source[8].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
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
    // 15: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 16: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 17: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 18: mul r0.x, r0.x, v5.z
    r0.x = ((r0.xxxx)*(v5.zzzz)).x;
    // 19: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 20: mad r0.yz, r0.yyzy, cb0[3].xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)*(source[3].xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 22: mul r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)*(abs(r0.xxxx))).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.y, r0.y, cb0[10].x
    r0.y = ((r0.yyyy)*(source[10].xxxx)).y;
    // 26: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 27: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 28: mul r0.yz, v4.xxyx, cb0[6].xxyx
    r0.yz = ((v4.xxyx)*(source[6].xxyx)).yz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s2, l(0.000000)
    r0.yzw = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 30: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 31: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 32: mad r0.yzw, cb0[10].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[10].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 33: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 34: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 35: mul r0.yzw, r0.yyzw, cb0[11].xxxx
    r0.yzw = ((r0.yyzw)*(source[11].xxxx)).yzw;
    // 36: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 37: mul r1.xyz, cb0[7].xyzx, cb0[7].wwww
    r1.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 38: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 39: mul r1.xy, v4.xyxx, cb0[8].zwzz
    r1.xy = ((v4.xyxx)*(source[8].zwzz)).xy;
    // 40: mad r2.x, cb0[8].y, cb0[8].x, r1.x
    r2.x = ((source[8].yyyy)*(source[8].xxxx)+(r1.xxxx)).x;
    // 41: mad r2.y, cb0[8].y, cb0[9].x, r1.y
    r2.y = ((source[8].yyyy)*(source[9].xxxx)+(r1.yyyy)).y;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 43: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 44: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 45: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 47: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 48: add r1.z, r1.w, l(0.000010)
    r1.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 49: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 50: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 51: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 52: mad r2.x, v1.z, r1.w, l(1.000000)
    r2.x = ((v1.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mul r2.yzw, r1.wwww, v1.xxyz
    r2.yzw = ((r1.wwww)*(v1.xxyz)).yzw;
    // 54: mul_sat r1.w, r2.x, l(0.500000)
    r1.w = (saturate((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 55: mad r1.w, r1.w, l(0.950000), l(0.050000)
    r1.w = ((r1.wwww)*(float4(0.950000,0.950000,0.950000,0.950000))+(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 56: dp3 r1.w, r1.xyzx, r1.wwww
    r1.w = (dot((r1.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 57: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 58: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 59: mul r2.x, r2.x, cb0[11].y
    r2.x = ((r2.xxxx)*(source[11].yyyy)).x;
    // 60: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 61: mul r2.x, r2.x, cb0[11].z
    r2.x = ((r2.xxxx)*(source[11].zzzz)).x;
    // 62: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 63: mul r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = ((r0.yyzw)*(r1.wwww)).yzw;
    // 64: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 65: mul r3.xyz, r3.xyzx, cb0[5].xxxx
    r3.xyz = ((r3.xyzx)*(source[5].xxxx)).xyz;
    // 66: mad r0.xyz, r0.xxxx, r3.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 67: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 68: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif
