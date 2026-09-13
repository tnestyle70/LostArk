// Original Kouku material programs 3328..3391; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_zoomblur_01_tr: 3fc4c0de7f119c49b1e0478e97872fc9; selected map d759fa1ad738c7c8c48ad5775499deb1dafee1b7c91903e48fb751adeb6a9a0d.
float4 ArtistNative3328(ARTIST_NATIVE_INPUT input)
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
    // 10: add r2.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r2.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 11: mul r1.xy, r2.xyxx, r2.xyxx
    r1.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 12: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 16: mul r0.w, r0.w, cb0[2].x
    r0.w = ((r0.wwww)*(source[2].xxxx)).w;
    // 17: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 18: mul_sat r0.w, r0.w, cb0[2].y
    r0.w = (saturate((r0.wwww)*(source[2].yyyy))).w;
    // 19: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 20: mul r1.x, r0.w, v3.w
    r1.x = ((r0.wwww)*(v3.wwww)).x;
    // 21: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 22: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 23: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 24: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r1.xyxx, t0.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r3.xyz = (Read_EffectSceneColor(LinearClampUVSampler, (r1.xyxx).xy).xyzw).xyz;
    // 25: mul r1.w, v4.x, l(-0.010000)
    r1.w = ((v4.xxxx)*(float4(-0.010000,-0.010000,-0.010000,-0.010000))).w;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r1.xyxx, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 27: dp2 r2.x, r2.zwzz, r2.zwzz
    r2.x = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).x;
    // 28: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 29: mul r5.xy, r2.xxxx, r2.zwzz
    r5.xy = ((r2.xxxx)*(r2.zwzz)).xy;
    // 30: mad r2.xy, -r2.zwzz, r2.xxxx, r2.zwzz
    r2.xy = ((-(r2.zwzz))*(r2.xxxx)+(r2.zwzz)).xy;
    // 31: mad r2.xy, r2.xyxx, l(0.900000, 0.900000, 0.000000, 0.000000), r5.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.900000,0.900000,0.000000,0.000000))+(r5.xyxx)).xy;
    // 32: mul r2.xy, r1.wwww, r2.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)).xy;
    // 33: mad r2.xy, r2.xyxx, l(2.000000, -2.000000, 0.000000, 0.000000), l(-1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,-2.000000,0.000000,0.000000))+(float4(-1.000000,1.000000,0.000000,0.000000))).xy;
    // 34: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 35: mov r5.xyz, r4.xyzx
    r5.xyz = (r4.xyzx).xyz;
    // 36: mov r2.zw, r1.xxxy
    r2.zw = (r1.xxxy).zw;
    // 37: mov r1.w, l(0)
    r1.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 38: loop
    [loop] while (true) {
    // 39: ge r3.w, r1.w, l(3.000000)
    r3.w = (asfloat((uint4)((r1.wwww)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 40: breakc_nz r3.w
    if ((asuint(r3.wwww)).x != 0u) break;
    // 41: add r2.zw, r2.xxxy, r2.zzzw
    r2.zw = ((r2.xxxy)+(r2.zzzw)).zw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r6.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 43: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 44: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: endloop
    }
    // 46: mad r1.xyw, r5.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000), -r3.xyxz
    r1.xyw = ((r5.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))+(-(r3.xyxz))).xyw;
    // 47: mul r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)*(r1.xyxw)).xyw;
    // 48: mad r1.xyw, cb0[2].zzzz, r1.xyxw, r3.xyxz
    r1.xyw = ((source[2].zzzz)*(r1.xyxw)+(r3.xyxz)).xyw;
    // 49: mad r1.xyw, v3.xyxz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((v3.xyxz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 50: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_q_grass_02: 3c75890c1e848a43ae2e96d93272d7a5; selected map b44c47d98eeef6b31ae244a317bffe98b35a7e3a4e6ecbfdcb97d8da1c3d18ed.
float4 ArtistNative3329(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[0u];
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 3: mad o0.xyz, r0.xyzx, v3.xyzx, cb0[0].xyzx
    output.xyz = ((r0.xyzx)*(v3.xyzx)+(source[0].xyzx)).xyz;
    // 4: mov_sat r0.w, r0.w
    r0.w = (saturate(r0.wwww)).w;
    // 5: add r0.x, r0.w, l(-0.333300)
    r0.x = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 6: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 7: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 8: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 9: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_me_field_02_02_tr: 99ec8cbb3d88554d889d51492df33758; selected map 884219491cceffdcd8ff6b9f270ae6e50ae734850c77830e09cabc76aeb22cfc.
float4 ArtistNative3330(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4] = g_ArtistSourceMaterialParameters[5u];
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[9].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
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
    // 10: add r0.y, -cb0[9].x, l(1.000000)
    r0.y = ((-(source[9].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 15: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 16: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 17: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 18: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[8].x
    r0.z = ((r0.zzzz)*(source[8].xxxx)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul_sat r0.z, r0.z, cb0[8].y
    r0.z = (saturate((r0.zzzz)*(source[8].yyyy))).z;
    // 22: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 23: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 24: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 25: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 26: mul r0.xy, v4.xyxx, cb0[5].zwzz
    r0.xy = ((v4.xyxx)*(source[5].zwzz)).xy;
    // 27: mad r1.x, cb0[5].y, cb0[5].x, r0.x
    r1.x = ((source[5].yyyy)*(source[5].xxxx)+(r0.xxxx)).x;
    // 28: mad r1.y, cb0[5].y, cb0[6].x, r0.y
    r1.y = ((source[5].yyyy)*(source[6].xxxx)+(r0.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: mul r1.xy, v4.xyxx, cb0[6].zwzz
    r1.xy = ((v4.xyxx)*(source[6].zwzz)).xy;
    // 31: mad r2.x, cb0[5].y, cb0[6].y, r1.x
    r2.x = ((source[5].yyyy)*(source[6].yyyy)+(r1.xxxx)).x;
    // 32: mad r2.y, cb0[5].y, cb0[7].x, r1.y
    r2.y = ((source[5].yyyy)*(source[7].xxxx)+(r1.yyyy)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 35: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 36: mad r0.xyz, -r0.xyzx, r1.xyzx, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(r1.xyzx)+(r0.wwww)).xyz;
    // 37: mad r0.xyz, cb0[7].yyyy, r0.xyzx, r2.xyzx
    r0.xyz = ((source[7].yyyy)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 38: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 39: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 40: mul r0.xyz, r0.xyzx, cb0[7].zzzz
    r0.xyz = ((r0.xyzx)*(source[7].zzzz)).xyz;
    // 41: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 42: mul_sat r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = (saturate((r0.xyzx)*(source[7].wwww))).xyz;
    // 43: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 44: mad r2.xyz, cb0[4].wwww, cb0[4].xyzx, -r1.xyzx
    r2.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r1.xyzx))).xyz;
    // 45: mad r0.xyz, r0.xyzx, r2.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 46: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 47: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bg_rad_koukusaton_card01a_mi: 1e5c374e163c74468caf9b27d3d32081; selected map 283b519353e0898323683b51386666d69cae47198a4ef0d3ede7a5d75d6cb5b5.
float4 ArtistNative3331(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=float4(input.sourceActorPosition,0.f); // Native card UV seed is actor location in UE centimetres.
    source[6]=float4(input.skyUpperColor,0.f);
    source[7]=float4(input.skyLowerColor,0.f);
    source[8]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyz;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: mul r0.xyz, r0.xyzx, cb0[4].xxwx
    r0.xyz = ((r0.xyzx)*(source[4].xxwx)).xyz;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 9: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 11: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 12: div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // 13: dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // 14: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 15: mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // 16: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 17: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 18: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 19: dp3 r1.w, r0.xywx, r1.xyzx
    r1.w = (dot((r0.xywx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 20: mul r2.xyz, r0.xywx, r1.wwww
    r2.xyz = ((r0.xywx)*(r1.wwww)).xyz;
    // 21: mad r1.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 22: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 25: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 26: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 27: mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 28: mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // 29: mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 30: dp3 r2.z, r2.xyzx, r0.xywx
    r2.z = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // 31: mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: dp3 r2.x, r3.xyzx, r0.xywx
    r2.x = (dot((r3.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // 35: dp3 r2.y, r4.xyzx, r0.xywx
    r2.y = (dot((r4.xyzx).xyz,(r0.xywx).xyz).xxxx).y;
    // 36: mul r1.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r1.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 37: mad r1.xy, cb0[4].yyyy, r5.xyxx, r1.xyxx
    r1.xy = ((source[4].yyyy)*(r5.xyxx)+(r1.xyxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 39: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 40: mad r1.xyz, cb0[4].zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((source[4].zzzz)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 41: add r1.xyz, r1.xyzx, -cb0[4].zzzz
    r1.xyz = ((r1.xyzx)+(-(source[4].zzzz))).xyz;
    // 42: mov_sat r3.xyz, r1.xyzx
    r3.xyz = (saturate(r1.xyzx)).xyz;
    // 43: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 44: mad r1.xyz, -r0.zzzz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: mul r5.xyz, cb0[3].xyzx, cb0[5].xxxx
    r5.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // 47: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 48: mad r3.xyz, r0.zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 49: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 50: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 51: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 52: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 53: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 54: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 55: mul r3.xyz, r0.zzzz, v6.xyzx
    r3.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 56: dp3 r0.x, r3.xyzx, r0.xywx
    r0.x = (dot((r3.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // 57: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 59: mul r0.yzw, r0.yyyy, cb0[7].xxyz
    r0.yzw = ((r0.yyyy)*(source[7].xxyz)).yzw;
    // 60: mad r0.xyz, r0.xxxx, cb0[6].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[6].xyzx)+(r0.yzwy)).xyz;
    // 61: mul r0.xyz, r0.xyzx, cb0[8].wwww
    r0.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 62: mad r3.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r3.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 63: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 65: mad o0.xyz, r1.xyzx, cb0[8].xyzx, r3.xyzx
    output.xyz = ((r1.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // 67: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bg_rad_koukusaton_card01b_mi: 1e5c374e163c74468caf9b27d3d32081; selected map 283b519353e0898323683b51386666d69cae47198a4ef0d3ede7a5d75d6cb5b5.
float4 ArtistNative3332(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=float4(input.sourceActorPosition,0.f); // Native card UV seed is actor location in UE centimetres.
    source[6]=float4(input.skyUpperColor,0.f);
    source[7]=float4(input.skyLowerColor,0.f);
    source[8]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = g_ArtistSourceMaterialParameters[2u];
    source[4].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyz;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: mul r0.xyz, r0.xyzx, cb0[4].xxwx
    r0.xyz = ((r0.xyzx)*(source[4].xxwx)).xyz;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 9: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 11: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 12: div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // 13: dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // 14: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 15: mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // 16: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 17: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 18: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 19: dp3 r1.w, r0.xywx, r1.xyzx
    r1.w = (dot((r0.xywx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 20: mul r2.xyz, r0.xywx, r1.wwww
    r2.xyz = ((r0.xywx)*(r1.wwww)).xyz;
    // 21: mad r1.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 22: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 25: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 26: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 27: mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 28: mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // 29: mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 30: dp3 r2.z, r2.xyzx, r0.xywx
    r2.z = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // 31: mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: dp3 r2.x, r3.xyzx, r0.xywx
    r2.x = (dot((r3.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // 35: dp3 r2.y, r4.xyzx, r0.xywx
    r2.y = (dot((r4.xyzx).xyz,(r0.xywx).xyz).xxxx).y;
    // 36: mul r1.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r1.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 37: mad r1.xy, cb0[4].yyyy, r5.xyxx, r1.xyxx
    r1.xy = ((source[4].yyyy)*(r5.xyxx)+(r1.xyxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 39: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 40: mad r1.xyz, cb0[4].zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((source[4].zzzz)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 41: add r1.xyz, r1.xyzx, -cb0[4].zzzz
    r1.xyz = ((r1.xyzx)+(-(source[4].zzzz))).xyz;
    // 42: mov_sat r3.xyz, r1.xyzx
    r3.xyz = (saturate(r1.xyzx)).xyz;
    // 43: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 44: mad r1.xyz, -r0.zzzz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = (ArtistNativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: mul r5.xyz, cb0[3].xyzx, cb0[5].xxxx
    r5.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // 47: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 48: mad r3.xyz, r0.zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 49: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 50: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 51: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 52: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 53: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 54: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 55: mul r3.xyz, r0.zzzz, v6.xyzx
    r3.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 56: dp3 r0.x, r3.xyzx, r0.xywx
    r0.x = (dot((r3.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // 57: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 59: mul r0.yzw, r0.yyyy, cb0[7].xxyz
    r0.yzw = ((r0.yyyy)*(source[7].xxyz)).yzw;
    // 60: mad r0.xyz, r0.xxxx, cb0[6].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[6].xyzx)+(r0.yzwy)).xyz;
    // 61: mul r0.xyz, r0.xyzx, cb0[8].wwww
    r0.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 62: mad r3.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r3.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 63: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 65: mad o0.xyz, r1.xyzx, cb0[8].xyzx, r3.xyzx
    output.xyz = ((r1.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // 67: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_q_me_chip_01: 9b28c9df74055140a218839b324676e8; selected map 8e1ca828c0bed65c3bc9ec1efa2438077f3f683f200622cb1e6b847ede23609b.
float4 ArtistNative3333(ARTIST_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[14]=float4(input.skyUpperColor,0.f);
    source[15]=float4(input.skyLowerColor,0.f);
    source[16]=float4(input.ambientColor,input.skyIntensity);
    source[0] = g_ArtistSourceMaterialParameters[12u];
    source[1] = g_ArtistSourceMaterialParameters[11u];
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = g_ArtistSourceMaterialParameters[7u];
    source[4] = g_ArtistSourceMaterialParameters[10u];
    source[5] = g_ArtistSourceMaterialParameters[9u];
    source[6] = g_ArtistSourceMaterialParameters[14u];
    source[7] = g_ArtistSourceMaterialParameters[5u];
    source[8] = g_ArtistSourceMaterialParameters[6u];
    source[9].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].z = ((g_ArtistSourceMaterialParameters[0u].wwww*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[11].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].x = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[12].y = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_ArtistSourceMaterialParameters[1u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[1u].wwww)).x;
    source[13].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).w;
    // 5: add r1.x, r0.w, -cb0[9].x
    r1.x = ((r0.wwww)+(-(source[9].xxxx))).x;
    // 6: mul r1.x, r1.x, cb0[9].y
    r1.x = ((r1.xxxx)*(source[9].yyyy)).x;
    // 7: mad r1.xy, r0.xyxx, r1.xxxx, v4.xyxx
    r1.xy = ((r0.xyxx)*(r1.xxxx)+(v4.xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: add_sat r1.z, r0.w, r2.w
    r1.z = (saturate((r0.wwww)+(r2.wwww))).z;
    // 12: add_sat r0.w, r0.w, cb0[13].z
    r0.w = (saturate((r0.wwww)+(source[13].zzzz))).w;
    // 13: add r1.z, r1.z, l(-0.333300)
    r1.z = ((r1.zzzz)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).z;
    // 14: lt r1.z, r1.z, l(0.000000)
    r1.z = (asfloat((uint4)((r1.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 15: discard_nz r1.z
    if ((asuint(r1.zzzz)).x != 0u) clip(-1.f);
    // 16: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 17: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 18: mul r1.xy, r1.xyxx, cb0[9].zzzz
    r1.xy = ((r1.xyxx)*(source[9].zzzz)).xy;
    // 19: mul r3.xy, r1.xyxx, v2.wwww
    r3.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 20: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 21: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 22: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 23: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 25: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 26: div r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)/(r1.xxxx)).xyz;
    // 27: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 28: add r0.xy, -|r0.xzxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(abs(r0.xzxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 29: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 30: mad_sat r0.y, r0.x, cb0[10].z, -cb0[10].w
    r0.y = (saturate((r0.xxxx)*(source[10].zzzz)+(-(source[10].wwww)))).y;
    // 31: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 32: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 33: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 34: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 35: mul r3.xyz, r0.zzzz, cb0[6].xyzx
    r3.xyz = ((r0.zzzz)*(source[6].xyzx)).xyz;
    // 36: movc r3.xyz, r0.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 37: add r3.xyz, r3.xyzx, -cb0[6].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[6].xyzx))).xyz;
    // 38: mad r3.xyz, cb0[6].wwww, r3.xyzx, cb0[6].xyzx
    r3.xyz = ((source[6].wwww)*(r3.xyzx)+(source[6].xyzx)).xyz;
    // 39: mul r4.xyz, cb0[7].xyzx, cb0[11].zzzz
    r4.xyz = ((source[7].xyzx)*(source[11].zzzz)).xyz;
    // 40: mul r4.xyz, r4.xyzx, cb0[12].yyyy
    r4.xyz = ((r4.xyzx)*(source[12].yyyy)).xyz;
    // 41: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 42: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 43: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 45: mul r4.xyz, r4.xyzx, cb0[12].zzzz
    r4.xyz = ((r4.xyzx)*(source[12].zzzz)).xyz;
    // 46: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 47: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 48: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 49: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 50: mul r0.yz, v4.xxyx, cb0[2].xxyx
    r0.yz = ((v4.xxyx)*(source[2].xxyx)).yz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.yzyy, t2.xyzw, s2, l(0.000000)
    r4.xyz = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 52: mul r5.xyz, cb0[3].xyzx, cb0[9].wwww
    r5.xyz = ((source[3].xyzx)*(source[9].wwww)).xyz;
    // 53: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 54: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 55: mad r4.xyz, -r4.xyzx, r5.xyzx, r0.yyyy
    r4.xyz = ((-(r4.xyzx))*(r5.xyzx)+(r0.yyyy)).xyz;
    // 56: mad r4.xyz, cb0[10].xxxx, r4.xyzx, r6.xyzx
    r4.xyz = ((source[10].xxxx)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 57: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 58: add r5.xyz, -r4.xyzx, r0.yyyy
    r5.xyz = ((-(r4.xyzx))+(r0.yyyy)).xyz;
    // 59: mad r4.xyz, cb0[10].yyyy, r5.xyzx, r4.xyzx
    r4.xyz = ((source[10].yyyy)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 60: mad r5.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mad r6.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 63: mad r3.xyz, r4.xyzx, r5.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 64: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 65: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 67: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 68: mul r4.xyz, r0.yyyy, cb0[1].xyzx
    r4.xyz = ((r0.yyyy)*(source[1].xyzx)).xyz;
    // 69: movc r0.xyz, r0.xxxx, l(0,0,0,0), r4.xyzx
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 70: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 71: add r0.xyz, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 72: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 74: mad r2.xyz, cb0[13].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[13].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 75: mul r3.xyz, cb0[8].xyzx, cb0[13].yyyy
    r3.xyz = ((source[8].xyzx)*(source[13].yyyy)).xyz;
    // 76: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 77: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 78: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 79: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 80: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 81: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 82: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 83: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 84: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 85: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 86: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 87: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 88: mul r3.yzw, r3.yyyy, cb0[15].xxyz
    r3.yzw = ((r3.yyyy)*(source[15].xxyz)).yzw;
    // 89: mad r3.xyz, r3.xxxx, cb0[14].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[14].xyzx)+(r3.yzwy)).xyz;
    // 90: mul r3.xyz, r3.xyzx, cb0[16].wwww
    r3.xyz = ((r3.xyzx)*(source[16].wwww)).xyz;
    // 91: mad r0.xyz, r3.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 92: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 94: mad o0.xyz, r2.xyzx, cb0[16].xyzx, r0.xyzx
    output.xyz = ((r2.xyzx)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 96: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_ribtrails_02_tr: a8ce8a0d3d751d4a92e5c763aaee218e; selected map c706f78fe92733122222aa8a1acf08defcfe8ea2da22cac6658a08653f82eb3c.
float4 ArtistNative3334(ARTIST_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mad_sat r0.x, -v2.z, l(2.000000), l(1.000000)
    r0.x = (saturate((-(v2.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 2: mul r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)*(r0.xxxx)).y;
    // 3: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 4: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 5: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 6: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 7: mul r0.z, v4.x, cb0[2].x
    r0.z = ((v4.xxxx)*(source[2].xxxx)).z;
    // 8: mul r1.xyzw, r0.zzzz, l(-0.700000, -2.000000, -1.500000, -0.500000)
    r1.xyzw = ((r0.zzzz)*(float4(-0.700000,-2.000000,-1.500000,-0.500000))).xyzw;
    // 9: mov r2.xz, r1.wwyw
    r2.xz = (r1.wwyw).xz;
    // 10: mov r2.yw, l(0,0,0,0)
    r2.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yw;
    // 11: add r0.zw, r2.xxxy, v2.xxxy
    r0.zw = ((r2.xxxy)+(v2.xxxy)).zw;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: mul r0.z, r0.z, l(0.200000)
    r0.z = ((r0.zzzz)*(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 14: mad r3.xy, v2.zwzz, l(1.500000, 1.000000, 0.000000, 0.000000), r0.zzzz
    r3.xy = ((v2.zwzz)*(float4(1.500000,1.000000,0.000000,0.000000))+(r0.zzzz)).xy;
    // 15: add r0.z, r3.y, l(-0.500000)
    r0.z = ((r3.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).z;
    // 16: div r3.z, r0.z, r0.y
    r3.z = ((r0.zzzz)/(r0.yyyy)).z;
    // 17: mov r1.yw, l(0,0.500000,0,0.500000)
    r1.yw = (float4(asfloat(0u),0.500000,asfloat(0u),0.500000)).yw;
    // 18: add r1.xyzw, r1.xyzw, r3.xzxz
    r1.xyzw = ((r1.xyzw)+(r3.xzxz)).xyzw;
    // 19: add r0.yz, r2.zzwz, r3.xxyx
    r0.yz = ((r2.zzwz)+(r3.xxyx)).yz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t3.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 24: mul r1.xyz, r1.xyzx, l(0.700000, 0.700000, 0.700000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.700000,0.700000,0.700000,0.000000))).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 26: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 27: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 28: mad r0.xyz, r0.xxxx, l(8.000000, 8.000000, 8.000000, 0.000000), r1.xyzx
    r0.xyz = ((r0.xxxx)*(float4(8.000000,8.000000,8.000000,0.000000))+(r1.xyzx)).xyz;
    // 29: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 30: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 31: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 32: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 33: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_cd_13_1_tr: 784bcfc4f7fcbf4d9aafad87c953ae52; selected map b3317ad48f64301df551df0863617f878fbef3ec9ec183e871c682b23501083c.
float4 ArtistNative3335(ARTIST_NATIVE_INPUT input)
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
    // 30: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 32: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 33: mul r0.xyz, r1.xyzx, cb0[2].yyyy
    r0.xyz = ((r1.xyzx)*(source[2].yyyy)).xyz;
    // 34: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: mad r1.xyz, -cb0[2].yyyy, r1.xyzx, r0.wwww
    r1.xyz = ((-(source[2].yyyy))*(r1.xyzx)+(r0.wwww)).xyz;
    // 36: mad r0.xyz, cb0[2].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[2].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 37: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 38: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_dust_01_ad: ea34ae47e5e0ca49a3fc0085adc055a6; selected map d2ce9759e3a1a15dc12c5668dd6d9217f1e09d6b29edac69c038c095b2deb42f.
float4 ArtistNative3336(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2] = g_ArtistSourceMaterialParameters[0u];
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
    float4 r0=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: mul r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 3: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 4: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 5: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 6: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 7: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_c_pa_blackcircle_01_tr: c3ce069d1a35c94eab7eaa1e05be7314; selected map b1647f2c9fee28ae485610fe65fde93ec6051c5cd15e99737eb9465aeb93c45d.
float4 ArtistNative3337(ARTIST_NATIVE_INPUT input)
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
    // 11: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 12: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 13: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 14: mad o0.xyz, cb0[1].xyzx, v5.wwww, v5.xyzx
    output.xyz = ((source[1].xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_hole_09_tr: 292086d7aea2844988d4c88f6facb7cf; selected map b05bc39e955edc224e9b56bdecc4622d65a742da24af4c7c40ab6124f923c138.
float4 ArtistNative3338(ARTIST_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))*float4(0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))*float4(0.865999997, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0930000022, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.075000003, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[10] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[11] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[12] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[13] = ArtistNativeAppend(cos((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[14] = ArtistNativeAppend(sin((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[15] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.00999999978, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.075000003, 0.0, 0.0, 0.0))),1u);
    source[16] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.00999999978, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.224999994, 0.0, 0.0, 0.0))),1u);
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
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xy, r0.xxxx, v6.xyxx
    r0.xy = ((r0.xxxx)*(v6.xyxx)).xy;
    // 4: mad r0.zw, r0.xxxy, l(0.000000, 0.000000, -0.375000, -0.375000), v2.xxxy
    r0.zw = ((r0.xxxy)*(float4(0.000000,0.000000,-0.375000,-0.375000))+(v2.xxxy)).zw;
    // 5: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), cb0[7].xyxx
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(source[7].xyxx)).xy;
    // 6: mad r1.xy, v2.xyxx, l(1.300000, 1.300000, 0.000000, 0.000000), cb0[2].xyxx
    r1.xy = ((v2.xyxx)*(float4(1.300000,1.300000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: add r1.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 9: dp2 r1.w, r1.yzyy, r1.yzyy
    r1.w = (dot((r1.yzyy).xy,(r1.yzyy).xy).xxxx).w;
    // 10: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 11: add r2.x, r1.w, r1.w
    r2.x = ((r1.wwww)+(r1.wwww)).x;
    // 12: mul r2.y, r1.x, r2.x
    r2.y = ((r1.xxxx)*(r2.xxxx)).y;
    // 13: mad r2.yz, r2.yyyy, l(0.000000, -0.400000, -0.400000, 0.000000), r0.zzwz
    r2.yz = ((r2.yyyy)*(float4(0.000000,-0.400000,-0.400000,0.000000))+(r0.zzwz)).yz;
    // 14: add r2.yz, r2.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r2.yz = ((r2.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 15: mul r2.yz, r2.yyzy, v4.yyyy
    r2.yz = ((r2.yyzy)*(v4.yyyy)).yz;
    // 16: dp2 r3.x, cb0[11].xyxx, r2.yzyy
    r3.x = (dot((source[11].xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 17: dp2 r3.y, cb0[12].xyxx, r2.yzyy
    r3.y = (dot((source[12].xyxx).xy,(r2.yzyy).xy).xxxx).y;
    // 18: add_sat r2.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = (saturate((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000)))).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r2.yzyy, t7.yxzw, s7, l(0.000000)
    r2.y = (ArtistNativeSample7((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 20: mad r2.zw, v2.xxxy, l(0.000000, 0.000000, 1.300000, 1.300000), cb0[3].xxxy
    r2.zw = ((v2.xxxy)*(float4(0.000000,0.000000,1.300000,1.300000))+(source[3].xxxy)).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.zwzz, t1.yzxw, s2, l(0.000000)
    r2.z = (ArtistNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 22: mul r2.w, r2.x, r2.z
    r2.w = ((r2.xxxx)*(r2.zzzz)).w;
    // 23: mul r2.z, r2.z, l(0.500000)
    r2.z = ((r2.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 24: mad r1.x, r1.x, l(0.500000), r2.z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r2.zzzz)).x;
    // 25: mad r2.zw, r2.wwww, l(0.000000, 0.000000, -0.400000, -0.400000), r0.zzzw
    r2.zw = ((r2.wwww)*(float4(0.000000,0.000000,-0.400000,-0.400000))+(r0.zzzw)).zw;
    // 26: add r2.zw, r2.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 27: mul r2.zw, r2.zzzw, v4.yyyy
    r2.zw = ((r2.zzzw)*(v4.yyyy)).zw;
    // 28: dp2 r3.x, cb0[13].xyxx, r2.zwzz
    r3.x = (dot((source[13].xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 29: dp2 r3.y, cb0[14].xyxx, r2.zwzz
    r3.y = (dot((source[14].xyxx).xy,(r2.zwzz).xy).xxxx).y;
    // 30: add_sat r2.zw, r3.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = (saturate((r3.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000)))).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.zwzz, t7.yzxw, s7, l(0.000000)
    r2.z = (ArtistNativeSample7((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mul r2.y, r2.z, r2.y
    r2.y = ((r2.zzzz)*(r2.yyyy)).y;
    // 33: mul r2.z, r1.x, l(-0.400000)
    r2.z = ((r1.xxxx)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))).z;
    // 34: mad r0.zw, r2.xxxx, r2.zzzz, r0.zzzw
    r0.zw = ((r2.xxxx)*(r2.zzzz)+(r0.zzzw)).zw;
    // 35: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 37: mad r1.x, -r1.x, l(0.300000), r2.x
    r1.x = ((-(r1.xxxx))*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.xxxx)).x;
    // 38: dp2_sat r1.x, r1.xxxx, v3.wwww
    r1.x = (saturate(dot((r1.xxxx).xy,(v3.wwww).xy).xxxx)).x;
    // 39: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 40: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 41: mad r0.zw, v4.yyyy, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((v4.yyyy)*(r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 42: mov_sat r2.xw, r0.zzzw
    r2.xw = (saturate(r0.zzzw)).xw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.xw, r2.xwxx, t7.xywz, s7, l(0.000000)
    r2.xw = (ArtistNativeSample7((r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xw;
    // 44: mul r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)*(r2.xxxx)).x;
    // 45: mul r1.x, r1.x, l(25.000000)
    r1.x = ((r1.xxxx)*(float4(25.000000,25.000000,25.000000,25.000000))).x;
    // 46: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), cb0[9].xyxx
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(source[9].xyxx)).xy;
    // 47: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), cb0[10].xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(source[10].xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t9.xyzw, s9, l(0.000000)
    r3.xyz = (ArtistNativeSample9((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r2.xyxx, t8.xyzw, s8, l(0.000000)
    r4.xyz = (ArtistNativeSample8((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 50: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 51: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 52: mad r4.xyz, r3.xyzx, r5.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = ((r3.xyzx)*(r5.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 53: mad r3.xyz, r1.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 54: max r0.z, |r1.z|, |r1.y|
    r0.z = (max(abs(r1.zzzz),abs(r1.yyyy))).z;
    // 55: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 56: min r0.w, |r1.z|, |r1.y|
    r0.w = (min(abs(r1.zzzz),abs(r1.yyyy))).w;
    // 57: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 58: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 59: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 60: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 61: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 62: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 63: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 64: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 65: lt r2.x, |r1.z|, |r1.y|
    r2.x = (asfloat((uint4)((abs(r1.zzzz))<(abs(r1.yyyy))) * 0xffffffffu)).x;
    // 66: and r1.x, r1.x, r2.x
    r1.x = (asfloat(asuint(r1.xxxx) & asuint(r2.xxxx))).x;
    // 67: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 68: lt r0.w, r1.z, -r1.z
    r0.w = (asfloat((uint4)((r1.zzzz)<(-(r1.zzzz))) * 0xffffffffu)).w;
    // 69: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 70: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 71: min r0.w, r1.z, r1.y
    r0.w = (min(r1.zzzz,r1.yyyy)).w;
    // 72: max r1.x, r1.z, r1.y
    r1.x = (max(r1.zzzz,r1.yyyy)).x;
    // 73: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 74: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 75: and r0.w, r1.x, r0.w
    r0.w = (asfloat(asuint(r1.xxxx) & asuint(r0.wwww))).w;
    // 76: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 77: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 78: add r0.z, v4.x, l(-0.500000)
    r0.z = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).z;
    // 79: mad r0.z, r1.w, l(3.000000), -r0.z
    r0.z = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))+(-(r0.zzzz))).z;
    // 80: mad r0.w, -r1.w, l(2.000000), l(1.000000)
    r0.w = ((-(r1.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 82: min r1.y, r0.z, l(10.000000)
    r1.y = (min(r0.zzzz,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 83: mad r0.zw, r0.wwww, r2.zzzz, r1.xxxy
    r0.zw = ((r0.wwww)*(r2.zzzz)+(r1.xxxy)).zw;
    // 84: round_pi_sat r1.w, r0.w
    r1.w = (saturate(ceil(r0.wwww))).w;
    // 85: sample_l_indexable(texture2d)(float,float,float,float) r1.yz, r0.zwzz, t4.yzxw, s4, l(-1.000000)
    r1.yz = (ArtistNativeSample4((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).yz;
    // 86: mad r2.x, -r1.w, r1.y, l(1.000000)
    r2.x = ((-(r1.wwww))*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 87: mul r4.x, r1.y, r1.w
    r4.x = ((r1.yyyy)*(r1.wwww)).x;
    // 88: mul r1.y, r2.w, r2.x
    r1.y = ((r2.wwww)*(r2.xxxx)).y;
    // 89: mul r2.xyz, r3.xyzx, r1.yyyy
    r2.xyz = ((r3.xyzx)*(r1.yyyy)).xyz;
    // 90: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 91: mad r3.xyz, -r1.yyyy, r3.xyzx, r1.wwww
    r3.xyz = ((-(r1.yyyy))*(r3.xyzx)+(r1.wwww)).xyz;
    // 92: mad r2.xyz, r3.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.xyzx)).xyz;
    // 93: mul_sat r2.xyz, r2.xyzx, l(0.400000, 0.250000, 0.600000, 0.000000)
    r2.xyz = (saturate((r2.xyzx)*(float4(0.400000,0.250000,0.600000,0.000000)))).xyz;
    // 94: mad r1.yw, r0.zzzw, l(0.000000, 9.000000, 0.000000, 0.330000), cb0[15].xxxy
    r1.yw = ((r0.zzzw)*(float4(0.000000,9.000000,0.000000,0.330000))+(source[15].xxxy)).yw;
    // 95: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r1.ywyy, t8.xyzw, s8, l(0.000000)
    r3.xyz = (ArtistNativeSample8((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 96: mad r1.yw, r0.zzzw, l(0.000000, 7.000000, 0.000000, 0.500000), cb0[16].xxxy
    r1.yw = ((r0.zzzw)*(float4(0.000000,7.000000,0.000000,0.500000))+(source[16].xxxy)).yw;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r1.ywyy, t9.xyzw, s9, l(0.000000)
    r5.xyz = (ArtistNativeSample9((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 98: add r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)+(r5.xyzx)).xyz;
    // 99: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 100: mul r3.xyz, r3.xyzx, l(3.750000, 3.750000, 3.750000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.750000,3.750000,3.750000,0.000000))).xyz;
    // 101: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 102: mul r5.xyz, r6.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000)
    r5.xyz = ((r6.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))).xyz;
    // 103: mul r6.xyz, r3.xyzx, r3.xyzx
    r6.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 104: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 105: mad r3.xyz, r3.xyzx, r6.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 106: dp3 r1.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 107: add r5.xyz, -r3.xyzx, r1.yyyy
    r5.xyz = ((-(r3.xyzx))+(r1.yyyy)).xyz;
    // 108: mad r3.xyz, r5.xyzx, l(0.750000, 0.750000, 0.750000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.750000,0.750000,0.750000,0.000000))+(r3.xyzx)).xyz;
    // 109: mov r4.yzw, l(0,1.000000,1.300000,2.500000)
    r4.yzw = (float4(asfloat(0u),1.000000,1.300000,2.500000)).yzw;
    // 110: mul r3.xyz, r4.yxxy, r3.xyzx
    r3.xyz = ((r4.yxxy)*(r3.xyzx)).xyz;
    // 111: mad r2.xyz, r4.xzwx, r3.xyzx, r2.xyzx
    r2.xyz = ((r4.xzwx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 112: mad r1.yw, r0.zzzw, l(0.000000, 11.000000, 0.000000, 1.000000), cb0[4].xxxy
    r1.yw = ((r0.zzzw)*(float4(0.000000,11.000000,0.000000,1.000000))+(source[4].xxxy)).yw;
    // 113: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 7.000000, 0.500000), cb0[5].xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,7.000000,0.500000))+(source[5].xxxy)).zw;
    // 114: sample_l_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t3.xyzw, s3, l(-1.000000)
    r3.xyz = (ArtistNativeSample3((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 115: sample_l_indexable(texture2d)(float,float,float,float) r4.yzw, r1.ywyy, t2.wxyz, s0, l(-1.000000)
    r4.yzw = (ArtistNativeSample0((r1.ywyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 116: mul r5.xyz, r3.xyzx, r4.yzwy
    r5.xyz = ((r3.xyzx)*(r4.yzwy)).xyz;
    // 117: add r3.xyz, r3.xyzx, r4.yzwy
    r3.xyz = ((r3.xyzx)+(r4.yzwy)).xyz;
    // 118: mul r3.xyz, r1.zzzz, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r3.xyzx)).xyz;
    // 119: mul r3.xyz, r3.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(5.000000,5.000000,5.000000,0.000000))).xyz;
    // 120: mad r3.xyz, r5.xyzx, l(8.000000, 8.000000, 8.000000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(8.000000,8.000000,8.000000,0.000000))+(r3.xyzx)).xyz;
    // 121: mad r0.zw, r1.xxxz, l(0.000000, 0.000000, 11.000000, 2.000000), cb0[6].xxxy
    r0.zw = ((r1.xxxz)*(float4(0.000000,0.000000,11.000000,2.000000))+(source[6].xxxy)).zw;
    // 122: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 123: mul r0.z, r0.z, r1.z
    r0.z = ((r0.zzzz)*(r1.zzzz)).z;
    // 124: mul_sat r0.z, r0.z, l(22.000000)
    r0.z = (saturate((r0.zzzz)*(float4(22.000000,22.000000,22.000000,22.000000)))).z;
    // 125: add r1.xyz, r0.zzzz, r3.xyzx
    r1.xyz = ((r0.zzzz)+(r3.xyzx)).xyz;
    // 126: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), cb0[8].xxxy
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(source[8].xxxy)).zw;
    // 127: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.yzxw, s5, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 128: mad r0.xy, r0.zzzz, l(0.120000, 0.120000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r0.zzzz)*(float4(0.120000,0.120000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 129: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s6, l(0.000000)
    r0.xyz = (ArtistNativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 130: mul r3.xyz, r0.xyzx, r1.xyzx
    r3.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 131: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 133: mad r0.xyz, r0.xyzx, l(0.330000, 0.330000, 0.330000, 0.000000), r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(0.330000,0.330000,0.330000,0.000000))+(r3.xyzx)).xyz;
    // 134: mul r0.xyz, r0.xyzx, l(2.000000, 1.250000, 3.000000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(2.000000,1.250000,3.000000,0.000000))).xyz;
    // 135: mad r0.xyz, r4.xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((r4.xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 136: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 137: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_afterburn_01_26_dt_tr: db590b4337e6ba4fa3e0b138317032df; selected map d3bbf19c7e240e8f06cb0a989f675240c0f08203863116dabcd6b7518bf9ea2c.
float4 ArtistNative3339(ARTIST_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(cos((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = g_ArtistSourceMaterialParameters[4u];
    source[9] = g_ArtistSourceMaterialParameters[3u];
    source[10].x = (cos((g_ArtistSourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].w = ((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[11].x = (cos((((g_ArtistSourceMaterialParameters[2u].yyyy+g_ArtistSourceMaterialParameters[2u].zzzz)+g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[12].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[12].z = ((g_ArtistSourceMaterialParameters[1u].zzzz*float4(3.0, 0.0, 0.0, 0.0))).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].x = ((g_ArtistSourceMaterialParameters[1u].wwww*float4(20.0, 0.0, 0.0, 0.0))).x;
    source[13].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[14].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[14].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy))).x;
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
    // 10: add r0.y, -cb0[14].y, l(1.000000)
    r0.y = ((-(source[14].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: mul r0.y, v4.w, cb0[10].y
    r0.y = ((v4.wwww)*(source[10].yyyy)).y;
    // 15: mad r0.zw, r0.yyyy, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.yyyy)*(v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 16: mul r1.xyzw, r0.yyyy, l(1.330000, 1.330000, 1.768900, 1.768900)
    r1.xyzw = ((r0.yyyy)*(float4(1.330000,1.330000,1.768900,1.768900))).xyzw;
    // 17: mad r1.xyzw, r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((r1.xyzw)*(v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 18: dp2 r0.y, cb0[3].xyxx, r0.zwzz
    r0.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 19: dp2 r2.x, cb0[2].xyxx, r0.zwzz
    r2.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 20: mad r2.y, v4.x, l(0.020000), r0.y
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 21: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t0.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t2.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 24: dp2 r0.y, cb0[5].xyxx, r1.xyxx
    r0.y = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 25: mad r3.y, v4.x, l(0.020000), r0.y
    r3.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 26: dp2 r3.x, cb0[4].xyxx, r1.xyxx
    r3.x = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 27: add r0.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.yzyy, t0.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.yzyy, t2.xyzw, s1, l(0.000000)
    r3.xyz = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: mul r3.xyz, r3.xyzx, l(0.333300, 0.333300, 0.333300, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.333300,0.333300,0.333300,0.000000))).xyz;
    // 31: mad r2.xyz, r2.xyzx, l(0.333300, 0.333300, 0.333300, 0.000000), r3.xyzx
    r2.xyz = ((r2.xyzx)*(float4(0.333300,0.333300,0.333300,0.000000))+(r3.xyzx)).xyz;
    // 32: mul r0.y, r1.x, l(0.333300)
    r0.y = ((r1.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))).y;
    // 33: mad r0.y, r0.w, l(0.333300), r0.y
    r0.y = ((r0.wwww)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).y;
    // 34: dp2 r0.z, cb0[7].xyxx, r1.zwzz
    r0.z = (dot((source[7].xyxx).xy,(r1.zwzz).xy).xxxx).z;
    // 35: dp2 r1.x, cb0[6].xyxx, r1.zwzz
    r1.x = (dot((source[6].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 36: mad r1.y, v4.x, l(0.020000), r0.z
    r1.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.zzzz)).y;
    // 37: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.zwzz, t0.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r0.zwzz, t2.wxyz, s1, l(0.000000)
    r1.yzw = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 40: mad r1.yzw, r1.yyzw, l(0.000000, 0.333300, 0.333300, 0.333300), r2.xxyz
    r1.yzw = ((r1.yyzw)*(float4(0.000000,0.333300,0.333300,0.333300))+(r2.xxyz)).yzw;
    // 41: mad r0.y, r1.x, l(0.333300), r0.y
    r0.y = ((r1.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).y;
    // 42: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 43: mul r0.z, r0.z, cb0[12].z
    r0.z = ((r0.zzzz)*(source[12].zzzz)).z;
    // 44: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 45: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 46: mov_sat r0.w, v4.z
    r0.w = (saturate(v4.zzzz)).w;
    // 47: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 49: lt r0.w, |r0.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 50: movc r0.z, r0.w, l(-0.000000), -r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.zzzz))).z;
    // 51: add r2.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 52: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 53: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 54: mad r0.w, -r0.w, l(1.428571), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(1.428571,1.428571,1.428571,1.428571))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 56: add r1.x, r0.w, r0.y
    r1.x = ((r0.wwww)+(r0.yyyy)).x;
    // 57: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 58: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 59: add r1.x, r1.x, -cb0[12].x
    r1.x = ((r1.xxxx)+(-(source[12].xxxx))).x;
    // 60: mad r0.z, r1.x, r0.w, r0.z
    r0.z = ((r1.xxxx)*(r0.wwww)+(r0.zzzz)).z;
    // 61: mul_sat r0.z, r0.z, cb0[13].y
    r0.z = (saturate((r0.zzzz)*(source[13].yyyy))).z;
    // 62: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 63: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 64: mul r0.w, r0.w, cb0[13].z
    r0.w = ((r0.wwww)*(source[13].zzzz)).w;
    // 65: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 66: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 67: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 68: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 69: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 70: dp3 r0.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 71: add r0.xzw, -r1.yyzw, r0.xxxx
    r0.xzw = ((-(r1.yyzw))+(r0.xxxx)).xzw;
    // 72: mad r0.xzw, cb0[11].yyyy, r0.xxzw, r1.yyzw
    r0.xzw = ((source[11].yyyy)*(r0.xxzw)+(r1.yyzw)).xzw;
    // 73: mul r0.xyz, r0.xzwx, r0.yyyy
    r0.xyz = ((r0.xzwx)*(r0.yyyy)).xyz;
    // 74: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 75: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 76: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 77: mul r0.xyz, r0.xyzx, cb0[11].wwww
    r0.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 78: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 79: mul r1.xyz, cb0[8].xyzx, cb0[8].wwww
    r1.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 80: mul r1.xyz, r1.xyzx, v4.yyyy
    r1.xyz = ((r1.xyzx)*(v4.yyyy)).xyz;
    // 81: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 82: mad r0.xyz, r0.xyzx, r1.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 83: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 84: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_j_pa_ring_07_08_ad: 107e821614ee944aa4b7e35e6f57cded; selected map 33b95adfbb6e274fffd0a5208ae5c12ab5da2fc5d2a1420d1fc6ceb5a288e459.
float4 ArtistNative3340(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_distort_multi_02_ad: cdcb319ec96e8444a9658ff6277c3289; selected map 9695266030c28e0810a00b44cb2b278ced9ba494ad0c2ff9b4e7c8049ebe5e2c.
float4 ArtistNative3341(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialParameters[0u].yyyy*g_ArtistSourceMaterialTime.xxxx),(g_ArtistSourceMaterialParameters[0u].zzzz*g_ArtistSourceMaterialTime.xxxx),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].wwww,g_ArtistSourceMaterialParameters[2u].xxxx,1u);
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialParameters[1u].yyyy*g_ArtistSourceMaterialTime.xxxx),(g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx),1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialParameters[2u].yyyy*g_ArtistSourceMaterialTime.xxxx),(g_ArtistSourceMaterialParameters[2u].zzzz*g_ArtistSourceMaterialTime.xxxx),1u);
    source[8].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].y = ((g_ArtistSourceMaterialParameters[1u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[8].z = ((g_ArtistSourceMaterialParameters[1u].yyyy*g_ArtistSourceMaterialTime.xxxx)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mad r0.xy, v2.xyxx, cb0[3].xyxx, v4.yyyy
    r0.xy = ((v2.xyxx)*(source[3].xyxx)+(v4.yyyy)).xy;
    // 2: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, v2.xxyx, cb0[5].xxyx, v4.yyyy
    r0.yz = ((v2.xxyx)*(source[5].xxyx)+(v4.yyyy)).yz;
    // 5: add r0.yz, r0.yyzy, cb0[6].xxyx
    r0.yz = ((r0.yyzy)+(source[6].xxyx)).yz;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 7: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 8: mad r0.yz, cb0[8].wwww, r0.xxxx, v2.xxyx
    r0.yz = ((source[8].wwww)*(r0.xxxx)+(v2.xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t3.yzwx, s4, l(0.000000)
    r0.w = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 11: add r0.z, -r0.y, r0.w
    r0.z = ((-(r0.yyyy))+(r0.wwww)).z;
    // 12: mad r0.y, v4.x, r0.z, r0.y
    r0.y = ((v4.xxxx)*(r0.zzzz)+(r0.yyyy)).y;
    // 13: max r0.y, |r0.y|, l(0.000001)
    r0.y = (max(abs(r0.yyyy),float4(0.000001,0.000001,0.000001,0.000001))).y;
    // 14: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 16: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 17: mul r0.y, r0.y, cb0[9].w
    r0.y = ((r0.yyyy)*(source[9].wwww)).y;
    // 18: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 19: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 20: mad r0.zw, v2.xxxy, cb0[2].xxxy, v4.yyyy
    r0.zw = ((v2.xxxy)*(source[2].xxxy)+(v4.yyyy)).zw;
    // 21: mad r0.xz, cb0[8].wwww, r0.xxxx, r0.zzwz
    r0.xz = ((source[8].wwww)*(r0.xxxx)+(r0.zzwz)).xz;
    // 22: add r0.xz, r0.xxzx, cb0[7].xxyx
    r0.xz = ((r0.xxzx)+(source[7].xxyx)).xz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t4.xwyz, s2, l(0.000000)
    r0.xzw = (ArtistNativeSample2((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 25: log r0.xzw, r0.xxzw
    r0.xzw = (log2(r0.xxzw)).xzw;
    // 26: mul r0.xzw, r0.xxzw, cb0[9].xxxx
    r0.xzw = ((r0.xxzw)*(source[9].xxxx)).xzw;
    // 27: exp r0.xzw, r0.xxzw
    r0.xzw = (exp2(r0.xxzw)).xzw;
    // 28: mul r0.xzw, r0.xxzw, cb0[9].yyyy
    r0.xzw = ((r0.xxzw)*(source[9].yyyy)).xzw;
    // 29: mad r0.xzw, r0.xxzw, v3.xxyz, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)+(source[1].xxyz)).xzw;
    // 30: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 31: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 32: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_shine_02_02_ad: 2285f472dbd31046a56e88f5c17600ae; selected map b1cb69b854bebbba839bb9ddf7d2437017c38accd30e729e19a3bf4900d4164e.
float4 ArtistNative3342(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
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
    // 2: dp2 r1.x, cb0[2].xyxx, r0.xyxx
    r1.x = (dot((source[2].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.xyxx
    r1.y = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.z, r0.x, cb0[6].w
    r0.z = ((r0.xxxx)*(source[6].wwww)).z;
    // 6: mad r1.x, cb0[5].w, cb0[6].z, r0.z
    r1.x = ((source[5].wwww)*(source[6].zzzz)+(r0.zzzz)).x;
    // 7: mul r0.z, cb0[5].w, cb0[7].y
    r0.z = ((source[5].wwww)*(source[7].yyyy)).z;
    // 8: mad r1.y, cb0[7].x, r0.y, r0.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 10: mad r0.zw, cb0[7].zzzz, r0.zzzw, r0.xxxy
    r0.zw = ((source[7].zzzz)*(r0.zzzw)+(r0.xxxy)).zw;
    // 11: mul r1.xy, r0.zwzz, cb0[6].xyxx
    r1.xy = ((r0.zwzz)*(source[6].xyxx)).xy;
    // 12: mul r0.zw, r0.zzzw, cb0[8].yyyz
    r0.zw = ((r0.zzzw)*(source[8].yyyz)).zw;
    // 13: mad r0.zw, cb0[5].wwww, cb0[8].xxxw, r0.zzzw
    r0.zw = ((source[5].wwww)*(source[8].xxxw)+(r0.zzzw)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: mad r2.x, cb0[5].w, cb0[5].z, r1.x
    r2.x = ((source[5].wwww)*(source[5].zzzz)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[5].w, cb0[7].w, r1.y
    r2.y = ((source[5].wwww)*(source[7].wwww)+(r1.yyyy)).y;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 19: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 20: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 21: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 22: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 23: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 24: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 25: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 26: mul r0.w, r0.w, cb0[5].x
    r0.w = ((r0.wwww)*(source[5].xxxx)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul r0.w, r0.w, cb0[5].y
    r0.w = ((r0.wwww)*(source[5].yyyy)).w;
    // 29: lt r1.x, |r0.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 31: add r1.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 32: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 33: mul_sat r0.y, r1.y, cb0[9].z
    r0.y = (saturate((r1.yyyy)*(source[9].zzzz))).y;
    // 34: mul r1.x, r0.x, l(4.000000)
    r1.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 35: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 36: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 37: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 38: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 39: mul r1.x, r1.x, cb0[4].w
    r1.x = ((r1.xxxx)*(source[4].wwww)).x;
    // 40: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 41: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 42: mul_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)*(r0.xxxx))).x;
    // 43: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 44: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[9].w
    r0.z = ((r0.zzzz)*(source[9].wwww)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 48: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 49: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 50: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 51: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 52: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 53: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 54: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ap_22_1_tr: fcbea3791fa93e4c903329208982c802; selected map 4c301be45c25945b81669f82fd2decda49d899cfdc59f4199e2b6058db388c20.
float4 ArtistNative3343(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[5].zwzz
    r0.xy = ((v2.xyxx)*(source[5].zwzz)).xy;
    // 2: mad r1.x, cb0[4].x, cb0[5].y, r0.x
    r1.x = ((source[4].xxxx)*(source[5].yyyy)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[4].x, cb0[6].x, r0.y
    r1.y = ((source[4].xxxx)*(source[6].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mul r0.xy, r0.xyxx, cb0[6].yyyy
    r0.xy = ((r0.xyxx)*(source[6].yyyy)).xy;
    // 6: mul r0.zw, v2.xxxy, cb0[4].yyyz
    r0.zw = ((v2.xxxy)*(source[4].yyyz)).zw;
    // 7: mad r1.x, cb0[4].x, cb0[3].w, r0.z
    r1.x = ((source[4].xxxx)*(source[3].wwww)+(r0.zzzz)).x;
    // 8: mad r1.y, cb0[4].x, cb0[4].w, r0.w
    r1.y = ((source[4].xxxx)*(source[4].wwww)+(r0.wwww)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 10: mad r0.xy, cb0[5].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[5].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 11: add r0.xy, r0.xyxx, v2.xyxx
    r0.xy = ((r0.xyxx)+(v2.xyxx)).xy;
    // 12: mad r0.z, cb0[6].z, v4.z, l(-1.000000)
    r0.z = ((source[6].zzzz)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 13: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 14: mul r0.w, v4.z, cb0[6].z
    r0.w = ((v4.zzzz)*(source[6].zzzz)).w;
    // 15: mad r0.xy, r0.wwww, r0.xyxx, -r0.zzzz
    r0.xy = ((r0.wwww)*(r0.xyxx)+(-(r0.zzzz))).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t2.yzxw, s4, l(0.000000)
    r0.z = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t4.xywz, s3, l(0.000000)
    r0.xyw = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 18: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 19: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 20: mul r1.x, r1.x, cb0[7].y
    r1.x = ((r1.xxxx)*(source[7].yyyy)).x;
    // 21: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 22: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 23: movc r0.z, r0.z, l(0), |r1.x|
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).z;
    // 24: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 25: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 27: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.y, v2.xyxx, t3.yxzw, s0, l(0.000000)
    r1.y = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: mul r1.y, r1.y, cb0[3].x
    r1.y = ((r1.yyyy)*(source[3].xxxx)).y;
    // 30: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 31: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 32: mul r1.z, r1.z, cb0[3].y
    r1.z = ((r1.zzzz)*(source[3].yyyy)).z;
    // 33: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 34: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 35: mad r1.z, v4.x, l(2.000000), l(-1.000000)
    r1.z = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 36: add r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)+(r1.zzzz)).y;
    // 37: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 38: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: mul r1.z, r1.z, cb0[3].z
    r1.z = ((r1.zzzz)*(source[3].zzzz)).z;
    // 40: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 41: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 43: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 44: ge r1.y, l(0.250000), r1.y
    r1.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r1.yyyy)) * 0xffffffffu)).y;
    // 45: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 47: movc o0.w, r0.z, l(0), r1.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 48: mul r1.xzw, r0.xxyw, cb0[6].wwww
    r1.xzw = ((r0.xxyw)*(source[6].wwww)).xzw;
    // 49: dp3 r0.z, r1.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 50: mad r0.xyz, -cb0[6].wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(source[6].wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 51: mad r0.xyz, cb0[7].xxxx, r0.xyzx, r1.xzwx
    r0.xyz = ((source[7].xxxx)*(r0.xyzx)+(r1.xzwx)).xyz;
    // 52: mul r0.xyz, r0.xyzx, v3.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)).xyz;
    // 53: mul r1.xzw, cb0[2].xxyz, cb0[2].wwww
    r1.xzw = ((source[2].xxyz)*(source[2].wwww)).xzw;
    // 54: movc r0.xyz, r1.yyyy, r1.xzwx, r0.xyzx
    r0.xyz = ((asuint(r1.yyyy) != 0u) ? (r1.xzwx) : (r0.xyzx)).xyz;
    // 55: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 56: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_pa_tearsurface_01_tr: 9bad8ccc3bbf6a429c66a796748443b9; selected map 28bdafc6e84263439c2c9bdbaf9426012a29ead161415b7d88033ee46f4b8a86.
float4 ArtistNative3344(ARTIST_NATIVE_INPUT input)
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
    // 1: add r0.x, v4.x, l(0.100000)
    r0.x = ((v4.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 3: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 4: ge r0.x, r0.x, r0.y
    r0.x = (asfloat((uint4)((r0.xxxx)>=(r0.yyyy)) * 0xffffffffu)).x;
    // 5: ge r0.y, v4.x, r0.y
    r0.y = (asfloat((uint4)((v4.xxxx)>=(r0.yyyy)) * 0xffffffffu)).y;
    // 6: movc r0.xy, r0.xyxx, l(-0.000000,0,0,0), l(-1.000000,1.000000,0,0)
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(-0.000000,asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(-1.000000,1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 7: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 8: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 9: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 11: mul r0.y, r0.y, l(10.000000)
    r0.y = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: mad r0.xyz, r0.yyyy, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xxxx)+(r1.xyzx)).xyz;
    // 14: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 15: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_dirt_04_03_tr: 1dee627450623d4699684df94a092879; selected map fd09d853c3414943101dab6dece30cab598b55cc307a3d1aa0d399e29220d944.
float4 ArtistNative3345(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_frag_07_1_tr: 455e793420c6f840a9c617b61133dcd9; selected map 480b6847d64506112866889d7c6a1a0377881b152fd192230799bab549620fce.
float4 ArtistNative3346(ARTIST_NATIVE_INPUT input)
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s2, l(0.000000)
    r0.yzw = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 23: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 24: mul r1.x, r1.x, cb0[4].y
    r1.x = ((r1.xxxx)*(source[4].yyyy)).x;
    // 25: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 26: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 27: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 28: movc r1.x, r1.y, l(0), |r1.x|
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).x;
    // 29: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 30: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: mul r1.y, r1.y, v4.y
    r1.y = ((r1.yyyy)*(v4.yyyy)).y;
    // 32: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 33: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 34: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 36: movc o0.w, r1.x, l(0), r0.x
    output.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 37: mul r1.xyz, r0.yzwy, cb0[3].wwww
    r1.xyz = ((r0.yzwy)*(source[3].wwww)).xyz;
    // 38: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 39: mad r0.xyz, -cb0[3].wwww, r0.yzwy, r0.xxxx
    r0.xyz = ((-(source[3].wwww))*(r0.yzwy)+(r0.xxxx)).xyz;
    // 40: mad r0.xyz, cb0[4].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 41: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 42: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_ht_21_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative3347(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3330Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3335Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3340Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3343Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3345Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3346Distortion(ARTIST_NATIVE_INPUT input)
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
