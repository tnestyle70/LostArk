// Original Kouku material programs 3840..3903; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_ring_11_tr: caba7a7e86cd69479d29dd2f256fbf4a; selected map 33109ed0b5092938858fe35bc6d94adddb4d4375fe840de8d94dc802d7edb2d0.
float4 ArtistNative3840(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 23: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 24: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 25: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3840Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_d_pa_smoke_ulit_01_02_tr: 2997711161642949af1acc17189ac71e; selected map eb33aacae6bff05e44ce71eedda760c1048a4f5a47b1e524a724f3fb3eb57fe8.
float4 ArtistNative3841(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[0u].wwww*float4(0.0, 0.0, 0.0, 0.0))).x;
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
    // 1: mov r0.y, l(0)
    r0.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 2: mul r0.x, v4.x, l(0.200000)
    r0.x = ((v4.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 3: mad r0.xyzw, cb0[2].yyzz, v2.xyxy, r0.yxxy
    r0.xyzw = ((source[2].yyzz)*(v2.xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_01_07_dt_tr: f4c884edfebbfc468ebc706159d8d1c6; selected map 57990282a79c1e89f275517d3329a9cb7c191c7ed7f3810d5c613b1310735a18.
float4 ArtistNative3842(ARTIST_NATIVE_INPUT input)
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
    // 27: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 28: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 29: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_rainfloor_01_ad: 655be5efa3fd4f429c131d654acc0994; selected map 29f1fe1505eacfc7cd3104090556f73e169d3a7b31ba8eafb11758263c6e4679.
float4 ArtistNative3843(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0250000004, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0250000004, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0309999995, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0309999995, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.023, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.023, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.050999999, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.050999999, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.129999995, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v2.xyxx, l(7.000000, 7.000000, 0.000000, 0.000000), cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(float4(7.000000,7.000000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v2.xxyx, l(0.000000, 13.000000, 13.000000, 0.000000), cb0[2].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,13.000000,13.000000,0.000000))+(source[2].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 6: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 7.000000, 7.000000), r0.yyyy
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,7.000000,7.000000))+(r0.yyyy)).zw;
    // 7: add r1.xy, r0.yyyy, cb0[5].xyxx
    r1.xy = ((r0.yyyy)+(source[5].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xy = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 9: add r0.xy, r0.xxxx, r0.zwzz
    r0.xy = ((r0.xxxx)+(r0.zwzz)).xy;
    // 10: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 12: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: mad r0.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 14: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 15: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 16: mad r0.xy, v6.xyxx, r0.zzzz, r0.xyxx
    r0.xy = ((v6.xyxx)*(r0.zzzz)+(r0.xyxx)).xy;
    // 17: mul r0.zw, r0.zzzz, v6.xxxy
    r0.zw = ((r0.zzzz)*(v6.xxxy)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 19: mad r1.xyz, r0.zzzz, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r0.zzzz)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 20: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 21: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 22: add r0.xy, r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 24: mul r0.y, |r0.x|, |r0.x|
    r0.y = ((abs(r0.xxxx))*(abs(r0.xxxx))).y;
    // 25: mul r0.y, r0.y, |r0.x|
    r0.y = ((r0.yyyy)*(abs(r0.xxxx))).y;
    // 26: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 27: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t4.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample5((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 30: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 31: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 32: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 33: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3843Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0250000004, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0250000004, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0309999995, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0309999995, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.023, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.023, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.050999999, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.050999999, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.129999995, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v1.xyxx, l(7.000000, 7.000000, 0.000000, 0.000000), cb0[1].xyxx
    r0.xy = ((v1.xyxx)*(float4(7.000000,7.000000,0.000000,0.000000))+(source[1].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v1.xxyx, l(0.000000, 13.000000, 13.000000, 0.000000), cb0[0].xxyx
    r0.yz = ((v1.xxyx)*(float4(0.000000,13.000000,13.000000,0.000000))+(source[0].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 6: mad r0.zw, v1.xxxy, l(0.000000, 0.000000, 7.000000, 7.000000), r0.yyyy
    r0.zw = ((v1.xxxy)*(float4(0.000000,0.000000,7.000000,7.000000))+(r0.yyyy)).zw;
    // 7: add r1.xy, r0.yyyy, cb0[3].xyxx
    r1.xy = ((r0.yyyy)+(source[3].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xy = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 9: add r0.xy, r0.xxxx, r0.zwzz
    r0.xy = ((r0.xxxx)+(r0.zwzz)).xy;
    // 10: add r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)+(source[2].xyxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 12: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: mad r0.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 14: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 15: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 16: mad r0.xy, v5.xyxx, r0.zzzz, r0.xyxx
    r0.xy = ((v5.xyxx)*(r0.zzzz)+(r0.xyxx)).xy;
    // 17: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 18: add r0.xy, r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v1.xyxx, t4.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample5((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 21: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 22: mul_sat r0.x, r0.x, v2.w
    r0.x = (saturate((r0.xxxx)*(v2.wwww))).x;
    // 23: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
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
// bfx_d_pa_master_01_02_tr: 8f271aa4fe1f634bac6f728bea42e1ff; selected map a233b818ee6e980160ebe411416d626eb083d2e2a90f6536c3f15dc1410bcddd.
float4 ArtistNative3844(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].zzzz,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[4] = g_ArtistSourceMaterialParameters[5u];
    source[5].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[8].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
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
    // 2: mad r0.y, cb0[5].w, cb0[8].y, cb0[8].z
    r0.y = ((source[5].wwww)*(source[8].yyyy)+(source[8].zzzz)).y;
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: mul r0.yz, v2.xxyx, cb0[7].yyzy
    r0.yz = ((v2.xxyx)*(source[7].yyzy)).yz;
    // 15: mul r0.w, cb0[5].z, cb0[5].w
    r0.w = ((source[5].zzzz)*(source[5].wwww)).w;
    // 16: mad r0.yz, r0.wwww, cb0[7].xxwx, r0.yyzy
    r0.yz = ((r0.wwww)*(source[7].xxwx)+(r0.yyzy)).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 18: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 19: mul r0.yz, v2.xxyx, cb0[6].yyzy
    r0.yz = ((v2.xxyx)*(source[6].yyzy)).yz;
    // 20: mad r0.yz, r0.wwww, cb0[6].xxwx, r0.yyzy
    r0.yz = ((r0.wwww)*(source[6].xxwx)+(r0.yyzy)).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 22: mad r0.z, r0.y, r0.x, l(1.000000)
    r0.z = ((r0.yyyy)*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 23: mul r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)*(source[9].yyyy)).z;
    // 24: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r1.xy, r0.wwww, v6.xyxx
    r1.xy = ((r0.wwww)*(v6.xyxx)).xy;
    // 27: mad r0.zw, r1.xxxy, cb0[2].xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)*(source[2].xxxy)+(r0.zzzz)).zw;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r1.xyz = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 29: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 30: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 31: mad r1.xyz, cb0[9].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[9].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 32: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 33: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 34: mul r1.xyz, r1.xyzx, cb0[9].wwww
    r1.xyz = ((r1.xyzx)*(source[9].wwww)).xyz;
    // 35: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 36: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 37: mad r1.xyz, r1.xyzx, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 38: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 39: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 40: add r0.z, -v4.x, l(1.000000)
    r0.z = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 41: mad r0.x, r0.y, r0.x, -r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 42: mul_sat r0.x, r0.x, cb0[10].x
    r0.x = (saturate((r0.xxxx)*(source[10].xxxx))).x;
    // 43: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 44: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 45: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 46: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 47: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 48: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 49: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3844Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_m_a_lightningglow_01_tr: dc748fe07b3d9a49b82570643151e4fc; selected map ad082747b2523256247428aaa454f37191d80af73b1e44848b49c051333f6a09.
float4 ArtistNative3845(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2].x = ((g_ArtistSourceMaterialTime.xxxx*float4(41.8879013, 0.0, 0.0, 0.0))).x;
    source[2].y = (cos((g_ArtistSourceMaterialTime.xxxx*float4(41.8879013, 0.0, 0.0, 0.0)))).x;
    source[2].z = ((float4(0.5, 0.0, 0.0, 0.0)+(float4(0.5, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(31.4159279, 0.0, 0.0, 0.0)))))).x;
    source[2].w = (((float4(0.5, 0.0, 0.0, 0.0)+(float4(0.5, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(31.4159279, 0.0, 0.0, 0.0)))))+cos((g_ArtistSourceMaterialTime.xxxx*float4(41.8879013, 0.0, 0.0, 0.0))))).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: mad r0.y, cb0[2].w, l(-0.600000), l(1.000000)
    r0.y = ((source[2].wwww)*(float4(-0.600000,-0.600000,-0.600000,-0.600000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 3: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 4: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 5: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 6: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 7: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_c_pa_electric_01_ad: 4223e695ae9337459ccbe89184fcb94d; selected map a7f7751fbff9e8f247cde21ec3d40ac20060842bf4c2e88e030139f0b64670dc.
float4 ArtistNative3846(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(6.2831831, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(6.2831831, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(6.2831831, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(6.2831831, 0.0, 0.0, 0.0))),1u);
    source[4].x = (cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(6.2831831, 0.0, 0.0, 0.0)))).x;
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
    // 1: add r0.xy, v2.yyyy, -v4.xzxx
    r0.xy = ((v2.yyyy)+(-(v4.xzxx))).xy;
    // 2: mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // 3: mov_sat r0.y, r0.y
    r0.y = (saturate(r0.yyyy)).y;
    // 4: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 5: mul r0.x, r0.x, l(5.000000)
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 6: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 7: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 8: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 9: dp2 r1.x, cb0[2].xyxx, r0.yzyy
    r1.x = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 10: dp2 r1.y, cb0[3].xyxx, r0.yzyy
    r1.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 11: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 13: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 14: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 17: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 18: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 19: lt r1.x, |r0.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 20: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 21: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 22: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 23: mad r0.yzw, cb0[4].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 24: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 25: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 26: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 27: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_01_03_dt_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 ArtistNative3847(ARTIST_NATIVE_INPUT input)
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
// bfx_i_pa_thunder_01_ad: 4e0871ffb9d96b4bafc08ddbe2d34b7a; selected map 340f1fa031b2bebec195137e0c3c46c757729dabc48018a1cd3b0a67d28ddafd.
float4 ArtistNative3848(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-1.0, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
    source[4].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))).x;
    source[5].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0)))).x;
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
    // 1: log r0.x, |v2.y|
    r0.x = (log2(abs(v2.yyyy))).x;
    // 2: mul r0.x, r0.x, l(1.600000)
    r0.x = ((r0.xxxx)*(float4(1.600000,1.600000,1.600000,1.600000))).x;
    // 3: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 4: lt r0.y, |v2.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(v2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 5: movc r1.y, r0.y, l(0), r0.x
    r1.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).y;
    // 6: mad r0.xz, v2.xxyx, l(1.000000, 0.000000, 0.700000, 0.000000), cb0[2].xxyx
    r0.xz = ((v2.xxyx)*(float4(1.000000,0.000000,0.700000,0.000000))+(source[2].xxyx)).xz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t2.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 9: mul r0.x, r0.x, v4.x
    r0.x = ((r0.xxxx)*(v4.xxxx)).x;
    // 10: mul r0.z, |v2.y|, |v2.y|
    r0.z = ((abs(v2.yyyy))*(abs(v2.yyyy))).z;
    // 11: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 12: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 13: mov r1.x, v2.x
    r1.x = (v2.xxxx).x;
    // 14: mad r0.xz, r0.xxxx, l(0.100000, 0.000000, 0.100000, 0.000000), r1.xxyx
    r0.xz = ((r0.xxxx)*(float4(0.100000,0.000000,0.100000,0.000000))+(r1.xxyx)).xz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t3.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 16: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 17: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 18: mul r0.z, r0.z, l(1.200000)
    r0.z = ((r0.zzzz)*(float4(1.200000,1.200000,1.200000,1.200000))).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 21: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 22: mul r0.yz, v2.xxyx, cb0[4].yyzy
    r0.yz = ((v2.xxyx)*(source[4].yyzy)).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s3, l(0.000000)
    r0.yzw = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 24: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 25: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 26: mul r0.yzw, r0.yyzw, cb0[4].wwww
    r0.yzw = ((r0.yyzw)*(source[4].wwww)).yzw;
    // 27: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 28: mul r1.xyz, r0.yzwy, r0.xxxx
    r1.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 29: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: mad r0.xyz, -r0.xxxx, r0.yzwy, r1.wwww
    r0.xyz = ((-(r0.xxxx))*(r0.yzwy)+(r1.wwww)).xyz;
    // 31: mad r0.xyz, cb0[5].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[5].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 32: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 33: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 34: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 35: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 36: source device depth mapped to centimetre view depth; reconstruction at 38.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 38-41: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 42: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 43: mul_sat r0.w, r0.w, l(0.010000)
    r0.w = (saturate((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000)))).w;
    // 44: add r1.xy, v2.xyxx, cb0[3].xyxx
    r1.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 46: add_sat r1.x, r1.x, v4.y
    r1.x = (saturate((r1.xxxx)+(v4.yyyy))).x;
    // 47: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 48: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 49: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 50: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 51: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_dbalphainvert_01_ad: 1580550d49b90542b5d48cfc52b103f4; selected map 9cbd14a54cdcb1c1451fea808fb1d2537b3f8adb62859c329f4f38534d307344.
float4 ArtistNative3849(ARTIST_NATIVE_INPUT input)
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
    // 10: mul_sat r0.x, r0.x, l(0.111111)
    r0.x = (saturate((r0.xxxx)*(float4(0.111111,0.111111,0.111111,0.111111)))).x;
    // 11: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 13: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 14: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 15: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: mad r0.w, -r0.w, l(2.000000), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: add_sat r0.w, r0.w, r0.w
    r0.w = (saturate((r0.wwww)+(r0.wwww))).w;
    // 19: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 20: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 21: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 22: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ri_08_1_tr: 991ecc5522859c49bb034318a827af71; selected map fd85ade76eaa888dbc097dfdfc3900cacd64ae68812cae449a674d712986917f.
float4 ArtistNative3850(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3850Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_e_pa_ri_02_1_ma: faf2641b19ab8a47bf5b66368e857d40; selected map 9f241bb47ec96e9db02729a413cc4a1141dce916523e0e8c59358b657297eb0d.
float4 ArtistNative3851(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[3u];
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mad r0.x, cb0[2].x, v4.w, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.w, cb0[2].x
    r0.y = ((v4.wwww)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul r1.xyzw, r0.wxyz, cb0[2].wyyy
    r1.xyzw = ((r0.wxyz)*(source[2].wyyy)).xyzw;
    // 7: log r0.w, |r1.x|
    r0.w = (log2(abs(r1.xxxx))).w;
    // 8: mul r0.w, r0.w, cb0[3].x
    r0.w = ((r0.wwww)*(source[3].xxxx)).w;
    // 9: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 10: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 12: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 13: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 18: mul r2.x, r1.x, cb0[3].y
    r2.x = ((r1.xxxx)*(source[3].yyyy)).x;
    // 19: mov_sat r1.x, r1.x
    r1.x = (saturate(r1.xxxx)).x;
    // 20: mad r2.y, v4.x, l(2.000000), l(-1.000000)
    r2.y = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 21: ge r2.x, r2.y, r2.x
    r2.x = (asfloat((uint4)((r2.yyyy)>=(r2.xxxx)) * 0xffffffffu)).x;
    // 22: ge r1.x, r2.y, r1.x
    r1.x = (asfloat((uint4)((r2.yyyy)>=(r1.xxxx)) * 0xffffffffu)).x;
    // 23: movc r1.x, r1.x, l(-1.000000), l(-0.000000)
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).x;
    // 24: and r2.x, r2.x, l(0x3f800000)
    r2.x = (asfloat(asuint(r2.xxxx) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).x;
    // 25: mul r2.y, r0.w, r2.x
    r2.y = ((r0.wwww)*(r2.xxxx)).y;
    // 26: add r1.x, r1.x, r2.x
    r1.x = ((r1.xxxx)+(r2.xxxx)).x;
    // 27: mul_sat r2.x, r2.y, v3.w
    r2.x = (saturate((r2.yyyy)*(v3.wwww))).x;
    // 28: add r2.x, r2.x, l(-0.333300)
    r2.x = ((r2.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 29: lt r2.x, r2.x, l(0.000000)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 30: discard_nz r2.x
    if ((asuint(r2.xxxx)).x != 0u) clip(-1.f);
    // 31: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 32: dp3 r2.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 33: mad r0.xyz, -r0.xyzx, cb0[2].yyyy, r2.xxxx
    r0.xyz = ((-(r0.xyzx))*(source[2].yyyy)+(r2.xxxx)).xyz;
    // 34: mad r0.xyz, cb0[2].zzzz, r0.xyzx, r1.yzwy
    r0.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 35: mul r1.yzw, cb0[1].xxyz, cb0[1].wwww
    r1.yzw = ((source[1].xxyz)*(source[1].wwww)).yzw;
    // 36: mul r1.xyz, r1.yzwy, r1.xxxx
    r1.xyz = ((r1.yzwy)*(r1.xxxx)).xyz;
    // 37: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 38: mad r0.xyz, r0.xyzx, v3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(r1.xyzx)).xyz;
    // 39: add o0.xyz, r0.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 40: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_de_master_01_40_tr: 31e28c4451b3b942b46cdca51e41ed25; selected map 65239cdeec1584e46f9092ff2642d6b664995e5d6ae61036c456c38f8c1208db.
float4 ArtistNative3852(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_05_04_tr: 1166c98a352b934594aa881b9923e0f3; selected map c2d8d01196bc1ad3c52eed22b3401991f762707ca1c0e1467fc60f3a974cf0b1.
float4 ArtistNative3853(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_turbulence_01_10_tr: f2ecc1c1e40aa34d879ceeabe82dfc21; selected map c345cefde3ac6ecbf5923057310970d247e2eee9b09817d3af90d16e36f482f6.
float4 ArtistNative3854(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r1.xyzw, -r0.xyxy, v2.xyxy
    r1.xyzw = ((-(r0.xyxy))+(v2.xyxy)).xyzw;
    // 3: mad r0.xyzw, v4.xxyy, r1.xyzw, r0.xyxy
    r0.xyzw = ((v4.xxyy)*(r1.xyzw)+(r0.xyxy)).xyzw;
    // 4: mul r1.xy, r0.xyxx, cb0[5].yyyy
    r1.xy = ((r0.xyxx)*(source[5].yyyy)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: mul_sat r1.x, r1.x, cb0[5].z
    r1.x = (saturate((r1.xxxx)*(source[5].zzzz))).x;
    // 7: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 8: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 9: mul r1.y, r1.y, cb0[5].w
    r1.y = ((r1.yyyy)*(source[5].wwww)).y;
    // 10: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 11: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.wxyz, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 13: mul r0.yz, r0.zzwz, cb0[3].zzwz
    r0.yz = ((r0.zzwz)*(source[3].zzwz)).yz;
    // 14: mul_sat r0.x, r0.x, cb0[4].w
    r0.x = (saturate((r0.xxxx)*(source[4].wwww))).x;
    // 15: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 16: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mul r0.w, r0.w, cb0[5].x
    r0.w = ((r0.wwww)*(source[5].xxxx)).w;
    // 18: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 19: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 20: mul r0.w, r1.x, r0.x
    r0.w = ((r1.xxxx)*(r0.xxxx)).w;
    // 21: mad r0.x, -r0.x, r1.x, r0.x
    r0.x = ((-(r0.xxxx))*(r1.xxxx)+(r0.xxxx)).x;
    // 22: mad r0.x, v4.z, r0.x, r0.w
    r0.x = ((v4.zzzz)*(r0.xxxx)+(r0.wwww)).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 24: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 25: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 26: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 27: mad r0.x, cb0[3].y, cb0[3].x, r0.y
    r0.x = ((source[3].yyyy)*(source[3].xxxx)+(r0.yyyy)).x;
    // 28: mad r0.y, cb0[3].y, cb0[4].x, r0.z
    r0.y = ((source[3].yyyy)*(source[4].xxxx)+(r0.zzzz)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 31: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 32: mad r0.xyz, cb0[4].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 33: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 34: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 35: mul r0.xyz, r0.xyzx, cb0[4].zzzz
    r0.xyz = ((r0.xyzx)*(source[4].zzzz)).xyz;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_022_ts_ad: eca2a408b89c0043bcd84d43f91d7995; selected map 65536a1afe883aee3420afe913749666570847e1f5f21760abde0ae44227a5eb.
float4 ArtistNative3855(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: add r0.x, cb0[3].y, l(-1.000000)
    r0.x = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[5].y, cb0[7].w, cb0[8].x
    r0.y = ((source[5].yyyy)*(source[7].wwww)+(source[8].xxxx)).y;
    // 3: sincos r1.x, r2.x, r0.y
    r1.x = (sin(r0.yyyy)).x; r2.x = (cos(r0.yyyy)).x;
    // 4: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 5: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 7: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 8: dp2 r0.w, r3.zyzz, r0.yzyy
    r0.w = (dot((r3.zyzz).xy,(r0.yzyy).xy).xxxx).w;
    // 9: dp2 r0.y, r3.yxyy, r0.yzyy
    r0.y = (dot((r3.yxyy).xy,(r0.yzyy).xy).xxxx).y;
    // 10: mad r0.x, r0.y, cb0[4].x, r0.x
    r0.x = ((r0.yyyy)*(source[4].xxxx)+(r0.xxxx)).x;
    // 11: mul r0.z, r0.w, cb0[4].y
    r0.z = ((r0.wwww)*(source[4].yyyy)).z;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.zxyw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 14: mul r0.y, v4.y, cb0[7].x
    r0.y = ((v4.yyyy)*(source[7].xxxx)).y;
    // 15: mul r0.z, cb0[5].x, cb0[5].y
    r0.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 16: mad r1.w, r0.z, cb0[7].y, r0.y
    r1.w = ((r0.zzzz)*(source[7].yyyy)+(r0.yyyy)).w;
    // 17: mul r0.yw, v4.yyyx, cb0[6].xxxw
    r0.yw = ((v4.yyyx)*(source[6].xxxw)).yw;
    // 18: mad r1.yz, r0.zzzz, cb0[6].yyzy, r0.yywy
    r1.yz = ((r0.zzzz)*(source[6].yyzy)+(r0.yywy)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t1.xzyw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: mul r0.y, v4.x, cb0[5].w
    r0.y = ((v4.xxxx)*(source[5].wwww)).y;
    // 22: mad r1.x, r0.z, cb0[5].z, r0.y
    r1.x = ((r0.zzzz)*(source[5].zzzz)+(r0.yyyy)).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xzyw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 24: add r0.z, -cb0[3].x, l(1.000000)
    r0.z = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: mad r0.x, r0.y, r0.x, -r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 26: mul_sat r0.x, r0.x, cb0[8].w
    r0.x = (saturate((r0.xxxx)*(source[8].wwww))).x;
    // 27: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 28: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.y, r0.y, cb0[9].x
    r0.y = ((r0.yyyy)*(source[9].xxxx)).y;
    // 30: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 31: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 32: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 33: source device depth mapped to centimetre view depth; reconstruction at 35.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 35-38: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 39: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 40: add r0.w, -cb0[9].y, l(1.000000)
    r0.w = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 42: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 45: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 46: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 47: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 48: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 49: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 50: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3855Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_atta_05_04_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative3857(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_020_ad: 7c31909c9df918499fdae3c98350c9f0; selected map a026a6ead4a32d8c472ffa6b2ec174803e1b270dc883d1035ffff49472be8cf6.
float4 ArtistNative3858(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
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
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
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
    r0.y = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
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
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3858Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_atta_09_15_dt_ad: 7ac71a915017cd4a9d3d5bf82a144e4c; selected map 8239ba7023c898c554c48f062c5d5d827687061479508db2546a9a9bd1c11f6a.
float4 ArtistNative3859(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
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
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[5].x, l(1.000000)
    r0.y = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t1.xyzw, s2, l(0.000000)
    r1.xyzw = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 23: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 24: mul r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)*(source[4].wwww)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 27: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 28: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 29: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 30: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 32: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 33: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 34: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 35: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 36: mul r0.yzw, r1.xxyz, cb0[3].wwww
    r0.yzw = ((r1.xxyz)*(source[3].wwww)).yzw;
    // 37: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 38: mad r1.xyz, -cb0[3].wwww, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[3].wwww))*(r1.xyzx)+(r1.wwww)).xyz;
    // 39: mad r0.yzw, cb0[4].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 40: mul r1.xyz, r0.yzwy, cb0[4].yyyy
    r1.xyz = ((r0.yzwy)*(source[4].yyyy)).xyz;
    // 41: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 42: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 43: mul r1.xyz, r1.xyzx, cb0[4].zzzz
    r1.xyz = ((r1.xyzx)*(source[4].zzzz)).xyz;
    // 44: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 45: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 46: add r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = ((r0.yyzw)+(r1.xxxx)).yzw;
    // 47: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 48: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 49: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 50: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3859Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_m_pa_shorkwave_01_2_tr: 5c707b8a1ddaf5478de6e37c4b55bf26; selected map f1f290972b5b31eec74b516c45ee383023e1c16a6224be179cff954bdcae7562.
float4 ArtistNative3860(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = (cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_me_depthline_01_tr: 918e1fae3ccec645b990f7d55542bd8d; selected map b1818fbf19f8090ccea5868bb33c2c2d8d172fa5eab8bd44873f98ae41d53c86.
float4 ArtistNative3861(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),1u);
    source[4] = input.dynamicParameter;
    source[5] = g_ArtistSourceMaterialParameters[2u];
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[7].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))).x;
    source[8].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0)))).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.xy, v4.xyxx, l(15.000000, 3.000000, 0.000000, 0.000000), cb0[6].xyxx
    r0.xy = ((v4.xyxx)*(float4(15.000000,3.000000,0.000000,0.000000))+(source[6].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r1.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: add r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)+(r0.xyxx)).xy;
    // 5: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 6: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 8: sqrt r0.z, r0.w
    r0.z = (sqrt(r0.wwww)).z;
    // 9: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: add r0.xyz, r0.xyzx, l(-1.000000, -1.000000, 0.100010, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(-1.000000,-1.000000,0.100010,0.000000))).xyz;
    // 11: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 12: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 13: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 14: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 15: dp3 r0.x, r2.xyzx, r0.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 16: max r0.y, r0.w, l(0.000000)
    r0.y = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 17: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 18: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 19: mul r0.yw, |r0.yyyx|, |r0.yyyx|
    r0.yw = ((abs(r0.yyyx))*(abs(r0.yyyx))).yw;
    // 20: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 21: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 22: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: mul r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)*(source[8].yyyy)).z;
    // 24: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 25: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 26: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 27: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: movc r0.w, r0.x, l(0), r0.w
    r0.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 31: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 32: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 34: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 35: mul r0.x, r0.x, l(4.000000)
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 36: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 37: mul r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)*(source[8].zzzz)).x;
    // 38: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 39: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 40: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 41: source device depth mapped to centimetre view depth; reconstruction at 43.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 43-46: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 47: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 48: add r0.z, -cb0[7].y, l(1.000000)
    r0.z = ((-(source[7].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 49: mul r0.z, r0.z, l(5.000000)
    r0.z = ((r0.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 50: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 51: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 52: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: mad r0.x, cb0[7].z, r0.y, r0.x
    r0.x = ((source[7].zzzz)*(r0.yyyy)+(r0.xxxx)).x;
    // 54: mad r0.yz, v4.xxyx, l(0.000000, 5.000000, 2.000000, 0.000000), cb0[3].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,5.000000,2.000000,0.000000))+(source[3].xxyx)).yz;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 56: add r0.z, cb0[4].x, l(-1.000000)
    r0.z = ((source[4].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 57: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 58: mul_sat r0.yz, r0.yyyy, l(0.000000, 50.000000, 20.000000, 0.000000)
    r0.yz = (saturate((r0.yyyy)*(float4(0.000000,50.000000,20.000000,0.000000)))).yz;
    // 59: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 60: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 61: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 62: add r0.x, -r0.z, r0.y
    r0.x = ((-(r0.zzzz))+(r0.yyyy)).x;
    // 63: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 64: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 65: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: mul r0.y, r0.y, l(1.200000)
    r0.y = ((r0.yyyy)*(float4(1.200000,1.200000,1.200000,1.200000))).y;
    // 67: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 68: mul r1.xyz, r0.yyyy, cb0[5].xyzx
    r1.xyz = ((r0.yyyy)*(source[5].xyzx)).xyz;
    // 69: movc r0.xyw, r0.xxxx, l(0,0,0,0), r1.xyxz
    r0.xyw = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyxz)).xyw;
    // 70: add r0.xyz, r0.xywx, r0.zzzz
    r0.xyz = ((r0.xywx)+(r0.zzzz)).xyz;
    // 71: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_smoke_01_tr: a35ac6bf9e759346b6f4179900726708; selected map 1205a761ceecf4f8c1ac57893825f8d162714d89e7499abeeea1ccf5ca1a5890.
float4 ArtistNative3862(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 28: mul r1.x, r0.y, cb0[3].w
    r1.x = ((r0.yyyy)*(source[3].wwww)).x;
    // 29: mul r2.xy, r0.yyyy, cb0[2].xzxx
    r2.xy = ((r0.yyyy)*(source[2].xzxx)).xy;
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
    // 41: mul r2.zw, r0.yyyy, cb0[2].yyyw
    r2.zw = ((r0.yyyy)*(source[2].yyyw)).zw;
    // 42: mul r1.y, r0.y, cb0[4].x
    r1.y = ((r0.yyyy)*(source[4].xxxx)).y;
    // 43: mad r0.yz, v4.zzzz, l(0.000000, -0.030000, -0.100000, 0.000000), r2.yywy
    r0.yz = ((v4.zzzz)*(float4(0.000000,-0.030000,-0.100000,0.000000))+(r2.yywy)).yz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t0.zxyw, s0, l(0.000000)
    r0.yz = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 45: mad r3.xyzw, r0.yzyz, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.500000, -1.500000)
    r3.xyzw = ((r0.yzyz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.500000,-1.500000))).xyzw;
    // 46: mad r0.yz, r3.xxyx, l(0.000000, 0.080000, 0.080000, 0.000000), r1.xxyx
    r0.yz = ((r3.xxyx)*(float4(0.000000,0.080000,0.080000,0.000000))+(r1.xxyx)).yz;
    // 47: mad r0.yz, v4.zzzz, l(0.000000, -0.050000, -0.100000, 0.000000), r0.yyzy
    r0.yz = ((v4.zzzz)*(float4(0.000000,-0.050000,-0.100000,0.000000))+(r0.yyzy)).yz;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 49: add r0.zw, v4.yyyx, l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((v4.yyyx)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 50: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 51: add_sat r0.y, r0.y, r0.y
    r0.y = (saturate((r0.yyyy)+(r0.yyyy))).y;
    // 52: dp2 r1.x, l(0.000796, -1.000000, 0.000000, 0.000000), r3.zwzz
    r1.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).x;
    // 53: dp2 r1.y, l(1.000000, 0.000796, 0.000000, 0.000000), r3.zwzz
    r1.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).y;
    // 54: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 55: mad r0.zw, r0.zzzz, r1.xxxy, r2.xxxz
    r0.zw = ((r0.zzzz)*(r1.xxxy)+(r2.xxxz)).zw;
    // 56: mov r1.x, l(0)
    r1.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 57: mul r1.y, v4.w, l(0.200000)
    r1.y = ((v4.wwww)*(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 58: add r0.zw, r0.zzzw, r1.xxxy
    r0.zw = ((r0.zzzw)+(r1.xxxy)).zw;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 60: add r0.z, r1.x, r1.x
    r0.z = ((r1.xxxx)+(r1.xxxx)).z;
    // 61: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 62: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 63: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 64: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 65: max r0.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 66: mul r1.xyz, r1.xyzx, cb0[3].zzzz
    r1.xyz = ((r1.xyzx)*(source[3].zzzz)).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[3].xxxx
    r0.xyz = ((r0.xyzx)*(source[3].xxxx)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mad r0.xyz, cb0[3].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[3].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 71: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_noise_01_1_ad: 8a6fb3184af41f4290ecdfca77148f7a; selected map e4a5c36f1d0d9975b905d94569d7bb59d19f0854a20bde0712c55b18da54ac0f.
float4 ArtistNative3863(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[4u];
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[2].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.x, cb0[4].y, v4.x, l(-1.000000)
    r0.x = ((source[4].yyyy)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mad r0.yz, cb0[2].yyyy, cb0[1].xxyx, v2.xxyx
    r0.yz = ((source[2].yyyy)*(source[1].xxyx)+(v2.xxyx)).yz;
    // 4: mul r0.w, v4.x, cb0[4].y
    r0.w = ((v4.xxxx)*(source[4].yyyy)).w;
    // 5: mad r0.xy, r0.wwww, r0.yzyy, -r0.xxxx
    r0.xy = ((r0.wwww)*(r0.yzyy)+(-(r0.xxxx))).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.wxyz, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 7: mul r0.x, r0.x, cb0[4].z
    r0.x = ((r0.xxxx)*(source[4].zzzz)).x;
    // 8: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 9: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: mul r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)*(source[4].wwww)).y;
    // 11: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 12: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 13: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 14: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 16: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 17: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 19: mad r0.x, r0.y, cb0[5].x, r0.x
    r0.x = ((r0.yyyy)*(source[5].xxxx)+(r0.xxxx)).x;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 21: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 22: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 23: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 24: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 25: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 26: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 27: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 28: mul r0.xy, v2.xyxx, cb0[2].zwzz
    r0.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 29: mad r1.x, cb0[2].y, cb0[2].x, r0.x
    r1.x = ((source[2].yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 30: mad r1.y, cb0[2].y, cb0[3].x, r0.y
    r1.y = ((source[2].yyyy)*(source[3].xxxx)+(r0.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t3.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: mul r1.xyz, r0.xyzx, cb0[3].yyyy
    r1.xyz = ((r0.xyzx)*(source[3].yyyy)).xyz;
    // 33: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: mad r0.xyz, -cb0[3].yyyy, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[3].yyyy))*(r0.xyzx)+(r0.wwww)).xyz;
    // 35: mad r0.xyz, cb0[3].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[3].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 36: mad o0.xyz, r0.xyzx, v3.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)*(v3.xyzx)+(source[0].xyzx)).xyz;
    // 37: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_worldoffset_ribbon_01_1_tr: a3c9f4c7ac8f5e4d8ea4e24ffac7abde; selected map bf06532c5ca03e8c527f08eff2e987a981692b293c1f7c2702b72b0607820737.
float4 ArtistNative3866(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0149999997, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0209999997, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0109999999, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].yyyy,1u);
    source[5].x = ((float4(0.00200000009, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[5].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0209999997, 0.0, 0.0, 0.0)))).x;
    source[5].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0149999997, 0.0, 0.0, 0.0)))).x;
    source[5].w = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.0109999999, 0.0, 0.0, 0.0))).x;
    source[6].x = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[6].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[6].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0109999999, 0.0, 0.0, 0.0)))).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.zxyz
    r0.xyz = ((r0.xxxx)*(v1.zxyz)).xyz;
    // 4: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v0.yzxy
    r1.xyz = ((r0.wwww)*(v0.yzxy)).xyz;
    // 7: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 8: mad r2.xyz, r0.zxyz, r1.yzxy, -r2.xyzx
    r2.xyz = ((r0.zxyz)*(r1.yzxy)+(-(r2.xyzx))).xyz;
    // 9: mul r2.xyz, r2.xzyx, v1.wwww
    r2.xyz = ((r2.xzyx)*(v1.wwww)).xyz;
    // 10: add r3.xyz, v7.xyzx, cb0[0].xyzx
    r3.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 11: mul r4.xyz, r2.xxzx, r3.yyyy
    r4.xyz = ((r2.xxzx)*(r3.yyyy)).xyz;
    // 12: mad r1.xzw, r3.xxxx, r1.zzzx, r4.xxyz
    r1.xzw = ((r3.xxxx)*(r1.zzzx)+(r4.xxyz)).xzw;
    // 13: mov r2.x, r1.y
    r2.x = (r1.yyyy).x;
    // 14: mad r0.yzw, r3.zzzz, r0.yyyz, r1.xxzw
    r0.yzw = ((r3.zzzz)*(r0.yyyz)+(r1.xxzw)).yzw;
    // 15: mov r2.z, r0.x
    r2.z = (r0.xxxx).z;
    // 16: mul r1.xy, r0.zwzz, l(0.005000, 0.005000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(0.005000,0.005000,0.000000,0.000000))).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 18: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 19: mul r1.xy, r1.xyxx, l(0.150000, 0.150000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.150000,0.150000,0.000000,0.000000))).xy;
    // 20: mad r0.xz, r0.zzwz, l(0.001000, 0.000000, 0.001000, 0.000000), cb0[3].xxyx
    r0.xz = ((r0.zzwz)*(float4(0.001000,0.000000,0.001000,0.000000))+(source[3].xxyx)).xz;
    // 21: mul r3.x, r0.y, cb0[5].x
    r3.x = ((r0.yyyy)*(source[5].xxxx)).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xzxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mov r3.y, v2.y
    r3.y = (v2.yyyy).y;
    // 24: add r0.zw, r3.xxxy, cb0[2].xxxy
    r0.zw = ((r3.xxxy)+(source[2].xxxy)).zw;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 26: mul r0.xy, r0.xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(r0.zwzz)).xy;
    // 27: mad r0.zw, r0.xxxy, l(0.000000, 0.000000, 0.600000, 0.600000), r1.xxxy
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,0.600000,0.600000))+(r1.xxxy)).zw;
    // 28: mad r0.xy, cb0[6].wwww, r0.xyxx, r3.xyxx
    r0.xy = ((source[6].wwww)*(r0.xyxx)+(r3.xyxx)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: mad r0.xy, v2.xyxx, l(1.500000, 1.500000, 0.000000, 0.000000), r0.zwzz
    r0.xy = ((v2.xyxx)*(float4(1.500000,1.500000,0.000000,0.000000))+(r0.zwzz)).xy;
    // 31: add r0.xy, r0.xyxx, l(-0.200000, -0.750000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.200000,-0.750000,0.000000,0.000000))).xy;
    // 32: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 33: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 34: mad r0.x, -r0.x, l(2.857143), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.857143,2.857143,2.857143,2.857143))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mul r0.x, r0.x, l(0.333333)
    r0.x = ((r0.xxxx)*(float4(0.333333,0.333333,0.333333,0.333333))).x;
    // 36: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 37: mul r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)*(r0.xxxx)).y;
    // 38: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r0.y, r0.y, l(10.000000)
    r0.y = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 40: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 41: mul r1.yzw, r1.xxyz, r0.xxxx
    r1.yzw = ((r1.xxyz)*(r0.xxxx)).yzw;
    // 42: mul r1.yzw, r1.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r1.yzw = ((r1.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 43: max r3.xyz, |r1.yzwy|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r1.yzwy),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 45: mad r1.yzw, r3.xxyz, l(0.000000, 50.000000, 50.000000, 50.000000), r1.yyzw
    r1.yzw = ((r3.xxyz)*(float4(0.000000,50.000000,50.000000,50.000000))+(r1.yyzw)).yzw;
    // 46: mad r0.xy, v2.xyxx, l(0.600000, 1.000000, 0.000000, 0.000000), cb0[4].xyxx
    r0.xy = ((v2.xyxx)*(float4(0.600000,1.000000,0.000000,0.000000))+(source[4].xyxx)).xy;
    // 47: add r0.xy, r0.zwzz, r0.xyxx
    r0.xy = ((r0.zwzz)+(r0.xyxx)).xy;
    // 48: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 49: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 50: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 51: mad r0.x, -r0.x, l(2.857143), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.857143,2.857143,2.857143,2.857143))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy))).x;
    // 53: mul r0.x, r0.x, cb0[7].z
    r0.x = ((r0.xxxx)*(source[7].zzzz)).x;
    // 54: mul r0.y, r0.x, r1.x
    r0.y = ((r0.xxxx)*(r1.xxxx)).y;
    // 55: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 56: mad r0.yzw, r0.yyyy, cb0[7].wwww, r1.yyzw
    r0.yzw = ((r0.yyyy)*(source[7].wwww)+(r1.yyzw)).yzw;
    // 57: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 58: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 59: add r0.y, v4.x, l(-1.000000)
    r0.y = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 60: add r0.y, -r0.y, r1.x
    r0.y = ((-(r0.yyyy))+(r1.xxxx)).y;
    // 61: mul_sat r0.y, r0.y, l(30.000000)
    r0.y = (saturate((r0.yyyy)*(float4(30.000000,30.000000,30.000000,30.000000)))).y;
    // 62: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 63: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_skull_02_tr: 254a726258718e40a81b3cc797ab88ba; selected map 9ef47db7ef89d0508b564298e53ce91fa969c72f073c4dae0297df7763eebbfb.
float4 ArtistNative3867(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_lensflare_02_2_ad_dt: 9b42d2cef8e8af4497883a8e3de48c40; selected map 3f597b201508f81017955f4a88b62f3aa265823203a94532f582ab9d5b4b64bd.
float4 ArtistNative3868(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = ((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))).x;
    source[2].w = (((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[3].x = (sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].w = (((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].x = (sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[4].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[4].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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
// fx_m_pa_shorkwave_01_7_tr: 5c707b8a1ddaf5478de6e37c4b55bf26; selected map f1f290972b5b31eec74b516c45ee383023e1c16a6224be179cff954bdcae7562.
float4 ArtistNative3869(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = (cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_turbpa_09_tr: 142d7eeccb6dec4b8b4233dcf6db51a2; selected map 1f8ae9aed7bae93344c781be239cb610ad315e63de228050a9f4abce3bea8db8.
float4 ArtistNative3870(ARTIST_NATIVE_INPUT input)
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
// fx_j_pa_ring_07_84_ad: 269949c285041d4c8425dc471c06fcd9; selected map e204f2b9b01d8a8b56285c0084801e9b38822c5a167cd4d3a3bc103c8a21ce6c.
float4 ArtistNative3871(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].w = ((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[6].x = (((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].y = (((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[3u].yyyy*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].w = ((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[10].x = (((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[10].y = (((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[11].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 29: mad r0.y, r0.y, l(0.159155), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 30: mad r1.x, cb0[10].z, r0.y, cb0[4].x
    r1.x = ((source[10].zzzz)*(r0.yyyy)+(source[4].xxxx)).x;
    // 31: mad r2.x, cb0[6].z, r0.y, cb0[2].x
    r2.x = ((source[6].zzzz)*(r0.yyyy)+(source[2].xxxx)).x;
    // 32: mul r0.y, v2.x, cb0[7].w
    r0.y = ((v2.xxxx)*(source[7].wwww)).y;
    // 33: mad r3.x, cb0[5].y, cb0[7].z, r0.y
    r3.x = ((source[5].yyyy)*(source[7].zzzz)+(r0.yyyy)).x;
    // 34: mul r0.y, v2.y, cb0[8].x
    r0.y = ((v2.yyyy)*(source[8].xxxx)).y;
    // 35: mad r3.y, cb0[5].y, cb0[8].y, r0.y
    r3.y = ((source[5].yyyy)*(source[8].yyyy)+(r0.yyyy)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r3.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 37: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 38: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 40: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 41: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 42: add r0.z, v4.x, l(-1.000000)
    r0.z = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 43: mad r1.y, cb0[11].y, r0.x, r0.z
    r1.y = ((source[11].yyyy)*(r0.xxxx)+(r0.zzzz)).y;
    // 44: mad r2.y, cb0[7].y, r0.x, r0.z
    r2.y = ((source[7].yyyy)*(r0.xxxx)+(r0.zzzz)).y;
    // 45: mad r0.xz, r0.yyyy, cb0[8].wwww, r2.xxyx
    r0.xz = ((r0.yyyy)*(source[8].wwww)+(r2.xxyx)).xz;
    // 46: mad r0.yw, r0.yyyy, cb0[8].wwww, r1.xxxy
    r0.yw = ((r0.yyyy)*(source[8].wwww)+(r1.xxxy)).yw;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t1.yxzw, s2, cb0[5].x
    r0.y = (ArtistNativeSample2((r0.ywyy).xy, (source[5].xxxx).x, true).yxzw).y;
    // 48: mul_sat r0.y, r0.y, cb0[11].z
    r0.y = (saturate((r0.yyyy)*(source[11].zzzz))).y;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t3.xwyz, s0, cb0[5].x
    r0.xzw = (ArtistNativeSample0((r0.xzxx).xy, (source[5].xxxx).x, true).xwyz).xzw;
    // 50: mov_sat r1.x, v4.y
    r1.x = (saturate(v4.yyyy)).x;
    // 51: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: add r0.y, r0.y, -r1.x
    r0.y = ((r0.yyyy)+(-(r1.xxxx))).y;
    // 53: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t2.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 55: mul_sat r0.y, r0.y, r1.x
    r0.y = (saturate((r0.yyyy)*(r1.xxxx))).y;
    // 56: log r1.x, r0.y
    r1.x = (log2(r0.yyyy)).x;
    // 57: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 58: mul r1.x, r1.x, cb0[11].w
    r1.x = ((r1.xxxx)*(source[11].wwww)).x;
    // 59: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 60: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 61: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 62: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 63: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 64: add r1.xyz, -r0.xzwx, r1.xxxx
    r1.xyz = ((-(r0.xzwx))+(r1.xxxx)).xyz;
    // 65: mad r0.xzw, cb0[9].xxxx, r1.xxyz, r0.xxzw
    r0.xzw = ((source[9].xxxx)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 66: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 67: log r0.xzw, r0.xxzw
    r0.xzw = (log2(r0.xxzw)).xzw;
    // 68: mul r0.xzw, r0.xxzw, cb0[9].yyyy
    r0.xzw = ((r0.xxzw)*(source[9].yyyy)).xzw;
    // 69: exp r0.xzw, r0.xxzw
    r0.xzw = (exp2(r0.xxzw)).xzw;
    // 70: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 71: mul r0.xzw, r0.xxzw, r1.xxyz
    r0.xzw = ((r0.xxzw)*(r1.xxyz)).xzw;
    // 72: mad r0.xzw, r0.xxzw, v3.xxyz, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)+(source[1].xxyz)).xzw;
    // 73: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 74: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 75: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3871Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_k_me_makeflow_02_21_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative3872(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3872Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_o_pa_decmaster_01_03_tr: 57428f3e4c6a944caf236af1182cd898; selected map d7bd6eaa116e905bfbf293b885ddaca967b7032b4019b45f43e03a855b5a1aeb.
float4 ArtistNative3873(ARTIST_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f;
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
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t4.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 11: mov_sat r1.xy, r1.xyxx
    r1.xy = (saturate(r1.xyxx)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 13: mul r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)*(source[7].zzzz)).x;
    // 14: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: dp2 r0.z, r2.xyxx, r2.xyxx
    r0.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 16: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 17: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 18: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 19: add r2.z, r0.z, l(0.000010)
    r2.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 20: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 21: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 22: div r0.z, r2.z, r0.z
    r0.z = ((r2.zzzz)/(r0.zzzz)).z;
    // 23: max r0.z, r0.z, l(0.150000)
    r0.z = (max(r0.zzzz,float4(0.150000,0.150000,0.150000,0.150000))).z;
    // 24: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: add r1.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 26: mad r1.yz, r1.yyzy, cb0[3].xxyx, r0.xxyx
    r1.yz = ((r1.yyzy)*(source[3].xxyx)+(r0.xxyx)).yz;
    // 27: mad r0.xy, cb0[6].xxxx, v2.xyxx, r0.xyxx
    r0.xy = ((source[6].xxxx)*(v2.xyxx)+(r0.xyxx)).xy;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t3.xywz, s1, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 29: add r1.yz, r1.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r1.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t5.xyzw, s3, l(0.000000)
    r1.y = (ArtistNativeSample3((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 31: mad r1.zw, r0.xxxy, l(0.000000, 0.000000, 0.200000, 0.200000), v2.xxxy
    r1.zw = ((r0.xxxy)*(float4(0.000000,0.000000,0.200000,0.200000))+(v2.xxxy)).zw;
    // 32: add r1.zw, r1.zzzw, cb0[4].xxxy
    r1.zw = ((r1.zzzw)+(source[4].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t3.yzxw, s1, l(0.000000)
    r1.z = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 34: add r1.z, r1.z, r1.y
    r1.z = ((r1.zzzz)+(r1.yyyy)).z;
    // 35: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 36: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 37: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 38: mul r1.z, r1.z, cb0[7].y
    r1.z = ((r1.zzzz)*(source[7].yyyy)).z;
    // 39: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 40: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 41: mul r2.xyz, r1.zzzz, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r2.xyzx)).xyz;
    // 42: mul r2.xyz, r2.xyzx, v3.xyzx
    r2.xyz = ((r2.xyzx)*(v3.xyzx)).xyz;
    // 43: movc r1.yzw, r1.yyyy, l(0,0,0,0), r2.xxyz
    r1.yzw = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 44: dp3 r2.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 45: add r2.xyz, -r0.xywx, r2.xxxx
    r2.xyz = ((-(r0.xywx))+(r2.xxxx)).xyz;
    // 46: mad r0.xyw, cb0[6].wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((source[6].wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 47: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 48: mul r0.xyw, r0.xyxw, r2.xyxz
    r0.xyw = ((r0.xyxw)*(r2.xyxz)).xyw;
    // 49: mad r0.xyz, r0.zzzz, r0.xywx, r1.yzwy
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r1.yzwy)).xyz;
    // 50: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 51: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 52: log r0.x, |r1.x|
    r0.x = (log2(abs(r1.xxxx))).x;
    // 53: lt r0.y, |r1.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 54: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 55: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 56: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 57: mul r0.yz, v2.xxyx, cb0[8].xxxx
    r0.yz = ((v2.xxyx)*(source[8].xxxx)).yz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 59: mov_sat r0.z, v3.w
    r0.z = (saturate(v3.wwww)).z;
    // 60: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 61: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 62: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 64: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_i_pa_shine_01_04_tr: a14d57c96465f346be3437ad63c9ad10; selected map 0716202d132c7fa3cb9c994bb5370a1db2710298fb3e58040e580c23ad3e1c1b.
float4 ArtistNative3874(ARTIST_NATIVE_INPUT input)
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
// fx_b_pa_blood_01_tr: 294d163b9831d341bf69a06b3cf9378c; selected map e68474ea280796b25e6ca13e9b7939b68a781951fd9c67ed7592c33006a8a6e7.
float4 ArtistNative3875(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: mad r0.xy, r0.xyxx, l(0.200000, 0.200000, 0.000000, 0.000000), l(0.000000, 0.650000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.200000,0.200000,0.000000,0.000000))+(float4(0.000000,0.650000,0.000000,0.000000))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 6: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 7: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: mul r0.yzw, r0.yyyy, cb0[2].xxyz
    r0.yzw = ((r0.yyyy)*(source[2].xxyz)).yzw;
    // 10: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 11: mul r1.xy, v2.xyxx, l(12.000000, 12.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(float4(12.000000,12.000000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 14: add r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 15: mad r0.xyz, r0.wwww, v3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(v3.xyzx)+(r0.xyzx)).xyz;
    // 16: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 17: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.zwzz, t0.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 20: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 21: mad r0.x, v0.w, r0.x, r0.y
    r0.x = ((v0.wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 22: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul r0.x, r0.x, cb0[3].y
    r0.x = ((r0.xxxx)*(source[3].yyyy)).x;
    // 24: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 25: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 26: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 27: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 28: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 29: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 30: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 31: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_b_pa_water_02_tr: 60145a4ebbbf514fbc6a4cc0a24c1f39; selected map e25b7a28058a686b9b1ffa581b4d7e5da9d8401793a13ff3358aab9e5a79a5fb.
float4 ArtistNative3877(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xy, v4.xxxx, l(-0.070000, 0.200000, 0.000000, 0.000000)
    r0.xy = ((v4.xxxx)*(float4(-0.070000,0.200000,0.000000,0.000000))).xy;
    // 2: mad r0.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.x, r0.x, l(2.000000), l(-1.000000)
    r0.x = ((r0.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 5: mad r0.yz, v4.xxxx, l(0.000000, 0.100000, 0.130000, 0.000000), v2.xxyx
    r0.yz = ((v4.xxxx)*(float4(0.000000,0.100000,0.130000,0.000000))+(v2.xxyx)).yz;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 7: mad r0.x, r0.y, l(2.000000), r0.x
    r0.x = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(r0.xxxx)).x;
    // 8: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 9: mad r0.xy, r0.xxxx, l(0.170000, 0.170000, 0.000000, 0.000000), v2.xyxx
    r0.xy = ((r0.xxxx)*(float4(0.170000,0.170000,0.000000,0.000000))+(v2.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.yxzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 12: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 13: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 14: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 15: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 16: source device depth mapped to centimetre view depth; reconstruction at 18.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 18-21: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 22: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 23: add r0.z, -cb0[2].x, l(1.000000)
    r0.z = ((-(source[2].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 24: mul r0.z, r0.z, l(50.000000)
    r0.z = ((r0.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000))).z;
    // 25: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 26: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 27: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 28: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 29: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 30: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_ribtrails_01_tr: 42c3e19906952a49b08de7d377bb2296; selected map 3d09eeccfcd5e4217dcc4f3d52d4c10908a61e433e9af86b768aa147b4cd88ea.
float4 ArtistNative3878(ARTIST_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: mul r0.x, v4.x, cb0[2].x
    r0.x = ((v4.xxxx)*(source[2].xxxx)).x;
    // 2: mul r0.xyzw, r0.xxxx, l(-0.500000, -2.000000, -1.500000, -0.700000)
    r0.xyzw = ((r0.xxxx)*(float4(-0.500000,-2.000000,-1.500000,-0.700000))).xyzw;
    // 3: mov r1.xz, r0.yywy
    r1.xz = (r0.yywy).xz;
    // 4: mov r1.yw, l(0,0,0,0.500000)
    r1.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),0.500000)).yw;
    // 5: mad_sat r2.x, -v2.x, l(2.000000), l(1.000000)
    r2.x = (saturate((-(v2.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 6: mul r2.y, r2.x, r2.x
    r2.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 7: mul r2.y, r2.y, r2.x
    r2.y = ((r2.yyyy)*(r2.xxxx)).y;
    // 8: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 9: movc r2.x, r2.x, l(0), r2.y
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).x;
    // 10: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mov r0.yw, l(0,0,0,0.500000)
    r0.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),0.500000)).yw;
    // 12: add r0.xy, r0.xyxx, v2.xyxx
    r0.xy = ((r0.xyxx)+(v2.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: mul r3.xyz, v2.xxyx, l(6.000000, 1.500000, 1.000000, 0.000000)
    r3.xyz = ((v2.xxyx)*(float4(6.000000,1.500000,1.000000,0.000000))).xyz;
    // 15: mad r4.xy, r0.xxxx, l(0.200000, 0.200000, 0.000000, 0.000000), r3.yzyy
    r4.xy = ((r0.xxxx)*(float4(0.200000,0.200000,0.000000,0.000000))+(r3.yzyy)).xy;
    // 16: mov_sat r3.x, r3.x
    r3.x = (saturate(r3.xxxx)).x;
    // 17: add r0.x, r4.y, l(-0.500000)
    r0.x = ((r4.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 18: div r4.z, r0.x, r2.y
    r4.z = ((r0.xxxx)/(r2.yyyy)).z;
    // 19: add r1.xyzw, r1.xyzw, r4.xyxz
    r1.xyzw = ((r1.xyzw)+(r4.xyxz)).xyzw;
    // 20: add r0.xy, r0.zwzz, r4.xzxx
    r0.xy = ((r0.zwzz)+(r4.xzxx)).xy;
    // 21: min r0.z, |r4.z|, l(1.000000)
    r0.z = (min(abs(r4.zzzz),float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 22: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t1.xywz, s2, l(0.000000)
    r0.xyw = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t3.yzxw, s3, l(0.000000)
    r1.z = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 26: mul r1.yzw, r0.xxyw, r1.zzzz
    r1.yzw = ((r0.xxyw)*(r1.zzzz)).yzw;
    // 27: mul r1.yzw, r1.yyzw, l(0.000000, 0.700000, 0.700000, 0.700000)
    r1.yzw = ((r1.yyzw)*(float4(0.000000,0.700000,0.700000,0.700000))).yzw;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t2.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: mul r0.y, r0.y, r2.x
    r0.y = ((r0.yyyy)*(r2.xxxx)).y;
    // 30: mul r0.y, r1.x, r0.y
    r0.y = ((r1.xxxx)*(r0.yyyy)).y;
    // 31: mad r1.xyz, r0.yyyy, l(8.000000, 8.000000, 8.000000, 0.000000), r1.yzwy
    r1.xyz = ((r0.yyyy)*(float4(8.000000,8.000000,8.000000,0.000000))+(r1.yzwy)).xyz;
    // 32: mad r1.xyz, v3.xyzx, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((v3.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 33: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 34: mad_sat r0.y, v2.x, l(2.000000), l(-1.000000)
    r0.y = (saturate((v2.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul r0.y, r3.x, r0.y
    r0.y = ((r3.xxxx)*(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 38: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 39: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 40: mul r0.z, r0.z, l(0.850000)
    r0.z = ((r0.zzzz)*(float4(0.850000,0.850000,0.850000,0.850000))).z;
    // 41: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 42: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 43: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 44: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 45: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_f_pa_distort_01_1_ad: 7b4032bee7c7ac4b9b00079d1cc065e5; selected map 1e0d64da08b9e031b15db5a242c8fae16a7d21fba9563e1ed9b74931dea0979b.
float4 ArtistNative3879(ARTIST_NATIVE_INPUT input)
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
    // 10: mul_sat r0.x, r0.x, l(0.062500)
    r0.x = (saturate((r0.xxxx)*(float4(0.062500,0.062500,0.062500,0.062500)))).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v2.xyxx, t0.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 12: mul r1.x, r0.y, v3.w
    r1.x = ((r0.yyyy)*(v3.wwww)).x;
    // 13: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 14: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 15: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 16: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 17: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 18: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3879Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v1.xyxx, l(1.400000, 1.400000, 0.000000, 0.000000), cb0[1].xyxx
    r0.xy = ((v1.xyxx)*(float4(1.400000,1.400000,0.000000,0.000000))+(source[1].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xy = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mul r0.xyzw, r0.xyxy, l(100.000000, 100.000000, 100.000000, 100.000000)
    r0.xyzw = ((r0.xyxy)*(float4(100.000000,100.000000,100.000000,100.000000))).xyzw;
    // 4: add r1.xy, v1.xyxx, cb0[0].xyxx
    r1.xy = ((v1.xyxx)+(source[0].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xyzw, r1.xyxy, l(55.000000, 55.000000, 55.000000, 55.000000), r0.xyzw
    r0.xyzw = ((r1.xyxy)*(float4(55.000000,55.000000,55.000000,55.000000))+(r0.xyzw)).xyzw;
    // 7: add r1.xy, v1.xyxx, cb0[2].xyxx
    r1.xy = ((v1.xyxx)+(source[2].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r1.xy = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 9: mad r1.xyzw, r1.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r1.xyzw = ((r1.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 10: mul r0.xyzw, r0.xyzw, r1.xyzw
    r0.xyzw = ((r0.xyzw)*(r1.xyzw)).xyzw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample0((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 13: mul r0.xyzw, r0.xyzw, v3.xxxx
    r0.xyzw = ((r0.xyzw)*(v3.xxxx)).xyzw;
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
    // 20: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
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
    // 33: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
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
// fx_b_pa_distort_03_ad: d91dfd0634340d438c95f296d4802f92; selected map a9aef74f86a1f355041bb4e6dfe34a117ed462deeff5032ebf5c2753c369c7e2.
float4 ArtistNative3880(ARTIST_NATIVE_INPUT input)
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
    // 10: mul_sat r0.x, r0.x, l(0.062500)
    r0.x = (saturate((r0.xxxx)*(float4(0.062500,0.062500,0.062500,0.062500)))).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 12: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 13: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 14: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 15: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 16: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 17: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 18: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3880Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0700000003, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[0].xyxx
    r0.xy = ((v1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(source[0].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.yz, v1.xxyx, cb0[1].xxyx
    r0.yz = ((v1.xxyx)+(source[1].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: mad r0.xy, r0.xxxx, l(3.000000, 3.000000, 0.000000, 0.000000), v1.xyxx
    r0.xy = ((r0.xxxx)*(float4(3.000000,3.000000,0.000000,0.000000))+(v1.xyxx)).xy;
    // 7: add r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)+(source[2].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t3.xyzw, s4, l(0.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v1.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 10: mul r0.xyzw, r0.xyxy, r0.zzzz
    r0.xyzw = ((r0.xyxy)*(r0.zzzz)).xyzw;
    // 11: mul r0.xyzw, r0.xyzw, v3.xxxx
    r0.xyzw = ((r0.xyzw)*(v3.xxxx)).xyzw;
    // 12: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 13: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 14: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 15: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 16: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 17: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 18: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 19: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 20: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 21: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 22: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 23: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 24: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 25: source device depth mapped to centimetre view depth; reconstruction at 27.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 27-30: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 31: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 32: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 33: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 34: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 35: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ap_06_1_tr: 7acbcf3cd1edd246ae1a2765f2a04004; selected map 058ec721c523b7ed33e958679d828057c42e642a58813f35bef210c7b562f500.
float4 ArtistNative3881(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[2u];
    source[1].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[1].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[1].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[1].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.x, cb0[1].x, v4.w, l(-1.000000)
    r0.x = ((source[1].xxxx)*(v4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.w, cb0[1].x
    r0.y = ((v4.wwww)*(source[1].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul r1.xyzw, r0.wxyz, cb0[1].wyyy
    r1.xyzw = ((r0.wxyz)*(source[1].wyyy)).xyzw;
    // 7: log r0.w, |r1.x|
    r0.w = (log2(abs(r1.xxxx))).w;
    // 8: mul r0.w, r0.w, cb0[2].x
    r0.w = ((r0.wwww)*(source[2].xxxx)).w;
    // 9: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 10: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 12: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 13: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 17: add r1.x, r1.x, l(-0.333300)
    r1.x = ((r1.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 18: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 19: or r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) | asuint(r1.xxxx))).w;
    // 20: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) clip(-1.f);
    // 21: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 22: dp3 r0.w, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 23: mad r0.xyz, -r0.xyzx, cb0[1].yyyy, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(source[1].yyyy)+(r0.wwww)).xyz;
    // 24: mad r0.xyz, cb0[1].zzzz, r0.xyzx, r1.yzwy
    r0.xyz = ((source[1].zzzz)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 25: mad o0.xyz, r0.xyzx, v3.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)*(v3.xyzx)+(source[0].xyzx)).xyz;
    // 26: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ap_33_1_tr: 8465aa2d053e0a49a2a1ebaf2d74252f; selected map aafc90fd5955a9df6457eab89b2ed9f23e98bdb7a9a6247ef767cb399e675e91.
float4 ArtistNative3882(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].zzzz,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mad r0.xy, cb0[4].xxxx, cb0[2].xyxx, v2.xyxx
    r0.xy = ((source[4].xxxx)*(source[2].xyxx)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[4].wwww
    r0.xy = ((r0.xyxx)*(source[4].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[4].xxxx, cb0[3].xxxy, v2.xxxy
    r0.zw = ((source[4].xxxx)*(source[3].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[5].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 8: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 9: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 11: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 12: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 13: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 14: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 15: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 16: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ap_29_1_tr: 33399c59f6435c46a1b065bf07309c47; selected map 08b90d3ca001533bbbbb512828aeee73843b6c6c9ec1f304c4c0d01334dd2666.
float4 ArtistNative3883(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[8].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[8].z = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.25, 0.0, 0.0, 0.0))).x;
    source[8].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.25, 0.0, 0.0, 0.0)))).x;
    source[9].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 2: dp2 r0.x, cb0[2].xyxx, r0.xyxx
    r0.x = (dot((source[2].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: add r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 4: add r0.y, r0.x, v4.x
    r0.y = ((r0.xxxx)+(v4.xxxx)).y;
    // 5: add r0.x, -r0.x, l(1.900000)
    r0.x = ((-(r0.xxxx))+(float4(1.900000,1.900000,1.900000,1.900000))).x;
    // 6: add r0.y, r0.y, l(-0.500000)
    r0.y = ((r0.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 7: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 8: mul r0.z, r0.z, l(10.000000)
    r0.z = ((r0.zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 9: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 10: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 11: lt r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((abs(r0.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 12: or r0.x, r0.y, r0.x
    r0.x = (asfloat(asuint(r0.yyyy) | asuint(r0.xxxx))).x;
    // 13: mul r0.y, r0.w, l(50.000000)
    r0.y = ((r0.wwww)*(float4(50.000000,50.000000,50.000000,50.000000))).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: min r0.yz, r0.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = (min(r0.yyzy,float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 16: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 17: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 18: mad r1.xyzw, v2.xyxy, l(2.000000, 1.000000, 0.500000, 0.500000), l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((v2.xyxy)*(float4(2.000000,1.000000,0.500000,0.500000))+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 19: dp2 r2.x, cb0[2].xyxx, r1.zwzz
    r2.x = (dot((source[2].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 20: dp2 r2.y, cb0[3].xyxx, r1.zwzz
    r2.y = (dot((source[3].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 21: add r0.yz, r2.xxyx, cb0[7].xxyx
    r0.yz = ((r2.xxyx)+(source[7].xxyx)).yz;
    // 22: add r0.yz, r0.yyzy, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t1.zxyw, s2, l(0.000000)
    r0.yz = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 24: mul r0.yz, r0.yyzy, cb0[10].xxxx
    r0.yz = ((r0.yyzy)*(source[10].xxxx)).yz;
    // 25: dp2 r2.x, cb0[2].xyxx, r1.xyxx
    r2.x = (dot((source[2].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 26: dp2 r2.y, cb0[3].xyxx, r1.xyxx
    r2.y = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 27: add r1.xy, r2.xyxx, cb0[6].xyxx
    r1.xy = ((r2.xyxx)+(source[6].xyxx)).xy;
    // 28: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 30: mad r0.yz, cb0[9].yyyy, r1.xxyx, r0.yyzy
    r0.yz = ((source[9].yyyy)*(r1.xxyx)+(r0.yyzy)).yz;
    // 31: mad r1.xy, v2.xyxx, l(-1.000000, 1.000000, 0.000000, 0.000000), l(0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(float4(-1.000000,1.000000,0.000000,0.000000))+(float4(0.500000,-0.500000,0.000000,0.000000))).xy;
    // 32: dp2 r2.x, cb0[2].xyxx, r1.xyxx
    r2.x = (dot((source[2].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 33: dp2 r2.y, cb0[3].xyxx, r1.xyxx
    r2.y = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 34: add r1.xy, r2.xyxx, cb0[4].xyxx
    r1.xy = ((r2.xyxx)+(source[4].xyxx)).xy;
    // 35: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 36: add r1.xy, -r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 37: mad r1.xy, r1.xyxx, cb0[5].xyxx, cb0[8].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xyxx)+(source[8].xxxx)).xy;
    // 38: add r0.yz, r0.yyzy, r1.xxyx
    r0.yz = ((r0.yyzy)+(r1.xxyx)).yz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s3, l(0.000000)
    r0.yzw = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 40: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 41: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 42: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 43: mul r0.z, r0.z, cb0[10].y
    r0.z = ((r0.zzzz)*(source[10].yyyy)).z;
    // 44: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 45: mul r0.z, r0.z, cb0[10].z
    r0.z = ((r0.zzzz)*(source[10].zzzz)).z;
    // 46: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 47: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 48: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 49: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 50: source device depth mapped to centimetre view depth; reconstruction at 52.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 52-55: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 56: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 57: add r0.w, -cb0[10].w, l(1.000000)
    r0.w = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 59: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 60: mul_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)*(r0.xxxx))).x;
    // 61: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 62: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 63: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 64: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_cd_09_1_tr: 85b8854050c4a54092e293beee88bcfd; selected map 5703f27e9bc3ef7b09813e2decc9bf81d4097c75ada5c7709c72e320e9f1f96b.
float4 ArtistNative3884(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[6].w, l(1.000000)
    r0.y = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t2.yxzw, s1, l(0.000000)
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
    // 28: mul r0.zw, v2.xxxy, cb0[4].yyyz
    r0.zw = ((v2.xxxy)*(source[4].yyyz)).zw;
    // 29: mad r1.x, cb0[4].x, cb0[3].w, r0.z
    r1.x = ((source[4].xxxx)*(source[3].wwww)+(r0.zzzz)).x;
    // 30: mad r1.y, cb0[4].x, cb0[4].w, r0.w
    r1.y = ((source[4].xxxx)*(source[4].wwww)+(r0.wwww)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 32: mad r0.zw, cb0[5].xxxx, r0.zzzw, v2.xxxy
    r0.zw = ((source[5].xxxx)*(r0.zzzw)+(v2.xxxy)).zw;
    // 33: mad r1.x, cb0[5].y, v4.z, l(-1.000000)
    r1.x = ((source[5].yyyy)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 34: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 35: mul r1.y, v4.z, cb0[5].y
    r1.y = ((v4.zzzz)*(source[5].yyyy)).y;
    // 36: mad r0.zw, r1.yyyy, r0.zzzw, -r1.xxxx
    r0.zw = ((r1.yyyy)*(r0.zzzw)+(-(r1.xxxx))).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.zwzz, t1.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r0.zwzz, t4.wxyz, s3, l(0.000000)
    r1.yzw = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 39: log r0.z, |r1.x|
    r0.z = (log2(abs(r1.xxxx))).z;
    // 40: lt r0.w, |r1.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 41: mul r0.z, r0.z, cb0[6].z
    r0.z = ((r0.zzzz)*(source[6].zzzz)).z;
    // 42: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 43: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 44: movc r0.z, r0.w, l(0), |r0.z|
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.zzzz))).z;
    // 45: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 46: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 47: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 48: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 49: mul r0.w, r0.y, r0.w
    r0.w = ((r0.yyyy)*(r0.wwww)).w;
    // 50: ge r0.y, l(0.250000), r0.y
    r0.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.yyyy)) * 0xffffffffu)).y;
    // 51: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 52: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 54: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 55: mul r0.xzw, r1.yyzw, cb0[5].zzzz
    r0.xzw = ((r1.yyzw)*(source[5].zzzz)).xzw;
    // 56: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 57: mad r1.xyz, -cb0[5].zzzz, r1.yzwy, r1.xxxx
    r1.xyz = ((-(source[5].zzzz))*(r1.yzwy)+(r1.xxxx)).xyz;
    // 58: mad r0.xzw, cb0[5].wwww, r1.xxyz, r0.xxzw
    r0.xzw = ((source[5].wwww)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 59: mul r1.xyz, r0.xzwx, cb0[6].xxxx
    r1.xyz = ((r0.xzwx)*(source[6].xxxx)).xyz;
    // 60: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 61: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 62: mul r1.xyz, r1.xyzx, cb0[6].yyyy
    r1.xyz = ((r1.xyzx)*(source[6].yyyy)).xyz;
    // 63: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 64: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 65: add r0.xzw, r0.xxzw, r1.xxxx
    r0.xzw = ((r0.xxzw)+(r1.xxxx)).xzw;
    // 66: mul r0.xzw, r0.xxzw, v3.xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)).xzw;
    // 67: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 68: movc r0.xyz, r0.yyyy, r1.xyzx, r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (r1.xyzx) : (r0.xzwx)).xyz;
    // 69: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 70: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3884Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_e_pa_ri_14_1_tr: df7604a8d1f8034a9ef7eb666b1fb750; selected map d1583bec6da2d4138cae14e238de1bdc5d527804d6a584a4a7b7205891845817.
float4 ArtistNative3885(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[4].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[3u].xxxx)).x;
    source[7].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[3u].xxxx))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].wwww)).x;
    source[7].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].zzzz))).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[9].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 27: mad r0.y, r0.y, l(0.159155), cb0[2].x
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(source[2].xxxx)).y;
    // 28: add r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 29: mul r1.x, r0.y, cb0[4].w
    r1.x = ((r0.yyyy)*(source[4].wwww)).x;
    // 30: mad r0.y, r0.x, l(2.000000), cb0[3].y
    r0.y = ((r0.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[3].yyyy)).y;
    // 31: mul r1.y, r0.y, cb0[5].z
    r1.y = ((r0.yyyy)*(source[5].zzzz)).y;
    // 32: sample_l_indexable(texture2d)(float,float,float,float) r0.yz, r1.xyxx, t0.zxyw, s1, l(0.000000)
    r0.yz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).zxyw).yz;
    // 33: mad r0.yz, v4.zzzz, r0.yyzy, v2.xxyx
    r0.yz = ((v4.zzzz)*(r0.yyzy)+(v2.xxyx)).yz;
    // 34: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 35: mul r0.yz, r0.yyzy, cb0[5].wwww
    r0.yz = ((r0.yyzy)*(source[5].wwww)).yz;
    // 36: max r0.w, |r0.z|, |r0.y|
    r0.w = (max(abs(r0.zzzz),abs(r0.yyyy))).w;
    // 37: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 38: min r1.x, |r0.z|, |r0.y|
    r1.x = (min(abs(r0.zzzz),abs(r0.yyyy))).x;
    // 39: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 40: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 41: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 42: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 43: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 44: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 45: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 46: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 47: lt r1.z, |r0.z|, |r0.y|
    r1.z = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.yyyy))) * 0xffffffffu)).z;
    // 48: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 49: mad r0.w, r0.w, r1.x, r1.y
    r0.w = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).w;
    // 50: lt r1.x, r0.z, -r0.z
    r1.x = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).x;
    // 51: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 52: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 53: min r1.x, r0.z, r0.y
    r1.x = (min(r0.zzzz,r0.yyyy)).x;
    // 54: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 55: max r1.y, r0.z, r0.y
    r1.y = (max(r0.zzzz,r0.yyyy)).y;
    // 56: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 57: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 58: mad r0.y, r0.y, l(2.000000), v4.y
    r0.y = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(v4.yyyy)).y;
    // 59: add r2.y, r0.y, l(-1.000000)
    r2.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 60: ge r0.y, r1.y, -r1.y
    r0.y = (asfloat((uint4)((r1.yyyy)>=(-(r1.yyyy))) * 0xffffffffu)).y;
    // 61: and r0.y, r0.y, r1.x
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r1.xxxx))).y;
    // 62: movc r0.y, r0.y, -r0.w, r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.wwww)) : (r0.wwww)).y;
    // 63: mad r0.y, r0.y, l(0.159155), v4.x
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(v4.xxxx)).y;
    // 64: add r2.x, r0.y, l(-0.500000)
    r2.x = ((r0.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 65: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t1.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
    // 66: mad r1.x, -r0.x, cb0[8].w, l(1.000000)
    r1.x = ((-(r0.xxxx))*(source[8].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 67: mad r0.x, -r0.x, cb0[7].x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[7].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 68: mul_sat r0.x, r0.x, cb0[8].x
    r0.x = (saturate((r0.xxxx)*(source[8].xxxx))).x;
    // 69: mul_sat r1.x, r1.x, cb0[9].w
    r1.x = (saturate((r1.xxxx)*(source[9].wwww))).x;
    // 70: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 71: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 72: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 73: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 74: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 75: mul r0.x, r0.x, cb0[10].x
    r0.x = ((r0.xxxx)*(source[10].xxxx)).x;
    // 76: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 77: add r1.x, -v4.w, l(1.000000)
    r1.x = ((-(v4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: mad r0.x, cb0[10].y, r0.x, -r1.x
    r0.x = ((source[10].yyyy)*(r0.xxxx)+(-(r1.xxxx))).x;
    // 79: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 80: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 81: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 82: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 83: mad r0.xyz, cb0[6].xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((source[6].xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 84: mul r0.xyz, r0.xyzx, v3.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)).xyz;
    // 85: mad r0.xyz, cb0[6].yyyy, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((source[6].yyyy)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 86: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_shorkwave_01_10_tr: 5c707b8a1ddaf5478de6e37c4b55bf26; selected map f1f290972b5b31eec74b516c45ee383023e1c16a6224be179cff954bdcae7562.
float4 ArtistNative3886(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = (cos((g_ArtistSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r1.xyz = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ri_03_1_tr: 991ecc5522859c49bb034318a827af71; selected map fd85ade76eaa888dbc097dfdfc3900cacd64ae68812cae449a674d712986917f.
float4 ArtistNative3887(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3887Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_m_me_trail_02_5_tr: 1438f1e4a3f5a24ca4e9866f51a98ec0; selected map dd3b9874b1c5cc07226d48fed12cb259612fc0e17634adb8f59dec6885a55b04.
float4 ArtistNative3888(ARTIST_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[13u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[8u].xxxx,g_ArtistSourceMaterialParameters[8u].yyyy,1u);
    source[4] = input.dynamicParameter;
    source[5] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[9].x = (cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[9].w = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[11u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[16].y = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[16].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[17].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[17].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[18].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[19].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[19].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[20].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[20].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[20].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
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
    // 1: mul r0.x, v4.x, cb0[13].w
    r0.x = ((v4.xxxx)*(source[13].wwww)).x;
    // 2: mul r0.y, v4.y, cb0[14].x
    r0.y = ((v4.yyyy)*(source[14].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)+(source[7].xyxx)).xy;
    // 4: mad r1.x, cb0[4].z, cb0[14].w, r0.x
    r1.x = ((source[4].zzzz)*(source[14].wwww)+(r0.xxxx)).x;
    // 5: mad r1.y, cb0[4].z, cb0[15].x, r0.y
    r1.y = ((source[4].zzzz)*(source[15].xxxx)+(r0.yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.yz, v4.xxyx, cb0[10].yyzy
    r0.yz = ((v4.xxyx)*(source[10].yyzy)).yz;
    // 8: mad r0.yz, cb0[4].zzzz, cb0[11].xxyx, r0.yyzy
    r0.yz = ((source[4].zzzz)*(source[11].xxyx)+(r0.yyzy)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mad r0.z, v4.y, cb0[10].z, cb0[10].w
    r0.z = ((v4.yyyy)*(source[10].zzzz)+(source[10].wwww)).z;
    // 11: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 12: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 13: mad r0.zw, v4.xxxy, cb0[15].yyyz, cb0[8].xxxy
    r0.zw = ((v4.xxxy)*(source[15].yyyz)+(source[8].xxxy)).zw;
    // 14: mad r0.zw, cb0[16].yyyy, r0.yyyy, r0.zzzw
    r0.zw = ((source[16].yyyy)*(r0.yyyy)+(r0.zzzw)).zw;
    // 15: add r1.xy, cb0[4].xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[4].xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r0.zw, r1.xxxx, cb0[16].zzzw, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[16].zzzw)+(r0.zzzw)).zw;
    // 17: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 18: dp2 r2.x, cb0[5].xyxx, r0.zwzz
    r2.x = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 19: dp2 r2.y, cb0[6].xyxx, r0.zwzz
    r2.y = (dot((source[6].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 20: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 22: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 23: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 24: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 28: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r0.z, r0.z, cb0[17].y
    r0.z = ((r0.zzzz)*(source[17].yyyy)).z;
    // 31: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 32: mul_sat r0.z, r0.z, cb0[17].z
    r0.z = (saturate((r0.zzzz)*(source[17].zzzz))).z;
    // 33: mul r1.zw, cb0[4].zzzz, cb0[19].zzzw
    r1.zw = ((source[4].zzzz)*(source[19].zzzw)).zw;
    // 34: mad r1.zw, cb0[19].xxxy, v4.xxxy, r1.zzzw
    r1.zw = ((source[19].xxxy)*(v4.xxxy)+(r1.zzzw)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t3.yzwx, s4, l(0.000000)
    r0.w = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: mul r1.zw, cb0[4].zzzz, cb0[18].zzzw
    r1.zw = ((source[4].zzzz)*(source[18].zzzw)).zw;
    // 37: mad r1.zw, cb0[18].xxxy, v4.xxxy, r1.zzzw
    r1.zw = ((source[18].xxxy)*(v4.xxxy)+(r1.zzzw)).zw;
    // 38: mad r1.zw, r0.wwww, cb0[20].xxxx, r1.zzzw
    r1.zw = ((r0.wwww)*(source[20].xxxx)+(r1.zzzw)).zw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t4.yzwx, s5, l(0.000000)
    r0.w = (ArtistNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: add r0.w, -r1.y, r0.w
    r0.w = ((-(r1.yyyy))+(r0.wwww)).w;
    // 41: mul_sat r0.w, r0.w, cb0[20].y
    r0.w = (saturate((r0.wwww)*(source[20].yyyy))).w;
    // 42: add r1.y, -v4.y, l(1.000000)
    r1.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: mul r1.y, r1.y, v4.y
    r1.y = ((r1.yyyy)*(v4.yyyy)).y;
    // 44: mul_sat r1.y, r1.y, cb0[17].w
    r1.y = (saturate((r1.yyyy)*(source[17].wwww))).y;
    // 45: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 46: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 47: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 48: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 49: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 50: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 51: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 52: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 53: mul r0.w, r0.w, cb0[20].z
    r0.w = ((r0.wwww)*(source[20].zzzz)).w;
    // 54: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 55: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 56: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 59: mad r0.xz, v4.xxyx, cb0[9].yyzy, cb0[3].xxyx
    r0.xz = ((v4.xxyx)*(source[9].yyzy)+(source[3].xxyx)).xz;
    // 60: mad r0.xy, r0.yyyy, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xzxx
    r0.xy = ((r0.yyyy)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xzxx)).xy;
    // 61: mad r0.xy, r1.xxxx, cb0[11].zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[11].zwzz)+(r0.xyxx)).xy;
    // 62: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 64: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 65: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 67: mul r0.yz, cb0[4].zzzz, cb0[12].zzwz
    r0.yz = ((source[4].zzzz)*(source[12].zzwz)).yz;
    // 68: mad r0.yz, v4.xxyx, cb0[12].xxyx, r0.yyzy
    r0.yz = ((v4.xxyx)*(source[12].xxyx)+(r0.yyzy)).yz;
    // 69: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 70: dp2 r1.x, cb0[5].xyxx, r0.yzyy
    r1.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 71: dp2 r1.y, cb0[6].xyxx, r0.yzyy
    r1.y = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 72: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 74: add r0.y, -r0.x, r0.y
    r0.y = ((-(r0.xxxx))+(r0.yyyy)).y;
    // 75: mad r0.x, r0.y, l(0.500000), r0.x
    r0.x = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.xxxx)).x;
    // 76: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 77: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 78: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 79: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 80: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 81: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 82: mul r0.z, r0.z, cb0[13].y
    r0.z = ((r0.zzzz)*(source[13].yyyy)).z;
    // 83: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 84: mad r0.x, r0.y, cb0[13].z, r0.x
    r0.x = ((r0.yyyy)*(source[13].zzzz)+(r0.xxxx)).x;
    // 85: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 86: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_ht_02_1_ad: b0d551a53864c9408d2c03215f9ef88f; selected map 4dd93e1b4ffae10c2db3ed0c4c9132c6db631aa6aa2fdbffcc7ea3e1b5d89ab4.
float4 ArtistNative3889(ARTIST_NATIVE_INPUT input)
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
    // 29: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 30: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 31: mul r0.yzw, r1.xxyz, cb0[2].yyyy
    r0.yzw = ((r1.xxyz)*(source[2].yyyy)).yzw;
    // 32: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 33: mad r1.xyz, -cb0[2].yyyy, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[2].yyyy))*(r1.xyzx)+(r1.wwww)).xyz;
    // 34: mad r0.yzw, cb0[2].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[2].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 35: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 36: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 37: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 38: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3889Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_ring_07_03_ad: 55c8f3a3029acc42a10aaa723568e34b; selected map b1f2e930fa663280c0c2095c371485964b730a4146e6c152bdccff5a33239493.
float4 ArtistNative3890(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3890Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_d_pa_smoke_ulit_01_09_dt_tr: c09ef819991e34448b72ea36ee9d1242; selected map 0fa557db3ddb45f15aabefc9953ec128db20431b08a8a53f2bf6c1c3e973117b.
float4 ArtistNative3891(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[1u].xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
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
    // 1: mov r0.y, l(0)
    r0.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 2: mul r0.x, v4.x, l(0.200000)
    r0.x = ((v4.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 3: mad r0.xyzw, cb0[2].yyzz, v2.xyxy, r0.yxxy
    r0.xyzw = ((source[2].yyzz)*(v2.xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 15: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 16: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 17: source device depth mapped to centimetre view depth; reconstruction at 19.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 19-22: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 23: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 24: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 26: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 27: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 28: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 29: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 30: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 31: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 32: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_q_me_flowsoul_01_tr: 068829d56a2d2442806349551702f3d7; selected map 3a05f7867374e38a16c1d146f31452af93f30919353b78375245cb33df187659.
float4 ArtistNative3892(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = input.dynamicParameter;
    source[6] = g_ArtistSourceMaterialParameters[4u];
    source[7].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[8].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].y = ((float4(0.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
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
    // 6: mul r0.y, r0.y, cb0[10].z
    r0.y = ((r0.yyyy)*(source[10].zzzz)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul_sat r0.y, r0.y, cb0[10].w
    r0.y = (saturate((r0.yyyy)*(source[10].wwww))).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 11: dp2 r1.x, cb0[3].xyxx, r0.yzyy
    r1.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 12: dp2 r1.y, cb0[4].xyxx, r0.yzyy
    r1.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 13: dp2 r0.y, r1.xyxx, r1.xyxx
    r0.y = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 14: add r2.x, r1.x, l(0.500000)
    r2.x = ((r1.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 15: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 16: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 17: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 18: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 19: add r0.z, cb0[5].y, l(-1.000000)
    r0.z = ((source[5].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 20: add r3.x, r2.x, r0.z
    r3.x = ((r2.xxxx)+(r0.zzzz)).x;
    // 21: mul r0.z, cb0[5].z, cb0[8].z
    r0.z = ((source[5].zzzz)*(source[8].zzzz)).z;
    // 22: mad_sat r2.y, r1.y, r0.z, l(0.500000)
    r2.y = (saturate((r1.yyyy)*(r0.zzzz)+(float4(0.500000,0.500000,0.500000,0.500000)))).y;
    // 23: mov r3.y, r2.y
    r3.y = (r2.yyyy).y;
    // 24: mul r0.zw, r2.xxxy, cb0[7].zzzw
    r0.zw = ((r2.xxxy)*(source[7].zzzw)).zw;
    // 25: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r3.xyxx, t0.xyzw, s2, l(-1.000000)
    r1.x = (ArtistNativeSample2((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r3.xyxx, t2.yxzw, s1, l(0.000000)
    r1.y = (ArtistNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 27: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 28: mul_sat r0.y, r0.y, cb0[10].x
    r0.y = (saturate((r0.yyyy)*(source[10].xxxx))).y;
    // 29: log r1.x, r0.y
    r1.x = (log2(r0.yyyy)).x;
    // 30: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r1.x, r1.x, cb0[10].y
    r1.x = ((r1.xxxx)*(source[10].yyyy)).x;
    // 32: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 33: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 34: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 35: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 36: add r0.x, r0.x, cb0[11].y
    r0.x = ((r0.xxxx)+(source[11].yyyy)).x;
    // 37: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 38: log r0.x, |r1.y|
    r0.x = (log2(abs(r1.yyyy))).x;
    // 39: lt r0.y, |r1.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 40: mul r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)*(source[9].zzzz)).x;
    // 41: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 42: mul r0.x, r0.x, cb0[9].w
    r0.x = ((r0.xxxx)*(source[9].wwww)).x;
    // 43: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 44: mad r1.x, cb0[7].y, cb0[7].x, r0.z
    r1.x = ((source[7].yyyy)*(source[7].xxxx)+(r0.zzzz)).x;
    // 45: mad r1.y, cb0[7].y, cb0[8].w, r0.w
    r1.y = ((source[7].yyyy)*(source[8].wwww)+(r0.wwww)).y;
    // 46: mul r0.yz, r1.xxyx, l(0.000000, 1.660000, 1.660000, 0.000000)
    r0.yz = ((r1.xxyx)*(float4(0.000000,1.660000,1.660000,0.000000))).yz;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s0, l(-1.000000)
    r1.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s0, l(-1.000000)
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 49: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 50: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: mad r0.yzw, -r1.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r1.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 52: mad r0.yzw, cb0[9].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[9].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 53: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 54: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 55: mul r0.yzw, r0.yyzw, cb0[9].yyyy
    r0.yzw = ((r0.yyzw)*(source[9].yyyy)).yzw;
    // 56: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 57: mul r1.xyz, cb0[6].xyzx, cb0[6].wwww
    r1.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 58: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 59: mad r0.xyz, r0.xxxx, r0.yzwy, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r0.yzwy)).xyz;
    // 60: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 61: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3892Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_maskedrib_01_tr: 9530b47c36e0c844846fe7b003b626bd; selected map 6d5618d781a285c6c619b6b77fc41bace5296fa22ad01d2e7d2702bf8f10a8af.
float4 ArtistNative3893(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[3] = g_ArtistSourceMaterialParameters[3u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].wwww,g_ArtistSourceMaterialParameters[2u].xxxx,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
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
    // 1: mul r0.x, v2.z, cb0[6].w
    r0.x = ((v2.zzzz)*(source[6].wwww)).x;
    // 2: mul r0.z, v2.w, v4.y
    r0.z = ((v2.wwww)*(v4.yyyy)).z;
    // 3: mul r0.y, r0.z, cb0[7].x
    r0.y = ((r0.zzzz)*(source[7].xxxx)).y;
    // 4: mul r1.y, r0.z, cb0[5].y
    r1.y = ((r0.zzzz)*(source[5].yyyy)).y;
    // 5: mul r0.zw, r0.xxxy, l(0.000000, 0.000000, 1.500000, 1.000000)
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,1.500000,1.000000))).zw;
    // 6: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 8: mad r0.xy, r0.zzzz, v4.wwww, r0.xyxx
    r0.xy = ((r0.zzzz)*(v4.wwww)+(r0.xyxx)).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 11: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: mul_sat r0.y, r0.y, cb0[7].w
    r0.y = (saturate((r0.yyyy)*(source[7].wwww))).y;
    // 15: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 16: add r0.y, -v2.x, l(1.000000)
    r0.y = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 17: mul r0.z, |r0.y|, |r0.y|
    r0.z = ((abs(r0.yyyy))*(abs(r0.yyyy))).z;
    // 18: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 20: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 21: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 22: mul r0.z, r0.x, r0.y
    r0.z = ((r0.xxxx)*(r0.yyyy)).z;
    // 23: mad r0.x, r0.z, l(7.000000), r0.x
    r0.x = ((r0.zzzz)*(float4(7.000000,7.000000,7.000000,7.000000))+(r0.xxxx)).x;
    // 24: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 26: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 27: mul r1.x, v2.z, cb0[5].x
    r1.x = ((v2.zzzz)*(source[5].xxxx)).x;
    // 28: add r0.xz, r1.xxyx, cb0[2].xxyx
    r0.xz = ((r1.xxyx)+(source[2].xxyx)).xz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t2.xwyz, s0, l(0.000000)
    r0.xzw = (ArtistNativeSample0((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 30: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 31: add r1.xyz, -r0.xzwx, r1.xxxx
    r1.xyz = ((-(r0.xzwx))+(r1.xxxx)).xyz;
    // 32: mad r0.xzw, cb0[6].xxxx, r1.xxyz, r0.xxzw
    r0.xzw = ((source[6].xxxx)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 33: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 34: log r0.xzw, r0.xxzw
    r0.xzw = (log2(r0.xxzw)).xzw;
    // 35: mul r0.xzw, r0.xxzw, cb0[6].yyyy
    r0.xzw = ((r0.xxzw)*(source[6].yyyy)).xzw;
    // 36: exp r0.xzw, r0.xxzw
    r0.xzw = (exp2(r0.xxzw)).xzw;
    // 37: mul r0.xzw, r0.xxzw, cb0[3].xxyz
    r0.xzw = ((r0.xxzw)*(source[3].xxyz)).xzw;
    // 38: mad_sat r0.xyz, cb0[6].zzzz, r0.xzwx, r0.yyyy
    r0.xyz = (saturate((source[6].zzzz)*(r0.xzwx)+(r0.yyyy))).xyz;
    // 39: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3893Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_j_flowblur_01_2_tr: 7f34a1fedf7a614cbcd4e656c906fe5e; selected map f1a0c4de3fe3ecc81a70446042999006b2557b4999a4e140c5146df24f61bc38.
float4 ArtistNative3864(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = ((g_ArtistSourceMaterialParameters[0u].zzzz*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (clamp(g_ArtistSourceMaterialParameters[0u].yyyy,float4(-1.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
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
    // 7: mul r0.w, r0.y, r1.x
    r0.w = ((r0.yyyy)*(r1.xxxx)).w;
    // 8: mad r0.w, r0.x, r1.y, -r0.w
    r0.w = ((r0.xxxx)*(r1.yyyy)+(-(r0.wwww))).w;
    // 9: mul r0.y, r0.w, v1.w
    r0.y = ((r0.wwww)*(v1.wwww)).y;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 11: mul_sat r1.x, r0.w, cb0[3].y
    r1.x = (saturate((r0.wwww)*(source[3].yyyy))).x;
    // 12: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 13: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 14: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 15: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r2.xyz = (Read_EffectSceneColor(LinearClampUVSampler, (r1.xyxx).xy).xyzw).xyz;
    // 17: mad r3.xy, cb0[2].xxxx, v2.xyxx, cb0[2].yyyy
    r3.xy = ((source[2].xxxx)*(v2.xyxx)+(source[2].yyyy)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t2.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 19: mul r2.w, r1.w, r3.x
    r2.w = ((r1.wwww)*(r3.xxxx)).w;
    // 20: mad r3.xy, r2.wwww, cb0[2].zzzz, r1.xyxx
    r3.xy = ((r2.wwww)*(source[2].zzzz)+(r1.xyxx)).xy;
    // 21: mul r1.x, r1.w, v4.x
    r1.x = ((r1.wwww)*(v4.xxxx)).x;
    // 22: mul r1.x, r1.x, l(0.025000)
    r1.x = ((r1.xxxx)*(float4(0.025000,0.025000,0.025000,0.025000))).x;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t1.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 24: mov r5.xyz, r4.xyzx
    r5.xyz = (r4.xyzx).xyz;
    // 25: mov r1.y, l(0)
    r1.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 26: mov r1.w, r1.x
    r1.w = (r1.xxxx).w;
    // 27: loop
    [loop] while (true) {
    // 28: itof r2.w, r1.y
    r2.w = ((float4)(asint(r1.yyyy))).w;
    // 29: ge r3.w, r2.w, l(8.000000)
    r3.w = (asfloat((uint4)((r2.wwww)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).w;
    // 30: breakc_nz r3.w
    if ((asuint(r3.wwww)).x != 0u) break;
    // 31: mad r3.z, r1.w, cb0[3].x, r3.y
    r3.z = ((r1.wwww)*(source[3].xxxx)+(r3.yyyy)).z;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r3.xzxx, t1.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r6.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 33: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 34: mul r2.w, r1.x, r2.w
    r2.w = ((r1.xxxx)*(r2.wwww)).w;
    // 35: mad r1.w, r2.w, l(0.200000), r1.w
    r1.w = ((r2.wwww)*(float4(0.200000,0.200000,0.200000,0.200000))+(r1.wwww)).w;
    // 36: iadd r1.y, r1.y, l(1)
    r1.y = (asfloat(asuint(r1.yyyy) + uint4(1u,1u,1u,1u))).y;
    // 37: endloop
    }
    // 38: mul_sat r1.xyw, r5.xyxz, l(0.111111, 0.111111, 0.000000, 0.111111)
    r1.xyw = (saturate((r5.xyxz)*(float4(0.111111,0.111111,0.000000,0.111111)))).xyw;
    // 39: add r1.xyw, -r2.xyxz, r1.xyxw
    r1.xyw = ((-(r2.xyxz))+(r1.xyxw)).xyw;
    // 40: mad r1.xyw, r0.wwww, r1.xyxw, r2.xyxz
    r1.xyw = ((r0.wwww)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 41: mad r1.xyw, v3.xyxz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((v3.xyxz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 42: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ri_15_1_tr: a43001dc99c4224bbab23b82863a4bc7; selected map fc1a96a93c22b1b79c43c1891a18f4fcb5e4960a3b5ef2b34025936fa007d61f.
float4 ArtistNative3865(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].w = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[4].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[4].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[4].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[4].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: mad r0.xy, cb0[3].yyyy, l(0.002000, -0.002000, 0.000000, 0.000000), l(-1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((source[3].yyyy)*(float4(0.002000,-0.002000,0.000000,0.000000))+(float4(-1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 4: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 5: add r1.zw, r0.xxxy, r0.zzzw
    r1.zw = ((r0.xxxy)+(r0.zzzw)).zw;
    // 6: mov r2.yz, r1.wwzw
    r2.yz = (r1.wwzw).yz;
    // 7: add r2.xw, r1.zzzw, -cb0[3].xxxx
    r2.xw = ((r1.zzzw)+(-(source[3].xxxx))).xw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t0.zxyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).zxyw).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.zwzz, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 10: add r1.xy, r1.zwzz, cb0[3].xxxx
    r1.xy = ((r1.zwzz)+(source[3].xxxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r1.xwxx, t0.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zyzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.z = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).z;
    // 13: add r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)+(r1.wwww)).x;
    // 14: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 15: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 17: mov r2.yz, r1.yyxy
    r2.yz = (r1.yyxy).yz;
    // 18: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xwxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 20: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.zwzz, t0.zxyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).zxyw).x;
    // 23: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 24: add r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)+(r0.xxxx)).x;
    // 25: mul r1.z, r0.x, l(0.125000)
    r1.z = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 26: mad r2.xyzw, cb0[3].yyyy, l(-0.002000, 0.002000, 0.004000, 0.004000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r2.xyzw = ((source[3].yyyy)*(float4(-0.002000,0.002000,0.004000,0.004000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 27: mad r2.xyzw, r2.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r2.xyzw = ((r2.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 28: add r2.xyzw, r0.zzww, r2.zxyw
    r2.xyzw = ((r0.zzww)+(r2.zxyw)).xyzw;
    // 29: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.zwzz, t0.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.xyz = (Read_EffectSceneColor(LinearClampUVSampler, (r0.zwzz).xy).xyzw).xyz;
    // 30: mov r3.y, r2.z
    r3.y = (r2.zzzz).y;
    // 31: add r4.xyzw, r2.xyzw, cb0[3].xxxx
    r4.xyzw = ((r2.xyzw)+(source[3].xxxx)).xyzw;
    // 32: mov r3.x, r4.y
    r3.x = (r4.yyyy).x;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.xyxx, t0.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 34: mov r5.w, r3.x
    r5.w = (r3.xxxx).w;
    // 35: add r6.xyzw, r2.yzxw, -cb0[3].xxxx
    r6.xyzw = ((r2.yzxw)+(-(source[3].xxxx))).xyzw;
    // 36: mov r3.z, r6.x
    r3.z = (r6.xxxx).z;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.zyzz, t0.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 38: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 39: mov r5.x, r2.y
    r5.x = (r2.yyyy).x;
    // 40: mov r5.y, r4.z
    r5.y = (r4.zzzz).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r5.xyxx, t0.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 42: mov r3.w, r5.y
    r3.w = (r5.yyyy).w;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r3.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).x;
    // 44: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 45: mov r5.z, r6.y
    r5.z = (r6.yyyy).z;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r5.xzxx, t0.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r5.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r3.y, r5.wzww, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.y = (Read_EffectSceneColorBias(LinearClampUVSampler, (r5.wzww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 48: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.yzyy, t0.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r4.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 50: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r6.xyxx, t0.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 52: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 53: add r0.w, r3.x, r0.w
    r0.w = ((r3.xxxx)+(r0.wwww)).w;
    // 54: add r0.w, r3.y, r0.w
    r0.w = ((r3.yyyy)+(r0.wwww)).w;
    // 55: mul r1.x, r0.w, l(0.125000)
    r1.x = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 56: mov r4.y, r2.w
    r4.y = (r2.wwww).y;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r4.xyxx, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 58: mov r4.z, r6.z
    r4.z = (r6.zzzz).z;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.zyzz, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r4.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 60: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 61: mov r2.yw, r4.wwwx
    r2.yw = (r4.wwwx).yw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.xwxx, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r4.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 64: mov r4.w, r2.y
    r4.w = (r2.yyyy).w;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r4.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (Read_EffectSceneColorBias(LinearClampUVSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).y;
    // 66: add r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)+(r3.xxxx)).w;
    // 67: mov r2.z, r6.w
    r2.z = (r6.wwww).z;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r6.zwzz, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r6.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xzxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.wzww, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.wzww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).z;
    // 71: add r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)+(r2.xxxx)).w;
    // 72: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 73: add r0.w, r3.x, r0.w
    r0.w = ((r3.xxxx)+(r0.wwww)).w;
    // 74: add r0.w, r2.y, r0.w
    r0.w = ((r2.yyyy)+(r0.wwww)).w;
    // 75: add r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)+(r0.wwww)).w;
    // 76: mul r1.y, r0.w, l(0.125000)
    r1.y = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 77: add r0.xyz, -r0.xyzx, r1.xyzx
    r0.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 78: add r1.xy, v2.xyxx, cb0[2].xyxx
    r1.xy = ((v2.xyxx)+(source[2].xyxx)).xy;
    // 79: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 80: mad r0.xyz, r1.xyzx, l(0.000000, 0.050000, 0.100000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.000000,0.050000,0.100000,0.000000))+(r0.xyzx)).xyz;
    // 81: mul r0.xyz, r0.xyzx, v3.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)).xyz;
    // 82: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 83: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 84: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 85: mad r0.w, -r0.w, cb0[4].y, l(1.000000)
    r0.w = ((-(r0.wwww))*(source[4].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: mul_sat r0.w, r0.w, l(5.000000)
    r0.w = (saturate((r0.wwww)*(float4(5.000000,5.000000,5.000000,5.000000)))).w;
    // 87: mad r0.xyz, r0.wwww, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 88: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 89: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 90: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 91: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 92: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3865Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[2].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[2].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[2].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[2].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
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
    // 1: add r0.xy, v1.xyxx, cb0[0].xyxx
    r0.xy = ((v1.xyxx)+(source[0].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.yz, v1.xxyx, cb0[1].xxyx
    r0.yz = ((v1.xxyx)+(source[1].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 6: mad r0.x, r0.x, l(0.500000), r0.y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.yyyy)).x;
    // 7: add r0.yz, v1.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v1.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 8: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 9: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 10: mad r0.y, -r0.y, cb0[2].y, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[2].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 12: mov_sat r1.xz, r0.yyyy
    r1.xz = (saturate(r0.yyyy)).xz;
    // 13: mul r1.yw, r0.xxxx, r1.zzzz
    r1.yw = ((r0.xxxx)*(r1.zzzz)).yw;
    // 14: mad r0.xyzw, r1.xyzw, l(0.000000, -2.000000, 0.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r1.xyzw)*(float4(0.000000,-2.000000,0.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
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
    // 20: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
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
    // 33: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
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
// fx_d_pa_ring_07_53_ad: 9b521e0cc6316246aea52aeca9cdf714; selected map a72da109ae3925c166c4d0c5721aca2a65915ddc9ea7bdd97ae7e67d5b2149f1.
float4 ArtistNative3894(ARTIST_NATIVE_INPUT input)
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
    // 42: mad r0.yzw, cb0[3].zzzz, r1.xxyz, r0.xxyz
    r0.yzw = ((source[3].zzzz)*(r1.xxyz)+(r0.xxyz)).yzw;
    // 43: mul_sat r0.x, r0.x, cb0[3].w
    r0.x = (saturate((r0.xxxx)*(source[3].wwww))).x;
    // 44: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 45: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 46: mov_sat r1.x, v4.y
    r1.x = (saturate(v4.yyyy)).x;
    // 47: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: add r0.x, r0.x, -r1.x
    r0.x = ((r0.xxxx)+(-(r1.xxxx))).x;
    // 49: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 52: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 53: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 54: mul r1.x, r1.x, cb0[4].x
    r1.x = ((r1.xxxx)*(source[4].xxxx)).x;
    // 55: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 56: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 57: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 58: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 59: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 60: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3894Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_shockwave_01_13_ad: a8d5b0c827b1a6439fd1061829d908c1; selected map 6f2cfcb7b1c4708ee89fc0da76921944245c214f04c438122a0523cdeb299bc1.
float4 ArtistNative3895(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].x = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].y = (((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[7].z = (((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 28: mad r2.x, cb0[7].w, r1.x, cb0[4].x
    r2.x = ((source[7].wwww)*(r1.xxxx)+(source[4].xxxx)).x;
    // 29: mul r0.y, r1.x, cb0[6].y
    r0.y = ((r1.xxxx)*(source[6].yyyy)).y;
    // 30: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 31: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 32: mad r0.y, r0.y, l(0.500000), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 33: add r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 34: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 35: mul r0.zw, r0.zzzz, v4.wwwz
    r0.zw = ((r0.zzzz)*(v4.wwwz)).zw;
    // 36: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 37: lt r1.w, r0.x, l(0.000000)
    r1.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 38: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 40: movc r1.yz, r1.wwww, l(0,0,0,0), r0.zzwz
    r1.yz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzwz)).yz;
    // 41: mul r0.z, r1.z, cb0[8].x
    r0.z = ((r1.zzzz)*(source[8].xxxx)).z;
    // 42: mad r2.y, cb0[5].y, cb0[8].w, r0.z
    r2.y = ((source[5].yyyy)*(source[8].wwww)+(r0.zzzz)).y;
    // 43: mad r0.zw, r0.yyyy, cb0[6].zzzz, r2.xxxy
    r0.zw = ((r0.yyyy)*(source[6].zzzz)+(r2.xxxy)).zw;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
    // 45: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 46: add r0.w, -v4.x, l(1.000000)
    r0.w = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mad r0.z, r0.z, r0.x, -r0.w
    r0.z = ((r0.zzzz)*(r0.xxxx)+(-(r0.wwww))).z;
    // 48: mad r0.w, -r0.z, r0.x, l(1.000000)
    r0.w = ((-(r0.zzzz))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 50: mul_sat r0.x, r0.x, cb0[9].y
    r0.x = (saturate((r0.xxxx)*(source[9].yyyy))).x;
    // 51: log r0.z, |r0.w|
    r0.z = (log2(abs(r0.wwww))).z;
    // 52: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 53: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 54: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 55: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 56: mul r1.xz, r1.xxzx, cb0[5].zzwz
    r1.xz = ((r1.xxzx)*(source[5].zzwz)).xz;
    // 57: mad r2.x, cb0[5].y, cb0[5].x, r1.x
    r2.x = ((source[5].yyyy)*(source[5].xxxx)+(r1.xxxx)).x;
    // 58: mad r2.y, cb0[5].y, cb0[6].x, r1.z
    r2.y = ((source[5].yyyy)*(source[6].xxxx)+(r1.zzzz)).y;
    // 59: mad r0.yw, r0.yyyy, cb0[6].zzzz, r2.xxxy
    r0.yw = ((r0.yyyy)*(source[6].zzzz)+(r2.xxxy)).yw;
    // 60: sample_l_indexable(texture2d)(float,float,float,float) r1.xzw, r0.ywyy, t1.xwyz, s0, l(0.000000)
    r1.xzw = (ArtistNativeSample0((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xwyz).xzw;
    // 61: dp3 r0.y, r1.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 62: add r2.xyz, -r1.xzwx, r0.yyyy
    r2.xyz = ((-(r1.xzwx))+(r0.yyyy)).xyz;
    // 63: mad r1.xzw, cb0[6].wwww, r2.xxyz, r1.xxzw
    r1.xzw = ((source[6].wwww)*(r2.xxyz)+(r1.xxzw)).xzw;
    // 64: mul r1.xzw, r1.xxzw, cb0[2].xxyz
    r1.xzw = ((r1.xxzw)*(source[2].xxyz)).xzw;
    // 65: mad r2.xyz, r1.xzwx, cb0[3].xyzx, -r1.xzwx
    r2.xyz = ((r1.xzwx)*(source[3].xyzx)+(-(r1.xzwx))).xyz;
    // 66: mad r0.yzw, r0.zzzz, r2.xxyz, r1.xxzw
    r0.yzw = ((r0.zzzz)*(r2.xxyz)+(r1.xxzw)).yzw;
    // 67: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 68: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 69: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 70: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: mul r1.x, r1.x, cb0[9].z
    r1.x = ((r1.xxxx)*(source[9].zzzz)).x;
    // 72: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 73: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 74: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 75: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 76: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 77: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 78: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 79: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_frag_07_05_tr: f722f536b758064f82fd4ae5f7378dc3; selected map a2bd643b9498a4258b86d798be5c4afd49cbb3bcc877cc67546e3bec5ca5ed4b.
float4 ArtistNative3896(ARTIST_NATIVE_INPUT input)
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
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t2.xywz, s1, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 12: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 13: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 14: mul r1.x, r1.x, cb0[4].w
    r1.x = ((r1.xxxx)*(source[4].wwww)).x;
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
    // 22: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 24: movc o0.w, r0.z, l(0), r1.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 25: mul r1.xyz, r0.xywx, cb0[3].wwww
    r1.xyz = ((r0.xywx)*(source[3].wwww)).xyz;
    // 26: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 27: mad r0.xyz, -cb0[3].wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(source[3].wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 28: mad r0.xyz, cb0[4].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 29: mul r1.xyz, r0.xyzx, cb0[4].yyyy
    r1.xyz = ((r0.xyzx)*(source[4].yyyy)).xyz;
    // 30: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 31: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 32: mul r1.xyz, r1.xyzx, cb0[4].zzzz
    r1.xyz = ((r1.xyzx)*(source[4].zzzz)).xyz;
    // 33: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 34: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: add r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)+(r0.wwww)).xyz;
    // 36: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 37: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3896Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_me_atta_08_1_ad: f1910c31dcacfe4eb4e2639126daa457; selected map 5a16853721d6605cb8b03775f6690fde138e627576c7d21c918124b3b4c2897c.
float4 ArtistNative3897(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 6: mul r0.z, cb0[3].w, cb0[5].z
    r0.z = ((source[3].wwww)*(source[5].zzzz)).z;
    // 7: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: mad r0.y, cb0[4].x, cb0[3].x, l(-1.000000)
    r0.y = ((source[4].xxxx)*(source[3].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 11: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 12: mul r0.z, cb0[3].x, cb0[4].x
    r0.z = ((source[3].xxxx)*(source[4].xxxx)).z;
    // 13: mad r0.yz, r0.zzzz, v4.xxyx, -r0.yyyy
    r0.yz = ((r0.zzzz)*(v4.xxyx)+(-(r0.yyyy))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 15: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 16: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 19: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: movc r1.x, r1.y, l(0), |r1.x|
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).x;
    // 21: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 22: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 23: mul r1.y, r1.y, cb0[3].y
    r1.y = ((r1.yyyy)*(source[3].yyyy)).y;
    // 24: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 25: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 26: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 27: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 28: mul r1.xyz, r0.yzwy, cb0[4].yyyy
    r1.xyz = ((r0.yzwy)*(source[4].yyyy)).xyz;
    // 29: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: mad r0.yzw, -cb0[4].yyyy, r0.yyzw, r1.wwww
    r0.yzw = ((-(source[4].yyyy))*(r0.yyzw)+(r1.wwww)).yzw;
    // 31: mad r0.yzw, cb0[4].zzzz, r0.yyzw, r1.xxyz
    r0.yzw = ((source[4].zzzz)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 32: mul r1.xyz, r0.yzwy, cb0[4].wwww
    r1.xyz = ((r0.yzwy)*(source[4].wwww)).xyz;
    // 33: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 34: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[5].xxxx
    r1.xyz = ((r1.xyzx)*(source[5].xxxx)).xyz;
    // 36: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 37: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 38: add r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = ((r0.yyzw)+(r1.xxxx)).yzw;
    // 39: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
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
float4 ArtistNative3897Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_ring_07_76_ts_ad: 61a90418cd1e884bb1e86a159d34d7ef; selected map 044f3ccb3783cb23e1cd753a534ed95b408239f972e28760c9ec560bf86c0ac8.
float4 ArtistNative3898(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
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
    // 35: mul r0.xy, r1.xyxx, cb0[3].xyxx
    r0.xy = ((r1.xyxx)*(source[3].xyxx)).xy;
    // 36: mad r1.x, cb0[2].w, cb0[2].z, r0.x
    r1.x = ((source[2].wwww)*(source[2].zzzz)+(r0.xxxx)).x;
    // 37: mad r1.y, cb0[2].w, cb0[3].z, r0.y
    r1.y = ((source[2].wwww)*(source[3].zzzz)+(r0.yyyy)).y;
    // 38: mul r0.xy, v2.xyxx, cb0[4].xyxx
    r0.xy = ((v2.xyxx)*(source[4].xyxx)).xy;
    // 39: mad r2.x, cb0[2].w, cb0[3].w, r0.x
    r2.x = ((source[2].wwww)*(source[3].wwww)+(r0.xxxx)).x;
    // 40: mad r2.y, cb0[2].w, cb0[4].z, r0.y
    r2.y = ((source[2].wwww)*(source[4].zzzz)+(r0.yyyy)).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 42: mul r0.y, v4.x, cb0[4].w
    r0.y = ((v4.xxxx)*(source[4].wwww)).y;
    // 43: mad r0.zw, r0.xxxx, r0.yyyy, r1.xxxy
    r0.zw = ((r0.xxxx)*(r0.yyyy)+(r1.xxxy)).zw;
    // 44: mad r0.xy, r0.xxxx, r0.yyyy, v2.xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(v2.xyxx)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t1.yxzw, s0, cb0[2].y
    r0.y = (ArtistNativeSample0((r0.zwzz).xy, (source[2].yyyy).x, true).yxzw).y;
    // 47: mul_sat r0.y, r0.y, cb0[5].x
    r0.y = (saturate((r0.yyyy)*(source[5].xxxx))).y;
    // 48: mov_sat r0.z, v4.y
    r0.z = (saturate(v4.yyyy)).z;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 51: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 52: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 53: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 54: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 56: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 57: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 58: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 59: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 60: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 61: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 62: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 64: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 65: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 66: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 67: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 68: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 69: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 70: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 71: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 72: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3898Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_d_pa_dust_02_01_tr: 21310dd53f047b4c9c6d215723e659ae; selected map 9a5065a684482b39573cad842bafa7252825dd4ce943af6e75781d622d09ea24.
float4 ArtistNative3899(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[7]=float4(input.skyUpperColor,0.f);
    source[8]=float4(input.skyLowerColor,0.f);
    source[9]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].wwww,g_ArtistSourceMaterialParameters[2u].xxxx,1u);
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v8 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v9 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mad r0.xy, cb0[4].xxxx, cb0[2].xyxx, v2.xyxx
    r0.xy = ((source[4].xxxx)*(source[2].xyxx)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[4].wwww
    r0.xy = ((r0.xyxx)*(source[4].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[4].xxxx, cb0[3].xxxy, v2.xxxy
    r0.zw = ((source[4].xxxx)*(source[3].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[5].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 7: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 8: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 9: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 10: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 11: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 12: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 14: movc o0.w, r0.w, l(0), r1.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 15: mul r1.xyz, r0.xyzx, cb0[5].wwww
    r1.xyz = ((r0.xyzx)*(source[5].wwww)).xyz;
    // 16: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: mad r0.xyz, -cb0[5].wwww, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[5].wwww))*(r0.xyzx)+(r0.wwww)).xyz;
    // 18: mad r0.xyz, cb0[6].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[6].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 19: mul r0.xyz, r0.xyzx, v3.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)).xyz;
    // 20: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 21: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 22: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 23: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 24: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 26: mul r1.yzw, r1.yyyy, cb0[8].xxyz
    r1.yzw = ((r1.yyyy)*(source[8].xxyz)).yzw;
    // 27: mad r1.xyz, r1.xxxx, cb0[7].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[7].xyzx)+(r1.yzwy)).xyz;
    // 28: mul r1.xyz, r1.xyzx, cb0[9].wwww
    r1.xyz = ((r1.xyzx)*(source[9].wwww)).xyz;
    // 29: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 30: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 32: mad r1.xyz, r0.xyzx, cb0[9].xyzx, r2.xyzx
    r1.xyz = ((r0.xyzx)*(source[9].xyzx)+(r2.xyzx)).xyz;
    // 34: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_a_pa_dust_02_tr: 5a1005e5f3036e41b690c9f170421d47; selected map bf206e2a2cb40857c163513f1c55e0cae8fc369ca84488421942436ad5f88576.
float4 ArtistNative3900(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.119999997, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0599999987, 0.0, 0.0, 0.0))),1u);
    source[4].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
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
    // 10: mul_sat r0.x, r0.x, l(0.040000)
    r0.x = (saturate((r0.xxxx)*(float4(0.040000,0.040000,0.040000,0.040000)))).x;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t0.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 14: mad r0.y, v4.x, r0.y, r0.z
    r0.y = ((v4.xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 15: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 16: mul_sat r0.y, r0.y, l(3.500000)
    r0.y = (saturate((r0.yyyy)*(float4(3.500000,3.500000,3.500000,3.500000)))).y;
    // 17: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 18: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 19: mad r0.xy, v2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 21: mad r0.yz, v2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000), cb0[3].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,0.500000,0.500000,0.000000))+(source[3].xxyx)).yz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 23: mad r0.xy, r0.xxxx, r0.yyyy, v2.xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(v2.xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 25: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 27: mad r0.xyz, cb0[4].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[4].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 28: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 29: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_db_02_1_tr: 9e4781052a66534b9646434db8d482db; selected map 4bdc6bcaf84fc1de676b6c243995d7aec03cd12fbb6a44cdf8836763e5c834ca.
float4 ArtistNative3901(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_gibs_stone_m: 30019952641d6c4aa3c4b808a72336b5; selected map 9cadd2253a547309e14d44aac76a80a5c46a6a08e08ffe971a0cc4d1916979cd.
float4 ArtistNative3902(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = g_ArtistSourceMaterialParameters[1u];
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (clamp(g_ArtistSourceMaterialParameters[0u].xxxx,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mad r0.y, v1.z, r0.x, l(1.000000)
    r0.y = ((v1.zzzz)*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 4: mul r0.xzw, r0.xxxx, v1.xxyz
    r0.xzw = ((r0.xxxx)*(v1.xxyz)).xzw;
    // 5: mul_sat r0.y, r0.y, l(0.500000)
    r0.y = (saturate((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000)))).y;
    // 6: mul r1.x, r0.y, r0.y
    r1.x = ((r0.yyyy)*(r0.yyyy)).x;
    // 7: mul r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)*(r1.xxxx)).x;
    // 8: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 9: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 10: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 11: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 12: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 13: mul r2.xyz, r0.wxzw, r1.yzxy
    r2.xyz = ((r0.wxzw)*(r1.yzxy)).xyz;
    // 14: mad r2.xyz, r0.zwxz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwxz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 15: mul r2.xyz, r2.xzyx, v1.wwww
    r2.xyz = ((r2.xzyx)*(v1.wwww)).xyz;
    // 16: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 17: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 18: mul r3.xyz, r1.wwww, v5.xyzx
    r3.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 19: mad r4.xyz, r3.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r3.xyzx
    r4.xyz = ((r3.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 20: dp3 r5.y, r2.xzyx, r4.xyzx
    r5.y = (dot((r2.xzyx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 21: dp3 r5.z, r0.xzwx, r4.xyzx
    r5.z = (dot((r0.xzwx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 22: mov r2.z, r0.w
    r2.z = (r0.wwww).z;
    // 23: dp3 r5.x, r1.xyzx, r4.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 24: mov r2.x, r1.z
    r2.x = (r1.zzzz).x;
    // 25: dp3_sat r0.x, r5.xyzx, r2.xyzx
    r0.x = (saturate(dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // 26: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 27: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 29: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 30: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 33: add r4.xyz, -r1.xyzx, r0.zzzz
    r4.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 34: mad r1.xyz, cb0[4].yyyy, r4.xyzx, r1.xyzx
    r1.xyz = ((source[4].yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[4].wwww
    r1.xyz = ((r1.xyzx)*(source[4].wwww)).xyz;
    // 36: mul_sat r0.xzw, r0.xxxx, r1.xxyz
    r0.xzw = (saturate((r0.xxxx)*(r1.xxyz))).xzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyz = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 38: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: add r4.xyz, -r1.xyzx, r1.wwww
    r4.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 40: mad r1.xyz, cb0[4].yyyy, r4.xyzx, r1.xyzx
    r1.xyz = ((source[4].yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 41: mul r4.xyz, r1.xyzx, cb0[2].xyzx
    r4.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 42: mul r4.xyz, r4.xyzx, cb0[0].xyzx
    r4.xyz = ((r4.xyzx)*(source[0].xyzx)).xyz;
    // 43: mad r0.xyz, r0.yyyy, r4.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r4.xyzx)+(r0.xzwx)).xyz;
    // 44: mov r1.w, l(1.000000)
    r1.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 45: dp3 r0.w, r1.xywx, |r3.xyzx|
    r0.w = (dot((r1.xywx).xyz,(abs(r3.xyzx)).xyz).xxxx).w;
    // 46: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 48: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 49: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 50: add o0.xyz, r0.xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 51: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_me_ht_07_1_tr: 63b3600fc7b02a43999ea2a77bb81937; selected map 3d5aabf61e0ea070933600b4e9aeed4ce23e68cbe558ae68926cd5282fd7d629.
float4 ArtistNative3903(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[7]=float4(input.skyUpperColor,0.f);
    source[8]=float4(input.skyLowerColor,0.f);
    source[9]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = g_ArtistSourceMaterialParameters[3u];
    source[4] = g_ArtistSourceMaterialParameters[1u];
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]; [unroll] for(uint passIndex=0u;passIndex<5u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    passValues[3]=float4(0.f,0.f,0.f,1.f); // Neutral source diffuse override: preserve material color.
    passValues[4]=float4(0.f,0.f,0.f,1.f); // Neutral specular override; only the original secondary MRT consumes it.
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: mul r1.xy, v4.xyxx, cb0[5].xxxx
    r1.xy = ((v4.xyxx)*(source[5].xxxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 7: mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 9: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 11: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 12: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 14: rsq r1.w, r0.w
    r1.w = (rsqrt(r0.wwww)).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: div r3.xyz, r1.xyzx, r0.wwww
    r3.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 17: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 18: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 19: mul r4.xyz, r0.wwww, r1.xyzx
    r4.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 20: mad r0.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 21: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 22: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 23: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 24: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r5.xyz, r0.wwww, v0.xyzx
    r5.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 27: mul r6.xyz, r4.zxyz, r5.yzxy
    r6.xyz = ((r4.zxyz)*(r5.yzxy)).xyz;
    // 28: mad r6.xyz, r4.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r4.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 29: mul r6.xyz, r6.xzyx, v1.wwww
    r6.xyz = ((r6.xzyx)*(v1.wwww)).xyz;
    // 30: dp3 r7.y, r6.xzyx, r0.xyzx
    r7.y = (dot((r6.xzyx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 31: dp3 r7.x, r5.xyzx, r0.xyzx
    r7.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: dp3 r7.z, r4.xyzx, r0.xyzx
    r7.z = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 33: dp3 r0.y, r6.xzyx, r1.xyzx
    r0.y = (dot((r6.xzyx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 34: mov r6.x, r5.z
    r6.x = (r5.zzzz).x;
    // 35: dp3 r0.x, r5.xyzx, r1.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 36: mov r6.z, r4.z
    r6.z = (r4.zzzz).z;
    // 37: dp3 r0.z, r4.xyzx, r1.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 38: dp3_sat r0.w, r7.xyzx, r6.xyzx
    r0.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 39: dp3_sat r1.w, r3.xyzx, r6.xyzx
    r1.w = (saturate(dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 40: mul r3.x, r1.w, l(10.000000)
    r3.x = ((r1.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 41: mul r3.yzw, r1.wwww, cb0[4].xxyz
    r3.yzw = ((r1.wwww)*(source[4].xxyz)).yzw;
    // 42: min r1.w, r3.x, l(1.000000)
    r1.w = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 44: log r1.w, r0.w
    r1.w = (log2(r0.wwww)).w;
    // 45: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: mul r1.w, r1.w, cb0[5].y
    r1.w = ((r1.wwww)*(source[5].yyyy)).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // 49: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 51: mul r3.xyz, r3.yzwy, r2.xyzx
    r3.xyz = ((r3.yzwy)*(r2.xyzx)).xyz;
    // 52: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 53: mad r2.xyz, r4.xyzx, r2.xyzx, r3.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 54: mad r2.xyz, r2.wwww, r0.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r0.wwww)+(r2.xyzx)).xyz;
    // 55: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 56: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 57: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 58: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 59: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 60: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 61: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 62: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 63: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 64: mul r1.yzw, r1.yyyy, cb0[8].xxyz
    r1.yzw = ((r1.yyyy)*(source[8].xxyz)).yzw;
    // 65: mad r1.xyz, r1.xxxx, cb0[7].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[7].xyzx)+(r1.yzwy)).xyz;
    // 66: mul r1.xyz, r1.xyzx, cb0[9].wwww
    r1.xyz = ((r1.xyzx)*(source[9].wwww)).xyz;
    // 67: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[2].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[2].xyzx)).xyz;
    // 68: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 70: mad r1.xyz, r2.xyzx, cb0[9].xyzx, r3.xyzx
    r1.xyz = ((r2.xyzx)*(source[9].xyzx)+(r3.xyzx)).xyz;
    // 72: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 73: mov_sat r0.w, cb0[1].w
    r0.w = (saturate(source[1].wwww)).w;
    // 74: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// Installed source-identical native reuse
float4 ArtistNative3876(ARTIST_NATIVE_INPUT input)
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
    // 53: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 54: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 55: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// Installed source-identical native reuse
float4 ArtistNative3856(ARTIST_NATIVE_INPUT input)
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
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4] = g_ArtistSourceMaterialParameters[3u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[6] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].xxxx,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 11: mul r0.xy, v4.xyxx, cb0[9].xxxx
    r0.xy = ((v4.xyxx)*(source[9].xxxx)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 13: mov_sat r0.y, cb0[1].w
    r0.y = (saturate(source[1].wwww)).y;
    // 14: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 17: add r0.y, r0.y, -cb0[7].y
    r0.y = ((r0.yyyy)+(-(source[7].yyyy))).y;
    // 18: mul r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)*(source[7].zzzz)).y;
    // 19: mad r0.y, r0.y, l(0.050000), l(-0.025000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).y;
    // 20: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 21: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 22: mul r0.zw, r0.zzzz, v6.xxxy
    r0.zw = ((r0.zzzz)*(v6.xxxy)).zw;
    // 23: mad_sat r1.xy, r0.yyyy, r0.zwzz, v4.xyxx
    r1.xy = (saturate((r0.yyyy)*(r0.zwzz)+(v4.xyxx))).xy;
    // 24: mul r0.yz, r0.zzwz, r0.yyyy
    r0.yz = ((r0.zzwz)*(r0.yyyy)).yz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 26: mul r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)*(source[8].zzzz)).w;
    // 27: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 28: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 29: mul r1.x, r1.x, cb0[8].w
    r1.x = ((r1.xxxx)*(source[8].wwww)).x;
    // 30: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 31: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 32: mad r0.x, -r0.x, l(10.000000), r0.w
    r0.x = ((-(r0.xxxx))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.wwww)).x;
    // 33: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 34: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 35: add r0.w, -|v4.w|, cb0[0].y
    r0.w = ((-(abs(v4.wwww)))+(source[0].yyyy)).w;
    // 36: mul r0.w, r0.w, l(5.000000)
    r0.w = ((r0.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 37: div_sat r0.w, r0.w, cb0[0].y
    r0.w = (saturate((r0.wwww)/(source[0].yyyy))).w;
    // 38: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 39: mul o0.w, r0.w, r0.x
    output.w = ((r0.wwww)*(r0.xxxx)).w;
    // 40: mad r0.xw, v4.xxxy, cb0[5].xxxy, r0.yyyz
    r0.xw = ((v4.xxxy)*(source[5].xxxy)+(r0.yyyz)).xw;
    // 41: mad r0.yz, cb0[7].xxxx, v4.xxyx, r0.yyzy
    r0.yz = ((source[7].xxxx)*(v4.xxyx)+(r0.yyzy)).yz;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t3.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xwxx, t4.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample2((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 44: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 46: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 48: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 49: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 50: dp3 r0.x, r0.xyzx, r0.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 51: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 52: div r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)/(r0.xxxx)).x;
    // 53: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 54: max r0.x, r0.x, l(0.150000)
    r0.x = (max(r0.xxxx,float4(0.150000,0.150000,0.150000,0.150000))).x;
    // 55: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 56: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 57: add r0.yzw, -r1.xxyz, r0.yyyy
    r0.yzw = ((-(r1.xxyz))+(r0.yyyy)).yzw;
    // 58: mad r0.yzw, cb0[7].wwww, r0.yyzw, r1.xxyz
    r0.yzw = ((source[7].wwww)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 59: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 60: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 61: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 62: mad r1.xyz, r0.xyzx, l(0.150000, 0.150000, 0.150000, 0.000000), cb0[3].xyzx
    r1.xyz = ((r0.xyzx)*(float4(0.150000,0.150000,0.150000,0.000000))+(source[3].xyzx)).xyz;
    // 63: mul r0.xyz, r0.xyzx, cb2[3].wwww
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)).xyz;
    // 64: mad r0.xyz, r0.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 65: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 68: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 69: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 70: mul r2.yzw, r2.yyyy, cb0[13].xxyz
    r2.yzw = ((r2.yyyy)*(source[13].xxyz)).yzw;
    // 71: mad r2.xyz, r2.xxxx, cb0[12].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[12].xyzx)+(r2.yzwy)).xyz;
    // 72: mul r2.xyz, r2.xyzx, cb0[14].wwww
    r2.xyz = ((r2.xyzx)*(source[14].wwww)).xyz;
    // 73: mad r1.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 74: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 76: mad r1.xyz, r0.xyzx, cb0[14].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[14].xyzx)+(r1.xyzx)).xyz;
    // 78: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif
