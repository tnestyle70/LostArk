// Original Kouku material programs 5248..5311; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_ht_03_ad: 91be15ced4f10d4b8ea34e3043e66d0e; selected map 405418a6d015fb4a844027832b2279d682d53ec175f8953ae16db22bc6c30593.
float4 ArtistNative5248(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t2.xywz, s1, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 12: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 13: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 14: mul r1.x, r1.x, cb0[4].y
    r1.x = ((r1.xxxx)*(source[4].yyyy)).x;
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
    // 22: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 23: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 24: mul r1.xyz, r0.xywx, cb0[3].wwww
    r1.xyz = ((r0.xywx)*(source[3].wwww)).xyz;
    // 25: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: mad r0.xyw, -cb0[3].wwww, r0.xyxw, r1.wwww
    r0.xyw = ((-(source[3].wwww))*(r0.xyxw)+(r1.wwww)).xyw;
    // 27: mad r0.xyw, cb0[4].xxxx, r0.xyxw, r1.xyxz
    r0.xyw = ((source[4].xxxx)*(r0.xyxw)+(r1.xyxz)).xyw;
    // 28: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 29: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 30: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 31: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5248Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[0].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[0].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[0].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[1].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[1].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[1].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[1].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 21: mul r0.y, r0.y, cb0[2].z
    r0.y = ((r0.yyyy)*(source[2].zzzz)).y;
    // 22: mad r1.xyzw, r0.yyyy, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r0.yyyy)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 23: movc r0.xyzw, r0.xxxx, l(-1.000000,1.000000,-1.000000,1.000000), r1.xyzw
    r0.xyzw = ((asuint(r0.xxxx) != 0u) ? (float4(-1.000000,1.000000,-1.000000,1.000000)) : (r1.xyzw)).xyzw;
    // 24: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 25: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 26: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 27: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 28: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 29: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 30: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 31: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 32: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 33: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 34: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 35: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 36: source device depth mapped to centimetre view depth; reconstruction at 38.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 38-41: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 42: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 43: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 44: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 45: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 46: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_crackmove_07_tr: 0a5a7ebca2041e418f40b9e0aae26309; selected map 31a2db99f0537a635a2e725c31cd4ce624630eab51e7af2cc5e0a88b7628cb4b.
float4 ArtistNative5249(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[4].x = ((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
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
    r0.xzw = (ArtistNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xwyz).xzw;
    // 49: mul r2.y, r0.y, cb0[6].y
    r2.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s0, l(0.000000)
    r1.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.xwyz, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_de_crack_01_4_tr: c03ef74fecbf0243a343e11d3ab63d0a; selected map 82054e002067bf206e81d4ce2f91d0d2c3fca75d9bb82456d375d8fd99f3d7ef.
float4 ArtistNative5250(ARTIST_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[14]=float4(input.skyUpperColor,0.f);
    source[15]=float4(input.skyLowerColor,0.f);
    source[16]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ArtistSourceMaterialParameters[5u];
    source[6] = g_ArtistSourceMaterialParameters[4u];
    source[7].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].w = (clamp(g_ArtistSourceMaterialParameters[2u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[8].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ArtistSourceMaterialParameters[2u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].x = ((g_ArtistSourceMaterialParameters[0u].xxxx+g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[10].y = ((float4(0.0, 0.0, 0.0, 0.0)*(g_ArtistSourceMaterialParameters[0u].xxxx+g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[11].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[11].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[11].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 11: mul r0.xy, v4.xyxx, cb0[7].xxxx
    r0.xy = ((v4.xyxx)*(source[7].xxxx)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 13: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 14: mad_sat r0.x, r0.x, cb0[7].y, l(0.500000)
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy)+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 15: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 16: dp2 r0.w, r0.yzyy, r0.yzyy
    r0.w = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).w;
    // 17: mad r0.yz, cb0[8].wwww, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((source[8].wwww)*(r0.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 18: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(-1.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 19: sqrt r0.z, r0.w
    r0.z = (sqrt(r0.wwww)).z;
    // 20: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 21: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 22: mul r0.z, r0.z, cb0[8].x
    r0.z = ((r0.zzzz)*(source[8].xxxx)).z;
    // 23: mad r0.x, r0.x, cb0[7].w, r0.z
    r0.x = ((r0.xxxx)*(source[7].wwww)+(r0.zzzz)).x;
    // 24: add r0.z, -cb0[1].x, l(1.000000)
    r0.z = ((-(source[1].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: mad_sat r0.y, r0.y, r0.x, -r0.z
    r0.y = (saturate((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz)))).y;
    // 26: add r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)+(source[1].yyyy)).x;
    // 27: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 28: mul_sat r0.x, r0.x, cb0[8].y
    r0.x = (saturate((r0.xxxx)*(source[8].yyyy))).x;
    // 29: mul_sat r0.y, r0.y, cb0[9].x
    r0.y = (saturate((r0.yyyy)*(source[9].xxxx))).y;
    // 30: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 31: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 32: mul r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)*(source[9].yyyy)).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 35: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 36: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 37: mul r0.z, r0.z, cb0[8].z
    r0.z = ((r0.zzzz)*(source[8].zzzz)).z;
    // 38: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 39: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 40: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 41: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 43: mad r0.x, r0.x, r0.y, cb0[10].y
    r0.x = ((r0.xxxx)*(r0.yyyy)+(source[10].yyyy)).x;
    // 44: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 45: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 46: add r0.y, -|v4.w|, cb0[0].y
    r0.y = ((-(abs(v4.wwww)))+(source[0].yyyy)).y;
    // 47: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 48: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 49: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 50: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    // 51: mov_sat r0.x, cb0[1].z
    r0.x = (saturate(source[1].zzzz)).x;
    // 52: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 54: add r0.x, -r0.x, r0.y
    r0.x = ((-(r0.xxxx))+(r0.yyyy)).x;
    // 55: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 56: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 57: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 58: mul r0.y, r0.y, cb0[10].w
    r0.y = ((r0.yyyy)*(source[10].wwww)).y;
    // 59: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 60: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 61: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, v4.xyxx, t5.wxyz, s5, l(-1.000000)
    r0.yzw = (ArtistNativeSample5((v4.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 62: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 63: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 64: mad r0.yzw, cb0[11].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[11].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 65: mul r1.xyz, cb0[6].xyzx, cb0[6].wwww
    r1.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 66: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 67: add r1.xy, v4.xyxx, cb0[4].xyxx
    r1.xy = ((v4.xyxx)+(source[4].xyxx)).xy;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 69: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 70: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 71: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 72: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 73: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 74: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 75: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 76: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 77: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 78: mul r1.yzw, r1.yyyy, cb0[15].xxyz
    r1.yzw = ((r1.yyyy)*(source[15].xxyz)).yzw;
    // 79: mad r1.xyz, r1.xxxx, cb0[14].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[14].xyzx)+(r1.yzwy)).xyz;
    // 80: mul r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = ((r1.xyzx)*(source[16].wwww)).xyz;
    // 81: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[3].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[3].xyzx)).xyz;
    // 82: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 84: mad r1.xyz, r0.xyzx, cb0[16].xyzx, r2.xyzx
    r1.xyz = ((r0.xyzx)*(source[16].xyzx)+(r2.xyzx)).xyz;
    // 86: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_floorstrm_03_4_tr: 4268ec628979214cb96296a51fbbf893; selected map 74a5216f9f9f064a1d84e3d566ace0d054f8e6363dd807e6a98acce147196e5e.
float4 ArtistNative5251(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[5].zwzz
    r0.xy = ((v2.xyxx)*(source[5].zwzz)).xy;
    // 2: mul r0.z, cb0[4].x, cb0[4].y
    r0.z = ((source[4].xxxx)*(source[4].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[5].y, r0.x
    r1.x = ((r0.zzzz)*(source[5].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[6].x, r0.y
    r1.y = ((r0.zzzz)*(source[6].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[6].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[6].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[4].w
    r0.w = ((r0.xxxx)*(source[4].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[4].z, r0.w
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.wwww)).x;
    // 9: mul r0.z, r0.z, cb0[6].w
    r0.z = ((r0.zzzz)*(source[6].wwww)).z;
    // 10: mad r1.y, cb0[5].x, r0.y, r0.z
    r1.y = ((source[5].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 14: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 15: mad r1.xyz, cb0[7].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[7].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 16: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 18: mul r1.xyz, r1.xyzx, cb0[7].yyyy
    r1.xyz = ((r1.xyzx)*(source[7].yyyy)).xyz;
    // 19: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 20: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 21: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 22: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 23: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 24: mad r0.z, cb0[4].y, cb0[7].w, cb0[8].x
    r0.z = ((source[4].yyyy)*(source[7].wwww)+(source[8].xxxx)).z;
    // 25: sincos r1.x, r2.x, r0.z
    { const float4 sourceAngle = r0.zzzz; r1.x = (sin(sourceAngle)).x; r2.x = (cos(sourceAngle)).x; }
    // 26: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 27: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 28: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 29: dp2 r0.z, r3.zyzz, r0.xyxx
    r0.z = (dot((r3.zyzz).xy,(r0.xyxx).xy).xxxx).z;
    // 30: dp2 r0.x, r3.yxyy, r0.xyxx
    r0.x = (dot((r3.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 31: mul r1.z, r0.z, cb0[3].y
    r1.z = ((r0.zzzz)*(source[3].yyyy)).z;
    // 32: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 33: mad r1.x, r0.x, cb0[3].x, r0.y
    r1.x = ((r0.xxxx)*(source[3].xxxx)+(r0.yyyy)).x;
    // 34: add r0.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 36: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, cb0[8].w
    r0.x = (saturate((r0.xxxx)*(source[8].wwww))).x;
    // 39: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 40: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 41: mul r0.x, r0.x, cb0[9].x
    r0.x = ((r0.xxxx)*(source[9].xxxx)).x;
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
    // 48: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: mul_sat r0.w, r0.w, cb0[9].z
    r0.w = (saturate((r0.wwww)*(source[9].zzzz))).w;
    // 51: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 52: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 53: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 54: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 55: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5251Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[1].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[1].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[1].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[1].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v2 = input.color; // native texcoord1
    float4 v3 = input.dynamicParameter; // native texcoord2
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.x, cb0[1].x, cb0[1].y
    r0.x = ((source[1].xxxx)*(source[1].yyyy)).x;
    // 2: mul r0.yz, v1.xxyx, cb0[2].zzwz
    r0.yz = ((v1.xxyx)*(source[2].zzwz)).yz;
    // 3: mad r1.x, r0.x, cb0[2].y, r0.y
    r1.x = ((r0.xxxx)*(source[2].yyyy)+(r0.yyyy)).x;
    // 4: mad r1.y, r0.x, cb0[3].x, r0.z
    r1.y = ((r0.xxxx)*(source[3].xxxx)+(r0.zzzz)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[3].zzzz, r0.xyxx, v1.xyxx
    r0.xy = ((source[3].zzzz)*(r0.xyxx)+(v1.xyxx)).xy;
    // 7: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 8: mad r0.z, cb0[1].y, cb0[4].w, cb0[5].x
    r0.z = ((source[1].yyyy)*(source[4].wwww)+(source[5].xxxx)).z;
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
    // 15: mul r1.z, r0.z, cb0[0].y
    r1.z = ((r0.zzzz)*(source[0].yyyy)).z;
    // 16: add r0.y, v3.y, l(-1.000000)
    r0.y = ((v3.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 17: mad r1.x, r0.x, cb0[0].x, r0.y
    r1.x = ((r0.xxxx)*(source[0].xxxx)+(r0.yyyy)).x;
    // 18: add r0.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 21: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 22: mul r0.y, r0.y, v5.z
    r0.y = ((r0.yyyy)*(v5.zzzz)).y;
    // 23: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 24: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: mul r0.z, r0.z, cb0[6].y
    r0.z = ((r0.zzzz)*(source[6].yyyy)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: mul_sat r0.z, r0.z, cb0[6].z
    r0.z = (saturate((r0.zzzz)*(source[6].zzzz))).z;
    // 28: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 29: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 30: mul r0.x, r0.x, cb0[6].w
    r0.x = ((r0.xxxx)*(source[6].wwww)).x;
    // 31: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_ht_01_1_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ArtistNative5252(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[5].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mad r0.xy, cb0[5].xxxx, cb0[3].xyxx, v2.xyxx
    r0.xy = ((source[5].xxxx)*(source[3].xyxx)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[5].wwww
    r0.xy = ((r0.xyxx)*(source[5].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_dist_04_1_ad: c5628b65281b75468f8a9cd23a6f76d2; selected map e271d0b463b325de428e733f479d3d08d9fb86bbe2315ecb125b4463386cdc42.
float4 ArtistNative5253(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t0.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t2.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 19: log r0.y, |r0.w|
    r0.y = (log2(abs(r0.wwww))).y;
    // 20: lt r0.z, |r0.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 21: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 22: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 23: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 24: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 25: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 26: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 27: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 28: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 29: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 30: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 31: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 32: mul r0.yzw, r1.xxyz, cb0[2].yyyy
    r0.yzw = ((r1.xxyz)*(source[2].yyyy)).yzw;
    // 33: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: mad r1.xyz, -cb0[2].yyyy, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[2].yyyy))*(r1.xyzx)+(r1.wwww)).xyz;
    // 35: mad r0.yzw, cb0[2].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[2].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 36: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 37: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 38: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 39: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5253Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[0].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[0].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[0].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[1].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[1].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mad r0.x, cb0[0].x, v3.x, l(-1.000000)
    r0.x = ((source[0].xxxx)*(v3.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v3.x, cb0[0].x
    r0.y = ((v3.xxxx)*(source[0].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v1.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v1.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 7: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 8: mul r0.y, r0.y, cb0[0].w
    r0.y = ((r0.yyyy)*(source[0].wwww)).y;
    // 9: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 10: mul r0.y, r0.y, v2.w
    r0.y = ((r0.yyyy)*(v2.wwww)).y;
    // 11: movc r0.x, r0.x, l(0), |r0.y|
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).x;
    // 12: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 13: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 14: mul r0.y, r0.y, v3.y
    r0.y = ((r0.yyyy)*(v3.yyyy)).y;
    // 15: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 16: mul r0.y, r0.y, cb0[1].y
    r0.y = ((r0.yyyy)*(source[1].yyyy)).y;
    // 17: div r0.zw, v4.xxxy, v4.wwww
    r0.zw = ((v4.xxxy)/(v4.wwww)).zw;
    // 18: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 19: source device depth mapped to centimetre view depth; reconstruction at 21.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 21-24: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 25: add r1.x, r1.x, -v4.w
    r1.x = ((r1.xxxx)+(-(v4.wwww))).x;
    // 26: add r1.y, -cb0[1].x, l(1.000000)
    r1.y = ((-(source[1].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 27: max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 28: div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // 29: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 30: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 31: mul r1.xz, r0.xxxx, l(2.000000, 0.000000, 2.000000, 0.000000)
    r1.xz = ((r0.xxxx)*(float4(2.000000,0.000000,2.000000,0.000000))).xz;
    // 32: mov r1.yw, l(0,-0.000000,0,-0.000000)
    r1.yw = (float4(asfloat(0u),-0.000000,asfloat(0u),-0.000000)).yw;
    // 33: add r1.xyzw, r1.xyzw, l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r1.xyzw)+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 34: mad r1.xyzw, r1.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r1.xyzw = ((r1.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 35: dp2 r0.x, r1.zwzz, r1.zwzz
    r0.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 36: add r0.x, r0.x, l(-0.100000)
    r0.x = ((r0.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 37: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 38: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) return 0.f;
    // 39: mad r0.xy, r1.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r0.zwzz
    r0.xy = ((r1.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r0.zwzz)).xy;
    // 40: mul r0.zw, r1.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r1.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 41: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 42: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 43: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 44: source device depth mapped to centimetre view depth; reconstruction at 46.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 46-49: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 50: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 51: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 52: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 53: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 54: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ht_18_1_tr: a14d57c96465f346be3437ad63c9ad10; selected map 0716202d132c7fa3cb9c994bb5370a1db2710298fb3e58040e580c23ad3e1c1b.
float4 ArtistNative5254(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,g_ArtistSourceMaterialParameters[3u].xxxx,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[10].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[3u].zzzz)).x;
    source[13].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[3u].zzzz))).x;
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
    // 1: mad r0.xy, cb0[3].xyxx, v2.xyxx, v4.xyxx
    r0.xy = ((source[3].xyxx)*(v2.xyxx)+(v4.xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, cb0[11].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[11].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 4: mad r1.x, cb0[4].x, r0.x, cb0[5].x
    r1.x = ((source[4].xxxx)*(r0.xxxx)+(source[5].xxxx)).x;
    // 5: mad r1.y, cb0[4].y, r0.y, cb0[6].y
    r1.y = ((source[4].yyyy)*(r0.yyyy)+(source[6].yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mad r1.x, cb0[7].x, r0.x, cb0[8].x
    r1.x = ((source[7].xxxx)*(r0.xxxx)+(source[8].xxxx)).x;
    // 8: mad r1.y, cb0[7].y, r0.y, cb0[9].y
    r1.y = ((source[7].yyyy)*(r0.yyyy)+(source[9].yyyy)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_worldoffset_02_01_tr: c33eb51395d71c4b804cf843eee2a488; selected map fa9a896d4a29e62ac45e73baafbd861d00c4f40edd027dbe3705b659446031e2.
float4 ArtistNative5255(ARTIST_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f); // Absolute source world origin and original opacity W.
    [unroll] for(uint row=0u;row<3u;++row) source[1u+row]=g_ArtistSourceWorldToLocal[row];
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
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
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
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5255Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_decmaster_01_22_tr: 9415e9ca2c02c44eb1224d42153fc97b; selected map 4ab5f573965d90995e32a3924042691fa996f95680ccf046236f73b2b272f873.
float4 ArtistNative5256(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].zzzz,1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ArtistSourceMaterialParameters[3u];
    source[6].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 2: mad r0.xy, r0.xyxx, cb0[3].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(source[3].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.yxzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 4: mul r0.yz, v2.xxyx, cb0[6].xxxx
    r0.yz = ((v2.xxyx)*(source[6].xxxx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mad r0.yz, r1.xxyx, l(0.000000, 0.200000, 0.200000, 0.000000), v2.xxyx
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.200000,0.200000,0.000000))+(v2.xxyx)).yz;
    // 7: add r0.yz, r0.yyzy, cb0[4].xxyx
    r0.yz = ((r0.yyzy)+(source[4].xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 24: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 25: mad r0.xyz, r1.xyzx, v3.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(v3.xyzx)+(r0.xyzx)).xyz;
    // 26: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 27: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 28: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 29: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 30: mul r0.x, r0.x, cb0[8].y
    r0.x = ((r0.xxxx)*(source[8].yyyy)).x;
    // 31: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 32: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 33: mul r0.yz, v2.xxyx, cb0[8].zzzz
    r0.yz = ((v2.xxyx)*(source[8].zzzz)).yz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 35: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 36: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 38: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 39: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 40: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_31_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative5257(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative5257Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_ring_11_09_ts_tr: 14465aa42edf404dafced7434bca7e8a; selected map 01d7f91374213b751435f61477a95c92aa20c2401f42bff27704c5421c25ef4c.
float4 ArtistNative5258(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].xxxx))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[4].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
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
    // 23: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 24: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 25: source device depth mapped to centimetre view depth; reconstruction at 27.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 27-30: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 31: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 32: add r0.z, -cb0[4].x, l(1.000000)
    r0.z = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 34: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 35: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 36: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 37: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 38: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 39: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5258Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_ring_12_ad_inst35: 2ff88b3f0d9a8c43a28b46e6f7a596b5; selected map d997538f1226f51ee7f6c69629306ef01597ba8e4708973b644d0a8f6facf118.
float4 ArtistNative5259(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].wwww))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 22: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 23: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 24: source device depth mapped to centimetre view depth; reconstruction at 26.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 26-29: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 30: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 31: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 33: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 34: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 35: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 36: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 37: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 38: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 39: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 40: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 41: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5259Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_c_pa_ring_06_ad: e91186b92721ed429a45977dbd4c5e7e; selected map 1994ac3e07582e978a16e8fdfc9b15127be2206134c65dde6db4fc3232563321.
float4 ArtistNative5260(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2].x = (((float4(1.70000005, 0.0, 0.0, 0.0)+sin((g_ArtistSourceMaterialTime.xxxx*float4(20.9439507, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))).x;
    source[2].y = ((((float4(1.70000005, 0.0, 0.0, 0.0)+sin((g_ArtistSourceMaterialTime.xxxx*float4(20.9439507, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))*((float4(1.70000005, 0.0, 0.0, 0.0)+cos((g_ArtistSourceMaterialTime.xxxx*float4(8.9759798, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0)))).x;
    source[2].z = (clamp((((float4(1.70000005, 0.0, 0.0, 0.0)+sin((g_ArtistSourceMaterialTime.xxxx*float4(20.9439507, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))*((float4(1.70000005, 0.0, 0.0, 0.0)+cos((g_ArtistSourceMaterialTime.xxxx*float4(8.9759798, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))),float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
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
    // 7: mul r0.yz, r0.yyyy, l(0.000000, 15.000000, 30.000000, 0.000000)
    r0.yz = ((r0.yyyy)*(float4(0.000000,15.000000,30.000000,0.000000))).yz;
    // 8: exp r0.yz, r0.yyzy
    r0.yz = (exp2(r0.yyzy)).yz;
    // 9: mul r0.y, r0.y, l(3.000000)
    r0.y = ((r0.yyyy)*(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 10: min r0.yz, r0.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = (min(r0.yyzy,float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 11: movc r0.z, r0.x, l(-0.000000), -r0.z
    r0.z = ((asuint(r0.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.zzzz))).z;
    // 12: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 13: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 14: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 15: mul r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)*(r0.xxxx)).y;
    // 16: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mad r0.z, cb0[2].z, l(0.500000), l(0.500000)
    r0.z = ((source[2].zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 18: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 19: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 20: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 21: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 22: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 23: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
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
// fx_m_pa_skull_02_21_tr: 254a726258718e40a81b3cc797ab88ba; selected map 33ca7273aa7af49780a5e23e2101a3414961d2117e9c88644fb4b04118db4c09.
float4 ArtistNative5261(ARTIST_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].wwww,g_ArtistSourceMaterialParameters[6u].xxxx,1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,g_ArtistSourceMaterialParameters[3u].xxxx,1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[2u].zzzz*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[2u].zzzz*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[2u].zzzz*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[2u].zzzz*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[10].x = (((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[11].x = (((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0)))).x;
    source[12].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[12].w = ((g_ArtistSourceMaterialParameters[2u].zzzz*float4(3.1400001, 0.0, 0.0, 0.0))).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[14].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[14].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[16].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].xxxx))).x;
    source[16].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[16].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[16].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[17].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 1: mad r0.xy, v2.xyxx, cb0[10].yzyy, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[10].yzyy)+(source[2].xyxx)).xy;
    // 2: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, v2.xxyx, cb0[11].yyzy, cb0[4].xxyx
    r0.yz = ((v2.xxyx)*(source[11].yyzy)+(source[4].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, cb0[12].y
    r0.x = ((r0.xxxx)*(source[12].yyyy)).x;
    // 8: mad r0.yz, v2.xxyx, cb0[13].xxyx, cb0[5].xxyx
    r0.yz = ((v2.xxyx)*(source[13].xxyx)+(source[5].xxyx)).yz;
    // 9: mad r1.xy, cb0[14].xxxx, r0.xxxx, r0.yzyy
    r1.xy = ((source[14].xxxx)*(r0.xxxx)+(r0.yzyy)).xy;
    // 10: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 11: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: dp2 r2.x, cb0[7].xyxx, r1.xyxx
    r2.x = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 13: dp2 r2.y, cb0[8].xyxx, r1.xyxx
    r2.y = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 14: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: mul r1.x, r0.x, cb0[14].x
    r1.x = ((r0.xxxx)*(source[14].xxxx)).x;
    // 17: mad r0.yz, r1.xxxx, cb0[14].wwww, r0.yyzy
    r0.yz = ((r1.xxxx)*(source[14].wwww)+(r0.yyzy)).yz;
    // 18: add r0.yz, r0.yyzy, cb0[6].xxyx
    r0.yz = ((r0.yyzy)+(source[6].xxyx)).yz;
    // 19: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 20: dp2 r1.x, cb0[7].xyxx, r0.yzyy
    r1.x = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 21: dp2 r1.y, cb0[8].xyxx, r0.yzyy
    r1.y = (dot((source[8].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 22: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 24: mul r0.z, r0.y, r0.w
    r0.z = ((r0.yyyy)*(r0.wwww)).z;
    // 25: mad r0.w, r0.w, r0.y, r0.w
    r0.w = ((r0.wwww)*(r0.yyyy)+(r0.wwww)).w;
    // 26: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 27: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 28: mul r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 29: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 30: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 31: mul r0.z, r0.z, r0.x
    r0.z = ((r0.zzzz)*(r0.xxxx)).z;
    // 32: mul r0.x, r0.x, l(0.300000)
    r0.x = ((r0.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))).x;
    // 33: mad r1.xy, v2.xyxx, l(1.000000, 0.820000, 0.000000, 0.000000), r0.xxxx
    r1.xy = ((v2.xyxx)*(float4(1.000000,0.820000,0.000000,0.000000))+(r0.xxxx)).xy;
    // 34: add r1.xy, r1.xyxx, cb0[9].xyxx
    r1.xy = ((r1.xyxx)+(source[9].xyxx)).xy;
    // 35: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 36: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 37: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 38: mad r0.x, -r0.x, cb0[16].x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[16].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mul_sat r0.x, r0.x, cb0[17].x
    r0.x = (saturate((r0.xxxx)*(source[17].xxxx))).x;
    // 40: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 41: mul_sat r0.x, r0.x, cb0[17].y
    r0.x = (saturate((r0.xxxx)*(source[17].yyyy))).x;
    // 42: mad r0.x, r0.y, cb0[15].w, r0.x
    r0.x = ((r0.yyyy)*(source[15].wwww)+(r0.xxxx)).x;
    // 43: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 44: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 45: log r0.x, |r0.z|
    r0.x = (log2(abs(r0.zzzz))).x;
    // 46: lt r0.y, |r0.z|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 47: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 48: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 49: mul r0.x, r0.x, cb0[15].y
    r0.x = ((r0.xxxx)*(source[15].yyyy)).x;
    // 50: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 51: mad r0.x, r0.w, cb0[15].z, r0.x
    r0.x = ((r0.wwww)*(source[15].zzzz)+(r0.xxxx)).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_b_de_dc_06_1_tr: 26fddc3c63c71642b5dbb20d55a3e61c; selected map e2f9554c42af10e46b0bcdf3b89d232da11b6225ac76e8f70d417cd00eccfc3f.
float4 ArtistNative5262(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4] = g_ArtistSourceMaterialParameters[1u];
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    float4 r0=0.f, r1=0.f;
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
    // 11: add r0.x, -|v4.w|, cb0[0].y
    r0.x = ((-(abs(v4.wwww)))+(source[0].yyyy)).x;
    // 12: mul r0.x, r0.x, l(5.000000)
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 13: div_sat r0.x, r0.x, cb0[0].y
    r0.x = (saturate((r0.xxxx)/(source[0].yyyy))).x;
    // 14: mul r0.x, r0.x, v4.z
    r0.x = ((r0.xxxx)*(v4.zzzz)).x;
    // 15: add r0.y, cb0[5].x, l(-1.000000)
    r0.y = ((source[5].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 16: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 17: mad r0.yz, cb0[5].xxxx, v4.xxyx, -r0.yyyy
    r0.yz = ((source[5].xxxx)*(v4.xxyx)+(-(r0.yyyy))).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 19: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 20: mul r0.y, r0.y, cb0[5].z
    r0.y = ((r0.yyyy)*(source[5].zzzz)).y;
    // 21: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 22: mul r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)*(source[5].wwww)).y;
    // 23: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 24: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 25: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 26: lt r0.y, |r1.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 27: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 28: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 29: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 30: mad r0.xyz, cb0[5].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[5].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 32: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 33: mad r0.xyz, r1.xyzx, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(source[3].xyzx)).xyz;
    // 34: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5262Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
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
    // 6: div r1.xy, v5.xyxx, v5.wwww
    r1.xy = ((v5.xyxx)/(v5.wwww)).xy;
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
    // 19: ge r0.x, v5.w, r0.x
    r0.x = (asfloat((uint4)((v5.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
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
// fx_c_pa_ring_03_ad: d63ba436efa9024f8bf9d60e1f2b3916; selected map 04d2f970e313acccf98355cbddace55c579d9114ce14bc9409c9cff82569e6ff.
float4 ArtistNative5263(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_01_01_dt_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 ArtistNative5264(ARTIST_NATIVE_INPUT input)
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
// fx_k_me_watertrail_01_08_tr: 70bf2a6e9bf4f0478cecbfc43c4e160f; selected map a92c76ce525a64b0cbad0cc8239cd562cacc27a9be9f0bef12bede7feb4e6f40.
float4 ArtistNative5265(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[7].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[11].y = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[15].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 6: mul r0.y, r0.y, cb0[15].z
    r0.y = ((r0.yyyy)*(source[15].zzzz)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 9: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 10: dp2 r1.x, cb0[4].xyxx, r0.yzyy
    r1.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 11: dp2 r1.y, cb0[5].xyxx, r0.yzyy
    r1.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 12: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 13: mul r0.w, r0.y, cb0[7].w
    r0.w = ((r0.yyyy)*(source[7].wwww)).w;
    // 14: mad r1.x, cb0[7].z, cb0[7].y, r0.w
    r1.x = ((source[7].zzzz)*(source[7].yyyy)+(r0.wwww)).x;
    // 15: mul r0.w, r0.z, cb0[8].x
    r0.w = ((r0.zzzz)*(source[8].xxxx)).w;
    // 16: mad r1.y, cb0[7].z, cb0[9].w, r0.w
    r1.y = ((source[7].zzzz)*(source[9].wwww)+(r0.wwww)).y;
    // 17: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 18: add r0.w, cb0[3].w, cb0[12].x
    r0.w = ((source[3].wwww)+(source[12].xxxx)).w;
    // 19: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 20: mul r1.zw, r0.yyyz, cb0[11].xxxy
    r1.zw = ((r0.yyyz)*(source[11].xxxy)).zw;
    // 21: mul r0.yz, r0.yyzy, cb0[14].xxyx
    r0.yz = ((r0.yyzy)*(source[14].xxyx)).yz;
    // 22: mad r1.z, cb0[7].z, cb0[10].w, r1.z
    r1.z = ((source[7].zzzz)*(source[10].wwww)+(r1.zzzz)).z;
    // 23: mad r1.w, cb0[7].z, cb0[11].z, r1.w
    r1.w = ((source[7].zzzz)*(source[11].zzzz)+(r1.wwww)).w;
    // 24: mad r2.y, cb0[3].y, cb0[11].w, r1.w
    r2.y = ((source[3].yyyy)*(source[11].wwww)+(r1.wwww)).y;
    // 25: mad r2.x, cb0[3].y, cb0[10].z, r1.z
    r2.x = ((source[3].yyyy)*(source[10].zzzz)+(r1.zzzz)).x;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r2.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 27: mad r1.xy, r0.wwww, r1.zwzz, r1.xyxx
    r1.xy = ((r0.wwww)*(r1.zwzz)+(r1.xyxx)).xy;
    // 28: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 29: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 30: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 33: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 34: mul r3.xyzw, r1.xxyz, cb0[1].wxyz
    r3.xyzw = ((r1.xxyz)*(source[1].wxyz)).xyzw;
    // 35: mad r0.y, cb0[7].z, cb0[13].w, r0.y
    r0.y = ((source[7].zzzz)*(source[13].wwww)+(r0.yyyy)).y;
    // 36: mad r0.z, cb0[7].z, cb0[14].z, r0.z
    r0.z = ((source[7].zzzz)*(source[14].zzzz)+(r0.zzzz)).z;
    // 37: mad r4.y, cb0[3].y, cb0[14].w, r0.z
    r4.y = ((source[3].yyyy)*(source[14].wwww)+(r0.zzzz)).y;
    // 38: mad r4.x, cb0[3].y, cb0[13].z, r0.y
    r4.x = ((source[3].yyyy)*(source[13].zzzz)+(r0.yyyy)).x;
    // 39: add r0.yz, r4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 40: dp2 r4.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.yzyy
    r4.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 41: dp2 r4.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.yzyy
    r4.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).y;
    // 42: add r0.yz, r4.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r4.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 44: add r0.y, -r1.w, r0.y
    r0.y = ((-(r1.wwww))+(r0.yyyy)).y;
    // 45: mul_sat r0.y, r0.y, cb0[15].x
    r0.y = (saturate((r0.yyyy)*(source[15].xxxx))).y;
    // 46: mul r0.y, r0.y, r3.x
    r0.y = ((r0.yyyy)*(r3.xxxx)).y;
    // 47: add r0.zw, -v4.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(v4.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 48: mul r0.zw, r0.zzzw, v4.xxxy
    r0.zw = ((r0.zzzw)*(v4.xxxy)).zw;
    // 49: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 50: mul_sat r0.z, r0.z, cb0[15].y
    r0.z = (saturate((r0.zzzz)*(source[15].yyyy))).z;
    // 51: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 52: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 53: mul_sat r0.x, r0.x, cb0[15].w
    r0.x = (saturate((r0.xxxx)*(source[15].wwww))).x;
    // 54: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 55: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 56: mad r0.xyz, -cb0[1].xyzx, r1.xyzx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 57: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 58: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 59: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 60: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 61: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 62: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 63: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 64: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 65: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5265Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_m_pa_worldoffset_02_15_tr: a0f7f5723e826143b3970e44f6a045f2; selected map 556a53205748259b14772cac15cc51ca4b99fc29a3e8ad2940bdffdafc2ff11e.
float4 ArtistNative5266(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5266Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_flowmask_01_26_tr: a7f2f31eaeb6e240b8de40abdd7e5bae; selected map 5ef6b688a34a724e45145f48c375dddb8ad2172fe22a6672107272840e9e3973.
float4 ArtistNative5267(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[5] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),1u);
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
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
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mad r1.xy, v2.xyxx, cb0[4].xyxx, cb0[5].xyxx
    r1.xy = ((v2.xyxx)*(source[4].xyxx)+(source[5].xyxx)).xy;
    // 3: mov r0.xz, l(0,0,0,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 4: add r2.xyzw, r0.yxxy, r1.xyxy
    r2.xyzw = ((r0.yxxy)+(r1.xyxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.zwzz, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r1.y, -r0.x, r1.x
    r1.y = ((-(r0.xxxx))+(r1.xxxx)).y;
    // 9: add r1.x, -r0.x, r0.y
    r1.x = ((-(r0.xxxx))+(r0.yyyy)).x;
    // 10: mul r1.xy, r1.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 11: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 12: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 13: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 14: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 15: div r1.xyzw, r1.xyxy, r0.xxxx
    r1.xyzw = ((r1.xyxy)/(r0.xxxx)).xyzw;
    // 16: mad r1.xyzw, r1.xyzw, l(0.500000, 0.500000, 0.500000, 0.500000), l(0.500000, 0.500000, 0.500000, 0.500000)
    r1.xyzw = ((r1.xyzw)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).xyzw;
    // 17: mul r1.xyzw, r1.xyzw, v4.xxxx
    r1.xyzw = ((r1.xyzw)*(v4.xxxx)).xyzw;
    // 18: max r0.x, v4.z, l(0.000010)
    r0.x = (max(v4.zzzz,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 19: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 20: add r2.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 21: dp2 r0.y, r2.xyxx, r2.xyxx
    r0.y = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 22: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 23: mad r0.x, -r0.y, r0.x, l(1.000000)
    r0.x = ((-(r0.yyyy))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 24: add_sat r0.x, r0.x, r0.x
    r0.x = (saturate((r0.xxxx)+(r0.xxxx))).x;
    // 25: mul r1.xyzw, r1.xyzw, r0.xxxx
    r1.xyzw = ((r1.xyzw)*(r0.xxxx)).xyzw;
    // 26: mul r1.xy, r1.xyxx, l(0.200000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.200000,1.000000,0.000000,0.000000))).xy;
    // 27: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.100000, 0.500000), v2.xxxy
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.100000,0.500000))+(v2.xxxy)).zw;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t1.yzxw, s3, l(0.000000)
    r1.z = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 29: mad r2.xy, v2.xyxx, cb0[3].xyxx, r1.xyxx
    r2.xy = ((v2.xyxx)*(source[3].xyxx)+(r1.xyxx)).xy;
    // 30: mov r0.w, v4.w
    r0.w = (v4.wwww).w;
    // 31: add r0.zw, r0.zzzw, r2.xxxy
    r0.zw = ((r0.zzzw)+(r2.xxxy)).zw;
    // 32: add r0.zw, r0.zzzw, cb0[6].xxxy
    r0.zw = ((r0.zzzw)+(source[6].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mad r0.zw, v2.xxxy, cb0[7].xxxy, r1.xxxy
    r0.zw = ((v2.xxxy)*(source[7].xxxy)+(r1.xxxy)).zw;
    // 35: add r0.zw, r0.zzzw, cb0[8].xxxy
    r0.zw = ((r0.zzzw)+(source[8].xxxy)).zw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t4.xyzw, s2, l(0.000000)
    r3.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 37: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 38: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 39: mad r2.xyz, -r2.xyzx, r3.xyzx, r0.zzzz
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r0.zzzz)).xyz;
    // 40: mad r2.xyz, cb0[13].xxxx, r2.xyzx, r4.xyzx
    r2.xyz = ((source[13].xxxx)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 41: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 42: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 43: mul r2.xyz, r2.xyzx, cb0[13].yyyy
    r2.xyz = ((r2.xyzx)*(source[13].yyyy)).xyz;
    // 44: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 45: mad r2.xyz, cb0[13].zzzz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((source[13].zzzz)*(r2.xyzx)+(source[2].xyzx)).xyz;
    // 46: mul r0.xzw, r0.xxxx, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)).xzw;
    // 47: mad r0.xzw, v3.xxyz, r0.xxzw, cb0[1].xxyz
    r0.xzw = ((v3.xxyz)*(r0.xxzw)+(source[1].xxyz)).xzw;
    // 48: mad o0.xyz, r0.xzwx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xzwx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 49: add r0.x, v4.z, l(0.100000)
    r0.x = ((v4.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 50: max r0.x, r0.x, l(0.000010)
    r0.x = (max(r0.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 51: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 52: mad r0.x, -r0.y, r0.x, l(1.000000)
    r0.x = ((-(r0.yyyy))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: add_sat r0.x, r0.x, r0.x
    r0.x = (saturate((r0.xxxx)+(r0.xxxx))).x;
    // 54: mul r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)*(r0.xxxx)).x;
    // 55: mad r0.yz, v2.xxyx, cb0[9].xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)*(source[9].xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 56: dp2 r0.w, cb0[11].xyxx, r0.yzyy
    r0.w = (dot((source[11].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 57: dp2 r2.x, cb0[10].xyxx, r0.yzyy
    r2.x = (dot((source[10].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 58: add r2.y, r0.w, v4.y
    r2.y = ((r0.wwww)+(v4.yyyy)).y;
    // 59: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 60: mad r0.yz, cb0[14].zzzz, r1.xxyx, r0.yyzy
    r0.yz = ((source[14].zzzz)*(r1.xxyx)+(r0.yyzy)).yz;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 62: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 63: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 64: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 65: mul r0.x, r0.x, cb0[14].w
    r0.x = ((r0.xxxx)*(source[14].wwww)).x;
    // 66: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 67: mul_sat r0.x, r0.x, cb0[15].x
    r0.x = (saturate((r0.xxxx)*(source[15].xxxx))).x;
    // 68: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 69: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5267Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_i_pa_backglow_cl_02_tr: 29e0d672dc69374e9b04f75f48ac3e5b; selected map b9047631b19c73caa8be78b0b3b2b300018bb731ac9e3d7f0a46ac44cf3e6724.
float4 ArtistNative5268(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_makeflow_02_09_tr: 6681761729fba24896145fb070705f70; selected map fdfa3146587d6e058328ddecec678833d5f6bda76940ba9f8bd2a35254a53ed4.
float4 ArtistNative5269(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5269Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_makeflow_02_02_tr: 6681761729fba24896145fb070705f70; selected map fdfa3146587d6e058328ddecec678833d5f6bda76940ba9f8bd2a35254a53ed4.
float4 ArtistNative5270(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative5270Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_d_pa_circ_02_03_tr: e56fbbbe7ddbeb4f9d88e9589c4c9041; selected map 602087ba9fcece50e885c83b9adc7d7ea5864015ebc31076cf9dc34244afc0f4.
float4 ArtistNative5271(ARTIST_NATIVE_INPUT input)
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
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 3: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 4: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 5: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 6: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 8: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 11: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 12: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 13: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_smokes_1_tr: a83e5c536b16af489329afa9f08e8de4; selected map 9ab44cf37664f698acb3a1e6470e7836b159cd3c89dd93e97df3816c02760646.
float4 ArtistNative5272(ARTIST_NATIVE_INPUT input)
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
    // 10: mul_sat r0.x, r0.x, l(0.020000)
    r0.x = (saturate((r0.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000)))).x;
    // 11: mov r1.x, l(0)
    r1.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 12: mov r1.y, v4.x
    r1.y = (v4.xxxx).y;
    // 13: add r0.yz, r1.xxyx, v2.xxyx
    r0.yz = ((r1.xxyx)+(v2.xxyx)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.xwyz, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 15: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 16: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 17: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 18: mul r0.xy, v2.xyxx, l(6.000000, 6.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(6.000000,6.000000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: mul r0.xyz, r0.xxxx, v3.xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
#endif
