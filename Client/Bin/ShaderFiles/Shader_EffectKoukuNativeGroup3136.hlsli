// Original Kouku material programs 3136..3199; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_me_flowtrail_01_23_tr: 481d3b1eaa260041af8d5365d014810e; selected map 8e8bdf58c7520eb1cd937f4238520d68e961689159765b6d948c2e5a7d8d5164.
float4 ArtistNative3136(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = g_ArtistSourceMaterialParameters[7u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,g_ArtistSourceMaterialParameters[3u].yyyy,1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialParameters[2u].zzzz*g_ArtistSourceMaterialTime.xxxx),(g_ArtistSourceMaterialParameters[3u].xxxx*g_ArtistSourceMaterialTime.xxxx),1u);
    source[8] = input.dynamicParameter;
    source[9] = ArtistNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialParameters[5u].xxxx*g_ArtistSourceMaterialTime.xxxx),1u);
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].yyyy,g_ArtistSourceMaterialParameters[4u].zzzz,1u);
    source[13] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[14] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[15].x = ((g_ArtistSourceMaterialParameters[3u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[15].y = ((g_ArtistSourceMaterialParameters[2u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[16].y = ((g_ArtistSourceMaterialParameters[5u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[16].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[17].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[17].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[17].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[17].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[18].x = (cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[18].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[19].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    r0.x = (sin(r0.xxxx)).x;
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
    // 51: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 52: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 53: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 54: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 55: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 56: mul r0.z, r0.z, cb0[19].y
    r0.z = ((r0.zzzz)*(source[19].yyyy)).z;
    // 57: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 58: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 59: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 60: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_worldoffset_02_44_tr: a0f7f5723e826143b3970e44f6a045f2; selected map 556a53205748259b14772cac15cc51ca4b99fc29a3e8ad2940bdffdafc2ff11e.
float4 ArtistNative3137(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_me_watertrail_01_45_tr: 0c986668f7b6b8438ad4e59533ac0d95; selected map 04f23ee5f526fb7f1b8f245b449c842f3c96ad9cc0aef03ea1bde8686efad069.
float4 ArtistNative3138(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[7].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[4].xyxx, r0.xyxx
    r1.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[5].xyxx, r0.xyxx
    r1.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.z, r0.x, cb0[7].w
    r0.z = ((r0.xxxx)*(source[7].wwww)).z;
    // 6: mad r1.x, cb0[7].z, cb0[7].y, r0.z
    r1.x = ((source[7].zzzz)*(source[7].yyyy)+(r0.zzzz)).x;
    // 7: mul r0.z, r0.y, cb0[8].x
    r0.z = ((r0.yyyy)*(source[8].xxxx)).z;
    // 8: mad r1.y, cb0[7].z, cb0[9].w, r0.z
    r1.y = ((source[7].zzzz)*(source[9].wwww)+(r0.zzzz)).y;
    // 9: add r0.zw, r1.xxxy, cb0[6].xxxy
    r0.zw = ((r1.xxxy)+(source[6].xxxy)).zw;
    // 10: add r1.x, cb0[3].w, cb0[12].x
    r1.x = ((source[3].wwww)+(source[12].xxxx)).x;
    // 11: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 12: mul r1.yz, r0.xxyx, cb0[11].xxyx
    r1.yz = ((r0.xxyx)*(source[11].xxyx)).yz;
    // 13: mul r0.xy, r0.xyxx, cb0[14].xyxx
    r0.xy = ((r0.xyxx)*(source[14].xyxx)).xy;
    // 14: mad r1.y, cb0[7].z, cb0[10].w, r1.y
    r1.y = ((source[7].zzzz)*(source[10].wwww)+(r1.yyyy)).y;
    // 15: mad r1.z, cb0[7].z, cb0[11].z, r1.z
    r1.z = ((source[7].zzzz)*(source[11].zzzz)+(r1.zzzz)).z;
    // 16: mad r2.y, cb0[3].y, cb0[11].w, r1.z
    r2.y = ((source[3].yyyy)*(source[11].wwww)+(r1.zzzz)).y;
    // 17: mad r2.x, cb0[3].y, cb0[10].z, r1.y
    r2.x = ((source[3].yyyy)*(source[10].zzzz)+(r1.yyyy)).x;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r2.xyxx, t0.zxyw, s0, l(0.000000)
    r1.yz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 19: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 20: add r1.xy, cb0[3].xzxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[3].xzxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mad r2.x, r1.x, cb0[7].x, r0.z
    r2.x = ((r1.xxxx)*(source[7].xxxx)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[12].y, r0.w
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r2.xyxx, t2.xwyz, s2, l(0.000000)
    r1.xzw = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 25: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r3.xyzw, r1.xxzw, cb0[1].wxyz
    r3.xyzw = ((r1.xxzw)*(source[1].wxyz)).xyzw;
    // 27: mad r0.x, cb0[7].z, cb0[13].w, r0.x
    r0.x = ((source[7].zzzz)*(source[13].wwww)+(r0.xxxx)).x;
    // 28: mad r0.y, cb0[7].z, cb0[14].z, r0.y
    r0.y = ((source[7].zzzz)*(source[14].zzzz)+(r0.yyyy)).y;
    // 29: mad r4.y, cb0[3].y, cb0[14].w, r0.y
    r4.y = ((source[3].yyyy)*(source[14].wwww)+(r0.yyyy)).y;
    // 30: mad r4.x, cb0[3].y, cb0[13].z, r0.x
    r4.x = ((source[3].yyyy)*(source[13].zzzz)+(r0.xxxx)).x;
    // 31: add r0.xy, r4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 32: dp2 r4.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.xyxx
    r4.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).x;
    // 33: dp2 r4.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.xyxx
    r4.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).y;
    // 34: add r0.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 36: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 37: mul_sat r0.x, r0.x, cb0[15].x
    r0.x = (saturate((r0.xxxx)*(source[15].xxxx))).x;
    // 38: mul r0.x, r0.x, r3.x
    r0.x = ((r0.xxxx)*(r3.xxxx)).x;
    // 39: mul_sat r0.x, r0.x, cb0[15].y
    r0.x = (saturate((r0.xxxx)*(source[15].yyyy))).x;
    // 40: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 41: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 42: mad r0.xyz, -cb0[1].xyzx, r1.xzwx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xzwx)+(r0.xxxx)).xyz;
    // 43: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 44: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 45: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 46: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 47: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 48: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 49: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 50: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 51: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_me_watertrail_01_44_tr: 0c986668f7b6b8438ad4e59533ac0d95; selected map 04f23ee5f526fb7f1b8f245b449c842f3c96ad9cc0aef03ea1bde8686efad069.
float4 ArtistNative3139(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[7].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[4].xyxx, r0.xyxx
    r1.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[5].xyxx, r0.xyxx
    r1.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.z, r0.x, cb0[7].w
    r0.z = ((r0.xxxx)*(source[7].wwww)).z;
    // 6: mad r1.x, cb0[7].z, cb0[7].y, r0.z
    r1.x = ((source[7].zzzz)*(source[7].yyyy)+(r0.zzzz)).x;
    // 7: mul r0.z, r0.y, cb0[8].x
    r0.z = ((r0.yyyy)*(source[8].xxxx)).z;
    // 8: mad r1.y, cb0[7].z, cb0[9].w, r0.z
    r1.y = ((source[7].zzzz)*(source[9].wwww)+(r0.zzzz)).y;
    // 9: add r0.zw, r1.xxxy, cb0[6].xxxy
    r0.zw = ((r1.xxxy)+(source[6].xxxy)).zw;
    // 10: add r1.x, cb0[3].w, cb0[12].x
    r1.x = ((source[3].wwww)+(source[12].xxxx)).x;
    // 11: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 12: mul r1.yz, r0.xxyx, cb0[11].xxyx
    r1.yz = ((r0.xxyx)*(source[11].xxyx)).yz;
    // 13: mul r0.xy, r0.xyxx, cb0[14].xyxx
    r0.xy = ((r0.xyxx)*(source[14].xyxx)).xy;
    // 14: mad r1.y, cb0[7].z, cb0[10].w, r1.y
    r1.y = ((source[7].zzzz)*(source[10].wwww)+(r1.yyyy)).y;
    // 15: mad r1.z, cb0[7].z, cb0[11].z, r1.z
    r1.z = ((source[7].zzzz)*(source[11].zzzz)+(r1.zzzz)).z;
    // 16: mad r2.y, cb0[3].y, cb0[11].w, r1.z
    r2.y = ((source[3].yyyy)*(source[11].wwww)+(r1.zzzz)).y;
    // 17: mad r2.x, cb0[3].y, cb0[10].z, r1.y
    r2.x = ((source[3].yyyy)*(source[10].zzzz)+(r1.yyyy)).x;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r2.xyxx, t0.zxyw, s0, l(0.000000)
    r1.yz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 19: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 20: add r1.xy, cb0[3].xzxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[3].xzxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mad r2.x, r1.x, cb0[7].x, r0.z
    r2.x = ((r1.xxxx)*(source[7].xxxx)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[12].y, r0.w
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r2.xyxx, t2.xwyz, s2, l(0.000000)
    r1.xzw = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 25: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r3.xyzw, r1.xxzw, cb0[1].wxyz
    r3.xyzw = ((r1.xxzw)*(source[1].wxyz)).xyzw;
    // 27: mad r0.x, cb0[7].z, cb0[13].w, r0.x
    r0.x = ((source[7].zzzz)*(source[13].wwww)+(r0.xxxx)).x;
    // 28: mad r0.y, cb0[7].z, cb0[14].z, r0.y
    r0.y = ((source[7].zzzz)*(source[14].zzzz)+(r0.yyyy)).y;
    // 29: mad r4.y, cb0[3].y, cb0[14].w, r0.y
    r4.y = ((source[3].yyyy)*(source[14].wwww)+(r0.yyyy)).y;
    // 30: mad r4.x, cb0[3].y, cb0[13].z, r0.x
    r4.x = ((source[3].yyyy)*(source[13].zzzz)+(r0.xxxx)).x;
    // 31: add r0.xy, r4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 32: dp2 r4.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.xyxx
    r4.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).x;
    // 33: dp2 r4.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.xyxx
    r4.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).y;
    // 34: add r0.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 36: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 37: mul_sat r0.x, r0.x, cb0[15].x
    r0.x = (saturate((r0.xxxx)*(source[15].xxxx))).x;
    // 38: mul r0.x, r0.x, r3.x
    r0.x = ((r0.xxxx)*(r3.xxxx)).x;
    // 39: mul_sat r0.x, r0.x, cb0[15].y
    r0.x = (saturate((r0.xxxx)*(source[15].yyyy))).x;
    // 40: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 41: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 42: mad r0.xyz, -cb0[1].xyzx, r1.xzwx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xzwx)+(r0.xxxx)).xyz;
    // 43: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 44: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 45: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 46: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 47: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 48: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 49: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 50: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 51: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_gaurdmode_01_ad: ae9901ef1562c642968d456861e89b2f; selected map 6fb985c1a8185757d9407af15eafeb1ba0dcb424796f0e2202ed36a1105a422f.
float4 ArtistNative3140(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[5].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].z = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[6].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.5, 0.0, 0.0, 0.0)))).x;
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
    // 10: mul_sat r0.xy, r0.xxxx, l(0.010000, 0.033333, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xxxx)*(float4(0.010000,0.033333,0.000000,0.000000)))).xy;
    // 11: log r0.zw, r0.xxxy
    r0.zw = (log2(r0.xxxy)).zw;
    // 12: lt r0.xy, r0.xyxx, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 13: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.250000, 0.125000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.250000,0.125000))).zw;
    // 14: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 15: add r0.zw, -r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.zzzw))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 16: movc r0.xy, r0.xyxx, l(1.000000,1.000000,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 17: mul r1.xyzw, v4.xyxy, cb0[5].yyww
    r1.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 18: mad r0.zw, r1.zzzw, l(0.000000, 0.000000, 3.000000, 0.500000), cb0[3].xxxy
    r0.zw = ((r1.zzzw)*(float4(0.000000,0.000000,3.000000,0.500000))+(source[3].xxxy)).zw;
    // 19: mul r1.xy, r1.xyxx, l(4.000000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(4.000000,0.500000,0.000000,0.000000))).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: mad r0.zw, r0.zzzz, cb0[6].zzzz, r1.xxxy
    r0.zw = ((r0.zzzz)*(source[6].zzzz)+(r1.xxxy)).zw;
    // 22: add r0.zw, r0.zzzw, cb0[4].xxxy
    r0.zw = ((r0.zzzw)+(source[4].xxxy)).zw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 24: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 25: mad r0.y, r0.y, r0.z, r0.y
    r0.y = ((r0.yyyy)*(r0.zzzz)+(r0.yyyy)).y;
    // 26: mad r0.x, r0.x, r0.z, r0.y
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).x;
    // 27: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 28: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 29: mul r0.w, v4.y, l(0.500000)
    r0.w = ((v4.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 30: mad r1.x, -v4.y, l(0.500000), l(1.000000)
    r1.x = ((-(v4.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 32: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 33: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 34: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 35: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 36: mul r1.x, r1.x, l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 37: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 38: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 39: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_atta_12_11_dt_ad: b0d551a53864c9408d2c03215f9ef88f; selected map 4dd93e1b4ffae10c2db3ed0c4c9132c6db631aa6aa2fdbffcc7ea3e1b5d89ab4.
float4 ArtistNative3141(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_me_floorstrm_1_ad: 00cd866295cc494dba563a698dea14df; selected map 25dad5f905785b3aa0b8fbe349a5a17d05495ab568518506e1e61c8b6a29d983.
float4 ArtistNative3142(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_me_spritewave_01_3_tr: 3ba0867fab1e9942995dc6ecd33b1057; selected map 7272a141700a237737811c8ca8a7bd51fd64fa7f342bf067d57285fde23c989b.
float4 ArtistNative3143(ARTIST_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[12u];
    source[3] = input.dynamicParameter;
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
    source[12].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[16].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[17].x = (cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[17].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[17].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[18].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[18].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[19].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[19].w = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[20].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[20].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[20].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    // 22: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 23: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 24: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 25: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 27: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 28: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 29: mul r0.w, r0.w, cb0[3].w
    r0.w = ((r0.wwww)*(source[3].wwww)).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: lt r1.z, r0.z, l(0.000001)
    r1.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 34: mul r0.z, r0.z, cb0[19].w
    r0.z = ((r0.zzzz)*(source[19].wwww)).z;
    // 35: max r0.z, r0.z, cb0[20].y
    r0.z = (max(r0.zzzz,source[20].yyyy)).z;
    // 36: min r0.z, r0.z, cb0[20].x
    r0.z = (min(r0.zzzz,source[20].xxxx)).z;
    // 37: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 38: mul r1.xy, r1.xyxx, cb0[12].xyxx
    r1.xy = ((r1.xyxx)*(source[12].xyxx)).xy;
    // 39: mad r2.x, cb0[11].w, cb0[11].z, r1.x
    r2.x = ((source[11].wwww)*(source[11].zzzz)+(r1.xxxx)).x;
    // 40: mad r2.y, cb0[11].w, cb0[12].z, r1.y
    r2.y = ((source[11].wwww)*(source[12].zzzz)+(r1.yyyy)).y;
    // 41: mul r1.x, cb0[3].x, cb0[12].w
    r1.x = ((source[3].xxxx)*(source[12].wwww)).x;
    // 42: mul r1.y, cb0[3].x, cb0[13].x
    r1.y = ((source[3].xxxx)*(source[13].xxxx)).y;
    // 43: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r1.xyxx
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 44: mul r1.zw, v4.xxxy, cb0[13].zzzw
    r1.zw = ((v4.xxxy)*(source[13].zzzw)).zw;
    // 45: mad r2.x, cb0[11].w, cb0[13].y, r1.z
    r2.x = ((source[11].wwww)*(source[13].yyyy)+(r1.zzzz)).x;
    // 46: mad r2.y, cb0[11].w, cb0[14].x, r1.w
    r2.y = ((source[11].wwww)*(source[14].xxxx)+(r1.wwww)).y;
    // 47: add r1.zw, r2.xxxy, cb0[4].xxxy
    r1.zw = ((r2.xxxy)+(source[4].xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: add r1.z, cb0[3].z, cb0[14].w
    r1.z = ((source[3].zzzz)+(source[14].wwww)).z;
    // 50: mad r1.zw, r0.wwww, r1.zzzz, cb0[5].xxxy
    r1.zw = ((r0.wwww)*(r1.zzzz)+(source[5].xxxy)).zw;
    // 51: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 52: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 53: dp2 r2.x, cb0[6].xyxx, r1.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 54: dp2 r2.y, cb0[7].xyxx, r1.xyxx
    r2.y = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 55: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 57: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 58: mul r1.x, r1.x, cb0[15].z
    r1.x = ((r1.xxxx)*(source[15].zzzz)).x;
    // 59: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 60: mul r1.x, r1.x, cb0[15].w
    r1.x = ((r1.xxxx)*(source[15].wwww)).x;
    // 61: lt r1.y, |r0.w|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 62: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 63: mad r1.x, r0.w, cb0[16].x, r1.x
    r1.x = ((r0.wwww)*(source[16].xxxx)+(r1.xxxx)).x;
    // 64: dp2 r2.x, cb0[8].xyxx, r0.xyxx
    r2.x = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[9].xyxx, r0.xyxx
    r2.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 66: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: mul r1.yz, v4.xxyx, cb0[17].zzwz
    r1.yz = ((v4.xxyx)*(source[17].zzwz)).yz;
    // 68: mad r2.x, cb0[11].w, cb0[17].y, r1.y
    r2.x = ((source[11].wwww)*(source[17].yyyy)+(r1.yyyy)).x;
    // 69: mad r2.y, cb0[11].w, cb0[18].x, r1.z
    r2.y = ((source[11].wwww)*(source[18].xxxx)+(r1.zzzz)).y;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t2.yxzw, s2, l(0.000000)
    r1.y = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 71: mad r0.xy, r1.yyyy, cb0[18].yyyy, r0.xyxx
    r0.xy = ((r1.yyyy)*(source[18].yyyy)+(r0.xyxx)).xy;
    // 72: mul r0.xy, r0.xyxx, cb0[16].zwzz
    r0.xy = ((r0.xyxx)*(source[16].zwzz)).xy;
    // 73: mad r0.x, cb0[11].w, cb0[16].y, r0.x
    r0.x = ((source[11].wwww)*(source[16].yyyy)+(r0.xxxx)).x;
    // 74: mad r0.y, cb0[11].w, cb0[18].z, r0.y
    r0.y = ((source[11].wwww)*(source[18].zzzz)+(r0.yyyy)).y;
    // 75: add r2.y, r0.y, cb0[19].x
    r2.y = ((r0.yyyy)+(source[19].xxxx)).y;
    // 76: add r2.x, r0.x, cb0[18].w
    r2.x = ((r0.xxxx)+(source[18].wwww)).x;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 78: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 79: add r0.y, cb0[3].y, l(-1.000000)
    r0.y = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 80: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 81: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 82: mul r0.y, r0.y, cb0[19].z
    r0.y = ((r0.yyyy)*(source[19].zzzz)).y;
    // 83: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 84: mul_sat r0.y, r0.y, cb0[19].y
    r0.y = (saturate((r0.yyyy)*(source[19].yyyy))).y;
    // 85: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 86: mul r0.x, r0.x, cb0[19].y
    r0.x = ((r0.xxxx)*(source[19].yyyy)).x;
    // 87: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 88: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 89: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 90: mul r0.x, r0.x, cb0[20].z
    r0.x = ((r0.xxxx)*(source[20].zzzz)).x;
    // 91: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 92: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 93: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 94: add r0.x, r0.y, r1.y
    r0.x = ((r0.yyyy)+(r1.yyyy)).x;
    // 95: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 96: mad r0.xyz, r0.xxxx, cb0[10].xyzx, r1.xxxx
    r0.xyz = ((r0.xxxx)*(source[10].xyzx)+(r1.xxxx)).xyz;
    // 97: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 98: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_rampshape_01_2_ad: ddc17986a1ab1c4dae2e850eb3dcc358; selected map 863d90418f599906771b3d9647bf4e7e7ffedef97e9c39c5854c0acd1b82da9a.
float4 ArtistNative3144(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww)*float4(-0.699999988, 0.0, 0.0, 0.0))),1u);
    source[4] = input.dynamicParameter;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].wwww)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 6: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 7: mul r0.y, r0.y, cb0[4].z
    r0.y = ((r0.yyyy)*(source[4].zzzz)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 10: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 11: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 12: add r0.zw, v4.xxxy, cb0[3].xxxy
    r0.zw = ((v4.xxxy)+(source[3].xxxy)).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 14: mul r0.w, r0.z, cb0[6].y
    r0.w = ((r0.zzzz)*(source[6].yyyy)).w;
    // 15: add_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)+(source[1].wwww))).z;
    // 16: mad r1.xy, cb0[5].yyyy, v4.xyxx, r0.wwww
    r1.xy = ((source[5].yyyy)*(v4.xyxx)+(r0.wwww)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: mul r0.w, r0.w, cb0[6].z
    r0.w = ((r0.wwww)*(source[6].zzzz)).w;
    // 19: mad_sat r0.x, r0.w, r0.x, r0.y
    r0.x = (saturate((r0.wwww)*(r0.xxxx)+(r0.yyyy))).x;
    // 20: add r0.x, r0.x, cb0[4].x
    r0.x = ((r0.xxxx)+(source[4].xxxx)).x;
    // 21: mul r0.xy, r0.xxxx, l(0.800000, 5.026548, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.800000,5.026548,0.000000,0.000000))).xy;
    // 22: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 23: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 25: mul r1.x, r0.w, cb0[5].x
    r1.x = ((r0.wwww)*(source[5].xxxx)).x;
    // 26: mad r0.x, r0.w, cb0[5].x, r0.x
    r0.x = ((r0.wwww)*(source[5].xxxx)+(r0.xxxx)).x;
    // 27: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 28: mul_sat r0.x, r0.x, cb0[4].w
    r0.x = (saturate((r0.xxxx)*(source[4].wwww))).x;
    // 29: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 31: mul r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)*(r0.xxxx)).w;
    // 32: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 33: mul r0.w, r0.w, r0.x
    r0.w = ((r0.wwww)*(r0.xxxx)).w;
    // 34: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 35: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 36: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 37: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 38: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 39: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 40: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 41: mul r0.y, r0.y, l(3.000000)
    r0.y = ((r0.yyyy)*(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 42: mad r0.yzw, r0.yyyy, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyyy)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 43: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 44: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 45: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_graring_01_1_tr: 2a737440fc2bc845acd802ab62530b5b; selected map 3bde9ce981d3bac63c7979c6a11c3b5c323c0975611cbbc96127cf29c4a184ff.
float4 ArtistNative3145(ARTIST_NATIVE_INPUT input)
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
    source[8].w = ((g_ArtistSourceMaterialParameters[0u].xxxx*float4(1.0, 0.0, 0.0, 0.0))).x;
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
    // 62: movc o0.w, r0.y, l(0), r1.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 63: dp3 r0.y, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 64: add r1.xyz, -r0.xzwx, r0.yyyy
    r1.xyz = ((-(r0.xzwx))+(r0.yyyy)).xyz;
    // 65: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xzwx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xzwx)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 71: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
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
// fx_j_pa_graring_01_tr: 2a737440fc2bc845acd802ab62530b5b; selected map 3bde9ce981d3bac63c7979c6a11c3b5c323c0975611cbbc96127cf29c4a184ff.
float4 ArtistNative3146(ARTIST_NATIVE_INPUT input)
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
    source[8].w = ((g_ArtistSourceMaterialParameters[0u].xxxx*float4(1.0, 0.0, 0.0, 0.0))).x;
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
    // 62: movc o0.w, r0.y, l(0), r1.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 63: dp3 r0.y, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 64: add r1.xyz, -r0.xzwx, r0.yyyy
    r1.xyz = ((-(r0.xzwx))+(r0.yyyy)).xyz;
    // 65: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xzwx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xzwx)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 71: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
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
// fx_k_pa_ring_05_ad: 107e821614ee944aa4b7e35e6f57cded; selected map 33b95adfbb6e274fffd0a5208ae5c12ab5da2fc5d2a1420d1fc6ceb5a288e459.
float4 ArtistNative3147(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_ring_07_17_ad: ece6597deb3b6341bb7555529a7b31fa; selected map 0b06b1fda0d5d6e344017b0e9f40be578a686c01b18ec5034f09127a3ac11257.
float4 ArtistNative3148(ARTIST_NATIVE_INPUT input)
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
    source[5].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
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
    // 37: mul r0.w, v4.x, cb0[4].z
    r0.w = ((v4.xxxx)*(source[4].zzzz)).w;
    // 38: log r1.z, r0.x
    r1.z = (log2(r0.xxxx)).z;
    // 39: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 40: mul r1.w, r1.z, v4.z
    r1.w = ((r1.zzzz)*(v4.zzzz)).w;
    // 41: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 42: movc r1.w, r0.x, l(0), r1.w
    r1.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 43: mad r1.y, cb0[3].x, r1.w, r0.y
    r1.y = ((source[3].xxxx)*(r1.wwww)+(r0.yyyy)).y;
    // 44: mad r0.yz, r0.zzzz, r0.wwww, r1.xxyx
    r0.yz = ((r0.zzzz)*(r0.wwww)+(r1.xxyx)).yz;
    // 45: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s0, cb0[2].x
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (source[2].xxxx).x, true).wxyz).yzw;
    // 46: mul_sat r1.x, r0.y, cb0[5].x
    r1.x = (saturate((r0.yyyy)*(source[5].xxxx))).x;
    // 47: mov_sat r1.y, v4.y
    r1.y = (saturate(v4.yyyy)).y;
    // 48: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 50: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r1.y, v2.xyxx, t2.yxzw, s2, l(0.000000)
    r1.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 52: mul_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)*(r1.xxxx))).x;
    // 53: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 54: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: mul r1.y, r1.y, cb0[5].y
    r1.y = ((r1.yyyy)*(source[5].yyyy)).y;
    // 56: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 57: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 58: mul r1.y, v4.w, cb0[5].z
    r1.y = ((v4.wwww)*(source[5].zzzz)).y;
    // 59: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 60: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 61: movc r0.x, r0.x, l(0), r1.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 62: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 64: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 65: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 66: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 67: mad r0.yzw, cb0[4].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
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
// bfx_d_pa_circ_01_03_tr: 653a5bb9cb1a754193f542b0fda91232; selected map b9a00a284058a01b21ea0d6fd6eec705cebc4a6f9f17beb3a1ba17da3210361f.
float4 ArtistNative3149(ARTIST_NATIVE_INPUT input)
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
// fx_m_pa_ap_01_2_tr: 9bd32611087e674eb9772288fd730bed; selected map 668cc569785a03268ecb8380a49eff747caff479be009fad05ecfcee0ffc813c.
float4 ArtistNative3150(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[11].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
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
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.z, r0.x, cb0[6].w
    r0.z = ((r0.xxxx)*(source[6].wwww)).z;
    // 6: mad r1.x, cb0[5].y, cb0[6].z, r0.z
    r1.x = ((source[5].yyyy)*(source[6].zzzz)+(r0.zzzz)).x;
    // 7: mul r0.z, cb0[5].y, cb0[7].y
    r0.z = ((source[5].yyyy)*(source[7].yyyy)).z;
    // 8: mad r1.y, cb0[7].x, r0.y, r0.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 10: mad r0.zw, cb0[7].zzzz, r0.zzzw, r0.xxxy
    r0.zw = ((source[7].zzzz)*(r0.zzzw)+(r0.xxxy)).zw;
    // 11: mul r1.xy, r0.zwzz, cb0[5].zwzz
    r1.xy = ((r0.zwzz)*(source[5].zwzz)).xy;
    // 12: mul r0.zw, r0.zzzw, cb0[8].yyyz
    r0.zw = ((r0.zzzw)*(source[8].yyyz)).zw;
    // 13: mad r0.zw, cb0[5].yyyy, cb0[8].xxxw, r0.zzzw
    r0.zw = ((source[5].yyyy)*(source[8].xxxw)+(r0.zzzw)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: mad r2.x, cb0[5].y, cb0[5].x, r1.x
    r2.x = ((source[5].yyyy)*(source[5].xxxx)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[5].y, cb0[7].w, r1.y
    r2.y = ((source[5].yyyy)*(source[7].wwww)+(r1.yyyy)).y;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: add r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)+(r0.wwww)).z;
    // 19: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 20: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 21: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 25: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 26: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 27: mul r0.w, r0.w, cb0[10].x
    r0.w = ((r0.wwww)*(source[10].xxxx)).w;
    // 28: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 29: mul r0.w, r0.w, cb0[10].y
    r0.w = ((r0.wwww)*(source[10].yyyy)).w;
    // 30: lt r1.x, |r0.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 31: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 32: add r1.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 33: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 34: mul_sat r0.y, r1.y, cb0[10].z
    r0.y = (saturate((r1.yyyy)*(source[10].zzzz))).y;
    // 35: mul r1.x, r0.x, l(4.000000)
    r1.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 36: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 37: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 38: mul r1.x, r1.x, cb0[9].z
    r1.x = ((r1.xxxx)*(source[9].zzzz)).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: mul r1.x, r1.x, cb0[9].w
    r1.x = ((r1.xxxx)*(source[9].wwww)).x;
    // 41: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 42: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 43: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 44: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 45: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 46: source device depth mapped to centimetre view depth; reconstruction at 48.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 48-51: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 52: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 53: add r0.w, -cb0[11].z, l(1.000000)
    r0.w = ((-(source[11].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 55: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 56: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 57: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 58: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 59: mul r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)*(source[10].wwww)).w;
    // 60: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 61: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 62: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 64: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 65: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 66: mul r0.xyz, cb0[2].xyzx, cb0[2].wwww
    r0.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 67: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 68: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_mi_v_pa_skull_02_tr: 254a726258718e40a81b3cc797ab88ba; selected map 33ca7273aa7af49780a5e23e2101a3414961d2117e9c88644fb4b04118db4c09.
float4 ArtistNative3151(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_01_05_tr: ad42f283a770bb4ca78f8baa88727bbd; selected map dae87ca73a1142a5b49a2e9752391354f42ce3d26558852fb0e6ef85a5fadca4.
float4 ArtistNative3152(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].xxxx,g_ArtistSourceMaterialParameters[3u].yyyy,1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].zzzz,g_ArtistSourceMaterialParameters[4u].wwww,1u);
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.xyzw, v4.xyxy, cb0[4].xyxy, r0.yxxy
    r0.xyzw = ((v4.xyxy)*(source[4].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.zw, v4.xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)).zw;
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
    // 16: mul r0.xy, r0.xyxx, cb0[5].xxxx
    r0.xy = ((r0.xyxx)*(source[5].xxxx)).xy;
    // 17: mad r0.zw, v4.xxxy, cb0[7].xxxy, r0.xxxy
    r0.zw = ((v4.xxxy)*(source[7].xxxy)+(r0.xxxy)).zw;
    // 18: mad r0.xy, v4.xyxx, cb0[3].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)+(r0.xyxx)).xy;
    // 19: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: add r0.xy, r0.zwzz, cb0[8].xyxx
    r0.xy = ((r0.zwzz)+(source[8].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 25: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 26: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 27: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 28: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 29: mul r0.xyz, r0.xyzx, cb0[13].yyyy
    r0.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 30: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, cb0[13].zzzz
    r0.xyz = ((r0.xyzx)*(source[13].zzzz)).xyz;
    // 32: mad r0.xyz, cb0[1].xyzx, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 33: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 34: mad r0.xy, v4.xyxx, cb0[9].xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)*(source[9].xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 35: dp2 r2.x, cb0[10].xyxx, r0.xyxx
    r2.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 36: dp2 r2.y, cb0[11].xyxx, r0.xyxx
    r2.y = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 37: add r0.xy, r2.xyxx, cb0[5].zyzz
    r0.xy = ((r2.xyxx)+(source[5].zyzz)).xy;
    // 38: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 39: mad r0.xy, r1.xxxx, cb0[14].zzzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[14].zzzz)+(r0.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 41: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 42: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 44: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 50: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 51: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 52: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 53: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 55: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_rotgather_01_4_tr: 01965d920a0f364aa039d408038ac971; selected map 45b62d096f2eb56cd462840236d06fb68543901dea13545391705474301ff0df.
float4 ArtistNative3153(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[5].yzyy
    r0.xy = ((v2.xyxx)*(source[5].yzyy)).xy;
    // 2: mad r0.xy, cb0[2].yyyy, cb0[5].xwxx, r0.xyxx
    r0.xy = ((source[2].yyyy)*(source[5].xwxx)+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyz = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 4: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 6: mad r0.xyz, cb0[6].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[6].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 7: mul r0.xyz, r0.xyzx, cb0[6].yyyy
    r0.xyz = ((r0.xyzx)*(source[6].yyyy)).xyz;
    // 8: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 9: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 10: mul r0.xyz, r0.xyzx, v4.zzzz
    r0.xyz = ((r0.xyzx)*(v4.zzzz)).xyz;
    // 11: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 12: mul r1.xy, v2.xyxx, cb0[2].zwzz
    r1.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 13: mad r2.x, cb0[2].y, cb0[2].x, r1.x
    r2.x = ((source[2].yyyy)*(source[2].xxxx)+(r1.xxxx)).x;
    // 14: mad r2.y, cb0[2].y, cb0[3].x, r1.y
    r2.y = ((source[2].yyyy)*(source[3].xxxx)+(r1.yyyy)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: mul r0.w, r0.w, v4.w
    r0.w = ((r0.wwww)*(v4.wwww)).w;
    // 17: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 18: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 19: mul r1.x, r1.x, cb0[3].y
    r1.x = ((r1.xxxx)*(source[3].yyyy)).x;
    // 20: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 21: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 22: mul r1.x, cb0[2].y, cb0[3].z
    r1.x = ((source[2].yyyy)*(source[3].zzzz)).x;
    // 23: mad r1.x, cb0[3].w, v2.x, r1.x
    r1.x = ((source[3].wwww)*(v2.xxxx)+(r1.xxxx)).x;
    // 24: mul r1.z, v2.y, cb0[4].x
    r1.z = ((v2.yyyy)*(source[4].xxxx)).z;
    // 25: mad r1.y, cb0[2].y, cb0[4].y, r1.z
    r1.y = ((source[2].yyyy)*(source[4].yyyy)+(r1.zzzz)).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: mad r1.xy, r1.xxxx, cb0[4].zzzz, v2.xyxx
    r1.xy = ((r1.xxxx)*(source[4].zzzz)+(v2.xyxx)).xy;
    // 28: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 29: sincos r2.x, r3.x, v4.x
    r2.x = (sin(v4.xxxx)).x; r3.x = (cos(v4.xxxx)).x;
    // 30: mov r4.x, -r2.x
    r4.x = (-(r2.xxxx)).x;
    // 31: mov r4.y, r3.x
    r4.y = (r3.xxxx).y;
    // 32: mov r4.z, r2.x
    r4.z = (r2.xxxx).z;
    // 33: dp2 r2.y, r4.zyzz, r1.xyxx
    r2.y = (dot((r4.zyzz).xy,(r1.xyxx).xy).xxxx).y;
    // 34: dp2 r2.x, r4.yxyy, r1.xyxx
    r2.x = (dot((r4.yxyy).xy,(r1.xyxx).xy).xxxx).x;
    // 35: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: mul r1.y, r1.x, cb0[4].w
    r1.y = ((r1.xxxx)*(source[4].wwww)).y;
    // 38: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 39: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 40: log r1.x, |r1.y|
    r1.x = (log2(abs(r1.yyyy))).x;
    // 41: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 42: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 43: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 44: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 45: mad r0.xyz, r1.xxxx, r0.xyzx, r0.wwww
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r0.wwww)).xyz;
    // 46: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 47: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_34_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative3154(ARTIST_NATIVE_INPUT input)
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
// fx_h_me_movepanlerp_01_2_ad: 79950b9981542c4587c2313e2866e5ac; selected map dd28fee51e89e66a4fdb11e90222259370a314bf6bd2824af80d6b3d9e2a7a2f.
float4 ArtistNative3155(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0700000003, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
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
    // 1: mad r0.xy, v4.xyxx, l(2.000000, 3.000000, 0.000000, 0.000000), cb0[7].xyxx
    r0.xy = ((v4.xyxx)*(float4(2.000000,3.000000,0.000000,0.000000))+(source[7].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: mad r0.xy, v4.xyxx, l(4.000000, 13.000000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v4.xyxx)*(float4(4.000000,13.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 5: add r0.xy, r0.xyxx, cb0[8].xyxx
    r0.xy = ((r0.xyxx)+(source[8].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mad r0.yz, v4.xxyx, l(0.000000, 8.000000, 8.000000, 0.000000), cb0[6].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,8.000000,8.000000,0.000000))+(source[6].xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 9: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 10: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 11: mul r0.x, |r0.x|, |r0.x|
    r0.x = ((abs(r0.xxxx))*(abs(r0.xxxx))).x;
    // 12: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 13: mov r0.y, l(0)
    r0.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 14: mul r0.z, cb0[3].x, l(-0.200000)
    r0.z = ((source[3].xxxx)*(float4(-0.200000,-0.200000,-0.200000,-0.200000))).z;
    // 15: mad r0.yz, v4.xxyx, l(0.000000, 1.000000, 3.000000, 0.000000), r0.yyzy
    r0.yz = ((v4.xxyx)*(float4(0.000000,1.000000,3.000000,0.000000))+(r0.yyzy)).yz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t0.zxyw, s0, l(0.000000)
    r0.yz = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 17: mad r0.yz, r0.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 18: mad r1.xy, v4.xyxx, l(1.500000, 1.000000, 0.000000, 0.000000), l(-0.250000, 0.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)*(float4(1.500000,1.000000,0.000000,0.000000))+(float4(-0.250000,0.000000,0.000000,0.000000))).xy;
    // 19: add r1.zw, cb0[3].wwwz, l(0.000000, 0.000000, -0.800000, -1.000000)
    r1.zw = ((source[3].wwwz)+(float4(0.000000,0.000000,-0.800000,-1.000000))).zw;
    // 20: mad r1.xy, r1.zzzz, r0.yzyy, r1.xyxx
    r1.xy = ((r1.zzzz)*(r0.yzyy)+(r1.xyxx)).xy;
    // 21: mad r1.z, cb0[3].x, l(0.300000), r1.y
    r1.z = ((source[3].xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.yyyy)).z;
    // 22: add r0.yz, r1.xxzx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r1.xxzx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 23: dp2 r1.x, cb0[4].xyxx, r0.yzyy
    r1.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 24: dp2 r1.y, cb0[5].xyxx, r0.yzyy
    r1.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 25: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 27: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 28: mul r0.w, r0.z, l(10.000000)
    r0.w = ((r0.zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 29: mul r0.xz, r0.wwzw, r0.xxzx
    r0.xz = ((r0.wwzw)*(r0.xxzx)).xz;
    // 30: mul r0.z, r0.z, l(20.000000)
    r0.z = ((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000))).z;
    // 31: mad r0.y, r0.y, l(1.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))+(r0.zzzz)).y;
    // 32: mad r0.x, r0.x, l(3000.000000), -r0.y
    r0.x = ((r0.xxxx)*(float4(3000.000000,3000.000000,3000.000000,3000.000000))+(-(r0.yyyy))).x;
    // 33: mov r1.x, l(0)
    r1.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 34: mul r1.y, cb0[3].x, l(0.200000)
    r1.y = ((source[3].xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 35: add r0.zw, r1.xxxy, v4.xxxy
    r0.zw = ((r1.xxxy)+(v4.xxxy)).zw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: add r0.w, r0.z, cb0[3].y
    r0.w = ((r0.zzzz)+(source[3].yyyy)).w;
    // 38: add r0.z, r0.z, l(0.100000)
    r0.z = ((r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 39: add r0.z, -r1.w, r0.z
    r0.z = ((-(r1.wwww))+(r0.zzzz)).z;
    // 40: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 41: mad r0.x, r0.w, r0.x, r0.y
    r0.x = ((r0.wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 42: add r0.yw, -v4.yyyx, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = ((-(v4.yyyx))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 43: mul r0.yw, r0.yyyw, v4.yyyx
    r0.yw = ((r0.yyyw)*(v4.yyyx)).yw;
    // 44: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 45: mul_sat r0.yz, r0.yyzy, l(0.000000, 100.000000, 20.000000, 0.000000)
    r0.yz = (saturate((r0.yyzy)*(float4(0.000000,100.000000,20.000000,0.000000)))).yz;
    // 46: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 47: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 48: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 49: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 50: mul r0.w, cb0[0].x, cb0[1].w
    r0.w = ((source[0].xxxx)*(source[1].wwww)).w;
    // 51: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 52: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_21_tr: f77f84bb00757048a8cbf965b1d6f276; selected map 004a31e283bc7194223ff241d8722eaf6bc6ab70cdf452f4d336436cc61fcd58.
float4 ArtistNative3156(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].zzzz,g_ArtistSourceMaterialParameters[5u].wwww,1u);
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
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
    // 49: mad r0.xy, r0.xxxx, cb0[14].zzzz, r0.yzyy
    r0.xy = ((r0.xxxx)*(source[14].zzzz)+(r0.yzyy)).xy;
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
    // 55: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 59: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 60: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 61: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 62: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, cb0[15].y
    r0.x = (saturate((r0.xxxx)*(source[15].yyyy))).x;
    // 64: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 65: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_n_pa_decmaster_01_20_tr: 3c48bb45666c774aaa7572418dbcfdd8; selected map 13642ba4ed9b478a9ea2621afa27c8738db876b860f7c64f128af60465c4f30c.
float4 ArtistNative3157(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = g_ArtistSourceMaterialParameters[4u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ArtistSourceMaterialParameters[3u];
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 5: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 6: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 7: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: dp3 r0.x, r0.xyzx, r0.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 9: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 10: div r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)/(r0.xxxx)).x;
    // 11: max r0.x, r0.x, l(0.150000)
    r0.x = (max(r0.xxxx,float4(0.150000,0.150000,0.150000,0.150000))).x;
    // 12: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 14: mad r0.yz, r0.yyzy, cb0[3].xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r0.yyzy)*(source[3].xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t5.xyzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 16: mul r0.zw, v2.xxxy, cb0[6].xxxx
    r0.zw = ((v2.xxxy)*(source[6].xxxx)).zw;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t3.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 18: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.200000, 0.200000), v2.xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.200000,0.200000))+(v2.xxxy)).zw;
    // 19: add r0.zw, r0.zzzw, cb0[4].xxxy
    r0.zw = ((r0.zzzw)+(source[4].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: add r0.z, r0.z, r0.y
    r0.z = ((r0.zzzz)+(r0.yyyy)).z;
    // 22: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 23: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 24: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 28: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 29: mul r2.xyz, r2.xyzx, v3.xyzx
    r2.xyz = ((r2.xyzx)*(v3.xyzx)).xyz;
    // 30: movc r0.yzw, r0.yyyy, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 31: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 33: mad r1.xyz, cb0[6].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[6].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 34: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 35: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 36: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 37: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 38: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 39: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 40: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 41: source device depth mapped to centimetre view depth; reconstruction at 43.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 43-46: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 47: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 48: add r0.y, -cb0[8].w, l(1.000000)
    r0.y = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 50: div_sat r0.y, r0.x, r0.y
    r0.y = (saturate((r0.xxxx)/(r0.yyyy))).y;
    // 51: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 52: add r0.y, -r0.y, v4.x
    r0.y = ((-(r0.yyyy))+(v4.xxxx)).y;
    // 53: min r0.y, |r0.y|, l(1.000000)
    r0.y = (min(abs(r0.yyyy),float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: add r0.z, -cb0[9].x, l(1.000000)
    r0.z = ((-(source[9].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 55: max r0.z, -r0.z, l(0.001000)
    r0.z = (max(-(r0.zzzz),float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 56: div_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)/(r0.zzzz))).x;
    // 57: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 58: mov_sat r0.yz, v2.xxyx
    r0.yz = (saturate(v2.xxyx)).yz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 60: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 61: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 62: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)*(source[8].yyyy)).z;
    // 64: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 65: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 66: mul r0.zw, v2.xxxy, cb0[8].zzzz
    r0.zw = ((v2.xxxy)*(source[8].zzzz)).zw;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s5, l(0.000000)
    r0.z = (ArtistNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 68: mov_sat r0.w, v3.w
    r0.w = (saturate(v3.wwww)).w;
    // 69: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 70: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 71: mad r0.y, -r0.z, l(10.000000), r0.y
    r0.y = ((-(r0.zzzz))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.yyyy)).y;
    // 72: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 73: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 74: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_me_makeflow_05_02_tr: f77f84bb00757048a8cbf965b1d6f276; selected map a1c7a5d3ff928dd3db26d6e855af7ebaa149da3ec1e7137601debc20686f8343.
float4 ArtistNative3158(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[12].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
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
    // 49: mad r0.xy, r0.xxxx, cb0[14].zzzz, r0.yzyy
    r0.xy = ((r0.xxxx)*(source[14].zzzz)+(r0.yzyy)).xy;
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
    // 55: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 56: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 57: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 58: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 59: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 60: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 61: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 62: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 63: mul_sat r0.x, r0.x, cb0[15].y
    r0.x = (saturate((r0.xxxx)*(source[15].yyyy))).x;
    // 64: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 65: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_01_12_tr: ad42f283a770bb4ca78f8baa88727bbd; selected map dae87ca73a1142a5b49a2e9752391354f42ce3d26558852fb0e6ef85a5fadca4.
float4 ArtistNative3159(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].xxxx,g_ArtistSourceMaterialParameters[3u].yyyy,1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].zzzz,g_ArtistSourceMaterialParameters[4u].wwww,1u);
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.xyzw, v4.xyxy, cb0[4].xyxy, r0.yxxy
    r0.xyzw = ((v4.xyxy)*(source[4].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.zw, v4.xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)).zw;
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
    // 16: mul r0.xy, r0.xyxx, cb0[5].xxxx
    r0.xy = ((r0.xyxx)*(source[5].xxxx)).xy;
    // 17: mad r0.zw, v4.xxxy, cb0[7].xxxy, r0.xxxy
    r0.zw = ((v4.xxxy)*(source[7].xxxy)+(r0.xxxy)).zw;
    // 18: mad r0.xy, v4.xyxx, cb0[3].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)+(r0.xyxx)).xy;
    // 19: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: add r0.xy, r0.zwzz, cb0[8].xyxx
    r0.xy = ((r0.zwzz)+(source[8].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 25: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 26: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 27: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 28: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 29: mul r0.xyz, r0.xyzx, cb0[13].yyyy
    r0.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 30: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, cb0[13].zzzz
    r0.xyz = ((r0.xyzx)*(source[13].zzzz)).xyz;
    // 32: mad r0.xyz, cb0[1].xyzx, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 33: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 34: mad r0.xy, v4.xyxx, cb0[9].xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)*(source[9].xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 35: dp2 r2.x, cb0[10].xyxx, r0.xyxx
    r2.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 36: dp2 r2.y, cb0[11].xyxx, r0.xyxx
    r2.y = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 37: add r0.xy, r2.xyxx, cb0[5].zyzz
    r0.xy = ((r2.xyxx)+(source[5].zyzz)).xy;
    // 38: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 39: mad r0.xy, r1.xxxx, cb0[14].zzzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[14].zzzz)+(r0.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 41: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 42: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 44: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 50: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 51: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 52: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 53: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 55: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_crackmove_05_tr: dc451ebd84b20547a455375494444b0f; selected map cf36e4c40f41db0f282f9659348265656861acc471186fc7e590e946902bc14b.
float4 ArtistNative3160(ARTIST_NATIVE_INPUT input)
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
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_me_watertrail_01_1_tr: 70bf2a6e9bf4f0478cecbfc43c4e160f; selected map 54a52a78d4c82bd42962193bcb3a64e28cce1275eb69c9affb1d3006478abcc3.
float4 ArtistNative3161(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[7].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_waterflow_02_54_tr: d9f2c7cdd613b64d9edfa91fd56b4360; selected map 57e2aaddb6b601020326685c9f27458cf177068c4a1bff5d11f64647f85b5ddb.
float4 ArtistNative3162(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_v_pa_waterflow_03_51_tr: d9f2c7cdd613b64d9edfa91fd56b4360; selected map 57e2aaddb6b601020326685c9f27458cf177068c4a1bff5d11f64647f85b5ddb.
float4 ArtistNative3163(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_atta_05_14_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative3164(ARTIST_NATIVE_INPUT input)
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
// fx_k_me_makeflow_02_31_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ArtistNative3165(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_s_maskedrib_01_2_tr: 2668cf8f704b6847822e1dfb58f7c37e; selected map 24eae5b4cb3a3750e750c3b9832b32d158b3e81ce7460b623c33c715c1b64a33.
float4 ArtistNative3166(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[3] = g_ArtistSourceMaterialParameters[4u];
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: mad r0.x, v2.x, l(2.000000), l(-1.000000)
    r0.x = ((v2.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 4: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 5: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 6: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 7: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 8: mul r1.x, v2.z, cb0[6].w
    r1.x = ((v2.zzzz)*(source[6].wwww)).x;
    // 9: mul r0.y, v2.w, v4.y
    r0.y = ((v2.wwww)*(v4.yyyy)).y;
    // 10: mul r1.y, r0.y, cb0[7].x
    r1.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 11: mul r2.y, r0.y, cb0[5].y
    r2.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 12: mul r0.yz, r1.xxyx, l(0.000000, 1.500000, 1.000000, 0.000000)
    r0.yz = ((r1.xxyx)*(float4(0.000000,1.500000,1.000000,0.000000))).yz;
    // 13: add r1.xy, r1.xyxx, cb0[4].xyxx
    r1.xy = ((r1.xyxx)+(source[4].xyxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mad r0.yz, r0.yyyy, v4.wwww, r1.xxyx
    r0.yz = ((r0.yyyy)*(v4.wwww)+(r1.xxyx)).yz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 17: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 18: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)*(v4.zzzz)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 22: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 23: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 24: add r0.y, -v2.x, l(1.000000)
    r0.y = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 25: mul r0.z, |r0.y|, |r0.y|
    r0.z = ((abs(r0.yyyy))*(abs(r0.yyyy))).z;
    // 26: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 27: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 28: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 29: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 30: mul r0.z, r0.x, r0.y
    r0.z = ((r0.xxxx)*(r0.yyyy)).z;
    // 31: mad r0.x, r0.z, l(7.000000), r0.x
    r0.x = ((r0.zzzz)*(float4(7.000000,7.000000,7.000000,7.000000))+(r0.xxxx)).x;
    // 32: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 34: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 35: mul r2.x, v2.z, cb0[5].x
    r2.x = ((v2.zzzz)*(source[5].xxxx)).x;
    // 36: add r0.xz, r2.xxyx, cb0[2].xxyx
    r0.xz = ((r2.xxyx)+(source[2].xxyx)).xz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t2.xwyz, s0, l(0.000000)
    r0.xzw = (ArtistNativeSample0((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 38: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 39: add r1.xyz, -r0.xzwx, r1.xxxx
    r1.xyz = ((-(r0.xzwx))+(r1.xxxx)).xyz;
    // 40: mad r0.xzw, cb0[6].xxxx, r1.xxyz, r0.xxzw
    r0.xzw = ((source[6].xxxx)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 41: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 42: log r0.xzw, r0.xxzw
    r0.xzw = (log2(r0.xxzw)).xzw;
    // 43: mul r0.xzw, r0.xxzw, cb0[6].yyyy
    r0.xzw = ((r0.xxzw)*(source[6].yyyy)).xzw;
    // 44: exp r0.xzw, r0.xxzw
    r0.xzw = (exp2(r0.xxzw)).xzw;
    // 45: mul r0.xzw, r0.xxzw, cb0[3].xxyz
    r0.xzw = ((r0.xxzw)*(source[3].xxyz)).xzw;
    // 46: mad_sat r0.xyz, cb0[6].zzzz, r0.xzwx, r0.yyyy
    r0.xyz = (saturate((source[6].zzzz)*(r0.xzwx)+(r0.yyyy))).xyz;
    // 47: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 48: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_de_master_01_12_tr: 2b965d3b41e96649a3bdbb1f627220f5; selected map 74902d0c9eec9341b1997fd355e800f0db2ae4db3af032f3b060dd308d5a999d.
float4 ArtistNative3167(ARTIST_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=float4(input.decalProjection.xy,0.f,0.f);
    source[1]=input.color; // Source decal material color, including particle color modules.
    source[2].x=input.decalProjection.z;
    source[15]=float4(input.skyUpperColor,0.f);
    source[16]=float4(input.skyLowerColor,0.f);
    source[17]=float4(input.ambientColor,input.skyIntensity);
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = g_ArtistSourceMaterialParameters[5u];
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[6] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].zzzz,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[8] = (g_ArtistSourceMaterialTime.xxxx*ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,float4(0.0, 0.0, 0.0, 0.0),1u));
    source[9] = g_ArtistSourceMaterialParameters[4u];
    source[10].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
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
    // 19: mad r1.zw, v4.xxxy, cb0[5].xxxy, r1.xxxy
    r1.zw = ((v4.xxxy)*(source[5].xxxy)+(r1.xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t4.zwxy, s2, l(0.000000)
    r1.zw = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 21: mad r2.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 23: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 26: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 28: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 29: div r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)/(r0.wwww)).w;
    // 30: mul r0.w, r0.w, cb0[6].z
    r0.w = ((r0.wwww)*(source[6].zzzz)).w;
    // 31: max r0.w, r0.w, l(0.150000)
    r0.w = (max(r0.wwww,float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 32: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r1.zw, cb0[10].xxxx, v4.xxxy, r1.xxxy
    r1.zw = ((source[10].xxxx)*(v4.xxxy)+(r1.xxxy)).zw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t3.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 35: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 36: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 37: mad r3.xyz, cb0[10].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 38: mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 0.200000, 0.200000), v4.xxxy
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,0.200000,0.200000))+(v4.xxxy)).zw;
    // 39: add r1.zw, r1.zzzw, cb0[8].xxxy
    r1.zw = ((r1.zzzw)+(source[8].xxxy)).zw;
    // 40: mad r1.zw, r0.xxxx, r0.yyyz, r1.zzzw
    r1.zw = ((r0.xxxx)*(r0.yyyz)+(r1.zzzw)).zw;
    // 41: mad_sat r0.xy, r0.xxxx, r0.yzyy, v4.xyxx
    r0.xy = (saturate((r0.xxxx)*(r0.yzyy)+(v4.xyxx))).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.wxyz, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 43: mul r0.x, r0.x, cb0[11].w
    r0.x = ((r0.xxxx)*(source[11].wwww)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t3.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 45: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 46: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 47: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 48: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 49: mad r0.zw, r0.zzzw, cb0[7].xxxy, r1.xxxy
    r0.zw = ((r0.zzzw)*(source[7].xxxy)+(r1.xxxy)).zw;
    // 50: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.xzyw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 52: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 54: mul r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 55: mul r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 56: add r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)+(r0.zzzz)).w;
    // 57: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 58: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 59: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 60: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 61: mul r0.y, r0.y, cb0[11].z
    r0.y = ((r0.yyyy)*(source[11].zzzz)).y;
    // 62: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 63: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 64: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 65: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 66: movc r0.yzw, r0.zzzz, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 67: mad r0.yzw, r1.xxyz, l(0.000000, 0.150000, 0.150000, 0.150000), r0.yyzw
    r0.yzw = ((r1.xxyz)*(float4(0.000000,0.150000,0.150000,0.150000))+(r0.yyzw)).yzw;
    // 68: mul r1.xyz, r1.xyzx, cb2[3].wwww
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)).xyz;
    // 69: mad r1.xyz, r1.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 70: add r0.yzw, r0.yyzw, cb0[3].xxyz
    r0.yzw = ((r0.yyzw)+(source[3].xxyz)).yzw;
    // 71: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 72: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 73: mul r1.w, r1.w, v7.z
    r1.w = ((r1.wwww)*(v7.zzzz)).w;
    // 74: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 76: mul r2.yzw, r2.yyyy, cb0[16].xxyz
    r2.yzw = ((r2.yyyy)*(source[16].xxyz)).yzw;
    // 77: mad r2.xyz, r2.xxxx, cb0[15].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[15].xyzx)+(r2.yzwy)).xyz;
    // 78: mul r2.xyz, r2.xyzx, cb0[17].wwww
    r2.xyz = ((r2.xyzx)*(source[17].wwww)).xyz;
    // 79: mad r0.yzw, r2.xxyz, r1.xxyz, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 80: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 82: mad r0.yzw, r1.xxyz, cb0[17].xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(source[17].xxyz)+(r0.yyzw)).yzw;
    // 84: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 85: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 86: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 87: mul r0.y, r0.y, cb0[12].x
    r0.y = ((r0.yyyy)*(source[12].xxxx)).y;
    // 88: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 89: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 90: mul r0.yz, v4.xxyx, cb0[12].yyyy
    r0.yz = ((v4.xxyx)*(source[12].yyyy)).yz;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 92: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 93: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 94: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 95: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 96: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 97: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 98: add r0.y, -|v4.w|, cb0[0].y
    r0.y = ((-(abs(v4.wwww)))+(source[0].yyyy)).y;
    // 99: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 100: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 101: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 102: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_me_lightning_03_ad: 7fb17aadf7b64e41af802ddf76c3b62c; selected map 3b1bde9dd8b91d644da91cb0e026a0232866858271e4f3c2198653713505abc0.
float4 ArtistNative3168(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: mad r0.yz, v4.xxyx, l(0.000000, 3.000000, 3.000000, 0.000000), r0.xxxx
    r0.yz = ((v4.xxyx)*(float4(0.000000,3.000000,3.000000,0.000000))+(r0.xxxx)).yz;
    // 3: add r0.xw, r0.xxxx, v4.xxxy
    r0.xw = ((r0.xxxx)+(v4.xxxy)).xw;
    // 4: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 5: mul r2.x, cb0[3].x, cb0[4].x
    r2.x = ((source[3].xxxx)*(source[4].xxxx)).x;
    // 6: mul r3.xzw, r2.xxxx, l(0.200000, 0.000000, -0.200000, 0.700000)
    r3.xzw = ((r2.xxxx)*(float4(0.200000,0.000000,-0.200000,0.700000))).xzw;
    // 7: mad r2.xy, r2.xxxx, l(-0.100000, -0.100000, 0.000000, 0.000000), r0.xwxx
    r2.xy = ((r2.xxxx)*(float4(-0.100000,-0.100000,0.000000,0.000000))+(r0.xwxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: mov r1.yw, r3.zzzw
    r1.yw = (r3.zzzw).yw;
    // 10: add r0.yz, r0.yyzy, r1.xxyx
    r0.yz = ((r0.yyzy)+(r1.xxyx)).yz;
    // 11: add r1.xy, r0.xwxx, r1.zwzz
    r1.xy = ((r0.xwxx)+(r1.zwzz)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyz = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.yzyy, t2.xyzw, s2, l(0.000000)
    r4.xyz = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, l(400.000000)
    r0.y = ((r0.yyyy)*(float4(400.000000,400.000000,400.000000,400.000000))).y;
    // 16: mul r2.xyz, r2.xyzx, r0.yyyy
    r2.xyz = ((r2.xyzx)*(r0.yyyy)).xyz;
    // 17: add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // 18: mov r3.y, l(0)
    r3.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 19: add r0.xy, r0.xwxx, r3.xyxx
    r0.xy = ((r0.xwxx)+(r3.xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r0.xyz = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 22: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 23: mad r0.xyz, cb0[1].xyzx, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 24: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 25: mul r0.w, cb0[0].x, cb0[1].w
    r0.w = ((source[0].xxxx)*(source[1].wwww)).w;
    // 26: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 27: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_gl_01_2_ad: b0d551a53864c9408d2c03215f9ef88f; selected map 4dd93e1b4ffae10c2db3ed0c4c9132c6db631aa6aa2fdbffcc7ea3e1b5d89ab4.
float4 ArtistNative3169(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_rampshape_01_05_ad: 103a9b035e2cdf478664fac24e797f6f; selected map 1d2fc584c5e9ed63771635d086940889d38c063a1693af4d6e59d037bcbe3ed9.
float4 ArtistNative3170(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].yyyy,g_ArtistSourceMaterialParameters[0u].zzzz,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx)*float4(-0.699999988, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 23: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 24: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 25: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 26: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 27: mad r0.z, r0.z, l(0.318471), l(1.000000)
    r0.z = ((r0.zzzz)*(float4(0.318471,0.318471,0.318471,0.318471))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: mul r0.x, r0.z, l(0.500000)
    r0.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 29: mad r0.zw, r0.xxxy, cb0[3].xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)+(source[4].xxxy)).zw;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 31: mul r0.w, r0.z, cb0[6].x
    r0.w = ((r0.zzzz)*(source[6].xxxx)).w;
    // 32: add_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)+(v3.wwww))).z;
    // 33: mad r0.xw, cb0[2].xxxy, r0.xxxy, r0.wwww
    r0.xw = ((source[2].xxxy)*(r0.xxxy)+(r0.wwww)).xw;
    // 34: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 38: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 39: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 40: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 41: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 42: lt r1.x, r0.y, l(0.000001)
    r1.x = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 43: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 44: mad_sat r0.x, r0.x, r0.y, r0.w
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(r0.wwww))).x;
    // 45: add r0.x, r0.x, v4.x
    r0.x = ((r0.xxxx)+(v4.xxxx)).x;
    // 46: mul r0.xy, r0.xxxx, l(0.800000, 5.026548, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.800000,5.026548,0.000000,0.000000))).xy;
    // 47: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 48: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 50: mul r1.x, r0.w, cb0[5].x
    r1.x = ((r0.wwww)*(source[5].xxxx)).x;
    // 51: mad r0.x, r0.w, cb0[5].x, r0.x
    r0.x = ((r0.wwww)*(source[5].xxxx)+(r0.xxxx)).x;
    // 52: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 53: mul_sat r0.x, r0.x, v4.w
    r0.x = (saturate((r0.xxxx)*(v4.wwww))).x;
    // 54: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 55: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 56: mul r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)*(r0.xxxx)).w;
    // 57: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 58: mul r0.w, r0.w, r0.x
    r0.w = ((r0.wwww)*(r0.xxxx)).w;
    // 59: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 60: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 61: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 62: mul r0.y, r0.y, r0.x
    r0.y = ((r0.yyyy)*(r0.xxxx)).y;
    // 63: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 64: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 65: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 66: mul r0.y, r0.y, l(3.000000)
    r0.y = ((r0.yyyy)*(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 67: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 68: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 69: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 70: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_i_pa_darkaurapan_01_tr: c32c8cb20b653f4194a5f1f195a26bdc; selected map 53f0efe4f46d4b6abc87da298fcb1770d11c8c7d0618b2101b1a7ef3b9c2be6f.
float4 ArtistNative3171(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
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
    // 1: add r0.x, -|v2.y|, l(1.000000)
    r0.x = ((-(abs(v2.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 3: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 4: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 5: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 6: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 7: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 8: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 9: mad r0.yz, v2.xxyx, l(0.000000, 2.000000, 1.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((v2.xxyx)*(float4(0.000000,2.000000,1.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 10: add r0.yz, -|r0.yyzy|, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(abs(r0.yyzy)))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 11: mul r1.xy, |r0.yzyy|, |r0.yzyy|
    r1.xy = ((abs(r0.yzyy))*(abs(r0.yzyy))).xy;
    // 12: mul r0.w, r1.y, r1.y
    r0.w = ((r1.yyyy)*(r1.yyyy)).w;
    // 13: mul r0.w, r0.w, |r0.z|
    r0.w = ((r0.wwww)*(abs(r0.zzzz))).w;
    // 14: lt r0.yz, |r0.yyzy|, l(0.000000, 0.000001, 0.000001, 0.000000)
    r0.yz = (asfloat((uint4)((abs(r0.yyzy))<(float4(0.000000,0.000001,0.000001,0.000000))) * 0xffffffffu)).yz;
    // 15: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 16: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 17: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 18: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 19: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 20: mul r0.x, r0.x, l(5.000000)
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 21: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 22: mul r0.xzw, v2.yyxy, l(0.500000, 0.000000, 2.000000, 0.400000)
    r0.xzw = ((v2.yyxy)*(float4(0.500000,0.000000,2.000000,0.400000))).xzw;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.y, r0.y, l(1.300000)
    r0.y = ((r0.yyyy)*(float4(1.300000,1.300000,1.300000,1.300000))).y;
    // 26: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 27: movc r0.y, r0.x, l(0), r0.y
    r0.y = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 28: mov r0.x, v2.x
    r0.x = (v2.xxxx).x;
    // 29: add r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)+(source[2].xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 31: mad r0.xy, r0.zzzz, l(0.065000, 0.065000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r0.zzzz)*(float4(0.065000,0.065000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 33: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 34: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 35: mul r0.y, r0.y, l(1.800000)
    r0.y = ((r0.yyyy)*(float4(1.800000,1.800000,1.800000,1.800000))).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 38: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
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
// fx_o_pa_shine_02_22_tr: bd32b9177680a148a111bbb69cd349ec; selected map 8c382813b9e1ce909f00fb4e7aae9e1ff85f5e0fb081b7a592ad19c17ca380c9.
float4 ArtistNative3172(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 6: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 7: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 8: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 9: lt r0.w, |r0.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 10: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 11: add r0.yw, -r0.xxxy, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = ((-(r0.xxxy))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 12: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 13: mul_sat r0.y, r0.w, cb0[5].z
    r0.y = (saturate((r0.wwww)*(source[5].zzzz))).y;
    // 14: mul r0.w, r0.x, l(4.000000)
    r0.w = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 15: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 16: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 17: mul r0.w, r0.w, cb0[4].z
    r0.w = ((r0.wwww)*(source[4].zzzz)).w;
    // 18: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 19: mul r0.w, r0.w, cb0[4].w
    r0.w = ((r0.wwww)*(source[4].wwww)).w;
    // 20: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 21: mul_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)*(r0.xxxx))).x;
    // 22: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 23: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 24: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 25: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 26: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 27: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 28: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 29: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 30: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 31: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_02_tr: e40b438f7d7cc640821309314c4e94cc; selected map 94cc1ea759f182b4457be9fb389031fd373fd9eaf93d09f1b65fae491c008ed0.
float4 ArtistNative3173(ARTIST_NATIVE_INPUT input)
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
    // 9: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 10: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 11: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_makeflow_01_05_tr: 9772d84a6e5142409d0c87d36bb0afe5; selected map 3d808d79d784bc1cff9f1d1afb2ab903470c1003c99cca306eacfef61040b0d5.
float4 ArtistNative3174(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),1u);
    source[5] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[7] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[8] = g_ArtistSourceMaterialParameters[6u];
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
    // 22: dp2 r0.x, -r0.xyxx, -r0.xyxx
    r0.x = (dot((-(r0.xyxx)).xy,(-(r0.xyxx)).xy).xxxx).x;
    // 23: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 24: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 25: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 26: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 27: mad r0.z, r0.z, l(0.318471), l(1.000000)
    r0.z = ((r0.zzzz)*(float4(0.318471,0.318471,0.318471,0.318471))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: mul r0.x, r0.z, l(0.500000)
    r0.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 29: mad r0.zw, r0.xxxy, cb0[3].xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)+(source[4].xxxy)).zw;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.zwzz, t1.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 31: mov r2.y, cb0[12].z
    r2.y = (source[12].zzzz).y;
    // 32: mov r2.xz, l(0,0,0,0)
    r2.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 33: add r3.xyzw, r0.zwzw, r2.yxxy
    r3.xyzw = ((r0.zwzw)+(r2.yxxy)).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r3.xyxx, t1.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.zwzz, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: add r2.xy, -r1.xxxx, r0.zwzz
    r2.xy = ((-(r1.xxxx))+(r0.zwzz)).xy;
    // 37: mul r1.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 38: mov r1.z, l(0)
    r1.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 39: add r1.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 40: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 41: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 42: div r0.zw, r1.xxxy, r0.zzzz
    r0.zw = ((r1.xxxy)/(r0.zzzz)).zw;
    // 43: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 44: mul r0.zw, r0.zzzw, v4.xxxx
    r0.zw = ((r0.zzzw)*(v4.xxxx)).zw;
    // 45: mad r1.xy, r0.xyxx, cb0[2].xyxx, r0.zwzz
    r1.xy = ((r0.xyxx)*(source[2].xyxx)+(r0.zwzz)).xy;
    // 46: mov r2.w, v4.w
    r2.w = (v4.wwww).w;
    // 47: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 48: add r1.xy, r1.xyxx, cb0[5].xyxx
    r1.xy = ((r1.xyxx)+(source[5].xyxx)).xy;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 50: mad r2.xy, r0.xyxx, cb0[6].xyxx, r0.zwzz
    r2.xy = ((r0.xyxx)*(source[6].xyxx)+(r0.zwzz)).xy;
    // 51: mad r0.xy, r0.xyxx, cb0[9].xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(source[9].xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 52: add r2.xy, r2.xyxx, cb0[7].xyxx
    r2.xy = ((r2.xyxx)+(source[7].xyxx)).xy;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 54: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 55: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: mad r1.xyz, -r1.xyzx, r2.xyzx, r1.wwww
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 57: mad r1.xyz, cb0[13].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[13].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 58: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 59: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 60: mul r1.xyz, r1.xyzx, cb0[13].yyyy
    r1.xyz = ((r1.xyzx)*(source[13].yyyy)).xyz;
    // 61: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 62: mad r1.xyz, cb0[13].zzzz, r1.xyzx, cb0[8].xyzx
    r1.xyz = ((source[13].zzzz)*(r1.xyzx)+(source[8].xyzx)).xyz;
    // 63: mad r1.xyz, v3.xyzx, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((v3.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 64: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 65: dp2 r1.x, cb0[10].xyxx, r0.xyxx
    r1.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 66: dp2 r1.y, cb0[11].xyxx, r0.xyxx
    r1.y = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 67: add r0.xy, r1.xyxx, v4.zyzz
    r0.xy = ((r1.xyxx)+(v4.zyzz)).xy;
    // 68: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 69: mad r0.xy, cb0[14].zzzz, r0.zwzz, r0.xyxx
    r0.xy = ((source[14].zzzz)*(r0.zwzz)+(r0.xyxx)).xy;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 72: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 73: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 74: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 75: mul r0.x, r0.x, cb0[14].w
    r0.x = ((r0.xxxx)*(source[14].wwww)).x;
    // 76: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 77: mul_sat r0.x, r0.x, cb0[15].x
    r0.x = (saturate((r0.xxxx)*(source[15].xxxx))).x;
    // 78: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 79: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_missiletrail_01_1_tr: cdef428a9b88db42a4d7a5fa664c6446; selected map 071ff3626fe9535a255d8f0a0fa0bae4a7336aa6056f63c8362302af85cfbf02.
float4 ArtistNative3175(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].yyyy,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[6].x = (cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[8].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
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
    // 1: mul r0.x, v2.x, cb0[9].w
    r0.x = ((v2.xxxx)*(source[9].wwww)).x;
    // 2: mul r0.y, v2.y, cb0[10].x
    r0.y = ((v2.yyyy)*(source[10].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 4: mul r0.zw, v4.zzzz, l(0.000000, 0.000000, 0.080000, -0.100000)
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,0.080000,-0.100000))).zw;
    // 5: mad r0.zw, v2.xxxy, cb0[7].yyyz, r0.zzzw
    r0.zw = ((v2.xxxy)*(source[7].yyyz)+(r0.zzzw)).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mov r1.xz, r0.zzzz
    r1.xz = (r0.zzzz).xz;
    // 8: mul r2.xyzw, v2.xyxy, cb0[8].xyzw
    r2.xyzw = ((v2.xyxy)*(source[8].xyzw)).xyzw;
    // 9: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, -0.050000, -0.050000), r2.xxxy
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,-0.050000,-0.050000))+(r2.xxxy)).zw;
    // 10: mad r2.xy, v4.xxxx, l(0.080000, 3.000000, 0.000000, 0.000000), r2.zwzz
    r2.xy = ((v4.xxxx)*(float4(0.080000,3.000000,0.000000,0.000000))+(r2.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.x = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.xzyw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 14: mov r1.yw, r0.zzzz
    r1.yw = (r0.zzzz).yw;
    // 15: mad r0.z, v2.y, cb0[7].z, cb0[7].w
    r0.z = ((v2.yyyy)*(source[7].zzzz)+(source[7].wwww)).z;
    // 16: mul r1.xyzw, r1.xyzw, r0.zzzz
    r1.xyzw = ((r1.xyzw)*(r0.zzzz)).xyzw;
    // 17: mul r1.xyzw, r1.xyzw, v4.yyyy
    r1.xyzw = ((r1.xyzw)*(v4.yyyy)).xyzw;
    // 18: mad r0.xy, r1.xyxx, l(0.400000, 0.400000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.xyxx)*(float4(0.400000,0.400000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 19: add r1.xy, v4.xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 20: mad r0.z, r1.x, l(0.300000), r0.y
    r0.z = ((r1.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy)).z;
    // 21: add r0.xy, r0.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 22: dp2 r3.x, cb0[3].xyxx, r0.xyxx
    r3.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: dp2 r3.y, cb0[4].xyxx, r0.xyxx
    r3.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 24: add r0.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 26: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 27: mul r0.yz, v2.xxyx, cb0[11].zzwz
    r0.yz = ((v2.xxyx)*(source[11].zzwz)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: mad r0.y, r0.z, cb0[11].y, r0.y
    r0.y = ((r0.zzzz)*(source[11].yyyy)+(r0.yyyy)).y;
    // 31: mul r0.z, r0.z, v2.y
    r0.z = ((r0.zzzz)*(v2.yyyy)).z;
    // 32: mul_sat r0.z, r0.z, cb0[11].x
    r0.z = (saturate((r0.zzzz)*(source[11].xxxx))).z;
    // 33: add r0.y, -r1.y, r0.y
    r0.y = ((-(r1.yyyy))+(r0.yyyy)).y;
    // 34: mul r0.w, r1.x, cb0[6].y
    r0.w = ((r1.xxxx)*(source[6].yyyy)).w;
    // 35: mul_sat r0.y, r0.y, cb0[12].x
    r0.y = (saturate((r0.yyyy)*(source[12].xxxx))).y;
    // 36: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 37: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 39: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 40: mad r0.xy, v2.xyxx, cb0[6].zyzz, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[6].zyzz)+(source[2].xyxx)).xy;
    // 41: mad r0.xy, r1.zwzz, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.zwzz)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 42: mad r0.z, r0.w, l(0.300000), r0.y
    r0.z = ((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy)).z;
    // 43: add r0.xy, r0.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 44: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 45: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 46: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: mul r0.y, r2.x, r0.x
    r0.y = ((r2.xxxx)*(r0.xxxx)).y;
    // 49: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 50: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 51: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 52: mul r0.y, r0.y, cb0[9].x
    r0.y = ((r0.yyyy)*(source[9].xxxx)).y;
    // 53: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 54: mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // 55: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 56: mad r0.x, r0.x, cb0[9].z, r0.y
    r0.x = ((r0.xxxx)*(source[9].zzzz)+(r0.yyyy)).x;
    // 57: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_beam_01_01_ad: c5f94926451d2442898502c8b6fe0a21; selected map f60cd45128435daae385c313cb0d357142f8ba01e54107379cb6d71e64786a91.
float4 ArtistNative3176(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[5u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].yyyy,float4(1.0, 0.0, 0.0, 0.0),1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,float4(1.0, 0.0, 0.0, 0.0),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[6].xyxx
    r0.xy = ((v2.xyxx)*(source[6].xyxx)).xy;
    // 2: mad r1.x, cb0[4].y, cb0[5].w, r0.x
    r1.x = ((source[4].yyyy)*(source[5].wwww)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[4].y, cb0[6].z, r0.y
    r1.y = ((source[4].yyyy)*(source[6].zzzz)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 6: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 7: mul r0.y, r0.y, cb0[6].w
    r0.y = ((r0.yyyy)*(source[6].wwww)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 10: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 11: mad r0.y, v2.x, l(2.000000), l(-1.000000)
    r0.y = ((v2.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 12: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 13: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 14: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 15: mul r0.z, r0.z, cb0[7].y
    r0.z = ((r0.zzzz)*(source[7].yyyy)).z;
    // 16: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 17: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 18: mad r0.x, r0.x, r0.y, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r0.yyyy)).x;
    // 19: mul r0.yz, v2.xxyx, cb0[4].zzwz
    r0.yz = ((v2.xxyx)*(source[4].zzwz)).yz;
    // 20: mad r1.x, cb0[4].y, cb0[4].x, r0.y
    r1.x = ((source[4].yyyy)*(source[4].xxxx)+(r0.yyyy)).x;
    // 21: mad r1.y, cb0[4].y, cb0[5].x, r0.z
    r1.y = ((source[4].yyyy)*(source[5].xxxx)+(r0.zzzz)).y;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 23: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 24: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 25: mul r0.yzw, r0.yyzw, cb0[5].yyyy
    r0.yzw = ((r0.yyzw)*(source[5].yyyy)).yzw;
    // 26: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 27: mul r0.yzw, r0.yyzw, cb0[5].zzzz
    r0.yzw = ((r0.yyzw)*(source[5].zzzz)).yzw;
    // 28: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 29: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 30: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 31: mad r0.w, v2.y, cb0[3].y, cb0[8].z
    r0.w = ((v2.yyyy)*(source[3].yyyy)+(source[8].zzzz)).w;
    // 32: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 34: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: mul r1.x, r1.x, cb0[8].w
    r1.x = ((r1.xxxx)*(source[8].wwww)).x;
    // 36: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 37: mad r1.y, v2.y, cb0[2].y, cb0[7].w
    r1.y = ((v2.yyyy)*(source[2].yyyy)+(source[7].wwww)).y;
    // 38: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 39: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 40: or r0.w, r0.w, r1.y
    r0.w = (asfloat(asuint(r0.wwww) | asuint(r1.yyyy))).w;
    // 41: mul r1.y, r1.z, cb0[8].x
    r1.y = ((r1.zzzz)*(source[8].xxxx)).y;
    // 42: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 43: min r1.xy, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = (min(r1.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 44: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 45: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 46: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 47: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 48: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 49: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_dt_01_1_ad: 0fd407002585204b86d3520d6610fd43; selected map c9da319b6e34e8a5c43c9f970996a7a3e624784c8d8ff9e046a636babad10802.
float4 ArtistNative3177(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
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
    // 1: add r0.xy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((v2.xyxx)+(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000), cb0[3].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,0.500000,0.500000,0.000000))+(source[3].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.xzyw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 5: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 6: mul r0.x, r0.x, l(30.000000)
    r0.x = ((r0.xxxx)*(float4(30.000000,30.000000,30.000000,30.000000))).x;
    // 7: mul r0.xyz, r0.xxxx, v3.xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 8: mad r0.xyz, r0.xyzx, l(0.050000, 0.050000, 0.050000, 0.000000), cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(float4(0.050000,0.050000,0.050000,0.000000))+(source[1].xyzx)).xyz;
    // 9: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 10: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 11: mad r1.xy, r1.xyxx, l(0.450000, 0.450000, 0.000000, 0.000000), l(0.275000, 0.275000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.450000,0.450000,0.000000,0.000000))+(float4(0.275000,0.275000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 13: add r1.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 14: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 15: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 16: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 17: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
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
// bfx_d_pa_flare_02_02_dt_ad: 31a56416afc48b48975d5f75c4d52157; selected map 4d77068b2fd78f85fcdac963e7906195871e4e3e5ba7a45f5fe9ea5bb462c047.
float4 ArtistNative3178(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 24: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 26: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 27: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 28: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 29: mul r1.x, r0.y, l(0.159155)
    r1.x = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))).x;
    // 30: mad r1.y, r0.y, l(0.159155), l(0.500000)
    r1.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 31: mov r2.xz, l(0.500000,0,0.500000,0)
    r2.xz = (float4(0.500000,asfloat(0u),0.500000,asfloat(0u))).xz;
    // 32: mul r2.yw, v4.xxxx, l(0.000000, 0.400000, 0.000000, -0.700000)
    r2.yw = ((v4.xxxx)*(float4(0.000000,0.400000,0.000000,-0.700000))).yw;
    // 33: add r1.xyzw, r1.xyxy, r2.xyzw
    r1.xyzw = ((r1.xyxy)+(r2.xyzw)).xyzw;
    // 34: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s1, l(-1.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t0.xyzw, s1, l(-1.000000)
    r1.xyz = (ArtistNativeSample0((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 36: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 37: add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // 38: add r0.z, r1.y, r1.x
    r0.z = ((r1.yyyy)+(r1.xxxx)).z;
    // 39: add r0.z, r1.z, r0.z
    r0.z = ((r1.zzzz)+(r0.zzzz)).z;
    // 40: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 41: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 42: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 43: mul r0.zw, r0.zzzz, cb0[2].xxxz
    r0.zw = ((r0.zzzz)*(source[2].xxxz)).zw;
    // 44: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 45: mul r0.yz, r0.yyzy, cb0[2].wwyw
    r0.yz = ((r0.yyzy)*(source[2].wwyw)).yz;
    // 46: movc r0.w, r0.x, l(0), r0.w
    r0.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 47: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 48: mad r0.x, r0.w, r0.y, r0.x
    r0.x = ((r0.wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 49: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 50: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 51: source device depth mapped to centimetre view depth; reconstruction at 53.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 53-56: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 57: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 58: add r0.z, -cb0[3].x, l(1.000000)
    r0.z = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 59: max r0.z, -r0.z, l(0.001000)
    r0.z = (max(-(r0.zzzz),float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 60: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 61: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 62: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 63: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 64: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_ap_23_2_tr: c9e2873553a59d42857bb5e72fae97c3; selected map 10507a3a0c024475158714574bdf0b83d9009d1538f833f7cc434126ffd9fcec.
float4 ArtistNative3179(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[5].w = (clamp(g_ArtistSourceMaterialParameters[5u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ArtistSourceMaterialParameters[5u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[11].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
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
    // 4: add r1.y, r0.y, l(0.500000)
    r1.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 5: add r0.y, -r1.y, l(1.000000)
    r0.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 6: mad r0.z, r0.y, cb0[5].w, cb0[6].x
    r0.z = ((r0.yyyy)*(source[5].wwww)+(source[6].xxxx)).z;
    // 7: mul_sat r0.y, r0.y, cb0[10].y
    r0.y = (saturate((r0.yyyy)*(source[10].yyyy))).y;
    // 8: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 9: div r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)/(r0.zzzz)).x;
    // 10: mad_sat r1.x, r0.x, l(0.500000), l(0.500000)
    r1.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 11: mul r0.xz, r1.xxyx, cb0[6].zzwz
    r0.xz = ((r1.xxyx)*(source[6].zzwz)).xz;
    // 12: mad r2.x, cb0[4].y, cb0[6].y, r0.x
    r2.x = ((source[4].yyyy)*(source[6].yyyy)+(r0.xxxx)).x;
    // 13: mad r2.y, cb0[4].y, cb0[7].x, r0.z
    r2.y = ((source[4].yyyy)*(source[7].xxxx)+(r0.zzzz)).y;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.xz, r2.xyxx, t0.xzyw, s1, l(0.000000)
    r0.xz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).xz;
    // 15: mad r0.xz, cb0[7].yyyy, r0.xxzx, r1.xxyx
    r0.xz = ((source[7].yyyy)*(r0.xxzx)+(r1.xxyx)).xz;
    // 16: mul r1.zw, r0.xxxz, cb0[4].zzzw
    r1.zw = ((r0.xxxz)*(source[4].zzzw)).zw;
    // 17: mul r0.xz, r0.xxzx, cb0[8].xxyx
    r0.xz = ((r0.xxzx)*(source[8].xxyx)).xz;
    // 18: mad r2.x, cb0[4].y, cb0[4].x, r1.z
    r2.x = ((source[4].yyyy)*(source[4].xxxx)+(r1.zzzz)).x;
    // 19: mad r2.y, cb0[4].y, cb0[7].z, r1.w
    r2.y = ((source[4].yyyy)*(source[7].zzzz)+(r1.wwww)).y;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 21: mad r2.x, cb0[4].y, cb0[7].w, r0.x
    r2.x = ((source[4].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 22: mad r2.y, cb0[4].y, cb0[8].z, r0.z
    r2.y = ((source[4].yyyy)*(source[8].zzzz)+(r0.zzzz)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 24: add r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)+(r0.wwww)).x;
    // 25: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 27: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.z, r0.z, cb0[8].w
    r0.z = ((r0.zzzz)*(source[8].wwww)).z;
    // 29: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 30: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 31: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 32: log r0.z, |r1.y|
    r0.z = (log2(abs(r1.yyyy))).z;
    // 33: lt r0.w, |r1.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 34: mul r0.z, r0.z, cb0[9].w
    r0.z = ((r0.zzzz)*(source[9].wwww)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, cb0[10].x
    r0.z = ((r0.zzzz)*(source[10].xxxx)).z;
    // 37: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 38: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r1.x, r0.w
    r0.w = ((r1.xxxx)*(r0.wwww)).w;
    // 40: lt r1.x, r0.w, l(0.000000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 41: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 42: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 43: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 44: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 45: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 46: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 47: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 48: mul_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)*(r0.xxxx))).x;
    // 49: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 50: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 51: source device depth mapped to centimetre view depth; reconstruction at 53.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 53-56: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 57: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 58: add r0.w, -cb0[11].y, l(1.000000)
    r0.w = ((-(source[11].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 60: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 61: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 62: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 63: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 64: mul r0.w, r0.w, cb0[10].z
    r0.w = ((r0.wwww)*(source[10].zzzz)).w;
    // 65: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 66: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 67: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 68: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 69: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 70: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 71: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_e_pa_gl_10_1_tr: d465be14468f91489ecb9184a42842a4; selected map bb4d665ef4d8c477c1f8a322a50949dcd37b2f563e5fddb80dc209d6bc3462ef.
float4 ArtistNative3180(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[3u];
    source[2] = g_ArtistSourceMaterialParameters[2u];
    source[3] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[0u].xxxx*float4(6.28310013, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend((float4(1.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[1u].zzzz),(float4(1.0, 0.0, 0.0, 0.0)/g_ArtistSourceMaterialParameters[1u].wwww),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[1u].yyyy,1u);
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[7].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: mad r0.xy, cb0[5].xyxx, r1.xyxx, -cb0[6].xyxx
    r0.xy = ((source[5].xyxx)*(r1.xyxx)+(-(source[6].xyxx))).xy;
    // 5: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 6: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 7: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 9: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 10: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 12: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 13: mul_sat r0.y, r0.y, cb0[8].y
    r0.y = (saturate((r0.yyyy)*(source[8].yyyy))).y;
    // 14: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 15: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 16: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 17: mad r0.xyz, cb0[2].xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((source[2].xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 18: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_flowblur_01_1_tr: d7c41b166d92df4bbdbdd2d52910bb43; selected map cf5307bb357c63ef65726a3cf7a9af8c452377b63b4908657a4e9b8ae66dff59.
float4 ArtistNative3181(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].z = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (clamp(g_ArtistSourceMaterialParameters[0u].xxxx,float4(-1.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
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
    // 15: mad r2.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r1.xyw, r2.xyxx, t1.xywz, s0 (project resolved HDR SceneColor snapshot adapter)
    r1.xyw = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r2.xyxx).xy).xywz).xyw;
    // 17: mul r3.y, v4.y, cb0[2].x
    r3.y = ((v4.yyyy)*(source[2].xxxx)).y;
    // 18: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 19: add r3.xy, r3.xyxx, v2.xyxx
    r3.xy = ((r3.xyxx)+(v2.xyxx)).xy;
    // 20: mad r3.xy, cb0[2].yyyy, r3.xyxx, cb0[2].zzzz
    r3.xy = ((source[2].yyyy)*(r3.xyxx)+(source[2].zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t2.yzwx, s1, l(0.000000)
    r2.w = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 22: mul r2.w, r2.w, v4.x
    r2.w = ((r2.wwww)*(v4.xxxx)).w;
    // 23: mul r2.w, r2.w, l(0.025000)
    r2.w = ((r2.wwww)*(float4(0.025000,0.025000,0.025000,0.025000))).w;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t1.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 25: mov r4.xyz, r3.xyzx
    r4.xyz = (r3.xyzx).xyz;
    // 26: mov r3.w, l(0)
    r3.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 27: mov r4.w, r2.w
    r4.w = (r2.wwww).w;
    // 28: loop
    [loop] while (true) {
    // 29: itof r5.x, r3.w
    r5.x = ((float4)(asint(r3.wwww))).x;
    // 30: ge r5.y, r5.x, l(8.000000)
    r5.y = (asfloat((uint4)((r5.xxxx)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).y;
    // 31: breakc_nz r5.y
    if ((asuint(r5.yyyy)).x != 0u) break;
    // 32: mad r2.z, r4.w, cb0[3].x, r2.y
    r2.z = ((r4.wwww)*(source[3].xxxx)+(r2.yyyy)).z;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r5.yzw, r2.xzxx, t1.wxyz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r5.yzw = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).wxyz).yzw;
    // 34: add r4.xyz, r4.xyzx, r5.yzwy
    r4.xyz = ((r4.xyzx)+(r5.yzwy)).xyz;
    // 35: mul r2.z, r2.w, r5.x
    r2.z = ((r2.wwww)*(r5.xxxx)).z;
    // 36: mad r4.w, r2.z, l(0.200000), r4.w
    r4.w = ((r2.zzzz)*(float4(0.200000,0.200000,0.200000,0.200000))+(r4.wwww)).w;
    // 37: iadd r3.w, r3.w, l(1)
    r3.w = (asfloat(asuint(r3.wwww) + uint4(1u,1u,1u,1u))).w;
    // 38: endloop
    }
    // 39: mul_sat r2.xyz, r4.xyzx, l(0.111111, 0.111111, 0.111111, 0.000000)
    r2.xyz = (saturate((r4.xyzx)*(float4(0.111111,0.111111,0.111111,0.000000)))).xyz;
    // 40: add r2.xyz, -r1.xywx, r2.xyzx
    r2.xyz = ((-(r1.xywx))+(r2.xyzx)).xyz;
    // 41: mad r1.xyw, r0.wwww, r2.xyxz, r1.xyxw
    r1.xyw = ((r0.wwww)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 42: mad r1.xyw, v3.xyxz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((v3.xyxz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 43: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_ri_03_1_tr: 6f9be43ce288b94fa12c01679f002719; selected map 6c9b8155bedc5fb7608609e98cce5a8148431cbff137acdc10860d167552e98d.
float4 ArtistNative3182(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0149999997, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.330000013, 0.0, 0.0, 0.0))),1u);
    source[7].x = ((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0)))).x;
    source[8].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].w = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[9].x = (max((float4(1.0, 0.0, 0.0, 0.0)-(float4(2.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].xxxx)),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(float4(2.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].xxxx)),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].xxxx)).x;
    source[9].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[1u].xxxx))).x;
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
    // 1: mad r0.xy, cb0[7].yyyy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((source[7].yyyy)*(v2.xyxx)+(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.yxzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).x;
    // 3: mad r0.yz, cb0[7].yyyy, v2.xxyx, cb0[3].xxyx
    r0.yz = ((source[7].yyyy)*(v2.xxyx)+(source[3].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.xyzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: mad r0.xy, cb0[8].xxxx, r0.xxxx, v2.xyxx
    r0.xy = ((source[8].xxxx)*(r0.xxxx)+(v2.xyxx)).xy;
    // 7: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 8: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 9: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 10: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 11: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 12: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 13: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 14: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 15: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 16: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 17: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 18: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 19: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 20: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 21: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 22: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 23: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 24: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 25: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 26: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 27: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 28: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 29: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 30: dp2 r0.y, r0.xxxx, cb0[8].zzzz
    r0.y = (dot((r0.xxxx).xy,(source[8].zzzz).xy).xxxx).y;
    // 31: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 32: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 33: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 34: mad r0.z, r0.z, l(0.159155), l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 35: mul r0.x, r0.z, cb0[8].y
    r0.x = ((r0.zzzz)*(source[8].yyyy)).x;
    // 36: add r0.zw, r0.xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)+(source[4].xxxy)).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 38: add r1.xy, r0.xyxx, cb0[5].xyxx
    r1.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 39: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t3.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 42: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 43: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 44: mul r0.x, r0.x, l(30.000000)
    r0.x = ((r0.xxxx)*(float4(30.000000,30.000000,30.000000,30.000000))).x;
    // 45: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 46: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 47: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 48: mad r0.z, -r0.y, cb0[9].w, l(1.000000)
    r0.z = ((-(r0.yyyy))*(source[9].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 49: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 50: mul_sat r0.y, r0.y, cb0[9].y
    r0.y = (saturate((r0.yyyy)*(source[9].yyyy))).y;
    // 51: mul_sat r0.z, r0.z, l(47.618980)
    r0.z = (saturate((r0.zzzz)*(float4(47.618980,47.618980,47.618980,47.618980)))).z;
    // 52: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 53: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 54: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 55: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 56: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 57: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_me_distoline_01_ad: a25e104919ece4498a5bb5ddb6b21901; selected map 27ba86f101f8c4bf7a3ce57ae6feabea5807fdcf93890c22ec70601eb0ff4ab2.
float4 ArtistNative3183(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[7u];
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = input.dynamicParameter;
    source[5] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.200000003, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[8].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].z = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))).x;
    source[11].x = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0)))).x;
    source[11].y = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
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
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: max r0.w, r0.z, l(0.000000)
    r0.w = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 5: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 6: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 7: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 8: mul r1.x, |r0.w|, r1.x
    r1.x = ((abs(r0.wwww))*(r1.xxxx)).x;
    // 9: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 10: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 11: mul r1.x, v4.x, cb0[11].w
    r1.x = ((v4.xxxx)*(source[11].wwww)).x;
    // 12: mad r1.x, cb0[8].z, cb0[11].z, r1.x
    r1.x = ((source[8].zzzz)*(source[11].zzzz)+(r1.xxxx)).x;
    // 13: mul r1.z, v4.y, cb0[12].x
    r1.z = ((v4.yyyy)*(source[12].xxxx)).z;
    // 14: mad r1.y, cb0[8].z, cb0[12].y, r1.z
    r1.y = ((source[8].zzzz)*(source[12].yyyy)+(r1.zzzz)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 16: mad r1.yz, r1.xxxx, cb0[12].wwww, v4.xxyx
    r1.yz = ((r1.xxxx)*(source[12].wwww)+(v4.xxyx)).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r1.yzyy, t6.yzwx, s5, l(0.000000)
    r1.w = (ArtistNativeSample5((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 18: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 19: dp3 r0.x, r1.wwww, r0.xyzx
    r0.x = (dot((r1.wwww).xyz,(r0.xyzx).xyz).xxxx).x;
    // 20: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 21: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 22: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: mul r0.x, |r0.x|, |r0.x|
    r0.x = ((abs(r0.xxxx))*(abs(r0.xxxx))).x;
    // 24: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 25: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 26: add r0.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 27: mul r0.w, r1.z, r1.y
    r0.w = ((r1.zzzz)*(r1.yyyy)).w;
    // 28: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 29: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 30: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 31: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 32: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 33: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 34: log r0.z, |r0.w|
    r0.z = (log2(abs(r0.wwww))).z;
    // 35: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 36: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 37: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 38: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 39: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 40: mul r0.y, r0.y, l(50.000000)
    r0.y = ((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000))).y;
    // 41: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 42: add r0.zw, v4.xxxy, cb0[5].xxxy
    r0.zw = ((v4.xxxy)+(source[5].xxxy)).zw;
    // 43: mad r0.zw, r1.xxxx, cb0[12].wwww, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[12].wwww)+(r0.zzzw)).zw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r0.zwzz, t5.wxyz, s4, l(0.000000)
    r1.yzw = (ArtistNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 45: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 0.500000, 0.300000), cb0[6].xxxy
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,0.500000,0.300000))+(source[6].xxxy)).zw;
    // 46: mad r0.zw, r1.xxxx, cb0[12].wwww, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[12].wwww)+(r0.zzzw)).zw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t5.xyzw, s4, l(0.000000)
    r2.xyz = (ArtistNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 48: mul r1.xyz, r1.yzwy, r2.xyzx
    r1.xyz = ((r1.yzwy)*(r2.xyzx)).xyz;
    // 49: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 50: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 51: mul r1.xyz, r1.xyzx, l(1.200000, 1.200000, 1.200000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(1.200000,1.200000,1.200000,0.000000))).xyz;
    // 52: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 53: mul r0.z, v4.x, cb0[8].w
    r0.z = ((v4.xxxx)*(source[8].wwww)).z;
    // 54: mad r2.x, cb0[8].z, cb0[8].y, r0.z
    r2.x = ((source[8].zzzz)*(source[8].yyyy)+(r0.zzzz)).x;
    // 55: mul r0.zw, v4.yyyx, cb0[9].xxxw
    r0.zw = ((v4.yyyx)*(source[9].xxxw)).zw;
    // 56: mad r2.yz, cb0[8].zzzz, cb0[9].yyzy, r0.zzwz
    r2.yz = ((source[8].zzzz)*(source[9].yyzy)+(r0.zzwz)).yz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 58: mul r0.z, v4.y, cb0[10].x
    r0.z = ((v4.yyyy)*(source[10].xxxx)).z;
    // 59: mad r2.w, cb0[8].z, cb0[10].y, r0.z
    r2.w = ((source[8].zzzz)*(source[10].yyyy)+(r0.zzzz)).w;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.zwzz, t4.xyzw, s2, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 61: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 62: mad r1.xyz, r1.xyzx, l(10.000000, 10.000000, 10.000000, 0.000000), r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(10.000000,10.000000,10.000000,0.000000))+(r2.xyzx)).xyz;
    // 63: mul r0.xzw, r0.xxxx, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r1.xxyz)).xzw;
    // 64: mul r1.w, cb0[4].y, cb0[8].x
    r1.w = ((source[4].yyyy)*(source[8].xxxx)).w;
    // 65: mul r2.x, v4.x, l(6.283185)
    r2.x = ((v4.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 66: sincos r2.x, r3.x, r2.x
    r2.x = (sin(r2.xxxx)).x; r3.x = (cos(r2.xxxx)).x;
    // 67: mov r2.y, r3.x
    r2.y = (r3.xxxx).y;
    // 68: mad r2.xy, -r1.wwww, r2.xyxx, cb0[3].xyxx
    r2.xy = ((-(r1.wwww))*(r2.xyxx)+(source[3].xyxx)).xy;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s0, l(0.000000)
    r2.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 70: mul r1.w, v4.y, v4.y
    r1.w = ((v4.yyyy)*(v4.yyyy)).w;
    // 71: mad r2.w, -v4.y, v4.y, l(1.000000)
    r2.w = ((-(v4.yyyy))*(v4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 73: mul r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)*(r1.wwww)).xyz;
    // 74: mul r2.xyz, r2.xyzx, l(0.025000, 0.025000, 0.025000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.025000,0.025000,0.025000,0.000000))).xyz;
    // 75: mad r0.xzw, r2.xxyz, r1.xxyz, r0.xxzw
    r0.xzw = ((r2.xxyz)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 76: mad r0.xzw, r0.xxzw, cb0[1].xxyz, cb0[2].xxyz
    r0.xzw = ((r0.xxzw)*(source[1].xxyz)+(source[2].xxyz)).xzw;
    // 77: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 78: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 79: dp2 r1.x, cb0[7].xyxx, r1.xyxx
    r1.x = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r1.y, v4.xyxx, t0.yxzw, s6, l(0.000000)
    r1.y = (ArtistNativeSample6((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 81: mad r1.x, r1.y, l(0.100000), r1.x
    r1.x = ((r1.yyyy)*(float4(0.100000,0.100000,0.100000,0.100000))+(r1.xxxx)).x;
    // 82: add r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 83: mul r1.x, r1.x, cb0[4].x
    r1.x = ((r1.xxxx)*(source[4].xxxx)).x;
    // 84: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 85: mul r0.y, r0.y, cb0[1].w
    r0.y = ((r0.yyyy)*(source[1].wwww)).y;
    // 86: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 87: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 88: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_l_me_master_01_12_ad: 623238a87e487645b35ddb3253d16e82; selected map 8bc912aecd5923405f50077c461586195f1e5735e01c3c7e8d80ac9025a8486c.
float4 ArtistNative3184(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = g_ArtistSourceMaterialParameters[7u];
    source[4] = input.dynamicParameter;
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].yyyy,g_ArtistSourceMaterialParameters[4u].zzzz,1u);
    source[6].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[12].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
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
    // 6: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul_sat r0.y, r0.y, cb0[13].z
    r0.y = (saturate((r0.yyyy)*(source[13].zzzz))).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: add r0.z, cb0[4].y, l(-1.000000)
    r0.z = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 12: mad r0.w, cb0[6].y, cb0[11].w, cb0[12].x
    r0.w = ((source[6].yyyy)*(source[11].wwww)+(source[12].xxxx)).w;
    // 13: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 14: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 15: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 16: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 17: mul r1.xy, v4.xyxx, cb0[7].zwzz
    r1.xy = ((v4.xyxx)*(source[7].zwzz)).xy;
    // 18: mul r0.w, cb0[6].x, cb0[6].y
    r0.w = ((source[6].xxxx)*(source[6].yyyy)).w;
    // 19: mad r2.x, r0.w, cb0[7].y, r1.x
    r2.x = ((r0.wwww)*(source[7].yyyy)+(r1.xxxx)).x;
    // 20: mad r2.y, r0.w, cb0[8].x, r1.y
    r2.y = ((r0.wwww)*(source[8].xxxx)+(r1.yyyy)).y;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 22: mad r1.xy, cb0[8].zzzz, r1.xyxx, v4.xyxx
    r1.xy = ((source[8].zzzz)*(r1.xyxx)+(v4.xyxx)).xy;
    // 23: add r1.zw, r1.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((r1.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 24: dp2 r2.x, r3.yxyy, r1.zwzz
    r2.x = (dot((r3.yxyy).xy,(r1.zwzz).xy).xxxx).x;
    // 25: dp2 r1.z, r3.zyzz, r1.zwzz
    r1.z = (dot((r3.zyzz).xy,(r1.zwzz).xy).xxxx).z;
    // 26: mul r3.z, r1.z, cb0[5].y
    r3.z = ((r1.zzzz)*(source[5].yyyy)).z;
    // 27: mad r3.x, r2.x, cb0[5].x, r0.z
    r3.x = ((r2.xxxx)*(source[5].xxxx)+(r0.zzzz)).x;
    // 28: add r1.zw, r3.xxxz, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r3.xxxz)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r0.z = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 30: mul r1.z, r1.x, cb0[10].w
    r1.z = ((r1.xxxx)*(source[10].wwww)).z;
    // 31: mad r2.x, r0.w, cb0[10].z, r1.z
    r2.x = ((r0.wwww)*(source[10].zzzz)+(r1.zzzz)).x;
    // 32: mul r1.z, r1.y, cb0[11].x
    r1.z = ((r1.yyyy)*(source[11].xxxx)).z;
    // 33: mad r2.y, r0.w, cb0[11].y, r1.z
    r2.y = ((r0.wwww)*(source[11].yyyy)+(r1.zzzz)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.xyxx, t1.yzxw, s3, l(0.000000)
    r1.z = (ArtistNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: mad r0.y, r1.z, r0.z, -r0.y
    r0.y = ((r1.zzzz)*(r0.zzzz)+(-(r0.yyyy))).y;
    // 36: mul_sat r0.y, r0.y, cb0[12].w
    r0.y = (saturate((r0.yyyy)*(source[12].wwww))).y;
    // 37: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 38: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 39: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 40: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 41: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 42: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 43: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 44: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 45: mul r0.y, r1.x, cb0[6].w
    r0.y = ((r1.xxxx)*(source[6].wwww)).y;
    // 46: mad r2.x, r0.w, cb0[6].z, r0.y
    r2.x = ((r0.wwww)*(source[6].zzzz)+(r0.yyyy)).x;
    // 47: mul r0.y, r0.w, cb0[8].w
    r0.y = ((r0.wwww)*(source[8].wwww)).y;
    // 48: mad r2.y, cb0[7].x, r1.y, r0.y
    r2.y = ((source[7].xxxx)*(r1.yyyy)+(r0.yyyy)).y;
    // 49: mul r0.yz, r1.xxyx, cb0[9].yyzy
    r0.yz = ((r1.xxyx)*(source[9].yyzy)).yz;
    // 50: mad r0.yz, r0.wwww, cb0[9].xxwx, r0.yyzy
    r0.yz = ((r0.wwww)*(source[9].xxwx)+(r0.yyzy)).yz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s2, l(0.000000)
    r0.yzw = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 54: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 55: mad r0.yzw, -r1.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r1.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 56: mad r0.yzw, cb0[10].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[10].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 57: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 58: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 59: mul r0.yzw, r0.yyzw, cb0[10].yyyy
    r0.yzw = ((r0.yyzw)*(source[10].yyyy)).yzw;
    // 60: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 61: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 62: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 63: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 64: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 65: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 66: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_087_tr: d045ce88323e0443a8ad8731ff6d91da; selected map ace1bd123c3535baf7804fbc019e98557be24dc93d47149a29ca204095fb9583.
float4 ArtistNative3185(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[1u].yyyy)).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
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
    // 6: mul r0.y, r0.y, cb0[11].x
    r0.y = ((r0.yyyy)*(source[11].xxxx)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul_sat r0.y, r0.y, cb0[11].y
    r0.y = (saturate((r0.yyyy)*(source[11].yyyy))).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.zw, v4.xxxy, cb0[6].zzzw
    r0.zw = ((v4.xxxy)*(source[6].zzzw)).zw;
    // 12: mul r1.x, cb0[5].x, cb0[5].y
    r1.x = ((source[5].xxxx)*(source[5].yyyy)).x;
    // 13: mad r2.x, r1.x, cb0[6].y, r0.z
    r2.x = ((r1.xxxx)*(source[6].yyyy)+(r0.zzzz)).x;
    // 14: mad r2.y, r1.x, cb0[7].x, r0.w
    r2.y = ((r1.xxxx)*(source[7].xxxx)+(r0.wwww)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 16: mad r0.zw, cb0[7].zzzz, r0.zzzw, v4.xxxy
    r0.zw = ((source[7].zzzz)*(r0.zzzw)+(v4.xxxy)).zw;
    // 17: mul r1.y, r0.z, cb0[8].w
    r1.y = ((r0.zzzz)*(source[8].wwww)).y;
    // 18: mad r2.x, r1.x, cb0[8].z, r1.y
    r2.x = ((r1.xxxx)*(source[8].zzzz)+(r1.yyyy)).x;
    // 19: mul r1.yz, r0.wwzw, cb0[9].xxwx
    r1.yz = ((r0.wwzw)*(source[9].xxwx)).yz;
    // 20: mad r2.yz, r1.xxxx, cb0[9].yyzy, r1.yyzy
    r2.yz = ((r1.xxxx)*(source[9].yyzy)+(r1.yyzy)).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s2, l(0.000000)
    r1.y = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 22: mul r1.z, r0.w, cb0[10].x
    r1.z = ((r0.wwww)*(source[10].xxxx)).z;
    // 23: mad r2.w, r1.x, cb0[10].y, r1.z
    r2.w = ((r1.xxxx)*(source[10].yyyy)+(r1.zzzz)).w;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t2.yzxw, s3, l(0.000000)
    r1.z = (ArtistNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 25: mad r0.y, r1.y, r1.z, -r0.y
    r0.y = ((r1.yyyy)*(r1.zzzz)+(-(r0.yyyy))).y;
    // 26: mul_sat r0.y, r0.y, cb0[10].z
    r0.y = (saturate((r0.yyyy)*(source[10].zzzz))).y;
    // 27: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 28: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 29: mul r1.y, r1.y, cb0[10].w
    r1.y = ((r1.yyyy)*(source[10].wwww)).y;
    // 30: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 31: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 32: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 33: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 34: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 35: mul r0.x, r0.z, cb0[5].w
    r0.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 36: mad r0.x, r1.x, cb0[5].z, r0.x
    r0.x = ((r1.xxxx)*(source[5].zzzz)+(r0.xxxx)).x;
    // 37: mul r0.z, r1.x, cb0[7].w
    r0.z = ((r1.xxxx)*(source[7].wwww)).z;
    // 38: mad r0.y, cb0[6].x, r0.w, r0.z
    r0.y = ((source[6].xxxx)*(r0.wwww)+(r0.zzzz)).y;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r0.xyz, cb0[8].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[8].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 43: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 45: mul r0.xyz, r0.xyzx, cb0[8].yyyy
    r0.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 46: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 47: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 48: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 49: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 50: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_gl_01_1_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ArtistNative3186(ARTIST_NATIVE_INPUT input)
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
// fx_c_pa_glow_02_ad: 00ef1ab5901165418b0b5fe1ff26ade3; selected map 2823bd21c644c6a0769c28ffd9b9eaf4d13a8f54e5396bbc17126de64279c27f.
float4 ArtistNative3187(ARTIST_NATIVE_INPUT input)
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
    float4 r0=0.f;
    // 1: add r0.x, v4.x, cb0[2].x
    r0.x = ((v4.xxxx)+(source[2].xxxx)).x;
    // 2: mul r0.x, r0.x, v4.y
    r0.x = ((r0.xxxx)*(v4.yyyy)).x;
    // 3: mul r0.xy, r0.xxxx, l(20.943951, 12.566371, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(20.943951,12.566371,0.000000,0.000000))).xy;
    // 4: sincos r0.xy, null, r0.xyxx
    r0.xy = (sin(r0.xyxx)).xy;
    // 5: add r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 7: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 8: mad r0.x, r0.x, l(0.400000), l(0.200000)
    r0.x = ((r0.xxxx)*(float4(0.400000,0.400000,0.400000,0.400000))+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v2.xyxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 10: mul r0.yzw, r0.yyzw, v3.xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)).yzw;
    // 11: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[1].xyzx)).xyz;
    // 12: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 13: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 14: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 15: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_flare_02_ad: 68e66a6e0ae7dd4db3cb9055fc265c60; selected map 2095759cc2bda5d40098ba0fbb1788831a84116ff48e932b2c95a0ddd975cbb0.
float4 ArtistNative3188(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    // 24: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 26: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 27: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 28: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 29: mul r1.x, r0.y, l(0.159155)
    r1.x = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))).x;
    // 30: mad r1.y, r0.y, l(0.159155), l(0.500000)
    r1.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 31: mov r2.xz, l(0.500000,0,0.500000,0)
    r2.xz = (float4(0.500000,asfloat(0u),0.500000,asfloat(0u))).xz;
    // 32: mul r2.yw, v4.xxxx, l(0.000000, 0.400000, 0.000000, -0.700000)
    r2.yw = ((v4.xxxx)*(float4(0.000000,0.400000,0.000000,-0.700000))).yw;
    // 33: add r1.xyzw, r1.xyxy, r2.xyzw
    r1.xyzw = ((r1.xyxy)+(r2.xyzw)).xyzw;
    // 34: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t0.wxyz, s0, l(-1.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 35: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t0.xyzw, s0, l(-1.000000)
    r1.xyz = (ArtistNativeSample0((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 36: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 37: add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // 38: add r0.z, r1.y, r1.x
    r0.z = ((r1.yyyy)+(r1.xxxx)).z;
    // 39: add r0.z, r1.z, r0.z
    r0.z = ((r1.zzzz)+(r0.zzzz)).z;
    // 40: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 41: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 42: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 43: mul r0.zw, r0.zzzz, cb0[2].xxxz
    r0.zw = ((r0.zzzz)*(source[2].xxxz)).zw;
    // 44: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 45: mul r0.yz, r0.yyzy, cb0[2].wwyw
    r0.yz = ((r0.yyzy)*(source[2].wwyw)).yz;
    // 46: movc r0.w, r0.x, l(0), r0.w
    r0.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 47: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 48: mad r0.x, r0.w, r0.y, r0.x
    r0.x = ((r0.wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 49: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
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
// fx_k_pa_gl_01_01_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ArtistNative3189(ARTIST_NATIVE_INPUT input)
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
// fx_f_pa_gl_02_1_ad: 096d7ee0efa1eb4bb2e91b406fe61913; selected map d3ae7817e320de17bcfcd6b6dde23265c1c47d93ae32f74e18619b11001ac907.
float4 ArtistNative3190(ARTIST_NATIVE_INPUT input)
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 6: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 7: mul r0.w, r0.w, cb0[2].w
    r0.w = ((r0.wwww)*(source[2].wwww)).w;
    // 8: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 9: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 10: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 11: movc r0.w, r1.x, l(0), |r0.w|
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.wwww))).w;
    // 12: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 13: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 17: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 18: mul r1.xyz, r0.xyzx, cb0[2].yyyy
    r1.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 19: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: mad r0.xyz, -cb0[2].yyyy, r0.xyzx, r1.wwww
    r0.xyz = ((-(source[2].yyyy))*(r0.xyzx)+(r1.wwww)).xyz;
    // 21: mad r0.xyz, cb0[2].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
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
// fx_e_pa_ap_03_1_tr: 991ecc5522859c49bb034318a827af71; selected map fd85ade76eaa888dbc097dfdfc3900cacd64ae68812cae449a674d712986917f.
float4 ArtistNative3191(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_u_pa_ringaura_01_ad: 6941275e665e9847a31e0c40601c40b4; selected map e96524d42c8b254a9b40a6fa5d1a0ed7c46a4976d8dbd3606737dfefa70e3a17.
float4 ArtistNative3192(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_radialcolor_13_ad: bf86704582fae940b1eacb3a9fd9c278; selected map fcedd7387a61bac4dfb16a4869939e66f57f8dccab8db269fafec116df056329.
float4 ArtistNative3193(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[4].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].x = ((g_ArtistSourceMaterialParameters[3u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].w = ((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[7].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].w = ((g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[8].x = (((g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx)*g_ArtistSourceMaterialParameters[4u].xxxx)).x;
    source[8].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
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
    // 27: mul r0.y, r0.y, cb0[4].z
    r0.y = ((r0.yyyy)*(source[4].zzzz)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[4].wwww
    r0.z = (dot((r0.zzzz).xy,(source[4].wwww).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r1.x, r0.y, cb0[5].x
    r1.x = ((r0.yyyy)+(source[5].xxxx)).x;
    // 32: mul r2.x, r1.x, cb0[7].x
    r2.x = ((r1.xxxx)*(source[7].xxxx)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 36: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 37: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 38: mul r0.y, r0.y, cb0[5].z
    r0.y = ((r0.yyyy)*(source[5].zzzz)).y;
    // 39: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 40: add r0.y, v4.x, cb0[5].w
    r0.y = ((v4.xxxx)+(source[5].wwww)).y;
    // 41: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 42: mad r2.y, r1.y, cb0[7].y, cb0[8].x
    r2.y = ((r1.yyyy)*(source[7].yyyy)+(source[8].xxxx)).y;
    // 43: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xy;
    // 44: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 45: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 46: mul r0.xy, r0.xyxx, cb0[8].yyyy
    r0.xy = ((r0.xyxx)*(source[8].yyyy)).xy;
    // 47: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 48: mul r2.x, r1.x, cb0[6].x
    r2.x = ((r1.xxxx)*(source[6].xxxx)).x;
    // 49: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[3].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[3].xxxy)).zw;
    // 50: mad r2.y, r1.y, cb0[6].y, cb0[6].w
    r2.y = ((r1.yyyy)*(source[6].yyyy)+(source[6].wwww)).y;
    // 51: mad r1.xy, cb0[8].zzzz, r0.xyxx, r2.xyxx
    r1.xy = ((source[8].zzzz)*(r0.xyxx)+(r2.xyxx)).xy;
    // 52: mad r0.xy, cb0[8].zzzz, r0.xyxx, r0.zwzz
    r0.xy = ((source[8].zzzz)*(r0.xyxx)+(r0.zwzz)).xy;
    // 53: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 54: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t2.wxyz, s0, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
    // 55: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 56: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 57: mul r0.yzw, r0.yyzw, cb0[8].wwww
    r0.yzw = ((r0.yyzw)*(source[8].wwww)).yzw;
    // 58: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 59: mul r1.xyz, r0.yzwy, cb0[9].xxxx
    r1.xyz = ((r0.yzwy)*(source[9].xxxx)).xyz;
    // 60: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: mad r0.yzw, -cb0[9].xxxx, r0.yyzw, r1.wwww
    r0.yzw = ((-(source[9].xxxx))*(r0.yyzw)+(r1.wwww)).yzw;
    // 62: mad r0.yzw, cb0[9].yyyy, r0.yyzw, r1.xxyz
    r0.yzw = ((source[9].yyyy)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 63: mul r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[2].xxyz)).yzw;
    // 64: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 65: mul r0.xyz, r0.xyzx, cb0[10].xxxx
    r0.xyz = ((r0.xyzx)*(source[10].xxxx)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[10].yyyy
    r0.xyz = ((r0.xyzx)*(source[10].yyyy)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 71: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 73: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 74: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 75: mul r0.w, r0.w, cb0[10].z
    r0.w = ((r0.wwww)*(source[10].zzzz)).w;
    // 76: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 77: mul r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)*(source[10].wwww)).w;
    // 78: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 79: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 80: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 81: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_me_magiccircle_03_1_tr: 3cee380e77fbc3428da1115259121db2; selected map 5db18f310ea7359258466f255aca949220160897a87933b7879ea868bd366a98.
float4 ArtistNative3194(ARTIST_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].xxxx,g_ArtistSourceMaterialParameters[6u].yyyy,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[6] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[3u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[3u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[3u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[3u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ArtistSourceMaterialParameters[7u];
    source[11].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[3u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[11].y = (cos(((g_ArtistSourceMaterialParameters[3u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[15].w = ((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[16].x = (sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[16].z = (cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[17].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[18].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[18].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[12].xyxx
    r0.xy = ((v4.xyxx)*(source[12].xyxx)).xy;
    // 2: mad r1.x, cb0[11].w, cb0[11].z, r0.x
    r1.x = ((source[11].wwww)*(source[11].zzzz)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[11].w, cb0[12].z, r0.y
    r1.y = ((source[11].wwww)*(source[12].zzzz)+(r0.yyyy)).y;
    // 4: add r0.xy, r1.xyxx, cb0[4].xyxx
    r0.xy = ((r1.xyxx)+(source[4].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: add r0.y, cb0[3].z, cb0[13].y
    r0.y = ((source[3].zzzz)+(source[13].yyyy)).y;
    // 7: mad r0.xy, r0.xxxx, r0.yyyy, cb0[5].xyxx
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(source[5].xyxx)).xy;
    // 8: add r0.zw, cb0[3].xxxy, l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((source[3].xxxy)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 9: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 10: mad r1.xy, cb0[3].xxxx, v4.xyxx, -r0.zzzz
    r1.xy = ((source[3].xxxx)*(v4.xyxx)+(-(r0.zzzz))).xy;
    // 11: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 12: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 13: dp2 r1.x, cb0[6].xyxx, r0.xyxx
    r1.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 14: dp2 r1.y, cb0[7].xyxx, r0.xyxx
    r1.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 15: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(-1.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 17: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 18: mul r0.y, r0.y, cb0[14].x
    r0.y = ((r0.yyyy)*(source[14].xxxx)).y;
    // 19: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 20: mul r0.y, r0.y, cb0[14].y
    r0.y = ((r0.yyyy)*(source[14].yyyy)).y;
    // 21: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 22: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 23: mad r0.y, r0.x, cb0[14].z, r0.y
    r0.y = ((r0.xxxx)*(source[14].zzzz)+(r0.yyyy)).y;
    // 24: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 25: dp2 r2.x, cb0[8].xyxx, r1.xyxx
    r2.x = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 26: dp2 r2.y, cb0[9].xyxx, r1.xyxx
    r2.y = (dot((source[9].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 27: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 28: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 29: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 31: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 32: max r0.z, r0.z, cb0[18].z
    r0.z = (max(r0.zzzz,source[18].zzzz)).z;
    // 33: min r0.z, r0.z, cb0[18].y
    r0.z = (min(r0.zzzz,source[18].yyyy)).z;
    // 34: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 35: mul r1.xy, r1.xyxx, cb0[15].xyxx
    r1.xy = ((r1.xyxx)*(source[15].xyxx)).xy;
    // 36: mad r2.x, cb0[11].w, cb0[14].w, r1.x
    r2.x = ((source[11].wwww)*(source[14].wwww)+(r1.xxxx)).x;
    // 37: mad r2.y, cb0[11].w, cb0[16].w, r1.y
    r2.y = ((source[11].wwww)*(source[16].wwww)+(r1.yyyy)).y;
    // 38: add r1.xy, r2.xyxx, cb0[17].xyxx
    r1.xy = ((r2.xyxx)+(source[17].xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 40: add r1.x, r1.x, l(0.100000)
    r1.x = ((r1.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 41: add_sat r0.w, -r0.w, r1.x
    r0.w = (saturate((-(r0.wwww))+(r1.xxxx))).w;
    // 42: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 43: mul r1.x, r1.x, cb0[17].w
    r1.x = ((r1.xxxx)*(source[17].wwww)).x;
    // 44: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 45: mul_sat r1.x, r1.x, cb0[17].z
    r1.x = (saturate((r1.xxxx)*(source[17].zzzz))).x;
    // 46: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 47: mul r0.w, r0.w, cb0[17].z
    r0.w = ((r0.wwww)*(source[17].zzzz)).w;
    // 48: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 49: mov_sat r1.y, r0.w
    r1.y = (saturate(r0.wwww)).y;
    // 50: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 51: mul r0.x, r0.x, cb0[18].w
    r0.x = ((r0.xxxx)*(source[18].wwww)).x;
    // 52: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 53: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 54: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 55: add r0.x, r1.x, r1.y
    r0.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 56: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 57: mad r0.xyz, r0.xxxx, cb0[10].xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(source[10].xyzx)+(r0.yyyy)).xyz;
    // 58: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 59: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_wave_01_02_tr: d3bc2b015d847b43b526d21ea5cf3729; selected map 1bf1bec4ca1f1189ec7f0a8a520a6cd7bbcf10be68aa5af092867e4167883642.
float4 ArtistNative3195(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
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
    // 29: mul r0.zw, r0.xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)*(source[4].xxxy)).zw;
    // 30: mad r1.x, cb0[2].y, cb0[3].w, r0.z
    r1.x = ((source[2].yyyy)*(source[3].wwww)+(r0.zzzz)).x;
    // 31: mad r1.y, cb0[2].y, cb0[4].z, r0.w
    r1.y = ((source[2].yyyy)*(source[4].zzzz)+(r0.wwww)).y;
    // 32: mul r0.zw, r0.xxxy, cb0[5].xxxy
    r0.zw = ((r0.xxxy)*(source[5].xxxy)).zw;
    // 33: mad r2.x, cb0[2].y, cb0[4].w, r0.z
    r2.x = ((source[2].yyyy)*(source[4].wwww)+(r0.zzzz)).x;
    // 34: mad r2.y, cb0[2].y, cb0[5].z, r0.w
    r2.y = ((source[2].yyyy)*(source[5].zzzz)+(r0.wwww)).y;
    // 35: mov r3.xz, l(0,0,0,0)
    r3.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 36: mul r3.yw, v4.zzzy, l(0.000000, 0.500000, 0.000000, 0.500000)
    r3.yw = ((v4.zzzy)*(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 37: add r0.zw, r2.xxxy, r3.xxxy
    r0.zw = ((r2.xxxy)+(r3.xxxy)).zw;
    // 38: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(1.000000)
    r0.zw = (ArtistNativeSample2((r0.zwzz).xy, (float4(1.000000,1.000000,1.000000,1.000000)).x, true).zwxy).zw;
    // 39: mad r0.zw, cb0[5].wwww, r0.zzzw, r1.xxxy
    r0.zw = ((source[5].wwww)*(r0.zzzw)+(r1.xxxy)).zw;
    // 40: add r0.zw, r0.zzzw, r3.zzzw
    r0.zw = ((r0.zzzw)+(r3.zzzw)).zw;
    // 41: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s1, l(0.100000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.100000,0.100000,0.100000,0.100000)).x, true).yzxw).z;
    // 42: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 43: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 44: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 45: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 46: mul r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)*(source[6].yyyy)).w;
    // 47: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 48: mul r1.xy, r0.xyxx, cb0[2].zwzz
    r1.xy = ((r0.xyxx)*(source[2].zwzz)).xy;
    // 49: mul r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)*(source[7].xyxx)).xy;
    // 50: mad r3.x, cb0[2].y, cb0[2].x, r1.x
    r3.x = ((source[2].yyyy)*(source[2].xxxx)+(r1.xxxx)).x;
    // 51: mad r3.y, cb0[2].y, cb0[3].x, r1.y
    r3.y = ((source[2].yyyy)*(source[3].xxxx)+(r1.yyyy)).y;
    // 52: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 53: mul r1.yw, v4.zzzw, l(0.000000, 0.200000, 0.000000, -0.200000)
    r1.yw = ((v4.zzzw)*(float4(0.000000,0.200000,0.000000,-0.200000))).yw;
    // 54: add r1.zw, r3.xxxy, r1.zzzw
    r1.zw = ((r3.xxxy)+(r1.zzzw)).zw;
    // 55: add r1.xy, r2.xyxx, r1.xyxx
    r1.xy = ((r2.xyxx)+(r1.xyxx)).xy;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s2, l(1.000000)
    r1.xy = (ArtistNativeSample2((r1.xyxx).xy, (float4(1.000000,1.000000,1.000000,1.000000)).x, true).xyzw).xy;
    // 57: mad r1.xy, cb0[6].zzzz, r1.xyxx, v2.xyxx
    r1.xy = ((source[6].zzzz)*(r1.xyxx)+(v2.xyxx)).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 59: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t2.xyzw, s0, l(0.100000)
    r1.x = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.100000,0.100000,0.100000,0.100000)).x, true).xyzw).x;
    // 60: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 61: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 62: mul r1.y, r1.y, cb0[3].y
    r1.y = ((r1.yyyy)*(source[3].yyyy)).y;
    // 63: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 64: mul r1.y, r1.y, cb0[3].z
    r1.y = ((r1.yyyy)*(source[3].zzzz)).y;
    // 65: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 66: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 67: mad r1.xyz, r0.zzzz, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r0.zzzz)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 68: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 69: mad r1.x, cb0[2].y, cb0[6].w, r0.x
    r1.x = ((source[2].yyyy)*(source[6].wwww)+(r0.xxxx)).x;
    // 70: mad r1.y, cb0[2].y, cb0[7].z, r0.y
    r1.y = ((source[2].yyyy)*(source[7].zzzz)+(r0.yyyy)).y;
    // 71: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s0, l(0.100000)
    r0.x = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.100000,0.100000,0.100000,0.100000)).x, true).xyzw).x;
    // 72: add r0.y, v4.x, l(-1.000000)
    r0.y = ((v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 73: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 74: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 75: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 76: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 77: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_ringmaster_01_07_ad: 573e34964fe780418e7305173e0b95f1; selected map 36518053a95cee002da454ef0c6dd169920b557bb17d0c92b6b843b7a8c68159.
float4 ArtistNative3196(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[6].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[10].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[10].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[11].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].w = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].wwww)).x;
    source[12].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[13].z, l(1.000000)
    r0.y = ((-(source[13].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 14: max r0.w, |r0.z|, |r0.y|
    r0.w = (max(abs(r0.zzzz),abs(r0.yyyy))).w;
    // 15: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 16: min r1.x, |r0.z|, |r0.y|
    r1.x = (min(abs(r0.zzzz),abs(r0.yyyy))).x;
    // 17: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 18: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 19: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 20: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 21: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 22: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 23: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 24: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 25: lt r1.z, |r0.z|, |r0.y|
    r1.z = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.yyyy))) * 0xffffffffu)).z;
    // 26: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 27: mad r0.w, r0.w, r1.x, r1.y
    r0.w = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).w;
    // 28: lt r1.x, r0.z, -r0.z
    r1.x = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).x;
    // 29: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 30: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 31: min r1.x, r0.z, r0.y
    r1.x = (min(r0.zzzz,r0.yyyy)).x;
    // 32: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 33: max r1.y, r0.z, r0.y
    r1.y = (max(r0.zzzz,r0.yyyy)).y;
    // 34: ge r1.y, r1.y, -r1.y
    r1.y = (asfloat((uint4)((r1.yyyy)>=(-(r1.yyyy))) * 0xffffffffu)).y;
    // 35: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 36: movc r0.w, r1.x, -r0.w, r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 37: mad r1.x, r0.w, l(0.159155), l(0.500000)
    r1.x = ((r0.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 38: mul r0.w, v2.x, cb0[7].w
    r0.w = ((v2.xxxx)*(source[7].wwww)).w;
    // 39: mul r1.z, cb0[6].x, cb0[6].y
    r1.z = ((source[6].xxxx)*(source[6].yyyy)).z;
    // 40: mad r2.x, r1.z, cb0[7].z, r0.w
    r2.x = ((r1.zzzz)*(source[7].zzzz)+(r0.wwww)).x;
    // 41: mul r2.zw, r1.zzzz, cb0[8].yyyw
    r2.zw = ((r1.zzzz)*(source[8].yyyw)).zw;
    // 42: mad r2.y, cb0[8].x, v2.y, r2.z
    r2.y = ((source[8].xxxx)*(v2.yyyy)+(r2.zzzz)).y;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xy = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 44: mul r0.w, v4.y, cb0[8].z
    r0.w = ((v4.yyyy)*(source[8].zzzz)).w;
    // 45: mul r1.w, v4.z, cb0[7].y
    r1.w = ((v4.zzzz)*(source[7].yyyy)).w;
    // 46: dp2 r2.z, r0.yzyy, r0.yzyy
    r2.z = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).z;
    // 47: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 48: add r3.x, r2.z, r2.z
    r3.x = ((r2.zzzz)+(r2.zzzz)).x;
    // 49: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 50: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 51: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 52: lt r3.x, r2.z, l(0.000000)
    r3.x = (asfloat((uint4)((r2.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 53: mad r2.z, -r2.z, cb0[10].x, l(1.000000)
    r2.z = ((-(r2.zzzz))*(source[10].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 54: mul_sat r2.z, r2.z, cb0[11].x
    r2.z = (saturate((r2.zzzz)*(source[11].xxxx))).z;
    // 55: movc r1.y, r3.x, l(0), r1.w
    r1.y = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 56: mad r1.xy, r0.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((r0.wwww)*(r2.xyxx)+(r1.xyxx)).xy;
    // 57: mul r3.x, r1.x, cb0[5].x
    r3.x = ((r1.xxxx)*(source[5].xxxx)).x;
    // 58: mad r2.y, r1.y, cb0[5].y, v4.x
    r2.y = ((r1.yyyy)*(source[5].yyyy)+(v4.xxxx)).y;
    // 59: mul r2.x, cb0[6].y, cb0[12].w
    r2.x = ((source[6].yyyy)*(source[12].wwww)).x;
    // 60: mov r3.z, l(-1.000000)
    r3.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 61: add r2.xy, r2.xyxx, r3.xzxx
    r2.xy = ((r2.xyxx)+(r3.xzxx)).xy;
    // 62: mul r2.xy, r2.xyxx, cb0[6].xxxx
    r2.xy = ((r2.xyxx)*(source[6].xxxx)).xy;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s4, l(-1.000000)
    r0.w = (ArtistNativeSample3((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 64: dp2 r2.x, cb0[3].xyxx, r0.yzyy
    r2.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[4].xyxx, r0.yzyy
    r2.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 66: mad r0.yz, cb0[12].xxxx, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((source[12].xxxx)*(r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 68: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 69: log r0.z, r2.z
    r0.z = (log2(r2.zzzz)).z;
    // 70: lt r0.w, r2.z, l(0.000001)
    r0.w = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 71: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 72: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 73: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 74: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 75: mul_sat r0.y, r0.y, cb0[13].x
    r0.y = (saturate((r0.yyyy)*(source[13].xxxx))).y;
    // 76: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 77: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 78: mul r0.z, r0.z, cb0[13].y
    r0.z = ((r0.zzzz)*(source[13].yyyy)).z;
    // 79: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 80: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 81: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 82: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 83: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 84: mul r0.y, r1.x, cb0[6].w
    r0.y = ((r1.xxxx)*(source[6].wwww)).y;
    // 85: mad r1.y, cb0[7].x, r1.y, r2.w
    r1.y = ((source[7].xxxx)*(r1.yyyy)+(r2.wwww)).y;
    // 86: mad r1.x, r1.z, cb0[6].z, r0.y
    r1.x = ((r1.zzzz)*(source[6].zzzz)+(r0.yyyy)).x;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t4.wxyz, s1, l(-1.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 88: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 89: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 90: mad r0.yzw, cb0[9].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[9].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 91: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 92: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 93: mul r0.yzw, r0.yyzw, cb0[9].yyyy
    r0.yzw = ((r0.yyzw)*(source[9].yyyy)).yzw;
    // 94: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 95: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 96: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 97: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 98: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 99: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 100: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_01_09_tr: ad42f283a770bb4ca78f8baa88727bbd; selected map dae87ca73a1142a5b49a2e9752391354f42ce3d26558852fb0e6ef85a5fadca4.
float4 ArtistNative3197(ARTIST_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].xxxx,g_ArtistSourceMaterialParameters[3u].yyyy,1u);
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].zzzz,g_ArtistSourceMaterialParameters[4u].wwww,1u);
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mov r0.y, cb0[12].z
    r0.y = (source[12].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.xyzw, v4.xyxy, cb0[4].xyxy, r0.yxxy
    r0.xyzw = ((v4.xyxy)*(source[4].xyxy)+(r0.yxxy)).xyzw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.zw, v4.xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)).zw;
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
    // 16: mul r0.xy, r0.xyxx, cb0[5].xxxx
    r0.xy = ((r0.xyxx)*(source[5].xxxx)).xy;
    // 17: mad r0.zw, v4.xxxy, cb0[7].xxxy, r0.xxxy
    r0.zw = ((v4.xxxy)*(source[7].xxxy)+(r0.xxxy)).zw;
    // 18: mad r0.xy, v4.xyxx, cb0[3].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)+(r0.xyxx)).xy;
    // 19: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: add r0.xy, r0.zwzz, cb0[8].xyxx
    r0.xy = ((r0.zwzz)+(source[8].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 25: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 26: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 27: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 28: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 29: mul r0.xyz, r0.xyzx, cb0[13].yyyy
    r0.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 30: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, cb0[13].zzzz
    r0.xyz = ((r0.xyzx)*(source[13].zzzz)).xyz;
    // 32: mad r0.xyz, cb0[1].xyzx, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 33: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 34: mad r0.xy, v4.xyxx, cb0[9].xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)*(source[9].xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 35: dp2 r2.x, cb0[10].xyxx, r0.xyxx
    r2.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 36: dp2 r2.y, cb0[11].xyxx, r0.xyxx
    r2.y = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 37: add r0.xy, r2.xyxx, cb0[5].zyzz
    r0.xy = ((r2.xyxx)+(source[5].zyzz)).xy;
    // 38: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 39: mad r0.xy, r1.xxxx, cb0[14].zzzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[14].zzzz)+(r0.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 41: mad r0.y, v4.y, l(2.000000), l(-1.000000)
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 42: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 44: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[14].w
    r0.z = ((r0.zzzz)*(source[14].wwww)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 50: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 51: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 52: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 53: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 55: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_ring_07_03_ad: 2e9f758287cb5f4eb21139e861d1db8e; selected map 5fb3b5a074b6f07f0ae700b728b14e35adab76cd4529f25b2f6eec2072d92e8a.
float4 ArtistNative3198(ARTIST_NATIVE_INPUT input)
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
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t3.xwyz, s0, cb0[5].x
    r0.xzw = (ArtistNativeSample0((r0.xzxx).xy, (source[5].xxxx).x, true).xwyz).xzw;
    // 47: mad r1.xy, r0.yyyy, cb0[8].wwww, r1.xyxx
    r1.xy = ((r0.yyyy)*(source[8].wwww)+(r1.xyxx)).xy;
    // 48: mad r1.zw, r0.yyyy, cb0[8].wwww, v2.xxxy
    r1.zw = ((r0.yyyy)*(source[8].wwww)+(v2.xxxy)).zw;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 50: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, cb0[5].x
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (source[5].xxxx).x, true).xyzw).x;
    // 51: mul_sat r1.x, r1.x, cb0[11].z
    r1.x = (saturate((r1.xxxx)*(source[11].zzzz))).x;
    // 52: mov_sat r1.y, v4.y
    r1.y = (saturate(v4.yyyy)).y;
    // 53: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 55: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 56: mul_sat r0.y, r0.y, r1.x
    r0.y = (saturate((r0.yyyy)*(r1.xxxx))).y;
    // 57: log r1.x, r0.y
    r1.x = (log2(r0.yyyy)).x;
    // 58: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 59: mul r1.x, r1.x, cb0[11].w
    r1.x = ((r1.xxxx)*(source[11].wwww)).x;
    // 60: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 61: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 62: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 63: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 64: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 65: add r1.xyz, -r0.xzwx, r1.xxxx
    r1.xyz = ((-(r0.xzwx))+(r1.xxxx)).xyz;
    // 66: mad r0.xzw, cb0[9].xxxx, r1.xxyz, r0.xxzw
    r0.xzw = ((source[9].xxxx)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 67: max r0.xzw, |r0.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r0.xzw = (max(abs(r0.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 68: log r0.xzw, r0.xxzw
    r0.xzw = (log2(r0.xxzw)).xzw;
    // 69: mul r0.xzw, r0.xxzw, cb0[9].yyyy
    r0.xzw = ((r0.xxzw)*(source[9].yyyy)).xzw;
    // 70: exp r0.xzw, r0.xxzw
    r0.xzw = (exp2(r0.xxzw)).xzw;
    // 71: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 72: mul r0.xzw, r0.xxzw, r1.xxyz
    r0.xzw = ((r0.xxzw)*(r1.xxyz)).xzw;
    // 73: mad r0.xzw, r0.xxzw, v3.xxyz, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)+(source[1].xxyz)).xzw;
    // 74: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 75: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 76: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_c_pa_ring_04_ad: 74031fe490d5d84eb9deafc3fb4f4c98; selected map 5a2c861629a6bbd6b9b467b66de6ec9481566b17a206bbaf3698481526c3143f.
float4 ArtistNative3199(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3136Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = input.dynamicParameter;
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].yyyy,g_ArtistSourceMaterialParameters[4u].zzzz,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_ArtistSourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
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
    // 13: mul r0.xyzw, r0.xyzw, cb0[6].zzzz
    r0.xyzw = ((r0.xyzw)*(source[6].zzzz)).xyzw;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3137Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3138Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3139Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3140Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[3].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[4].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)*float4(0.5, 0.0, 0.0, 0.0)))).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.xyzw, v2.xyxy, cb0[3].yyww
    r0.xyzw = ((v2.xyxy)*(source[3].yyww)).xyzw;
    // 2: mul r0.xy, r0.xyxx, l(4.000000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(4.000000,0.500000,0.000000,0.000000))).xy;
    // 3: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 3.000000, 0.500000), cb0[1].xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,3.000000,0.500000))+(source[1].xxxy)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 5: mad r0.xy, r0.zzzz, cb0[4].zzzz, r0.xyxx
    r0.xy = ((r0.zzzz)*(source[4].zzzz)+(r0.xyxx)).xy;
    // 6: add r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)+(source[2].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 8: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 9: div r0.yz, v3.xxyx, v3.wwww
    r0.yz = ((v3.xxyx)/(v3.wwww)).yz;
    // 10: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 11: source device depth mapped to centimetre view depth; reconstruction at 13.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 13-16: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 17: add r0.w, r0.w, -v3.w
    r0.w = ((r0.wwww)+(-(v3.wwww))).w;
    // 18: mul_sat r0.w, r0.w, l(0.010000)
    r0.w = (saturate((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000)))).w;
    // 19: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 20: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 21: mul r1.x, r1.x, l(0.250000)
    r1.x = ((r1.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 22: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 23: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 24: movc r0.w, r0.w, l(1.000000), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r1.xxxx)).w;
    // 25: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 26: mul r0.x, r0.x, cb0[0].w
    r0.x = ((r0.xxxx)*(source[0].wwww)).x;
    // 27: mad r1.xyzw, r0.xxxx, l(90.000000, -90.000000, 90.000000, -90.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r1.xyzw = ((r0.xxxx)*(float4(90.000000,-90.000000,90.000000,-90.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 28: mad r1.xyzw, r1.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r1.xyzw = ((r1.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 29: dp2 r0.x, r1.zwzz, r1.zwzz
    r0.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 30: add r0.x, r0.x, l(-0.100000)
    r0.x = ((r0.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 31: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 32: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) return 0.f;
    // 33: mad r0.xy, r1.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r0.yzyy
    r0.xy = ((r1.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r0.yzyy)).xy;
    // 34: mul r0.zw, r1.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r1.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 35: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 36: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 37: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 38: source device depth mapped to centimetre view depth; reconstruction at 40.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 40-43: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 44: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 45: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 46: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 47: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 48: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3141Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3142Distortion(ARTIST_NATIVE_INPUT input)
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
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3145Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3146Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3147Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3148Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3154Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3156Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[2] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].yyyy),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[3u].zzzz),1u);
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy),1u);
    source[5] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[6].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[7].y = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
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
    // 43: mul r0.xyzw, r0.xyzw, cb0[9].zzzz
    r0.xyzw = ((r0.xyzw)*(source[9].zzzz)).xyzw;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3158Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3161Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = input.dynamicParameter;
    source[2] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].yyyy,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[6].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)).x;
    source[6].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[7].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[7].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[7].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[9].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[10].x = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.xyxx
    r1.x = (dot((source[2].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.xyxx
    r1.y = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.z, r0.x, cb0[5].w
    r0.z = ((r0.xxxx)*(source[5].wwww)).z;
    // 6: mad r1.x, cb0[5].z, cb0[5].y, r0.z
    r1.x = ((source[5].zzzz)*(source[5].yyyy)+(r0.zzzz)).x;
    // 7: mul r0.z, r0.y, cb0[6].x
    r0.z = ((r0.yyyy)*(source[6].xxxx)).z;
    // 8: mad r1.y, cb0[5].z, cb0[7].w, r0.z
    r1.y = ((source[5].zzzz)*(source[7].wwww)+(r0.zzzz)).y;
    // 9: add r0.zw, r1.xxxy, cb0[4].xxxy
    r0.zw = ((r1.xxxy)+(source[4].xxxy)).zw;
    // 10: add r1.x, cb0[1].w, cb0[10].x
    r1.x = ((source[1].wwww)+(source[10].xxxx)).x;
    // 11: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 12: mul r1.yz, r0.xxyx, cb0[9].xxyx
    r1.yz = ((r0.xxyx)*(source[9].xxyx)).yz;
    // 13: mul r0.xy, r0.xyxx, cb0[12].xyxx
    r0.xy = ((r0.xyxx)*(source[12].xyxx)).xy;
    // 14: mad r1.y, cb0[5].z, cb0[8].w, r1.y
    r1.y = ((source[5].zzzz)*(source[8].wwww)+(r1.yyyy)).y;
    // 15: mad r1.z, cb0[5].z, cb0[9].z, r1.z
    r1.z = ((source[5].zzzz)*(source[9].zzzz)+(r1.zzzz)).z;
    // 16: mad r2.y, cb0[1].y, cb0[9].w, r1.z
    r2.y = ((source[1].yyyy)*(source[9].wwww)+(r1.zzzz)).y;
    // 17: mad r2.x, cb0[1].y, cb0[8].z, r1.y
    r2.x = ((source[1].yyyy)*(source[8].zzzz)+(r1.yyyy)).x;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r2.xyxx, t0.zxyw, s1, l(0.000000)
    r1.yz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 19: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 20: add r1.xy, cb0[1].xzxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[1].xzxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mad r2.x, r1.x, cb0[5].x, r0.z
    r2.x = ((r1.xxxx)*(source[5].xxxx)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[10].y, r0.w
    r2.y = ((r1.xxxx)*(source[10].yyyy)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s2, l(0.000000)
    r0.zw = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: mul r1.x, r1.x, l(20.000000)
    r1.x = ((r1.xxxx)*(float4(20.000000,20.000000,20.000000,20.000000))).x;
    // 26: mad r2.xyzw, r0.zwzw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r2.xyzw = ((r0.zwzw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 27: mul r2.xyzw, r1.xxxx, r2.xyzw
    r2.xyzw = ((r1.xxxx)*(r2.xyzw)).xyzw;
    // 28: mad r0.x, cb0[5].z, cb0[11].w, r0.x
    r0.x = ((source[5].zzzz)*(source[11].wwww)+(r0.xxxx)).x;
    // 29: mad r0.y, cb0[5].z, cb0[12].z, r0.y
    r0.y = ((source[5].zzzz)*(source[12].zzzz)+(r0.yyyy)).y;
    // 30: mad r3.y, cb0[1].y, cb0[12].w, r0.y
    r3.y = ((source[1].yyyy)*(source[12].wwww)+(r0.yyyy)).y;
    // 31: mad r3.x, cb0[1].y, cb0[11].z, r0.x
    r3.x = ((source[1].yyyy)*(source[11].zzzz)+(r0.xxxx)).x;
    // 32: add r0.xy, r3.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r3.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 33: dp2 r3.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.xyxx
    r3.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).x;
    // 34: dp2 r3.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.xyxx
    r3.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).y;
    // 35: add r0.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, cb0[13].x
    r0.x = (saturate((r0.xxxx)*(source[13].xxxx))).x;
    // 39: mul r0.xyzw, r2.xyzw, r0.xxxx
    r0.xyzw = ((r2.xyzw)*(r0.xxxx)).xyzw;
    // 40: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
    // 41: mul r0.xyzw, r0.xyzw, cb0[14].xxxx
    r0.xyzw = ((r0.xyzw)*(source[14].xxxx)).xyzw;
    // 42: add r1.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 43: mul r1.xy, r1.xyxx, v2.xyxx
    r1.xy = ((r1.xyxx)*(v2.xyxx)).xy;
    // 44: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 45: mul_sat r1.x, r1.x, cb0[13].y
    r1.x = (saturate((r1.xxxx)*(source[13].yyyy))).x;
    // 46: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 47: dp3 r1.x, v4.xyzx, v4.xyzx
    r1.x = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).x;
    // 48: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 49: mul r1.x, r1.x, v4.z
    r1.x = ((r1.xxxx)*(v4.zzzz)).x;
    // 50: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 51: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 52: mul r1.y, r1.y, cb0[13].z
    r1.y = ((r1.yyyy)*(source[13].zzzz)).y;
    // 53: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 54: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 55: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 56: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 57: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 58: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 59: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 60: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 61: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 62: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 63: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 64: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 65: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 66: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 67: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 68: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 69: source device depth mapped to centimetre view depth; reconstruction at 71.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 71-74: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 75: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 76: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 77: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 78: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 79: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3162Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3163Distortion(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3165Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3166Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3168Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = input.dynamicParameter;
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mov r0.z, l(0)
    r0.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 2: mul r0.x, cb0[1].x, cb0[2].x
    r0.x = ((source[1].xxxx)*(source[2].xxxx)).x;
    // 3: mul r0.y, r0.x, l(-0.200000)
    r0.y = ((r0.xxxx)*(float4(-0.200000,-0.200000,-0.200000,-0.200000))).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 5: mad r1.xy, v2.xyxx, l(3.000000, 3.000000, 0.000000, 0.000000), r0.wwww
    r1.xy = ((v2.xyxx)*(float4(3.000000,3.000000,0.000000,0.000000))+(r0.wwww)).xy;
    // 6: add r1.zw, r0.wwww, v2.xxxy
    r1.zw = ((r0.wwww)+(v2.xxxy)).zw;
    // 7: mad r0.xw, r0.xxxx, l(-0.100000, 0.000000, 0.000000, -0.100000), r1.zzzw
    r0.xw = ((r0.xxxx)*(float4(-0.100000,0.000000,0.000000,-0.100000))+(r1.zzzw)).xw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xw, r0.xwxx, t1.xzwy, s2, l(0.000000)
    r0.xw = (ArtistNativeSample1((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzwy).xw;
    // 9: add r0.yz, r0.zzyz, r1.xxyx
    r0.yz = ((r0.zzyz)+(r1.xxyx)).yz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 11: mul r0.y, r0.y, l(400.000000)
    r0.y = ((r0.yyyy)*(float4(400.000000,400.000000,400.000000,400.000000))).y;
    // 12: mul r0.xyzw, r0.xwxw, r0.yyyy
    r0.xyzw = ((r0.xwxw)*(r0.yyyy)).xyzw;
    // 13: mul r0.xyzw, r0.xyzw, cb0[0].wwww
    r0.xyzw = ((r0.xyzw)*(source[0].wwww)).xyzw;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3169Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3174Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3177Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[1] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
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
    // 3: mad r0.yz, v1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000), cb0[1].xxyx
    r0.yz = ((v1.xxyx)*(float4(0.000000,0.500000,0.500000,0.000000))+(source[1].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.xywz, s1, l(0.000000)
    r0.w = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).w;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t2.zxyw, s3, l(0.000000)
    r0.yz = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 6: mad r1.xyzw, r0.yzyz, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r1.xyzw = ((r0.yzyz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 7: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 8: mul_sat r0.x, r0.x, l(30.000000)
    r0.x = (saturate((r0.xxxx)*(float4(30.000000,30.000000,30.000000,30.000000)))).x;
    // 9: mul r0.xyzw, r1.xyzw, r0.xxxx
    r0.xyzw = ((r1.xyzw)*(r0.xxxx)).xyzw;
    // 10: mul r0.xyzw, r0.xyzw, v2.wwww
    r0.xyzw = ((r0.xyzw)*(v2.wwww)).xyzw;
    // 11: add r1.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: mad r1.xy, r1.xyxx, l(0.450000, 0.450000, 0.000000, 0.000000), l(0.275000, 0.275000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.450000,0.450000,0.000000,0.000000))+(float4(0.275000,0.275000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: add r1.yz, -v1.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(v1.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 15: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 16: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 17: mul r1.x, r1.x, v2.w
    r1.x = ((r1.xxxx)*(v2.wwww)).x;
    // 18: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3184Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3185Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3190Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3191Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3192Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3193Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = g_ArtistSourceMaterialParameters[6u];
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[2].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[2].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[3].x = ((g_ArtistSourceMaterialParameters[3u].xxxx*g_ArtistSourceMaterialTime.xxxx)).x;
    source[3].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[3].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].w = ((g_ArtistSourceMaterialParameters[1u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[5].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[5].w = ((g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[6].x = (((g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx)*g_ArtistSourceMaterialParameters[4u].xxxx)).x;
    source[6].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].y = ((float4(0.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].yyyy)).x;
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
    // 29: mul r0.yz, r0.yyzy, cb0[2].zzwz
    r0.yz = ((r0.yyzy)*(source[2].zzwz)).yz;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r1.x, r0.y, cb0[3].x
    r1.x = ((r0.yyyy)+(source[3].xxxx)).x;
    // 32: mul r2.x, r1.x, cb0[5].x
    r2.x = ((r1.xxxx)*(source[5].xxxx)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 35: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 36: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
    // 37: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 38: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 39: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 40: add r0.y, v3.x, cb0[3].w
    r0.y = ((v3.xxxx)+(source[3].wwww)).y;
    // 41: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 42: mad r2.y, r1.y, cb0[5].y, cb0[6].x
    r2.y = ((r1.yyyy)*(source[5].yyyy)+(source[6].xxxx)).y;
    // 43: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r2.xyxx, t0.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xy;
    // 44: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 45: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 46: mul r0.xy, r0.xyxx, cb0[6].yyyy
    r0.xy = ((r0.xyxx)*(source[6].yyyy)).xy;
    // 47: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 48: mul r2.x, r1.x, cb0[4].x
    r2.x = ((r1.xxxx)*(source[4].xxxx)).x;
    // 49: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[1].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[1].xxxy)).zw;
    // 50: mad r2.y, r1.y, cb0[4].y, cb0[4].w
    r2.y = ((r1.yyyy)*(source[4].yyyy)+(source[4].wwww)).y;
    // 51: mad r1.xy, cb0[6].zzzz, r0.xyxx, r2.xyxx
    r1.xy = ((source[6].zzzz)*(r0.xyxx)+(r2.xyxx)).xy;
    // 52: mad r0.xy, cb0[6].zzzz, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].zzzz)*(r0.xyxx)+(r0.zwzz)).xy;
    // 53: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 54: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t1.wxyz, s1, l(0.000000)
    r0.yzw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).wxyz).yzw;
    // 55: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 56: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 57: mul r0.yzw, r0.yyzw, cb0[6].wwww
    r0.yzw = ((r0.yyzw)*(source[6].wwww)).yzw;
    // 58: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 59: mul r1.xyz, r0.yzwy, cb0[7].xxxx
    r1.xyz = ((r0.yzwy)*(source[7].xxxx)).xyz;
    // 60: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: mad r2.xyzw, -cb0[7].xxxx, r0.yzyz, r0.wwww
    r2.xyzw = ((-(source[7].xxxx))*(r0.yzyz)+(r0.wwww)).xyzw;
    // 62: mad r1.xyzw, cb0[7].yyyy, r2.xyzw, r1.xyxy
    r1.xyzw = ((source[7].yyyy)*(r2.xyzw)+(r1.xyxy)).xyzw;
    // 63: mul r1.xyzw, r1.xyzw, cb0[0].xyxy
    r1.xyzw = ((r1.xyzw)*(source[0].xyxy)).xyzw;
    // 64: mul r0.xyzw, r0.xxxx, r1.xyzw
    r0.xyzw = ((r0.xxxx)*(r1.xyzw)).xyzw;
    // 65: mul r0.xyzw, r0.xyzw, cb0[8].xxxx
    r0.xyzw = ((r0.xyzw)*(source[8].xxxx)).xyzw;
    // 66: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 67: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 68: mul r0.xyzw, r0.xyzw, cb0[8].yyyy
    r0.xyzw = ((r0.xyzw)*(source[8].yyyy)).xyzw;
    // 69: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 70: mul r0.xyzw, r0.xyzw, cb0[9].yyyy
    r0.xyzw = ((r0.xyzw)*(source[9].yyyy)).xyzw;
    // 71: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 72: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 73: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 74: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 75: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 76: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 77: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 78: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 79: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 80: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 81: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 82: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 83: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 84: source device depth mapped to centimetre view depth; reconstruction at 86.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 86-89: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 90: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 91: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 92: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 93: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 94: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative3196Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3198Distortion(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative3199Distortion(ARTIST_NATIVE_INPUT input)
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
