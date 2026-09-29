// Original Kouku material programs 5312..5375; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_de_master_01_18_tr: ac639f46d134fb4d8c0b10ae1cc97fed; selected map d51cc7cfa2543d5518acc3767f015f1aea71fb23326c54306750dce4dd9c7f55.
float4 ArtistNative5312(ARTIST_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[12]=float4(input.skyUpperColor,0.f);
    source[13]=float4(input.skyLowerColor,0.f);
    source[14]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = g_ArtistSourceMaterialParameters[4u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[6] = (g_ArtistSourceMaterialTime.xxxx*ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,float4(0.0, 0.0, 0.0, 0.0),1u));
    source[7] = g_ArtistSourceMaterialParameters[3u];
    source[8].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 12: add r0.x, r0.x, -cb0[8].y
    r0.x = ((r0.xxxx)+(-(source[8].yyyy))).x;
    // 13: mul r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)*(source[8].zzzz)).x;
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
    // 19: mad r1.zw, cb0[8].xxxx, v4.xxxy, r1.xxxy
    r1.zw = ((source[8].xxxx)*(v4.xxxy)+(r1.xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t2.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.200000, 0.200000), v4.xxxy
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,0.200000,0.200000))+(v4.xxxy)).zw;
    // 22: add r1.zw, r1.zzzw, cb0[6].xxxy
    r1.zw = ((r1.zzzw)+(source[6].xxxy)).zw;
    // 23: mad r1.zw, r0.xxxx, r0.yyyz, r1.zzzw
    r1.zw = ((r0.xxxx)*(r0.yyyz)+(r1.zzzw)).zw;
    // 24: mad_sat r0.xy, r0.xxxx, r0.yzyy, v4.xyxx
    r0.xy = (saturate((r0.xxxx)*(r0.yzyy)+(v4.xyxx))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 26: mul r0.x, r0.x, cb0[9].y
    r0.x = ((r0.xxxx)*(source[9].yyyy)).x;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t2.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 28: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 29: mad r0.zw, r0.zzzw, cb0[5].xxxy, r1.xxxy
    r0.zw = ((r0.zzzw)*(source[5].xxxy)+(r1.xxxy)).zw;
    // 30: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.xzyw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 32: add r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)+(r0.zzzz)).w;
    // 33: mul r0.w, r0.w, r0.z
    r0.w = ((r0.wwww)*(r0.zzzz)).w;
    // 34: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 35: mul r0.yz, r0.yyzy, r0.wwzw
    r0.yz = ((r0.yyzy)*(r0.wwzw)).yz;
    // 36: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 37: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 38: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 39: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 40: mul r1.xyz, cb0[7].xyzx, cb0[7].wwww
    r1.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 41: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 42: mul r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 43: movc r1.xyz, r0.yyyy, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 44: dp3 r0.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 45: add r3.xyz, -r2.xyzx, r0.yyyy
    r3.xyz = ((-(r2.xyzx))+(r0.yyyy)).xyz;
    // 46: mad r2.xyz, cb0[8].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[8].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 47: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 48: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 49: mul r0.yzw, r0.zzzz, r2.xxyz
    r0.yzw = ((r0.zzzz)*(r2.xxyz)).yzw;
    // 50: mul r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)).yzw;
    // 51: mad r1.xyz, r0.yzwy, l(0.150000, 0.150000, 0.150000, 0.000000), r1.xyzx
    r1.xyz = ((r0.yzwy)*(float4(0.150000,0.150000,0.150000,0.000000))+(r1.xyzx)).xyz;
    // 52: mul r0.yzw, r0.yyzw, cb2[3].wwww
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)).yzw;
    // 53: mad r0.yzw, r0.yyzw, l(0.000000, 0.850000, 0.850000, 0.850000), cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.850000,0.850000,0.850000))+(passValues[3].xxyz)).yzw;
    // 54: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 55: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 56: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 57: mul r1.w, r1.w, v7.z
    r1.w = ((r1.wwww)*(v7.zzzz)).w;
    // 58: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 59: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 60: mul r2.yzw, r2.yyyy, cb0[13].xxyz
    r2.yzw = ((r2.yyyy)*(source[13].xxyz)).yzw;
    // 61: mad r2.xyz, r2.xxxx, cb0[12].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[12].xyzx)+(r2.yzwy)).xyz;
    // 62: mul r2.xyz, r2.xyzx, cb0[14].wwww
    r2.xyz = ((r2.xyzx)*(source[14].wwww)).xyz;
    // 63: mad r1.xyz, r2.xyzx, r0.yzwy, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 64: mul r2.xyz, r0.yzwy, r2.xyzx
    r2.xyz = ((r0.yzwy)*(r2.xyzx)).xyz;
    // 66: mad r1.xyz, r0.yzwy, cb0[14].xyzx, r1.xyzx
    r1.xyz = ((r0.yzwy)*(source[14].xyzx)+(r1.xyzx)).xyz;
    // 68: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 69: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 70: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 72: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 73: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 74: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 75: add r0.z, -|v4.w|, cb0[0].y
    r0.z = ((-(abs(v4.wwww)))+(source[0].yyyy)).z;
    // 76: mul r0.z, r0.z, l(5.000000)
    r0.z = ((r0.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 77: div_sat r0.z, r0.z, cb0[0].y
    r0.z = (saturate((r0.zzzz)/(source[0].yyyy))).z;
    // 78: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 79: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 80: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_i_pa_backglow_cl_02_tr: 29e0d672dc69374e9b04f75f48ac3e5b; selected map b9047631b19c73caa8be78b0b3b2b300018bb731ac9e3d7f0a46ac44cf3e6724.
float4 ArtistNative5313(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[2].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_dist_05_ad: 0fc154ee4cdbcc408e07e6b3989115cd; selected map 51630230004172d119766ea88ac7bbd7786f285ef0fc919f13cb27502021606a.
float4 ArtistNative5314(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative5314Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_aura_02_tr: 845a7f642142e64c9a42769724b7f8e9; selected map 0db7bf127e3cc8866e3815e0cae2c9a6add9fbe4c31bc3594001134936310ccb.
float4 ArtistNative5315(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.zwzz, t0.wxyz, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.xwyz, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 3: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 4: mad r0.x, v0.w, r0.x, r0.y
    r0.x = ((v0.wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 5: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 6: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 7: mul r0.xy, v2.xyxx, l(8.000000, 4.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(8.000000,4.000000,0.000000,0.000000))).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: mul r1.xyz, v3.xyzx, l(0.359551, 0.330854, 0.274713, 0.000000)
    r1.xyz = ((v3.xyzx)*(float4(0.359551,0.330854,0.274713,0.000000))).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 11: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_01_02_ad: f223e0c7e9a78643ab33afec0a30fd52; selected map daf9dc99efc3b51b9abd1280ed355304a129abffa6febba2650d6878216675d4.
float4 ArtistNative5316(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 13: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 14: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 15: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 16: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 17: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_fluid_01_1_tr: 28af9ecbaa26d040820b16dea7d6fbdd; selected map 7b903d701d364ecd68b56da9eb8569e4558885ac9e5c7d5cfbb0fd6bf5a89843.
float4 ArtistNative5317(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative5317Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_a_me_panning_01_ts_ad: 905554360efecf4e81927d2bf03deaa7; selected map 3745764fc4800dd265e1171186d40052b1d9f01814668929cf1c9e28149ae648.
float4 ArtistNative5318(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
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
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, cb0[1].w
    r0.y = ((r0.yyyy)*(source[1].wwww)).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v4.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_cd_01_2_tr: 1dee627450623d4699684df94a092879; selected map fd09d853c3414943101dab6dece30cab598b55cc307a3d1aa0d399e29220d944.
float4 ArtistNative5319(ARTIST_NATIVE_INPUT input)
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5319Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_a_pa_firework_01_ad: d5c750d0a3de034088d0bf418e743b55; selected map 4443ea3769501b48648d2d5700bf26a7cb105482d82e14ed609f4ec35912c887.
float4 ArtistNative5320(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = g_ArtistSourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 3: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 4: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 5: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 6: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 7: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 8: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 10: mul r0.y, r1.w, l(3.000000)
    r0.y = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 11: mad r1.xyz, cb0[2].xyzx, r1.xyzx, r1.xxxx
    r1.xyz = ((source[2].xyzx)*(r1.xyzx)+(r1.xxxx)).xyz;
    // 12: mad r1.xyz, v3.xyzx, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((v3.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 13: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 14: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 15: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 16: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 17: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_fire_18_03_dt_tr: 17bc89b1218b4b41a6b98675ed3797d9; selected map 56ddb00a6f2a6800a6d3bf72f84fe004f00ddd7306aad44deb915bad18f416d1.
float4 ArtistNative5321(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = g_ArtistSourceMaterialParameters[3u];
    source[5] = g_ArtistSourceMaterialParameters[4u];
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: mov_sat r0.y, v4.z
    r0.y = (saturate(v4.zzzz)).y;
    // 14: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, v2.zwzz, t0.xwyz, s2, l(0.000000)
    r0.zw = (ArtistNativeSample1((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).zw;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v2.xyxx, t0.yzxw, s2, l(0.000000)
    r1.xy = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).xy;
    // 17: add r0.zw, r0.wwwz, -r1.yyyx
    r0.zw = ((r0.wwwz)+(-(r1.yyyx))).zw;
    // 18: mad r0.zw, v0.wwww, r0.zzzw, r1.yyyx
    r0.zw = ((v0.wwww)*(r0.zzzw)+(r1.yyyx)).zw;
    // 19: add_sat r0.y, -r0.y, r0.z
    r0.y = (saturate((-(r0.yyyy))+(r0.zzzz))).y;
    // 20: mul r0.z, r0.w, v4.w
    r0.z = ((r0.wwww)*(v4.wwww)).z;
    // 21: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 24: mul r0.xyw, cb0[5].xyxz, cb0[5].wwww
    r0.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 25: mul r0.xyz, r0.xywx, r0.zzzz
    r0.xyz = ((r0.xywx)*(r0.zzzz)).xyz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v2.zwzz, t2.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 28: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 29: mad r1.xyz, v0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((v0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 30: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 31: mad r0.xyz, r1.zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 32: mul r1.xy, r1.xyxx, v4.xyxx
    r1.xy = ((r1.xyxx)*(v4.xyxx)).xy;
    // 33: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 34: mul r1.yzw, r1.yyyy, r2.xxyz
    r1.yzw = ((r1.yyyy)*(r2.xxyz)).yzw;
    // 35: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 36: mad r1.xyz, r1.xxxx, r2.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 37: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 38: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[6].yyyy
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[6].yyyy)).xyz;
    // 39: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_fire_18_01_tr: b907311020f1bc488c0712cb83550f86; selected map 2ec3e916b008bf0ff43ede033e99db3867fc1484193381d5b664187d7234b6db.
float4 ArtistNative5322(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = g_ArtistSourceMaterialParameters[3u];
    source[5] = g_ArtistSourceMaterialParameters[4u];
    source[6].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].y = ((g_ArtistSourceMaterialParameters[0u].xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.zwzz, t1.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v2.xyxx, t1.xyzw, s0, l(0.000000)
    r1.xyz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 3: add r0.xyz, r0.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)+(-(r1.xyzx))).xyz;
    // 4: mad r0.xyz, v0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((v0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 5: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
    // 6: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 7: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 8: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 9: mad r0.xyw, r0.xxxx, r2.xyxz, r1.xyxz
    r0.xyw = ((r0.xxxx)*(r2.xyxz)+(r1.xyxz)).xyw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v2.zwzz, t0.yzxw, s1, l(0.000000)
    r1.xy = (ArtistNativeSample1((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v2.xyxx, t0.xwyz, s1, l(0.000000)
    r1.zw = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).zw;
    // 12: add r1.xy, -r1.wzww, r1.yxyy
    r1.xy = ((-(r1.wzww))+(r1.yxyy)).xy;
    // 13: mad r1.xy, v0.wwww, r1.xyxx, r1.wzww
    r1.xy = ((v0.wwww)*(r1.xyxx)+(r1.wzww)).xy;
    // 14: mul r1.y, r1.y, v4.w
    r1.y = ((r1.yyyy)*(v4.wwww)).y;
    // 15: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 16: mul r1.yzw, r1.yyyy, r2.xxyz
    r1.yzw = ((r1.yyyy)*(r2.xxyz)).yzw;
    // 17: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 18: mad r1.yzw, r0.zzzz, r2.xxyz, r1.yyzw
    r1.yzw = ((r0.zzzz)*(r2.xxyz)+(r1.yyzw)).yzw;
    // 19: add r0.xyz, r0.xywx, r1.yzwy
    r0.xyz = ((r0.xywx)+(r1.yzwy)).xyz;
    // 20: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[6].yyyy
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[6].yyyy)).xyz;
    // 21: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 22: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 23: mov_sat r0.x, v4.z
    r0.x = (saturate(v4.zzzz)).x;
    // 24: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: add_sat r0.x, -r0.x, r1.x
    r0.x = (saturate((-(r0.xxxx))+(r1.xxxx))).x;
    // 26: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 27: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_fi_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative5323(ARTIST_NATIVE_INPUT input)
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
// fx_a_pa_db_02_1_tr: 9e4781052a66534b9646434db8d482db; selected map 4bdc6bcaf84fc1de676b6c243995d7aec03cd12fbb6a44cdf8836763e5c834ca.
float4 ArtistNative5324(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[3]=float4(input.skyUpperColor,0.f);
    source[4]=float4(input.skyLowerColor,0.f);
    source[5]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v8 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v9 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v7.z
    r0.x = ((r0.xxxx)*(v7.zzzz)).x;
    // 4: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: mul r0.yzw, r0.yyyy, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(source[4].xxyz)).yzw;
    // 7: mad r0.xyz, r0.xxxx, cb0[3].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[3].xyzx)+(r0.yzwy)).xyz;
    // 8: mul r0.xyz, r0.xyzx, cb0[5].wwww
    r0.xyz = ((r0.xyzx)*(source[5].wwww)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 11: add r1.xyzw, r1.wxyz, -r2.wxyz
    r1.xyzw = ((r1.wxyz)+(-(r2.wxyz))).xyzw;
    // 12: mad r1.xyzw, v0.wwww, r1.xyzw, r2.wxyz
    r1.xyzw = ((v0.wwww)*(r1.xyzw)+(r2.wxyz)).xyzw;
    // 13: mul r2.xyz, r1.yzwy, cb0[2].xxxx
    r2.xyz = ((r1.yzwy)*(source[2].xxxx)).xyz;
    // 14: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: mad r1.yzw, -cb0[2].xxxx, r1.yyzw, r0.wwww
    r1.yzw = ((-(source[2].xxxx))*(r1.yyzw)+(r0.wwww)).yzw;
    // 16: mul r0.w, r1.x, v3.w
    r0.w = ((r1.xxxx)*(v3.wwww)).w;
    // 17: mad r1.xyz, cb0[2].yyyy, r1.yzwy, r2.xyzx
    r1.xyz = ((source[2].yyyy)*(r1.yzwy)+(r2.xyzx)).xyz;
    // 18: mul r1.xyz, r1.xyzx, v3.xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)).xyz;
    // 19: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 20: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 21: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 23: mad r0.xyz, r1.xyzx, cb0[5].xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)*(source[5].xyzx)+(r2.xyzx)).xyz;
    // 25: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 26: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 27: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 28: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 29: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 30: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 32: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_b_pa_fi_01_1_tr: 37842b5dba72e14faea175dc62c7eba0; selected map 6832df3326975cb041b52309fb877b99ab156cb13e7f59177a1a5192adc21522.
float4 ArtistNative5325(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
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
    // 10: mul_sat r0.x, r0.x, l(0.166667)
    r0.x = (saturate((r0.xxxx)*(float4(0.166667,0.166667,0.166667,0.166667)))).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v2.zwzz, t0.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: add r0.yzw, r0.yyzw, -r1.xxyz
    r0.yzw = ((r0.yyzw)+(-(r1.xxyz))).yzw;
    // 14: mad r0.yzw, v0.wwww, r0.yyzw, r1.xxyz
    r0.yzw = ((v0.wwww)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 15: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 16: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 17: mul r1.y, r1.y, cb0[2].y
    r1.y = ((r1.yyyy)*(source[2].yyyy)).y;
    // 18: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 19: mul r1.y, r1.y, v3.w
    r1.y = ((r1.yyyy)*(v3.wwww)).y;
    // 20: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 21: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 22: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: add r1.xzw, -r0.yyzw, r1.xxxx
    r1.xzw = ((-(r0.yyzw))+(r1.xxxx)).xzw;
    // 24: mad r0.yzw, cb0[2].xxxx, r1.xxzw, r0.yyzw
    r0.yzw = ((source[2].xxxx)*(r1.xxzw)+(r0.yyzw)).yzw;
    // 25: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 26: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 27: movc o0.w, r1.y, l(0), r0.x
    output.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative5326(ARTIST_NATIVE_INPUT input)
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
// fx_j_pa_ring_07_20_tr: fa502144d8b78142b6f5f4b07eded49c; selected map 8ea91646f499633e55b966d2c4225a3dbfd13bc5f632517a851a85c9518c7826.
float4 ArtistNative5327(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = ((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[4].x = (((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].y = (((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[5].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 31: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 32: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 33: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 34: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 36: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 37: add r0.y, v4.x, l(-1.000000)
    r0.y = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 38: add r1.y, r0.x, r0.y
    r1.y = ((r0.xxxx)+(r0.yyyy)).y;
    // 39: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, cb0[3].x
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (source[3].xxxx).x, true).xyzw).x;
    // 40: mul_sat r0.x, r0.x, cb0[5].z
    r0.x = (saturate((r0.xxxx)*(source[5].zzzz))).x;
    // 41: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 42: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 44: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 46: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 47: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 48: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 49: mul r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)*(source[5].wwww)).y;
    // 50: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 51: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 52: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 53: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 54: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 55: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5327Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_twirl_05_12_tr: 9a2dab1499cc8a49874b98f683faf4e7; selected map 27755abe394407f43c34d91d127891cde973a422e99888123e62f2d590abba92.
float4 ArtistNative5328(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[8].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
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
    // 2: add r0.zw, r0.xxxy, r0.xxxy
    r0.zw = ((r0.xxxy)+(r0.xxxy)).zw;
    // 3: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 4: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 5: max r0.y, |r0.z|, |r0.w|
    r0.y = (max(abs(r0.zzzz),abs(r0.wwww))).y;
    // 6: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 7: min r1.x, |r0.z|, |r0.w|
    r1.x = (min(abs(r0.zzzz),abs(r0.wwww))).x;
    // 8: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 9: mul r1.x, r0.y, r0.y
    r1.x = ((r0.yyyy)*(r0.yyyy)).x;
    // 10: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 11: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 12: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 13: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 14: mul r1.y, r0.y, r1.x
    r1.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 15: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 16: lt r1.z, |r0.z|, |r0.w|
    r1.z = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.wwww))) * 0xffffffffu)).z;
    // 17: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 18: mad r0.y, r0.y, r1.x, r1.y
    r0.y = ((r0.yyyy)*(r1.xxxx)+(r1.yyyy)).y;
    // 19: lt r1.x, r0.z, -r0.z
    r1.x = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).x;
    // 20: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 21: add r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)+(r1.xxxx)).y;
    // 22: min r1.x, r0.z, r0.w
    r1.x = (min(r0.zzzz,r0.wwww)).x;
    // 23: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 24: max r1.y, r0.z, r0.w
    r1.y = (max(r0.zzzz,r0.wwww)).y;
    // 25: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 26: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: ge r0.w, r1.y, -r1.y
    r0.w = (asfloat((uint4)((r1.yyyy)>=(-(r1.yyyy))) * 0xffffffffu)).w;
    // 28: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 29: movc r0.y, r0.w, -r0.y, r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (-(r0.yyyy)) : (r0.yyyy)).y;
    // 30: add r0.w, -r0.x, l(0.500000)
    r0.w = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 31: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 32: mul r1.x, v4.w, cb0[4].z
    r1.x = ((v4.wwww)*(source[4].zzzz)).x;
    // 33: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 34: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 35: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 36: mul r1.x, r1.x, cb0[4].w
    r1.x = ((r1.xxxx)*(source[4].wwww)).x;
    // 37: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 38: mul r1.x, r1.x, cb0[5].x
    r1.x = ((r1.xxxx)*(source[5].xxxx)).x;
    // 39: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 40: mad r1.x, r0.y, l(0.318310), r0.w
    r1.x = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.wwww)).x;
    // 41: mul r0.y, cb0[3].y, cb0[3].z
    r0.y = ((source[3].yyyy)*(source[3].zzzz)).y;
    // 42: mul r0.w, r0.y, cb0[5].z
    r0.w = ((r0.yyyy)*(source[5].zzzz)).w;
    // 43: mad r2.x, cb0[5].w, r1.x, r0.w
    r2.x = ((source[5].wwww)*(r1.xxxx)+(r0.wwww)).x;
    // 44: add r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)+(r0.xxxx)).w;
    // 45: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 46: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 47: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 48: lt r1.z, r0.x, l(0.000000)
    r1.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 49: mad r0.x, -r0.x, cb0[7].w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mul_sat r0.x, r0.x, cb0[8].w
    r0.x = (saturate((r0.xxxx)*(source[8].wwww))).x;
    // 51: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 52: mul r0.w, r1.y, cb0[6].x
    r0.w = ((r1.yyyy)*(source[6].xxxx)).w;
    // 53: mul r1.xy, r1.xyxx, cb0[4].xyxx
    r1.xy = ((r1.xyxx)*(source[4].xyxx)).xy;
    // 54: mad r2.y, r0.y, cb0[6].y, r0.w
    r2.y = ((r0.yyyy)*(source[6].yyyy)+(r0.wwww)).y;
    // 55: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, cb0[3].x
    r2.xyz = (ArtistNativeSample1((r2.xyxx).xy, (source[3].xxxx).x, true).xyzw).xyz;
    // 56: mad r3.x, r0.y, cb0[3].w, r1.x
    r3.x = ((r0.yyyy)*(source[3].wwww)+(r1.xxxx)).x;
    // 57: mad r3.y, r0.y, cb0[5].y, r1.y
    r3.y = ((r0.yyyy)*(source[5].yyyy)+(r1.yyyy)).y;
    // 58: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r3.xyxx, t0.xyzw, s0, cb0[3].x
    r1.xyz = (ArtistNativeSample0((r3.xyxx).xy, (source[3].xxxx).x, true).xyzw).xyz;
    // 59: add r0.y, -r1.x, r2.x
    r0.y = ((-(r1.xxxx))+(r2.xxxx)).y;
    // 60: mad r0.y, r0.y, l(0.500000), r1.x
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xxxx)).y;
    // 61: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 62: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 63: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 64: mad_sat r0.x, r0.y, cb0[7].x, r0.x
    r0.x = (saturate((r0.yyyy)*(source[7].xxxx)+(r0.xxxx))).x;
    // 65: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 66: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 67: mul r0.y, r0.y, cb0[9].x
    r0.y = ((r0.yyyy)*(source[9].xxxx)).y;
    // 68: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 69: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 70: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 71: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 72: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 73: mul r0.xyz, r2.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 74: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 75: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.wwww)).xyz;
    // 76: mad r0.xyz, cb0[6].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[6].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 77: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 78: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 79: mul r0.xyz, r0.xyzx, cb0[6].wwww
    r0.xyz = ((r0.xyzx)*(source[6].wwww)).xyz;
    // 80: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 81: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 82: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 83: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 84: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5328Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_i_pa_absorb_03_ad: 2ec5063e0538944284e870291ecd50b0; selected map 6e7148a50bcf7b23a155dcec76d9d429577e3a8928c8a9d44e6c4ac1cbdaf44a.
float4 ArtistNative5329(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = ((g_ArtistSourceMaterialParameters[0u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].z = ((g_ArtistSourceMaterialParameters[4u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: add r0.zw, r0.yyyx, r0.yyyx
    r0.zw = ((r0.yyyx)+(r0.yyyx)).zw;
    // 3: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 4: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 5: max r0.y, |r0.w|, |r0.z|
    r0.y = (max(abs(r0.wwww),abs(r0.zzzz))).y;
    // 6: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 7: min r1.x, |r0.w|, |r0.z|
    r1.x = (min(abs(r0.wwww),abs(r0.zzzz))).x;
    // 8: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 9: mul r1.x, r0.y, r0.y
    r1.x = ((r0.yyyy)*(r0.yyyy)).x;
    // 10: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 11: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 12: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 13: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 14: mul r1.y, r0.y, r1.x
    r1.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 15: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 16: lt r1.z, |r0.w|, |r0.z|
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).z;
    // 17: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 18: mad r0.y, r0.y, r1.x, r1.y
    r0.y = ((r0.yyyy)*(r1.xxxx)+(r1.yyyy)).y;
    // 19: lt r1.x, r0.w, -r0.w
    r1.x = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).x;
    // 20: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 21: add r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)+(r1.xxxx)).y;
    // 22: min r1.x, r0.w, r0.z
    r1.x = (min(r0.wwww,r0.zzzz)).x;
    // 23: max r0.z, r0.w, r0.z
    r0.z = (max(r0.wwww,r0.zzzz)).z;
    // 24: ge r0.z, r0.z, -r0.z
    r0.z = (asfloat((uint4)((r0.zzzz)>=(-(r0.zzzz))) * 0xffffffffu)).z;
    // 25: lt r0.w, r1.x, -r1.x
    r0.w = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).w;
    // 26: and r0.z, r0.z, r0.w
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r0.wwww))).z;
    // 27: movc r0.y, r0.z, -r0.y, r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (-(r0.yyyy)) : (r0.yyyy)).y;
    // 28: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 29: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 30: dp2 r0.z, r0.zzzz, cb0[3].yyyy
    r0.z = (dot((r0.zzzz).xy,(source[3].yyyy).xy).xxxx).z;
    // 31: mad r1.x, r0.y, l(0.318310), r0.z
    r1.x = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).x;
    // 32: mul r2.x, r1.x, cb0[4].w
    r2.x = ((r1.xxxx)*(source[4].wwww)).x;
    // 33: mul r3.x, r2.x, cb0[5].w
    r3.x = ((r2.xxxx)*(source[5].wwww)).x;
    // 34: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 35: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 36: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 38: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 39: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 40: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 41: add r1.y, r0.x, cb0[4].z
    r1.y = ((r0.xxxx)+(source[4].zzzz)).y;
    // 42: mad r2.y, r1.y, cb0[5].x, cb0[5].z
    r2.y = ((r1.yyyy)*(source[5].xxxx)+(source[5].zzzz)).y;
    // 43: mad r3.y, r2.y, cb0[6].x, cb0[6].z
    r3.y = ((r2.yyyy)*(source[6].xxxx)+(source[6].zzzz)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r3.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 45: mad r0.yz, cb0[6].wwww, r0.xxxx, r2.xxyx
    r0.yz = ((source[6].wwww)*(r0.xxxx)+(r2.xxyx)).yz;
    // 46: mad r0.xw, cb0[6].wwww, r0.xxxx, r1.xxxy
    r0.xw = ((source[6].wwww)*(r0.xxxx)+(r1.xxxy)).xw;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t1.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 49: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 50: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: mul r0.z, r0.z, cb0[7].x
    r0.z = ((r0.zzzz)*(source[7].xxxx)).z;
    // 52: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 53: mul r0.z, r0.z, cb0[7].y
    r0.z = ((r0.zzzz)*(source[7].yyyy)).z;
    // 54: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 55: dp3 r0.z, r0.yyyy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r0.yyyy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 56: add r0.z, -r0.y, r0.z
    r0.z = ((-(r0.yyyy))+(r0.zzzz)).z;
    // 57: mad r0.y, cb0[7].z, r0.z, r0.y
    r0.y = ((source[7].zzzz)*(r0.zzzz)+(r0.yyyy)).y;
    // 58: mul r0.yzw, r0.yyyy, cb0[2].xxyz
    r0.yzw = ((r0.yyyy)*(source[2].xxyz)).yzw;
    // 59: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 60: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 61: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 62: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 63: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 64: mul r0.xyz, r0.xyzx, v3.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)).xyz;
    // 65: mad r0.xyz, r0.xyzx, l(1.500000, 1.500000, 1.500000, 0.000000), cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(float4(1.500000,1.500000,1.500000,0.000000))+(source[1].xyzx)).xyz;
    // 66: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 67: mad r1.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 68: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 69: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 70: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 71: mul r1.x, r1.x, cb0[8].y
    r1.x = ((r1.xxxx)*(source[8].yyyy)).x;
    // 72: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 73: mul r1.x, r1.x, cb0[8].z
    r1.x = ((r1.xxxx)*(source[8].zzzz)).x;
    // 74: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 75: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 77: log r1.y, r0.w
    r1.y = (log2(r0.wwww)).y;
    // 78: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 79: mul r1.y, r1.y, cb0[8].x
    r1.y = ((r1.yyyy)*(source[8].xxxx)).y;
    // 80: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 81: movc r0.w, r0.w, l(0), r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).w;
    // 82: mul r0.w, r1.x, r0.w
    r0.w = ((r1.xxxx)*(r0.wwww)).w;
    // 83: mul r0.w, r0.w, cb0[8].w
    r0.w = ((r0.wwww)*(source[8].wwww)).w;
    // 84: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 85: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 86: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5329Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[6u];
    source[1].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[1].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[1].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[1].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[2].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].z = ((g_ArtistSourceMaterialParameters[0u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[3].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[4].z = ((g_ArtistSourceMaterialParameters[4u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[4].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: add r0.zw, r0.yyyx, r0.yyyx
    r0.zw = ((r0.yyyx)+(r0.yyyx)).zw;
    // 3: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 4: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 5: max r0.y, |r0.w|, |r0.z|
    r0.y = (max(abs(r0.wwww),abs(r0.zzzz))).y;
    // 6: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 7: min r1.x, |r0.w|, |r0.z|
    r1.x = (min(abs(r0.wwww),abs(r0.zzzz))).x;
    // 8: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 9: mul r1.x, r0.y, r0.y
    r1.x = ((r0.yyyy)*(r0.yyyy)).x;
    // 10: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 11: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 12: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 13: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 14: mul r1.y, r0.y, r1.x
    r1.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 15: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 16: lt r1.z, |r0.w|, |r0.z|
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).z;
    // 17: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 18: mad r0.y, r0.y, r1.x, r1.y
    r0.y = ((r0.yyyy)*(r1.xxxx)+(r1.yyyy)).y;
    // 19: lt r1.x, r0.w, -r0.w
    r1.x = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).x;
    // 20: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 21: add r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)+(r1.xxxx)).y;
    // 22: min r1.x, r0.w, r0.z
    r1.x = (min(r0.wwww,r0.zzzz)).x;
    // 23: max r0.z, r0.w, r0.z
    r0.z = (max(r0.wwww,r0.zzzz)).z;
    // 24: ge r0.z, r0.z, -r0.z
    r0.z = (asfloat((uint4)((r0.zzzz)>=(-(r0.zzzz))) * 0xffffffffu)).z;
    // 25: lt r0.w, r1.x, -r1.x
    r0.w = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).w;
    // 26: and r0.z, r0.z, r0.w
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r0.wwww))).z;
    // 27: movc r0.y, r0.z, -r0.y, r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (-(r0.yyyy)) : (r0.yyyy)).y;
    // 28: mul r0.y, r0.y, cb0[1].x
    r0.y = ((r0.yyyy)*(source[1].xxxx)).y;
    // 29: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 30: dp2 r0.z, r0.zzzz, cb0[1].yyyy
    r0.z = (dot((r0.zzzz).xy,(source[1].yyyy).xy).xxxx).z;
    // 31: mad r1.x, r0.y, l(0.318310), r0.z
    r1.x = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).x;
    // 32: mul r2.x, r1.x, cb0[2].w
    r2.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 33: mul r3.x, r2.x, cb0[3].w
    r3.x = ((r2.xxxx)*(source[3].wwww)).x;
    // 34: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 35: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 36: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, cb0[1].z
    r0.y = ((r0.yyyy)*(source[1].zzzz)).y;
    // 38: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 39: mul r0.y, r0.y, cb0[1].w
    r0.y = ((r0.yyyy)*(source[1].wwww)).y;
    // 40: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 41: add r1.y, r0.x, cb0[2].z
    r1.y = ((r0.xxxx)+(source[2].zzzz)).y;
    // 42: mad r2.y, r1.y, cb0[3].x, cb0[3].z
    r2.y = ((r1.yyyy)*(source[3].xxxx)+(source[3].zzzz)).y;
    // 43: mad r3.y, r2.y, cb0[4].x, cb0[4].z
    r3.y = ((r2.yyyy)*(source[4].xxxx)+(source[4].zzzz)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r3.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 45: mad r0.yz, cb0[4].wwww, r0.xxxx, r2.xxyx
    r0.yz = ((source[4].wwww)*(r0.xxxx)+(r2.xxyx)).yz;
    // 46: mad r0.xw, cb0[4].wwww, r0.xxxx, r1.xxxy
    r0.xw = ((source[4].wwww)*(r0.xxxx)+(r1.xxxy)).xw;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 49: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 50: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 52: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 53: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 54: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 55: dp3 r0.z, r0.yyyy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r0.yyyy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 56: add r0.z, -r0.y, r0.z
    r0.z = ((-(r0.yyyy))+(r0.zzzz)).z;
    // 57: mad r0.y, cb0[5].z, r0.z, r0.y
    r0.y = ((source[5].zzzz)*(r0.zzzz)+(r0.yyyy)).y;
    // 58: mul r1.xyzw, r0.yyyy, cb0[0].xyxy
    r1.xyzw = ((r0.yyyy)*(source[0].xyxy)).xyzw;
    // 59: mul r0.xyzw, r0.xxxx, r1.xyzw
    r0.xyzw = ((r0.xxxx)*(r1.xyzw)).xyzw;
    // 60: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 61: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 62: mul r0.xyzw, r0.xyzw, cb0[5].wwww
    r0.xyzw = ((r0.xyzw)*(source[5].wwww)).xyzw;
    // 63: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 64: mul r0.xyzw, r0.xyzw, v2.xyxy
    r0.xyzw = ((r0.xyzw)*(v2.xyxy)).xyzw;
    // 65: mul r0.xyzw, r0.xyzw, l(1.500000, 1.500000, 1.500000, 1.500000)
    r0.xyzw = ((r0.xyzw)*(float4(1.500000,1.500000,1.500000,1.500000))).xyzw;
    // 66: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 67: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 68: mul r0.xyzw, r0.xyzw, cb0[7].xxxx
    r0.xyzw = ((r0.xyzw)*(source[7].xxxx)).xyzw;
    // 69: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 70: mul r0.xyzw, r0.xyzw, cb0[7].yyyy
    r0.xyzw = ((r0.xyzw)*(source[7].yyyy)).xyzw;
    // 71: mul r0.xyzw, r0.xyzw, v2.wwww
    r0.xyzw = ((r0.xyzw)*(v2.wwww)).xyzw;
    // 72: mad r1.xy, v1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 73: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 74: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 76: mul r1.y, r1.y, cb0[6].y
    r1.y = ((r1.yyyy)*(source[6].yyyy)).y;
    // 77: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 78: mul r1.y, r1.y, cb0[6].z
    r1.y = ((r1.yyyy)*(source[6].zzzz)).y;
    // 79: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 80: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 81: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 82: log r1.z, r1.x
    r1.z = (log2(r1.xxxx)).z;
    // 83: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 84: mul r1.z, r1.z, cb0[6].x
    r1.z = ((r1.zzzz)*(source[6].xxxx)).z;
    // 85: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 86: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 87: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 88: mul r1.x, r1.x, cb0[6].w
    r1.x = ((r1.xxxx)*(source[6].wwww)).x;
    // 89: mul r1.x, r1.x, v2.w
    r1.x = ((r1.xxxx)*(v2.wwww)).x;
    // 90: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 91: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 92: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 93: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 94: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 95: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 96: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 97: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 98: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 99: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 100: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 101: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 102: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 103: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 104: source device depth mapped to centimetre view depth; reconstruction at 106.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 106-109: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 110: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 111: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 112: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 113: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 114: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_05_11_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative5330(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_master_01_019_dt_tr: 996321332f8eca4ca8d6a2009a097b17; selected map 386c9086e6ac014ecf37df31bf58401910764496b14e734284ce698935c74884.
float4 ArtistNative5331(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[4].zwzz
    r0.xy = ((v2.xyxx)*(source[4].zwzz)).xy;
    // 2: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[4].y, r0.x
    r1.x = ((r0.zzzz)*(source[4].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[5].x, r0.y
    r1.y = ((r0.zzzz)*(source[5].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.z = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: mad r0.w, cb0[3].y, cb0[6].y, cb0[6].z
    r0.w = ((source[3].yyyy)*(source[6].yyyy)+(source[6].zzzz)).w;
    // 14: sincos r1.x, r2.x, r0.w
    { const float4 sourceAngle = r0.wwww; r1.x = (sin(sourceAngle)).x; r2.x = (cos(sourceAngle)).x; }
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
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5331Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_b_pa_smoke_03_tr: 1c74b0c6adfea94ab2bde04b48de775e; selected map aa44bdfe4996f07760d9d283c211293a18498fe41be916fb05ae734209d28236.
float4 ArtistNative5332(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.119999997, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0599999987, 0.0, 0.0, 0.0))),1u);
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
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 18: add r0.yz, v2.xxyx, cb0[3].xxyx
    r0.yz = ((v2.xxyx)+(source[3].xxyx)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 20: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 21: mad r0.xy, r0.xxxx, l(0.100000, 0.100000, 0.000000, 0.000000), v2.xyxx
    r0.xy = ((r0.xxxx)*(float4(0.100000,0.100000,0.000000,0.000000))+(v2.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 24: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ap_03_1_tr: 991ecc5522859c49bb034318a827af71; selected map fd85ade76eaa888dbc097dfdfc3900cacd64ae68812cae449a674d712986917f.
float4 ArtistNative5333(ARTIST_NATIVE_INPUT input)
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
    // 16: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 18: movc o0.w, r0.w, l(0), r1.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 19: mul r1.xyz, r0.xyzx, cb0[2].yyyy
    r1.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 20: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyz, -cb0[2].yyyy, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[2].yyyy))*(r0.xyzx)+(r0.wwww)).xyz;
    // 22: mad r0.xyz, cb0[2].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 24: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5333Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_a_me_panning_02_ad: 68e524b21d128d4e80f91e40d32bfdd9; selected map 2aa8c157751816e6129b344bb66e57cfeeda1feb15680d91d73c9208a81392ae.
float4 ArtistNative5334(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
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
    // 1: mov r0.xz, v4.xxxx
    r0.xz = (v4.xxxx).xz;
    // 2: mul r0.yw, v4.yyyy, cb0[3].yyyy
    r0.yw = ((v4.yyyy)*(source[3].yyyy)).yw;
    // 3: mad r0.xyzw, cb0[3].xxxx, l(0.100000, -0.500000, -0.100000, -0.750000), r0.xyzw
    r0.xyzw = ((source[3].xxxx)*(float4(0.100000,-0.500000,-0.100000,-0.750000))+(r0.xyzw)).xyzw;
    // 4: mul r1.xyzw, v4.xyxy, l(3.000000, 1.000000, 4.000000, 1.000000)
    r1.xyzw = ((v4.xyxy)*(float4(3.000000,1.000000,4.000000,1.000000))).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.zwzz, t2.yxzw, s2, l(0.000000)
    r1.y = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: mad r0.zw, r1.yyyy, l(0.000000, 0.000000, 0.300000, 0.300000), r0.zzzw
    r0.zw = ((r1.yyyy)*(float4(0.000000,0.000000,0.300000,0.300000))+(r0.zzzw)).zw;
    // 8: mad r0.xy, r1.xxxx, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.xxxx)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyz = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.zwzz, t4.xyzw, s4, l(0.000000)
    r0.xyz = (ArtistNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 12: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t5.yzwx, s5, l(0.000000)
    r0.w = (ArtistNativeSample5((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 14: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 15: add r1.xy, v4.xyxx, v4.xyxx
    r1.xy = ((v4.xyxx)+(v4.xyxx)).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.xzwy, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzwy).w;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 18: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 19: add r0.w, r0.w, cb0[3].z
    r0.w = ((r0.wwww)+(source[3].zzzz)).w;
    // 20: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 21: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 22: mul r1.x, r1.x, cb0[3].w
    r1.x = ((r1.xxxx)*(source[3].wwww)).x;
    // 23: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 24: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 25: mad r0.xyz, r0.wwww, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 26: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 27: mul r0.w, cb0[0].x, cb0[1].w
    r0.w = ((source[0].xxxx)*(source[1].wwww)).w;
    // 28: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 29: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_me_ap_02_5_ts_tr: f3e012370cd62d4f9a62c9c2d507b775; selected map 3f47e48618d77fb4eb098e3fb3db46f5a5252affc813ce1120fd377c1569187b.
float4 ArtistNative5335(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mad r0.x, cb0[4].x, cb0[3].x, l(-1.000000)
    r0.x = ((source[4].xxxx)*(source[3].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, cb0[3].x, cb0[4].x
    r0.y = ((source[3].xxxx)*(source[4].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v4.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v4.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5335Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_j_pa_windwv_01_1_ad: 55c8f3a3029acc42a10aaa723568e34b; selected map b1f2e930fa663280c0c2095c371485964b730a4146e6c152bdccff5a33239493.
float4 ArtistNative5336(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 32: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 33: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 34: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 35: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 36: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 37: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 38: mad r1.y, cb0[2].z, cb0[3].y, r0.x
    r1.y = ((source[2].zzzz)*(source[3].yyyy)+(r0.xxxx)).y;
    // 39: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, cb0[2].x
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).x;
    // 40: mul_sat r0.x, r0.x, cb0[3].z
    r0.x = (saturate((r0.xxxx)*(source[3].zzzz))).x;
    // 41: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 42: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 44: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 46: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 47: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 48: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 49: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 50: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 51: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 52: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 53: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 54: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 55: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 56: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 57: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5336Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_e_pa_gl_03_1_ad: 0fc154ee4cdbcc408e07e6b3989115cd; selected map d5b11bc5bf7cc81b7010fcfda736799dded7600752766e0b014b71fba4e86c6b.
float4 ArtistNative5337(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative5337Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[1u];
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
    // 1: add r0.xy, v1.yxyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.yxyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 3: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 4: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 5: add r0.y, -r0.z, l(1.000000)
    r0.y = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 6: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 7: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 8: mul r0.z, r0.z, l(10.000000)
    r0.z = ((r0.zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 9: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 10: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 11: add r0.y, r0.y, v3.x
    r0.y = ((r0.yyyy)+(v3.xxxx)).y;
    // 12: mul r0.y, r0.y, l(3.141593)
    r0.y = ((r0.yyyy)*(float4(3.141593,3.141593,3.141593,3.141593))).y;
    // 13: sincos r0.y, null, r0.y
    { const float4 sourceAngle = r0.yyyy; r0.y = (sin(sourceAngle)).y; }
    // 14: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 15: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 16: mul r0.z, r0.z, l(20.000000)
    r0.z = ((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000))).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 19: sincos r0.z, null, r0.z
    { const float4 sourceAngle = r0.zzzz; r0.z = (sin(sourceAngle)).z; }
    // 20: mad r1.xy, v3.xxxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.300000, 0.170000, 0.000000, 0.000000)
    r1.xy = ((v3.xxxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.300000,0.170000,0.000000,0.000000))).xy;
    // 21: max r1.xy, r1.xyxx, l(0.000010, 0.000010, 0.000000, 0.000000)
    r1.xy = (max(r1.xyxx,float4(0.000010,0.000010,0.000000,0.000000))).xy;
    // 22: div r1.xy, l(1.000000, 1.000000, 1.000000, 1.000000), r1.xyxx
    r1.xy = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xyxx)).xy;
    // 23: mad r1.xy, -r0.xxxx, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r0.xxxx))*(r1.xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: add_sat r0.x, r0.x, r0.x
    r0.x = (saturate((r0.xxxx)+(r0.xxxx))).x;
    // 26: max r1.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (max(r1.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 27: lt r0.w, l(0.000000), r1.y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(r1.yyyy)) * 0xffffffffu)).w;
    // 28: or r0.y, r0.w, r0.y
    r0.y = (asfloat(asuint(r0.wwww) | asuint(r0.yyyy))).y;
    // 29: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 30: rsq r0.z, r1.x
    r0.z = (rsqrt(r1.xxxx)).z;
    // 31: lt r0.w, r1.x, l(0.000001)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 32: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 33: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 34: movc r0.z, r0.w, l(-0.000000), -r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.zzzz))).z;
    // 35: add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // 36: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 37: mul r0.x, r0.x, -v3.z
    r0.x = ((r0.xxxx)*(-(v3.zzzz))).x;
    // 38: mul r0.x, r0.x, v3.y
    r0.x = ((r0.xxxx)*(v3.yyyy)).x;
    // 39: div r0.y, l(1536.000000), v4.z
    r0.y = ((float4(1536.000000,1536.000000,1536.000000,1536.000000))/(v4.zzzz)).y;
    // 40: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 41: mul r0.yw, r0.xxxx, cb0[0].xxxx
    r0.yw = ((r0.xxxx)*(source[0].xxxx)).yw;
    // 42: mov r0.xz, cb0[0].xxxx
    r0.xz = (source[0].xxxx).xz;
    // 43: mad r0.xyzw, r0.xyzw, l(0.000000, -2.000000, 0.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(0.000000,-2.000000,0.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 44: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 45: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 46: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 47: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 48: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 49: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 50: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 51: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 52: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 53: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 54: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 55: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 56: source device depth mapped to centimetre view depth; reconstruction at 58.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 58-61: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 62: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 63: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 64: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 65: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 66: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_gl_07_1_ad: 7061f1db6f1b3e46839b1cfecd6ab49a; selected map 208a1504c3ebd40baa89540c23fb33982c5d2af03a379c54ef8a1a4a1433b87f.
float4 ArtistNative5338(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend((float4(1.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[1u].zzzz),(float4(1.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[1u].wwww),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[6].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[6].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_flowtrail_01_14_tr_ts: 3174030136b0cf44a810bf8afd13e8fc; selected map 2966a80941d7724da6e5198a2fc8a5f005fddb8c7500222142d1e394686b0088.
float4 ArtistNative5339(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = g_ArtistSourceMaterialParameters[7u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].zzzz,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[3u].xxxx,1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx),(g_ArtistSourceMaterialParameters[2u].wwww*g_ArtistSourceMaterialTime.xxxx),1u);
    source[8] = input.dynamicParameter;
    source[9] = ArtistNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx),1u);
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[13] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[14] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[15].x = ((g_ArtistSourceMaterialParameters[2u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[15].y = ((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[16].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].y = ((g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[16].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[16].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[17].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[17].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[18].x = (cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[19].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[19].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: add r0.x, v4.y, cb0[9].y
    r0.x = ((v4.yyyy)+(source[9].yyyy)).x;
    // 2: mul r0.x, r0.x, cb0[16].z
    r0.x = ((r0.xxxx)*(source[16].zzzz)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    { const float4 sourceAngle = r0.xxxx; r0.x = (sin(sourceAngle)).x; }
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
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5339Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = input.dynamicParameter;
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v2.xyxx, cb0[2].xyxx, l(0.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(source[2].xyxx)+(float4(0.000000,-1.000000,0.000000,0.000000))).xy;
    // 2: add r1.x, r0.x, cb0[5].w
    r1.x = ((r0.xxxx)+(source[5].wwww)).x;
    // 3: add r1.y, r0.y, cb0[1].y
    r1.y = ((r0.yyyy)+(source[1].yyyy)).y;
    // 4: add r0.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 5: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 6: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 7: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, v2.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 10: mul r0.xyzw, r0.xyxy, r0.zwzw
    r0.xyzw = ((r0.xyxy)*(r0.zwzw)).xyzw;
    // 11: mul_sat r0.xyzw, r0.xyzw, cb0[6].xxxx
    r0.xyzw = (saturate((r0.xyzw)*(source[6].xxxx))).xyzw;
    // 12: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
    // 13: mul r0.xyzw, r0.xyzw, cb0[6].yyyy
    r0.xyzw = ((r0.xyzw)*(source[6].yyyy)).xyzw;
    // 14: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 15: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 16: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 17: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 18: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 19: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 20: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 21: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 22: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 23: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 24: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 25: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 26: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 27: source device depth mapped to centimetre view depth; reconstruction at 29.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 29-32: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 33: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 34: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 35: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 36: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 37: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_h_pa_circle_01_02_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 ArtistNative5340(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[5].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].xxxx))).x;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_f_pa_ri_01_2_ad: 22fe1d84b183a24ba061efbe70ff88fe; selected map dcb4af1b0c51e126ebfcd50d3245e9fb4ef45276b0b47937288bd666ab550511.
float4 ArtistNative5341(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_ht_02_ad: ea3e8860d5fea3429ec05d193a929497; selected map 8f87d54985091d3ab901d1c208b7a70bee69a38f2a322027fb9b83f28b130576.
float4 ArtistNative5342(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    r0.yz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
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
    r0.w = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t3.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    // 35: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 36: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 37: mul r0.yzw, r1.xxyz, cb0[3].wwww
    r0.yzw = ((r1.xxyz)*(source[3].wwww)).yzw;
    // 38: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: mad r1.xyz, -cb0[3].wwww, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[3].wwww))*(r1.xyzx)+(r1.wwww)).xyz;
    // 40: mad r0.yzw, cb0[4].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 41: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 42: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 43: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 44: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5342Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[0].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[0].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[0].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[1].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[1].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[1].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[1].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mul r0.xy, v1.xyxx, cb0[0].zwzz
    r0.xy = ((v1.xyxx)*(source[0].zwzz)).xy;
    // 2: mad r1.x, cb0[0].y, cb0[0].x, r0.x
    r1.x = ((source[0].yyyy)*(source[0].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[0].y, cb0[1].x, r0.y
    r1.y = ((source[0].yyyy)*(source[1].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, cb0[1].yyyy, r0.xyxx, v1.xyxx
    r0.xy = ((source[1].yyyy)*(r0.xyxx)+(v1.xyxx)).xy;
    // 6: mad r0.z, cb0[1].z, v3.x, l(-1.000000)
    r0.z = ((source[1].zzzz)*(v3.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 7: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 8: mul r0.w, v3.x, cb0[1].z
    r0.w = ((v3.xxxx)*(source[1].zzzz)).w;
    // 9: mad r0.xy, r0.wwww, r0.xyxx, -r0.zzzz
    r0.xy = ((r0.wwww)*(r0.xyxx)+(-(r0.zzzz))).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 12: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, cb0[2].y
    r0.y = ((r0.yyyy)*(source[2].yyyy)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v2.w
    r0.y = ((r0.yyyy)*(v2.wwww)).y;
    // 16: movc r0.x, r0.x, l(0), |r0.y|
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, v3.y
    r0.y = ((r0.yyyy)*(v3.yyyy)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 22: div r0.zw, v4.xxxy, v4.wwww
    r0.zw = ((v4.xxxy)/(v4.wwww)).zw;
    // 23: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 24: source device depth mapped to centimetre view depth; reconstruction at 26.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 26-29: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 30: add r1.x, r1.x, -v4.w
    r1.x = ((r1.xxxx)+(-(v4.wwww))).x;
    // 31: add r1.y, -cb0[2].z, l(1.000000)
    r1.y = ((-(source[2].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 33: div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // 34: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 35: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 36: mul r1.xz, r0.xxxx, l(2.000000, 0.000000, 2.000000, 0.000000)
    r1.xz = ((r0.xxxx)*(float4(2.000000,0.000000,2.000000,0.000000))).xz;
    // 37: mov r1.yw, l(0,-0.000000,0,-0.000000)
    r1.yw = (float4(asfloat(0u),-0.000000,asfloat(0u),-0.000000)).yw;
    // 38: add r1.xyzw, r1.xyzw, l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r1.xyzw)+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 39: mad r1.xyzw, r1.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r1.xyzw = ((r1.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 40: dp2 r0.x, r1.zwzz, r1.zwzz
    r0.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 41: add r0.x, r0.x, l(-0.100000)
    r0.x = ((r0.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 42: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 43: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) return 0.f;
    // 44: mad r0.xy, r1.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r0.zwzz
    r0.xy = ((r1.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r0.zwzz)).xy;
    // 45: mul r0.zw, r1.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r1.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 46: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 47: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 48: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 49: source device depth mapped to centimetre view depth; reconstruction at 51.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 51-54: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 55: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 56: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 57: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 58: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 59: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_i_pa_aurawave_002_ad: 61fc5d8bae59bd43b0388e41c7e9a272; selected map 958b2157faeb26b1ad29a4eaf23885dd742a29e789d252f20022249902f6f4fa.
float4 ArtistNative5343(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    // 1: add r0.xy, v2.yxyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.yxyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
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
    // 27: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[3].yyyy
    r0.z = (dot((r0.zzzz).xy,(source[3].yyyy).xy).xxxx).z;
    // 30: mad r1.x, r0.y, l(0.318310), r0.z
    r1.x = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).x;
    // 31: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 32: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 33: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 34: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 35: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 37: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 38: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 39: movc r1.y, r0.z, l(0), r0.y
    r1.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: mad r1.z, r1.y, v4.y, v4.x
    r1.z = ((r1.yyyy)*(v4.yyyy)+(v4.xxxx)).z;
    // 42: mad r0.yz, r0.yyyy, l(0.000000, 0.200000, 0.200000, 0.000000), r1.xxzx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.200000,0.200000,0.000000))+(r1.xxzx)).yz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 44: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 45: mul r0.z, r0.z, cb0[3].z
    r0.z = ((r0.zzzz)*(source[3].zzzz)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: mul r0.z, r0.z, l(8.000000)
    r0.z = ((r0.zzzz)*(float4(8.000000,8.000000,8.000000,8.000000))).z;
    // 48: lt r0.w, |r0.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 49: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 50: add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // 51: mul r2.x, r1.x, cb0[3].w
    r2.x = ((r1.xxxx)*(source[3].wwww)).x;
    // 52: mul r0.zw, r1.xxxz, cb0[5].zzzw
    r0.zw = ((r1.xxxz)*(source[5].zzzw)).zw;
    // 53: add r1.x, r1.z, cb0[4].z
    r1.x = ((r1.zzzz)+(source[4].zzzz)).x;
    // 54: mul r2.y, r1.x, cb0[4].w
    r2.y = ((r1.xxxx)*(source[4].wwww)).y;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r0.zwzz, t3.wxyz, s3, l(0.000000)
    r1.yzw = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 57: max r1.yzw, |r1.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r1.yzw = (max(abs(r1.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 58: log r1.yzw, r1.yyzw
    r1.yzw = (log2(r1.yyzw)).yzw;
    // 59: mul r1.yzw, r1.yyzw, cb0[6].xxxx
    r1.yzw = ((r1.yyzw)*(source[6].xxxx)).yzw;
    // 60: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 61: log r0.z, |r1.x|
    r0.z = (log2(abs(r1.xxxx))).z;
    // 62: lt r0.w, |r1.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 63: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 64: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 65: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 66: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 67: mad r0.y, r0.y, r0.z, r0.y
    r0.y = ((r0.yyyy)*(r0.zzzz)+(r0.yyyy)).y;
    // 68: mul r2.xyz, r1.yzwy, cb0[6].yyyy
    r2.xyz = ((r1.yzwy)*(source[6].yyyy)).xyz;
    // 69: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 70: mad r1.xyz, -cb0[6].yyyy, r1.yzwy, r0.zzzz
    r1.xyz = ((-(source[6].yyyy))*(r1.yzwy)+(r0.zzzz)).xyz;
    // 71: mad r1.xyz, cb0[6].zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((source[6].zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 72: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 73: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 74: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 75: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 76: mul r1.x, |r0.x|, |r0.x|
    r1.x = ((abs(r0.xxxx))*(abs(r0.xxxx))).x;
    // 77: mul r1.x, |r0.x|, r1.x
    r1.x = ((abs(r0.xxxx))*(r1.xxxx)).x;
    // 78: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 79: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 80: mul r1.x, r1.x, l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 81: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 82: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 83: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_ring_05_ad: 107e821614ee944aa4b7e35e6f57cded; selected map 33b95adfbb6e274fffd0a5208ae5c12ab5da2fc5d2a1420d1fc6ceb5a288e459.
float4 ArtistNative5344(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].w = ((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[6].x = (((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].y = (((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].w = ((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[10].x = (((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[10].y = (((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[11].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[12].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
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
    // 10: add r0.y, -cb0[12].z, l(1.000000)
    r0.y = ((-(source[12].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    // 42: mad r0.z, r0.z, l(0.159155), l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 43: mad r1.x, cb0[10].z, r0.z, cb0[4].x
    r1.x = ((source[10].zzzz)*(r0.zzzz)+(source[4].xxxx)).x;
    // 44: mad r2.x, cb0[6].z, r0.z, cb0[2].x
    r2.x = ((source[6].zzzz)*(r0.zzzz)+(source[2].xxxx)).x;
    // 45: mul r0.z, v2.x, cb0[7].w
    r0.z = ((v2.xxxx)*(source[7].wwww)).z;
    // 46: mad r3.x, cb0[5].y, cb0[7].z, r0.z
    r3.x = ((source[5].yyyy)*(source[7].zzzz)+(r0.zzzz)).x;
    // 47: mul r0.z, v2.y, cb0[8].x
    r0.z = ((v2.yyyy)*(source[8].xxxx)).z;
    // 48: mad r3.y, cb0[5].y, cb0[8].y, r0.z
    r3.y = ((source[5].yyyy)*(source[8].yyyy)+(r0.zzzz)).y;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r3.xyxx, t0.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 50: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 51: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 52: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 53: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 54: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 55: add r0.w, v4.x, l(-1.000000)
    r0.w = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 56: mad r1.y, cb0[11].y, r0.y, r0.w
    r1.y = ((source[11].yyyy)*(r0.yyyy)+(r0.wwww)).y;
    // 57: mad r2.y, cb0[7].y, r0.y, r0.w
    r2.y = ((source[7].yyyy)*(r0.yyyy)+(r0.wwww)).y;
    // 58: mad r0.yw, r0.zzzz, cb0[8].wwww, r2.xxxy
    r0.yw = ((r0.zzzz)*(source[8].wwww)+(r2.xxxy)).yw;
    // 59: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.ywyy, t4.xyzw, s1, cb0[5].x
    r2.xyz = (ArtistNativeSample0((r0.ywyy).xy, (source[5].xxxx).x, true).xyzw).xyz;
    // 60: mad r0.yw, r0.zzzz, cb0[8].wwww, r1.xxxy
    r0.yw = ((r0.zzzz)*(source[8].wwww)+(r1.xxxy)).yw;
    // 61: mad r1.xy, r0.zzzz, cb0[8].wwww, v2.xyxx
    r1.xy = ((r0.zzzz)*(source[8].wwww)+(v2.xyxx)).xy;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s4, l(0.000000)
    r0.z = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t1.yxzw, s3, cb0[5].x
    r0.y = (ArtistNativeSample2((r0.ywyy).xy, (source[5].xxxx).x, true).yxzw).y;
    // 64: mul_sat r0.y, r0.y, cb0[11].z
    r0.y = (saturate((r0.yyyy)*(source[11].zzzz))).y;
    // 65: mov_sat r0.w, v4.y
    r0.w = (saturate(v4.yyyy)).w;
    // 66: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 68: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 69: mul_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)*(r0.yyyy))).y;
    // 70: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 71: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 72: mul r0.z, r0.z, cb0[11].w
    r0.z = ((r0.zzzz)*(source[11].wwww)).z;
    // 73: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 74: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 75: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 76: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 77: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 78: dp3 r0.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 79: add r0.yzw, -r2.xxyz, r0.yyyy
    r0.yzw = ((-(r2.xxyz))+(r0.yyyy)).yzw;
    // 80: mad r0.yzw, cb0[9].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[9].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 81: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 82: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 83: mul r0.yzw, r0.yyzw, cb0[9].yyyy
    r0.yzw = ((r0.yyzw)*(source[9].yyyy)).yzw;
    // 84: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 85: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 86: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 87: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 88: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 89: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 90: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5344Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_i_pa_glow_01_ad: ff1194d8453ded4fba5fe87ac55b4348; selected map 76880f6eef92d922eb8d9e84ec7ec5d9972d9282f897cd088bc864f08680555f.
float4 ArtistNative5345(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 29: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 30: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 31: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5345Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
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
// fx_e_pa_fd_07_1_ad: 8f3418a0c1946f4da7b4be5ed4bac6d3; selected map c6d16a81ea3b83954a8566d0e42f723c0014b008807285ddbe8ddedccbd1ce76.
float4 ArtistNative5346(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].z = ((g_ArtistSourceMaterialParameters[1u].yyyy*float4(3.5, 0.0, 0.0, 0.0))).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 1: mul r0.xyzw, v2.xyxy, cb0[4].yyzz
    r0.xyzw = ((v2.xyxy)*(source[4].yyzz)).xyzw;
    // 2: mad r0.xy, cb0[3].yyyy, cb0[4].xxxx, r0.xyxx
    r0.xy = ((source[3].yyyy)*(source[4].xxxx)+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 6: mul r0.x, r0.x, cb0[4].w
    r0.x = ((r0.xxxx)*(source[4].wwww)).x;
    // 7: mul r0.yz, v2.xxyx, cb0[3].zzzz
    r0.yz = ((v2.xxyx)*(source[3].zzzz)).yz;
    // 8: mad r0.yz, cb0[3].yyyy, cb0[3].xxxx, r0.yyzy
    r0.yz = ((source[3].yyyy)*(source[3].xxxx)+(r0.yyzy)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.xwyz, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 10: mad r0.x, cb0[3].w, r0.y, r0.x
    r0.x = ((source[3].wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t3.xwyz, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 12: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 13: mul r0.xyzw, r0.xxxx, v3.wxyz
    r0.xyzw = ((r0.xxxx)*(v3.wxyz)).xyzw;
    // 14: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 15: mad r0.yzw, r1.xxyz, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r1.xxyz)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 16: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 17: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 18: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 19: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_floorstrm_14_ad: 00cd866295cc494dba563a698dea14df; selected map 25dad5f905785b3aa0b8fbe349a5a17d05495ab568518506e1e61c8b6a29d983.
float4 ArtistNative5347(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[6].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[7].zwzz
    r0.xy = ((v4.xyxx)*(source[7].zwzz)).xy;
    // 2: mul r0.z, cb0[6].x, cb0[6].y
    r0.z = ((source[6].xxxx)*(source[6].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[7].y, r0.x
    r1.x = ((r0.zzzz)*(source[7].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[8].x, r0.y
    r1.y = ((r0.zzzz)*(source[8].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    { const float4 sourceAngle = r0.zzzz; r2.x = (sin(sourceAngle)).x; r3.x = (cos(sourceAngle)).x; }
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
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5347Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0] = input.dynamicParameter;
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[2].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.x, cb0[2].x, cb0[2].y
    r0.x = ((source[2].xxxx)*(source[2].yyyy)).x;
    // 2: mul r0.yz, v2.xxyx, cb0[3].zzwz
    r0.yz = ((v2.xxyx)*(source[3].zzwz)).yz;
    // 3: mad r1.x, r0.x, cb0[3].y, r0.y
    r1.x = ((r0.xxxx)*(source[3].yyyy)+(r0.yyyy)).x;
    // 4: mad r1.y, r0.x, cb0[4].x, r0.z
    r1.y = ((r0.xxxx)*(source[4].xxxx)+(r0.zzzz)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[4].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[4].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 8: mad r0.z, cb0[2].y, cb0[5].w, cb0[6].x
    r0.z = ((source[2].yyyy)*(source[5].wwww)+(source[6].xxxx)).z;
    // 9: sincos r1.x, r2.x, r0.z
    { const float4 sourceAngle = r0.zzzz; r1.x = (sin(sourceAngle)).x; r2.x = (cos(sourceAngle)).x; }
    // 10: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 11: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 12: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 13: dp2 r0.z, r3.zyzz, r0.xyxx
    r0.z = (dot((r3.zyzz).xy,(r0.xyxx).xy).xxxx).z;
    // 14: dp2 r0.x, r3.yxyy, r0.xyxx
    r0.x = (dot((r3.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 15: mul r1.z, r0.z, cb0[1].y
    r1.z = ((r0.zzzz)*(source[1].yyyy)).z;
    // 16: add r0.y, cb0[0].y, l(-1.000000)
    r0.y = ((source[0].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 17: mad r1.x, r0.x, cb0[1].x, r0.y
    r1.x = ((r0.xxxx)*(source[1].xxxx)+(r0.yyyy)).x;
    // 18: add r0.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: dp3 r0.y, v4.xyzx, v4.xyzx
    r0.y = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).y;
    // 21: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 22: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 23: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 24: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: mul r0.z, r0.z, cb0[7].y
    r0.z = ((r0.zzzz)*(source[7].yyyy)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: mul_sat r0.z, r0.z, cb0[7].z
    r0.z = (saturate((r0.zzzz)*(source[7].zzzz))).z;
    // 28: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 29: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
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
// bfx_k_pa_note_01_3_ad: 8a847d27d9c6564da0c04625c71b7f4f; selected map ce809eebf1a33abe2c9c95c03814890a5c66f8e8d46d10e8e3b22245fd4d6e28.
float4 ArtistNative5348(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    // 1: mad r0.x, v2.x, l(0.500000), cb0[2].z
    r0.x = ((v2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).x;
    // 2: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 3: mad r0.z, v2.y, l(0.500000), cb0[3].y
    r0.z = ((v2.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].yyyy)).z;
    // 4: mul r0.y, r0.z, cb0[3].z
    r0.y = ((r0.zzzz)*(source[3].zzzz)).y;
    // 5: add r0.xy, r0.xyxx, l(0.300000, 0.200000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.300000,0.200000,0.000000,0.000000))).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 7: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 8: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 9: mul r0.xyz, r0.xyzx, cb0[3].wwww
    r0.xyz = ((r0.xyzx)*(source[3].wwww)).xyz;
    // 10: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[4].xxxx
    r1.xyz = ((r0.xyzx)*(source[4].xxxx)).xyz;
    // 12: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: mad r0.xyz, -cb0[4].xxxx, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[4].xxxx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[4].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 17: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 19: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 20: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 21: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 22: mul r0.xyz, r0.xyzx, cb0[4].wwww
    r0.xyz = ((r0.xyzx)*(source[4].wwww)).xyz;
    // 23: add r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)+(r0.xyzx)).xyz;
    // 24: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 26: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 27: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 28: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_k_pa_note_01_ad: 8a847d27d9c6564da0c04625c71b7f4f; selected map ce809eebf1a33abe2c9c95c03814890a5c66f8e8d46d10e8e3b22245fd4d6e28.
float4 ArtistNative5349(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    // 1: mad r0.x, v2.x, l(0.500000), cb0[2].z
    r0.x = ((v2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).x;
    // 2: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 3: mad r0.z, v2.y, l(0.500000), cb0[3].y
    r0.z = ((v2.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].yyyy)).z;
    // 4: mul r0.y, r0.z, cb0[3].z
    r0.y = ((r0.zzzz)*(source[3].zzzz)).y;
    // 5: add r0.xy, r0.xyxx, l(0.300000, 0.200000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.300000,0.200000,0.000000,0.000000))).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 7: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 8: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 9: mul r0.xyz, r0.xyzx, cb0[3].wwww
    r0.xyz = ((r0.xyzx)*(source[3].wwww)).xyz;
    // 10: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[4].xxxx
    r1.xyz = ((r0.xyzx)*(source[4].xxxx)).xyz;
    // 12: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: mad r0.xyz, -cb0[4].xxxx, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[4].xxxx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[4].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 17: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 19: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 20: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 21: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 22: mul r0.xyz, r0.xyzx, cb0[4].wwww
    r0.xyz = ((r0.xyzx)*(source[4].wwww)).xyz;
    // 23: add r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)+(r0.xyzx)).xyz;
    // 24: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 26: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 27: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 28: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_k_pa_note_01_2_ad: 8a847d27d9c6564da0c04625c71b7f4f; selected map ce809eebf1a33abe2c9c95c03814890a5c66f8e8d46d10e8e3b22245fd4d6e28.
float4 ArtistNative5350(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    // 1: mad r0.x, v2.x, l(0.500000), cb0[2].z
    r0.x = ((v2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).x;
    // 2: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 3: mad r0.z, v2.y, l(0.500000), cb0[3].y
    r0.z = ((v2.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].yyyy)).z;
    // 4: mul r0.y, r0.z, cb0[3].z
    r0.y = ((r0.zzzz)*(source[3].zzzz)).y;
    // 5: add r0.xy, r0.xyxx, l(0.300000, 0.200000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.300000,0.200000,0.000000,0.000000))).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 7: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 8: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 9: mul r0.xyz, r0.xyzx, cb0[3].wwww
    r0.xyz = ((r0.xyzx)*(source[3].wwww)).xyz;
    // 10: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[4].xxxx
    r1.xyz = ((r0.xyzx)*(source[4].xxxx)).xyz;
    // 12: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: mad r0.xyz, -cb0[4].xxxx, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[4].xxxx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[4].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 17: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 19: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 20: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 21: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 22: mul r0.xyz, r0.xyzx, cb0[4].wwww
    r0.xyz = ((r0.xyzx)*(source[4].wwww)).xyz;
    // 23: add r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)+(r0.xyzx)).xyz;
    // 24: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 26: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 27: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 28: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_me_dissolve_01_01_ma: 18eec9fc58cc074daddbffa5f2dd376e; selected map 13a209d38e75f2c4884e436a82bbe1cb96af141750f4699d1a3feafbe50c978a.
float4 ArtistNative5351(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].xxxx,g_ArtistSourceMaterialParameters[0u].yyyy,1u);
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[5] = g_ArtistSourceMaterialParameters[3u];
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mov_sat r0.x, cb0[0].w
    r0.x = (saturate(source[0].wwww)).x;
    // 2: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 3: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 4: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 7: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 8: mul r0.xy, r0.xxxx, v5.xyxx
    r0.xy = ((r0.xxxx)*(v5.xyxx)).xy;
    // 9: mul r0.xy, -r0.xyxx, cb0[2].xyxx
    r0.xy = ((-(r0.xyxx))*(source[2].xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 12: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 13: mad r0.xyz, cb0[6].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[6].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 14: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 15: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, cb0[6].wwww
    r0.xyz = ((r0.xyzx)*(source[6].wwww)).xyz;
    // 17: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 18: mul r1.xy, v4.xyxx, cb0[4].xyxx
    r1.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 22: mad r1.xyz, cb0[7].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[7].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 23: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 24: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 25: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 26: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 27: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 28: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 29: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 30: mad r0.xyz, r0.xyzx, r2.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 31: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 32: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_cd_02_2_tr: 1dee627450623d4699684df94a092879; selected map fd09d853c3414943101dab6dece30cab598b55cc307a3d1aa0d399e29220d944.
float4 ArtistNative5352(ARTIST_NATIVE_INPUT input)
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5352Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_h_pa_smoke_01_tr: 1166c98a352b934594aa881b9923e0f3; selected map c2d8d01196bc1ad3c52eed22b3401991f762707ca1c0e1467fc60f3a974cf0b1.
float4 ArtistNative5353(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_01_02_tr: 653a5bb9cb1a754193f542b0fda91232; selected map b9a00a284058a01b21ea0d6fd6eec705cebc4a6f9f17beb3a1ba17da3210361f.
float4 ArtistNative5354(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_energyray_01_ad: e1d6856d3f002648ab2d707325ce2a15; selected map 618a8197ac8dddbddffc07650e6822e1bd53aa94fa38eae09ff35d2325883c12.
float4 ArtistNative5355(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 3: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 4: div r0.zw, r0.zzzw, r1.xxxx
    r0.zw = ((r0.zzzw)/(r1.xxxx)).zw;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 7: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mad r1.xy, |r0.zwzz|, l(-0.018729, -0.018729, 0.000000, 0.000000), l(0.074261, 0.074261, 0.000000, 0.000000)
    r1.xy = ((abs(r0.zwzz))*(float4(-0.018729,-0.018729,0.000000,0.000000))+(float4(0.074261,0.074261,0.000000,0.000000))).xy;
    // 9: mad r1.xy, r1.xyxx, |r0.zwzz|, l(-0.212114, -0.212114, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(abs(r0.zwzz))+(float4(-0.212114,-0.212114,0.000000,0.000000))).xy;
    // 10: mad r1.xy, r1.xyxx, |r0.zwzz|, l(1.570729, 1.570729, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(abs(r0.zwzz))+(float4(1.570729,1.570729,0.000000,0.000000))).xy;
    // 11: add r1.zw, -|r0.zzzw|, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(abs(r0.zzzw)))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 12: lt r0.yz, r0.zzwz, -r0.zzwz
    r0.yz = (asfloat((uint4)((r0.zzwz)<(-(r0.zzwz))) * 0xffffffffu)).yz;
    // 13: sqrt r1.zw, r1.zzzw
    r1.zw = (sqrt(r1.zzzw)).zw;
    // 14: mul r2.xy, r1.zwzz, r1.xyxx
    r2.xy = ((r1.zwzz)*(r1.xyxx)).xy;
    // 15: mad r2.xy, r2.xyxx, l(-2.000000, -2.000000, 0.000000, 0.000000), l(3.141593, 3.141593, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(-2.000000,-2.000000,0.000000,0.000000))+(float4(3.141593,3.141593,0.000000,0.000000))).xy;
    // 16: and r0.yz, r0.yyzy, r2.xxyx
    r0.yz = (asfloat(asuint(r0.yyzy) & asuint(r2.xxyx))).yz;
    // 17: mad r0.yz, r1.xxyx, r1.zzwz, r0.yyzy
    r0.yz = ((r1.xxyx)*(r1.zzwz)+(r0.yyzy)).yz;
    // 18: mul r0.yz, r0.yyzy, l(0.000000, 0.318310, 0.318310, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.318310,0.318310,0.000000))).yz;
    // 19: mul r0.w, v4.y, cb0[2].x
    r0.w = ((v4.yyyy)*(source[2].xxxx)).w;
    // 20: mad r0.yz, r0.wwww, l(0.000000, 0.100000, 0.250000, 0.000000), r0.yyzy
    r0.yz = ((r0.wwww)*(float4(0.000000,0.100000,0.250000,0.000000))+(r0.yyzy)).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 22: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 23: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 24: mul r1.y, r1.x, v4.x
    r1.y = ((r1.xxxx)*(v4.xxxx)).y;
    // 25: mul r1.xy, r1.xyxx, l(20.000000, 120.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(20.000000,120.000000,0.000000,0.000000))).xy;
    // 26: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 27: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 28: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 30: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 31: movc r1.x, r0.x, l(0), r1.x
    r1.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 32: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 33: movc r0.x, r0.x, l(0), r1.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 34: add_sat r0.xyz, r0.xxxx, r0.yzwy
    r0.xyz = (saturate((r0.xxxx)+(r0.yzwy))).xyz;
    // 35: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 36: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 37: mul o0.xyz, r1.xxxx, r0.xyzx
    output.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 38: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_i_pa_shockwave_07_ad: 8238a1a9b601624d808980b887eeacac; selected map 956768caed8befb013af997f30e867ea8f2b0fcfa5cfa8b3b3f56a89f89aed97.
float4 ArtistNative5356(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[11u];
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = g_ArtistSourceMaterialParameters[10u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].yyyy,g_ArtistSourceMaterialParameters[6u].zzzz,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
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
    r0.xy = (ArtistNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
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
    r3.xyz = (ArtistNativeSample3((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
    r0.z = (ArtistNativeSample4((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
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
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5356Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[9u];
    source[1] = g_ArtistSourceMaterialParameters[10u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].yyyy,g_ArtistSourceMaterialParameters[6u].zzzz,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[11].x = ((float4(0.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 29: mul r0.yz, r0.yyzy, cb0[3].xxyx
    r0.yz = ((r0.yyzy)*(source[3].xxyx)).yz;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r1.x, r0.y, cb0[3].z
    r1.x = ((r0.yyyy)+(source[3].zzzz)).x;
    // 32: mul r2.x, r1.x, cb0[6].w
    r2.x = ((r1.xxxx)*(source[6].wwww)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 36: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 37: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 38: mul r0.y, r0.y, v3.y
    r0.y = ((r0.yyyy)*(v3.yyyy)).y;
    // 39: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 40: add r0.y, v3.x, cb0[4].x
    r0.y = ((v3.xxxx)+(source[4].xxxx)).y;
    // 41: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 42: mul r0.xz, r1.xxyx, cb0[5].xxyx
    r0.xz = ((r1.xxyx)*(source[5].xxyx)).xz;
    // 43: mad r0.y, cb0[5].z, v3.y, r0.z
    r0.y = ((source[5].zzzz)*(v3.yyyy)+(r0.zzzz)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s2, l(-1.000000)
    r0.xy = (ArtistNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 45: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 46: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 47: mul r0.xy, r0.xyxx, cb0[5].wwww
    r0.xy = ((r0.xyxx)*(source[5].wwww)).xy;
    // 48: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 49: mad r2.y, r1.y, cb0[7].x, cb0[7].y
    r2.y = ((r1.yyyy)*(source[7].xxxx)+(source[7].yyyy)).y;
    // 50: mad r0.zw, cb0[6].xxxx, r0.xxxy, r2.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r2.xxxy)).zw;
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
    // 57: mul r2.xyz, r2.xyzx, cb0[7].zzzz
    r2.xyz = ((r2.xyzx)*(source[7].zzzz)).xyz;
    // 58: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 59: mul r3.xyz, r2.xyzx, cb0[7].wwww
    r3.xyz = ((r2.xyzx)*(source[7].wwww)).xyz;
    // 60: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 61: mad r2.xyzw, -cb0[7].wwww, r2.xyxy, r0.zzzz
    r2.xyzw = ((-(source[7].wwww))*(r2.xyxy)+(r0.zzzz)).xyzw;
    // 62: mad r2.xyzw, cb0[8].xxxx, r2.xyzw, r3.xyxy
    r2.xyzw = ((source[8].xxxx)*(r2.xyzw)+(r3.xyxy)).xyzw;
    // 63: mul r2.xyzw, r2.xyzw, cb0[0].xyxy
    r2.xyzw = ((r2.xyzw)*(source[0].xyxy)).xyzw;
    // 64: mul r3.x, r1.x, cb0[8].y
    r3.x = ((r1.xxxx)*(source[8].yyyy)).x;
    // 65: mad r3.y, r1.y, cb0[8].z, cb0[8].w
    r3.y = ((r1.yyyy)*(source[8].zzzz)+(source[8].wwww)).y;
    // 66: mad r0.zw, cb0[6].xxxx, r0.xxxy, r3.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r3.xxxy)).zw;
    // 67: sample_l_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t3.xyzw, s4, l(-1.000000)
    r3.xyz = (ArtistNativeSample3((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
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
    // 73: mul r3.xyz, r3.xyzx, cb0[9].xxxx
    r3.xyz = ((r3.xyzx)*(source[9].xxxx)).xyz;
    // 74: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 75: mul r4.xyz, r3.xyzx, cb0[9].yyyy
    r4.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 76: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 77: mad r3.xyzw, -cb0[9].yyyy, r3.xyxy, r0.zzzz
    r3.xyzw = ((-(source[9].yyyy))*(r3.xyxy)+(r0.zzzz)).xyzw;
    // 78: mad r3.xyzw, cb0[9].zzzz, r3.xyzw, r4.xyxy
    r3.xyzw = ((source[9].zzzz)*(r3.xyzw)+(r4.xyxy)).xyzw;
    // 79: mul r3.xyzw, r3.xyzw, cb0[1].xyxy
    r3.xyzw = ((r3.xyzw)*(source[1].xyxy)).xyzw;
    // 80: mul r2.xyzw, r2.xyzw, r3.xyzw
    r2.xyzw = ((r2.xyzw)*(r3.xyzw)).xyzw;
    // 81: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[2].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[2].xxxy)).zw;
    // 82: mul r1.xz, r1.xxyx, cb0[4].yyzy
    r1.xz = ((r1.xxyx)*(source[4].yyzy)).xz;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t4.yzxw, s5, l(-1.000000)
    r0.z = (ArtistNativeSample4((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 84: mul r2.xyzw, r2.xyzw, r0.zzzz
    r2.xyzw = ((r2.xyzw)*(r0.zzzz)).xyzw;
    // 85: mul r3.xyzw, r2.xyzw, cb0[10].yyyy
    r3.xyzw = ((r2.xyzw)*(source[10].yyyy)).xyzw;
    // 86: max r3.xyzw, |r3.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r3.xyzw = (max(abs(r3.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 87: log r3.xyzw, r3.xyzw
    r3.xyzw = (log2(r3.xyzw)).xyzw;
    // 88: mul r3.xyzw, r3.xyzw, cb0[10].zzzz
    r3.xyzw = ((r3.xyzw)*(source[10].zzzz)).xyzw;
    // 89: exp r3.xyzw, r3.xyzw
    r3.xyzw = (exp2(r3.xyzw)).xyzw;
    // 90: mad r1.y, cb0[4].w, v3.y, r1.z
    r1.y = ((source[4].wwww)*(v3.yyyy)+(r1.zzzz)).y;
    // 91: mad r0.xy, cb0[6].xxxx, r0.xyxx, r1.xyxx
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r1.xyxx)).xy;
    // 92: mul r0.xy, r0.xyxx, l(1.500000, 0.250000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(1.500000,0.250000,0.000000,0.000000))).xy;
    // 93: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(-1.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 94: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 95: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 96: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 97: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 98: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 99: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 100: mad r0.xyzw, r0.xxxx, r2.zwzw, r3.xyzw
    r0.xyzw = ((r0.xxxx)*(r2.zwzw)+(r3.xyzw)).xyzw;
    // 101: mul r0.xyzw, r0.xyzw, cb0[11].xxxx
    r0.xyzw = ((r0.xyzw)*(source[11].xxxx)).xyzw;
    // 102: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 103: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 104: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 105: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 106: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 107: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 108: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 109: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 110: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 111: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 112: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 113: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 114: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 115: source device depth mapped to centimetre view depth; reconstruction at 117.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 117-120: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 121: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 122: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 123: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 124: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 125: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_i_pa_ribbon_05_ad: d0606927ec71cd44b2536fdec23ebaf8; selected map 3fa905da6aac18d94e1c1f0e251f352c9836f068792a63ac255b92141fdc19ff.
float4 ArtistNative5357(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-1.29999995, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-0.349999994, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].xxxx,g_ArtistSourceMaterialParameters[0u].yyyy,1u);
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
    // 1: mad r0.xy, v2.xyxx, l(0.500000, 2.000000, 0.000000, 0.000000), cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(float4(0.500000,2.000000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 2.000000, 0.000000), cb0[4].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,2.000000,0.000000))+(source[4].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: mul r0.z, r0.y, r0.x
    r0.z = ((r0.yyyy)*(r0.xxxx)).z;
    // 6: add r1.xy, v2.xyxx, cb0[5].xyxx
    r1.xy = ((v2.xyxx)+(source[5].xyxx)).xy;
    // 7: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.070000, 0.070000), r1.xxxy
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.070000,0.070000))+(r1.xxxy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: add r0.zw, v2.xxxy, cb0[2].xxxy
    r0.zw = ((v2.xxxy)+(source[2].xxxy)).zw;
    // 10: mad r0.xy, r0.xxxx, r0.yyyy, r0.zwzz
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(r0.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 13: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 14: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 15: mul o0.xyz, r0.xyzx, cb0[0].xxxx
    output.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 16: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5357Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v1.xyxx, l(0.100000, 0.500000, 0.000000, 0.000000), cb0[0].xyxx
    r0.xy = ((v1.xyxx)*(float4(0.100000,0.500000,0.000000,0.000000))+(source[0].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v1.xxyx, l(0.000000, 0.150000, 1.000000, 0.000000), cb0[1].xxyx
    r0.yz = ((v1.xxyx)*(float4(0.000000,0.150000,1.000000,0.000000))+(source[1].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: mad r0.xy, r0.xxxx, l(0.060000, 0.060000, 0.000000, 0.000000), v1.xyxx
    r0.xy = ((r0.xxxx)*(float4(0.060000,0.060000,0.000000,0.000000))+(v1.xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 8: mul r0.xyzw, r0.xyxy, v2.wwww
    r0.xyzw = ((r0.xyxy)*(v2.wwww)).xyzw;
    // 9: mad r0.xyzw, r0.xyzw, l(80.000000, -50.000000, 80.000000, -50.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(80.000000,-50.000000,80.000000,-50.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 10: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 11: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 12: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 13: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 14: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 15: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 16: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 17: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 18: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 19: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 20: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 21: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 22: source device depth mapped to centimetre view depth; reconstruction at 24.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 24-27: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 28: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 29: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 30: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 31: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 32: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_shockwave_03_ad: 9f7d04fd3c6b0e448fa52dafeb5d5d43; selected map 10da5aa7496658c84693cc64ba5144d47c39f3f9e298e1dca0ff00adbeb7516f.
float4 ArtistNative5358(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].x = (((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[7].y = (((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
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
    // 28: mad r2.x, cb0[7].z, r1.x, cb0[3].x
    r2.x = ((source[7].zzzz)*(r1.xxxx)+(source[3].xxxx)).x;
    // 29: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 30: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 31: mul r0.yz, r0.yyyy, v4.wwzw
    r0.yz = ((r0.yyyy)*(v4.wwzw)).yz;
    // 32: exp r0.yz, r0.yyzy
    r0.yz = (exp2(r0.yyzy)).yz;
    // 33: lt r0.w, r0.x, l(0.000000)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 34: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 36: movc r1.yz, r0.wwww, l(0,0,0,0), r0.yyzy
    r1.yz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyzy)).yz;
    // 37: mul r0.yz, r1.xxzx, cb0[5].zzwz
    r0.yz = ((r1.xxzx)*(source[5].zzwz)).yz;
    // 38: mul r1.xw, r1.xxxz, cb0[4].zzzw
    r1.xw = ((r1.xxxz)*(source[4].zzzw)).xw;
    // 39: mad r3.x, cb0[4].y, cb0[5].y, r0.y
    r3.x = ((source[4].yyyy)*(source[5].yyyy)+(r0.yyyy)).x;
    // 40: mad r3.y, cb0[4].y, cb0[6].x, r0.z
    r3.y = ((source[4].yyyy)*(source[6].xxxx)+(r0.zzzz)).y;
    // 41: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r3.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 42: mul r0.z, r1.z, cb0[7].w
    r0.z = ((r1.zzzz)*(source[7].wwww)).z;
    // 43: mad r2.y, cb0[4].y, cb0[8].z, r0.z
    r2.y = ((source[4].yyyy)*(source[8].zzzz)+(r0.zzzz)).y;
    // 44: mad r0.zw, r0.yyyy, cb0[6].yyyy, r2.xxxy
    r0.zw = ((r0.yyyy)*(source[6].yyyy)+(r2.xxxy)).zw;
    // 45: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
    // 46: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 47: add r0.w, -v4.x, l(1.000000)
    r0.w = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mad r0.z, r0.z, r0.x, -r0.w
    r0.z = ((r0.zzzz)*(r0.xxxx)+(-(r0.wwww))).z;
    // 49: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 50: mul_sat r0.x, r0.x, cb0[8].w
    r0.x = (saturate((r0.xxxx)*(source[8].wwww))).x;
    // 51: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 52: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 53: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 54: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 55: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 56: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 57: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 58: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 59: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 60: mad r1.x, cb0[4].y, cb0[4].x, r1.x
    r1.x = ((source[4].yyyy)*(source[4].xxxx)+(r1.xxxx)).x;
    // 61: mad r1.y, cb0[4].y, cb0[5].x, r1.w
    r1.y = ((source[4].yyyy)*(source[5].xxxx)+(r1.wwww)).y;
    // 62: mad r0.yz, r0.yyyy, cb0[6].yyyy, r1.xxyx
    r0.yz = ((r0.yyyy)*(source[6].yyyy)+(r1.xxyx)).yz;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
    // 64: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 65: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 66: mad r0.yzw, cb0[6].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[6].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 67: mul r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[2].xxyz)).yzw;
    // 68: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 69: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 70: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 71: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_windwv_01_5_ad: 34883f9933fdac4894cd0c67f6ee266b; selected map 418e3b7d404571144ac149f5f99284d01506cf1f250a55ae1c0666b0a566dd13.
float4 ArtistNative5359(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 32: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 33: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 34: movc r1.y, r0.x, l(0), r0.y
    r1.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 35: mul r0.xy, r1.xyxx, cb0[5].yzyy
    r0.xy = ((r1.xyxx)*(source[5].yzyy)).xy;
    // 36: mul r0.z, r1.x, cb0[3].w
    r0.z = ((r1.xxxx)*(source[3].wwww)).z;
    // 37: mad r2.x, cb0[3].z, cb0[3].y, r0.z
    r2.x = ((source[3].zzzz)*(source[3].yyyy)+(r0.zzzz)).x;
    // 38: mul r0.z, r1.y, cb0[4].x
    r0.z = ((r1.yyyy)*(source[4].xxxx)).z;
    // 39: mad r2.y, cb0[3].z, cb0[4].y, r0.z
    r2.y = ((source[3].zzzz)*(source[4].yyyy)+(r0.zzzz)).y;
    // 40: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s0, cb0[3].x
    r1.xyz = (ArtistNativeSample0((r2.xyxx).xy, (source[3].xxxx).x, true).xyzw).xyz;
    // 41: mad r0.xy, cb0[3].zzzz, cb0[5].xwxx, r0.xyxx
    r0.xy = ((source[3].zzzz)*(source[5].xwxx)+(r0.xyxx)).xy;
    // 42: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, cb0[3].x
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (source[3].xxxx).x, true).xyzw).x;
    // 43: mul_sat r0.x, r0.x, cb0[6].x
    r0.x = (saturate((r0.xxxx)*(source[6].xxxx))).x;
    // 44: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 45: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 46: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 47: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 49: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 50: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 51: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 52: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 53: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 54: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 55: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 56: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 57: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 58: add r0.yzw, -r1.xxyz, r0.yyyy
    r0.yzw = ((-(r1.xxyz))+(r0.yyyy)).yzw;
    // 59: mad r0.yzw, cb0[4].zzzz, r0.yyzw, r1.xxyz
    r0.yzw = ((source[4].zzzz)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 60: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 61: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 62: mul r0.yzw, r0.yyzw, cb0[4].wwww
    r0.yzw = ((r0.yyzw)*(source[4].wwww)).yzw;
    // 63: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 64: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 65: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 66: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 67: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 68: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 69: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5359Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_ring_07_25_ad: b35a09a584c8f141b96670a072707144; selected map 4f918464fc213e449baf0b2339006e4135575e38b327f32a1bf2f304146737c2.
float4 ArtistNative5360(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    r0.z = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r0.x = (ArtistNativeSample2((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.ywyy, t1.wxyz, s0, cb0[2].x
    r0.yzw = (ArtistNativeSample0((r0.ywyy).xy, (source[2].xxxx).x, true).wxyz).yzw;
    // 48: mul_sat r1.x, r0.y, cb0[5].x
    r1.x = (saturate((r0.yyyy)*(source[5].xxxx))).x;
    // 49: mov_sat r1.y, v4.y
    r1.y = (saturate(v4.yyyy)).y;
    // 50: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 52: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 53: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 54: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 55: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 57: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 58: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 59: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 60: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 61: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 62: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 63: mad r0.yzw, cb0[4].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 64: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 65: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 66: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 67: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5360Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_d_pa_circ_01_01_dt_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 ArtistNative5361(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[5].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].xxxx))).x;
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
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_line_01_07_tr: f61dc52d23e2f248b59a846850aaa5af; selected map f29bf3b1db2f816811ca541e7d155b3360477e6ab8e26700256edc979c6b5b51.
float4 ArtistNative5362(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = input.dynamicParameter;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[7].y = (cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: max r0.x, |r1.y|, |r1.x|
    r0.x = (max(abs(r1.yyyy),abs(r1.xxxx))).x;
    // 5: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 6: min r0.y, |r1.y|, |r1.x|
    r0.y = (min(abs(r1.yyyy),abs(r1.xxxx))).y;
    // 7: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 8: mul r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)*(r0.xxxx)).y;
    // 9: mad r0.z, r0.y, l(0.020835), l(-0.085133)
    r0.z = ((r0.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 10: mad r0.z, r0.y, r0.z, l(0.180141)
    r0.z = ((r0.yyyy)*(r0.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 11: mad r0.z, r0.y, r0.z, l(-0.330299)
    r0.z = ((r0.yyyy)*(r0.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 12: mad r0.y, r0.y, r0.z, l(0.999866)
    r0.y = ((r0.yyyy)*(r0.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 13: mul r0.z, r0.y, r0.x
    r0.z = ((r0.yyyy)*(r0.xxxx)).z;
    // 14: mad r0.z, r0.z, l(-2.000000), l(1.570796)
    r0.z = ((r0.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 15: lt r0.w, |r1.y|, |r1.x|
    r0.w = (asfloat((uint4)((abs(r1.yyyy))<(abs(r1.xxxx))) * 0xffffffffu)).w;
    // 16: and r0.z, r0.w, r0.z
    r0.z = (asfloat(asuint(r0.wwww) & asuint(r0.zzzz))).z;
    // 17: mad r0.x, r0.x, r0.y, r0.z
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.zzzz)).x;
    // 18: lt r0.y, r1.y, -r1.y
    r0.y = (asfloat((uint4)((r1.yyyy)<(-(r1.yyyy))) * 0xffffffffu)).y;
    // 19: and r0.y, r0.y, l(0xc0490fdb)
    r0.y = (asfloat(asuint(r0.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 20: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 21: min r0.y, r1.y, r1.x
    r0.y = (min(r1.yyyy,r1.xxxx)).y;
    // 22: lt r0.y, r0.y, -r0.y
    r0.y = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).y;
    // 23: max r0.z, r1.y, r1.x
    r0.z = (max(r1.yyyy,r1.xxxx)).z;
    // 24: add r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)+(r1.xyxx)).xy;
    // 25: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 26: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 28: ge r0.z, r0.z, -r0.z
    r0.z = (asfloat((uint4)((r0.zzzz)>=(-(r0.zzzz))) * 0xffffffffu)).z;
    // 29: and r0.y, r0.z, r0.y
    r0.y = (asfloat(asuint(r0.zzzz) & asuint(r0.yyyy))).y;
    // 30: movc r0.x, r0.y, -r0.x, r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (-(r0.xxxx)) : (r0.xxxx)).x;
    // 31: mad r1.x, r0.x, l(0.159155), l(0.500000)
    r1.x = ((r0.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 32: mul r0.xy, r1.xyxx, cb0[6].zwzz
    r0.xy = ((r1.xyxx)*(source[6].zwzz)).xy;
    // 33: mad r2.x, cb0[6].y, cb0[6].x, r0.x
    r2.x = ((source[6].yyyy)*(source[6].xxxx)+(r0.xxxx)).x;
    // 34: mad r2.y, cb0[6].y, cb0[7].z, r0.y
    r2.y = ((source[6].yyyy)*(source[7].zzzz)+(r0.yyyy)).y;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r2.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 36: mul r1.zw, r1.xxxy, cb0[8].xxxy
    r1.zw = ((r1.xxxy)*(source[8].xxxy)).zw;
    // 37: add r0.w, r1.y, l(-0.500000)
    r0.w = ((r1.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 38: mad_sat r2.y, r0.w, cb0[5].y, l(0.500000)
    r2.y = (saturate((r0.wwww)*(source[5].yyyy)+(float4(0.500000,0.500000,0.500000,0.500000)))).y;
    // 39: mad r3.x, cb0[6].y, cb0[7].w, r1.z
    r3.x = ((source[6].yyyy)*(source[7].wwww)+(r1.zzzz)).x;
    // 40: mad r3.y, cb0[6].y, cb0[8].z, r1.w
    r3.y = ((source[6].yyyy)*(source[8].zzzz)+(r1.wwww)).y;
    // 41: sample_l_indexable(texture2d)(float,float,float,float) r1.yzw, r3.xyxx, t2.wxyz, s1, l(0.000000)
    r1.yzw = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
    // 42: mul r3.xyz, r0.xyzx, r1.yzwy
    r3.xyz = ((r0.xyzx)*(r1.yzwy)).xyz;
    // 43: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: mad r0.xyz, -r0.xyzx, r1.yzwy, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(r1.yzwy)+(r0.wwww)).xyz;
    // 45: mad r0.xyz, cb0[8].wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((source[8].wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 46: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 47: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 48: mul r0.xyz, r0.xyzx, cb0[9].xxxx
    r0.xyz = ((r0.xyzx)*(source[9].xxxx)).xyz;
    // 49: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 50: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 51: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 52: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 53: mov_sat r0.xy, cb0[5].xzxx
    r0.xy = (saturate(source[5].xzxx)).xy;
    // 54: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 55: add r0.z, -r0.x, r1.x
    r0.z = ((-(r0.xxxx))+(r1.xxxx)).z;
    // 56: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: div r0.x, l(1.000100), r0.x
    r0.x = ((float4(1.000100,1.000100,1.000100,1.000100))/(r0.xxxx)).x;
    // 58: mul_sat r2.x, r0.x, r0.z
    r2.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 59: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t0.xyzw, s2, cb0[9].z
    r0.x = (ArtistNativeSample2((r2.xyxx).xy, (source[9].zzzz).x, true).xyzw).x;
    // 60: mul_sat r0.x, r0.x, cb0[9].w
    r0.x = (saturate((r0.xxxx)*(source[9].wwww))).x;
    // 61: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 62: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 63: mul r0.z, r0.z, cb0[10].x
    r0.z = ((r0.zzzz)*(source[10].xxxx)).z;
    // 64: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 65: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 66: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 67: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 68: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 69: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 70: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 71: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 72: mul r0.z, r0.z, cb0[10].y
    r0.z = ((r0.zzzz)*(source[10].yyyy)).z;
    // 73: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 74: mul_sat r0.z, r0.z, cb0[10].z
    r0.z = (saturate((r0.zzzz)*(source[10].zzzz))).z;
    // 75: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 76: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 77: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 78: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif
