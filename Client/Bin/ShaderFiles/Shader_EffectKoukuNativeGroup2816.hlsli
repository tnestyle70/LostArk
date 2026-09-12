// Original Kouku material programs 2816..2879; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_pa_fire_01_01_tr: b907311020f1bc488c0712cb83550f86; selected map 2ec3e916b008bf0ff43ede033e99db3867fc1484193381d5b664187d7234b6db.
float4 ArtistNative2816(ARTIST_NATIVE_INPUT input)
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
// bfx_j_pa_lightdust_01_1_tr: 8a5cc23116d92a4d894fa89a15d5c64f; selected map 6575f98d15daef884390d85ac13cadd021cdff94e5225e70729808a38c568908.
float4 ArtistNative2817(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 1: mov r0.xz, l(0,0,0,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 2: mov r0.yw, v4.zzzx
    r0.yw = (v4.zzzx).yw;
    // 3: add r0.xyzw, r0.xyzw, v2.xyxy
    r0.xyzw = ((r0.xyzw)+(v2.xyxy)).xyzw;
    // 4: mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 6: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 7: dp3 r0.z, r0.zzzz, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r0.zzzz).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 8: add r0.z, r0.z, l(-1.000000)
    r0.z = ((r0.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 9: mad r0.z, v4.w, r0.z, l(1.000000)
    r0.z = ((v4.wwww)*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: add r0.w, cb0[3].x, l(-1.000000)
    r0.w = ((source[3].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 11: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 12: mad r0.xy, cb0[3].xxxx, r0.xyxx, -r0.wwww
    r0.xy = ((source[3].xxxx)*(r0.xyxx)+(-(r0.wwww))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 14: log r0.x, |r1.w|
    r0.x = (log2(abs(r1.wwww))).x;
    // 15: mul r0.x, r0.x, cb0[3].w
    r0.x = ((r0.xxxx)*(source[3].wwww)).x;
    // 16: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 17: lt r0.y, |r1.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 19: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 20: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 21: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 22: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 23: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 24: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 25: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 27: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 28: mul r0.xyz, r1.xyzx, cb0[3].yyyy
    r0.xyz = ((r1.xyzx)*(source[3].yyyy)).xyz;
    // 29: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: mad r1.xyz, -cb0[3].yyyy, r1.xyzx, r0.wwww
    r1.xyz = ((-(source[3].yyyy))*(r1.xyzx)+(r0.wwww)).xyz;
    // 31: mad r0.xyz, cb0[3].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[3].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 32: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 33: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// mn_cngn_00_mi: 2b41020e9d64484f9b0c844745af2143; selected map 11fefd5ba345d1bf97d044fe0da42d1e6808d2d83268f243e51ccebdb0f22619.
float4 ArtistNative2818(ARTIST_NATIVE_INPUT input)
{
    float4 source[32]; [unroll] for (uint i=0u; i<32u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=0.f; // Absolute source world position needs no pre-view translation.
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[2]=float4(input.sourceActorPosition,0.f); // Original actor-position pulse seed.
    source[29]=float4(input.skyUpperColor,0.f);
    source[30]=float4(input.skyLowerColor,0.f);
    source[31]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[19u];
    source[4] = g_ArtistSourceMaterialParameters[20u];
    source[5] = g_ArtistSourceMaterialParameters[9u];
    source[6] = g_ArtistSourceMaterialParameters[10u];
    source[7] = g_ArtistSourceMaterialParameters[11u];
    source[8] = g_ArtistSourceMaterialParameters[12u];
    source[9] = g_ArtistSourceMaterialParameters[15u];
    source[10] = g_ArtistSourceMaterialParameters[14u];
    source[11] = g_ArtistSourceMaterialParameters[17u];
    source[12] = g_ArtistSourceMaterialParameters[18u];
    source[13] = g_ArtistSourceMaterialParameters[13u];
    source[14] = g_ArtistSourceMaterialParameters[22u];
    source[15] = g_ArtistSourceMaterialParameters[7u];
    source[16] = g_ArtistSourceMaterialParameters[8u];
    source[17] = g_ArtistSourceMaterialParameters[16u];
    source[18] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[19] = g_ArtistSourceMaterialParameters[21u];
    source[20] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[21] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[22].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[22].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[22].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[22].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[23].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[23].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[23].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[23].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[24].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[24].y = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[24].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[24].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[25].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[25].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[25].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[25].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[26].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[26].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[26].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[26].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[27].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[27].y = ((g_ArtistSourceMaterialParameters[0u].wwww*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[27].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[27].w = ((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[28].x = (((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[28].y = (sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[28].z = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[28].w = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r0.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 4: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 8: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 9: add r1.x, -r0.x, l(1000.000000)
    r1.x = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 10: mad r0.x, cb0[26].w, r1.x, r0.x
    r0.x = ((source[26].wwww)*(r1.xxxx)+(r0.xxxx)).x;
    // 11: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 12: mad r0.x, cb0[26].y, cb0[26].z, r0.x
    r0.x = ((source[26].yyyy)*(source[26].zzzz)+(r0.xxxx)).x;
    // 13: mul r1.x, r0.x, l(3.524534)
    r1.x = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 14: sincos null, r1.x, r1.x
    r1.x = (cos(r1.xxxx)).x;
    // 15: add r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 16: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 17: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 18: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 19: mad r0.x, r0.x, l(0.500000), cb0[26].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[26].xxxx)).x;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r1.xyz = (ArtistNativeSample5((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mul r2.xyz, cb0[13].xyzx, cb0[25].wwww
    r2.xyz = ((source[13].xyzx)*(source[25].wwww)).xyz;
    // 22: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 24: mul r2.xyz, v7.yyyy, cb1[1].xywx
    r2.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 25: mad r2.xyz, cb1[0].xywx, v7.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v7.xxxx)+(r2.xyzx)).xyz;
    // 26: mad r2.xyz, cb1[2].xywx, v7.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v7.zzzz)+(r2.xyzx)).xyz;
    // 27: mad r2.xyz, cb1[3].xywx, v7.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v7.wwww)+(r2.xyzx)).xyz;
    // 28: div r2.xy, r2.xyxx, r2.zzzz
    r2.xy = ((r2.xyxx)/(r2.zzzz)).xy;
    // 29: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: mul r2.xy, r2.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 31: deriv_rtx_coarse r2.zw, r2.xxxy
    r2.zw = (ddx_coarse(r2.xxxy)).zw;
    // 32: deriv_rty_coarse r2.xy, r2.xyxx
    r2.xy = (ddy_coarse(r2.xyxx)).xy;
    // 33: dp2 r0.x, r2.xyxx, r2.xyxx
    r0.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 34: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 35: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 36: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 37: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 38: rcp r1.w, |r0.x|
    r1.w = (1.0/(abs(r0.xxxx))).w;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 40: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: log r3.x, |r2.w|
    r3.x = (log2(abs(r2.wwww))).x;
    // 42: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 43: mul r3.x, r3.x, cb0[23].x
    r3.x = ((r3.xxxx)*(source[23].xxxx)).x;
    // 44: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 45: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: movc r2.w, r2.w, l(0), r3.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 47: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 48: mul r3.x, r3.x, cb0[23].y
    r3.x = ((r3.xxxx)*(source[23].yyyy)).x;
    // 49: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: add r0.x, |r0.x|, r1.w
    r0.x = ((abs(r0.xxxx))+(r1.wwww)).x;
    // 52: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 54: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 55: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 56: mul r3.xy, r3.xyxx, cb0[22].xxxx
    r3.xy = ((r3.xyxx)*(source[22].xxxx)).xy;
    // 57: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 59: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 60: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 61: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r4.xyz, cb0[22].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[22].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 63: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 64: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 65: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 66: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 67: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 68: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 69: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 70: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 71: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 72: mul r7.xyz, r1.wwww, v1.xyzx
    r7.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 73: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 74: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 75: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 76: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 77: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 78: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 79: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 80: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 81: mad r9.xyz, v5.xyzx, r1.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r1.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 82: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 83: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 84: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 85: dp3 r1.w, r6.xyzx, r10.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 86: mul r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 87: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 88: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 89: dp2 r1.w, r6.ywyy, r6.ywyy
    r1.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 90: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 91: div r6.xy, r6.ywyy, r1.wwww
    r6.xy = ((r6.ywyy)/(r1.wwww)).xy;
    // 92: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 93: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 95: mad r6.xy, r1.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, r0.x
    r6.xyz = (ArtistNativeSample4((r6.xyxx).xy, (r0.xxxx).x, true).xyzw).xyz;
    // 97: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 98: rcp r0.x, cb0[23].z
    r0.x = (1.0/(source[23].zzzz)).x;
    // 99: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 100: mul r10.xyz, r10.xyzx, cb0[23].zzzz
    r10.xyz = ((r10.xyzx)*(source[23].zzzz)).xyz;
    // 101: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 102: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 103: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 104: mad r10.xyz, r10.xyzx, cb0[23].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[23].zzzz)+(r11.xyzx)).xyz;
    // 105: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 106: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 107: add r0.x, cb0[23].z, l(1.000000)
    r0.x = ((source[23].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 109: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r6.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r6.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 111: mad r6.xyz, r3.wwww, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[11].xyzx)).xyz;
    // 112: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 113: mul r6.xyz, r6.xyzx, cb0[23].wwww
    r6.xyz = ((r6.xyzx)*(source[23].wwww)).xyz;
    // 114: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 115: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 116: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 117: dp3 r0.x, r3.xyzx, r4.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 118: mul_sat r1.w, r0.x, cb0[24].y
    r1.w = (saturate((r0.xxxx)*(source[24].yyyy))).w;
    // 119: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 120: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul_sat r3.w, r4.z, cb0[24].y
    r3.w = (saturate((r4.zzzz)*(source[24].yyyy))).w;
    // 122: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add_sat r3.w, r3.w, -cb0[24].z
    r3.w = (saturate((r3.wwww)+(-(source[24].zzzz)))).w;
    // 124: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 125: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 126: mul r4.w, r4.w, cb0[24].w
    r4.w = ((r4.wwww)*(source[24].wwww)).w;
    // 127: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 128: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 129: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 130: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 132: mad r2.xyz, cb0[22].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 133: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 134: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 135: mad r2.xyz, cb0[22].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 136: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 138: mul r10.xyz, r10.xyzx, cb0[24].xxxx
    r10.xyz = ((r10.xyzx)*(source[24].xxxx)).xyz;
    // 139: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r11.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 140: add r3.w, r11.y, r11.x
    r3.w = ((r11.yyyy)+(r11.xxxx)).w;
    // 141: add r3.w, r11.z, r3.w
    r3.w = ((r11.zzzz)+(r3.wwww)).w;
    // 142: add_sat r3.w, r11.w, r3.w
    r3.w = (saturate((r11.wwww)+(r3.wwww))).w;
    // 143: mad r2.xyz, r3.wwww, r10.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 144: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 145: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 146: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 147: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 148: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 150: mul r3.w, r3.w, cb0[25].x
    r3.w = ((r3.wwww)*(source[25].xxxx)).w;
    // 151: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 152: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 155: div r4.w, cb0[25].y, r4.w
    r4.w = ((source[25].yyyy)/(r4.wwww)).w;
    // 156: mul r4.w, r1.w, r4.w
    r4.w = ((r1.wwww)*(r4.wwww)).w;
    // 157: mul r10.xyz, r6.xyzx, r4.wwww
    r10.xyz = ((r6.xyzx)*(r4.wwww)).xyz;
    // 158: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: add r12.xyz, -r0.yzwy, r4.wwww
    r12.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 160: mad r0.yzw, cb0[22].yyyy, r12.xxyz, r0.yyzw
    r0.yzw = ((source[22].yyyy)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 161: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r12.xyz, -r0.yzwy, r4.wwww
    r12.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 163: mad r0.yzw, cb0[22].zzzz, r12.xxyz, r0.yyzw
    r0.yzw = ((source[22].zzzz)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 164: mul r12.xyz, cb0[5].xyzx, cb0[5].wwww
    r12.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 165: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, -r12.xyzx
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r12.xyzx))).xyz;
    // 166: mad r12.xyz, r11.xxxx, r13.xyzx, r12.xyzx
    r12.xyz = ((r11.xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 167: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, -r12.xyzx
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r12.xyzx))).xyz;
    // 168: mad r11.xyw, r11.yyyy, r13.xyxz, r12.xyxz
    r11.xyw = ((r11.yyyy)*(r13.xyxz)+(r12.xyxz)).xyw;
    // 169: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, -r11.xywx
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r11.xywx))).xyz;
    // 170: mad r11.xyz, r11.zzzz, r12.xyzx, r11.xywx
    r11.xyz = ((r11.zzzz)*(r12.xyzx)+(r11.xywx)).xyz;
    // 171: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r12.xyz, -r11.xyzx, r4.wwww
    r12.xyz = ((-(r11.xyzx))+(r4.wwww)).xyz;
    // 173: mad r11.xyz, cb0[22].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 174: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: add r12.xyz, -r11.xyzx, r4.wwww
    r12.xyz = ((-(r11.xyzx))+(r4.wwww)).xyz;
    // 176: mad r11.xyz, cb0[22].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 177: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mad r13.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 180: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 181: mul r13.xyz, r0.yzwy, r11.xyzx
    r13.xyz = ((r0.yzwy)*(r11.xyzx)).xyz;
    // 182: mad r0.yzw, r11.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r11.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 183: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 184: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 185: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: mul r4.w, r4.w, cb0[25].z
    r4.w = ((r4.wwww)*(source[25].zzzz)).w;
    // 187: mad r2.xyz, r4.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r4.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 188: frc r4.w, cb0[4].x
    r4.w = (frac(source[4].xxxx)).w;
    // 189: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: mad r1.xyz, r5.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 191: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 193: mad r1.xyz, cb0[22].yyyy, r6.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 194: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 195: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 196: mad r1.xyz, cb0[22].zzzz, r6.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 197: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 198: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 199: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 200: dp3 r5.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 201: add r6.xyz, -r0.yzwy, r5.wwww
    r6.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 202: add r0.yzw, r0.yyzw, -r6.xxyz
    r0.yzw = ((r0.yyzw)+(-(r6.xxyz))).yzw;
    // 203: mul r6.xyz, cb0[16].xyzx, cb0[27].yyyy
    r6.xyz = ((source[16].xyzx)*(source[27].yyyy)).xyz;
    // 204: mul r6.xyz, r6.xyzx, cb0[28].wwww
    r6.xyz = ((r6.xyzx)*(source[28].wwww)).xyz;
    // 205: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 206: mad r10.xyz, r1.wwww, cb0[15].xyzx, -cb0[15].xyzx
    r10.xyz = ((r1.wwww)*(source[15].xyzx)+(-(source[15].xyzx))).xyz;
    // 207: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 208: mad r1.w, cb0[14].w, r1.w, l(1.000000)
    r1.w = ((source[14].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: mad r10.xyz, cb0[15].wwww, r10.xyzx, cb0[15].xyzx
    r10.xyz = ((source[15].wwww)*(r10.xyzx)+(source[15].xyzx)).xyz;
    // 210: mad r0.yzw, r0.yyzw, r6.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r6.xxyz)+(r10.xxyz)).yzw;
    // 211: mad r0.yzw, r1.wwww, cb0[14].xxyz, r0.yyzw
    r0.yzw = ((r1.wwww)*(source[14].xxyz)+(r0.yyzw)).yzw;
    // 212: mad r0.yzw, r1.xxyz, r12.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 213: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 214: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 215: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 216: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 217: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 218: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 219: mul r1.xyz, r1.xxxx, cb0[17].xyzx
    r1.xyz = ((r1.xxxx)*(source[17].xyzx)).xyz;
    // 220: movc r1.xyz, r0.xxxx, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 221: add r0.xyz, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 222: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 223: dp3 r0.w, r9.xyzx, r9.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 224: sqrt r1.x, r0.w
    r1.x = (sqrt(r0.wwww)).x;
    // 225: div r1.xyz, r9.xyzx, r1.xxxx
    r1.xyz = ((r9.xyzx)/(r1.xxxx)).xyz;
    // 226: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 227: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 228: mul r1.y, |r1.x|, |r1.x|
    r1.y = ((abs(r1.xxxx))*(abs(r1.xxxx))).y;
    // 229: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 230: mul r1.y, r1.y, |r1.x|
    r1.y = ((r1.yyyy)*(abs(r1.xxxx))).y;
    // 231: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 232: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 233: add r1.y, r1.x, l(-0.027778)
    r1.y = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 234: mad r1.x, r1.x, r1.y, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 235: div_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)/(r0.wwww))).w;
    // 236: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 238: mad r1.xyz, r0.wwww, r2.xyzx, -r13.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 239: mad r1.xyz, r3.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 240: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 241: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 242: mad r1.xyz, cb0[22].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 243: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 244: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 245: mad r1.xyz, cb0[22].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 246: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 247: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 248: mul r0.w, r0.w, cb0[26].z
    r0.w = ((r0.wwww)*(source[26].zzzz)).w;
    // 249: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 250: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 251: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 252: mul r1.w, cb0[4].z, l(1.500000)
    r1.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 253: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 254: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 255: add r1.w, -r4.w, cb0[4].x
    r1.w = ((-(r4.wwww))+(source[4].xxxx)).w;
    // 256: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 257: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 258: mul r4.x, r1.w, l(0.125000)
    r4.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 259: mul r2.y, cb0[4].y, cb0[18].y
    r2.y = ((source[4].yyyy)*(source[18].yyyy)).y;
    // 260: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 261: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 262: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 263: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 264: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r2.xyzw = (ArtistNativeSample6((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 265: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 266: mul r0.w, r4.w, r2.w
    r0.w = ((r4.wwww)*(r2.wwww)).w;
    // 267: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 268: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 269: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 270: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 271: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 272: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 273: mad r2.xy, cb0[19].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[19].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 274: mul r0.w, cb0[19].y, cb0[26].z
    r0.w = ((source[19].yyyy)*(source[26].zzzz)).w;
    // 275: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 276: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 277: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 278: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 279: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 280: mul r1.w, cb0[19].x, l(0.001000)
    r1.w = ((source[19].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 281: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 282: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 283: dp2 r1.w, cb0[20].xyxx, r2.xyxx
    r1.w = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 284: dp2 r2.y, cb0[21].xyxx, r2.xyxx
    r2.y = (dot((source[21].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 285: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 286: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r2.xyzw = (ArtistNativeSample6((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 288: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 289: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 290: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 291: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 292: mad r4.xyz, cb0[19].zzzz, r2.xyzx, -r1.xyzx
    r4.xyz = ((source[19].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 293: mul r2.xyz, r2.xyzx, cb0[19].zzzz
    r2.xyz = ((r2.xyzx)*(source[19].zzzz)).xyz;
    // 294: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 295: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 296: mad r1.xyz, r0.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 297: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 298: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 299: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 300: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
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
    // 310: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 311: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 313: mad o0.xyz, r1.xyzx, cb0[31].xyzx, r0.xyzx
    output.xyz = ((r1.xyzx)*(source[31].xyzx)+(r0.xyzx)).xyz;
    // 315: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_waterflow_02_23_tr: d9f2c7cdd613b64d9edfa91fd56b4360; selected map 57e2aaddb6b601020326685c9f27458cf177068c4a1bff5d11f64647f85b5ddb.
float4 ArtistNative2819(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].zzzz,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[5] = input.dynamicParameter;
    source[6] = g_ArtistSourceMaterialParameters[6u];
    source[7] = g_ArtistSourceMaterialParameters[7u];
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[10].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: mov r0.w, cb0[10].z
    r0.w = (source[10].zzzz).w;
    // 2: mov r0.yz, l(0,0,0,0)
    r0.yz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yz;
    // 3: mad r1.xyzw, v4.xyxy, cb0[4].xyxy, r0.wzzw
    r1.xyzw = ((v4.xyxy)*(source[4].xyxy)+(r0.wzzw)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: mul r1.xy, v4.xyxx, cb0[4].xyxx
    r1.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.zwzz, -r1.xxxx
    r2.xy = ((r0.zwzz)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 13: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 14: div r0.zw, r1.xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)/(r0.zzzz)).zw;
    // 15: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 16: mul r0.x, cb0[5].x, l(0.500000)
    r0.x = ((source[5].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 17: mad r0.xy, v4.xyxx, cb0[9].xyxx, -r0.xyxx
    r0.xy = ((v4.xyxx)*(source[9].xyxx)+(-(r0.xyxx))).xy;
    // 18: add r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)+(source[5].wwww)).y;
    // 19: mad r0.xy, cb0[5].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[5].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 20: mul r0.zw, r0.zzzw, cb0[5].xxxx
    r0.zw = ((r0.zzzw)*(source[5].xxxx)).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 22: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 23: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 24: mul r0.x, r0.x, cb0[13].z
    r0.x = ((r0.xxxx)*(source[13].zzzz)).x;
    // 25: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 26: mad r1.xy, v4.xyxx, cb0[8].xyxx, r0.zwzz
    r1.xy = ((v4.xyxx)*(source[8].xyxx)+(r0.zwzz)).xy;
    // 27: mad r0.yz, v4.xxyx, cb0[3].xxyx, r0.zzwz
    r0.yz = ((v4.xxyx)*(source[3].xxyx)+(r0.zzwz)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t3.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: add r1.w, r1.x, cb0[13].w
    r1.w = ((r1.xxxx)+(source[13].wwww)).w;
    // 31: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 32: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[12].zzzz
    r1.xyz = ((r1.xyzx)*(source[12].zzzz)).xyz;
    // 34: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 35: mad r1.xyz, -cb0[12].wwww, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[12].wwww))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 37: mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // 38: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 39: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 40: max r1.w, |r1.w|, l(0.000001)
    r1.w = (max(abs(r1.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 41: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 42: mul r1.w, r1.w, cb0[14].y
    r1.w = ((r1.wwww)*(source[14].yyyy)).w;
    // 43: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 44: mul_sat r1.w, r1.w, cb0[14].z
    r1.w = (saturate((r1.wwww)*(source[14].zzzz))).w;
    // 45: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 46: mul r0.x, r0.x, cb0[14].w
    r0.x = ((r0.xxxx)*(source[14].wwww)).x;
    // 47: mad r2.xy, v4.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((v4.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 48: add r2.xy, -|r2.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(abs(r2.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 49: log r2.zw, |r2.xxxy|
    r2.zw = (log2(abs(r2.xxxy))).zw;
    // 50: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 51: mul r2.zw, r2.zzzw, cb0[15].xxxx
    r2.zw = ((r2.zzzw)*(source[15].xxxx)).zw;
    // 52: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 53: movc r2.xy, r2.xyxx, l(0,0,0,0), r2.zwzz
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zwzz)).xy;
    // 54: mul r1.w, r2.y, r2.x
    r1.w = ((r2.yyyy)*(r2.xxxx)).w;
    // 55: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 56: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 58: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 59: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 60: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 61: add r2.xyz, -r0.yzwy, r0.xxxx
    r2.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 62: mad r0.xyz, cb0[11].yyyy, r2.xyzx, r0.yzwy
    r0.xyz = ((source[11].yyyy)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 63: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 64: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 65: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 66: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 67: mul r0.xyz, r0.xyzx, cb0[11].wwww
    r0.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 68: add r2.xyz, -cb0[6].xyzx, cb0[7].xyzx
    r2.xyz = ((-(source[6].xyzx))+(source[7].xyzx)).xyz;
    // 69: mad r1.xyz, r1.xyzx, r2.xyzx, cb0[6].xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(source[6].xyzx)).xyz;
    // 70: mad r0.xyz, r0.xyzx, cb0[1].xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(r1.xyzx)).xyz;
    // 71: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_waterflow_02_03_tr: efb355a7f7df3d43af08f34de793b1f1; selected map 06f744254df8ed9fabaa45b517e6274d5daab8974d482fabe3dc17b97ab08553.
float4 ArtistNative2820(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[8u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].zzzz,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = g_ArtistSourceMaterialParameters[7u];
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[8].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: mov r0.y, cb0[8].z
    r0.y = (source[8].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.xyzw, v2.xyxy, cb0[3].xyxy, r0.yxxy
    r0.xyzw = ((v2.xyxy)*(source[3].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.zw, v2.xxxy, cb0[3].xxxy
    r0.zw = ((v2.xxxy)*(source[3].xxxy)).zw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 8: add r1.xy, -r0.zzzz, r0.xyxx
    r1.xy = ((-(r0.zzzz))+(r0.xyxx)).xy;
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
    // 16: mul r1.x, v4.x, l(0.500000)
    r1.x = ((v4.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 17: mov r1.yz, l(0,0,0,0)
    r1.yz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yz;
    // 18: mad r0.zw, v2.xxxy, cb0[7].xxxy, -r1.xxxy
    r0.zw = ((v2.xxxy)*(source[7].xxxy)+(-(r1.xxxy))).zw;
    // 19: mov r1.w, v4.w
    r1.w = (v4.wwww).w;
    // 20: add r0.zw, r0.zzzw, r1.zzzw
    r0.zw = ((r0.zzzw)+(r1.zzzw)).zw;
    // 21: mad r0.zw, v4.xxxx, r0.xxxy, r0.zzzw
    r0.zw = ((v4.xxxx)*(r0.xxxy)+(r0.zzzw)).zw;
    // 22: mul r0.xy, r0.xyxx, v4.xxxx
    r0.xy = ((r0.xyxx)*(v4.xxxx)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 24: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 25: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 26: mul r0.z, r0.z, cb0[11].z
    r0.z = ((r0.zzzz)*(source[11].zzzz)).z;
    // 27: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 28: mad r1.xy, v2.xyxx, cb0[6].xyxx, r0.xyxx
    r1.xy = ((v2.xyxx)*(source[6].xyxx)+(r0.xyxx)).xy;
    // 29: mad r0.xy, v2.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v2.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t3.xywz, s1, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: add r1.w, r1.x, cb0[11].w
    r1.w = ((r1.xxxx)+(source[11].wwww)).w;
    // 33: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 34: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[10].zzzz
    r1.xyz = ((r1.xyzx)*(source[10].zzzz)).xyz;
    // 36: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 37: mad r1.xyz, -cb0[10].wwww, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[10].wwww))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 38: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 39: mul r1.w, r1.w, v4.z
    r1.w = ((r1.wwww)*(v4.zzzz)).w;
    // 40: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 41: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 42: max r1.w, |r1.w|, l(0.000001)
    r1.w = (max(abs(r1.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 43: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 44: mul r1.w, r1.w, cb0[12].y
    r1.w = ((r1.wwww)*(source[12].yyyy)).w;
    // 45: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 46: mul_sat r1.w, r1.w, cb0[12].z
    r1.w = (saturate((r1.wwww)*(source[12].zzzz))).w;
    // 47: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 48: mul r0.z, r0.z, cb0[12].w
    r0.z = ((r0.zzzz)*(source[12].wwww)).z;
    // 49: mad r2.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 50: add r2.xy, -|r2.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(abs(r2.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 51: log r2.zw, |r2.xxxy|
    r2.zw = (log2(abs(r2.xxxy))).zw;
    // 52: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 53: mul r2.zw, r2.zzzw, cb0[13].xxxx
    r2.zw = ((r2.zzzw)*(source[13].xxxx)).zw;
    // 54: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 55: movc r2.xy, r2.xyxx, l(0,0,0,0), r2.zwzz
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zwzz)).xy;
    // 56: mul r1.w, r2.y, r2.x
    r1.w = ((r2.yyyy)*(r2.xxxx)).w;
    // 57: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 58: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)*(r1.wwww))).z;
    // 60: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 61: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 62: dp3 r0.z, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 63: add r2.xyz, -r0.xywx, r0.zzzz
    r2.xyz = ((-(r0.xywx))+(r0.zzzz)).xyz;
    // 64: mad r0.xyz, cb0[9].yyyy, r2.xyzx, r0.xywx
    r0.xyz = ((source[9].yyyy)*(r2.xyzx)+(r0.xywx)).xyz;
    // 65: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 66: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 67: mul r0.xyz, r0.xyzx, cb0[9].zzzz
    r0.xyz = ((r0.xyzx)*(source[9].zzzz)).xyz;
    // 68: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 69: mul r0.xyz, r0.xyzx, cb0[9].wwww
    r0.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 70: add r2.xyz, -cb0[4].xyzx, cb0[5].xyzx
    r2.xyz = ((-(source[4].xyzx))+(source[5].xyzx)).xyz;
    // 71: mad r1.xyz, r1.xyzx, r2.xyzx, cb0[4].xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(source[4].xyzx)).xyz;
    // 72: mad r0.xyz, r0.xyzx, v3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(r1.xyzx)).xyz;
    // 73: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 74: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_sphereline_01_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative2821(ARTIST_NATIVE_INPUT input)
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
// bfx_d_pa_spla_01_07_tr: c951ffd21db8fa40abedb86074bf2fb5; selected map 3d2c3c14d4ed73ef7748d0994535788a9f9d5d88a7e21ad012ff51c494abe7e3.
float4 ArtistNative2822(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ArtistSourceMaterialParameters[3u];
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[6].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[7].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0))).x;
    source[8].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 1: mad r0.xy, cb0[7].yyyy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((source[7].yyyy)*(v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, cb0[7].yyyy, v2.xxyx, cb0[4].xxyx
    r0.yz = ((source[7].yyyy)*(v2.xxyx)+(source[4].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 6: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 7: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 8: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 9: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 10: mul r0.xyz, r0.xyzx, cb0[8].zzzz
    r0.xyz = ((r0.xyzx)*(source[8].zzzz)).xyz;
    // 11: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 12: mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 13: mad r0.xyz, cb0[8].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[8].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 15: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 16: mul r0.xy, v2.xyxx, cb0[9].yzyy
    r0.xy = ((v2.xyxx)*(source[9].yzyy)).xy;
    // 17: mad r0.xy, cb0[6].yyyy, cb0[9].xwxx, r0.xyxx
    r0.xy = ((source[6].yyyy)*(source[9].xwxx)+(r0.xyxx)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 19: mad r0.xy, r0.xxxx, cb0[10].xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)*(source[10].xxxx)+(v2.xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.yxzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 21: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 22: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 23: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 24: mul r0.y, r0.y, cb0[10].y
    r0.y = ((r0.yyyy)*(source[10].yyyy)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul_sat r0.y, r0.y, cb0[10].z
    r0.y = (saturate((r0.yyyy)*(source[10].zzzz))).y;
    // 27: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 28: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 29: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_bubblesq_01_tr: 5ee6df364f5a90479ebaa628d7fcf54d; selected map 127eb09ef9912ad892b985544d5d5c748fd4a8d7254dc399a9913c5f2613f758.
float4 ArtistNative2823(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww),1u);
    source[3] = ArtistNativeAppend(float4(1.0, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 2: mov r0.y, v4.x
    r0.y = (v4.xxxx).y;
    // 3: mad r0.xy, v2.xyxx, l(0.250000, 0.250000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(0.250000,0.250000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, v2.xyxx, t2.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 5: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 6: mad r0.xy, cb0[4].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[4].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 7: add r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)+(source[2].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 10: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 11: mad r0.xyz, cb0[5].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 12: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 13: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 14: mul r0.xyz, r0.xyzx, cb0[5].yyyy
    r0.xyz = ((r0.xyzx)*(source[5].yyyy)).xyz;
    // 15: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, cb0[5].zzzz
    r0.xyz = ((r0.xyzx)*(source[5].zzzz)).xyz;
    // 17: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 18: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 19: mul r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(source[3].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 21: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 22: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 23: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 24: mul r0.x, r0.x, cb0[6].x
    r0.x = ((r0.xxxx)*(source[6].xxxx)).x;
    // 25: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 27: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 28: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 29: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_et_01_2_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative2824(ARTIST_NATIVE_INPUT input)
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
// fx_a_pa_lightning04_ad: 73fa8bff8496124dad818302d74726a9; selected map d1668dc056704f707850cb2f4e7173e4b7ade32bb55cce7759b9f427aab302dd.
float4 ArtistNative2825(ARTIST_NATIVE_INPUT input)
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: mov r1.x, l(0)
    r1.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r1.y, v3.w, l(-0.400000), l(0.950000)
    r1.y = ((v3.wwww)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(float4(0.950000,0.950000,0.950000,0.950000))).y;
    // 4: add r0.xy, r0.xxxx, r1.xyxx
    r0.xy = ((r0.xxxx)+(r1.xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t2.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 7: add r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)+(r0.wwww)).x;
    // 8: mul r0.yzw, r0.xxyz, r0.wwww
    r0.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 9: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 10: mad r0.yzw, r0.yyzw, l(0.000000, 10.000000, 10.000000, 10.000000), cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,10.000000,10.000000,10.000000))+(source[1].xxyz)).yzw;
    // 11: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 12: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 14: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 15: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 16: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_shockwave_01_22_tr: 5016f7178c5a9e4b8f85fd8cad2330c4; selected map 991d7db1464abed57cf64ac434c5eebd64d0fc86e2d58ff66c7097f0b85cd245.
float4 ArtistNative2826(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[5].z = (((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[5].w = (((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[6].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 27: mad r0.y, r0.y, l(0.159155), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 28: mad r1.x, cb0[6].x, r0.y, cb0[2].x
    r1.x = ((source[6].xxxx)*(r0.yyyy)+(source[2].xxxx)).x;
    // 29: mul r2.y, r0.y, cb0[4].y
    r2.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 30: mul r3.x, r0.y, cb0[3].z
    r3.x = ((r0.yyyy)*(source[3].zzzz)).x;
    // 31: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 32: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 33: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 34: mul r0.yz, r0.yyyy, v4.wwzw
    r0.yz = ((r0.yyyy)*(v4.wwzw)).yz;
    // 35: exp r0.yz, r0.yyzy
    r0.yz = (exp2(r0.yyzy)).yz;
    // 36: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.yzyy
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzyy)).xy;
    // 37: mov r2.xz, r0.yyyy
    r2.xz = (r0.yyyy).xz;
    // 38: mad r0.zw, cb0[3].yyyy, cb0[4].xxxz, r2.yyyz
    r0.zw = ((source[3].yyyy)*(source[4].xxxz)+(r2.yyyz)).zw;
    // 39: mad r1.y, cb0[3].y, cb0[6].w, r2.x
    r1.y = ((source[3].yyyy)*(source[6].wwww)+(r2.xxxx)).y;
    // 40: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(-1.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 41: mad r1.xy, cb0[4].wwww, r0.zzzz, r1.xyxx
    r1.xy = ((source[4].wwww)*(r0.zzzz)+(r1.xyxx)).xy;
    // 42: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s2, l(-1.000000)
    r0.w = (ArtistNativeSample2((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 43: mad r1.xy, cb0[4].wwww, r0.zzzz, v2.xyxx
    r1.xy = ((source[4].wwww)*(r0.zzzz)+(v2.xyxx)).xy;
    // 44: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 45: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 46: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 47: mad r1.x, -r1.x, l(2.000000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 49: mov_sat r1.y, v4.y
    r1.y = (saturate(v4.yyyy)).y;
    // 50: mad r0.w, r0.w, r1.y, r1.x
    r0.w = ((r0.wwww)*(r1.yyyy)+(r1.xxxx)).w;
    // 51: add r1.y, -v4.x, l(1.000000)
    r1.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 52: add r0.w, r0.w, -r1.y
    r0.w = ((r0.wwww)+(-(r1.yyyy))).w;
    // 53: mul_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)*(r0.wwww))).w;
    // 54: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 55: mov r3.y, r0.y
    r3.y = (r0.yyyy).y;
    // 56: mad r0.yw, cb0[3].yyyy, cb0[3].xxxw, r3.xxxy
    r0.yw = ((source[3].yyyy)*(source[3].xxxw)+(r3.xxxy)).yw;
    // 57: mad r0.yz, cb0[4].wwww, r0.zzzz, r0.yywy
    r0.yz = ((source[4].wwww)*(r0.zzzz)+(r0.yywy)).yz;
    // 58: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s0, l(-1.000000)
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 59: mul r0.x, r0.x, cb0[7].x
    r0.x = ((r0.xxxx)*(source[7].xxxx)).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 62: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 63: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 64: mad r0.xyz, cb0[5].xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((source[5].xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 65: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 66: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_ring_07_90_ad: ab8d280a5e9f3b4f855b5cbb93059208; selected map db70879c02640af560c74d70780192ec09e00ae046e906475fa44c6e42569acc.
float4 ArtistNative2827(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].w = ((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[4].x = (((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].y = (((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[5].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    r0.y = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 44: mad r0.xy, r0.yyyy, cb0[6].wwww, r1.xyxx
    r0.xy = ((r0.yyyy)*(source[6].wwww)+(r1.xyxx)).xy;
    // 45: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, cb0[3].x
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (source[3].xxxx).x, true).xyzw).x;
    // 46: mul_sat r0.x, r0.x, cb0[7].x
    r0.x = (saturate((r0.xxxx)*(source[7].xxxx))).x;
    // 47: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 48: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 50: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 52: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 53: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 54: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 56: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 57: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 58: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 59: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 60: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 61: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 62: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 63: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_ring_01_1_ad: 6941275e665e9847a31e0c40601c40b4; selected map 352889af56305ecfe19a5374c0d320a9369c83a4f65394f276a4e46c7f0284a9.
float4 ArtistNative2828(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
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
    // 30: mul r0.y, r1.x, cb0[3].w
    r0.y = ((r1.xxxx)*(source[3].wwww)).y;
    // 31: mad r2.x, cb0[3].z, cb0[3].y, r0.y
    r2.x = ((source[3].zzzz)*(source[3].yyyy)+(r0.yyyy)).x;
    // 32: mul r0.y, v2.y, cb0[5].x
    r0.y = ((v2.yyyy)*(source[5].xxxx)).y;
    // 33: mad r3.y, cb0[3].z, cb0[5].y, r0.y
    r3.y = ((source[3].zzzz)*(source[5].yyyy)+(r0.yyyy)).y;
    // 34: mul r0.yz, cb0[3].zzzz, cb0[4].zzyz
    r0.yz = ((source[3].zzzz)*(source[4].zzyz)).yz;
    // 35: mad r3.x, cb0[4].w, v2.x, r0.y
    r3.x = ((source[4].wwww)*(v2.xxxx)+(r0.yyyy)).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r3.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 37: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 38: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 40: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 41: movc r1.y, r0.x, l(0), r0.w
    r1.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 42: mad r2.y, cb0[4].x, r1.y, r0.z
    r2.y = ((source[4].xxxx)*(r1.yyyy)+(r0.zzzz)).y;
    // 43: mul r0.xz, r1.xxyx, cb0[6].zzwz
    r0.xz = ((r1.xxyx)*(source[6].zzwz)).xz;
    // 44: mul r0.w, v4.x, cb0[5].z
    r0.w = ((v4.xxxx)*(source[5].zzzz)).w;
    // 45: mad r1.xy, r0.yyyy, r0.wwww, r2.xyxx
    r1.xy = ((r0.yyyy)*(r0.wwww)+(r2.xyxx)).xy;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s0, cb0[3].x
    r1.xyz = (ArtistNativeSample0((r1.xyxx).xy, (source[3].xxxx).x, true).xyzw).xyz;
    // 47: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 49: mad r1.xyz, cb0[5].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[5].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 50: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 51: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 52: mul r1.xyz, r1.xyzx, cb0[6].xxxx
    r1.xyz = ((r1.xyzx)*(source[6].xxxx)).xyz;
    // 53: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 54: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 55: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 56: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 57: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 58: mad r2.x, cb0[3].z, cb0[6].y, r0.x
    r2.x = ((source[3].zzzz)*(source[6].yyyy)+(r0.xxxx)).x;
    // 59: mad r2.y, cb0[3].z, cb0[7].x, r0.z
    r2.y = ((source[3].zzzz)*(source[7].xxxx)+(r0.zzzz)).y;
    // 60: mad r0.xy, r0.yyyy, r0.wwww, r2.xyxx
    r0.xy = ((r0.yyyy)*(r0.wwww)+(r2.xyxx)).xy;
    // 61: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, cb0[3].x
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (source[3].xxxx).x, true).xyzw).x;
    // 62: mul_sat r0.x, r0.x, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy))).x;
    // 63: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 64: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 66: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 68: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 69: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 70: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 71: mul r0.x, r0.x, cb0[7].z
    r0.x = ((r0.xxxx)*(source[7].zzzz)).x;
    // 72: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 73: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 74: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 75: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 76: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 77: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_ph_116_msk: 3b3eb9ea0d75844688479bb8f82cf8fc; selected map be717b1ff812a469ba8fd61f1f16ce4647b40ac4d50ebe7cb36913cb25086b0c.
float4 ArtistNative2829(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].zzzz,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[4] = g_ArtistSourceMaterialParameters[3u];
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    // 16: mul r0.xy, v4.xyxx, cb0[3].xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 18: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 19: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 20: mad r0.xyz, cb0[7].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[7].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 21: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 22: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 23: mul r0.xyz, r0.xyzx, cb0[7].yyyy
    r0.xyz = ((r0.xyzx)*(source[7].yyyy)).xyz;
    // 24: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 25: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 26: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 27: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 28: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_inksmoke_1_1_tr: b7ae3a4a901620438ec924503d67ae8a; selected map 7bfac0be20dd9cf022bd6b58576a011cb90c5c7f53c34884fe6e6107f2e4a8a7.
float4 ArtistNative2830(ARTIST_NATIVE_INPUT input)
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
    source[3].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[2].zwzz
    r0.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 2: mad r1.x, cb0[2].y, cb0[2].x, r0.x
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[2].y, cb0[3].x, r0.y
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 11: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 12: mul r1.x, r1.x, cb0[4].w
    r1.x = ((r1.xxxx)*(source[4].wwww)).x;
    // 13: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 14: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 15: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 16: movc r0.w, r0.w, l(0), |r1.x|
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).w;
    // 17: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 18: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 19: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 20: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 21: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 22: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 23: movc o0.w, r0.w, l(0), r1.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 24: mul r1.xyz, r0.xyzx, cb0[3].wwww
    r1.xyz = ((r0.xyzx)*(source[3].wwww)).xyz;
    // 25: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: mad r0.xyz, -cb0[3].wwww, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[3].wwww))*(r0.xyzx)+(r0.wwww)).xyz;
    // 27: mad r0.xyz, cb0[4].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 28: mul r1.xyz, r0.xyzx, cb0[4].yyyy
    r1.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 29: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 30: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 31: mul r1.xyz, r1.xyzx, cb0[4].zzzz
    r1.xyz = ((r1.xyzx)*(source[4].zzzz)).xyz;
    // 32: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 33: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: add r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)+(r0.wwww)).xyz;
    // 35: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 36: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_10_05_ds_ad: 5302b738e52f7244969833048f8196db; selected map 5a42b67096426feec50af003e5c1f1dc13f9ebca11acf2ba140585714426d762.
float4 ArtistNative2831(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[3].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[2].zwzz
    r0.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 2: mad r1.x, cb0[2].y, cb0[2].x, r0.x
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[2].y, cb0[3].x, r0.y
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_58_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative2832(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_waterflow_02_70_tr: d9f2c7cdd613b64d9edfa91fd56b4360; selected map 57e2aaddb6b601020326685c9f27458cf177068c4a1bff5d11f64647f85b5ddb.
float4 ArtistNative2833(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].zzzz,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[5] = input.dynamicParameter;
    source[6] = g_ArtistSourceMaterialParameters[6u];
    source[7] = g_ArtistSourceMaterialParameters[7u];
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[10].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: mov r0.w, cb0[10].z
    r0.w = (source[10].zzzz).w;
    // 2: mov r0.yz, l(0,0,0,0)
    r0.yz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yz;
    // 3: mad r1.xyzw, v4.xyxy, cb0[4].xyxy, r0.wzzw
    r1.xyzw = ((v4.xyxy)*(source[4].xyxy)+(r0.wzzw)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: mul r1.xy, v4.xyxx, cb0[4].xyxx
    r1.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.zwzz, -r1.xxxx
    r2.xy = ((r0.zwzz)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 13: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 14: div r0.zw, r1.xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)/(r0.zzzz)).zw;
    // 15: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 16: mul r0.x, cb0[5].x, l(0.500000)
    r0.x = ((source[5].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 17: mad r0.xy, v4.xyxx, cb0[9].xyxx, -r0.xyxx
    r0.xy = ((v4.xyxx)*(source[9].xyxx)+(-(r0.xyxx))).xy;
    // 18: add r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)+(source[5].wwww)).y;
    // 19: mad r0.xy, cb0[5].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[5].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 20: mul r0.zw, r0.zzzw, cb0[5].xxxx
    r0.zw = ((r0.zzzw)*(source[5].xxxx)).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 22: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 23: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 24: mul r0.x, r0.x, cb0[13].z
    r0.x = ((r0.xxxx)*(source[13].zzzz)).x;
    // 25: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 26: mad r1.xy, v4.xyxx, cb0[8].xyxx, r0.zwzz
    r1.xy = ((v4.xyxx)*(source[8].xyxx)+(r0.zwzz)).xy;
    // 27: mad r0.yz, v4.xxyx, cb0[3].xxyx, r0.zzwz
    r0.yz = ((v4.xxyx)*(source[3].xxyx)+(r0.zzwz)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t3.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: add r1.w, r1.x, cb0[13].w
    r1.w = ((r1.xxxx)+(source[13].wwww)).w;
    // 31: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 32: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[12].zzzz
    r1.xyz = ((r1.xyzx)*(source[12].zzzz)).xyz;
    // 34: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 35: mad r1.xyz, -cb0[12].wwww, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[12].wwww))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 37: mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // 38: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 39: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 40: max r1.w, |r1.w|, l(0.000001)
    r1.w = (max(abs(r1.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 41: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 42: mul r1.w, r1.w, cb0[14].y
    r1.w = ((r1.wwww)*(source[14].yyyy)).w;
    // 43: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 44: mul_sat r1.w, r1.w, cb0[14].z
    r1.w = (saturate((r1.wwww)*(source[14].zzzz))).w;
    // 45: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 46: mul r0.x, r0.x, cb0[14].w
    r0.x = ((r0.xxxx)*(source[14].wwww)).x;
    // 47: mad r2.xy, v4.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((v4.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 48: add r2.xy, -|r2.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(abs(r2.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 49: log r2.zw, |r2.xxxy|
    r2.zw = (log2(abs(r2.xxxy))).zw;
    // 50: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 51: mul r2.zw, r2.zzzw, cb0[15].xxxx
    r2.zw = ((r2.zzzw)*(source[15].xxxx)).zw;
    // 52: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 53: movc r2.xy, r2.xyxx, l(0,0,0,0), r2.zwzz
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zwzz)).xy;
    // 54: mul r1.w, r2.y, r2.x
    r1.w = ((r2.yyyy)*(r2.xxxx)).w;
    // 55: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 56: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 58: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 59: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 60: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 61: add r2.xyz, -r0.yzwy, r0.xxxx
    r2.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 62: mad r0.xyz, cb0[11].yyyy, r2.xyzx, r0.yzwy
    r0.xyz = ((source[11].yyyy)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 63: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 64: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 65: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 66: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 67: mul r0.xyz, r0.xyzx, cb0[11].wwww
    r0.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 68: add r2.xyz, -cb0[6].xyzx, cb0[7].xyzx
    r2.xyz = ((-(source[6].xyzx))+(source[7].xyzx)).xyz;
    // 69: mad r1.xyz, r1.xyzx, r2.xyzx, cb0[6].xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(source[6].xyzx)).xyz;
    // 70: mad r0.xyz, r0.xyzx, cb0[1].xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(r1.xyzx)).xyz;
    // 71: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_worldoffset_02_10_tr: a0f7f5723e826143b3970e44f6a045f2; selected map 556a53205748259b14772cac15cc51ca4b99fc29a3e8ad2940bdffdafc2ff11e.
float4 ArtistNative2834(ARTIST_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[4] = g_ArtistSourceMaterialParameters[8u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[6] = ArtistNativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[16].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r0.w = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r1.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_ring_11_30_ad: d82c7989d45d8443a24e07d3001a4218; selected map fcbe7216c0b51790dc9d3f87e35dc9aa2039963b078a494673e56dc9a5ef38df.
float4 ArtistNative2835(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[3].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[3u].yyyy)).x;
    source[6].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[3u].yyyy))).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    r0.z = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (ArtistNativeSample0((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 46: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 47: add r0.w, r1.z, r0.w
    r0.w = ((r1.zzzz)+(r0.wwww)).w;
    // 48: mad r1.x, cb0[2].z, cb0[4].w, r0.x
    r1.x = ((source[2].zzzz)*(source[4].wwww)+(r0.xxxx)).x;
    // 49: mad r1.y, cb0[2].z, cb0[5].z, r0.y
    r1.y = ((source[2].zzzz)*(source[5].zzzz)+(r0.yyyy)).y;
    // 50: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, cb0[2].x
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
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
    // 69: add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: mul_sat r0.z, r0.z, cb0[7].y
    r0.z = (saturate((r0.zzzz)*(source[7].yyyy))).z;
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_waterribbon_01_12_tr: 59a22eeec5a51f439595f929dddfe8bf; selected map 81d674443d6cdbfbf3ac4bbc2bffabd834c314ebbba584dd43fb398eca9cb191.
float4 ArtistNative2836(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[8u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww),float4(0.0, 0.0, 0.0, 0.0),1u);
    source[5] = g_ArtistSourceMaterialParameters[6u];
    source[6] = g_ArtistSourceMaterialParameters[7u];
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[8] = ArtistNativeAppend(float4(0.100000001, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[9] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww),float4(0.0, 0.0, 0.0, 0.0),1u);
    source[10].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    // 1: mov r0.y, cb0[10].z
    r0.y = (source[10].zzzz).y;
    // 2: mov r0.xz, l(0,0,1.000000,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 3: mad r1.xyzw, v2.zwzw, cb0[3].xyxy, r0.yxxy
    r1.xyzw = ((v2.zwzw)*(source[3].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r1.xy, v2.zwzz, cb0[3].xyxx
    r1.xy = ((v2.zwzz)*(source[3].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.xyxx, -r1.xxxx
    r2.xy = ((r0.xyxx)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: div r0.xy, r1.xyxx, r0.xxxx
    r0.xy = ((r1.xyxx)/(r0.xxxx)).xy;
    // 15: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: mov r0.w, v4.x
    r0.w = (v4.xxxx).w;
    // 17: mul r0.xy, r0.zwzz, r0.xyxx
    r0.xy = ((r0.zwzz)*(r0.xyxx)).xy;
    // 18: add r0.zw, v2.zzzw, cb0[8].xxxy
    r0.zw = ((v2.zzzw)+(source[8].xxxy)).zw;
    // 19: mov r1.x, v4.x
    r1.x = (v4.xxxx).x;
    // 20: mov r1.y, l(0.200000)
    r1.y = (float4(0.200000,0.200000,0.200000,0.200000)).y;
    // 21: mad r0.zw, r1.xxxy, r0.xxxy, r0.zzzw
    r0.zw = ((r1.xxxy)*(r0.xxxy)+(r0.zzzw)).zw;
    // 22: mul r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)*(r1.xyxx)).xy;
    // 23: add r0.zw, r0.zzzw, cb0[9].xxxy
    r0.zw = ((r0.zzzw)+(source[9].xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 25: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 26: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 27: mul r0.z, r0.z, cb0[13].z
    r0.z = ((r0.zzzz)*(source[13].zzzz)).z;
    // 28: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 29: mad r1.xy, v2.zwzz, cb0[7].xyxx, r0.xyxx
    r1.xy = ((v2.zwzz)*(source[7].xyxx)+(r0.xyxx)).xy;
    // 30: mad r0.xy, v2.zwzz, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v2.zwzz)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 31: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t3.xywz, s1, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: add r1.w, r1.x, cb0[13].w
    r1.w = ((r1.xxxx)+(source[13].wwww)).w;
    // 35: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 36: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 37: mul r1.xyz, r1.xyzx, cb0[12].yyyy
    r1.xyz = ((r1.xyzx)*(source[12].yyyy)).xyz;
    // 38: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 39: mad r1.xyz, -cb0[12].zzzz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[12].zzzz))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 40: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 41: mul r1.w, r1.w, v4.z
    r1.w = ((r1.wwww)*(v4.zzzz)).w;
    // 42: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 43: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 44: max r1.w, |r1.w|, l(0.000001)
    r1.w = (max(abs(r1.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 45: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 46: mul r1.w, r1.w, cb0[14].y
    r1.w = ((r1.wwww)*(source[14].yyyy)).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: mul_sat r1.w, r1.w, cb0[14].z
    r1.w = (saturate((r1.wwww)*(source[14].zzzz))).w;
    // 49: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 50: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 51: mad r1.w, v2.z, l(2.000000), l(-1.000000)
    r1.w = ((v2.zzzz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 52: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 54: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r2.x, r2.x, cb0[15].x
    r2.x = ((r2.xxxx)*(source[15].xxxx)).x;
    // 56: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 57: mul r2.x, r2.x, l(3.000000)
    r2.x = ((r2.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 58: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 60: mul_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)*(r1.wwww))).z;
    // 61: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 62: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 63: dp3 r0.z, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 64: add r2.xyz, -r0.xywx, r0.zzzz
    r2.xyz = ((-(r0.xywx))+(r0.zzzz)).xyz;
    // 65: mad r0.xyz, cb0[11].xxxx, r2.xyzx, r0.xywx
    r0.xyz = ((source[11].xxxx)*(r2.xyzx)+(r0.xywx)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[11].yyyy
    r0.xyz = ((r0.xyzx)*(source[11].yyyy)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 71: add r2.xyz, -cb0[5].xyzx, cb0[6].xyzx
    r2.xyz = ((-(source[5].xyzx))+(source[6].xyzx)).xyz;
    // 72: mad r1.xyz, r1.xyzx, r2.xyzx, cb0[5].xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(source[5].xyzx)).xyz;
    // 73: mad r0.xyz, r0.xyzx, v3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(r1.xyzx)).xyz;
    // 74: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 75: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_master_01_01_tr: 8b35be041c4bc74ca4ff45d74a399999; selected map 084e429d8752d25272b3b1e8eff968bca85dc7e2e0bf0d67e80e698df7670cec.
float4 ArtistNative2837(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.x, v4.y, l(-1.000000)
    r0.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[4].y, cb0[7].y, cb0[7].z
    r0.y = ((source[4].yyyy)*(source[7].yyyy)+(source[7].zzzz)).y;
    // 3: sincos r1.x, r2.x, r0.y
    r1.x = (sin(r0.yyyy)).x; r2.x = (cos(r0.yyyy)).x;
    // 4: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 5: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 7: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 8: dp2 r0.w, r3.yxyy, r0.yzyy
    r0.w = (dot((r3.yxyy).xy,(r0.yzyy).xy).xxxx).w;
    // 9: dp2 r0.y, r3.zyzz, r0.yzyy
    r0.y = (dot((r3.zyzz).xy,(r0.yzyy).xy).xxxx).y;
    // 10: mad r0.z, r0.y, cb0[3].y, r0.x
    r0.z = ((r0.yyyy)*(source[3].yyyy)+(r0.xxxx)).z;
    // 11: mul r0.x, r0.w, cb0[3].x
    r0.x = ((r0.wwww)*(source[3].xxxx)).x;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mul r0.zw, v2.xxxy, cb0[6].yyyz
    r0.zw = ((v2.xxxy)*(source[6].yyyz)).zw;
    // 16: mul r1.x, cb0[4].x, cb0[4].y
    r1.x = ((source[4].xxxx)*(source[4].yyyy)).x;
    // 17: mad r0.zw, r1.xxxx, cb0[6].xxxw, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[6].xxxw)+(r0.zzzw)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 19: mad r0.x, r0.z, r0.x, -r0.y
    r0.x = ((r0.zzzz)*(r0.xxxx)+(-(r0.yyyy))).x;
    // 20: mul_sat r0.x, r0.x, cb0[8].y
    r0.x = (saturate((r0.xxxx)*(source[8].yyyy))).x;
    // 21: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 22: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 23: mul r0.y, r0.y, cb0[8].z
    r0.y = ((r0.yyyy)*(source[8].zzzz)).y;
    // 24: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 25: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 26: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 27: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 28: mul r0.x, v2.x, cb0[4].w
    r0.x = ((v2.xxxx)*(source[4].wwww)).x;
    // 29: mad r0.x, r1.x, cb0[4].z, r0.x
    r0.x = ((r1.xxxx)*(source[4].zzzz)+(r0.xxxx)).x;
    // 30: mul r0.z, v2.y, cb0[5].x
    r0.z = ((v2.yyyy)*(source[5].xxxx)).z;
    // 31: mad r0.y, r1.x, cb0[5].y, r0.z
    r0.y = ((r1.xxxx)*(source[5].yyyy)+(r0.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 33: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 35: mad r0.xyz, cb0[5].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 36: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 37: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 38: mul r0.xyz, r0.xyzx, cb0[5].wwww
    r0.xyz = ((r0.xyzx)*(source[5].wwww)).xyz;
    // 39: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 40: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 41: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 42: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 43: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_03_03_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 ArtistNative2838(ARTIST_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[5] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),1u);
    source[6] = input.dynamicParameter;
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz),1u);
    source[8] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].zzzz,g_ArtistSourceMaterialParameters[5u].wwww,1u);
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[13].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[15].x = (cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
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
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[4].xxxy, cb0[5].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)+(source[5].xxxy)).zw;
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
    // 16: lt r0.z, |cb0[12].x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[12].xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 17: mul r0.w, |cb0[12].x|, |cb0[12].x|
    r0.w = ((abs(source[12].xxxx))*(abs(source[12].xxxx))).w;
    // 18: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 19: mul r1.y, r0.z, v4.y
    r1.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 20: mul r1.x, v4.x, cb0[12].y
    r1.x = ((v4.xxxx)*(source[12].yyyy)).x;
    // 21: mad r0.zw, cb0[6].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 22: add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // 23: add r0.zw, r0.zzzw, cb0[7].xxxy
    r0.zw = ((r0.zzzw)+(source[7].xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 25: lt r0.z, |cb0[13].z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(source[13].zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.w, |cb0[13].z|, |cb0[13].z|
    r0.w = ((abs(source[13].zzzz))*(abs(source[13].zzzz))).w;
    // 27: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 28: mul r2.y, r0.z, v4.y
    r2.y = ((r0.zzzz)*(v4.yyyy)).y;
    // 29: mul r2.x, v4.x, cb0[13].w
    r2.x = ((v4.xxxx)*(source[13].wwww)).x;
    // 30: mad r0.zw, cb0[6].xxxx, r0.xxxy, r2.xxxy
    r0.zw = ((source[6].xxxx)*(r0.xxxy)+(r2.xxxy)).zw;
    // 31: mul r0.xy, r0.xyxx, cb0[6].xxxx
    r0.xy = ((r0.xyxx)*(source[6].xxxx)).xy;
    // 32: add r0.zw, r0.zzzw, cb0[8].xxxy
    r0.zw = ((r0.zzzw)+(source[8].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 35: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 36: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 37: mad r1.xyz, cb0[14].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[14].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 38: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 39: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 40: mul r1.xyz, r1.xyzx, cb0[14].yyyy
    r1.xyz = ((r1.xyzx)*(source[14].yyyy)).xyz;
    // 41: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 42: mul r1.xyz, r1.xyzx, cb0[14].zzzz
    r1.xyz = ((r1.xyzx)*(source[14].zzzz)).xyz;
    // 43: mad r1.xyz, cb0[1].xyzx, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((source[1].xyzx)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 44: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 45: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 46: mad r0.zw, v4.xxxy, cb0[9].xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)*(source[9].xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 47: dp2 r1.x, cb0[10].xyxx, r0.zwzz
    r1.x = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 48: dp2 r1.y, cb0[11].xyxx, r0.zwzz
    r1.y = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 49: add r0.zw, r1.xxxy, cb0[6].zzzy
    r0.zw = ((r1.xxxy)+(source[6].zzzy)).zw;
    // 50: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 51: mad r0.xy, cb0[15].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[15].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 53: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 54: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 56: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 57: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 58: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 59: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 60: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 61: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 64: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 65: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 66: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 67: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 68: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 69: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 70: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 71: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 72: mul r0.x, r0.x, cb0[16].y
    r0.x = ((r0.xxxx)*(source[16].yyyy)).x;
    // 73: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 74: mul_sat r0.x, r0.x, cb0[16].z
    r0.x = (saturate((r0.xxxx)*(source[16].zzzz))).x;
    // 75: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 76: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// sk_mn_ppct_05_mi: 8c7feae3b54e7a46835555bfa86e7e6e; selected map 4715b529d8a6d848f145e45eca79ec4714277c9f25abbd2f5a78c5d0fab9a3cd.
float4 ArtistNative2839(ARTIST_NATIVE_INPUT input)
{
    float4 source[29]; [unroll] for (uint i=0u; i<29u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=0.f; // Absolute source world position needs no pre-view translation.
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[2]=float4(input.sourceActorPosition,0.f); // Original actor-position pulse seed.
    source[26]=float4(input.skyUpperColor,0.f);
    source[27]=float4(input.skyLowerColor,0.f);
    source[28]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[16u];
    source[4] = g_ArtistSourceMaterialParameters[17u];
    source[5] = g_ArtistSourceMaterialParameters[9u];
    source[6] = g_ArtistSourceMaterialParameters[12u];
    source[7] = g_ArtistSourceMaterialParameters[11u];
    source[8] = g_ArtistSourceMaterialParameters[14u];
    source[9] = g_ArtistSourceMaterialParameters[15u];
    source[10] = g_ArtistSourceMaterialParameters[10u];
    source[11] = g_ArtistSourceMaterialParameters[19u];
    source[12] = g_ArtistSourceMaterialParameters[7u];
    source[13] = g_ArtistSourceMaterialParameters[8u];
    source[14] = g_ArtistSourceMaterialParameters[13u];
    source[15] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[16] = g_ArtistSourceMaterialParameters[18u];
    source[17] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[18] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[19].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[19].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[20].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[20].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[20].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[20].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[21].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[21].y = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[21].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[21].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[22].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[22].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[22].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[22].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[23].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[23].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[23].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[23].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[24].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[24].y = ((g_ArtistSourceMaterialParameters[0u].wwww*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[24].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[24].w = ((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[25].x = (((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[25].y = (sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[25].z = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[25].w = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r0.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 4: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 8: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 9: add r1.x, -r0.x, l(1000.000000)
    r1.x = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 10: mad r0.x, cb0[23].w, r1.x, r0.x
    r0.x = ((source[23].wwww)*(r1.xxxx)+(r0.xxxx)).x;
    // 11: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 12: mad r0.x, cb0[23].y, cb0[23].z, r0.x
    r0.x = ((source[23].yyyy)*(source[23].zzzz)+(r0.xxxx)).x;
    // 13: mul r1.x, r0.x, l(3.524534)
    r1.x = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 14: sincos null, r1.x, r1.x
    r1.x = (cos(r1.xxxx)).x;
    // 15: add r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 16: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 17: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 18: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 19: mad r0.x, r0.x, l(0.500000), cb0[23].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[23].xxxx)).x;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r1.xyz = (ArtistNativeSample5((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mul r2.xyz, cb0[10].xyzx, cb0[22].wwww
    r2.xyz = ((source[10].xyzx)*(source[22].wwww)).xyz;
    // 22: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 24: mul r2.xyz, v7.yyyy, cb1[1].xywx
    r2.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 25: mad r2.xyz, cb1[0].xywx, v7.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v7.xxxx)+(r2.xyzx)).xyz;
    // 26: mad r2.xyz, cb1[2].xywx, v7.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v7.zzzz)+(r2.xyzx)).xyz;
    // 27: mad r2.xyz, cb1[3].xywx, v7.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v7.wwww)+(r2.xyzx)).xyz;
    // 28: div r2.xy, r2.xyxx, r2.zzzz
    r2.xy = ((r2.xyxx)/(r2.zzzz)).xy;
    // 29: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: mul r2.xy, r2.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 31: deriv_rtx_coarse r2.zw, r2.xxxy
    r2.zw = (ddx_coarse(r2.xxxy)).zw;
    // 32: deriv_rty_coarse r2.xy, r2.xyxx
    r2.xy = (ddy_coarse(r2.xyxx)).xy;
    // 33: dp2 r0.x, r2.xyxx, r2.xyxx
    r0.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 34: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 35: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 36: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 37: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 38: rcp r1.w, |r0.x|
    r1.w = (1.0/(abs(r0.xxxx))).w;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 40: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: log r3.x, |r2.w|
    r3.x = (log2(abs(r2.wwww))).x;
    // 42: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 43: mul r3.x, r3.x, cb0[20].x
    r3.x = ((r3.xxxx)*(source[20].xxxx)).x;
    // 44: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 45: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: movc r2.w, r2.w, l(0), r3.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 47: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 48: mul r3.x, r3.x, cb0[20].y
    r3.x = ((r3.xxxx)*(source[20].yyyy)).x;
    // 49: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: add r0.x, |r0.x|, r1.w
    r0.x = ((abs(r0.xxxx))+(r1.wwww)).x;
    // 52: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 54: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 55: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 56: mul r3.xy, r3.xyxx, cb0[19].xxxx
    r3.xy = ((r3.xyxx)*(source[19].xxxx)).xy;
    // 57: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 59: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 60: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 61: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r4.xyz, cb0[19].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[19].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 63: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 64: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 65: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 66: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 67: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 68: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 69: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 70: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 71: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 72: mul r7.xyz, r1.wwww, v1.xyzx
    r7.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 73: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 74: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 75: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 76: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 77: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 78: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 79: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 80: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 81: mad r9.xyz, v5.xyzx, r1.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r1.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 82: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 83: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 84: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 85: dp3 r1.w, r6.xyzx, r10.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 86: mul r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 87: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 88: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 89: dp2 r1.w, r6.ywyy, r6.ywyy
    r1.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 90: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 91: div r6.xy, r6.ywyy, r1.wwww
    r6.xy = ((r6.ywyy)/(r1.wwww)).xy;
    // 92: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 93: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 95: mad r6.xy, r1.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t3.xyzw, s3, r0.x
    r6.xyz = (ArtistNativeSample3((r6.xyxx).xy, (r0.xxxx).x, true).xyzw).xyz;
    // 97: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 98: rcp r0.x, cb0[20].z
    r0.x = (1.0/(source[20].zzzz)).x;
    // 99: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 100: mul r10.xyz, r10.xyzx, cb0[20].zzzz
    r10.xyz = ((r10.xyzx)*(source[20].zzzz)).xyz;
    // 101: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 102: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 103: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 104: mad r10.xyz, r10.xyzx, cb0[20].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[20].zzzz)+(r11.xyzx)).xyz;
    // 105: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 106: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 107: add r0.x, cb0[20].z, l(1.000000)
    r0.x = ((source[20].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 109: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r6.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r6.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 111: mad r6.xyz, r3.wwww, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[8].xyzx)).xyz;
    // 112: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 113: mul r6.xyz, r6.xyzx, cb0[20].wwww
    r6.xyz = ((r6.xyzx)*(source[20].wwww)).xyz;
    // 114: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 115: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 116: mad r2.xyz, cb0[19].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 117: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 118: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 119: mad r2.xyz, cb0[19].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 120: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 122: mul r10.xyz, r10.xyzx, cb0[21].xxxx
    r10.xyz = ((r10.xyzx)*(source[21].xxxx)).xyz;
    // 123: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r11.xyzw = (ArtistNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 124: add r0.x, r11.y, r11.x
    r0.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 125: add r0.x, r11.z, r0.x
    r0.x = ((r11.zzzz)+(r0.xxxx)).x;
    // 126: add_sat r0.x, r11.w, r0.x
    r0.x = (saturate((r11.wwww)+(r0.xxxx))).x;
    // 127: mad r2.xyz, r0.xxxx, r10.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 128: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 129: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 130: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 131: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 132: dp3 r0.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 133: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 134: mul r0.x, r0.x, cb0[22].x
    r0.x = ((r0.xxxx)*(source[22].xxxx)).x;
    // 135: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 136: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 137: mad r1.w, -r0.x, r0.x, l(1.000000)
    r1.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 139: div r1.w, cb0[22].y, r1.w
    r1.w = ((source[22].yyyy)/(r1.wwww)).w;
    // 140: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 141: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 142: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 143: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 144: mul_sat r4.w, r3.w, cb0[21].y
    r4.w = (saturate((r3.wwww)*(source[21].yyyy))).w;
    // 145: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul_sat r5.w, r4.z, cb0[21].y
    r5.w = (saturate((r4.zzzz)*(source[21].yyyy))).w;
    // 148: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: add_sat r5.w, r5.w, -cb0[21].z
    r5.w = (saturate((r5.wwww)+(-(source[21].zzzz)))).w;
    // 150: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 151: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 152: mul r6.w, r6.w, cb0[21].w
    r6.w = ((r6.wwww)*(source[21].wwww)).w;
    // 153: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 154: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 155: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 156: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 157: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 158: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: add r11.xyz, -r0.yzwy, r1.wwww
    r11.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 160: mad r0.yzw, cb0[19].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 161: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r11.xyz, -r0.yzwy, r1.wwww
    r11.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 163: mad r0.yzw, cb0[19].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 164: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 165: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 166: mad r12.xyz, -cb0[5].wwww, cb0[5].xyzx, r1.wwww
    r12.xyz = ((-(source[5].wwww))*(source[5].xyzx)+(r1.wwww)).xyz;
    // 167: mad r11.xyz, cb0[19].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 168: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 169: add r12.xyz, -r11.xyzx, r1.wwww
    r12.xyz = ((-(r11.xyzx))+(r1.wwww)).xyz;
    // 170: mad r11.xyz, cb0[19].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 171: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 174: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 175: mul r13.xyz, r0.yzwy, r11.xyzx
    r13.xyz = ((r0.yzwy)*(r11.xyzx)).xyz;
    // 176: mad r0.yzw, r11.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r11.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 177: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 178: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 179: add r1.w, -r0.x, l(1.000000)
    r1.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: mul r1.w, r1.w, cb0[22].z
    r1.w = ((r1.wwww)*(source[22].zzzz)).w;
    // 181: mad r2.xyz, r1.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 182: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 183: add r5.w, -r1.w, l(1.000000)
    r5.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mad r1.xyz, r5.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 185: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 187: mad r1.xyz, cb0[19].yyyy, r6.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 188: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 190: mad r1.xyz, cb0[19].zzzz, r6.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 191: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 192: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 193: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 194: dp3 r5.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 195: add r6.xyz, -r0.yzwy, r5.wwww
    r6.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 196: add r0.yzw, r0.yyzw, -r6.xxyz
    r0.yzw = ((r0.yyzw)+(-(r6.xxyz))).yzw;
    // 197: mul r6.xyz, cb0[13].xyzx, cb0[24].yyyy
    r6.xyz = ((source[13].xyzx)*(source[24].yyyy)).xyz;
    // 198: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 199: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 200: mad r10.xyz, r4.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r10.xyz = ((r4.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 201: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 202: mad r4.w, cb0[11].w, r4.w, l(1.000000)
    r4.w = ((source[11].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 203: mad r10.xyz, cb0[12].wwww, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((source[12].wwww)*(r10.xyzx)+(source[12].xyzx)).xyz;
    // 204: mad r0.yzw, r0.yyzw, r6.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r6.xxyz)+(r10.xxyz)).yzw;
    // 205: mad r0.yzw, r4.wwww, cb0[11].xxyz, r0.yyzw
    r0.yzw = ((r4.wwww)*(source[11].xxyz)+(r0.yyzw)).yzw;
    // 206: mad r0.yzw, r1.xxyz, r12.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 207: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 208: mul r1.x, r3.w, r1.x
    r1.x = ((r3.wwww)*(r1.xxxx)).x;
    // 209: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 210: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 211: mul r1.y, r1.y, l(1.500000)
    r1.y = ((r1.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 212: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 213: mul r6.xyz, r1.yyyy, cb0[14].xyzx
    r6.xyz = ((r1.yyyy)*(source[14].xyzx)).xyz;
    // 214: movc r1.xyz, r1.xxxx, l(0,0,0,0), r6.xyzx
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xyzx)).xyz;
    // 215: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 216: add r0.yzw, r0.yyzw, cb0[3].xxyz
    r0.yzw = ((r0.yyzw)+(source[3].xxyz)).yzw;
    // 217: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 218: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 219: div r6.xyz, r9.xyzx, r1.yyyy
    r6.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 220: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 221: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 222: mul r1.z, |r1.y|, |r1.y|
    r1.z = ((abs(r1.yyyy))*(abs(r1.yyyy))).z;
    // 223: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 224: mul r1.z, r1.z, |r1.y|
    r1.z = ((r1.zzzz)*(abs(r1.yyyy))).z;
    // 225: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 226: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 227: add r1.z, r1.y, l(-0.027778)
    r1.z = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 228: mad r1.y, r1.y, r1.z, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 229: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 230: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 231: mul r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)*(r2.wwww)).x;
    // 232: mad r1.xyz, r1.xxxx, r2.xyzx, -r13.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 233: mad r1.xyz, r0.xxxx, r1.xyzx, r13.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 234: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 235: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 236: mad r1.xyz, cb0[19].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 237: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 238: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 239: mad r1.xyz, cb0[19].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 240: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 241: add r0.x, -cb0[4].w, l(1.000000)
    r0.x = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 242: mul r0.x, r0.x, cb0[23].z
    r0.x = ((r0.xxxx)*(source[23].zzzz)).x;
    // 243: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 244: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 245: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 246: mul r2.x, cb0[4].z, l(1.500000)
    r2.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 247: mul r0.x, r0.x, r2.x
    r0.x = ((r0.xxxx)*(r2.xxxx)).x;
    // 248: mad r0.x, r0.x, l(0.500000), cb0[4].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).x;
    // 249: add r2.x, -r1.w, cb0[4].x
    r2.x = ((-(r1.wwww))+(source[4].xxxx)).x;
    // 250: mul r2.z, r2.x, l(0.125000)
    r2.z = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 251: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 252: mul r4.x, r3.w, l(0.125000)
    r4.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 253: mul r2.y, cb0[4].y, cb0[15].y
    r2.y = ((source[4].yyyy)*(source[15].yyyy)).y;
    // 254: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 255: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 256: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 257: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 258: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r2.xyzw = (ArtistNativeSample6((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 259: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 260: mul r0.x, r1.w, r2.w
    r0.x = ((r1.wwww)*(r2.wwww)).x;
    // 261: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 262: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 263: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 264: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 265: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 266: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 267: mad r2.xy, cb0[16].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[16].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 268: mul r0.x, cb0[16].y, cb0[23].z
    r0.x = ((source[16].yyyy)*(source[23].zzzz)).x;
    // 269: mul r0.x, r0.x, l(0.628319)
    r0.x = ((r0.xxxx)*(float4(0.628319,0.628319,0.628319,0.628319))).x;
    // 270: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 271: mul r4.y, r0.x, l(0.020000)
    r4.y = ((r0.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 272: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 273: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 274: mul r1.w, cb0[16].x, l(0.001000)
    r1.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 275: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 276: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 277: dp2 r1.w, cb0[17].xyxx, r2.xyxx
    r1.w = (dot((source[17].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 278: dp2 r2.y, cb0[18].xyxx, r2.xyxx
    r2.y = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 279: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 280: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 281: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r2.xyzw = (ArtistNativeSample6((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 282: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 283: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 284: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 285: mul_sat r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (saturate((r0.xxxx)*(r2.xyzx))).xyz;
    // 286: mad r4.xyz, cb0[16].zzzz, r2.xyzx, -r1.xyzx
    r4.xyz = ((source[16].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 287: mul r2.xyz, r2.xyzx, cb0[16].zzzz
    r2.xyz = ((r2.xyzx)*(source[16].zzzz)).xyz;
    // 288: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 289: mul r0.x, r0.x, l(3.000000)
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 290: mad r1.xyz, r0.xxxx, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 291: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 292: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 293: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 294: mul r2.xyz, r0.xxxx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 295: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 296: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 297: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 298: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 299: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 300: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 301: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 302: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 303: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 304: mad r0.xyz, r3.xyzx, r1.xyzx, r0.yzwy
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 305: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 307: mad o0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    output.xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 309: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_me_ap_02_5_ts_tr: f3e012370cd62d4f9a62c9c2d507b775; selected map 3f47e48618d77fb4eb098e3fb3db46f5a5252affc813ce1120fd377c1569187b.
float4 ArtistNative2840(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_floorstrm_01_10_ad: d830cb081fe2c54bbf386511e83c8d42; selected map d0ce068b1e75aad6ab6c6bb9730f9484c34db9a658e2fe7cd5eba959f3d28557.
float4 ArtistNative2841(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = input.dynamicParameter;
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[6].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[10].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
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
    // 43: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 44: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 45: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 46: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 47: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_05_12_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative2842(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_rgbline_01_ad: 5ea6abf36654c94eae8b523062def76d; selected map ddfa6780fd232822c32a57bef702012872510d7c744aec89366ef16885fdc498.
float4 ArtistNative2843(ARTIST_NATIVE_INPUT input)
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t1.xywz, s0, l(0.000000)
    r0.xyw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 7: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 8: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 9: mul r1.x, r1.x, cb0[2].w
    r1.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 10: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 11: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 12: movc r0.z, r0.z, l(0), |r1.x|
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).z;
    // 13: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 14: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 15: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 16: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 17: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 18: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 19: mul r1.xyz, r0.xywx, cb0[2].yyyy
    r1.xyz = ((r0.xywx)*(source[2].yyyy)).xyz;
    // 20: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyw, -cb0[2].yyyy, r0.xyxw, r1.wwww
    r0.xyw = ((-(source[2].yyyy))*(r0.xyxw)+(r1.wwww)).xyw;
    // 22: mad r0.xyw, cb0[2].zzzz, r0.xyxw, r1.xyxz
    r0.xyw = ((source[2].zzzz)*(r0.xyxw)+(r1.xyxz)).xyw;
    // 23: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 24: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 25: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 26: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// wp_mn_ppct_00_mi: ab0d30ff2ccc914b84c29564ae10ccd8; selected map af1969772b85aad5e259d978c8889080dab4b473c0e0cb46c31f93230975ce1b.
float4 ArtistNative2844(ARTIST_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=0.f; // Absolute source world position needs no pre-view translation.
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[27]=float4(input.skyUpperColor,0.f);
    source[28]=float4(input.skyLowerColor,0.f);
    source[29]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_ArtistSourceMaterialParameters[18u];
    source[3] = g_ArtistSourceMaterialParameters[19u];
    source[4] = g_ArtistSourceMaterialParameters[8u];
    source[5] = g_ArtistSourceMaterialParameters[9u];
    source[6] = g_ArtistSourceMaterialParameters[10u];
    source[7] = g_ArtistSourceMaterialParameters[11u];
    source[8] = g_ArtistSourceMaterialParameters[13u];
    source[9] = g_ArtistSourceMaterialParameters[12u];
    source[10] = g_ArtistSourceMaterialParameters[15u];
    source[11] = g_ArtistSourceMaterialParameters[16u];
    source[12] = g_ArtistSourceMaterialParameters[21u];
    source[13] = g_ArtistSourceMaterialParameters[6u];
    source[14] = g_ArtistSourceMaterialParameters[7u];
    source[15] = g_ArtistSourceMaterialParameters[14u];
    source[16] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[17] = g_ArtistSourceMaterialParameters[20u];
    source[18] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[19] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[20] = g_ArtistSourceMaterialParameters[17u];
    source[21].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[21].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[21].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[21].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[22].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[22].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[22].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[22].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[23].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[23].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[23].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[23].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[24].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[24].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[24].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[24].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[25].x = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[25].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[25].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[25].w = ((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[26].x = (((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[26].y = (sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[26].z = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[26].w = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
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
    // 15: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 16: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 17: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 18: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 19: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 20: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 21: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 22: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 23: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 24: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 25: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 26: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 27: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 28: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 29: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 31: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 33: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: mul r0.w, r0.w, cb0[22].x
    r0.w = ((r0.wwww)*(source[22].xxxx)).w;
    // 35: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 36: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 38: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 39: mul r0.w, r0.w, cb0[22].y
    r0.w = ((r0.wwww)*(source[22].yyyy)).w;
    // 40: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 41: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 42: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 43: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 45: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 46: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 47: mul r3.xy, r0.ywyy, cb0[21].xxxx
    r3.xy = ((r0.ywyy)*(source[21].xxxx)).xy;
    // 48: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 50: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 51: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[21].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[21].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 54: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 55: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 56: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 57: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 60: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 61: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 64: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 65: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 66: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 67: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 68: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 69: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 70: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 71: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 72: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 74: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 75: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 76: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 77: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 78: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 79: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 80: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 81: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 82: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 83: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 84: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 86: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = (ArtistNativeSample4((r0.ywyy).xy, (r0.xxxx).x, true).xywz).xyw;
    // 88: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 89: rcp r1.w, cb0[22].z
    r1.w = (1.0/(source[22].zzzz)).w;
    // 90: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 91: mul r6.xyz, r6.xyzx, cb0[22].zzzz
    r6.xyz = ((r6.xyzx)*(source[22].zzzz)).xyz;
    // 92: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 93: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 94: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 95: mad r6.xyz, r6.xyzx, cb0[22].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[22].zzzz)+(r10.xyzx)).xyz;
    // 96: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 97: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 98: add r1.w, cb0[22].z, l(1.000000)
    r1.w = ((source[22].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 100: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 102: mad r6.xyz, r2.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 103: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 104: mul r0.xyw, r0.xyxw, cb0[22].wwww
    r0.xyw = ((r0.xyxw)*(source[22].wwww)).xyw;
    // 105: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 106: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 107: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 108: dp3 r1.w, r3.xyzx, r4.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 109: mul_sat r2.w, r1.w, cb0[23].y
    r2.w = (saturate((r1.wwww)*(source[23].yyyy))).w;
    // 110: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul_sat r3.w, r4.z, cb0[23].y
    r3.w = (saturate((r4.zzzz)*(source[23].yyyy))).w;
    // 113: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: add_sat r3.w, r3.w, -cb0[23].z
    r3.w = (saturate((r3.wwww)+(-(source[23].zzzz)))).w;
    // 115: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 116: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 117: mul r4.w, r4.w, cb0[23].w
    r4.w = ((r4.wwww)*(source[23].wwww)).w;
    // 118: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 119: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 120: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 121: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 123: mad r2.xyz, cb0[21].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 124: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 126: mad r2.xyz, cb0[21].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[21].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 127: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 129: mul r6.xyz, r6.xyzx, cb0[23].xxxx
    r6.xyz = ((r6.xyzx)*(source[23].xxxx)).xyz;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r10.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 131: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 132: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 133: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 134: mad r2.xyz, r3.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 135: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 136: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 137: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 138: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 139: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 141: mul r3.w, r3.w, cb0[24].x
    r3.w = ((r3.wwww)*(source[24].xxxx)).w;
    // 142: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 143: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: div r4.w, cb0[24].y, r4.w
    r4.w = ((source[24].yyyy)/(r4.wwww)).w;
    // 147: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 148: mul r6.xyz, r0.xywx, r4.wwww
    r6.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 149: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 151: mad r1.xyz, cb0[21].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[21].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 152: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 154: mad r1.xyz, cb0[21].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[21].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 155: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 156: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 157: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 158: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 159: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 160: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r10.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r10.xywx))).xyz;
    // 161: mad r10.xyz, r10.zzzz, r11.xyzx, r10.xywx
    r10.xyz = ((r10.zzzz)*(r11.xyzx)+(r10.xywx)).xyz;
    // 162: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 164: mad r10.xyz, cb0[21].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[21].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 165: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 166: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 167: mad r10.xyz, cb0[21].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[21].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 171: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 172: mul r12.xyz, r1.xyzx, r10.xyzx
    r12.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 173: mad r1.xyz, r10.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 174: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 175: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 176: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r4.w, r4.w, cb0[24].z
    r4.w = ((r4.wwww)*(source[24].zzzz)).w;
    // 178: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 179: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 180: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 181: mul r6.xyz, r0.xywx, r2.yyyy
    r6.xyz = ((r0.xywx)*(r2.yyyy)).xyz;
    // 182: dp3 r2.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: mad r10.xyz, -r2.yyyy, r0.xywx, r2.zzzz
    r10.xyz = ((-(r2.yyyy))*(r0.xywx)+(r2.zzzz)).xyz;
    // 184: mad r6.xyz, cb0[21].yyyy, r10.xyzx, r6.xyzx
    r6.xyz = ((source[21].yyyy)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 185: dp3 r2.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 186: add r10.xyz, -r6.xyzx, r2.yyyy
    r10.xyz = ((-(r6.xyzx))+(r2.yyyy)).xyz;
    // 187: mad r6.xyz, cb0[21].zzzz, r10.xyzx, r6.xyzx
    r6.xyz = ((source[21].zzzz)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 188: dp3 r2.y, r1.xyzx, r1.xyzx
    r2.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 189: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 190: div r1.xyz, r1.xyzx, r2.yyyy
    r1.xyz = ((r1.xyzx)/(r2.yyyy)).xyz;
    // 191: dp3 r2.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 192: add r10.xyz, -r1.xyzx, r2.yyyy
    r10.xyz = ((-(r1.xyzx))+(r2.yyyy)).xyz;
    // 193: add r1.xyz, r1.xyzx, -r10.xyzx
    r1.xyz = ((r1.xyzx)+(-(r10.xyzx))).xyz;
    // 194: mul r10.xyz, cb0[14].xyzx, cb0[25].xxxx
    r10.xyz = ((source[14].xyzx)*(source[25].xxxx)).xyz;
    // 195: mul r10.xyz, r10.xyzx, cb0[26].wwww
    r10.xyz = ((r10.xyzx)*(source[26].wwww)).xyz;
    // 196: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 197: mad r13.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r13.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 198: add r2.y, r2.w, l(-1.000000)
    r2.y = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 199: mad r2.y, cb0[12].w, r2.y, l(1.000000)
    r2.y = ((source[12].wwww)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: mad r13.xyz, cb0[13].wwww, r13.xyzx, cb0[13].xyzx
    r13.xyz = ((source[13].wwww)*(r13.xyzx)+(source[13].xyzx)).xyz;
    // 201: mad r1.xyz, r1.xyzx, r10.xyzx, r13.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)+(r13.xyzx)).xyz;
    // 202: mad r1.xyz, r2.yyyy, cb0[12].xyzx, r1.xyzx
    r1.xyz = ((r2.yyyy)*(source[12].xyzx)+(r1.xyzx)).xyz;
    // 203: mad r1.xyz, r6.xyzx, r11.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 204: add r2.y, -|r4.z|, l(1.000000)
    r2.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 205: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 206: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 207: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 208: mul r2.y, r2.y, l(1.500000)
    r2.y = ((r2.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 209: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 210: mul r2.yzw, r2.yyyy, cb0[15].xxyz
    r2.yzw = ((r2.yyyy)*(source[15].xxyz)).yzw;
    // 211: movc r2.yzw, r1.wwww, l(0,0,0,0), r2.yyzw
    r2.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyzw)).yzw;
    // 212: add r1.xyz, r1.xyzx, r2.yzwy
    r1.xyz = ((r1.xyzx)+(r2.yzwy)).xyz;
    // 213: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 214: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 215: sqrt r2.y, r1.w
    r2.y = (sqrt(r1.wwww)).y;
    // 216: div r2.yzw, r9.xxyz, r2.yyyy
    r2.yzw = ((r9.xxyz)/(r2.yyyy)).yzw;
    // 217: dp3 r2.y, r2.yzwy, r4.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).y;
    // 218: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 219: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 220: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 221: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 222: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 223: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 224: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 225: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 226: div_sat r1.w, r2.y, r1.w
    r1.w = (saturate((r2.yyyy)/(r1.wwww))).w;
    // 227: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 228: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 229: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 230: mad r0.xyz, r3.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 231: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 232: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 233: mad r0.xyz, cb0[21].yyyy, r2.yzwy, r0.xyzx
    r0.xyz = ((source[21].yyyy)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 234: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 235: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 236: mad r0.xyz, cb0[21].zzzz, r2.yzwy, r0.xyzx
    r0.xyz = ((source[21].zzzz)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 237: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 238: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r0.w, r0.w, cb0[25].y
    r0.w = ((r0.wwww)*(source[25].yyyy)).w;
    // 240: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 241: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 242: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 244: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 245: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 246: add r1.w, -r2.x, cb0[3].x
    r1.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 247: mul r4.z, r1.w, l(0.125000)
    r4.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 248: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 249: mul r6.x, r1.w, l(0.125000)
    r6.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 250: mul r4.y, cb0[3].y, cb0[16].y
    r4.y = ((source[3].yyyy)*(source[16].yyyy)).y;
    // 251: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 252: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 253: add r2.yz, r4.xxyx, r6.xxyx
    r2.yz = ((r4.xxyx)+(r6.xxyx)).yz;
    // 254: add r2.yz, r2.yyzy, r4.zzwz
    r2.yz = ((r2.yyzy)+(r4.zzwz)).yz;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.yzyy, t6.xyzw, s5, l(0.000000)
    r4.xyzw = (ArtistNativeSample5((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 256: mul r2.yzw, r0.wwww, r4.xxyz
    r2.yzw = ((r0.wwww)*(r4.xxyz)).yzw;
    // 257: mul r0.w, r2.x, r4.w
    r0.w = ((r2.xxxx)*(r4.wwww)).w;
    // 258: mad r2.xyz, r2.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 259: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 260: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 261: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 262: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 263: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 264: mad r2.xy, cb0[17].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 265: mul r0.w, cb0[17].y, cb0[25].y
    r0.w = ((source[17].yyyy)*(source[25].yyyy)).w;
    // 266: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 267: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 268: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 269: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 271: mul r1.w, cb0[17].x, l(0.001000)
    r1.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 272: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 273: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 274: dp2 r1.w, cb0[18].xyxx, r2.xyxx
    r1.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 275: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 276: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 277: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 278: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r2.xyzw = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 279: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 280: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 281: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 282: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 283: mad r4.xyz, cb0[17].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[17].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 284: mul r2.xyz, r2.xyzx, cb0[17].zzzz
    r2.xyz = ((r2.xyzx)*(source[17].zzzz)).xyz;
    // 285: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 286: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 287: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 288: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 289: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 290: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 291: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 292: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 293: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 294: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 295: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 296: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 297: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 298: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 299: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 300: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 301: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 302: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 304: mad o0.xyz, r0.xyzx, cb0[29].xyzx, r1.xyzx
    output.xyz = ((r0.xyzx)*(source[29].xyzx)+(r1.xyzx)).xyz;
    // 306: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_lbeam_01_05_ad: e916b8b428ce2d4f9e6c9c1f921545f6; selected map c6dd09439718068dd0b1df175b964425adda3531defec8efa86787f044491394.
float4 ArtistNative2845(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.x, v2.x, l(-0.500000)
    r0.x = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 2: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 3: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 4: mul r0.y, r0.x, v2.y
    r0.y = ((r0.xxxx)*(v2.yyyy)).y;
    // 5: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 6: mad r0.x, r0.x, l(0.500000), r0.y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.yyyy)).x;
    // 7: sqrt r0.y, v2.y
    r0.y = (sqrt(v2.yyyy)).y;
    // 8: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 9: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 10: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 12: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 13: mul_sat r0.y, r0.y, cb0[2].y
    r0.y = (saturate((r0.yyyy)*(source[2].yyyy))).y;
    // 14: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 15: add r0.y, -v2.y, l(1.000000)
    r0.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 16: mul_sat r0.y, r0.y, cb0[2].z
    r0.y = (saturate((r0.yyyy)*(source[2].zzzz))).y;
    // 17: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 18: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[2].w
    r0.z = ((r0.zzzz)*(source[2].wwww)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_57_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative2846(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_gl_01_2_tr: 653a5bb9cb1a754193f542b0fda91232; selected map b9a00a284058a01b21ea0d6fd6eec705cebc4a6f9f17beb3a1ba17da3210361f.
float4 ArtistNative2847(ARTIST_NATIVE_INPUT input)
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
// fx_b_pa_ht_01_2_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative2848(ARTIST_NATIVE_INPUT input)
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
// fx_c_pa_ring_03_ad: d63ba436efa9024f8bf9d60e1f2b3916; selected map 04d2f970e313acccf98355cbddace55c579d9114ce14bc9409c9cff82569e6ff.
float4 ArtistNative2849(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.200000003, 0.0, 0.0, 0.0))),1u);
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
    // 10: mul_sat r0.x, r0.x, l(0.022222)
    r0.x = (saturate((r0.xxxx)*(float4(0.022222,0.022222,0.022222,0.022222)))).x;
    // 11: add r0.yz, v2.xxyx, cb0[2].xxyx
    r0.yz = ((v2.xxyx)+(source[2].xxyx)).yz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 13: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 1.400000, 1.400000), cb0[3].xxxy
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,1.400000,1.400000))+(source[3].xxxy)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.xzyw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 15: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 16: mad r0.yz, r0.yyyy, l(0.000000, 0.020000, 0.020000, 0.000000), v2.xxyx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.020000,0.020000,0.000000))+(v2.xxyx)).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t1.xyzw, s2, l(0.000000)
    r1.xyzw = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 18: mul r0.y, r1.w, v3.w
    r0.y = ((r1.wwww)*(v3.wwww)).y;
    // 19: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 20: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 21: mul r0.z, |r0.y|, |r0.y|
    r0.z = ((abs(r0.yyyy))*(abs(r0.yyyy))).z;
    // 22: mul r0.z, r0.z, |r0.y|
    r0.z = ((r0.zzzz)*(abs(r0.yyyy))).z;
    // 23: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 24: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 26: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 27: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 28: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 29: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_018_ad: c9cf79250d787c4b8faa16cdb042f143; selected map e8bae060e5bc8394b3e0755ed4d89824b43afc42dcdbf4d111327e2960a7236f.
float4 ArtistNative2850(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
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
    r1.x = (sin(r0.yyyy)).x; r2.x = (cos(r0.yyyy)).x;
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_sqc_01_06_dt_tr: 0d047bda09fdab409b233d70ee0f2dd8; selected map fdf5be0aacb0bd0229e4e2c301d24d4b9bee7d3a6b6d174dfc1579d638d2bf30.
float4 ArtistNative2851(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_dissolve_01_06_ma: dd0a0a865b8d784d91c2062555a084fa; selected map 0ea515df7dd0d38f80da6e74d395856ed834996fbbd785a2d5e32cad6d164a51.
float4 ArtistNative2852(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0] = g_ArtistSourceMaterialParameters[7u];
    source[1] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[2] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,g_ArtistSourceMaterialParameters[3u].xxxx,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[5] = g_ArtistSourceMaterialParameters[6u];
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].xxxx,g_ArtistSourceMaterialParameters[0u].yyyy,1u);
    source[7] = g_ArtistSourceMaterialParameters[4u];
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[9] = g_ArtistSourceMaterialParameters[5u];
    source[10].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: mov_sat r0.x, v2.w
    r0.x = (saturate(v2.wwww)).x;
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
    // 9: mul r0.xy, -r0.xyxx, cb0[6].xyxx
    r0.xy = ((-(r0.xyxx))*(source[6].xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 12: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 13: mad r0.xyz, cb0[11].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[11].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 14: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 15: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, cb0[11].wwww
    r0.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 17: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 18: mul r1.xy, v4.xyxx, cb0[8].xyxx
    r1.xy = ((v4.xyxx)*(source[8].xyxx)).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 22: mad r1.xyz, cb0[12].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 23: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 24: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 25: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 26: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 27: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 28: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 29: mul r2.xyz, cb0[7].xyzx, cb0[7].wwww
    r2.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 30: mad r0.xyz, r0.xyzx, r2.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 31: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 32: dp2 r2.x, cb0[1].xyxx, r1.xyxx
    r2.x = (dot((source[1].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 33: dp2 r2.y, cb0[2].xyxx, r1.xyxx
    r2.y = (dot((source[2].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 34: add r1.xy, r2.xyxx, cb0[3].xyxx
    r1.xy = ((r2.xyxx)+(source[3].xyxx)).xy;
    // 35: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 36: mul r1.xy, r1.xyxx, cb0[4].xyxx
    r1.xy = ((r1.xyxx)*(source[4].xyxx)).xy;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 38: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 39: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 40: mul r1.x, r1.x, cb0[10].z
    r1.x = ((r1.xxxx)*(source[10].zzzz)).x;
    // 41: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 42: mul r1.x, r1.x, cb0[10].w
    r1.x = ((r1.xxxx)*(source[10].wwww)).x;
    // 43: mul r1.yzw, cb0[5].xxyz, cb0[5].wwww
    r1.yzw = ((source[5].xxyz)*(source[5].wwww)).yzw;
    // 44: mul r1.xyz, r1.yzwy, r1.xxxx
    r1.xyz = ((r1.yzwy)*(r1.xxxx)).xyz;
    // 45: movc r1.xyz, r0.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 46: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 47: mad o0.xyz, r0.xyzx, v2.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)*(v2.xyzx)+(source[0].xyzx)).xyz;
    // 48: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_e_pa_ht_05_tr: 40d8570622405045b02163a3ce2f2808; selected map 53d79af6d22eb85b639fef8bdb70c8235812fa8ff5320c16b04c9958b9396302.
float4 ArtistNative2853(ARTIST_NATIVE_INPUT input)
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_card_01_1: d4219dbd58be09438f811c49d92a1132; selected map bcf55c3ef93713de7e955372a7272caf06c044cb20add281dacccecb6466407b.
float4 ArtistNative2854(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 3: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// mn_rpct_05-4_mi: e5ff54c5c354204e951b91b524341cb3; selected map 75a4cf8835d876e19fa5db39a495f7027b9189e9c0a930f17eb7ad3fc48b55c1.
float4 ArtistNative2855(ARTIST_NATIVE_INPUT input)
{
    float4 source[26]; [unroll] for (uint i=0u; i<26u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=0.f; // Absolute source world position needs no pre-view translation.
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[23]=float4(input.skyUpperColor,0.f);
    source[24]=float4(input.skyLowerColor,0.f);
    source[25]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_ArtistSourceMaterialParameters[14u];
    source[3] = g_ArtistSourceMaterialParameters[15u];
    source[4] = g_ArtistSourceMaterialParameters[8u];
    source[5] = g_ArtistSourceMaterialParameters[10u];
    source[6] = g_ArtistSourceMaterialParameters[9u];
    source[7] = g_ArtistSourceMaterialParameters[12u];
    source[8] = g_ArtistSourceMaterialParameters[13u];
    source[9] = g_ArtistSourceMaterialParameters[17u];
    source[10] = g_ArtistSourceMaterialParameters[6u];
    source[11] = g_ArtistSourceMaterialParameters[7u];
    source[12] = g_ArtistSourceMaterialParameters[11u];
    source[13] = ArtistNativeAppend(g_ArtistSourceMaterialTime.xxxx,g_ArtistSourceMaterialTime.xxxx,1u);
    source[14] = g_ArtistSourceMaterialParameters[16u];
    source[15] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[16] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[17].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[18].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[18].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[19].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[19].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[20].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[20].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[20].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[21].x = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[21].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[21].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[21].w = ((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[22].x = (((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[22].y = (sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[22].z = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[22].w = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[0u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r0.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 4: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: mul r1.xyz, v7.yyyy, cb1[1].xywx
    r1.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 8: mad r1.xyz, cb1[0].xywx, v7.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v7.xxxx)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[2].xywx, v7.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v7.zzzz)+(r1.xyzx)).xyz;
    // 10: mad r1.xyz, cb1[3].xywx, v7.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v7.wwww)+(r1.xyzx)).xyz;
    // 11: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 14: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 15: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 16: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 17: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 18: max r0.x, r0.x, r1.x
    r0.x = (max(r0.xxxx,r1.xxxx)).x;
    // 19: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 20: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 21: rcp r1.x, |r0.x|
    r1.x = (1.0/(abs(r0.xxxx))).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 25: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r1.z, r1.z, cb0[18].x
    r1.z = ((r1.zzzz)*(source[18].xxxx)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 30: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 31: mul r1.z, r1.z, cb0[18].y
    r1.z = ((r1.zzzz)*(source[18].yyyy)).z;
    // 32: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 33: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 34: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 35: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).xz;
    // 37: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 38: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 39: mul r3.xy, r1.xzxx, cb0[17].xxxx
    r3.xy = ((r1.xzxx)*(source[17].xxxx)).xy;
    // 40: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: add r1.xzw, -r3.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r3.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 45: mad r1.xzw, cb0[17].wwww, r1.xxzw, r3.xxyz
    r1.xzw = ((source[17].wwww)*(r1.xxzw)+(r3.xxyz)).xzw;
    // 46: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 47: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 48: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 49: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 50: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 51: mul r4.xyz, r2.wwww, v0.xyzx
    r4.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 52: dp3 r5.x, r4.xyzx, r1.xzwx
    r5.x = (dot((r4.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 53: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 54: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 55: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 56: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 57: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 58: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 59: dp3 r5.y, r7.xyzx, r1.xzwx
    r5.y = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 60: dp3 r5.z, r6.xyzx, r1.xzwx
    r5.z = (dot((r6.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 61: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 62: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 63: mul r8.xyz, r1.xxxx, v5.xyzx
    r8.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 64: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 65: dp3 r9.y, r7.xyzx, r8.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 66: dp3 r9.x, r4.xyzx, r8.xyzx
    r9.x = (dot((r4.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 67: dp3 r9.z, r6.xyzx, r8.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 68: dp3 r2.w, r5.xyzx, r9.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 69: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 70: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 71: mov r5.w, -r5.x
    r5.w = (-(r5.xxxx)).w;
    // 72: dp2 r2.w, r5.ywyy, r5.ywyy
    r2.w = (dot((r5.ywyy).xy,(r5.ywyy).xy).xxxx).w;
    // 73: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 74: div r5.xy, r5.ywyy, r2.wwww
    r5.xy = ((r5.ywyy)/(r2.wwww)).xy;
    // 75: mad r2.w, -r5.z, l(0.250000), l(0.250000)
    r2.w = ((-(r5.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 76: add r3.w, r5.z, l(1.000000)
    r3.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 78: mad r5.xy, r2.wwww, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 79: sample_l_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s3, r0.x
    r5.xyz = (ArtistNativeSample3((r5.xyxx).xy, (r0.xxxx).x, true).xyzw).xyz;
    // 80: log r9.xyz, r5.xyzx
    r9.xyz = (log2(r5.xyzx)).xyz;
    // 81: rcp r0.x, cb0[18].z
    r0.x = (1.0/(source[18].zzzz)).x;
    // 82: mul r10.xyz, r9.xyzx, r0.xxxx
    r10.xyz = ((r9.xyzx)*(r0.xxxx)).xyz;
    // 83: mul r9.xyz, r9.xyzx, cb0[18].zzzz
    r9.xyz = ((r9.xyzx)*(source[18].zzzz)).xyz;
    // 84: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 85: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 86: mul r10.xyz, r0.xxxx, r10.xyzx
    r10.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 87: mad r9.xyz, r9.xyzx, cb0[18].zzzz, r10.xyzx
    r9.xyz = ((r9.xyzx)*(source[18].zzzz)+(r10.xyzx)).xyz;
    // 88: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 89: mul r5.xyz, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 90: add r0.x, cb0[18].z, l(1.000000)
    r0.x = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 92: dp3 r0.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r5.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r5.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 94: mad r5.xyz, r3.wwww, r5.xyzx, cb0[7].xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(source[7].xyzx)).xyz;
    // 95: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 96: mul r5.xyz, r5.xyzx, cb0[18].wwww
    r5.xyz = ((r5.xyzx)*(source[18].wwww)).xyz;
    // 97: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 98: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 99: mad r2.xyz, cb0[17].yyyy, r9.xyzx, r2.xyzx
    r2.xyz = ((source[17].yyyy)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 100: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 102: mad r2.xyz, cb0[17].zzzz, r9.xyzx, r2.xyzx
    r2.xyz = ((source[17].zzzz)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 103: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 104: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 105: mul r9.xyz, r9.xyzx, cb0[19].xxxx
    r9.xyz = ((r9.xyzx)*(source[19].xxxx)).xyz;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r10.xyzw = (ArtistNativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 107: add r0.x, r10.y, r10.x
    r0.x = ((r10.yyyy)+(r10.xxxx)).x;
    // 108: add r0.x, r10.z, r0.x
    r0.x = ((r10.zzzz)+(r0.xxxx)).x;
    // 109: add_sat r0.x, r10.w, r0.x
    r0.x = (saturate((r10.wwww)+(r0.xxxx))).x;
    // 110: mad r2.xyz, r0.xxxx, r9.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 111: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 112: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 113: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 114: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 115: dp3 r0.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 117: mul r0.x, r0.x, cb0[20].x
    r0.x = ((r0.xxxx)*(source[20].xxxx)).x;
    // 118: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 119: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 120: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 122: div r2.w, cb0[20].y, r2.w
    r2.w = ((source[20].yyyy)/(r2.wwww)).w;
    // 123: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 124: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 125: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 126: dp3 r3.w, r3.xyzx, r8.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 127: mul_sat r4.w, r3.w, cb0[19].y
    r4.w = (saturate((r3.wwww)*(source[19].yyyy))).w;
    // 128: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul_sat r5.w, r8.z, cb0[19].y
    r5.w = (saturate((r8.zzzz)*(source[19].yyyy))).w;
    // 131: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: add_sat r5.w, r5.w, -cb0[19].z
    r5.w = (saturate((r5.wwww)+(-(source[19].zzzz)))).w;
    // 133: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 134: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 135: mul r6.w, r6.w, cb0[19].w
    r6.w = ((r6.wwww)*(source[19].wwww)).w;
    // 136: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 137: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 138: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 139: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 140: mul r9.xyz, r5.xyzx, r2.wwww
    r9.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 141: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r10.xyz, -r0.yzwy, r2.wwww
    r10.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 143: mad r0.yzw, cb0[17].yyyy, r10.xxyz, r0.yyzw
    r0.yzw = ((source[17].yyyy)*(r10.xxyz)+(r0.yyzw)).yzw;
    // 144: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: add r10.xyz, -r0.yzwy, r2.wwww
    r10.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 146: mad r0.yzw, cb0[17].zzzz, r10.xxyz, r0.yyzw
    r0.yzw = ((source[17].zzzz)*(r10.xxyz)+(r0.yyzw)).yzw;
    // 147: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 148: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 150: mad r10.xyz, cb0[17].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 151: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 152: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 153: mad r10.xyz, cb0[17].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 154: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 157: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 158: mul r12.xyz, r0.yzwy, r10.xyzx
    r12.xyz = ((r0.yzwy)*(r10.xyzx)).xyz;
    // 159: mad r0.yzw, r10.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r10.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 160: mul r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)*(r12.xyzx)).xyz;
    // 161: mad r2.xyz, r2.xyzx, r9.xyzx, -r5.xyzx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 162: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 164: mad r2.xyz, r2.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 165: frc r2.w, cb0[3].x
    r2.w = (frac(source[3].xxxx)).w;
    // 166: add r5.x, -r2.w, l(1.000000)
    r5.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 167: mul r5.yzw, r2.xxyz, r5.xxxx
    r5.yzw = ((r2.xxyz)*(r5.xxxx)).yzw;
    // 168: dp3 r6.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 169: mad r9.xyz, -r5.xxxx, r2.xyzx, r6.wwww
    r9.xyz = ((-(r5.xxxx))*(r2.xyzx)+(r6.wwww)).xyz;
    // 170: mad r5.xyz, cb0[17].yyyy, r9.xyzx, r5.yzwy
    r5.xyz = ((source[17].yyyy)*(r9.xyzx)+(r5.yzwy)).xyz;
    // 171: dp3 r5.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r9.xyz, -r5.xyzx, r5.wwww
    r9.xyz = ((-(r5.xyzx))+(r5.wwww)).xyz;
    // 173: mad r5.xyz, cb0[17].zzzz, r9.xyzx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 174: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 175: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 176: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 177: dp3 r5.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r9.xyz, -r0.yzwy, r5.wwww
    r9.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 179: add r0.yzw, r0.yyzw, -r9.xxyz
    r0.yzw = ((r0.yyzw)+(-(r9.xxyz))).yzw;
    // 180: mul r9.xyz, cb0[11].xyzx, cb0[21].xxxx
    r9.xyz = ((source[11].xyzx)*(source[21].xxxx)).xyz;
    // 181: mul r9.xyz, r9.xyzx, cb0[22].wwww
    r9.xyz = ((r9.xyzx)*(source[22].wwww)).xyz;
    // 182: mul r9.xyz, r4.wwww, r9.xyzx
    r9.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 183: mad r10.xyz, r4.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r10.xyz = ((r4.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 184: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 185: mad r4.w, cb0[9].w, r4.w, l(1.000000)
    r4.w = ((source[9].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: mad r10.xyz, cb0[10].wwww, r10.xyzx, cb0[10].xyzx
    r10.xyz = ((source[10].wwww)*(r10.xyzx)+(source[10].xyzx)).xyz;
    // 187: mad r0.yzw, r0.yyzw, r9.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r9.xxyz)+(r10.xxyz)).yzw;
    // 188: mad r0.yzw, r4.wwww, cb0[9].xxyz, r0.yyzw
    r0.yzw = ((r4.wwww)*(source[9].xxyz)+(r0.yyzw)).yzw;
    // 189: mad r0.yzw, r5.xxyz, r11.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 190: add r4.w, -|r8.z|, l(1.000000)
    r4.w = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 192: log r4.w, |r3.w|
    r4.w = (log2(abs(r3.wwww))).w;
    // 193: lt r3.w, |r3.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 194: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 195: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 196: mul r5.xyz, r4.wwww, cb0[12].xyzx
    r5.xyz = ((r4.wwww)*(source[12].xyzx)).xyz;
    // 197: movc r5.xyz, r3.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 198: add r0.yzw, r0.yyzw, r5.xxyz
    r0.yzw = ((r0.yyzw)+(r5.xxyz)).yzw;
    // 199: add r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)+(source[2].xxyz)).yzw;
    // 200: dp3 r3.w, r1.xzwx, r1.xzwx
    r3.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 201: sqrt r4.w, r3.w
    r4.w = (sqrt(r3.wwww)).w;
    // 202: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 203: dp3 r1.x, r1.xzwx, r8.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 204: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 206: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 207: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 208: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 209: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 210: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 211: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 212: div_sat r1.x, r1.x, r3.w
    r1.x = (saturate((r1.xxxx)/(r3.wwww))).x;
    // 213: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 214: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 215: mad r1.xyz, r1.xxxx, r2.xyzx, -r12.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r12.xyzx))).xyz;
    // 216: mad r1.xyz, r0.xxxx, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 217: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 219: mad r1.xyz, cb0[17].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 220: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 221: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 222: mad r1.xyz, cb0[17].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 223: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 224: add r0.x, -cb0[3].w, l(1.000000)
    r0.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 225: mul r0.x, r0.x, cb0[21].y
    r0.x = ((r0.xxxx)*(source[21].yyyy)).x;
    // 226: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 227: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 228: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 229: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 230: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 231: mad r0.x, r0.x, l(0.500000), cb0[3].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 232: add r1.w, -r2.w, cb0[3].x
    r1.w = ((-(r2.wwww))+(source[3].xxxx)).w;
    // 233: mul r5.z, r1.w, l(0.125000)
    r5.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 234: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 235: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 236: mul r5.y, cb0[3].y, cb0[13].y
    r5.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 237: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 238: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 239: add r2.xy, r2.xyxx, r5.xyxx
    r2.xy = ((r2.xyxx)+(r5.xyxx)).xy;
    // 240: add r2.xy, r2.xyxx, r5.zwzz
    r2.xy = ((r2.xyxx)+(r5.zwzz)).xy;
    // 241: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyzw = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 242: mul r2.xyz, r0.xxxx, r5.xyzx
    r2.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 243: mul r0.x, r2.w, r5.w
    r0.x = ((r2.wwww)*(r5.wwww)).x;
    // 244: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 245: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 246: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 247: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 248: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 249: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 250: mad r2.xy, cb0[14].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[14].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 251: mul r0.x, cb0[14].y, cb0[21].y
    r0.x = ((source[14].yyyy)*(source[21].yyyy)).x;
    // 252: mul r0.x, r0.x, l(0.628319)
    r0.x = ((r0.xxxx)*(float4(0.628319,0.628319,0.628319,0.628319))).x;
    // 253: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 254: mul r5.y, r0.x, l(0.020000)
    r5.y = ((r0.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 255: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 256: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 257: mul r1.w, cb0[14].x, l(0.001000)
    r1.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 258: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 259: mad r2.xy, r1.wwww, r2.xyxx, r5.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r5.xyxx)).xy;
    // 260: dp2 r1.w, cb0[15].xyxx, r2.xyxx
    r1.w = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 261: dp2 r2.y, cb0[16].xyxx, r2.xyxx
    r2.y = (dot((source[16].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 262: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 263: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 264: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 265: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 266: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 267: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 268: mul_sat r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (saturate((r0.xxxx)*(r2.xyzx))).xyz;
    // 269: mad r5.xyz, cb0[14].zzzz, r2.xyzx, -r1.xyzx
    r5.xyz = ((source[14].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 270: mul r2.xyz, r2.xyzx, cb0[14].zzzz
    r2.xyz = ((r2.xyzx)*(source[14].zzzz)).xyz;
    // 271: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 272: mul r0.x, r0.x, l(3.000000)
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 273: mad r1.xyz, r0.xxxx, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 274: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 275: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 276: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 277: mul r2.xyz, r0.xxxx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 278: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 279: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 280: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 281: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 282: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 283: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 284: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 285: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 286: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 287: mad r0.xyz, r3.xyzx, r1.xyzx, r0.yzwy
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 288: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 290: mad o0.xyz, r1.xyzx, cb0[25].xyzx, r0.xyzx
    output.xyz = ((r1.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 292: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_pa_spritewave_01_2_tr: 1b614321d13a674ba81d24973bb6b184; selected map 14be505a134258ff472c4ecfa430347520d50d720964438b3b672c805ef17b2f.
float4 ArtistNative2856(ARTIST_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[11u];
    source[2] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[8u].wwww,g_ArtistSourceMaterialParameters[9u].xxxx,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[6] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ArtistSourceMaterialParameters[10u];
    source[11].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[11].y = (cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[16].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[17].z = ((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[17].w = (sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[18].y = (cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[18].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[19].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[19].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[20].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[20].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[20].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[21].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[21].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[21].z = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[21].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    // 1: mul r0.x, cb0[11].w, cb0[13].z
    r0.x = ((source[11].wwww)*(source[13].zzzz)).x;
    // 2: mad r0.x, cb0[13].w, v2.x, r0.x
    r0.x = ((source[13].wwww)*(v2.xxxx)+(r0.xxxx)).x;
    // 3: mul r0.z, v2.y, cb0[14].x
    r0.z = ((v2.yyyy)*(source[14].xxxx)).z;
    // 4: mad r0.y, cb0[11].w, cb0[14].y, r0.z
    r0.y = ((source[11].wwww)*(source[14].yyyy)+(r0.zzzz)).y;
    // 5: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: add r0.y, v4.z, cb0[15].x
    r0.y = ((v4.zzzz)+(source[15].xxxx)).y;
    // 8: mad r0.xy, r0.xxxx, r0.yyyy, cb0[5].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[5].xyxx)).xy;
    // 9: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 10: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 11: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 12: mad r1.xy, cb0[12].zwzz, v4.xxxx, r1.xyxx
    r1.xy = ((source[12].zwzz)*(v4.xxxx)+(r1.xyxx)).xy;
    // 13: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: mul r1.xy, r1.xyxx, cb0[12].xyxx
    r1.xy = ((r1.xyxx)*(source[12].xyxx)).xy;
    // 15: mad r2.x, cb0[11].w, cb0[11].z, r1.x
    r2.x = ((source[11].wwww)*(source[11].zzzz)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[11].w, cb0[13].y, r1.y
    r2.y = ((source[11].wwww)*(source[13].yyyy)+(r1.yyyy)).y;
    // 17: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 18: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 19: dp2 r1.x, cb0[6].xyxx, r0.xyxx
    r1.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 20: dp2 r1.y, cb0[7].xyxx, r0.xyxx
    r1.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 21: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 22: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(-1.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[15].w
    r0.y = ((r0.yyyy)*(source[15].wwww)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 27: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 29: mad r0.y, r0.x, cb0[16].y, r0.y
    r0.y = ((r0.xxxx)*(source[16].yyyy)+(r0.yyyy)).y;
    // 30: dp2 r1.x, cb0[8].xyxx, r0.zwzz
    r1.x = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 31: dp2 r1.y, cb0[9].xyxx, r0.zwzz
    r1.y = (dot((source[9].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 32: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 33: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 34: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 35: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 36: mul r0.z, r0.z, cb0[21].x
    r0.z = ((r0.zzzz)*(source[21].xxxx)).z;
    // 37: max r0.z, r0.z, cb0[21].z
    r0.z = (max(r0.zzzz,source[21].zzzz)).z;
    // 38: min r0.z, r0.z, cb0[21].y
    r0.z = (min(r0.zzzz,source[21].yyyy)).z;
    // 39: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mul r0.w, v2.x, cb0[18].w
    r0.w = ((v2.xxxx)*(source[18].wwww)).w;
    // 41: mad r2.x, cb0[11].w, cb0[18].z, r0.w
    r2.x = ((source[11].wwww)*(source[18].zzzz)+(r0.wwww)).x;
    // 42: mul r1.zw, cb0[11].wwww, cb0[19].yyyw
    r1.zw = ((source[11].wwww)*(source[19].yyyw)).zw;
    // 43: mad r2.y, cb0[19].x, v2.y, r1.z
    r2.y = ((source[19].xxxx)*(v2.yyyy)+(r1.zzzz)).y;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 45: mad r1.xy, r0.wwww, cb0[19].zzzz, r1.xyxx
    r1.xy = ((r0.wwww)*(source[19].zzzz)+(r1.xyxx)).xy;
    // 46: mad r2.y, cb0[17].x, r1.y, r1.w
    r2.y = ((source[17].xxxx)*(r1.yyyy)+(r1.wwww)).y;
    // 47: mul r0.w, r1.x, cb0[16].w
    r0.w = ((r1.xxxx)*(source[16].wwww)).w;
    // 48: mad r2.x, cb0[11].w, cb0[16].z, r0.w
    r2.x = ((source[11].wwww)*(source[16].zzzz)+(r0.wwww)).x;
    // 49: add r1.xy, r2.xyxx, cb0[20].xyxx
    r1.xy = ((r2.xyxx)+(source[20].xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 51: add r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 52: add r1.x, v4.y, l(-1.000000)
    r1.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 53: add_sat r0.w, r0.w, -r1.x
    r0.w = (saturate((r0.wwww)+(-(r1.xxxx)))).w;
    // 54: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 55: mul r1.x, r1.x, cb0[20].w
    r1.x = ((r1.xxxx)*(source[20].wwww)).x;
    // 56: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 57: mul_sat r1.x, r1.x, cb0[20].z
    r1.x = (saturate((r1.xxxx)*(source[20].zzzz))).x;
    // 58: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 59: mul r0.w, r0.w, cb0[20].z
    r0.w = ((r0.wwww)*(source[20].zzzz)).w;
    // 60: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 61: mov_sat r1.y, r0.w
    r1.y = (saturate(r0.wwww)).y;
    // 62: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 63: mul r0.x, r0.x, cb0[21].w
    r0.x = ((r0.xxxx)*(source[21].wwww)).x;
    // 64: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 65: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 66: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 67: add r0.x, r1.x, r1.y
    r0.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 68: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 69: mad r0.xyz, r0.xxxx, cb0[10].xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(source[10].xyzx)+(r0.yyyy)).xyz;
    // 70: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 71: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_master_01_01_ad: e042827679e152468e9af203098ff8ae; selected map ac7b3b6c6c96f877181de9750eb9017464049052c612774604d7d9cc5ad38649.
float4 ArtistNative2857(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
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
    // 2: mad r0.y, cb0[4].y, cb0[7].y, cb0[7].z
    r0.y = ((source[4].yyyy)*(source[7].yyyy)+(source[7].zzzz)).y;
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 16: mul_sat r0.x, r0.x, cb0[8].y
    r0.x = (saturate((r0.xxxx)*(source[8].yyyy))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, cb0[8].z
    r0.y = ((r0.yyyy)*(source[8].zzzz)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 22: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 23: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 24: mul r0.y, v2.x, cb0[4].w
    r0.y = ((v2.xxxx)*(source[4].wwww)).y;
    // 25: mul r0.z, cb0[4].x, cb0[4].y
    r0.z = ((source[4].xxxx)*(source[4].yyyy)).z;
    // 26: mad r1.x, r0.z, cb0[4].z, r0.y
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.yyyy)).x;
    // 27: mul r0.yw, v2.yyyx, cb0[5].xxxw
    r0.yw = ((v2.yyyx)*(source[5].xxxw)).yw;
    // 28: mad r1.yz, r0.zzzz, cb0[5].yyzy, r0.yywy
    r1.yz = ((r0.zzzz)*(source[5].yyzy)+(r0.yywy)).yz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s0, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: mul r0.y, v2.y, cb0[6].x
    r0.y = ((v2.yyyy)*(source[6].xxxx)).y;
    // 31: mad r1.w, r0.z, cb0[6].y, r0.y
    r1.w = ((r0.zzzz)*(source[6].yyyy)+(r0.yyyy)).w;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.zwzz, t2.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 33: mul r1.xyz, r0.yzwy, r2.xyzx
    r1.xyz = ((r0.yzwy)*(r2.xyzx)).xyz;
    // 34: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: mad r0.yzw, -r2.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r2.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 36: mad r0.yzw, cb0[6].zzzz, r0.yyzw, r1.xxyz
    r0.yzw = ((source[6].zzzz)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 37: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 38: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 39: mul r0.yzw, r0.yyzw, cb0[6].wwww
    r0.yzw = ((r0.yyzw)*(source[6].wwww)).yzw;
    // 40: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 41: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 42: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 43: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 44: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 45: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 46: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_light_01_1_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative2858(ARTIST_NATIVE_INPUT input)
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
// fx_s_pa_glow_01_1_ad: 2bd85c08a26e594b945c597997daffea; selected map 721739ad9730ede6ef4b38fd43a1d852210faff01a4e05c1b6e0ae8ddb4d8df6.
float4 ArtistNative2859(ARTIST_NATIVE_INPUT input)
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_ring_07_83_ad: 8d8eae996ab1e94f9f993f1a7f4aca68; selected map 4e606f5cee84781e75c9f2314f1f528ed0c62107553aed72fae662fd99210b85.
float4 ArtistNative2860(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
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
    source[4].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 39: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t0.xyzw, s0, cb0[2].x
    r0.xyz = (ArtistNativeSample0((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r1.xyz, cb0[3].zzzz, r1.xyzx, r0.xyzx
    r1.xyz = ((source[3].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 43: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 44: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 45: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 46: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 47: mul r0.x, r0.x, cb0[3].w
    r0.x = ((r0.xxxx)*(source[3].wwww)).x;
    // 48: mul_sat r0.x, r0.x, l(0.333300)
    r0.x = (saturate((r0.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300)))).x;
    // 49: mov_sat r0.y, v4.y
    r0.y = (saturate(v4.yyyy)).y;
    // 50: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 52: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 54: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 55: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 56: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 57: mul r0.x, r0.x, cb0[4].x
    r0.x = ((r0.xxxx)*(source[4].xxxx)).x;
    // 58: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 59: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 60: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 61: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 62: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 63: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_makeflow_02_03_tr: 6681761729fba24896145fb070705f70; selected map fdfa3146587d6e058328ddecec678833d5f6bda76940ba9f8bd2a35254a53ed4.
float4 ArtistNative2861(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[3] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[5] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[6] = g_ArtistSourceMaterialParameters[6u];
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[11].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[12].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[13].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[14].x = (cos((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
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
    // 22: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 23: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 24: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 25: mad r0.z, r0.z, l(0.318471), l(1.000000)
    r0.z = ((r0.zzzz)*(float4(0.318471,0.318471,0.318471,0.318471))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 26: mul r1.x, r0.z, l(0.500000)
    r1.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 27: mul r2.x, r1.x, cb0[10].z
    r2.x = ((r1.xxxx)*(source[10].zzzz)).x;
    // 28: dp2 r0.z, -r0.xyxx, -r0.xyxx
    r0.z = (dot((-(r0.xyxx)).xy,(-(r0.xyxx)).xy).xxxx).z;
    // 29: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 30: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 31: sqrt r1.y, r0.z
    r1.y = (sqrt(r0.zzzz)).y;
    // 32: mad r0.yz, r1.xxyx, cb0[2].xxyx, cb0[3].xxyx
    r0.yz = ((r1.xxyx)*(source[2].xxyx)+(source[3].xxyx)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 34: mov r3.y, cb0[10].w
    r3.y = (source[10].wwww).y;
    // 35: mov r3.xz, l(0,0,0,0)
    r3.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 36: add r4.xyzw, r0.yzyz, r3.yxxy
    r4.xyzw = ((r0.yzyz)+(r3.yxxy)).xyzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r4.xyxx, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r4.zwzz, t1.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 39: add r3.xy, -r0.wwww, r0.yzyy
    r3.xy = ((-(r0.wwww))+(r0.yzyy)).xy;
    // 40: mul r4.xy, r3.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r4.xy = ((r3.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 41: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 42: add r0.yzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.yzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 43: dp3 r0.w, r0.yzwy, r0.yzwy
    r0.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 44: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 45: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 46: mad r0.yz, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.500000,0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 47: mul r0.yz, r0.yyzy, v4.xxxx
    r0.yz = ((r0.yyzy)*(v4.xxxx)).yz;
    // 48: log r0.w, |cb0[10].y|
    r0.w = (log2(abs(source[10].yyyy))).w;
    // 49: mul r0.w, r0.w, cb0[10].x
    r0.w = ((r0.wwww)*(source[10].xxxx)).w;
    // 50: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 51: lt r1.z, |cb0[10].y|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(source[10].yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 52: movc r0.w, r1.z, l(0), r0.w
    r0.w = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 53: mul r2.y, r0.w, r1.y
    r2.y = ((r0.wwww)*(r1.yyyy)).y;
    // 54: max r0.w, v4.z, l(0.000010)
    r0.w = (max(v4.zzzz,float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 55: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 56: mad r0.w, -r0.x, r0.w, l(1.000000)
    r0.w = ((-(r0.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mul_sat r0.w, r0.w, l(1.428571)
    r0.w = (saturate((r0.wwww)*(float4(1.428571,1.428571,1.428571,1.428571)))).w;
    // 58: mad r1.zw, r0.wwww, r0.yyyz, r2.xxxy
    r1.zw = ((r0.wwww)*(r0.yyyz)+(r2.xxxy)).zw;
    // 59: mov r3.w, v4.w
    r3.w = (v4.wwww).w;
    // 60: add r1.zw, r1.zzzw, r3.zzzw
    r1.zw = ((r1.zzzw)+(r3.zzzw)).zw;
    // 61: add r1.zw, r1.zzzw, cb0[4].xxxy
    r1.zw = ((r1.zzzw)+(source[4].xxxy)).zw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t3.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 63: mul r3.x, r1.x, cb0[12].x
    r3.x = ((r1.xxxx)*(source[12].xxxx)).x;
    // 64: mad r1.xz, r1.xxyx, cb0[7].xxyx, l(-0.500000, 0.000000, -0.500000, 0.000000)
    r1.xz = ((r1.xxyx)*(source[7].xxyx)+(float4(-0.500000,0.000000,-0.500000,0.000000))).xz;
    // 65: log r1.w, |cb0[11].w|
    r1.w = (log2(abs(source[11].wwww))).w;
    // 66: mul r1.w, r1.w, cb0[10].x
    r1.w = ((r1.wwww)*(source[10].xxxx)).w;
    // 67: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 68: lt r2.w, |cb0[11].w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(source[11].wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 69: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 70: mul r3.y, r1.w, r1.y
    r3.y = ((r1.wwww)*(r1.yyyy)).y;
    // 71: mad r1.yw, r0.wwww, r0.yyyz, r3.xxxy
    r1.yw = ((r0.wwww)*(r0.yyyz)+(r3.xxxy)).yw;
    // 72: mul r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)*(r0.wwww)).yz;
    // 73: add r1.yw, r1.yyyw, cb0[5].xxxy
    r1.yw = ((r1.yyyw)+(source[5].xxxy)).yw;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r1.ywyy, t4.xyzw, s2, l(0.000000)
    r3.xyz = (ArtistNativeSample2((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 75: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 76: dp3 r1.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 77: mad r2.xyz, -r2.xyzx, r3.xyzx, r1.yyyy
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r1.yyyy)).xyz;
    // 78: mad r2.xyz, cb0[13].yyyy, r2.xyzx, r4.xyzx
    r2.xyz = ((source[13].yyyy)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 79: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 80: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 81: mul r2.xyz, r2.xyzx, cb0[13].zzzz
    r2.xyz = ((r2.xyzx)*(source[13].zzzz)).xyz;
    // 82: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 83: mul r2.xyz, r2.xyzx, cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(source[13].wwww)).xyz;
    // 84: mad r2.xyz, r0.wwww, r2.xyzx, cb0[6].xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(source[6].xyzx)).xyz;
    // 85: mad r2.xyz, v3.xyzx, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((v3.xyzx)*(r2.xyzx)+(source[1].xyzx)).xyz;
    // 86: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 87: dp2 r0.w, cb0[9].xyxx, r1.xzxx
    r0.w = (dot((source[9].xyxx).xy,(r1.xzxx).xy).xxxx).w;
    // 88: dp2 r1.x, cb0[8].xyxx, r1.xzxx
    r1.x = (dot((source[8].xyxx).xy,(r1.xzxx).xy).xxxx).x;
    // 89: add r1.y, r0.w, v4.y
    r1.y = ((r0.wwww)+(v4.yyyy)).y;
    // 90: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 91: mad r0.yz, cb0[14].wwww, r0.yyzy, r1.xxyx
    r0.yz = ((source[14].wwww)*(r0.yyzy)+(r1.xxyx)).yz;
    // 92: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 93: add r0.z, v4.z, l(0.200000)
    r0.z = ((v4.zzzz)+(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 94: max r0.z, r0.z, l(0.000010)
    r0.z = (max(r0.zzzz,float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 95: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 96: mad r0.x, -r0.x, r0.z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 97: mul_sat r0.x, r0.x, l(1.428571)
    r0.x = (saturate((r0.xxxx)*(float4(1.428571,1.428571,1.428571,1.428571)))).x;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t0.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 99: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 100: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 101: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 102: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 103: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 104: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 105: mul_sat r0.x, r0.x, cb0[15].y
    r0.x = (saturate((r0.xxxx)*(source[15].yyyy))).x;
    // 106: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 107: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_shine_01_05_dt_ad: 1f3e2fdc0f35504493c2d7fca9bd10b1; selected map 99fb0a09edfa19b9d94ab099ff0afebeefd112ee9e07786e44da96f7a5ba1180.
float4 ArtistNative2862(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[7].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[3u].wwww))).x;
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
    // 1: mul r0.x, v2.x, cb0[3].w
    r0.x = ((v2.xxxx)*(source[3].wwww)).x;
    // 2: mad r0.x, cb0[2].w, cb0[3].z, r0.x
    r0.x = ((source[2].wwww)*(source[3].zzzz)+(r0.xxxx)).x;
    // 3: mul r0.z, cb0[2].w, cb0[4].y
    r0.z = ((source[2].wwww)*(source[4].yyyy)).z;
    // 4: mad r0.y, cb0[4].x, v2.y, r0.z
    r0.y = ((source[4].xxxx)*(v2.yyyy)+(r0.zzzz)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[4].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[4].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 8: mul r0.xy, r0.xyxx, cb0[5].yzyy
    r0.xy = ((r0.xyxx)*(source[5].yzyy)).xy;
    // 9: mad r0.xy, cb0[2].wwww, cb0[5].xwxx, r0.xyxx
    r0.xy = ((source[2].wwww)*(source[5].xwxx)+(r0.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.x, cb0[2].w, cb0[2].z, r0.z
    r1.x = ((source[2].wwww)*(source[2].zzzz)+(r0.zzzz)).x;
    // 12: mad r1.y, cb0[2].w, cb0[4].w, r0.w
    r1.y = ((source[2].wwww)*(source[4].wwww)+(r0.wwww)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 15: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 16: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 18: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 19: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 20: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 21: add r0.y, v2.x, l(-0.500000)
    r0.y = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 22: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 23: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: mul r0.z, r0.y, v2.y
    r0.z = ((r0.yyyy)*(v2.yyyy)).z;
    // 25: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 26: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 27: sqrt r0.z, v2.y
    r0.z = (sqrt(v2.yyyy)).z;
    // 28: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 29: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 30: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 32: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 33: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 34: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 35: mad_sat r0.x, r0.y, cb0[2].y, r0.x
    r0.x = (saturate((r0.yyyy)*(source[2].yyyy)+(r0.xxxx))).x;
    // 36: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 37: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 38: source device depth mapped to centimetre view depth; reconstruction at 40.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 40-43: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 44: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 45: add r0.z, -cb0[7].x, l(1.000000)
    r0.z = ((-(source[7].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 47: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 48: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 49: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 50: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 51: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 52: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 53: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 54: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 56: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 59: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 60: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 61: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_de_master_01_1_tr: 31e28c4451b3b942b46cdca51e41ed25; selected map 65239cdeec1584e46f9092ff2642d6b664995e5d6ae61036c456c38f8c1208db.
float4 ArtistNative2863(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[13]=float4(input.skyUpperColor,0.f);
    source[14]=float4(input.skyLowerColor,0.f);
    source[15]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = g_ArtistSourceMaterialParameters[4u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].zzzz,1u);
    source[6] = (g_ArtistSourceMaterialTime.xxxx*ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,float4(0.0, 0.0, 0.0, 0.0),1u));
    source[7] = g_ArtistSourceMaterialParameters[3u];
    source[8].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f;
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
    // 11: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: mad r0.xy, r0.xyxx, cb0[5].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(source[5].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.yxzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 14: mul r0.yz, v4.xxyx, cb0[8].xxxx
    r0.yz = ((v4.xxyx)*(source[8].xxxx)).yz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 16: mad r0.yz, r1.xxyx, l(0.000000, 0.200000, 0.200000, 0.000000), v4.xxyx
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.200000,0.200000,0.000000))+(v4.xxyx)).yz;
    // 17: add r0.yz, r0.yyzy, cb0[6].xxyx
    r0.yz = ((r0.yyzy)+(source[6].xxyx)).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 19: add r0.z, r0.y, r0.x
    r0.z = ((r0.yyyy)+(r0.xxxx)).z;
    // 20: mul r0.z, r0.z, r0.x
    r0.z = ((r0.zzzz)*(r0.xxxx)).z;
    // 21: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 22: mul r0.xy, r0.xyxx, r0.xzxx
    r0.xy = ((r0.xyxx)*(r0.xzxx)).xy;
    // 23: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 24: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: mul r0.z, r0.z, cb0[9].z
    r0.z = ((r0.zzzz)*(source[9].zzzz)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: mul r2.xyz, cb0[7].xyzx, cb0[7].wwww
    r2.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 28: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 29: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 30: movc r0.yzw, r0.yyyy, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 31: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 32: add r2.xyz, -r1.xyzx, r2.xxxx
    r2.xyz = ((-(r1.xyzx))+(r2.xxxx)).xyz;
    // 33: mad r1.xyz, cb0[8].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[8].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 34: mul r1.w, r1.w, cb0[9].w
    r1.w = ((r1.wwww)*(source[9].wwww)).w;
    // 35: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 36: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 37: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 38: mad r0.xyz, r1.xyzx, l(0.150000, 0.150000, 0.150000, 0.000000), r0.yzwy
    r0.xyz = ((r1.xyzx)*(float4(0.150000,0.150000,0.150000,0.000000))+(r0.yzwy)).xyz;
    // 39: mul r1.xyz, r1.xyzx, cb2[3].wwww
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)).xyz;
    // 40: mad r1.xyz, r1.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 41: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 42: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 43: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 44: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 45: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 46: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 47: mul r2.yzw, r2.yyyy, cb0[14].xxyz
    r2.yzw = ((r2.yyyy)*(source[14].xxyz)).yzw;
    // 48: mad r2.xyz, r2.xxxx, cb0[13].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[13].xyzx)+(r2.yzwy)).xyz;
    // 49: mul r2.xyz, r2.xyzx, cb0[15].wwww
    r2.xyz = ((r2.xyzx)*(source[15].wwww)).xyz;
    // 50: mad r0.xyz, r2.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 51: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 53: mad r0.xyz, r1.xyzx, cb0[15].xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(source[15].xyzx)+(r0.xyzx)).xyz;
    // 55: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 56: log r0.x, |r1.w|
    r0.x = (log2(abs(r1.wwww))).x;
    // 57: lt r0.y, |r1.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 58: mul r0.x, r0.x, cb0[10].x
    r0.x = ((r0.xxxx)*(source[10].xxxx)).x;
    // 59: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 60: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 61: mul r0.yz, v4.xxyx, cb0[10].yyyy
    r0.yz = ((v4.xxyx)*(source[10].yyyy)).yz;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 63: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 64: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 66: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 67: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 68: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 69: add r0.y, -|v4.w|, cb0[0].y
    r0.y = ((-(abs(v4.wwww)))+(source[0].yyyy)).y;
    // 70: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 71: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 72: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 73: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_g_pa_sy_09_1_tr: d4219dbd58be09438f811c49d92a1132; selected map bcf55c3ef93713de7e955372a7272caf06c044cb20add281dacccecb6466407b.
float4 ArtistNative2864(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 3: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_g_pa_sy_colorcard_01_tr: 2dd1dfce2c2e0143a74c4a19dc7f3fbb; selected map 648d439417eb366d79e9334ba9c36f972f54dcbd6d197454a700dee9c6480bdc.
float4 ArtistNative2865(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = g_ArtistSourceMaterialParameters[1u];
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[6] = input.dynamicParameter;
    source[7] = g_ArtistSourceMaterialParameters[5u];
    source[8].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: add r0.x, cb0[6].x, cb0[8].z
    r0.x = ((source[6].xxxx)+(source[8].zzzz)).x;
    // 2: add r0.y, v4.y, l(1.000000)
    r0.y = ((v4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 3: add r0.x, -r0.x, r0.y
    r0.x = ((-(r0.xxxx))+(r0.yyyy)).x;
    // 4: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 5: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), cb0[5].xxxy
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(source[5].xxxy)).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s3, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 7: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 8: mad r0.xzw, r0.xxxx, r1.xxyz, r0.xxxx
    r0.xzw = ((r0.xxxx)*(r1.xxyz)+(r0.xxxx)).xzw;
    // 9: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 10: mul r2.xyz, r0.xzwx, r0.xzwx
    r2.xyz = ((r0.xzwx)*(r0.xzwx)).xyz;
    // 11: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 12: mul r0.xzw, r0.xxzw, r2.xxyz
    r0.xzw = ((r0.xxzw)*(r2.xxyz)).xzw;
    // 13: min r0.xzw, r0.xxzw, l(1.000000, 0.000000, 1.000000, 1.000000)
    r0.xzw = (min(r0.xxzw,float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 14: add_sat r1.w, r0.y, -cb0[6].x
    r1.w = (saturate((r0.yyyy)+(-(source[6].xxxx)))).w;
    // 15: mad_sat r2.xyz, r1.wwww, r1.xyzx, r1.wwww
    r2.xyz = (saturate((r1.wwww)*(r1.xyzx)+(r1.wwww))).xyz;
    // 16: add r0.xzw, r0.xxzw, -r2.xxyz
    r0.xzw = ((r0.xxzw)+(-(r2.xxyz))).xzw;
    // 17: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 18: mul r3.xyz, r0.xzwx, r0.xzwx
    r3.xyz = ((r0.xzwx)*(r0.xzwx)).xyz;
    // 19: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 20: mul r0.xzw, r0.xxzw, r3.xxyz
    r0.xzw = ((r0.xxzw)*(r3.xxyz)).xzw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xyz = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mul r5.xyz, r4.xyzx, cb0[3].xyzx
    r5.xyz = ((r4.xyzx)*(source[3].xyzx)).xyz;
    // 24: mad r4.xyz, -r4.xyzx, cb0[3].xyzx, r3.wwww
    r4.xyz = ((-(r4.xyzx))*(source[3].xyzx)+(r3.wwww)).xyz;
    // 25: mad r3.xyz, cb0[4].xyzx, r3.xyzx, -r5.xyzx
    r3.xyz = ((source[4].xyzx)*(r3.xyzx)+(-(r5.xyzx))).xyz;
    // 26: mad r3.xyz, r2.xyzx, r3.xyzx, r5.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 27: mad r2.xyz, r2.xyzx, r4.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 28: add r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)+(r2.xyzx)).xyz;
    // 29: dp3_sat r1.w, r2.xyzx, l(0.200000, 0.200000, 0.200000, 0.000000)
    r1.w = (saturate(dot((r2.xyzx).xyz,(float4(0.200000,0.200000,0.200000,0.000000)).xyz).xxxx)).w;
    // 30: mul r1.w, r1.w, cb0[1].w
    r1.w = ((r1.wwww)*(source[1].wwww)).w;
    // 31: mul r0.xzw, r0.xxzw, r3.xxyz
    r0.xzw = ((r0.xxzw)*(r3.xxyz)).xzw;
    // 32: mad r0.xzw, r0.xxzw, cb0[7].xxyz, r3.xxyz
    r0.xzw = ((r0.xxzw)*(source[7].xxyz)+(r3.xxyz)).xzw;
    // 33: add r2.x, cb0[6].x, cb0[9].x
    r2.x = ((source[6].xxxx)+(source[9].xxxx)).x;
    // 34: add_sat r0.y, r0.y, -r2.x
    r0.y = (saturate((r0.yyyy)+(-(r2.xxxx)))).y;
    // 35: mad_sat r1.xyz, r0.yyyy, r1.xyzx, r0.yyyy
    r1.xyz = (saturate((r0.yyyy)*(r1.xyzx)+(r0.yyyy))).xyz;
    // 36: mul r0.xyz, r0.xzwx, r1.xyzx
    r0.xyz = ((r0.xzwx)*(r1.xyzx)).xyz;
    // 37: mad r0.xyz, cb0[1].xyzx, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 38: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 39: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 40: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 41: source device depth mapped to centimetre view depth; reconstruction at 43.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 43-46: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 47: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 48: mul_sat r0.w, r0.w, l(0.025000)
    r0.w = (saturate((r0.wwww)*(float4(0.025000,0.025000,0.025000,0.025000)))).w;
    // 49: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 50: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 51: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 52: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_shine_01_09_ad: eed5a0bfbb11fb4cbe047cc2cefd45f5; selected map 813c7a9653d5d31479688e52db86c7fc08a10cb679d4957d37544db222fd06a2.
float4 ArtistNative2866(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mul r0.x, v2.x, cb0[3].w
    r0.x = ((v2.xxxx)*(source[3].wwww)).x;
    // 2: mad r0.x, cb0[2].w, cb0[3].z, r0.x
    r0.x = ((source[2].wwww)*(source[3].zzzz)+(r0.xxxx)).x;
    // 3: mul r0.z, cb0[2].w, cb0[4].y
    r0.z = ((source[2].wwww)*(source[4].yyyy)).z;
    // 4: mad r0.y, cb0[4].x, v2.y, r0.z
    r0.y = ((source[4].xxxx)*(v2.yyyy)+(r0.zzzz)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[4].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[4].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 8: mul r0.xy, r0.xyxx, cb0[5].yzyy
    r0.xy = ((r0.xyxx)*(source[5].yzyy)).xy;
    // 9: mad r0.xy, cb0[2].wwww, cb0[5].xwxx, r0.xyxx
    r0.xy = ((source[2].wwww)*(source[5].xwxx)+(r0.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.x, cb0[2].w, cb0[2].z, r0.z
    r1.x = ((source[2].wwww)*(source[2].zzzz)+(r0.zzzz)).x;
    // 12: mad r1.y, cb0[2].w, cb0[4].w, r0.w
    r1.y = ((source[2].wwww)*(source[4].wwww)+(r0.wwww)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 15: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 16: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 18: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 19: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 20: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 21: add r0.y, v2.x, l(-0.500000)
    r0.y = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 22: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 23: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: mul r0.z, r0.y, v2.y
    r0.z = ((r0.yyyy)*(v2.yyyy)).z;
    // 25: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 26: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 27: sqrt r0.z, v2.y
    r0.z = (sqrt(v2.yyyy)).z;
    // 28: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 29: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 30: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 32: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 33: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 34: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 35: mad_sat r0.x, r0.y, cb0[2].y, r0.x
    r0.x = (saturate((r0.yyyy)*(source[2].yyyy)+(r0.xxxx))).x;
    // 36: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 37: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 38: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 39: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 40: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 42: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 43: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 44: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 45: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 46: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 47: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_pa_spritewave_01_3_tr: 8a6a92da58a4964daec9bc7777b127fd; selected map 704cc662b6e7449af34df47de21eab57347ab0d0778a04474c137b9e2488eeb9.
float4 ArtistNative2867(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[11u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[8u].wwww,g_ArtistSourceMaterialParameters[9u].xxxx,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[4] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_ArtistSourceMaterialParameters[10u];
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[16].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[16].z = ((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[16].w = (sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[17].y = (cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[18].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[19].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[19].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[12].yzyy
    r0.xy = ((v2.xyxx)*(source[12].yzyy)).xy;
    // 2: mad r0.xy, cb0[9].wwww, cb0[12].xwxx, r0.xyxx
    r0.xy = ((source[9].wwww)*(source[12].xwxx)+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
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
    r0.z = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_sqc_01_06_dt_tr: 0d047bda09fdab409b233d70ee0f2dd8; selected map fdf5be0aacb0bd0229e4e2c301d24d4b9bee7d3a6b6d174dfc1579d638d2bf30.
float4 ArtistNative2868(ARTIST_NATIVE_INPUT input)
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
// fx_g_me_trail_rainbow_01_ad: 56d164b9dbd6a24590b0ecec1afcfc33; selected map f2eabcd7546ae345ab92a7533a6e745c48c6f04d81dc4e45f4b69b87864b7222.
float4 ArtistNative2869(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,float4(0.0, 0.0, 0.0, 0.0),1u);
    source[4] = input.dynamicParameter;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[7].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
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
    // 1: mul r0.x, v4.y, cb0[5].w
    r0.x = ((v4.yyyy)*(source[5].wwww)).x;
    // 2: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 3: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 4: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 12: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 13: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 14: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 15: mul_sat r0.z, r0.z, cb0[5].y
    r0.z = (saturate((r0.zzzz)*(source[5].yyyy))).z;
    // 16: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 17: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 18: add r0.y, v4.x, cb0[7].x
    r0.y = ((v4.xxxx)+(source[7].xxxx)).y;
    // 19: mul r1.x, r0.y, cb0[7].y
    r1.x = ((r0.yyyy)*(source[7].yyyy)).x;
    // 20: mul r1.y, v4.y, cb0[7].z
    r1.y = ((v4.yyyy)*(source[7].zzzz)).y;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 22: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 23: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 24: mad r0.yzw, cb0[7].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 25: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 26: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 27: mul r0.yzw, r0.yyzw, cb0[8].xxxx
    r0.yzw = ((r0.yyzw)*(source[8].xxxx)).yzw;
    // 28: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 29: mul r0.yzw, r0.yyzw, cb0[8].yyyy
    r0.yzw = ((r0.yyzw)*(source[8].yyyy)).yzw;
    // 30: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 31: mad r1.xy, v4.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 33: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 34: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 35: or r0.w, r1.x, r1.y
    r0.w = (asfloat(asuint(r1.xxxx) | asuint(r1.yyyy))).w;
    // 36: mul r1.x, r1.w, l(1.500000)
    r1.x = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 37: mul r1.y, r1.z, cb0[8].z
    r1.y = ((r1.zzzz)*(source[8].zzzz)).y;
    // 38: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 39: mul_sat r1.y, r1.y, cb0[8].w
    r1.y = (saturate((r1.yyyy)*(source[8].wwww))).y;
    // 40: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 41: mul r1.x, r1.x, l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 42: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 44: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 45: mad r1.x, v4.x, l(2.000000), cb0[3].x
    r1.x = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[3].xxxx)).x;
    // 46: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 47: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 49: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 50: mul r1.y, r1.y, cb0[9].y
    r1.y = ((r1.yyyy)*(source[9].yyyy)).y;
    // 51: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 52: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 53: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 54: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 55: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 56: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 57: mad r0.w, v4.x, l(2.000000), cb0[4].x
    r0.w = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[4].xxxx)).w;
    // 58: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 59: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 61: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 62: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 63: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 64: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_g_pa_trail_rainbow_01_ad: c26400385d10654388337c9d54041365; selected map 1ae63eda74bad59106f76ae9ca02bcf2867eb42116efa6818900fec020b17b92.
float4 ArtistNative2870(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,float4(0.0, 0.0, 0.0, 0.0),1u);
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
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
    // 1: mul r0.x, v2.y, cb0[4].w
    r0.x = ((v2.yyyy)*(source[4].wwww)).x;
    // 2: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 3: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 4: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)*(source[5].xxxx)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.y, -v2.x, l(1.000000)
    r0.y = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 12: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 13: mul r0.z, r0.z, cb0[4].x
    r0.z = ((r0.zzzz)*(source[4].xxxx)).z;
    // 14: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 15: mul_sat r0.z, r0.z, cb0[4].y
    r0.z = (saturate((r0.zzzz)*(source[4].yyyy))).z;
    // 16: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 17: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 18: add r0.y, v2.x, cb0[6].x
    r0.y = ((v2.xxxx)+(source[6].xxxx)).y;
    // 19: mul r1.x, r0.y, cb0[6].y
    r1.x = ((r0.yyyy)*(source[6].yyyy)).x;
    // 20: mul r1.y, v2.y, cb0[6].z
    r1.y = ((v2.yyyy)*(source[6].zzzz)).y;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 22: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 23: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 24: mad r0.yzw, cb0[6].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[6].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 25: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 26: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 27: mul r0.yzw, r0.yyzw, cb0[7].xxxx
    r0.yzw = ((r0.yyzw)*(source[7].xxxx)).yzw;
    // 28: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 29: mul r0.yzw, r0.yyzw, cb0[7].yyyy
    r0.yzw = ((r0.yyzw)*(source[7].yyyy)).yzw;
    // 30: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 31: mad r1.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 33: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 34: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 35: or r0.w, r1.x, r1.y
    r0.w = (asfloat(asuint(r1.xxxx) | asuint(r1.yyyy))).w;
    // 36: mul r1.x, r1.w, l(1.500000)
    r1.x = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 37: mul r1.y, r1.z, cb0[7].z
    r1.y = ((r1.zzzz)*(source[7].zzzz)).y;
    // 38: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 39: mul_sat r1.y, r1.y, cb0[7].w
    r1.y = (saturate((r1.yyyy)*(source[7].wwww))).y;
    // 40: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 41: mul r1.x, r1.x, l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 42: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 44: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 45: mad r1.x, v2.x, l(2.000000), cb0[2].x
    r1.x = ((v2.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[2].xxxx)).x;
    // 46: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 47: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 49: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 50: mul r1.y, r1.y, cb0[8].y
    r1.y = ((r1.yyyy)*(source[8].yyyy)).y;
    // 51: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 52: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 53: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 54: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 55: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 56: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 57: mad r0.w, v2.x, l(2.000000), cb0[3].x
    r0.w = ((v2.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[3].xxxx)).w;
    // 58: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 59: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 61: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 62: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 63: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 64: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_shine_01_01_ad: f308e6c8bf787a4fbb242a07ff277211; selected map 7300c3c9b40788b80b30a5d639bda8170624b700959cd1ce48b334633e0f6586.
float4 ArtistNative2871(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].w = (clamp(g_ArtistSourceMaterialParameters[4u].yyyy,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[5].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ArtistSourceMaterialParameters[4u].yyyy,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
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
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 44: log r0.y, r0.z
    r0.y = (log2(r0.zzzz)).y;
    // 45: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // 47: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 48: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 49: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 51: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 52: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 53: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 54: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 55: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// wp_mn_ttbb_02_mi_dead: 0536b29c3525994fa5ccf365c7edc279; selected map aab7dcd9405cbc7826e8a997938d268b07141e1ea74cbb81ed41aef6b9e0838c.
float4 ArtistNative2872(ARTIST_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=0.f; // Absolute source world position needs no pre-view translation.
    source[1]=float4(input.sourceCameraPosition,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
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
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
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
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_fi_02_1_tr: 1c52877bf8998d4fa2659be93d5a9995; selected map a9f170e2760714a8c524b8ea96fca93519ca736a06ac41d45d7fb553e083f0b7.
float4 ArtistNative2873(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[3] = g_ArtistSourceMaterialParameters[0u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.119999997, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0599999987, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), v4.xxxx
    r0.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(v4.xxxx)).xy;
    // 2: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.yxzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 5: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 6: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 7: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 8: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 9: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 10: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 11: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 12: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 13: add r0.yz, r0.yyzy, -cb0[0].xxyx
    r0.yz = ((r0.yyzy)+(-(source[0].xxyx))).yz;
    // 14: mad r1.xy, r0.yzyy, cb0[1].zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.yzyy)*(source[1].zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 15: mov r1.zw, l(0,0,1.000000,1.200000)
    r1.zw = (float4(asfloat(0u),asfloat(0u),1.000000,1.200000)).zw;
    // 16: mul r0.yz, r1.xxzx, v2.xxyx
    r0.yz = ((r1.xxzx)*(v2.xxyx)).yz;
    // 17: mad r0.yz, r1.wwyw, r0.yyzy, v4.xxxx
    r0.yz = ((r1.wwyw)*(r0.yyzy)+(v4.xxxx)).yz;
    // 18: add r0.yz, r0.yyzy, cb0[6].xxyx
    r0.yz = ((r0.yyzy)+(source[6].xxyx)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 20: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 21: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 22: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul o0.w, r0.x, cb0[2].x
    output.w = ((r0.xxxx)*(source[2].xxxx)).w;
    // 24: mad r0.xy, v2.xyxx, l(1.000000, 0.900000, 0.000000, 0.000000), v4.xxxx
    r0.xy = ((v2.xyxx)*(float4(1.000000,0.900000,0.000000,0.000000))+(v4.xxxx)).xy;
    // 25: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[3].xyzx)).xyz;
    // 28: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_worldoffset_02_39_tr: 2cce8741b31d5049ab2b10e26293372d; selected map b62b4d84068ddcdad35cfacb576ce799a955f5824aac67e7740221f123d6be23.
float4 ArtistNative2874(ARTIST_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[4] = g_ArtistSourceMaterialParameters[8u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[6] = ArtistNativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[16].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 6: mul r0.zw, r0.xxxy, cb0[14].zzzw
    r0.zw = ((r0.xxxy)*(source[14].zzzw)).zw;
    // 7: mad r1.x, cb0[11].w, cb0[14].y, r0.z
    r1.x = ((source[11].wwww)*(source[14].yyyy)+(r0.zzzz)).x;
    // 8: mad r1.y, cb0[11].w, cb0[15].x, r0.w
    r1.y = ((source[11].wwww)*(source[15].xxxx)+(r0.wwww)).y;
    // 9: add r0.zw, r1.xxxy, cb0[8].xxxy
    r0.zw = ((r1.xxxy)+(source[8].xxxy)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 11: mad r1.xy, cb0[15].wwww, r0.zzzz, r0.xyxx
    r1.xy = ((source[15].wwww)*(r0.zzzz)+(r0.xyxx)).xy;
    // 12: mul r0.xy, r0.xyxx, cb0[12].xyxx
    r0.xy = ((r0.xyxx)*(source[12].xyxx)).xy;
    // 13: mul r0.w, r1.x, cb0[13].w
    r0.w = ((r1.xxxx)*(source[13].wwww)).w;
    // 14: mul r1.x, r1.y, cb0[14].x
    r1.x = ((r1.yyyy)*(source[14].xxxx)).x;
    // 15: mad r1.y, cb0[11].w, cb0[16].x, r1.x
    r1.y = ((source[11].wwww)*(source[16].xxxx)+(r1.xxxx)).y;
    // 16: mad r1.x, cb0[11].w, cb0[13].z, r0.w
    r1.x = ((source[11].wwww)*(source[13].zzzz)+(r0.wwww)).x;
    // 17: add r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)+(source[9].xyxx)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s4, l(0.000000)
    r0.w = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 19: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 25: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 26: mul r0.w, r0.w, cb0[17].x
    r0.w = ((r0.wwww)*(source[17].xxxx)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul r0.w, r0.w, cb0[17].y
    r0.w = ((r0.wwww)*(source[17].yyyy)).w;
    // 29: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 31: mad r0.z, r0.z, cb0[17].z, r0.w
    r0.z = ((r0.zzzz)*(source[17].zzzz)+(r0.wwww)).z;
    // 32: mad r1.x, cb0[11].w, cb0[11].z, r0.x
    r1.x = ((source[11].wwww)*(source[11].zzzz)+(r0.xxxx)).x;
    // 33: mad r1.y, cb0[11].w, cb0[12].z, r0.y
    r1.y = ((source[11].wwww)*(source[12].zzzz)+(r0.yyyy)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 35: mad r1.xy, cb0[10].zwzz, v2.xyxx, cb0[5].xyxx
    r1.xy = ((source[10].zwzz)*(v2.xyxx)+(source[5].xyxx)).xy;
    // 36: mul r0.w, v4.x, cb0[12].w
    r0.w = ((v4.xxxx)*(source[12].wwww)).w;
    // 37: mad r1.xy, r0.wwww, r0.xyxx, r1.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r1.xyxx)).xy;
    // 38: mad r1.z, v4.y, l(0.500000), r1.y
    r1.z = ((v4.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.yyyy)).z;
    // 39: add r0.xy, r1.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 40: dp2 r1.x, cb0[6].xyxx, r0.xyxx
    r1.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 41: dp2 r1.y, cb0[7].xyxx, r0.xyxx
    r1.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 42: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 45: add r0.y, r1.y, r1.x
    r0.y = ((r1.yyyy)+(r1.xxxx)).y;
    // 46: add r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)+(r0.yyyy)).y;
    // 47: mul r0.y, r0.y, l(0.333330)
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 50: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 51: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 52: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 53: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 54: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 55: mad r0.y, r0.x, r0.z, r0.y
    r0.y = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 56: mul r0.x, r0.x, cb0[17].w
    r0.x = ((r0.xxxx)*(source[17].wwww)).x;
    // 57: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 58: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[4].xxyz)).yzw;
    // 59: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 60: add r0.yz, -v2.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(v2.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 61: mul r0.yz, r0.yyzy, v2.xxyx
    r0.yz = ((r0.yyzy)*(v2.xxyx)).yz;
    // 62: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 63: mul_sat r0.y, r0.y, l(50.000000)
    r0.y = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).y;
    // 64: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 65: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_b_me_lava_01: cb42fe675cadf94aac44e861c822207d; selected map 39226ecfc3933393ba3263595b668399e56666ce7de4bbb9489542161f16f840.
float4 ArtistNative2875(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.150000006, 0.0, 0.0, 0.0))),1u);
    source[4] = g_ArtistSourceMaterialParameters[0u];
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
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, v4.x, l(-1.000000)
    r0.x = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, v4.x
    r0.x = ((r0.xxxx)*(v4.xxxx)).x;
    // 3: mul r0.y, |r0.x|, |r0.x|
    r0.y = ((abs(r0.xxxx))*(abs(r0.xxxx))).y;
    // 4: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 5: mul r0.y, r0.y, l(20.000000)
    r0.y = ((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000))).y;
    // 6: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 8: mad r0.yz, v4.xxyx, l(0.000000, 2.000000, 5.000000, 0.000000), cb0[3].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,2.000000,5.000000,0.000000))+(source[3].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 1.500000, 7.000000), cb0[2].xxxy
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,1.500000,7.000000))+(source[2].xxxy)).zw;
    // 11: mad r0.yz, r0.yyyy, l(0.000000, 0.100000, 0.100000, 0.000000), r0.zzwz
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.100000,0.100000,0.000000))+(r0.zzwz)).yz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t1.zxyw, s1, l(0.000000)
    r0.yz = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 13: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 14: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yyyy, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r0.yyyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 17: add r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)+(r1.xyzx)).xyz;
    // 18: mad o0.xyz, r1.xyzx, cb0[4].xyzx, cb0[1].xyzx
    output.xyz = ((r1.xyzx)*(source[4].xyzx)+(source[1].xyzx)).xyz;
    // 19: mad_sat r0.x, cb0[0].w, r0.w, r0.x
    r0.x = (saturate((source[0].wwww)*(r0.wwww)+(r0.xxxx))).x;
    // 20: add r0.x, r0.x, l(-0.100000)
    r0.x = ((r0.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 21: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 22: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 23: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 24: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_g_pa_twirl_01_04_ad: 1f377575d000c741acc21359a3b57bff; selected map 891633497c18c98704b26f55b0266657677b4f2793e6d9085d01e3f266d8838a.
float4 ArtistNative2876(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[8].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[9].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz))).x;
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
    // 10: add r0.y, -cb0[9].w, l(1.000000)
    r0.y = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: add r1.xy, r0.yzyy, r0.yzyy
    r1.xy = ((r0.yzyy)+(r0.yzyy)).xy;
    // 16: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 17: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 18: max r0.z, |r1.x|, |r1.y|
    r0.z = (max(abs(r1.xxxx),abs(r1.yyyy))).z;
    // 19: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 20: min r0.w, |r1.x|, |r1.y|
    r0.w = (min(abs(r1.xxxx),abs(r1.yyyy))).w;
    // 21: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 22: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 23: mad r1.z, r0.w, l(0.020835), l(-0.085133)
    r1.z = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 24: mad r1.z, r0.w, r1.z, l(0.180141)
    r1.z = ((r0.wwww)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 25: mad r1.z, r0.w, r1.z, l(-0.330299)
    r1.z = ((r0.wwww)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 26: mad r0.w, r0.w, r1.z, l(0.999866)
    r0.w = ((r0.wwww)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 27: mul r1.z, r0.w, r0.z
    r1.z = ((r0.wwww)*(r0.zzzz)).z;
    // 28: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 29: lt r1.w, |r1.x|, |r1.y|
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(abs(r1.yyyy))) * 0xffffffffu)).w;
    // 30: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 31: mad r0.z, r0.z, r0.w, r1.z
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.zzzz)).z;
    // 32: lt r0.w, r1.x, -r1.x
    r0.w = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).w;
    // 33: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 34: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 35: min r0.w, r1.x, r1.y
    r0.w = (min(r1.xxxx,r1.yyyy)).w;
    // 36: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 37: max r1.z, r1.x, r1.y
    r1.z = (max(r1.xxxx,r1.yyyy)).z;
    // 38: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 39: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: ge r1.y, r1.z, -r1.z
    r1.y = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).y;
    // 41: and r0.w, r0.w, r1.y
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.yyyy))).w;
    // 42: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 43: add r0.w, -r0.y, l(0.500000)
    r0.w = ((-(r0.yyyy))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 44: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 45: mul r1.y, v4.w, cb0[4].y
    r1.y = ((v4.wwww)*(source[4].yyyy)).y;
    // 46: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 47: log r1.y, |r0.w|
    r1.y = (log2(abs(r0.wwww))).y;
    // 48: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 49: mul r1.y, r1.y, cb0[4].z
    r1.y = ((r1.yyyy)*(source[4].zzzz)).y;
    // 50: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 51: movc r0.w, r0.w, l(0), r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).w;
    // 52: mad r2.x, r0.z, l(0.318310), r0.w
    r2.x = ((r0.zzzz)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.wwww)).x;
    // 53: mul r0.z, r2.x, cb0[3].w
    r0.z = ((r2.xxxx)*(source[3].wwww)).z;
    // 54: mul r0.w, cb0[3].x, cb0[3].y
    r0.w = ((source[3].xxxx)*(source[3].yyyy)).w;
    // 55: mad r3.x, r0.w, cb0[3].z, r0.z
    r3.x = ((r0.wwww)*(source[3].zzzz)+(r0.zzzz)).x;
    // 56: add r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)+(r0.yyyy)).z;
    // 57: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 58: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 59: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 60: lt r1.y, r0.y, l(0.000000)
    r1.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 61: mad r0.y, -r0.y, cb0[7].w, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 62: mul_sat r0.y, r0.y, cb0[8].w
    r0.y = (saturate((r0.yyyy)*(source[8].wwww))).y;
    // 63: movc r2.y, r1.y, l(0), r0.z
    r2.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 64: mul r0.z, r2.y, cb0[4].x
    r0.z = ((r2.yyyy)*(source[4].xxxx)).z;
    // 65: mul r1.yz, r2.xxyx, cb0[5].yyzy
    r1.yz = ((r2.xxyx)*(source[5].yyzy)).yz;
    // 66: mad r1.yz, r0.wwww, cb0[5].xxwx, r1.yyzy
    r1.yz = ((r0.wwww)*(source[5].xxwx)+(r1.yyzy)).yz;
    // 67: mad r3.y, r0.w, cb0[4].w, r0.z
    r3.y = ((r0.wwww)*(source[4].wwww)+(r0.zzzz)).y;
    // 68: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r3.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t1.wxyz, s2, l(0.000000)
    r1.yzw = (ArtistNativeSample1((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
    // 70: add r0.z, -r2.x, r1.y
    r0.z = ((-(r2.xxxx))+(r1.yyyy)).z;
    // 71: mad r0.z, r0.z, l(0.500000), r2.x
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r2.xxxx)).z;
    // 72: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul r0.w, r1.x, r0.w
    r0.w = ((r1.xxxx)*(r0.wwww)).w;
    // 74: mul_sat r1.x, r1.x, cb0[6].w
    r1.x = (saturate((r1.xxxx)*(source[6].wwww))).x;
    // 75: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 76: log r0.w, r1.x
    r0.w = (log2(r1.xxxx)).w;
    // 77: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 78: mul r0.w, r0.w, cb0[7].x
    r0.w = ((r0.wwww)*(source[7].xxxx)).w;
    // 79: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 80: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 81: movc r0.y, r1.x, l(0), r0.y
    r0.y = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 82: mad_sat r0.y, r0.z, cb0[6].z, r0.y
    r0.y = (saturate((r0.zzzz)*(source[6].zzzz)+(r0.yyyy))).y;
    // 83: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 84: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 86: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 87: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 88: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 89: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 90: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 91: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 92: mul r0.yzw, r1.yyzw, r2.xxyz
    r0.yzw = ((r1.yyzw)*(r2.xxyz)).yzw;
    // 93: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 94: mad r1.xyz, -r2.xyzx, r1.yzwy, r1.xxxx
    r1.xyz = ((-(r2.xyzx))*(r1.yzwy)+(r1.xxxx)).xyz;
    // 95: mad r0.yzw, cb0[6].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[6].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 96: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 97: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 98: mul r0.yzw, r0.yyzw, cb0[6].yyyy
    r0.yzw = ((r0.yyzw)*(source[6].yyyy)).yzw;
    // 99: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 100: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 101: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 102: mad r0.yzw, v3.xxyz, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 103: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 104: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 105: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_turbpa_01_01_tr: 142d7eeccb6dec4b8b4233dcf6db51a2; selected map 1f8ae9aed7bae93344c781be239cb610ad315e63de228050a9f4abce3bea8db8.
float4 ArtistNative2877(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[8].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
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
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 7: mul r1.xyzw, v2.xyxy, cb0[2].xyxy
    r1.xyzw = ((v2.xyxy)*(source[2].xyxy)).xyzw;
    // 8: mul r1.xyzw, r1.xyzw, l(0.500000, 0.500000, 0.300000, 0.300000)
    r1.xyzw = ((r1.xyzw)*(float4(0.500000,0.500000,0.300000,0.300000))).xyzw;
    // 9: mad r1.xyzw, v4.yyyy, l(0.100000, -0.100000, -0.070000, 0.100000), r1.xyzw
    r1.xyzw = ((v4.yyyy)*(float4(0.100000,-0.100000,-0.070000,0.100000))+(r1.xyzw)).xyzw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.xyzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 12: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 13: mad r0.x, r0.x, r0.y, r0.z
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.zzzz)).x;
    // 14: mul r0.y, r0.x, v4.x
    r0.y = ((r0.xxxx)*(v4.xxxx)).y;
    // 15: mad r0.yz, v2.xxyx, cb0[7].xxyx, r0.yyyy
    r0.yz = ((v2.xxyx)*(source[7].xxyx)+(r0.yyyy)).yz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s3, l(0.000000)
    r0.yzw = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 17: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 18: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 19: mul r0.yzw, r0.yyzw, cb0[9].wwww
    r0.yzw = ((r0.yyzw)*(source[9].wwww)).yzw;
    // 20: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 21: mul r1.xy, v2.xyxx, cb0[5].xyxx
    r1.xy = ((v2.xyxx)*(source[5].xyxx)).xy;
    // 22: mad r1.xy, r0.xxxx, cb0[8].yyyy, r1.xyxx
    r1.xy = ((r0.xxxx)*(source[8].yyyy)+(r1.xyxx)).xy;
    // 23: mad r1.zw, r0.xxxx, cb0[11].zzzz, v2.xxxy
    r1.zw = ((r0.xxxx)*(source[11].zzzz)+(v2.xxxy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.zwzz, t3.xyzw, s5, l(0.000000)
    r0.x = (ArtistNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: add r1.zw, r1.xxxy, cb0[6].xxxy
    r1.zw = ((r1.xxxy)+(source[6].xxxy)).zw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 28: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 29: mul r1.x, r1.x, cb0[11].x
    r1.x = ((r1.xxxx)*(source[11].xxxx)).x;
    // 30: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 31: mul r1.x, r1.x, cb0[11].y
    r1.x = ((r1.xxxx)*(source[11].yyyy)).x;
    // 32: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 33: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 34: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t4.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 36: mad r0.xyz, cb0[10].xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 37: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 38: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 39: mad r0.xyz, cb0[10].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[10].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 40: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 41: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 42: mul r0.xyz, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 43: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 44: mul r0.xyz, r0.xyzx, cb0[10].wwww
    r0.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 45: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 46: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_beam_02_tr: b5f3cf037deaba41934695da56fede45; selected map 2ae263e1ba7a5170145f8da51929dbdbabcaacf2584da9fd54ff2f612b8b069e.
float4 ArtistNative2878(ARTIST_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[2] = g_ArtistSourceMaterialParameters[11u];
    source[3] = g_ArtistSourceMaterialParameters[9u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[6].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[8u].xxxx)).x;
    source[6].z = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[7].z = ((g_ArtistSourceMaterialParameters[7u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[8].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[9].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[9].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].w = ((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[11].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[11].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[14].x = ((g_ArtistSourceMaterialParameters[5u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[14].y = ((float4(0.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)).x;
    source[14].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[16].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[16].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
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
    // 1: mul r0.xy, v2.yxyy, cb0[6].xwxx
    r0.xy = ((v2.yxyy)*(source[6].xwxx)).xy;
    // 2: mad r0.x, r0.x, l(4.000000), cb0[6].y
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))+(source[6].yyyy)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 6: mad r0.z, v2.y, cb0[7].x, cb0[7].z
    r0.z = ((v2.yyyy)*(source[7].xxxx)+(source[7].zzzz)).z;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 8: add r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 9: mul r0.y, r0.y, cb0[7].w
    r0.y = ((r0.yyyy)*(source[7].wwww)).y;
    // 10: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 11: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 12: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 13: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 14: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mul_sat r0.y, r0.y, cb0[8].w
    r0.y = (saturate((r0.yyyy)*(source[8].wwww))).y;
    // 16: add r0.z, v2.x, v2.x
    r0.z = ((v2.xxxx)+(v2.xxxx)).z;
    // 17: mad r0.x, r0.x, r0.y, r0.z
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.zzzz)).x;
    // 18: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 19: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[5].x
    r0.w = ((r0.wwww)*(source[5].xxxx)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: mul r0.w, r0.w, cb0[5].y
    r0.w = ((r0.wwww)*(source[5].yyyy)).w;
    // 25: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 26: add r0.x, -|r0.x|, r0.z
    r0.x = ((-(abs(r0.xxxx)))+(r0.zzzz)).x;
    // 27: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 28: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 31: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 32: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 33: mul r1.x, v2.x, cb0[9].w
    r1.x = ((v2.xxxx)*(source[9].wwww)).x;
    // 34: log r0.z, |v2.y|
    r0.z = (log2(abs(v2.yyyy))).z;
    // 35: mul r0.w, r0.z, cb0[10].x
    r0.w = ((r0.zzzz)*(source[10].xxxx)).w;
    // 36: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 37: mul r0.w, r0.w, cb0[10].y
    r0.w = ((r0.wwww)*(source[10].yyyy)).w;
    // 38: lt r1.z, |v2.y|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(v2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 39: movc r0.w, r1.z, l(0), r0.w
    r0.w = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 40: add r1.y, r0.w, cb0[10].w
    r1.y = ((r0.wwww)+(source[10].wwww)).y;
    // 41: add r1.xy, r1.xyxx, cb0[4].xyxx
    r1.xy = ((r1.xyxx)+(source[4].xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xyw, r1.xyxx, t1.xywz, s1, l(0.000000)
    r1.xyw = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 43: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: add r2.xyz, -r1.xywx, r0.wwww
    r2.xyz = ((-(r1.xywx))+(r0.wwww)).xyz;
    // 45: mad r1.xyw, cb0[11].zzzz, r2.xyxz, r1.xyxw
    r1.xyw = ((source[11].zzzz)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 46: max r1.xyw, |r1.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r1.xyw = (max(abs(r1.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 47: log r1.xyw, r1.xyxw
    r1.xyw = (log2(r1.xyxw)).xyw;
    // 48: mul r1.xyw, r1.xyxw, cb0[11].wwww
    r1.xyw = ((r1.xyxw)*(source[11].wwww)).xyw;
    // 49: exp r1.xyw, r1.xyxw
    r1.xyw = (exp2(r1.xyxw)).xyw;
    // 50: mul r1.xyw, r1.xyxw, cb0[12].xxxx
    r1.xyw = ((r1.xyxw)*(source[12].xxxx)).xyw;
    // 51: mul r1.xyw, r1.xyxw, cb0[3].xyxz
    r1.xyw = ((r1.xyxw)*(source[3].xyxz)).xyw;
    // 52: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 53: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 54: mul r1.xyw, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((r1.xyxw)*(source[1].xyxz)).xyw;
    // 55: mul r1.xyw, r1.xyxw, cb0[12].yyyy
    r1.xyw = ((r1.xyxw)*(source[12].yyyy)).xyw;
    // 56: log r0.y, |r1.x|
    r0.y = (log2(abs(r1.xxxx))).y;
    // 57: mul r0.y, r0.y, cb0[14].z
    r0.y = ((r0.yyyy)*(source[14].zzzz)).y;
    // 58: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 59: mul r0.y, r0.y, cb0[14].w
    r0.y = ((r0.yyyy)*(source[14].wwww)).y;
    // 60: mad r0.y, r0.y, l(0.050000), l(-0.025000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).y;
    // 61: lt r0.w, |r1.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 62: movc r0.y, r0.w, l(-0.025000), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(-0.025000,-0.025000,-0.025000,-0.025000)) : (r0.yyyy)).y;
    // 63: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r2.xy, r0.wwww, v6.xyxx
    r2.xy = ((r0.wwww)*(v6.xyxx)).xy;
    // 66: mul r0.w, r0.z, cb0[13].y
    r0.w = ((r0.zzzz)*(source[13].yyyy)).w;
    // 67: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 68: mul r0.w, r0.w, cb0[13].z
    r0.w = ((r0.wwww)*(source[13].zzzz)).w;
    // 69: movc r0.w, r1.z, l(0), r0.w
    r0.w = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 70: add r3.y, r0.w, cb0[14].x
    r3.y = ((r0.wwww)+(source[14].xxxx)).y;
    // 71: mul r3.x, v2.x, cb0[13].x
    r3.x = ((v2.xxxx)*(source[13].xxxx)).x;
    // 72: add r2.zw, r3.xxxy, cb0[14].yyyy
    r2.zw = ((r3.xxxy)+(source[14].yyyy)).zw;
    // 73: add r2.zw, r2.zzzw, cb0[4].xxxy
    r2.zw = ((r2.zzzw)+(source[4].xxxy)).zw;
    // 74: mad r0.yw, r0.yyyy, r2.xxxy, r2.zzzw
    r0.yw = ((r0.yyyy)*(r2.xxxy)+(r2.zzzw)).yw;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 76: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 77: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 78: mul r2.xy, r0.wwww, cb0[15].xzxx
    r2.xy = ((r0.wwww)*(source[15].xzxx)).xy;
    // 79: exp r2.xy, r2.xyxx
    r2.xy = (exp2(r2.xyxx)).xy;
    // 80: movc r0.w, r0.y, l(0), r2.x
    r0.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 81: mul r2.x, r2.y, cb0[15].w
    r2.x = ((r2.yyyy)*(source[15].wwww)).x;
    // 82: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 83: min r2.x, r2.x, l(2.000000)
    r2.x = (min(r2.xxxx,float4(2.000000,2.000000,2.000000,2.000000))).x;
    // 84: movc r0.y, r0.y, l(0), r2.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).y;
    // 85: mul r2.x, r0.z, cb0[12].z
    r2.x = ((r0.zzzz)*(source[12].zzzz)).x;
    // 86: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 87: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 88: mul_sat r0.z, r0.z, cb0[16].y
    r0.z = (saturate((r0.zzzz)*(source[16].yyyy))).z;
    // 89: movc r0.z, r1.z, l(0), r0.z
    r0.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 90: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 91: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 92: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 93: exp r0.x, r2.x
    r0.x = (exp2(r2.xxxx)).x;
    // 94: movc r0.x, r1.z, l(0), r0.x
    r0.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 95: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 96: mad r0.x, cb0[15].y, r0.x, r0.y
    r0.x = ((source[15].yyyy)*(r0.xxxx)+(r0.yyyy)).x;
    // 97: mad r0.xyz, r0.xxxx, r1.xywx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(r1.xywx)+(source[2].xyzx)).xyz;
    // 98: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_spla_05_02_tr: b49d3cdca58aa043b8a0910234843ced; selected map ca2c315c0e3f105a9c3db6598f77c5e701904d32cab7e7d764b3e4d24ffb684c.
float4 ArtistNative2879(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = g_ArtistSourceMaterialParameters[5u];
    source[6] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[9].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)).x;
    source[9].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[12].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0))).x;
    source[12].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[12].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[13].z = ((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)).x;
    source[13].w = (((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[14].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[10].zwzz
    r0.xy = ((v2.xyxx)*(source[10].zwzz)).xy;
    // 2: mad r1.x, cb0[9].y, cb0[10].y, r0.x
    r1.x = ((source[9].yyyy)*(source[10].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[9].y, cb0[11].x, r0.y
    r1.y = ((source[9].yyyy)*(source[11].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.y, v4.z, cb0[11].y
    r0.y = ((v4.zzzz)*(source[11].yyyy)).y;
    // 6: mad r0.xy, r0.xxxx, r0.yyyy, v2.xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(v2.xyxx)).xy;
    // 7: mul r0.z, r0.x, cb0[14].w
    r0.z = ((r0.xxxx)*(source[14].wwww)).z;
    // 8: mad r1.x, cb0[9].y, cb0[14].z, r0.z
    r1.x = ((source[9].yyyy)*(source[14].zzzz)+(r0.zzzz)).x;
    // 9: mul r0.z, r0.y, cb0[15].x
    r0.z = ((r0.yyyy)*(source[15].xxxx)).z;
    // 10: mad r1.y, cb0[9].y, cb0[15].y, r0.z
    r1.y = ((source[9].yyyy)*(source[15].yyyy)+(r0.zzzz)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 12: mov_sat r0.w, v4.y
    r0.w = (saturate(v4.yyyy)).w;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: add r1.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 15: dp2 r2.x, cb0[6].xyxx, r1.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 16: dp2 r2.y, cb0[7].xyxx, r1.xyxx
    r2.y = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 17: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 18: mul r1.xy, r1.xyxx, cb0[8].xyxx
    r1.xy = ((r1.xyxx)*(source[8].xyxx)).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: mad r0.z, r1.x, r0.z, -r0.w
    r0.z = ((r1.xxxx)*(r0.zzzz)+(-(r0.wwww))).z;
    // 21: add r0.w, -r1.x, cb0[14].y
    r0.w = ((-(r1.xxxx))+(source[14].yyyy)).w;
    // 22: add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 23: mul_sat r0.z, r0.z, cb0[15].z
    r0.z = (saturate((r0.zzzz)*(source[15].zzzz))).z;
    // 24: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 25: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r1.x, r1.x, cb0[15].w
    r1.x = ((r1.xxxx)*(source[15].wwww)).x;
    // 27: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 28: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 29: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 30: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 31: movc o0.w, r0.z, l(0), r0.w
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 32: mad r0.zw, cb0[11].zzzz, r0.xxxy, cb0[2].xxxy
    r0.zw = ((source[11].zzzz)*(r0.xxxy)+(source[2].xxxy)).zw;
    // 33: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t3.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 36: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 37: mul r0.x, r0.x, cb0[12].w
    r0.x = ((r0.xxxx)*(source[12].wwww)).x;
    // 38: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 40: mul r0.x, r0.x, cb0[13].x
    r0.x = ((r0.xxxx)*(source[13].xxxx)).x;
    // 41: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 42: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 43: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 44: add r0.xyz, r0.xyzx, cb0[5].xyzx
    r0.xyz = ((r0.xyzx)+(source[5].xyzx)).xyz;
    // 45: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 46: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2817Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2819Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[2] = input.dynamicParameter;
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mov r0.w, cb0[5].z
    r0.w = (source[5].zzzz).w;
    // 2: mov r0.yz, l(0,0,0,0)
    r0.yz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yz;
    // 3: mad r1.xyzw, v2.xyxy, cb0[1].xyxy, r0.wzzw
    r1.xyzw = ((v2.xyxy)*(source[1].xyxy)+(r0.wzzw)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: mul r1.xy, v2.xyxx, cb0[1].xyxx
    r1.xy = ((v2.xyxx)*(source[1].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.zwzz, -r1.xxxx
    r2.xy = ((r0.zwzz)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 13: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 14: div r0.zw, r1.xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)/(r0.zzzz)).zw;
    // 15: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 16: mul r0.x, cb0[2].x, l(0.500000)
    r0.x = ((source[2].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 17: mad r0.xy, v2.xyxx, cb0[4].xyxx, -r0.xyxx
    r0.xy = ((v2.xyxx)*(source[4].xyxx)+(-(r0.xyxx))).xy;
    // 18: add r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)+(source[2].wwww)).y;
    // 19: mad r0.xy, cb0[2].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[2].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 20: mul r0.zw, r0.zzzw, cb0[2].xxxx
    r0.zw = ((r0.zzzw)*(source[2].xxxx)).zw;
    // 21: mad r0.zw, v2.xxxy, cb0[3].xxxy, r0.zzzw
    r0.zw = ((v2.xxxy)*(source[3].xxxy)+(r0.zzzw)).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 23: add r1.xyzw, r0.zwzw, cb0[6].wwww
    r1.xyzw = ((r0.zwzw)+(source[6].wwww)).xyzw;
    // 24: mul r1.xyzw, r1.xyzw, cb0[7].xxxx
    r1.xyzw = ((r1.xyzw)*(source[7].xxxx)).xyzw;
    // 25: mul r1.xyzw, r1.xyzw, cb0[2].zzzz
    r1.xyzw = ((r1.xyzw)*(source[2].zzzz)).xyzw;
    // 26: mul r1.xyzw, r1.xyzw, l(6.283185, 6.283185, 6.283185, 6.283185)
    r1.xyzw = ((r1.xyzw)*(float4(6.283185,6.283185,6.283185,6.283185))).xyzw;
    // 27: sincos r1.xyzw, null, r1.xyzw
    r1.xyzw = (sin(r1.xyzw)).xyzw;
    // 28: max r1.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 30: mul r1.xyzw, r1.xyzw, cb0[7].yyyy
    r1.xyzw = ((r1.xyzw)*(source[7].yyyy)).xyzw;
    // 31: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 32: mul_sat r1.xyzw, r1.xyzw, cb0[7].zzzz
    r1.xyzw = (saturate((r1.xyzw)*(source[7].zzzz))).xyzw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 34: max r0.xyzw, |r0.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 35: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 36: mul r0.xyzw, r0.xyzw, cb0[6].zzzz
    r0.xyzw = ((r0.xyzw)*(source[6].zzzz)).xyzw;
    // 37: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 38: mul r0.xyzw, r1.xyzw, r0.xyzw
    r0.xyzw = ((r1.xyzw)*(r0.xyzw)).xyzw;
    // 39: mul r0.xyzw, r0.xyzw, cb0[7].wwww
    r0.xyzw = ((r0.xyzw)*(source[7].wwww)).xyzw;
    // 40: mad r1.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 42: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 43: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 44: mul r1.zw, r1.zzzw, cb0[8].xxxx
    r1.zw = ((r1.zzzw)*(source[8].xxxx)).zw;
    // 45: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 46: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 47: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 48: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 49: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mul_sat r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = (saturate((r0.xyzw)*(r1.xxxx))).xyzw;
    // 51: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
    // 52: mul r0.xyzw, r0.xyzw, cb0[8].yyyy
    r0.xyzw = ((r0.xyzw)*(source[8].yyyy)).xyzw;
    // 53: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 54: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 55: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 56: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 57: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 58: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 59: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 60: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 61: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 62: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 63: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 64: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 65: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 66: source device depth mapped to centimetre view depth; reconstruction at 68.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 68-71: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 72: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 73: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 74: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 75: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 76: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2820Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mov r0.y, cb0[3].z
    r0.y = (source[3].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.xyzw, v1.xyxy, cb0[0].xyxy, r0.yxxy
    r0.xyzw = ((v1.xyxy)*(source[0].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.zw, v1.xxxy, cb0[0].xxxy
    r0.zw = ((v1.xxxy)*(source[0].xxxy)).zw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 8: add r1.xy, -r0.zzzz, r0.xyxx
    r1.xy = ((-(r0.zzzz))+(r0.xyxx)).xy;
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
    // 16: mul r0.zw, r0.xxxy, v3.xxxx
    r0.zw = ((r0.xxxy)*(v3.xxxx)).zw;
    // 17: mad r0.zw, v1.xxxy, cb0[1].xxxy, r0.zzzw
    r0.zw = ((v1.xxxy)*(source[1].xxxy)+(r0.zzzw)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 19: add r1.xyzw, r0.zwzw, cb0[4].wwww
    r1.xyzw = ((r0.zwzw)+(source[4].wwww)).xyzw;
    // 20: mul r1.xyzw, r1.xyzw, cb0[5].xxxx
    r1.xyzw = ((r1.xyzw)*(source[5].xxxx)).xyzw;
    // 21: mul r1.xyzw, r1.xyzw, v3.zzzz
    r1.xyzw = ((r1.xyzw)*(v3.zzzz)).xyzw;
    // 22: mul r1.xyzw, r1.xyzw, l(6.283185, 6.283185, 6.283185, 6.283185)
    r1.xyzw = ((r1.xyzw)*(float4(6.283185,6.283185,6.283185,6.283185))).xyzw;
    // 23: sincos r1.xyzw, null, r1.xyzw
    r1.xyzw = (sin(r1.xyzw)).xyzw;
    // 24: max r1.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 25: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 26: mul r1.xyzw, r1.xyzw, cb0[5].yyyy
    r1.xyzw = ((r1.xyzw)*(source[5].yyyy)).xyzw;
    // 27: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 28: mul_sat r1.xyzw, r1.xyzw, cb0[5].zzzz
    r1.xyzw = (saturate((r1.xyzw)*(source[5].zzzz))).xyzw;
    // 29: mul r2.x, v3.x, l(0.500000)
    r2.x = ((v3.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 30: mov r2.yz, l(0,0,0,0)
    r2.yz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yz;
    // 31: mad r0.zw, v1.xxxy, cb0[2].xxxy, -r2.xxxy
    r0.zw = ((v1.xxxy)*(source[2].xxxy)+(-(r2.xxxy))).zw;
    // 32: mov r2.w, v3.w
    r2.w = (v3.wwww).w;
    // 33: add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // 34: mad r0.xy, v3.xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((v3.xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 36: max r0.xyzw, |r0.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 37: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 38: mul r0.xyzw, r0.xyzw, cb0[4].zzzz
    r0.xyzw = ((r0.xyzw)*(source[4].zzzz)).xyzw;
    // 39: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 40: mul r0.xyzw, r1.xyzw, r0.xyzw
    r0.xyzw = ((r1.xyzw)*(r0.xyzw)).xyzw;
    // 41: mul r0.xyzw, r0.xyzw, cb0[5].wwww
    r0.xyzw = ((r0.xyzw)*(source[5].wwww)).xyzw;
    // 42: mad r1.xy, v1.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v1.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 43: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 44: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 45: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 46: mul r1.zw, r1.zzzw, cb0[6].xxxx
    r1.zw = ((r1.zzzw)*(source[6].xxxx)).zw;
    // 47: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 48: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 49: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 50: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 51: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = (saturate((r0.xyzw)*(r1.xxxx))).xyzw;
    // 53: mul r0.xyzw, r0.xyzw, v2.wwww
    r0.xyzw = ((r0.xyzw)*(v2.wwww)).xyzw;
    // 54: mul r0.xyzw, r0.xyzw, cb0[6].yyyy
    r0.xyzw = ((r0.xyzw)*(source[6].yyyy)).xyzw;
    // 55: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 56: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 57: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 58: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 59: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 60: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 61: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 62: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 63: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 64: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 65: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 66: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 67: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 68: source device depth mapped to centimetre view depth; reconstruction at 70.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 70-73: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 74: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 75: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 76: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 77: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 78: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2827Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2828Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2830Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2831Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[0].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[0].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[0].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[1].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[1].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[1].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[1].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 12: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
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
    // 21: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 22: movc r0.yw, r0.xxxx, l(0,0,0,0), r0.yyyy
    r0.yw = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).yw;
    // 23: mov r0.xz, cb0[3].xxxx
    r0.xz = (source[3].xxxx).xz;
    // 24: mad r0.xyzw, r0.xyzw, l(0.000000, -2.000000, 0.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(0.000000,-2.000000,0.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 25: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 26: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 27: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 28: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 29: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 30: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 31: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 32: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 33: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 34: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 35: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 36: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 37: source device depth mapped to centimetre view depth; reconstruction at 39.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 39-42: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 43: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 44: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 45: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 46: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 47: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2832Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2833Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[2] = input.dynamicParameter;
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mov r0.w, cb0[5].z
    r0.w = (source[5].zzzz).w;
    // 2: mov r0.yz, l(0,0,0,0)
    r0.yz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yz;
    // 3: mad r1.xyzw, v2.xyxy, cb0[1].xyxy, r0.wzzw
    r1.xyzw = ((v2.xyxy)*(source[1].xyxy)+(r0.wzzw)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: mul r1.xy, v2.xyxx, cb0[1].xyxx
    r1.xy = ((v2.xyxx)*(source[1].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.zwzz, -r1.xxxx
    r2.xy = ((r0.zwzz)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 13: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 14: div r0.zw, r1.xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)/(r0.zzzz)).zw;
    // 15: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 16: mul r0.x, cb0[2].x, l(0.500000)
    r0.x = ((source[2].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 17: mad r0.xy, v2.xyxx, cb0[4].xyxx, -r0.xyxx
    r0.xy = ((v2.xyxx)*(source[4].xyxx)+(-(r0.xyxx))).xy;
    // 18: add r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)+(source[2].wwww)).y;
    // 19: mad r0.xy, cb0[2].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[2].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 20: mul r0.zw, r0.zzzw, cb0[2].xxxx
    r0.zw = ((r0.zzzw)*(source[2].xxxx)).zw;
    // 21: mad r0.zw, v2.xxxy, cb0[3].xxxy, r0.zzzw
    r0.zw = ((v2.xxxy)*(source[3].xxxy)+(r0.zzzw)).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 23: add r1.xyzw, r0.zwzw, cb0[6].wwww
    r1.xyzw = ((r0.zwzw)+(source[6].wwww)).xyzw;
    // 24: mul r1.xyzw, r1.xyzw, cb0[7].xxxx
    r1.xyzw = ((r1.xyzw)*(source[7].xxxx)).xyzw;
    // 25: mul r1.xyzw, r1.xyzw, cb0[2].zzzz
    r1.xyzw = ((r1.xyzw)*(source[2].zzzz)).xyzw;
    // 26: mul r1.xyzw, r1.xyzw, l(6.283185, 6.283185, 6.283185, 6.283185)
    r1.xyzw = ((r1.xyzw)*(float4(6.283185,6.283185,6.283185,6.283185))).xyzw;
    // 27: sincos r1.xyzw, null, r1.xyzw
    r1.xyzw = (sin(r1.xyzw)).xyzw;
    // 28: max r1.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 30: mul r1.xyzw, r1.xyzw, cb0[7].yyyy
    r1.xyzw = ((r1.xyzw)*(source[7].yyyy)).xyzw;
    // 31: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 32: mul_sat r1.xyzw, r1.xyzw, cb0[7].zzzz
    r1.xyzw = (saturate((r1.xyzw)*(source[7].zzzz))).xyzw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 34: max r0.xyzw, |r0.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 35: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 36: mul r0.xyzw, r0.xyzw, cb0[6].zzzz
    r0.xyzw = ((r0.xyzw)*(source[6].zzzz)).xyzw;
    // 37: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 38: mul r0.xyzw, r1.xyzw, r0.xyzw
    r0.xyzw = ((r1.xyzw)*(r0.xyzw)).xyzw;
    // 39: mul r0.xyzw, r0.xyzw, cb0[7].wwww
    r0.xyzw = ((r0.xyzw)*(source[7].wwww)).xyzw;
    // 40: mad r1.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 42: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 43: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 44: mul r1.zw, r1.zzzw, cb0[8].xxxx
    r1.zw = ((r1.zzzw)*(source[8].xxxx)).zw;
    // 45: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 46: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 47: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 48: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 49: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mul_sat r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = (saturate((r0.xyzw)*(r1.xxxx))).xyzw;
    // 51: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
    // 52: mul r0.xyzw, r0.xyzw, cb0[8].yyyy
    r0.xyzw = ((r0.xyzw)*(source[8].yyyy)).xyzw;
    // 53: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 54: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 55: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 56: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 57: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 58: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 59: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 60: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 61: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 62: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 63: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 64: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 65: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 66: source device depth mapped to centimetre view depth; reconstruction at 68.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 68-71: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 72: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 73: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 74: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 75: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 76: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2834Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
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
    // 6: mul r1.xyz, v5.yyyy, cb1[1].xywx
    r1.xyz = ((v5.yyyy)*(projection[1].xywx)).xyz;
    // 7: mad r1.xyz, cb1[0].xywx, v5.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v5.xxxx)+(r1.xyzx)).xyz;
    // 8: mad r1.xyz, cb1[2].xywx, v5.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v5.zzzz)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[3].xywx, v5.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v5.wwww)+(r1.xyzx)).xyz;
    // 10: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 11: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 12: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 13: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 14: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 15: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 16: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 17: source device depth mapped to centimetre view depth; reconstruction at 19.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 19-22: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 23: ge r0.x, r1.z, r0.x
    r0.x = (asfloat((uint4)((r1.zzzz)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 24: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 25: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 26: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 27: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2835Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2836Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[2] = ArtistNativeAppend(float4(0.100000001, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[3] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww),float4(0.0, 0.0, 0.0, 0.0),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mov r0.y, cb0[4].z
    r0.y = (source[4].zzzz).y;
    // 2: mov r0.xz, l(0,0,1.000000,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 3: mad r1.xyzw, v1.zwzw, cb0[0].xyxy, r0.yxxy
    r1.xyzw = ((v1.zwzw)*(source[0].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r1.xy, v1.zwzz, cb0[0].xyxx
    r1.xy = ((v1.zwzz)*(source[0].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r2.xy, r0.xyxx, -r1.xxxx
    r2.xy = ((r0.xyxx)+(-(r1.xxxx))).xy;
    // 9: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 11: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 12: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: div r0.xy, r1.xyxx, r0.xxxx
    r0.xy = ((r1.xyxx)/(r0.xxxx)).xy;
    // 15: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: mov r0.w, v3.x
    r0.w = (v3.xxxx).w;
    // 17: mul r0.xy, r0.zwzz, r0.xyxx
    r0.xy = ((r0.zwzz)*(r0.xyxx)).xy;
    // 18: mov r1.x, v3.x
    r1.x = (v3.xxxx).x;
    // 19: mov r1.y, l(0.200000)
    r1.y = (float4(0.200000,0.200000,0.200000,0.200000)).y;
    // 20: mul r0.zw, r0.xxxy, r1.xxxy
    r0.zw = ((r0.xxxy)*(r1.xxxy)).zw;
    // 21: mad r0.zw, v1.zzzw, cb0[1].xxxy, r0.zzzw
    r0.zw = ((v1.zzzw)*(source[1].xxxy)+(r0.zzzw)).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 23: add r2.xyzw, r0.zwzw, cb0[5].wwww
    r2.xyzw = ((r0.zwzw)+(source[5].wwww)).xyzw;
    // 24: mul r2.xyzw, r2.xyzw, cb0[6].xxxx
    r2.xyzw = ((r2.xyzw)*(source[6].xxxx)).xyzw;
    // 25: mul r2.xyzw, r2.xyzw, v3.zzzz
    r2.xyzw = ((r2.xyzw)*(v3.zzzz)).xyzw;
    // 26: mul r2.xyzw, r2.xyzw, l(6.283185, 6.283185, 6.283185, 6.283185)
    r2.xyzw = ((r2.xyzw)*(float4(6.283185,6.283185,6.283185,6.283185))).xyzw;
    // 27: sincos r2.xyzw, null, r2.xyzw
    r2.xyzw = (sin(r2.xyzw)).xyzw;
    // 28: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 30: mul r2.xyzw, r2.xyzw, cb0[6].yyyy
    r2.xyzw = ((r2.xyzw)*(source[6].yyyy)).xyzw;
    // 31: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 32: mul_sat r2.xyzw, r2.xyzw, cb0[6].zzzz
    r2.xyzw = (saturate((r2.xyzw)*(source[6].zzzz))).xyzw;
    // 33: add r0.zw, v1.zzzw, cb0[2].xxxy
    r0.zw = ((v1.zzzw)+(source[2].xxxy)).zw;
    // 34: mad r0.xy, r1.xyxx, r0.xyxx, r0.zwzz
    r0.xy = ((r1.xyxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 35: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 37: max r0.xyzw, |r0.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 38: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 39: mul r0.xyzw, r0.xyzw, cb0[5].zzzz
    r0.xyzw = ((r0.xyzw)*(source[5].zzzz)).xyzw;
    // 40: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 41: mul r0.xyzw, r2.xyzw, r0.xyzw
    r0.xyzw = ((r2.xyzw)*(r0.xyzw)).xyzw;
    // 42: mul r0.xyzw, r0.xyzw, cb0[6].wwww
    r0.xyzw = ((r0.xyzw)*(source[6].wwww)).xyzw;
    // 43: mad r1.x, v1.z, l(2.000000), l(-1.000000)
    r1.x = ((v1.zzzz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 44: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 46: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 47: mul r1.y, r1.y, cb0[7].x
    r1.y = ((r1.yyyy)*(source[7].xxxx)).y;
    // 48: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 49: mul r1.y, r1.y, l(3.000000)
    r1.y = ((r1.yyyy)*(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 50: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 52: mul_sat r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = (saturate((r0.xyzw)*(r1.xxxx))).xyzw;
    // 53: mul r0.xyzw, r0.xyzw, v2.wwww
    r0.xyzw = ((r0.xyzw)*(v2.wwww)).xyzw;
    // 54: mul r0.xyzw, r0.xyzw, cb0[7].yyyy
    r0.xyzw = ((r0.xyzw)*(source[7].yyyy)).xyzw;
    // 55: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 56: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 57: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 58: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 59: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 60: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 61: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 62: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 63: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 64: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 65: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 66: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 67: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 68: source device depth mapped to centimetre view depth; reconstruction at 70.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 70-73: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 74: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 75: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 76: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 77: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 78: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2837Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2838Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2840Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2841Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2843Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2846Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2850Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2853Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2857Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2859Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2860Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2861Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2869Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2870Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2874Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
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
    // 6: mul r1.xyz, v5.yyyy, cb1[1].xywx
    r1.xyz = ((v5.yyyy)*(projection[1].xywx)).xyz;
    // 7: mad r1.xyz, cb1[0].xywx, v5.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v5.xxxx)+(r1.xyzx)).xyz;
    // 8: mad r1.xyz, cb1[2].xywx, v5.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v5.zzzz)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[3].xywx, v5.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v5.wwww)+(r1.xyzx)).xyz;
    // 10: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 11: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 12: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 13: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 14: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 15: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 16: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 17: source device depth mapped to centimetre view depth; reconstruction at 19.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 19-22: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 23: ge r0.x, r1.z, r0.x
    r0.x = (asfloat((uint4)((r1.zzzz)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 24: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 25: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 26: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 27: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2876Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2878Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[1].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[1].z = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[1].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].x = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[2].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[8u].xxxx)).x;
    source[2].z = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[3].z = ((g_ArtistSourceMaterialParameters[7u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[3].w = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[4].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[4].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[4].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[5u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[5].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
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
    // 1: mul r0.xy, v1.yxyy, cb0[2].xwxx
    r0.xy = ((v1.yxyy)*(source[2].xwxx)).xy;
    // 2: mad r0.x, r0.x, l(4.000000), cb0[2].y
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))+(source[2].yyyy)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: mul r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)*(source[2].zzzz)).x;
    // 6: mad r0.z, v1.y, cb0[3].x, cb0[3].z
    r0.z = ((v1.yyyy)*(source[3].xxxx)+(source[3].zzzz)).z;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 8: add r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 9: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 10: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 11: add r0.yz, v1.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v1.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 12: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 13: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 14: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mul_sat r0.y, r0.y, cb0[4].w
    r0.y = (saturate((r0.yyyy)*(source[4].wwww))).y;
    // 16: add r0.z, v1.x, v1.x
    r0.z = ((v1.xxxx)+(v1.xxxx)).z;
    // 17: mad r0.x, r0.x, r0.y, r0.z
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.zzzz)).x;
    // 18: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 19: add r0.z, -v1.y, l(1.000000)
    r0.z = ((-(v1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[1].x
    r0.w = ((r0.wwww)*(source[1].xxxx)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: mul r0.w, r0.w, cb0[1].y
    r0.w = ((r0.wwww)*(source[1].yyyy)).w;
    // 25: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 26: add r0.x, -|r0.x|, r0.z
    r0.x = ((-(abs(r0.xxxx)))+(r0.zzzz)).x;
    // 27: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 28: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 31: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 32: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 33: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 34: log r0.y, |v1.y|
    r0.y = (log2(abs(v1.yyyy))).y;
    // 35: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: mul_sat r0.y, r0.y, cb0[6].y
    r0.y = (saturate((r0.yyyy)*(source[6].yyyy))).y;
    // 38: lt r0.z, |v1.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(v1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 39: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 40: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 41: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 42: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 43: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative2879Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)).x;
    source[3].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[4].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xy, v1.xyxx, cb0[4].zwzz
    r0.xy = ((v1.xyxx)*(source[4].zwzz)).xy;
    // 2: mad r1.x, cb0[3].y, cb0[4].y, r0.x
    r1.x = ((source[3].yyyy)*(source[4].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[3].y, cb0[5].x, r0.y
    r1.y = ((source[3].yyyy)*(source[5].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.y, v3.z, cb0[5].y
    r0.y = ((v3.zzzz)*(source[5].yyyy)).y;
    // 6: mad r0.xy, r0.xxxx, r0.yyyy, v1.xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(v1.xyxx)).xy;
    // 7: mul r0.z, r0.x, cb0[6].w
    r0.z = ((r0.xxxx)*(source[6].wwww)).z;
    // 8: mad r1.x, cb0[3].y, cb0[6].z, r0.z
    r1.x = ((source[3].yyyy)*(source[6].zzzz)+(r0.zzzz)).x;
    // 9: mul r0.z, r0.y, cb0[7].x
    r0.z = ((r0.yyyy)*(source[7].xxxx)).z;
    // 10: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 11: mad r1.y, cb0[3].y, cb0[7].y, r0.z
    r1.y = ((source[3].yyyy)*(source[7].yyyy)+(r0.zzzz)).y;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: dp2 r1.x, cb0[0].xyxx, r0.xyxx
    r1.x = (dot((source[0].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 14: dp2 r1.y, cb0[1].xyxx, r0.xyxx
    r1.y = (dot((source[1].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 15: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 18: mov_sat r0.y, v3.y
    r0.y = (saturate(v3.yyyy)).y;
    // 19: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 20: mad r0.y, r0.x, r0.z, -r0.y
    r0.y = ((r0.xxxx)*(r0.zzzz)+(-(r0.yyyy))).y;
    // 21: add r0.x, -r0.x, cb0[6].y
    r0.x = ((-(r0.xxxx))+(source[6].yyyy)).x;
    // 22: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 23: mul_sat r0.y, r0.y, cb0[7].z
    r0.y = (saturate((r0.yyyy)*(source[7].zzzz))).y;
    // 24: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 25: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 27: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 28: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 29: mul r0.z, v3.w, cb0[8].x
    r0.z = ((v3.wwww)*(source[8].xxxx)).z;
    // 30: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 31: mad r1.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 32: movc r0.xyzw, r0.yyyy, l(-1.000000,1.000000,-1.000000,1.000000), r1.xyzw
    r0.xyzw = ((asuint(r0.yyyy) != 0u) ? (float4(-1.000000,1.000000,-1.000000,1.000000)) : (r1.xyzw)).xyzw;
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
    // 38: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
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
    // 51: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
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
