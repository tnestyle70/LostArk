// Original Kouku material programs 4864..4927; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_me_leaf_01_01_ts_tr: de3a8243adf7524db7f4c6682b2cadb8; selected map d0dbd0145613bb03c353b80c81b76e38e4a70980e6b0dba4bfc9769d5e7d7ed6.
float4 ArtistNative4900(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = g_ArtistSourceMaterialParameters[3u];
    source[4] = g_ArtistSourceMaterialParameters[4u];
    source[5] = input.dynamicParameter;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.xy, v4.xyxx, cb0[4].xyxx, cb0[3].xyxx
    r0.xy = ((v4.xyxx)*(source[4].xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 3: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 4: mul r1.x, r1.x, cb0[6].z
    r1.x = ((r1.xxxx)*(source[6].zzzz)).x;
    // 5: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 6: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 7: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 8: movc r0.w, r0.w, l(0), |r1.x|
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).w;
    // 9: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 10: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 11: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 12: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 13: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 14: movc o0.w, r0.w, l(0), r1.x
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 15: mul r1.xyz, r0.xyzx, cb0[6].xxxx
    r1.xyz = ((r0.xyzx)*(source[6].xxxx)).xyz;
    // 16: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: mad r0.xyz, -cb0[6].xxxx, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[6].xxxx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 18: mad r0.xyz, cb0[6].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[6].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 19: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 20: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4900Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_mm_onelayerdistortion_02_01_ad: eb2bcd5c8f3c6c49805ab689687b14f6; selected map 5cdc3f3921ce9c64ed1a2b7836cd323203952b794cb58c5f718e69ddfab95aee.
float4 ArtistNative4901(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[10].x=0.f; // Existing SDNative374 project neutral scene attenuation; NOT a recovered source default.
    // Zero distortion/emission must preserve SceneColor: Scene*(1-x)=Scene requires x=0.
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))),1u);
    source[6] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[7].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0))))).x;
    source[7].y = (cos((g_ArtistSourceMaterialTime.xxxx*float4(0.699999988, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].w = (sin((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[8].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))))).x;
    source[8].y = (cos((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[8].w = ((g_ArtistSourceMaterialParameters[0u].wwww*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].y = ((float4(10.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[0u].zzzz)).x;
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
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 5: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 6: div r0.zw, r1.xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)/(r0.zzzz)).zw;
    // 7: mad r1.x, |r0.z|, l(-0.018729), l(0.074261)
    r1.x = ((abs(r0.zzzz))*(float4(-0.018729,-0.018729,-0.018729,-0.018729))+(float4(0.074261,0.074261,0.074261,0.074261))).x;
    // 8: mad r1.x, r1.x, |r0.z|, l(-0.212114)
    r1.x = ((r1.xxxx)*(abs(r0.zzzz))+(float4(-0.212114,-0.212114,-0.212114,-0.212114))).x;
    // 9: mad r1.x, r1.x, |r0.z|, l(1.570729)
    r1.x = ((r1.xxxx)*(abs(r0.zzzz))+(float4(1.570729,1.570729,1.570729,1.570729))).x;
    // 10: add r1.y, -|r0.z|, l(1.000000)
    r1.y = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 12: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 13: mad r1.z, r1.z, l(-2.000000), l(3.141593)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 14: lt r0.z, r0.z, -r0.z
    r0.z = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).z;
    // 15: and r0.z, r0.z, r1.z
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r1.zzzz))).z;
    // 16: mad r0.z, r1.x, r1.y, r0.z
    r0.z = ((r1.xxxx)*(r1.yyyy)+(r0.zzzz)).z;
    // 17: add r1.x, -r0.z, l(1.570796)
    r1.x = ((-(r0.zzzz))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 18: mad r0.z, r0.z, l(0.318310), l(1.000000)
    r0.z = ((r0.zzzz)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 19: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 20: mad r1.x, r1.x, l(0.636620), l(1.000000)
    r1.x = ((r1.xxxx)*(float4(0.636620,0.636620,0.636620,0.636620))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 21: mul r1.x, r1.x, l(0.250000)
    r1.x = ((r1.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 22: lt r1.y, l(0.000000), r0.w
    r1.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(r0.wwww)) * 0xffffffffu)).y;
    // 23: ge r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 24: movc r1.x, r1.y, r1.x, r0.z
    r1.x = ((asuint(r1.yyyy) != 0u) ? (r1.xxxx) : (r0.zzzz)).x;
    // 25: movc r0.z, r0.w, r1.x, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (r1.xxxx) : (r0.zzzz)).z;
    // 26: mul r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)*(source[7].zzzz)).z;
    // 27: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 28: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 29: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 30: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 31: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 32: div r1.xy, r1.xyxx, r0.wwww
    r1.xy = ((r1.xyxx)/(r0.wwww)).xy;
    // 33: mad r0.w, |r1.x|, l(-0.018729), l(0.074261)
    r0.w = ((abs(r1.xxxx))*(float4(-0.018729,-0.018729,-0.018729,-0.018729))+(float4(0.074261,0.074261,0.074261,0.074261))).w;
    // 34: mad r0.w, r0.w, |r1.x|, l(-0.212114)
    r0.w = ((r0.wwww)*(abs(r1.xxxx))+(float4(-0.212114,-0.212114,-0.212114,-0.212114))).w;
    // 35: mad r0.w, r0.w, |r1.x|, l(1.570729)
    r0.w = ((r0.wwww)*(abs(r1.xxxx))+(float4(1.570729,1.570729,1.570729,1.570729))).w;
    // 36: add r1.z, -|r1.x|, l(1.000000)
    r1.z = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: mul r1.w, r0.w, r1.z
    r1.w = ((r0.wwww)*(r1.zzzz)).w;
    // 39: mad r1.w, r1.w, l(-2.000000), l(3.141593)
    r1.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 40: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 41: and r1.x, r1.x, r1.w
    r1.x = (asfloat(asuint(r1.xxxx) & asuint(r1.wwww))).x;
    // 42: mad r0.w, r0.w, r1.z, r1.x
    r0.w = ((r0.wwww)*(r1.zzzz)+(r1.xxxx)).w;
    // 43: add r1.x, -r0.w, l(1.570796)
    r1.x = ((-(r0.wwww))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 44: mad r0.w, r0.w, l(0.318310), l(1.000000)
    r0.w = ((r0.wwww)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 46: mad r1.x, r1.x, l(0.636620), l(1.000000)
    r1.x = ((r1.xxxx)*(float4(0.636620,0.636620,0.636620,0.636620))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mul r1.x, r1.x, l(0.250000)
    r1.x = ((r1.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 48: lt r1.z, l(0.000000), r1.y
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(r1.yyyy)) * 0xffffffffu)).z;
    // 49: ge r1.y, r1.y, l(0.000000)
    r1.y = (asfloat((uint4)((r1.yyyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 50: movc r1.x, r1.z, r1.x, r0.w
    r1.x = ((asuint(r1.zzzz) != 0u) ? (r1.xxxx) : (r0.wwww)).x;
    // 51: movc r0.w, r1.y, r1.x, r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (r1.xxxx) : (r0.wwww)).w;
    // 52: mul r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)*(source[8].zzzz)).w;
    // 53: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 54: sincos r0.zw, null, r0.zzzw
    { const float4 sourceAngle = r0.zzzw; r0.zw = (sin(sourceAngle)).zw; }
    // 55: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 56: mul r0.z, r0.z, cb0[8].w
    r0.z = ((r0.zzzz)*(source[8].wwww)).z;
    // 57: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 58: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 60: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 61: mul r1.x, r1.x, l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 62: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 63: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 64: add r0.w, r0.w, v4.x
    r0.w = ((r0.wwww)+(v4.xxxx)).w;
    // 65: add r0.w, r0.w, l(-0.500000)
    r0.w = ((r0.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 66: mad r0.z, r0.z, l(0.500000), r0.w
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).z;
    // 67: mul_sat r0.z, |r0.z|, cb0[9].y
    r0.z = (saturate((abs(r0.zzzz))*(source[9].yyyy))).z;
    // 68: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: mad r1.xyz, v4.xxxx, l(0.500000, 0.500000, 0.500000, 0.000000), l(0.200000, 0.170000, 0.300000, 0.000000)
    r1.xyz = ((v4.xxxx)*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(0.200000,0.170000,0.300000,0.000000))).xyz;
    // 70: max r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 71: div r1.xyz, l(1.000000, 1.000000, 1.000000, 1.000000), r1.xyzx
    r1.xyz = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xyzx)).xyz;
    // 72: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 73: mul r0.xy, r0.xyxx, l(1.700000, 1.700000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(1.700000,1.700000,0.000000,0.000000))).xy;
    // 74: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 75: mad r1.xyz, -r0.wwww, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.wwww))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mad r0.w, -r0.w, l(2.000000), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: add_sat r0.w, r0.w, r0.w
    r0.w = (saturate((r0.wwww)+(r0.wwww))).w;
    // 78: max r1.yz, r1.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (max(r1.yyzy,float4(0.000000,0.000000,0.000000,0.000000))).yz;
    // 79: mul_sat r1.x, r1.x, l(5.000000)
    r1.x = (saturate((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 80: lt r1.w, l(0.000000), r1.y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(r1.yyyy)) * 0xffffffffu)).w;
    // 81: movc r0.z, r1.w, l(0), r0.z
    r0.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 82: rsq r1.w, r1.z
    r1.w = (rsqrt(r1.zzzz)).w;
    // 83: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 84: mul r1.w, r1.w, v4.w
    r1.w = ((r1.wwww)*(v4.wwww)).w;
    // 85: lt r1.z, r1.z, l(0.000001)
    r1.z = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 86: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 87: movc r1.z, r1.z, l(-0.000000), -r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.wwww))).z;
    // 88: add r0.z, r0.z, r1.z
    r0.z = ((r0.zzzz)+(r1.zzzz)).z;
    // 89: max r1.z, |r0.y|, |r0.x|
    r1.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 90: div r1.z, l(1.000000, 1.000000, 1.000000, 1.000000), r1.z
    r1.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.zzzz)).z;
    // 91: min r1.w, |r0.y|, |r0.x|
    r1.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 92: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 93: mul r1.w, r1.z, r1.z
    r1.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 94: mad r2.x, r1.w, l(0.020835), l(-0.085133)
    r2.x = ((r1.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 95: mad r2.x, r1.w, r2.x, l(0.180141)
    r2.x = ((r1.wwww)*(r2.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 96: mad r2.x, r1.w, r2.x, l(-0.330299)
    r2.x = ((r1.wwww)*(r2.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 97: mad r1.w, r1.w, r2.x, l(0.999866)
    r1.w = ((r1.wwww)*(r2.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 98: mul r2.x, r1.w, r1.z
    r2.x = ((r1.wwww)*(r1.zzzz)).x;
    // 99: mad r2.x, r2.x, l(-2.000000), l(1.570796)
    r2.x = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 100: lt r2.y, |r0.y|, |r0.x|
    r2.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 101: and r2.x, r2.y, r2.x
    r2.x = (asfloat(asuint(r2.yyyy) & asuint(r2.xxxx))).x;
    // 102: mad r1.z, r1.z, r1.w, r2.x
    r1.z = ((r1.zzzz)*(r1.wwww)+(r2.xxxx)).z;
    // 103: lt r1.w, r0.y, -r0.y
    r1.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 104: and r1.w, r1.w, l(0xc0490fdb)
    r1.w = (asfloat(asuint(r1.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 105: add r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)+(r1.zzzz)).z;
    // 106: min r1.w, r0.y, r0.x
    r1.w = (min(r0.yyyy,r0.xxxx)).w;
    // 107: lt r1.w, r1.w, -r1.w
    r1.w = (asfloat((uint4)((r1.wwww)<(-(r1.wwww))) * 0xffffffffu)).w;
    // 108: max r2.x, r0.y, r0.x
    r2.x = (max(r0.yyyy,r0.xxxx)).x;
    // 109: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 110: ge r0.y, r2.x, -r2.x
    r0.y = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).y;
    // 111: and r0.y, r0.y, r1.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r1.wwww))).y;
    // 112: movc r0.y, r0.y, -r1.z, r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r1.zzzz)) : (r1.zzzz)).y;
    // 113: mad r0.y, r0.y, l(0.636620), l(1.000000)
    r0.y = ((r0.yyyy)*(float4(0.636620,0.636620,0.636620,0.636620))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 114: mul r2.x, r0.y, l(0.500000)
    r2.x = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 115: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 117: movc r2.y, r0.y, l(0), r0.x
    r2.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).y;
    // 118: add r0.xy, r2.xyxx, cb0[2].xyxx
    r0.xy = ((r2.xyxx)+(source[2].xyxx)).xy;
    // 119: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 120: mul r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)*(r0.xxxx)).x;
    // 121: mad r0.x, r0.x, r1.y, r0.z
    r0.x = ((r0.xxxx)*(r1.yyyy)+(r0.zzzz)).x;
    // 122: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 123: mul r0.x, r0.x, -v4.z
    r0.x = ((r0.xxxx)*(-(v4.zzzz))).x;
    // 124: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 125: div r0.y, l(1536.000000), v7.z
    r0.y = ((float4(1536.000000,1536.000000,1536.000000,1536.000000))/(v7.zzzz)).y;
    // 126: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 127: mov r0.x, l(-64.000000)
    r0.x = (float4(-64.000000,-64.000000,-64.000000,-64.000000)).x;
    // 128: max r0.xy, r0.xyxx, l(0.000000, -64.000000, 0.000000, 0.000000)
    r0.xy = (max(r0.xyxx,float4(0.000000,-64.000000,0.000000,0.000000))).xy;
    // 129: min r0.xy, r0.xyxx, l(64.000000, 64.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(64.000000,64.000000,0.000000,0.000000))).xy;
    // 130: mad r0.xy, r0.xyxx, l(0.007843, 0.007843, 0.000000, 0.000000), l(-1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.007843,0.007843,0.000000,0.000000))+(float4(-1.000000,1.000000,0.000000,0.000000))).xy;
    // 131: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 132: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 133: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 134: add r0.xy, r0.xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)+(r0.zwzz)).xy;
    // 135: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.xyz = (Read_EffectSceneColor(LinearClampUVSampler, (r0.xyxx).xy).xyzw).xyz;
    // 136: add r0.w, -cb0[10].x, l(1.000000)
    r0.w = ((-(source[10].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 138: mul r1.xyz, v5.wwww, cb0[1].xyzx
    r1.xyz = ((v5.wwww)*(source[1].xyzx)).xyz;
    // 139: mad o0.xyz, r1.xyzx, cb0[0].xxxx, r0.xyzx
    output.xyz = ((r1.xyzx)*(source[0].xxxx)+(r0.xyzx)).xyz;
    // 140: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_hologram_01_01_tr: e0b5fe98a973904985c18256b8eb8f8b; selected map 9cbd4e991b8518651a155d9a17d7f315189883f1c0cd060f1d3b6ce469d7dc06.
float4 ArtistNative4902(ARTIST_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f); // Absolute source world origin and original opacity W.
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].zzzz,1u);
    source[6] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].yyyy)*float4(-5.0, 0.0, 0.0, 0.0))),1u);
    source[8].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[8].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[8].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[9].y = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[9].z = (sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].x = (sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[11].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].yyyy)).x;
    source[12].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[12].w = ((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialParameters[4u].wwww)).x;
    source[13].x = ((sin(((((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialTime.xxxx)/g_ArtistSourceMaterialParameters[4u].wwww)+g_ArtistSourceMaterialParameters[4u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*g_ArtistSourceMaterialParameters[4u].wwww)).x;
    source[13].y = (sin(((((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialParameters[4u].wwww)/(g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialParameters[4u].wwww))+(((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialTime.xxxx)/g_ArtistSourceMaterialParameters[4u].wwww)+g_ArtistSourceMaterialParameters[4u].wwww))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[13].z = ((sin(((((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialParameters[4u].wwww)/(g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialParameters[4u].wwww))+(((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialTime.xxxx)/g_ArtistSourceMaterialParameters[4u].wwww)+g_ArtistSourceMaterialParameters[4u].wwww))*float4(6.28318548, 0.0, 0.0, 0.0)))*(sin(((((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialTime.xxxx)/g_ArtistSourceMaterialParameters[4u].wwww)+g_ArtistSourceMaterialParameters[4u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*g_ArtistSourceMaterialParameters[4u].wwww))).x;
    source[13].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].x = ((float4(3.0, 0.0, 0.0, 0.0)+(sin(((((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialParameters[4u].wwww)/(g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialParameters[4u].wwww))+(((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialTime.xxxx)/g_ArtistSourceMaterialParameters[4u].wwww)+g_ArtistSourceMaterialParameters[4u].wwww))*float4(6.28318548, 0.0, 0.0, 0.0)))*(sin(((((g_ArtistSourceMaterialTime.xxxx+g_ArtistSourceMaterialTime.xxxx)/g_ArtistSourceMaterialParameters[4u].wwww)+g_ArtistSourceMaterialParameters[4u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*g_ArtistSourceMaterialParameters[4u].wwww)))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v2.xyxx, cb0[3].xyxx
    r0.xy = ((v2.xyxx)+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t2.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: max r0.x, r0.x, cb0[9].z
    r0.x = (max(r0.xxxx,source[9].zzzz)).x;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xxxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xxxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: mad r0.x, r0.x, l(2.000000), l(-1.000000)
    r0.x = ((r0.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 9: add r0.y, -r0.z, l(1.000000)
    r0.y = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 10: max r0.y, r0.y, cb0[11].x
    r0.y = (max(r0.yyyy,source[11].xxxx)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yyyy, t3.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yyyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 12: mad r0.y, r0.y, l(2.000000), l(-1.000000)
    r0.y = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 13: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 14: mad r1.xy, cb0[10].xxxx, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[10].xxxx)*(r0.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 15: mad r1.xy, r1.xyxx, cb0[4].xyxx, cb0[5].xyxx
    r1.xy = ((r1.xyxx)*(source[4].xyxx)+(source[5].xyxx)).xy;
    // 16: mad r1.xy, r0.xxxx, cb0[9].wwww, r1.xyxx
    r1.xy = ((r0.xxxx)*(source[9].wwww)+(r1.xyxx)).xy;
    // 17: mad r0.xy, r0.yyyy, cb0[11].yyyy, r1.xyxx
    r0.xy = ((r0.yyyy)*(source[11].yyyy)+(r1.xyxx)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.yxzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 19: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: mul r1.x, |r0.x|, |r0.x|
    r1.x = ((abs(r0.xxxx))*(abs(r0.xxxx))).x;
    // 21: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 22: dp2 r1.x, cb0[6].xyxx, r0.zwzz
    r1.x = (dot((source[6].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 23: dp2 r1.y, cb0[7].xyxx, r0.zwzz
    r1.y = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 24: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t6.yzxw, s5, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 26: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 27: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 28: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 29: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 30: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 31: mul r0.z, r0.z, cb0[12].x
    r0.z = ((r0.zzzz)*(source[12].xxxx)).z;
    // 32: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 33: add r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)+(r0.xxxx)).y;
    // 34: mad r0.yzw, r0.yyyy, cb0[2].xxyz, r0.yyyy
    r0.yzw = ((r0.yyyy)*(source[2].xxyz)+(r0.yyyy)).yzw;
    // 35: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 36: mad r2.xyz, r1.xyzx, r0.yzwy, -r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)+(-(r0.yzwy))).xyz;
    // 37: mad r0.yzw, cb0[12].yyyy, r2.xxyz, r0.yyzw
    r0.yzw = ((source[12].yyyy)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 38: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 39: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 40: mul r1.w, r1.w, v6.z
    r1.w = ((r1.wwww)*(v6.zzzz)).w;
    // 41: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r2.x, |r1.w|, |r1.w|
    r2.x = ((abs(r1.wwww))*(abs(r1.wwww))).x;
    // 44: mul r2.x, |r1.w|, r2.x
    r2.x = ((abs(r1.wwww))*(r2.xxxx)).x;
    // 45: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: mul r1.xyz, r1.xyzx, r2.xxxx
    r1.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 47: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 48: add r1.w, cb0[14].x, l(-1.000000)
    r1.w = ((source[14].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 49: mad r1.w, cb0[13].w, r1.w, l(1.000000)
    r1.w = ((source[13].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 51: mad r0.yzw, r1.wwww, r0.yyzw, r1.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 52: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 53: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 54: mad r0.y, v2.y, l(2.000000), l(-1.000000)
    r0.y = ((v2.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 55: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 56: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 57: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 58: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 59: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 60: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 61: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: add r0.z, -|r0.z|, l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mul r0.z, r0.z, l(30.000000)
    r0.z = ((r0.zzzz)*(float4(30.000000,30.000000,30.000000,30.000000))).z;
    // 64: round_pi r0.z, r0.z
    r0.z = (ceil(r0.zzzz)).z;
    // 65: add r0.w, -v4.x, l(1.000000)
    r0.w = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mad r0.z, r0.z, l(0.033333), -r0.w
    r0.z = ((r0.zzzz)*(float4(0.033333,0.033333,0.033333,0.033333))+(-(r0.wwww))).z;
    // 67: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 68: mul_sat r0.y, r0.y, l(20.000000)
    r0.y = (saturate((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000)))).y;
    // 69: dp3 r0.z, v1.xyzx, v1.xyzx
    r0.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 70: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 71: mul r1.xyzw, r0.zzzz, v1.zxxy
    r1.xyzw = ((r0.zzzz)*(v1.zxxy)).xyzw;
    // 72: dp3 r0.z, v0.xyzx, v0.xyzx
    r0.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 73: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 74: mul r2.xyzw, r0.zzzz, v0.xyzx
    r2.xyzw = ((r0.zzzz)*(v0.xyzx)).xyzw;
    // 75: mul r0.zw, r1.zzzw, r2.zzzw
    r0.zw = ((r1.zzzw)*(r2.zzzw)).zw;
    // 76: mad r0.zw, r1.xxxy, r2.xxxy, -r0.zzzw
    r0.zw = ((r1.xxxy)*(r2.xxxy)+(-(r0.zzzw))).zw;
    // 77: mul r3.xy, r0.zwzz, v1.wwww
    r3.xy = ((r0.zwzz)*(v1.wwww)).xy;
    // 78: add r4.xyz, v7.xyzx, cb0[0].xyzx
    r4.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 79: mul r0.z, r3.x, r4.y
    r0.z = ((r3.xxxx)*(r4.yyyy)).z;
    // 80: mad r0.z, r4.x, r2.y, r0.z
    r0.z = ((r4.xxxx)*(r2.yyyy)+(r0.zzzz)).z;
    // 81: mov r3.z, r2.z
    r3.z = (r2.zzzz).z;
    // 82: mad r0.z, r4.z, r1.w, r0.z
    r0.z = ((r4.zzzz)*(r1.wwww)+(r0.zzzz)).z;
    // 83: mov r3.w, r1.x
    r3.w = (r1.xxxx).w;
    // 84: mul r0.w, cb0[8].x, cb0[14].w
    r0.w = ((source[8].xxxx)*(source[14].wwww)).w;
    // 85: mad r0.z, cb0[14].y, r0.z, r0.w
    r0.z = ((source[14].yyyy)*(r0.zzzz)+(r0.wwww)).z;
    // 86: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 87: sincos r0.z, null, r0.z
    { const float4 sourceAngle = r0.zzzz; r0.z = (sin(sourceAngle)).z; }
    // 88: add r0.w, -cb0[15].y, cb0[15].x
    r0.w = ((-(source[15].yyyy))+(source[15].xxxx)).w;
    // 89: mad r0.z, r0.z, r0.w, cb0[15].y
    r0.z = ((r0.zzzz)*(r0.wwww)+(source[15].yyyy)).z;
    // 90: mul r0.w, |r0.z|, |r0.z|
    r0.w = ((abs(r0.zzzz))*(abs(r0.zzzz))).w;
    // 91: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 92: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: movc r0.z, r0.z, l(1.000000), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.wwww)).z;
    // 95: mul r0.z, r0.z, cb0[15].z
    r0.z = ((r0.zzzz)*(source[15].zzzz)).z;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t5.yzwx, s6, l(0.000000)
    r0.w = (ArtistNativeSample6((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 97: add r1.x, -cb0[15].w, l(1.000000)
    r1.x = ((-(source[15].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 98: mad_sat r0.w, r0.w, r1.x, cb0[15].w
    r0.w = (saturate((r0.wwww)*(r1.xxxx)+(source[15].wwww))).w;
    // 99: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 100: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 101: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 102: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 103: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_spriteinvert_01_02_tr: 743c811e01b5c74fabe2852946ed88f0; selected map 4798d7226108e27a380e7d0891028d32e05ec49f222fab42a1ed0fdad36cbe9e.
float4 ArtistNative4903(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].w = ((g_ArtistSourceMaterialParameters[0u].xxxx+float4(0.0, 0.0, 0.0, 0.0))).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].y = ((g_ArtistSourceMaterialParameters[0u].yyyy+float4(0.0, 0.0, 0.0, 0.0))).x;
    source[3].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].z = ((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].x = ((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].x = ((sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[7].y = (abs((sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))).x;
    source[7].w = ((fmod(abs((sin(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].wwww)*float4(6.28318548, 0.0, 0.0, 0.0)))*float4(2.0, 0.0, 0.0, 0.0))),float4(1.5, 0.0, 0.0, 0.0))+float4(0.300000012, 0.0, 0.0, 0.0))).x;
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
    // 1: mad r0.x, v2.x, cb0[3].z, cb0[4].z
    r0.x = ((v2.xxxx)*(source[3].zzzz)+(source[4].zzzz)).x;
    // 2: mad r0.y, v2.y, cb0[3].w, cb0[5].x
    r0.y = ((v2.yyyy)*(source[3].wwww)+(source[5].xxxx)).y;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 5: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 6: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 7: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 8: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 9: add r0.z, v4.y, l(-1.000000)
    r0.z = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 10: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 11: mad r1.x, v2.x, cb0[2].x, cb0[2].w
    r1.x = ((v2.xxxx)*(source[2].xxxx)+(source[2].wwww)).x;
    // 12: mad r1.y, v2.y, cb0[2].y, cb0[3].y
    r1.y = ((v2.yyyy)*(source[2].yyyy)+(source[3].yyyy)).y;
    // 13: mad r0.xz, r0.xxxx, r0.zzzz, r1.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzzz)+(r1.xxyx)).xz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 15: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 16: mul r0.y, r0.x, cb0[5].z
    r0.y = ((r0.xxxx)*(source[5].zzzz)).y;
    // 17: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 18: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul_sat r0.z, r0.z, cb0[6].x
    r0.z = (saturate((r0.zzzz)*(source[6].xxxx))).z;
    // 22: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 23: mad_sat r0.x, -r0.x, cb0[6].y, r0.y
    r0.x = (saturate((-(r0.xxxx))*(source[6].yyyy)+(r0.yyyy))).x;
    // 24: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 25: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 26: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 27: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)*(source[6].zzzz)).y;
    // 29: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 30: mul r0.y, r0.y, cb0[6].w
    r0.y = ((r0.yyyy)*(source[6].wwww)).y;
    // 31: mul r0.y, r0.y, cb0[7].w
    r0.y = ((r0.yyyy)*(source[7].wwww)).y;
    // 32: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 33: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 34: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 35: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_dot_ad_01: cdfe9a7700a1b745b352dd8c9a1c9318; selected map 4c069a2292465e80e67dce6093609d81b966a7837bff946f2eb40abfda680db5.
float4 ArtistNative4904(ARTIST_NATIVE_INPUT input)
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
    // 1: add r0.x, -v2.x, v3.w
    r0.x = ((-(v2.xxxx))+(v3.wwww)).x;
    // 2: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 4: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 5: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 6: mul o0.xyz, r0.xyzx, cb0[0].xxxx
    output.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 7: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_chromring_01_03_ad: b93fc2969ce70448ae7f5676cf3b950c; selected map 35d5d2e4490f33fd033fd6236fd154a91e387df1bff3aaacc8b1be15b9e6598a.
float4 ArtistNative4905(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[2].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 2: mov r0.y, v4.w
    r0.y = (v4.wwww).y;
    // 3: mad r0.xy, v2.xyxx, l(0.500000, 0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(0.500000,0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: add r0.z, v4.y, l(-1.000000)
    r0.z = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 6: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 7: mul r0.zw, v2.xxxy, cb0[2].xxxy
    r0.zw = ((v2.xxxy)*(source[2].xxxy)).zw;
    // 8: mad r0.xy, r0.zwzz, l(0.400000, 0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r0.zwzz)*(float4(0.400000,0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 9: add r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)+(v4.xxxx)).y;
    // 10: add r0.z, r0.y, l(-0.300000)
    r0.z = ((r0.yyyy)+(float4(-0.300000,-0.300000,-0.300000,-0.300000))).z;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r0.xzxx, t2.xyzw, s2, l(0.000000)
    r1.y = (ArtistNativeSample1((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 12: mul r0.y, v4.z, cb0[2].z
    r0.y = ((v4.zzzz)*(source[2].zzzz)).y;
    // 13: mad r0.xyzw, r0.yyyy, l(0.005000, -0.005000, -0.005000, 0.005000), r0.xzxz
    r0.xyzw = ((r0.yyyy)*(float4(0.005000,-0.005000,-0.005000,0.005000))+(r0.xzxz)).xyzw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
    // 16: max r0.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 17: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 18: mul r0.xyz, r0.xyzx, cb0[2].wwww
    r0.xyz = ((r0.xyzx)*(source[2].wwww)).xyz;
    // 19: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 20: mul r0.xyz, r0.xyzx, cb0[3].xxxx
    r0.xyz = ((r0.xyzx)*(source[3].xxxx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t3.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 22: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 23: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 24: mul r1.x, r1.x, cb0[3].y
    r1.x = ((r1.xxxx)*(source[3].yyyy)).x;
    // 25: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 26: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 27: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 28: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 29: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 30: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 31: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 32: source device depth mapped to centimetre view depth; reconstruction at 34.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 34-37: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 38: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 39: add r1.x, -cb0[3].z, l(1.000000)
    r1.x = ((-(source[3].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 41: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 42: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 43: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 44: add r1.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 45: mul r1.xy, r1.xyxx, v2.xyxx
    r1.xy = ((r1.xyxx)*(v2.xyxx)).xy;
    // 46: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 47: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 48: mul r0.w, r0.w, l(20.000000)
    r0.w = ((r0.wwww)*(float4(20.000000,20.000000,20.000000,20.000000))).w;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4905Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[0].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[0].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[0].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[1].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[1].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[1].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[1].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 1: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 2: mov r0.y, v3.w
    r0.y = (v3.wwww).y;
    // 3: mad r0.xy, v1.xyxx, l(0.500000, 0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v1.xyxx)*(float4(0.500000,0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: add r0.z, v3.y, l(-1.000000)
    r0.z = ((v3.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 6: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 7: mul r0.zw, v1.xxxy, cb0[0].xxxy
    r0.zw = ((v1.xxxy)*(source[0].xxxy)).zw;
    // 8: mad r0.xy, r0.zwzz, l(0.400000, 0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r0.zwzz)*(float4(0.400000,0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 9: add r0.y, r0.y, v3.x
    r0.y = ((r0.yyyy)+(v3.xxxx)).y;
    // 10: add r0.z, r0.y, l(-0.300000)
    r0.z = ((r0.yyyy)+(float4(-0.300000,-0.300000,-0.300000,-0.300000))).z;
    // 11: mul r0.y, v3.z, cb0[0].z
    r0.y = ((v3.zzzz)*(source[0].zzzz)).y;
    // 12: mad r0.yw, r0.yyyy, l(0.000000, 0.005000, 0.000000, -0.005000), r0.xxxz
    r0.yw = ((r0.yyyy)*(float4(0.000000,0.005000,0.000000,-0.005000))+(r0.xxxz)).yw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t1.yxzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 14: mov r1.yw, r0.xxxx
    r1.yw = (r0.xxxx).yw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.ywyy, t1.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 16: mov r1.xz, r0.xxxx
    r1.xz = (r0.xxxx).xz;
    // 17: max r0.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 18: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 19: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
    // 20: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 21: mul r0.xyzw, r0.xyzw, cb0[1].xxxx
    r0.xyzw = ((r0.xyzw)*(source[1].xxxx)).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample2((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 23: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 24: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r1.y, r1.y, cb0[1].y
    r1.y = ((r1.yyyy)*(source[1].yyyy)).y;
    // 26: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 27: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 28: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 29: mul r0.xyzw, r0.xyzw, cb0[1].wwww
    r0.xyzw = ((r0.xyzw)*(source[1].wwww)).xyzw;
    // 30: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 31: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 32: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 33: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 34: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 36: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 37: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 39: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 40: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 41: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 42: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 43: source device depth mapped to centimetre view depth; reconstruction at 45.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 45-48: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 49: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 50: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 51: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 52: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 53: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_w_pa_master_01_07_dt_tr: dcf7eced1d7dab4b8c9e4cf5924e53e5; selected map f986bfb1e6330aa91bcd81467e548823146374465733d4fe9f0eac86e446bdbd.
float4 ArtistNative4906(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].xxxx)).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[6].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[6].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[4].w
    r0.w = ((r0.xxxx)*(source[4].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[4].z, r0.w
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.wwww)).x;
    // 9: mul r0.w, r0.z, cb0[6].w
    r0.w = ((r0.zzzz)*(source[6].wwww)).w;
    // 10: mad r1.y, cb0[5].x, r0.y, r0.w
    r1.y = ((source[5].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r2.xy, r0.xyxx, cb0[7].yzyy
    r2.xy = ((r0.xyxx)*(source[7].yzyy)).xy;
    // 13: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 14: mad r0.zw, r0.zzzz, cb0[7].xxxw, r2.xxxy
    r0.zw = ((r0.zzzz)*(source[7].xxxw)+(r2.xxxy)).zw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t4.xyzw, s3, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 16: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 17: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 18: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 19: mad r1.xyz, cb0[8].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[8].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 20: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 21: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 22: mul r1.xyz, r1.xyzx, cb0[8].yyyy
    r1.xyz = ((r1.xyzx)*(source[8].yyyy)).xyz;
    // 23: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 24: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 25: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 26: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 27: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 28: mad r0.z, cb0[4].y, cb0[8].w, cb0[9].x
    r0.z = ((source[4].yyyy)*(source[8].wwww)+(source[9].xxxx)).z;
    // 29: sincos r1.x, r2.x, r0.z
    { const float4 sourceAngle = r0.zzzz; r1.x = (sin(sourceAngle)).x; r2.x = (cos(sourceAngle)).x; }
    // 30: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 31: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 32: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 33: dp2 r0.z, r3.zyzz, r0.xyxx
    r0.z = (dot((r3.zyzz).xy,(r0.xyxx).xy).xxxx).z;
    // 34: dp2 r0.x, r3.yxyy, r0.xyxx
    r0.x = (dot((r3.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 35: mul r1.z, r0.z, cb0[3].y
    r1.z = ((r0.zzzz)*(source[3].yyyy)).z;
    // 36: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 37: mad r1.x, r0.x, cb0[3].x, r0.y
    r1.x = ((r0.xxxx)*(source[3].xxxx)+(r0.yyyy)).x;
    // 38: add r0.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 40: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 42: mul_sat r0.x, r0.x, cb0[9].w
    r0.x = (saturate((r0.xxxx)*(source[9].wwww))).x;
    // 43: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 44: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 45: mul r0.x, r0.x, cb0[10].x
    r0.x = ((r0.xxxx)*(source[10].xxxx)).x;
    // 46: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 47: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 48: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 49: source device depth mapped to centimetre view depth; reconstruction at 51.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 51-54: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 55: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 56: add r0.w, -cb0[10].y, l(1.000000)
    r0.w = ((-(source[10].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 58: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 59: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 62: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4906Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_r_pa_flashbang_03_02_tr: 58e1a4d7a3da5d40a9872a0c55cc295f; selected map 64ee4fe278d46b60b34e916f70633058783cf3d035e347377bff587d04bf7e41.
float4 ArtistNative4907(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
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
    // 30: mul r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)*(source[4].wwww)).y;
    // 31: mad r1.x, cb0[4].z, cb0[4].y, r0.y
    r1.x = ((source[4].zzzz)*(source[4].yyyy)+(r0.yyyy)).x;
    // 32: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 33: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 34: mul r0.z, r0.y, cb0[3].z
    r0.z = ((r0.yyyy)*(source[3].zzzz)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: movc r0.z, r0.x, l(0), r0.z
    r0.z = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 37: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 38: mad r1.y, cb0[4].z, cb0[5].y, r0.z
    r1.y = ((source[4].zzzz)*(source[5].yyyy)+(r0.zzzz)).y;
    // 39: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s0, cb0[4].x
    r0.z = (ArtistNativeSample0((r1.xyxx).xy, (source[4].xxxx).x, true).yzxw).z;
    // 40: mul_sat r0.z, r0.z, cb0[5].z
    r0.z = (saturate((r0.zzzz)*(source[5].zzzz))).z;
    // 41: mov_sat r0.w, cb0[3].y
    r0.w = (saturate(source[3].yyyy)).w;
    // 42: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 46: mul_sat r0.z, r0.w, r0.z
    r0.z = (saturate((r0.wwww)*(r0.zzzz))).z;
    // 47: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 48: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 49: mul r0.w, r0.w, cb0[5].w
    r0.w = ((r0.wwww)*(source[5].wwww)).w;
    // 50: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 51: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 52: mul r0.w, cb0[3].w, cb0[6].x
    r0.w = ((source[3].wwww)*(source[6].xxxx)).w;
    // 53: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 54: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 55: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 56: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 57: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 58: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 59: add r0.xyz, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)+(source[2].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4907Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_impactscreen_01_01_tr: 0f467abd6fb85340b8354a10ea8765ae; selected map 5488f95368ad85c906230031df1ad62d18c2605a9af3994e2aed5c0de94d9e15.
float4 ArtistNative4908(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(5.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(261.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(261.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(5.0, 0.0, 0.0, 0.0)))).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = ((g_ArtistSourceMaterialParameters[0u].yyyy+float4(1.0, 0.0, 0.0, 0.0))).x;
    source[7].y = ((float4(0.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
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
    // 1: mad r0.xy, v2.xyxx, cb0[2].xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(source[2].xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: add r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)+(r0.xyxx)).xy;
    // 3: max r0.z, |r0.x|, |r0.y|
    r0.z = (max(abs(r0.xxxx),abs(r0.yyyy))).z;
    // 4: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 5: min r0.w, |r0.x|, |r0.y|
    r0.w = (min(abs(r0.xxxx),abs(r0.yyyy))).w;
    // 6: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 7: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 8: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 9: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 10: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 11: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 12: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 13: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 14: lt r1.y, |r0.x|, |r0.y|
    r1.y = (asfloat((uint4)((abs(r0.xxxx))<(abs(r0.yyyy))) * 0xffffffffu)).y;
    // 15: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 16: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 17: lt r0.w, r0.x, -r0.x
    r0.w = (asfloat((uint4)((r0.xxxx)<(-(r0.xxxx))) * 0xffffffffu)).w;
    // 18: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 19: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 20: min r0.w, r0.x, r0.y
    r0.w = (min(r0.xxxx,r0.yyyy)).w;
    // 21: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 22: max r1.x, r0.x, r0.y
    r1.x = (max(r0.xxxx,r0.yyyy)).x;
    // 23: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 24: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 25: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 26: mul r0.y, r0.x, cb0[5].x
    r0.y = ((r0.xxxx)*(source[5].xxxx)).y;
    // 27: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 28: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 29: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 30: mul r0.z, r0.z, l(0.159155)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))).z;
    // 31: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 32: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 33: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 34: div r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)/(source[4].zzzz)).z;
    // 35: mul r0.x, r0.z, cb0[4].w
    r0.x = ((r0.zzzz)*(source[4].wwww)).x;
    // 36: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 38: add r0.yz, r0.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 39: add r0.yz, r0.yyzy, r0.yyzy
    r0.yz = ((r0.yyzy)+(r0.yyzy)).yz;
    // 40: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 41: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 42: add r1.x, -r0.w, l(1.000000)
    r1.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 44: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 45: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 46: mul r1.y, r1.y, l(10.000000)
    r1.y = ((r1.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 47: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 48: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 49: mul r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)*(r1.xxxx)).yz;
    // 50: mad r1.x, r1.x, cb0[6].x, -cb0[6].y
    r1.x = ((r1.xxxx)*(source[6].xxxx)+(-(source[6].yyyy))).x;
    // 51: mul r0.yz, r0.yyzy, cb0[5].wwww
    r0.yz = ((r0.yyzy)*(source[5].wwww)).yz;
    // 52: mad r1.y, r1.x, l(2.000000), l(1.000000)
    r1.y = ((r1.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: mad_sat r0.x, r0.x, r1.y, -r1.x
    r0.x = (saturate((r0.xxxx)*(r1.yyyy)+(-(r1.xxxx)))).x;
    // 54: mul r0.xy, r0.yzyy, r0.xxxx
    r0.xy = ((r0.yzyy)*(r0.xxxx)).xy;
    // 55: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 56: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 57: mul r0.z, v4.w, cb0[6].z
    r0.z = ((v4.wwww)*(source[6].zzzz)).z;
    // 58: mad r0.xy, r0.zzzz, r0.xyxx, r1.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r1.xyxx)).xy;
    // 59: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.xyz = (Read_EffectSceneColor(LinearClampUVSampler, (r0.xyxx).xy).xyzw).xyz;
    // 60: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 61: mul r0.x, r0.x, cb0[6].w
    r0.x = ((r0.xxxx)*(source[6].wwww)).x;
    // 62: add r0.y, -cb0[7].y, cb0[7].x
    r0.y = ((-(source[7].yyyy))+(source[7].xxxx)).y;
    // 63: mad_sat r0.x, r0.x, r0.y, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(source[7].yyyy))).x;
    // 64: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 66: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 67: max r0.x, v4.x, l(0.000010)
    r0.x = (max(v4.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 68: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 69: mad r0.x, -r0.w, r0.x, l(1.000000)
    r0.x = ((-(r0.wwww))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 70: add r0.y, -v4.y, l(1.000000)
    r0.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 71: max r0.y, r0.y, l(0.000010)
    r0.y = (max(r0.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 72: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 73: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 74: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 75: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 76: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 77: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 78: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 79: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 80: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_glasshole_01_01_tr: 7b93641a7efece49b83cf5555e5b9b63; selected map ff0c51bf206901318de55c8923ee9086f64e74cfbc9a1370469e9295616bee3a.
float4 ArtistNative4909(ARTIST_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[9u];
    source[2] = ArtistNativeAppend((g_ArtistSourceMaterialParameters[3u].zzzz*g_ArtistSourceMaterialTime.xxxx),(g_ArtistSourceMaterialParameters[3u].wwww*g_ArtistSourceMaterialTime.xxxx),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].xxxx,g_ArtistSourceMaterialParameters[0u].yyyy,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].zzzz,1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(-0.230000004, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(-0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz)*float4(0.370000005, 0.0, 0.0, 0.0))),1u);
    source[8] = g_ArtistSourceMaterialParameters[8u];
    source[9] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[10] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))),1u);
    source[11] = g_ArtistSourceMaterialParameters[7u];
    source[12] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[13].x = ((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].z = ((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[13].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[15].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[15].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))).x;
    source[16].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[16].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[17].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[17].w = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[18].x = ((g_ArtistSourceMaterialParameters[6u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[18].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0)))).x;
    source[18].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[18].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))).x;
    source[19].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0)))).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[19].w = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[20].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[20].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 27: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[16].yyyy
    r0.z = (dot((r0.zzzz).xy,(source[16].yyyy).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r0.y, r0.y, cb0[16].z
    r0.y = ((r0.yyyy)+(source[16].zzzz)).y;
    // 32: mul r1.x, r0.y, cb0[17].y
    r1.x = ((r0.yyyy)*(source[17].yyyy)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, v4.w
    r0.y = ((r0.yyyy)*(v4.wwww)).y;
    // 38: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 39: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 40: add r0.z, v4.z, cb0[17].x
    r0.z = ((v4.zzzz)+(source[17].xxxx)).z;
    // 41: mad r0.y, r0.z, l(-0.400000), r0.y
    r0.y = ((r0.zzzz)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.yyyy)).y;
    // 42: mul r0.z, v4.w, cb0[18].x
    r0.z = ((v4.wwww)*(source[18].xxxx)).z;
    // 43: mad r1.y, r0.y, cb0[17].z, r0.z
    r1.y = ((r0.yyyy)*(source[17].zzzz)+(r0.zzzz)).y;
    // 44: add r0.yz, r1.xxyx, cb0[9].xxyx
    r0.yz = ((r1.xxyx)+(source[9].xxyx)).yz;
    // 45: add r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)+(source[10].xyxx)).xy;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t6.xyzw, s5, l(-1.000000)
    r1.xyz = (ArtistNativeSample4((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s4, l(-1.000000)
    r0.yzw = (ArtistNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 48: mul r2.xyz, r1.xyzx, r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)).xyz;
    // 49: add r0.yzw, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)+(r0.yyzw)).yzw;
    // 50: max r1.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 51: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 52: mul r1.xyz, r1.xyzx, cb0[19].yyyy
    r1.xyz = ((r1.xyzx)*(source[19].yyyy)).xyz;
    // 53: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 54: add r2.xy, v2.xyxx, -cb0[4].xyxx
    r2.xy = ((v2.xyxx)+(-(source[4].xyxx))).xy;
    // 55: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 56: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 57: min r2.z, |r2.y|, |r2.x|
    r2.z = (min(abs(r2.yyyy),abs(r2.xxxx))).z;
    // 58: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 59: mul r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)*(r1.wwww)).z;
    // 60: mad r2.w, r2.z, l(0.020835), l(-0.085133)
    r2.w = ((r2.zzzz)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).w;
    // 61: mad r2.w, r2.z, r2.w, l(0.180141)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 62: mad r2.w, r2.z, r2.w, l(-0.330299)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).w;
    // 63: mad r2.z, r2.z, r2.w, l(0.999866)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 64: mul r2.w, r1.w, r2.z
    r2.w = ((r1.wwww)*(r2.zzzz)).w;
    // 65: mad r2.w, r2.w, l(-2.000000), l(1.570796)
    r2.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).w;
    // 66: lt r3.x, |r2.y|, |r2.x|
    r3.x = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).x;
    // 67: and r2.w, r2.w, r3.x
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r3.xxxx))).w;
    // 68: mad r1.w, r1.w, r2.z, r2.w
    r1.w = ((r1.wwww)*(r2.zzzz)+(r2.wwww)).w;
    // 69: lt r2.z, r2.y, -r2.y
    r2.z = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).z;
    // 70: and r2.z, r2.z, l(0xc0490fdb)
    r2.z = (asfloat(asuint(r2.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 71: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 72: min r2.z, r2.y, r2.x
    r2.z = (min(r2.yyyy,r2.xxxx)).z;
    // 73: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 74: ge r2.x, r2.x, -r2.x
    r2.x = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).x;
    // 75: lt r2.y, r2.z, -r2.z
    r2.y = (asfloat((uint4)((r2.zzzz)<(-(r2.zzzz))) * 0xffffffffu)).y;
    // 76: and r2.x, r2.x, r2.y
    r2.x = (asfloat(asuint(r2.xxxx) & asuint(r2.yyyy))).x;
    // 77: movc r1.w, r2.x, -r1.w, r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (-(r1.wwww)) : (r1.wwww)).w;
    // 78: mad r2.x, r1.w, l(0.159155), l(0.500000)
    r2.x = ((r1.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 79: mad r3.xy, v2.xyxx, cb0[5].xyxx, cb0[6].xyxx
    r3.xy = ((v2.xyxx)*(source[5].xyxx)+(source[6].xyxx)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 81: mad r1.w, r1.w, l(2.000000), l(-1.000000)
    r1.w = ((r1.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 82: mad r3.xy, v2.xyxx, cb0[5].xyxx, cb0[7].xyxx
    r3.xy = ((v2.xyxx)*(source[5].xyxx)+(source[7].xyxx)).xy;
    // 83: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r2.w = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 84: mad r2.w, r2.w, l(2.000000), l(-1.000000)
    r2.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 85: add r3.xy, v4.xyxx, l(-0.500000, -0.900000, 0.000000, 0.000000)
    r3.xy = ((v4.xyxx)+(float4(-0.500000,-0.900000,0.000000,0.000000))).xy;
    // 86: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 87: mad r1.w, r3.y, r1.w, r2.w
    r1.w = ((r3.yyyy)*(r1.wwww)+(r2.wwww)).w;
    // 88: mad r2.w, r0.x, l(3.000000), -r3.x
    r2.w = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(-(r3.xxxx))).w;
    // 89: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 90: max r2.w, r2.w, l(-0.100000)
    r2.w = (max(r2.wwww,float4(-0.100000,-0.100000,-0.100000,-0.100000))).w;
    // 91: min r2.y, r2.w, l(10.000000)
    r2.y = (min(r2.wwww,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 92: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 93: mad r3.xy, r0.xxxx, r1.wwww, r2.xyxx
    r3.xy = ((r0.xxxx)*(r1.wwww)+(r2.xyxx)).xy;
    // 94: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 95: sample_l_indexable(texture2d)(float,float,float,float) r2.yz, r3.xyxx, t1.yzxw, s2, l(-1.000000)
    r2.yz = (ArtistNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 96: mul r3.xyz, r0.yzwy, r2.zzzz
    r3.xyz = ((r0.yzwy)*(r2.zzzz)).xyz;
    // 97: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 98: mad r1.xyz, cb0[19].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[19].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 99: mad r2.xw, r2.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[12].xxxy
    r2.xw = ((r2.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[12].xxxy)).xw;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xwxx, t7.xyzw, s6, l(0.000000)
    r3.xyz = (ArtistNativeSample5((r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 101: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 102: mul_sat r3.xyz, r3.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 103: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 104: mad r1.xyz, r1.xyzx, cb0[11].xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(source[11].xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 106: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 107: mul r2.xw, r0.wwww, v6.xxxy
    r2.xw = ((r0.wwww)*(v6.xxxy)).xw;
    // 108: mad r0.yz, r0.yyzy, l(0.000000, 0.200000, 0.200000, 0.000000), r2.xxwx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.200000,0.200000,0.000000))+(r2.xxwx)).yz;
    // 109: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t8.wxyz, s7, l(0.000000)
    r0.yzw = (ArtistNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 110: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 111: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 113: mad r0.yzw, cb0[20].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[20].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 114: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 115: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 116: source device depth mapped to centimetre view depth; reconstruction at 118.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 118-121: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 122: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 123: mul_sat r1.xy, r1.xxxx, l(0.034483, 0.066667, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xxxx)*(float4(0.034483,0.066667,0.000000,0.000000)))).xy;
    // 124: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 125: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 126: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 127: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 128: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 129: mul r3.xyz, r1.zzzz, l(10.000000, 10.000000, 20.000000, 0.000000)
    r3.xyz = ((r1.zzzz)*(float4(10.000000,10.000000,20.000000,0.000000))).xyz;
    // 130: movc r1.yzw, r1.yyyy, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 131: mad r0.yzw, r2.yyyy, r0.yyzw, r1.yyzw
    r0.yzw = ((r2.yyyy)*(r0.yyzw)+(r1.yyzw)).yzw;
    // 132: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 133: log r1.y, r0.x
    r1.y = (log2(r0.xxxx)).y;
    // 134: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 135: mul r1.y, r1.y, cb0[13].x
    r1.y = ((r1.yyyy)*(source[13].xxxx)).y;
    // 136: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 137: mul r1.y, r1.y, cb0[13].z
    r1.y = ((r1.yyyy)*(source[13].zzzz)).y;
    // 138: add r1.zw, -v2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((-(v2.xxxy))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 139: mul r1.yz, r1.zzwz, r1.yyyy
    r1.yz = ((r1.zzwz)*(r1.yyyy)).yz;
    // 140: movc r1.yz, r0.xxxx, l(0,0,0,0), r1.yyzy
    r1.yz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzy)).yz;
    // 141: add r1.yz, r1.yyzy, v2.xxyx
    r1.yz = ((r1.yyzy)+(v2.xxyx)).yz;
    // 142: mad r1.yz, cb0[13].wwww, r1.yyzy, cb0[2].xxyx
    r1.yz = ((source[13].wwww)*(r1.yyzy)+(source[2].xxyx)).yz;
    // 143: add r1.yz, r1.yyzy, cb0[3].xxyx
    r1.yz = ((r1.yyzy)+(source[3].xxyx)).yz;
    // 144: add r0.x, -r2.y, l(1.000000)
    r0.x = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 145: add_sat r1.w, r0.x, r2.z
    r1.w = (saturate((r0.xxxx)+(r2.zzzz))).w;
    // 146: mul r1.x, r1.x, r1.w
    r1.x = ((r1.xxxx)*(r1.wwww)).x;
    // 147: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 148: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v2.xyxx, t0.zxyw, s1, l(0.000000)
    r2.yz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 149: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 150: mul r2.yz, r0.xxxx, r2.yyzy
    r2.yz = ((r0.xxxx)*(r2.yyzy)).yz;
    // 151: mad r1.yz, cb0[14].yyyy, r2.yyzy, r1.yyzy
    r1.yz = ((source[14].yyyy)*(r2.yyzy)+(r1.yyzy)).yz;
    // 152: mad r1.w, cb0[14].z, l(0.500000), l(-0.250000)
    r1.w = ((source[14].zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(-0.250000,-0.250000,-0.250000,-0.250000))).w;
    // 153: mad r1.yz, r1.wwww, r2.xxwx, r1.yyzy
    r1.yz = ((r1.wwww)*(r2.xxwx)+(r1.yyzy)).yz;
    // 154: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t4.wxyz, s3, l(0.000000)
    r1.yzw = (ArtistNativeSample2((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 155: max r1.yzw, |r1.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r1.yzw = (max(abs(r1.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 156: log r1.yzw, r1.yyzw
    r1.yzw = (log2(r1.yyzw)).yzw;
    // 157: mul r1.yzw, r1.yyzw, cb0[14].wwww
    r1.yzw = ((r1.yyzw)*(source[14].wwww)).yzw;
    // 158: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 159: mul r2.xyz, r1.yzwy, cb0[15].xxxx
    r2.xyz = ((r1.yzwy)*(source[15].xxxx)).xyz;
    // 160: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: mad r1.yzw, -cb0[15].xxxx, r1.yyzw, r2.wwww
    r1.yzw = ((-(source[15].xxxx))*(r1.yyzw)+(r2.wwww)).yzw;
    // 162: mad r1.yzw, cb0[15].yyyy, r1.yyzw, r2.xxyz
    r1.yzw = ((source[15].yyyy)*(r1.yyzw)+(r2.xxyz)).yzw;
    // 163: mul r1.yzw, r1.yyzw, cb0[8].xxyz
    r1.yzw = ((r1.yyzw)*(source[8].xxyz)).yzw;
    // 164: mad r0.xyz, r0.xxxx, r1.yzwy, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.yzwy)+(r0.yzwy)).xyz;
    // 165: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 166: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 167: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t3.xyzw, s8, l(0.000000)
    r0.x = (ArtistNativeSample7((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 168: mul_sat r0.x, r1.x, r0.x
    r0.x = (saturate((r1.xxxx)*(r0.xxxx))).x;
    // 169: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_slice_01_07_tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 ArtistNative4910(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[6].zzwz, cb0[7].xxyx
    r0.yz = ((r0.yyzy)*(source[6].zzwz)+(source[7].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v4.z, cb0[7].z
    r0.z = ((v4.zzzz)*(source[7].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v2.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v2.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[4].xyxx, r0.yzyy
    r0.w = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[5].xyxx, r0.yzyy
    r0.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[8].x, l(10.000000), l(10.000000)
    r0.w = ((source[8].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 25: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 30: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 37: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 38: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 40: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 41: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 44: mad r0.xz, v2.xxyx, l(-1.000000, 0.000000, 1.000000, 0.000000), l(1.000000, 0.000000, 0.000000, 0.000000)
    r0.xz = ((v2.xxyx)*(float4(-1.000000,0.000000,1.000000,0.000000))+(float4(1.000000,0.000000,0.000000,0.000000))).xz;
    // 45: mad r0.xz, r0.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 46: dp2 r0.x, r0.xzxx, r0.xzxx
    r0.x = (dot((r0.xzxx).xy,(r0.xzxx).xy).xxxx).x;
    // 47: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 48: add r0.x, r0.x, -v4.x
    r0.x = ((r0.xxxx)+(-(v4.xxxx))).x;
    // 49: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 50: div r0.x, l(0.100000), |r0.x|
    r0.x = ((float4(0.100000,0.100000,0.100000,0.100000))/(abs(r0.xxxx))).x;
    // 51: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4910Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v1.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v1.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[0].xyxx, r0.zwzz
    r1.x = (dot((source[0].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[1].xyxx, r0.zwzz
    r1.y = (dot((source[1].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[4].zzwz, cb0[5].xxyx
    r0.yz = ((r0.yyzy)*(source[4].zzwz)+(source[5].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v3.z, cb0[5].z
    r0.z = ((v3.zzzz)*(source[5].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v1.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v1.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[2].xyxx, r0.yzyy
    r0.w = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[3].xyxx, r0.yzyy
    r0.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[6].x, l(10.000000), l(10.000000)
    r0.w = ((source[6].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v3.y
    r0.z = ((r0.zzzz)*(v3.yyyy)).z;
    // 25: mul r0.z, r0.z, v2.w
    r0.z = ((r0.zzzz)*(v2.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 30: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 31: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 32: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 33: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 34: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 36: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 37: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 39: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 40: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 41: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 42: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 43: source device depth mapped to centimetre view depth; reconstruction at 45.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 45-48: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 49: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 50: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 51: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 52: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 53: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_localcrack_01_07_tr: 8b228c7b319b544781cca408746673a2; selected map cb7fea6335f99773642abba576c9c60a2c351f25a720c8f239556eb64c8886fd.
float4 ArtistNative4911(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = input.dynamicParameter;
    source[6] = g_ArtistSourceMaterialParameters[3u];
    source[7] = g_ArtistSourceMaterialParameters[5u];
    source[8].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.zw, v4.xxxy, cb0[8].xxxy
    r0.zw = ((v4.xxxy)*(source[8].xxxy)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 5: mad r1.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 9: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 10: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 12: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 13: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 14: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 15: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 16: mul r2.xyz, r0.zzzz, v6.xyzx
    r2.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 17: dp3 r0.z, r1.xyzx, r2.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 18: mul r3.xyz, r0.zzzz, r1.xyzx
    r3.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 19: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 20: dp3 r0.z, r2.xyzx, r3.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 21: add r2.xy, r3.xyxx, cb0[3].xyxx
    r2.xy = ((r3.xyxx)+(source[3].xyxx)).xy;
    // 22: div r2.xy, r2.xyxx, cb0[9].yyyy
    r2.xy = ((r2.xyxx)/(source[9].yyyy)).xy;
    // 23: mad r2.xy, r2.xyxx, cb0[9].zwzz, cb0[10].xyxx
    r2.xy = ((r2.xyxx)*(source[9].zwzz)+(source[10].xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r2.xyw, r2.xyxx, t1.xywz, s1, l(0.000000)
    r2.xyw = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 25: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: add r1.w, -|r0.z|, l(1.000000)
    r1.w = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: mad r0.xy, r1.wwww, cb0[5].xxxx, r0.xyxx
    r0.xy = ((r1.wwww)*(source[5].xxxx)+(r0.xyxx)).xy;
    // 29: add r1.w, cb0[5].y, cb0[5].y
    r1.w = ((source[5].yyyy)+(source[5].yyyy)).w;
    // 30: div r0.xy, r0.xyxx, r1.wwww
    r0.xy = ((r0.xyxx)/(r1.wwww)).xy;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 33: add r4.xyz, -r3.xyzx, r0.xxxx
    r4.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 34: mad r3.xyz, r4.xyzx, l(0.880000, 0.880000, 0.880000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.880000,0.880000,0.880000,0.000000))+(r3.xyzx)).xyz;
    // 35: max r4.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 36: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 37: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 38: mad r3.xyz, -r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000), r3.xyzx
    r3.xyz = ((-(r4.xyzx))*(float4(4.000000,4.000000,4.000000,0.000000))+(r3.xyzx)).xyz;
    // 39: mul r4.xyz, r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000))).xyz;
    // 40: mad_sat r0.xyz, |r0.zzzz|, r3.xyzx, r4.xyzx
    r0.xyz = (saturate((abs(r0.zzzz))*(r3.xyzx)+(r4.xyzx))).xyz;
    // 41: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 42: mul r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))*(abs(r0.wwww))).w;
    // 43: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 44: mul r1.w, |r0.w|, r1.w
    r1.w = ((abs(r0.wwww))*(r1.wwww)).w;
    // 45: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mul r3.xyz, r0.wwww, cb0[7].xyzx
    r3.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 48: mul r0.w, r0.w, l(5.000000)
    r0.w = ((r0.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 49: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mad r0.xyz, r0.xyzx, cb0[6].xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(source[6].xyzx)+(r3.xyzx)).xyz;
    // 51: dp3 r1.w, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: add r3.xyz, -r2.xywx, r1.wwww
    r3.xyz = ((-(r2.xywx))+(r1.wwww)).xyz;
    // 53: mad r2.xyz, cb0[10].zzzz, r3.xyzx, r2.xywx
    r2.xyz = ((source[10].zzzz)*(r3.xyzx)+(r2.xywx)).xyz;
    // 54: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // 57: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 58: mad r0.xyz, r2.xyzx, cb0[4].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[4].xyzx)+(r0.xyzx)).xyz;
    // 59: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 61: log r0.x, r0.w
    r0.x = (log2(r0.wwww)).x;
    // 62: lt r0.y, r0.w, l(0.000001)
    r0.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 64: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 65: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 66: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 67: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 68: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4911Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=input.dynamicParameter; // Original local-mesh distortion dynamic parameter prefix.
    source[1].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[1].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: dp3 r0.x, v4.xyzx, v4.xyzx
    r0.x = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v4.z
    r0.x = ((r0.xxxx)*(v4.zzzz)).x;
    // 4: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 5: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: mul r0.y, |r0.x|, |r0.x|
    r0.y = ((abs(r0.xxxx))*(abs(r0.xxxx))).y;
    // 7: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 8: mul r0.y, r0.y, |r0.x|
    r0.y = ((r0.yyyy)*(abs(r0.xxxx))).y;
    // 9: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: mad r0.y, r0.y, l(0.500000), l(1.000000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: movc r0.x, r0.x, l(1.000000), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.yyyy)).x;
    // 12: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 13: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 14: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
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
// fx_j_pa_slice_01_04_tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 ArtistNative4912(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[6].zzwz, cb0[7].xxyx
    r0.yz = ((r0.yyzy)*(source[6].zzwz)+(source[7].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v4.z, cb0[7].z
    r0.z = ((v4.zzzz)*(source[7].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v2.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v2.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[4].xyxx, r0.yzyy
    r0.w = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[5].xyxx, r0.yzyy
    r0.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[8].x, l(10.000000), l(10.000000)
    r0.w = ((source[8].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 25: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 30: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 37: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 38: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 40: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 41: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 44: mad r0.xz, v2.xxyx, l(-1.000000, 0.000000, 1.000000, 0.000000), l(1.000000, 0.000000, 0.000000, 0.000000)
    r0.xz = ((v2.xxyx)*(float4(-1.000000,0.000000,1.000000,0.000000))+(float4(1.000000,0.000000,0.000000,0.000000))).xz;
    // 45: mad r0.xz, r0.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 46: dp2 r0.x, r0.xzxx, r0.xzxx
    r0.x = (dot((r0.xzxx).xy,(r0.xzxx).xy).xxxx).x;
    // 47: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 48: add r0.x, r0.x, -v4.x
    r0.x = ((r0.xxxx)+(-(v4.xxxx))).x;
    // 49: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 50: div r0.x, l(0.100000), |r0.x|
    r0.x = ((float4(0.100000,0.100000,0.100000,0.100000))/(abs(r0.xxxx))).x;
    // 51: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4912Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v1.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v1.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[0].xyxx, r0.zwzz
    r1.x = (dot((source[0].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[1].xyxx, r0.zwzz
    r1.y = (dot((source[1].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[4].zzwz, cb0[5].xxyx
    r0.yz = ((r0.yyzy)*(source[4].zzwz)+(source[5].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v3.z, cb0[5].z
    r0.z = ((v3.zzzz)*(source[5].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v1.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v1.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[2].xyxx, r0.yzyy
    r0.w = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[3].xyxx, r0.yzyy
    r0.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[6].x, l(10.000000), l(10.000000)
    r0.w = ((source[6].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v3.y
    r0.z = ((r0.zzzz)*(v3.yyyy)).z;
    // 25: mul r0.z, r0.z, v2.w
    r0.z = ((r0.zzzz)*(v2.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 30: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 31: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 32: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 33: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 34: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 36: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 37: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 39: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 40: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 41: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 42: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 43: source device depth mapped to centimetre view depth; reconstruction at 45.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 45-48: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 49: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 50: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 51: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 52: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 53: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_slice_01_01_tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 ArtistNative4913(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[6].zzwz, cb0[7].xxyx
    r0.yz = ((r0.yyzy)*(source[6].zzwz)+(source[7].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v4.z, cb0[7].z
    r0.z = ((v4.zzzz)*(source[7].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v2.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v2.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[4].xyxx, r0.yzyy
    r0.w = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[5].xyxx, r0.yzyy
    r0.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[8].x, l(10.000000), l(10.000000)
    r0.w = ((source[8].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 25: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 30: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 37: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 38: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 40: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 41: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 44: mad r0.xz, v2.xxyx, l(-1.000000, 0.000000, 1.000000, 0.000000), l(1.000000, 0.000000, 0.000000, 0.000000)
    r0.xz = ((v2.xxyx)*(float4(-1.000000,0.000000,1.000000,0.000000))+(float4(1.000000,0.000000,0.000000,0.000000))).xz;
    // 45: mad r0.xz, r0.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 46: dp2 r0.x, r0.xzxx, r0.xzxx
    r0.x = (dot((r0.xzxx).xy,(r0.xzxx).xy).xxxx).x;
    // 47: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 48: add r0.x, r0.x, -v4.x
    r0.x = ((r0.xxxx)+(-(v4.xxxx))).x;
    // 49: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 50: div r0.x, l(0.100000), |r0.x|
    r0.x = ((float4(0.100000,0.100000,0.100000,0.100000))/(abs(r0.xxxx))).x;
    // 51: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4913Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v1.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v1.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[0].xyxx, r0.zwzz
    r1.x = (dot((source[0].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[1].xyxx, r0.zwzz
    r1.y = (dot((source[1].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[4].zzwz, cb0[5].xxyx
    r0.yz = ((r0.yyzy)*(source[4].zzwz)+(source[5].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v3.z, cb0[5].z
    r0.z = ((v3.zzzz)*(source[5].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v1.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v1.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[2].xyxx, r0.yzyy
    r0.w = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[3].xyxx, r0.yzyy
    r0.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[6].x, l(10.000000), l(10.000000)
    r0.w = ((source[6].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v3.y
    r0.z = ((r0.zzzz)*(v3.yyyy)).z;
    // 25: mul r0.z, r0.z, v2.w
    r0.z = ((r0.zzzz)*(v2.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 30: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 31: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 32: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 33: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 34: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 36: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 37: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 39: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 40: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 41: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 42: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 43: source device depth mapped to centimetre view depth; reconstruction at 45.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 45-48: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 49: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 50: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 51: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 52: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 53: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_ring_04_ad: 74031fe490d5d84eb9deafc3fb4f4c98; selected map 5a2c861629a6bbd6b9b467b66de6ec9481566b17a206bbaf3698481526c3143f.
float4 ArtistNative4914(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0799999982, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.200000003, 0.0, 0.0, 0.0))),1u);
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
    // 1: add r0.xy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((v2.xyxx)+(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v2.xxyx, l(0.000000, 1.400000, 1.400000, 0.000000), cb0[3].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.400000,1.400000,0.000000))+(source[3].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: mad r0.xy, r0.xxxx, l(0.050000, 0.050000, 0.000000, 0.000000), v2.xyxx
    r0.xy = ((r0.xxxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v2.xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 8: mul r0.w, r0.x, v3.w
    r0.w = ((r0.xxxx)*(v3.wwww)).w;
    // 9: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 10: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 11: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 12: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 13: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4914Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0799999982, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0500000007, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.200000003, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.400000006, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.349999994, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.150000006, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v1.xyxx, l(1.400000, 1.400000, 0.000000, 0.000000), cb0[3].xyxx
    r0.xy = ((v1.xyxx)*(float4(1.400000,1.400000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.yxzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 3: mul r0.x, r0.x, l(100.000000)
    r0.x = ((r0.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 4: add r0.yz, v1.xxyx, cb0[2].xxyx
    r0.yz = ((v1.xxyx)+(source[2].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.xyzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 6: mad r0.x, r0.y, l(55.000000), r0.x
    r0.x = ((r0.yyyy)*(float4(55.000000,55.000000,55.000000,55.000000))+(r0.xxxx)).x;
    // 7: add r0.yz, v1.xxyx, cb0[4].xxyx
    r0.yz = ((v1.xxyx)+(source[4].xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t3.zxyw, s4, l(0.000000)
    r0.yz = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 9: mad r1.xyzw, r0.yzyz, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r1.xyzw = ((r0.yzyz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 10: mul r0.xyzw, r0.xxxx, r1.xyzw
    r0.xyzw = ((r0.xxxx)*(r1.xyzw)).xyzw;
    // 11: add r1.xy, v1.xyxx, cb0[0].xyxx
    r1.xy = ((v1.xyxx)+(source[0].xyxx)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 13: mad r1.yz, v1.xxyx, l(0.000000, 1.400000, 1.400000, 0.000000), cb0[1].xxyx
    r1.yz = ((v1.xxyx)*(float4(0.000000,1.400000,1.400000,0.000000))+(source[1].xxyx)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t0.xyzw, s1, l(0.000000)
    r1.y = (ArtistNativeSample0((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 15: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 16: mad r1.xy, r1.xxxx, l(0.050000, 0.050000, 0.000000, 0.000000), v1.xyxx
    r1.xy = ((r1.xxxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v1.xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 18: mul r1.x, r1.x, v2.w
    r1.x = ((r1.xxxx)*(v2.wwww)).x;
    // 19: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 20: mad r0.xyzw, r0.xyzw, l(10.000000, -10.000000, 10.000000, -10.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(10.000000,-10.000000,10.000000,-10.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 21: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 22: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 23: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 24: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 25: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 26: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 27: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 28: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 29: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 30: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 31: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 32: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 33: source device depth mapped to centimetre view depth; reconstruction at 35.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 35-38: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 39: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 40: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 41: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 42: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 43: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative4915(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_dark_05_02_tr: 28f1c571ae72fd4481756aab91e9b38f; selected map 0c2fb5f1dacf9a0b644d897a43b50d2148b8a2fd6695bc119a482dcfaea9c230.
float4 ArtistNative4916(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 4: mul r0.y, v4.x, cb0[3].x
    r0.y = ((v4.xxxx)*(source[3].xxxx)).y;
    // 5: max r0.y, r0.y, l(0.000010)
    r0.y = (max(r0.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 6: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 7: mad r0.x, -r0.x, r0.y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mad r0.y, -cb0[3].y, v4.y, l(1.000000)
    r0.y = ((-(source[3].yyyy))*(v4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 9: max r0.y, r0.y, l(0.000010)
    r0.y = (max(r0.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 10: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 11: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 12: mul_sat r0.x, r0.x, cb0[3].z
    r0.x = (saturate((r0.xxxx)*(source[3].zzzz))).x;
    // 13: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 14: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: mul r0.y, r0.y, cb0[3].w
    r0.y = ((r0.yyyy)*(source[3].wwww)).y;
    // 16: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 17: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 18: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 19: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 20: mad r0.xyz, cb0[2].xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((source[2].xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_lensflare_01_05_ad: 55883598a583ae4389d79bb0fdf1d392; selected map ae9ed651c317374306f77e3e999680cc10ae6be4f2b89b76263a55721100b6de.
float4 ArtistNative4917(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t1.wxyz, s1, l(0.000000)
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
    // 26: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 27: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 28: source device depth mapped to centimetre view depth; reconstruction at 30.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 30-33: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 34: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 35: add r1.x, -cb0[5].x, l(1.000000)
    r1.x = ((-(source[5].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 37: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 38: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 39: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 40: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 41: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_flare_03_ad: d6041fd1cc1ab44bae5b05c296b2d90f; selected map e8e8f2d00fec8e1235140f0675cd799a78a85cba5b1966b75552dfd207c86a98.
float4 ArtistNative4918(ARTIST_NATIVE_INPUT input)
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: add r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)+(r0.xyxx)).xy;
    // 3: mad r0.z, -|r0.x|, |r0.y|, l(1.000000)
    r0.z = ((-(abs(r0.xxxx)))*(abs(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 4: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 5: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 9: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 10: mul r0.y, r0.y, l(800.000000)
    r0.y = ((r0.yyyy)*(float4(800.000000,800.000000,800.000000,800.000000))).y;
    // 11: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 12: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 13: mul r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 14: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 15: mul r0.w, r0.w, l(0.150000)
    r0.w = ((r0.wwww)*(float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 16: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mad r0.x, r0.y, l(2.000000), r0.x
    r0.x = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(r0.xxxx)).x;
    // 18: mad r1.yzw, r0.xxxx, v3.xxyz, cb0[1].xxyz
    r1.yzw = ((r0.xxxx)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 19: mul r1.yzw, r1.yyzw, v5.wwww
    r1.yzw = ((r1.yyzw)*(v5.wwww)).yzw;
    // 20: movc r0.xz, r1.xxxx, l(0,0,0,0), r0.wwzw
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwzw)).xz;
    // 21: mad r0.x, r0.z, r0.y, r0.x
    r0.x = ((r0.zzzz)*(r0.yyyy)+(r0.xxxx)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: mul o0.xyz, r0.xxxx, r1.yzwy
    output.xyz = ((r0.xxxx)*(r1.yzwy)).xyz;
    // 25: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_rgbsplit_01_01_ad: d880c361f16370468cfdac8d4688d393; selected map e84de71c52a818919646772cd3b0a0181fe0758eaae2cdb6c89bc69d21826863.
float4 ArtistNative4919(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.xyz, r0.xxxx, l(0.100000, 0.500000, 0.100000, 0.000000)
    r0.xyz = ((r0.xxxx)*(float4(0.100000,0.500000,0.100000,0.000000))).xyz;
    // 4: mul r1.xz, v4.xxxx, l(0.100000, 0.000000, -0.100000, 0.000000)
    r1.xz = ((v4.xxxx)*(float4(0.100000,0.000000,-0.100000,0.000000))).xz;
    // 5: mov r1.yw, l(0,0,0,0)
    r1.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yw;
    // 6: mad r1.xyzw, v2.xyxy, cb0[2].xyxy, r1.xyzw
    r1.xyzw = ((v2.xyxy)*(source[2].xyxy)+(r1.xyzw)).xyzw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 9: mad r0.xyz, r0.wwww, l(0.500000, 0.100000, 0.100000, 0.000000), r0.xyzx
    r0.xyz = ((r0.wwww)*(float4(0.500000,0.100000,0.100000,0.000000))+(r0.xyzx)).xyz;
    // 10: mad r0.xyz, r1.xxxx, l(0.100000, 0.100000, 0.500000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xxxx)*(float4(0.100000,0.100000,0.500000,0.000000))+(r0.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[3].zzzz
    r1.xyz = ((r0.xyzx)*(source[3].zzzz)).xyz;
    // 12: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: mad r0.xyz, -cb0[3].zzzz, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[3].zzzz))*(r0.xyzx)+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[3].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[3].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 15: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 17: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 18: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 19: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_glasshole_02_01_tr: 377108e10f08cc488de94c405e789871; selected map 126881131d06544eff5f47c996bae29975b64dda72346c14c0b7e28b17025d9f.
float4 ArtistNative4920(ARTIST_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[10u];
    source[2] = ArtistNativeAppend((g_ArtistSourceMaterialParameters[4u].xxxx*g_ArtistSourceMaterialTime.xxxx),(g_ArtistSourceMaterialParameters[4u].yyyy*g_ArtistSourceMaterialTime.xxxx),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].xxxx,g_ArtistSourceMaterialParameters[0u].yyyy,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].wwww,g_ArtistSourceMaterialParameters[2u].xxxx,1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)*float4(-0.230000004, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)*float4(0.370000005, 0.0, 0.0, 0.0))),1u);
    source[9] = g_ArtistSourceMaterialParameters[9u];
    source[10] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[11] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))),1u);
    source[12] = g_ArtistSourceMaterialParameters[8u];
    source[13] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[14].x = ((g_ArtistSourceMaterialParameters[2u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].z = ((g_ArtistSourceMaterialParameters[2u].wwww*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[14].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[15].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)*float4(0.370000005, 0.0, 0.0, 0.0)))).x;
    source[15].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[15].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[17].x = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[17].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))).x;
    source[17].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[17].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[18].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[18].w = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[19].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[19].z = ((g_ArtistSourceMaterialParameters[6u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[19].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0)))).x;
    source[20].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[20].y = ((g_ArtistSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0))).x;
    source[20].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.174999997, 0.0, 0.0, 0.0)))).x;
    source[20].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[21].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[21].y = ((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[21].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 27: mul r0.y, r0.y, cb0[17].z
    r0.y = ((r0.yyyy)*(source[17].zzzz)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[17].wwww
    r0.z = (dot((r0.zzzz).xy,(source[17].wwww).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r0.y, r0.y, cb0[18].x
    r0.y = ((r0.yyyy)+(source[18].xxxx)).y;
    // 32: mul r1.x, r0.y, cb0[18].w
    r1.x = ((r0.yyyy)*(source[18].wwww)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, cb0[18].y
    r0.y = ((r0.yyyy)*(source[18].yyyy)).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, v4.w
    r0.y = ((r0.yyyy)*(v4.wwww)).y;
    // 38: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 39: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 41: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 42: add r0.z, v4.z, cb0[18].z
    r0.z = ((v4.zzzz)+(source[18].zzzz)).z;
    // 43: mad r0.y, r0.z, l(-0.400000), r0.y
    r0.y = ((r0.zzzz)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.yyyy)).y;
    // 44: mul r0.z, v4.w, cb0[19].z
    r0.z = ((v4.wwww)*(source[19].zzzz)).z;
    // 45: mad r1.y, r0.y, cb0[19].x, r0.z
    r1.y = ((r0.yyyy)*(source[19].xxxx)+(r0.zzzz)).y;
    // 46: add r0.yz, r1.xxyx, cb0[10].xxyx
    r0.yz = ((r1.xxyx)+(source[10].xxyx)).yz;
    // 47: add r1.xy, r1.xyxx, cb0[11].xyxx
    r1.xy = ((r1.xyxx)+(source[11].xyxx)).xy;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(-1.000000)
    r1.xyz = (ArtistNativeSample4((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s4, l(-1.000000)
    r0.yzw = (ArtistNativeSample3((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 50: mul r2.xyz, r1.xyzx, r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)).xyz;
    // 51: add r0.yzw, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)+(r0.yyzw)).yzw;
    // 52: max r1.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 53: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 54: mul r1.xyz, r1.xyzx, cb0[20].wwww
    r1.xyz = ((r1.xyzx)*(source[20].wwww)).xyz;
    // 55: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 56: add r2.xy, v2.xyxx, -cb0[4].xyxx
    r2.xy = ((v2.xyxx)+(-(source[4].xyxx))).xy;
    // 57: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 58: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 59: min r2.z, |r2.y|, |r2.x|
    r2.z = (min(abs(r2.yyyy),abs(r2.xxxx))).z;
    // 60: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 61: mul r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)*(r1.wwww)).z;
    // 62: mad r2.w, r2.z, l(0.020835), l(-0.085133)
    r2.w = ((r2.zzzz)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).w;
    // 63: mad r2.w, r2.z, r2.w, l(0.180141)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 64: mad r2.w, r2.z, r2.w, l(-0.330299)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).w;
    // 65: mad r2.z, r2.z, r2.w, l(0.999866)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 66: mul r2.w, r1.w, r2.z
    r2.w = ((r1.wwww)*(r2.zzzz)).w;
    // 67: mad r2.w, r2.w, l(-2.000000), l(1.570796)
    r2.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).w;
    // 68: lt r3.x, |r2.y|, |r2.x|
    r3.x = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).x;
    // 69: and r2.w, r2.w, r3.x
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r3.xxxx))).w;
    // 70: mad r1.w, r1.w, r2.z, r2.w
    r1.w = ((r1.wwww)*(r2.zzzz)+(r2.wwww)).w;
    // 71: lt r2.z, r2.y, -r2.y
    r2.z = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).z;
    // 72: and r2.z, r2.z, l(0xc0490fdb)
    r2.z = (asfloat(asuint(r2.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 73: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 74: min r2.z, r2.y, r2.x
    r2.z = (min(r2.yyyy,r2.xxxx)).z;
    // 75: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 76: ge r2.x, r2.x, -r2.x
    r2.x = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).x;
    // 77: lt r2.y, r2.z, -r2.z
    r2.y = (asfloat((uint4)((r2.zzzz)<(-(r2.zzzz))) * 0xffffffffu)).y;
    // 78: and r2.x, r2.x, r2.y
    r2.x = (asfloat(asuint(r2.xxxx) & asuint(r2.yyyy))).x;
    // 79: movc r1.w, r2.x, -r1.w, r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (-(r1.wwww)) : (r1.wwww)).w;
    // 80: mad r2.x, r1.w, l(0.159155), l(0.500000)
    r2.x = ((r1.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 81: mad r3.xy, v2.xyxx, cb0[6].xyxx, cb0[7].xyxx
    r3.xy = ((v2.xyxx)*(source[6].xyxx)+(source[7].xyxx)).xy;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 83: mad r1.w, r1.w, l(2.000000), l(-1.000000)
    r1.w = ((r1.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 84: mad r3.xy, v2.xyxx, cb0[6].xyxx, cb0[8].xyxx
    r3.xy = ((v2.xyxx)*(source[6].xyxx)+(source[8].xyxx)).xy;
    // 85: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r2.w = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 86: mad r2.w, r2.w, l(2.000000), l(-1.000000)
    r2.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 87: add r3.xy, v4.xyxx, l(-0.500000, -0.900000, 0.000000, 0.000000)
    r3.xy = ((v4.xyxx)+(float4(-0.500000,-0.900000,0.000000,0.000000))).xy;
    // 88: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 89: mad r1.w, r3.y, r1.w, r2.w
    r1.w = ((r3.yyyy)*(r1.wwww)+(r2.wwww)).w;
    // 90: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 91: add r3.yz, v2.xxyx, -cb0[5].xxyx
    r3.yz = ((v2.xxyx)+(-(source[5].xxyx))).yz;
    // 92: dp2 r2.w, r3.yzyy, r3.yzyy
    r2.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 93: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 94: mad r3.y, -r2.w, l(2.000000), l(1.000000)
    r3.y = ((-(r2.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 95: mad r2.w, r2.w, l(3.000000), -r3.x
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))+(-(r3.xxxx))).w;
    // 96: max r2.w, r2.w, l(-0.100000)
    r2.w = (max(r2.wwww,float4(-0.100000,-0.100000,-0.100000,-0.100000))).w;
    // 97: min r2.y, r2.w, l(10.000000)
    r2.y = (min(r2.wwww,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 98: mad r3.xy, r3.yyyy, r1.wwww, r2.xyxx
    r3.xy = ((r3.yyyy)*(r1.wwww)+(r2.xyxx)).xy;
    // 99: sample_l_indexable(texture2d)(float,float,float,float) r2.yz, r3.xyxx, t1.yzxw, s2, l(-1.000000)
    r2.yz = (ArtistNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 100: mul r3.xyz, r0.yzwy, r2.zzzz
    r3.xyz = ((r0.yzwy)*(r2.zzzz)).xyz;
    // 101: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 102: mad r1.xyz, cb0[21].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[21].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 103: mad r2.xw, r2.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[13].xxxy
    r2.xw = ((r2.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[13].xxxy)).xw;
    // 104: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xwxx, t6.xyzw, s6, l(0.000000)
    r3.xyz = (ArtistNativeSample5((r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 105: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 106: mul_sat r3.xyz, r3.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 107: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 108: mad r1.xyz, r1.xyzx, cb0[12].xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 109: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 110: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 111: mul r2.xw, r0.wwww, v6.xxxy
    r2.xw = ((r0.wwww)*(v6.xxxy)).xw;
    // 112: mad r0.yz, r0.yyzy, l(0.000000, 0.200000, 0.200000, 0.000000), r2.xxwx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.200000,0.200000,0.000000))+(r2.xxwx)).yz;
    // 113: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t7.wxyz, s7, l(0.000000)
    r0.yzw = (ArtistNativeSample6((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 114: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 115: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 117: mad r0.yzw, cb0[21].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[21].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 118: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 119: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 120: source device depth mapped to centimetre view depth; reconstruction at 122.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 122-125: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 126: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 127: mul_sat r1.xy, r1.xxxx, l(0.034483, 0.066667, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xxxx)*(float4(0.034483,0.066667,0.000000,0.000000)))).xy;
    // 128: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 129: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 130: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 131: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 132: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 133: mul r3.xyz, r1.zzzz, l(10.000000, 10.000000, 20.000000, 0.000000)
    r3.xyz = ((r1.zzzz)*(float4(10.000000,10.000000,20.000000,0.000000))).xyz;
    // 134: movc r1.yzw, r1.yyyy, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 135: mad r0.yzw, r2.yyyy, r0.yyzw, r1.yyzw
    r0.yzw = ((r2.yyyy)*(r0.yyzw)+(r1.yyzw)).yzw;
    // 136: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 137: add r1.y, -r2.y, l(1.000000)
    r1.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 138: add_sat r1.z, r1.y, r2.z
    r1.z = (saturate((r1.yyyy)+(r2.zzzz))).z;
    // 139: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 140: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 141: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 142: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 143: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 144: mul r1.x, r1.x, cb0[14].x
    r1.x = ((r1.xxxx)*(source[14].xxxx)).x;
    // 145: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 146: mul r1.x, r1.x, cb0[14].z
    r1.x = ((r1.xxxx)*(source[14].zzzz)).x;
    // 147: add r1.zw, -v2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((-(v2.xxxy))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 148: mul r1.xz, r1.zzwz, r1.xxxx
    r1.xz = ((r1.zzwz)*(r1.xxxx)).xz;
    // 149: movc r1.xz, r0.xxxx, l(0,0,0,0), r1.xxzx
    r1.xz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxzx)).xz;
    // 150: add r1.xz, r1.xxzx, v2.xxyx
    r1.xz = ((r1.xxzx)+(v2.xxyx)).xz;
    // 151: mad r1.xz, cb0[14].wwww, r1.xxzx, cb0[2].xxyx
    r1.xz = ((source[14].wwww)*(r1.xxzx)+(source[2].xxyx)).xz;
    // 152: add r1.xz, r1.xxzx, cb0[3].xxyx
    r1.xz = ((r1.xxzx)+(source[3].xxyx)).xz;
    // 153: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v2.xyxx, t0.zxyw, s1, l(0.000000)
    r2.yz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 154: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 155: mul r2.yz, r1.yyyy, r2.yyzy
    r2.yz = ((r1.yyyy)*(r2.yyzy)).yz;
    // 156: mad r1.xz, cb0[15].wwww, r2.yyzy, r1.xxzx
    r1.xz = ((source[15].wwww)*(r2.yyzy)+(r1.xxzx)).xz;
    // 157: mad r0.x, cb0[16].x, l(0.500000), l(-0.250000)
    r0.x = ((source[16].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(-0.250000,-0.250000,-0.250000,-0.250000))).x;
    // 158: mad r1.xz, r0.xxxx, r2.xxwx, r1.xxzx
    r1.xz = ((r0.xxxx)*(r2.xxwx)+(r1.xxzx)).xz;
    // 159: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r1.xzxx, t3.xwyz, s3, l(0.000000)
    r1.xzw = (ArtistNativeSample2((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 160: max r1.xzw, |r1.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r1.xzw = (max(abs(r1.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 161: log r1.xzw, r1.xxzw
    r1.xzw = (log2(r1.xxzw)).xzw;
    // 162: mul r1.xzw, r1.xxzw, cb0[16].yyyy
    r1.xzw = ((r1.xxzw)*(source[16].yyyy)).xzw;
    // 163: exp r1.xzw, r1.xxzw
    r1.xzw = (exp2(r1.xxzw)).xzw;
    // 164: mul r2.xyz, r1.xzwx, cb0[16].zzzz
    r2.xyz = ((r1.xzwx)*(source[16].zzzz)).xyz;
    // 165: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 166: mad r1.xzw, -cb0[16].zzzz, r1.xxzw, r0.xxxx
    r1.xzw = ((-(source[16].zzzz))*(r1.xxzw)+(r0.xxxx)).xzw;
    // 167: mad r1.xzw, cb0[16].wwww, r1.xxzw, r2.xxyz
    r1.xzw = ((source[16].wwww)*(r1.xxzw)+(r2.xxyz)).xzw;
    // 168: mul r1.xzw, r1.xxzw, cb0[9].xxyz
    r1.xzw = ((r1.xxzw)*(source[9].xxyz)).xzw;
    // 169: mad r0.xyz, r1.yyyy, r1.xzwx, r0.yzwy
    r0.xyz = ((r1.yyyy)*(r1.xzwx)+(r0.yzwy)).xyz;
    // 170: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 171: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_cubesample_01_02_tr: f548f39885bbfe42ac405ccd46dc2919; selected map aef058acaf07771e340ea3d44475c523831e5b59fa4aa74215f42b6d933ea963.
float4 ArtistNative4921(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = g_ArtistSourceMaterialParameters[1u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.z, cb0[4].x, l(0.100000)
    r0.z = ((source[4].xxxx)*(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 4: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 7: add r0.w, -|r1.z|, l(1.000000)
    r0.w = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 8: mul r2.xy, r0.wwww, r1.yxyy
    r2.xy = ((r0.wwww)*(r1.yxyy)).xy;
    // 9: mad r0.xy, r0.zzzz, r2.xyxx, r0.xyxx
    r0.xy = ((r0.zzzz)*(r2.xyxx)+(r0.xyxx)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.xyz = (Read_EffectSceneColor(LinearClampUVSampler, (r0.xyxx).xy).xyzw).xyz;
    // 11: max r2.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 12: mul r3.xyz, r2.xyzx, r2.xyzx
    r3.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 13: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 14: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 15: mad r0.xyz, -r2.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.xyzx))*(float4(5.000000,5.000000,5.000000,0.000000))+(r0.xyzx)).xyz;
    // 16: mul r2.xyz, r2.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(5.000000,5.000000,5.000000,0.000000))).xyz;
    // 17: mad r0.xyz, |r1.zzzz|, r0.xyzx, r2.xyzx
    r0.xyz = ((abs(r1.zzzz))*(r0.xyzx)+(r2.xyzx)).xyz;
    // 18: mul r1.xy, r1.xyxx, l(3.000000, 3.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(3.000000,3.000000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 21: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 22: mul r1.w, r1.w, l(7.000000)
    r1.w = ((r1.wwww)*(float4(7.000000,7.000000,7.000000,7.000000))).w;
    // 23: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 24: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 25: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 26: mad r0.xyz, cb0[5].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 27: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 28: add r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)+(r1.xyxx)).xy;
    // 29: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 30: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 31: mul r1.zw, r1.zzzw, cb0[5].yyyy
    r1.zw = ((r1.zzzw)*(source[5].yyyy)).zw;
    // 32: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 33: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 34: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 35: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mul r0.w, r0.w, cb0[5].z
    r0.w = ((r0.wwww)*(source[5].zzzz)).w;
    // 37: mul r1.xyz, r0.wwww, cb0[1].xyzx
    r1.xyz = ((r0.wwww)*(source[1].xyzx)).xyz;
    // 38: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 39: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 40: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 42: mov_sat r0.x, cb0[1].w
    r0.x = (saturate(source[1].wwww)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_slice_01_06_tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 ArtistNative4922(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[6].zzwz, cb0[7].xxyx
    r0.yz = ((r0.yyzy)*(source[6].zzwz)+(source[7].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v4.z, cb0[7].z
    r0.z = ((v4.zzzz)*(source[7].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v2.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v2.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[4].xyxx, r0.yzyy
    r0.w = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[5].xyxx, r0.yzyy
    r0.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[8].x, l(10.000000), l(10.000000)
    r0.w = ((source[8].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 25: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 30: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 37: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 38: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 40: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 41: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 44: mad r0.xz, v2.xxyx, l(-1.000000, 0.000000, 1.000000, 0.000000), l(1.000000, 0.000000, 0.000000, 0.000000)
    r0.xz = ((v2.xxyx)*(float4(-1.000000,0.000000,1.000000,0.000000))+(float4(1.000000,0.000000,0.000000,0.000000))).xz;
    // 45: mad r0.xz, r0.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 46: dp2 r0.x, r0.xzxx, r0.xzxx
    r0.x = (dot((r0.xzxx).xy,(r0.xzxx).xy).xxxx).x;
    // 47: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 48: add r0.x, r0.x, -v4.x
    r0.x = ((r0.xxxx)+(-(v4.xxxx))).x;
    // 49: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 50: div r0.x, l(0.100000), |r0.x|
    r0.x = ((float4(0.100000,0.100000,0.100000,0.100000))/(abs(r0.xxxx))).x;
    // 51: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4922Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v1.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v1.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[0].xyxx, r0.zwzz
    r1.x = (dot((source[0].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[1].xyxx, r0.zwzz
    r1.y = (dot((source[1].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[4].zzwz, cb0[5].xxyx
    r0.yz = ((r0.yyzy)*(source[4].zzwz)+(source[5].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v3.z, cb0[5].z
    r0.z = ((v3.zzzz)*(source[5].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v1.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v1.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[2].xyxx, r0.yzyy
    r0.w = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[3].xyxx, r0.yzyy
    r0.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[6].x, l(10.000000), l(10.000000)
    r0.w = ((source[6].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v3.y
    r0.z = ((r0.zzzz)*(v3.yyyy)).z;
    // 25: mul r0.z, r0.z, v2.w
    r0.z = ((r0.zzzz)*(v2.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 30: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 31: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 32: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 33: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 34: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 36: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 37: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 39: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 40: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 41: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 42: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 43: source device depth mapped to centimetre view depth; reconstruction at 45.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 45-48: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 49: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 50: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 51: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 52: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 53: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_localcrack_01_04_tr: 8b228c7b319b544781cca408746673a2; selected map cb7fea6335f99773642abba576c9c60a2c351f25a720c8f239556eb64c8886fd.
float4 ArtistNative4923(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = input.dynamicParameter;
    source[6] = g_ArtistSourceMaterialParameters[3u];
    source[7] = g_ArtistSourceMaterialParameters[5u];
    source[8].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.zw, v4.xxxy, cb0[8].xxxy
    r0.zw = ((v4.xxxy)*(source[8].xxxy)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 5: mad r1.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 9: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 10: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 12: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 13: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 14: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 15: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 16: mul r2.xyz, r0.zzzz, v6.xyzx
    r2.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 17: dp3 r0.z, r1.xyzx, r2.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 18: mul r3.xyz, r0.zzzz, r1.xyzx
    r3.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 19: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 20: dp3 r0.z, r2.xyzx, r3.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 21: add r2.xy, r3.xyxx, cb0[3].xyxx
    r2.xy = ((r3.xyxx)+(source[3].xyxx)).xy;
    // 22: div r2.xy, r2.xyxx, cb0[9].yyyy
    r2.xy = ((r2.xyxx)/(source[9].yyyy)).xy;
    // 23: mad r2.xy, r2.xyxx, cb0[9].zwzz, cb0[10].xyxx
    r2.xy = ((r2.xyxx)*(source[9].zwzz)+(source[10].xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r2.xyw, r2.xyxx, t1.xywz, s1, l(0.000000)
    r2.xyw = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 25: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: add r1.w, -|r0.z|, l(1.000000)
    r1.w = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: mad r0.xy, r1.wwww, cb0[5].xxxx, r0.xyxx
    r0.xy = ((r1.wwww)*(source[5].xxxx)+(r0.xyxx)).xy;
    // 29: add r1.w, cb0[5].y, cb0[5].y
    r1.w = ((source[5].yyyy)+(source[5].yyyy)).w;
    // 30: div r0.xy, r0.xyxx, r1.wwww
    r0.xy = ((r0.xyxx)/(r1.wwww)).xy;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 33: add r4.xyz, -r3.xyzx, r0.xxxx
    r4.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 34: mad r3.xyz, r4.xyzx, l(0.880000, 0.880000, 0.880000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.880000,0.880000,0.880000,0.000000))+(r3.xyzx)).xyz;
    // 35: max r4.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 36: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 37: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 38: mad r3.xyz, -r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000), r3.xyzx
    r3.xyz = ((-(r4.xyzx))*(float4(4.000000,4.000000,4.000000,0.000000))+(r3.xyzx)).xyz;
    // 39: mul r4.xyz, r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000))).xyz;
    // 40: mad_sat r0.xyz, |r0.zzzz|, r3.xyzx, r4.xyzx
    r0.xyz = (saturate((abs(r0.zzzz))*(r3.xyzx)+(r4.xyzx))).xyz;
    // 41: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 42: mul r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))*(abs(r0.wwww))).w;
    // 43: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 44: mul r1.w, |r0.w|, r1.w
    r1.w = ((abs(r0.wwww))*(r1.wwww)).w;
    // 45: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mul r3.xyz, r0.wwww, cb0[7].xyzx
    r3.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 48: mul r0.w, r0.w, l(5.000000)
    r0.w = ((r0.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 49: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mad r0.xyz, r0.xyzx, cb0[6].xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(source[6].xyzx)+(r3.xyzx)).xyz;
    // 51: dp3 r1.w, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: add r3.xyz, -r2.xywx, r1.wwww
    r3.xyz = ((-(r2.xywx))+(r1.wwww)).xyz;
    // 53: mad r2.xyz, cb0[10].zzzz, r3.xyzx, r2.xywx
    r2.xyz = ((source[10].zzzz)*(r3.xyzx)+(r2.xywx)).xyz;
    // 54: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // 57: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 58: mad r0.xyz, r2.xyzx, cb0[4].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[4].xyzx)+(r0.xyzx)).xyz;
    // 59: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 61: log r0.x, r0.w
    r0.x = (log2(r0.wwww)).x;
    // 62: lt r0.y, r0.w, l(0.000001)
    r0.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 64: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 65: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 66: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 67: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 68: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4923Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=input.dynamicParameter; // Original local-mesh distortion dynamic parameter prefix.
    source[1].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[1].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: dp3 r0.x, v4.xyzx, v4.xyzx
    r0.x = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v4.z
    r0.x = ((r0.xxxx)*(v4.zzzz)).x;
    // 4: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 5: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: mul r0.y, |r0.x|, |r0.x|
    r0.y = ((abs(r0.xxxx))*(abs(r0.xxxx))).y;
    // 7: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 8: mul r0.y, r0.y, |r0.x|
    r0.y = ((r0.yyyy)*(abs(r0.xxxx))).y;
    // 9: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: mad r0.y, r0.y, l(0.500000), l(1.000000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: movc r0.x, r0.x, l(1.000000), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.yyyy)).x;
    // 12: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 13: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 14: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_03_05_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 ArtistNative4924(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4924Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_j_me_linearflow_02_05_tr: 3171a915fc9cfe47a00d080d43a28620; selected map 3fea0200be542b186c1b892313f07f1b777387c12cb82b23dee161796533a943.
float4 ArtistNative4925(ARTIST_NATIVE_INPUT input)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[15u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[7] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)+g_ArtistSourceMaterialParameters[1u].xxxx),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)+g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[8] = g_ArtistSourceMaterialParameters[12u];
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].zzzz,g_ArtistSourceMaterialParameters[6u].wwww,1u);
    source[12] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].wwww)+g_ArtistSourceMaterialParameters[5u].yyyy),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)+g_ArtistSourceMaterialParameters[5u].zzzz),1u);
    source[13] = g_ArtistSourceMaterialParameters[13u];
    source[14].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[16].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[17].z = (floor(g_ArtistSourceMaterialParameters[4u].yyyy)).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[18].w = ((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[19].x = (sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[19].z = (cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].w = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[20].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[20].y = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[20].z = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[20].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[21].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[21].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[21].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[21].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[22].x = (floor(g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[22].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[22].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[22].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[23].x = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[23].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[23].z = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[23].w = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[24].x = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
    source[24].y = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[24].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[24].w = (g_ArtistSourceMaterialParameters[10u].wwww).x;
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
    // 1: mad r0.xy, v4.wzww, cb0[11].xyxx, cb0[12].xyxx
    r0.xy = ((v4.wzww)*(source[11].xyxx)+(source[12].xyxx)).xy;
    // 2: add r1.xyzw, r0.xyxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r1.xyzw = ((r0.xyxy)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r0.z = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 6: add r1.xy, -r0.xxxx, r0.yzyy
    r1.xy = ((-(r0.xxxx))+(r0.yzyy)).xy;
    // 7: mul r0.xy, r1.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 8: mov r0.z, l(0)
    r0.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 9: add r0.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 10: dp3 r0.z, r0.xyzx, r0.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 11: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 12: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 13: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: add r0.z, cb0[3].y, cb0[20].w
    r0.z = ((source[3].yyyy)+(source[20].wwww)).z;
    // 15: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mad r1.zw, cb0[19].wwww, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[19].wwww)*(r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 17: mad r2.y, r1.w, cb0[20].y, r0.z
    r2.y = ((r1.wwww)*(source[20].yyyy)+(r0.zzzz)).y;
    // 18: mad r2.x, r1.z, cb0[20].x, cb0[20].z
    r2.x = ((r1.zzzz)*(source[20].xxxx)+(source[20].zzzz)).x;
    // 19: add r0.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 20: dp2 r2.x, cb0[9].xyxx, r0.zwzz
    r2.x = (dot((source[9].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 21: dp2 r2.y, cb0[10].xyxx, r0.zwzz
    r2.y = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 22: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 23: mul r1.z, cb0[3].w, cb0[21].z
    r1.z = ((source[3].wwww)*(source[21].zzzz)).z;
    // 24: mad r0.xy, r1.zzzz, r0.xyxx, r0.zwzz
    r0.xy = ((r1.zzzz)*(r0.xyxx)+(r0.zwzz)).xy;
    // 25: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s3, l(-1.000000)
    r0.xyz = (ArtistNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 26: mad r0.xyzw, cb0[22].xxxx, -r0.xxyz, r0.xxyz
    r0.xyzw = ((source[22].xxxx)*(-(r0.xxyz))+(r0.xxyz)).xyzw;
    // 27: mul r0.xyzw, r0.xyzw, cb0[22].yyyy
    r0.xyzw = ((r0.xyzw)*(source[22].yyyy)).xyzw;
    // 28: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 30: mul r0.xyzw, r0.xyzw, cb0[22].zzzz
    r0.xyzw = ((r0.xyzw)*(source[22].zzzz)).xyzw;
    // 31: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 32: dp3 r1.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 33: add r2.xyz, -r0.yzwy, r1.zzzz
    r2.xyz = ((-(r0.yzwy))+(r1.zzzz)).xyz;
    // 34: mad r0.yzw, cb0[22].wwww, r2.xxyz, r0.yyzw
    r0.yzw = ((source[22].wwww)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 35: mad r1.zw, v4.xxxy, cb0[14].xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((v4.xxxy)*(source[14].xxxy)+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 36: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t4.xyzw, s0, l(-1.000000)
    r2.xyz = (ArtistNativeSample0((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 37: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 38: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 39: mad r2.xyz, cb0[14].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[14].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 40: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 41: mul r0.yzw, r0.yyzw, cb0[13].xxyz
    r0.yzw = ((r0.yyzw)*(source[13].xxyz)).yzw;
    // 42: mad r1.zw, v4.wwwz, cb0[6].xxxy, cb0[7].xxxy
    r1.zw = ((v4.wwwz)*(source[6].xxxy)+(source[7].xxxy)).zw;
    // 43: add r3.xyzw, r1.zwzw, l(0.200000, 0.000000, 0.000000, 0.200000)
    r3.xyzw = ((r1.zwzw)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t0.yzxw, s2, l(0.000000)
    r1.z = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s2, l(0.000000)
    r1.w = (ArtistNativeSample2((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.zwzz, t0.yzwx, s2, l(0.000000)
    r2.w = (ArtistNativeSample2((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 47: add r3.y, -r1.z, r2.w
    r3.y = ((-(r1.zzzz))+(r2.wwww)).y;
    // 48: add r3.x, -r1.z, r1.w
    r3.x = ((-(r1.zzzz))+(r1.wwww)).x;
    // 49: mul r3.xy, r3.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 50: mov r3.z, l(0)
    r3.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 51: add r3.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 52: dp3 r1.z, r3.xyzx, r3.xyzx
    r1.z = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 53: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 54: div r1.zw, r3.xxxy, r1.zzzz
    r1.zw = ((r3.xxxy)/(r1.zzzz)).zw;
    // 55: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 56: add r2.w, cb0[3].x, cb0[16].x
    r2.w = ((source[3].xxxx)+(source[16].xxxx)).w;
    // 57: mad r3.xy, cb0[15].xxxx, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((source[15].xxxx)*(r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 59: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 60: mad r4.y, r3.y, cb0[15].z, r2.w
    r4.y = ((r3.yyyy)*(source[15].zzzz)+(r2.wwww)).y;
    // 61: mad r4.x, r3.x, cb0[15].y, cb0[15].w
    r4.x = ((r3.xxxx)*(source[15].yyyy)+(source[15].wwww)).x;
    // 62: add r3.xy, r4.xyxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 63: dp2 r4.x, cb0[4].xyxx, r3.xyxx
    r4.x = (dot((source[4].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 64: dp2 r4.y, cb0[5].xyxx, r3.xyxx
    r4.y = (dot((source[5].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 65: add r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 66: mul r1.y, cb0[3].z, cb0[17].x
    r1.y = ((source[3].zzzz)*(source[17].xxxx)).y;
    // 67: mad r1.yz, r1.yyyy, r1.zzwz, r3.xxyx
    r1.yz = ((r1.yyyy)*(r1.zzwz)+(r3.xxyx)).yz;
    // 68: sample_l_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t1.wxyz, s1, l(-1.000000)
    r1.yzw = (ArtistNativeSample1((r1.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 69: mad r3.xyzw, cb0[17].zzzz, -r1.yyzw, r1.yyzw
    r3.xyzw = ((source[17].zzzz)*(-(r1.yyzw))+(r1.yyzw)).xyzw;
    // 70: mul r3.xyzw, r3.xyzw, cb0[17].wwww
    r3.xyzw = ((r3.xyzw)*(source[17].wwww)).xyzw;
    // 71: max r3.xyzw, |r3.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r3.xyzw = (max(abs(r3.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 72: log r3.xyzw, r3.xyzw
    r3.xyzw = (log2(r3.xyzw)).xyzw;
    // 73: mul r3.xyzw, r3.xyzw, cb0[18].xxxx
    r3.xyzw = ((r3.xyzw)*(source[18].xxxx)).xyzw;
    // 74: exp r3.xyzw, r3.xyzw
    r3.xyzw = (exp2(r3.xyzw)).xyzw;
    // 75: dp3 r1.y, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 76: add r1.yzw, -r3.yyzw, r1.yyyy
    r1.yzw = ((-(r3.yyzw))+(r1.yyyy)).yzw;
    // 77: mad r1.yzw, cb0[18].yyyy, r1.yyzw, r3.yyzw
    r1.yzw = ((source[18].yyyy)*(r1.yyzw)+(r3.yyzw)).yzw;
    // 78: mul r1.yzw, r2.xxyz, r1.yyzw
    r1.yzw = ((r2.xxyz)*(r1.yyzw)).yzw;
    // 79: mad r0.yzw, r1.yyzw, cb0[8].xxyz, r0.yyzw
    r0.yzw = ((r1.yyzw)*(source[8].xxyz)+(r0.yyzw)).yzw;
    // 80: mul r0.yzw, r0.yyzw, cb0[23].xxxx
    r0.yzw = ((r0.yyzw)*(source[23].xxxx)).yzw;
    // 81: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 82: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 83: mul r0.yzw, r0.yyzw, cb0[23].yyyy
    r0.yzw = ((r0.yyzw)*(source[23].yyyy)).yzw;
    // 84: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 85: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 86: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 87: log r0.y, |r1.x|
    r0.y = (log2(abs(r1.xxxx))).y;
    // 88: lt r0.z, |r1.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 89: mad r0.w, cb0[24].y, l(10.000000), l(10.000000)
    r0.w = ((source[24].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 90: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 91: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 92: mul r0.y, r0.y, cb0[24].z
    r0.y = ((r0.yyyy)*(source[24].zzzz)).y;
    // 93: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 94: add r0.x, r0.x, -r3.x
    r0.x = ((r0.xxxx)+(-(r3.xxxx))).x;
    // 95: mad r0.x, cb0[23].z, r0.x, r3.x
    r0.x = ((source[23].zzzz)*(r0.xxxx)+(r3.xxxx)).x;
    // 96: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 97: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 98: mul r0.x, r0.x, cb0[23].w
    r0.x = ((r0.xxxx)*(source[23].wwww)).x;
    // 99: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 100: mul r0.x, r0.x, cb0[24].x
    r0.x = ((r0.xxxx)*(source[24].xxxx)).x;
    // 101: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 102: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 103: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 104: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 105: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 106: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 107: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 108: mul r0.z, r0.z, cb0[24].w
    r0.z = ((r0.zzzz)*(source[24].wwww)).z;
    // 109: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 110: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 111: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 112: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4925Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = input.dynamicParameter;
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[5] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)+g_ArtistSourceMaterialParameters[1u].xxxx),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)+g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[6] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].zzzz,g_ArtistSourceMaterialParameters[6u].wwww,1u);
    source[9] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].wwww)+g_ArtistSourceMaterialParameters[5u].yyyy),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)+g_ArtistSourceMaterialParameters[5u].zzzz),1u);
    source[10].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (floor(g_ArtistSourceMaterialParameters[4u].yyyy)).x;
    source[12].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[13].w = ((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[14].x = (sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[14].z = (cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].w = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[15].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[17].x = (floor(g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[17].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[17].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[18].x = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[18].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[18].z = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[19].x = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
    source[19].y = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[19].w = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[20].x = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mad r0.xy, v2.wzww, cb0[8].xyxx, cb0[9].xyxx
    r0.xy = ((v2.wzww)*(source[8].xyxx)+(source[9].xyxx)).xy;
    // 2: add r1.xyzw, r0.xyxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r1.xyzw = ((r0.xyxy)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r0.z = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 6: add r1.xy, -r0.xxxx, r0.yzyy
    r1.xy = ((-(r0.xxxx))+(r0.yzyy)).xy;
    // 7: mul r0.xy, r1.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 8: mov r0.z, l(0)
    r0.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 9: add r0.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 10: dp3 r0.z, r0.xyzx, r0.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 11: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 12: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 13: mad r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: add r0.zw, cb0[1].xxxy, l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((source[1].xxxy)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 15: add r0.w, r0.w, cb0[15].w
    r0.w = ((r0.wwww)+(source[15].wwww)).w;
    // 16: add r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)+(source[11].xxxx)).z;
    // 17: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 18: mad r1.zw, cb0[14].wwww, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[14].wwww)*(r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 19: mad r2.y, r1.w, cb0[15].y, r0.w
    r2.y = ((r1.wwww)*(source[15].yyyy)+(r0.wwww)).y;
    // 20: mad r2.x, r1.z, cb0[15].x, cb0[15].z
    r2.x = ((r1.zzzz)*(source[15].xxxx)+(source[15].zzzz)).x;
    // 21: add r1.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 22: dp2 r2.x, cb0[6].xyxx, r1.zwzz
    r2.x = (dot((source[6].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 23: dp2 r2.y, cb0[7].xyxx, r1.zwzz
    r2.y = (dot((source[7].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 24: add r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 25: mul r0.w, cb0[1].w, cb0[16].z
    r0.w = ((source[1].wwww)*(source[16].zzzz)).w;
    // 26: mad r0.xy, r0.wwww, r0.xyxx, r1.zwzz
    r0.xy = ((r0.wwww)*(r0.xyxx)+(r1.zwzz)).xy;
    // 27: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t3.xyzw, s3, l(-1.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 28: mad r2.xyzw, cb0[17].xxxx, -r0.xyxy, r0.xyxy
    r2.xyzw = ((source[17].xxxx)*(-(r0.xyxy))+(r0.xyxy)).xyzw;
    // 29: mul r2.xyzw, r2.xyzw, cb0[17].yyyy
    r2.xyzw = ((r2.xyzw)*(source[17].yyyy)).xyzw;
    // 30: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 31: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 32: mul r2.xyzw, r2.xyzw, cb0[17].zzzz
    r2.xyzw = ((r2.xyzw)*(source[17].zzzz)).xyzw;
    // 33: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 34: mad r0.xy, cb0[10].xxxx, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[10].xxxx)*(r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 36: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mad r1.y, r0.y, cb0[10].z, r0.z
    r1.y = ((r0.yyyy)*(source[10].zzzz)+(r0.zzzz)).y;
    // 38: mad r1.x, r0.x, cb0[10].y, cb0[10].w
    r1.x = ((r0.xxxx)*(source[10].yyyy)+(source[10].wwww)).x;
    // 39: add r0.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 40: dp2 r1.x, cb0[2].xyxx, r0.xyxx
    r1.x = (dot((source[2].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 41: dp2 r1.y, cb0[3].xyxx, r0.xyxx
    r1.y = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 42: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 43: mad r1.xy, v2.wzww, cb0[4].xyxx, cb0[5].xyxx
    r1.xy = ((v2.wzww)*(source[4].xyxx)+(source[5].xyxx)).xy;
    // 44: add r3.xyzw, r1.xyxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r3.xyzw = ((r1.xyxy)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t0.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r3.xyxx, t0.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r3.zwzz, t0.yxzw, s2, l(0.000000)
    r1.y = (ArtistNativeSample2((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 48: add r3.xy, -r0.zzzz, r1.xyxx
    r3.xy = ((-(r0.zzzz))+(r1.xyxx)).xy;
    // 49: mul r1.xy, r3.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r3.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 50: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 51: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 52: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 53: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 54: div r1.xy, r1.xyxx, r0.zzzz
    r1.xy = ((r1.xyxx)/(r0.zzzz)).xy;
    // 55: mad r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: mul r0.z, cb0[1].z, cb0[12].x
    r0.z = ((source[1].zzzz)*(source[12].xxxx)).z;
    // 57: mad r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((r0.zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 58: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(-1.000000)
    r0.xy = (ArtistNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 59: mad r1.xyzw, cb0[12].zzzz, -r0.xyxy, r0.xyxy
    r1.xyzw = ((source[12].zzzz)*(-(r0.xyxy))+(r0.xyxy)).xyzw;
    // 60: mul r1.xyzw, r1.xyzw, cb0[12].wwww
    r1.xyzw = ((r1.xyzw)*(source[12].wwww)).xyzw;
    // 61: max r1.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 62: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 63: mul r1.xyzw, r1.xyzw, cb0[13].xxxx
    r1.xyzw = ((r1.xyzw)*(source[13].xxxx)).xyzw;
    // 64: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 65: add r2.xyzw, -r1.zwzw, r2.xyzw
    r2.xyzw = ((-(r1.zwzw))+(r2.xyzw)).xyzw;
    // 66: mad r1.xyzw, cb0[18].zzzz, r2.xyzw, r1.xyzw
    r1.xyzw = ((source[18].zzzz)*(r2.xyzw)+(r1.xyzw)).xyzw;
    // 67: max r1.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 68: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 69: mul r1.xyzw, r1.xyzw, cb0[18].wwww
    r1.xyzw = ((r1.xyzw)*(source[18].wwww)).xyzw;
    // 70: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 71: mul r1.xyzw, r1.xyzw, cb0[19].xxxx
    r1.xyzw = ((r1.xyzw)*(source[19].xxxx)).xyzw;
    // 72: log r0.x, |r0.w|
    r0.x = (log2(abs(r0.wwww))).x;
    // 73: lt r0.y, |r0.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: mad r0.z, cb0[19].y, l(10.000000), l(10.000000)
    r0.z = ((source[19].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 75: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 76: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 77: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 78: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 79: mul r0.xyzw, r1.xyzw, r0.xxxx
    r0.xyzw = ((r1.xyzw)*(r0.xxxx)).xyzw;
    // 80: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
    // 81: mul r0.xyzw, r0.xyzw, cb0[20].xxxx
    r0.xyzw = ((r0.xyzw)*(source[20].xxxx)).xyzw;
    // 82: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 83: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 84: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 85: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 86: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 87: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 88: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 89: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 90: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 91: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 92: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 93: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 94: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 95: source device depth mapped to centimetre view depth; reconstruction at 97.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 97-100: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 101: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 102: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 103: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 104: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 105: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_me_swing_01_1_tr: 0cbb7a45d8aa1a4ca69a1907b36fff29; selected map 19e4fda4abd98b350bd102a7bd1c5399c2104033636b3652c9c162016ef55aee.
float4 ArtistNative4926(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[4] = input.dynamicParameter;
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
    // 1: add r0.xyzw, v4.xyyx, l(-0.020000, -0.100000, -1.000000, -0.050000)
    r0.xyzw = ((v4.xyyx)+(float4(-0.020000,-0.100000,-1.000000,-0.050000))).xyzw;
    // 2: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 3: mul r1.x, r1.x, cb0[4].x
    r1.x = ((r1.xxxx)*(source[4].xxxx)).x;
    // 4: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 5: mad_sat r1.y, -r0.z, l(20.000000), l(1.000000)
    r1.y = (saturate((-(r0.zzzz))*(float4(20.000000,20.000000,20.000000,20.000000))+(float4(1.000000,1.000000,1.000000,1.000000)))).y;
    // 6: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 7: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 8: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 9: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 10: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 11: mul r0.xz, r0.xxzx, l(10.000000, 0.000000, 10.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(10.000000,0.000000,10.000000,0.000000))).xz;
    // 12: mul r1.x, |r0.z|, |r0.z|
    r1.x = ((abs(r0.zzzz))*(abs(r0.zzzz))).x;
    // 13: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 15: add r0.xw, -|r0.xxxw|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r0.xw = ((-(abs(r0.xxxw)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // 16: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 17: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 18: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 19: mul r1.x, |r0.w|, r1.x
    r1.x = ((abs(r0.wwww))*(r1.xxxx)).x;
    // 20: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 21: mul r1.yzw, v4.xxxy, l(0.000000, 10.000000, 1.500000, 1.000000)
    r1.yzw = ((v4.xxxy)*(float4(0.000000,10.000000,1.500000,1.000000))).yzw;
    // 22: mov_sat r1.y, r1.y
    r1.y = (saturate(r1.yyyy)).y;
    // 23: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 24: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 25: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 26: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 27: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 30: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: mul r1.x, r1.x, l(15.000000)
    r1.x = ((r1.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 32: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 33: mul r1.x, r1.x, cb0[4].y
    r1.x = ((r1.xxxx)*(source[4].yyyy)).x;
    // 34: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 35: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 36: mad r0.x, r0.x, r0.z, r0.w
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.wwww)).x;
    // 37: mad r0.yz, v4.xxyx, l(0.000000, 0.500000, 0.300000, 0.000000), cb0[3].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,0.500000,0.300000,0.000000))+(source[3].xxyx)).yz;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 39: mad r0.yz, r0.yyyy, l(0.000000, 0.100000, 0.100000, 0.000000), r1.zzwz
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.100000,0.100000,0.000000))+(r1.zzwz)).yz;
    // 40: add r0.yz, r0.yyzy, l(0.000000, 0.000000, 0.050000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.000000,0.050000,0.000000))).yz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 42: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 43: mad r0.y, r0.y, l(100.000000), r0.x
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))+(r0.xxxx)).y;
    // 44: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 45: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 46: mad r0.xyz, r0.yyyy, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.yyyy)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 47: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4926Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 2: add r0.xy, -|r0.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(abs(r0.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 3: mul_sat r0.xy, r0.xyxx, l(5.000000, 5.000000, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xyxx)*(float4(5.000000,5.000000,0.000000,0.000000)))).xy;
    // 4: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 5: add r0.y, v2.y, l(-0.100000)
    r0.y = ((v2.yyyy)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).y;
    // 6: mul r0.z, |r0.y|, |r0.y|
    r0.z = ((abs(r0.yyyy))*(abs(r0.yyyy))).z;
    // 7: mul r0.z, r0.z, |r0.y|
    r0.z = ((r0.zzzz)*(abs(r0.yyyy))).z;
    // 8: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 9: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 10: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 11: mad r0.yz, v2.xxyx, l(0.000000, 0.500000, 0.300000, 0.000000), cb0[1].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,0.500000,0.300000,0.000000))+(source[1].xxyx)).yz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 13: mul r0.zw, v2.xxxy, l(0.000000, 0.000000, 1.500000, 1.000000)
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,1.500000,1.000000))).zw;
    // 14: mad r0.yz, r0.yyyy, l(0.000000, 0.100000, 0.100000, 0.000000), r0.zzwz
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.100000,0.100000,0.000000))+(r0.zzwz)).yz;
    // 15: add r0.yz, r0.yyzy, l(0.000000, 0.000000, 0.050000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,0.000000,0.050000,0.000000))).yz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 17: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 18: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 19: mad r0.xyzw, r0.xxxx, l(20.000000, -20.000000, 20.000000, -20.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(20.000000,-20.000000,20.000000,-20.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 20: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 21: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 22: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 23: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 24: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 25: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 26: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 27: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 28: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 29: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 30: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 31: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 32: source device depth mapped to centimetre view depth; reconstruction at 34.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 34-37: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 38: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 39: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 40: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 41: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 42: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_me_makeflow_03_10_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 ArtistNative4927(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4927Distortion(ARTIST_NATIVE_INPUT input)
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
