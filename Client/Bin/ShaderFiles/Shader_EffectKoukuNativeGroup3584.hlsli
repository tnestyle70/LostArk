// Original Kouku material programs 3584..3647; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_ddk_crackboundary_01_1_tr: 8dc1179e23183a4989457474bd729b63; selected map 03f399f140a27ba331d647f0aa0ad387a212f79b6de06163bdf6ece7459d0e82.
float4 ArtistNative3584(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].xxxx)).x;
    source[5].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[5].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: mul r0.xyz, r0.xyzx, l(-1.000000, -1.000000, 1.000000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(-1.000000,-1.000000,1.000000,0.000000))).xyz;
    // 5: add r1.xy, v7.xyxx, cb0[0].xyxx
    r1.xy = ((v7.xyxx)+(source[0].xyxx)).xy;
    // 6: mul r1.zw, r1.xxxy, cb0[3].xxxx
    r1.zw = ((r1.xxxy)*(source[3].xxxx)).zw;
    // 7: mul r1.xy, r1.xyxx, l(0.005000, 0.005000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.005000,0.005000,0.000000,0.000000))).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 9: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mad r1.xy, r1.xyxx, l(0.030000, 0.030000, 0.000000, 0.000000), v2.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.030000,0.030000,0.000000,0.000000))+(v2.xyxx)).xy;
    // 11: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.zwzz, t3.xyzw, s3, l(0.000000)
    r1.xy = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 15: mad r2.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: dp2 r1.x, r2.xyxx, r2.xyxx
    r1.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 17: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 18: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 19: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 20: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: dp3_sat r0.x, r0.xyzx, r2.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // 22: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 23: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 24: mul r0.y, r0.y, cb0[4].y
    r0.y = ((r0.yyyy)*(source[4].yyyy)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[4].z
    r0.y = ((r0.yyyy)*(source[4].zzzz)).y;
    // 27: mad r0.z, -r0.w, l(2.000000), l(1.000000)
    r0.z = ((-(r0.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: mul_sat r1.x, r0.z, l(9.999998)
    r1.x = (saturate((r0.zzzz)*(float4(9.999998,9.999998,9.999998,9.999998)))).x;
    // 29: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 30: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: mul r1.y, r1.x, l(100.000000)
    r1.y = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 32: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: add r2.x, -r1.x, l(1.000000)
    r2.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: log r2.y, r2.x
    r2.y = (log2(r2.xxxx)).y;
    // 35: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 36: mul r2.y, r2.y, l(12.000000)
    r2.y = ((r2.yyyy)*(float4(12.000000,12.000000,12.000000,12.000000))).y;
    // 37: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 38: mul r1.y, r1.y, r2.y
    r1.y = ((r1.yyyy)*(r2.yyyy)).y;
    // 39: movc r1.y, r2.x, l(0), r1.y
    r1.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 40: add r2.x, -r1.y, l(1.000000)
    r2.x = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: mul r2.y, r2.x, r2.x
    r2.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 42: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 43: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 44: movc r2.x, r2.x, l(0), r2.y
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).x;
    // 45: mul r0.y, r0.y, r2.x
    r0.y = ((r0.yyyy)*(r2.xxxx)).y;
    // 46: mul r2.xyz, v3.xyzx, cb0[4].xxxx
    r2.xyz = ((v3.xyzx)*(source[4].xxxx)).xyz;
    // 47: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 48: movc r2.xyz, r0.xxxx, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 49: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 50: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 51: mad r2.xyz, cb0[4].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[4].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 52: log r0.x, r0.z
    r0.x = (log2(r0.zzzz)).x;
    // 53: lt r0.y, r0.z, l(0.000001)
    r0.y = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 54: mul r0.x, r0.x, l(12.000000)
    r0.x = ((r0.xxxx)*(float4(12.000000,12.000000,12.000000,12.000000))).x;
    // 55: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 56: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r1.zwzz, t2.zxyw, s0, l(0.000000)
    r0.yz = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 58: add r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)+(r1.zzzw)).zw;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t1.yzxw, s2, l(0.000000)
    r1.z = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 60: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 61: mul r1.z, r1.z, l(0.300000)
    r1.z = ((r1.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))).z;
    // 62: log r1.w, |r0.z|
    r1.w = (log2(abs(r0.zzzz))).w;
    // 63: mul r3.xy, r1.wwww, cb0[3].yzyy
    r3.xy = ((r1.wwww)*(source[3].yzyy)).xy;
    // 64: exp r3.xy, r3.xyxx
    r3.xy = (exp2(r3.xyxx)).xy;
    // 65: mul r1.w, r3.y, cb0[3].w
    r1.w = ((r3.yyyy)*(source[3].wwww)).w;
    // 66: mul r3.xyz, r3.xxxx, cb0[2].xyzx
    r3.xyz = ((r3.xxxx)*(source[2].xyzx)).xyz;
    // 67: mul r3.xyz, r3.xyzx, v4.yyyy
    r3.xyz = ((r3.xyzx)*(v4.yyyy)).xyz;
    // 68: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 69: dp2_sat r0.y, r0.yyyy, r1.xxxx
    r0.y = (saturate(dot((r0.yyyy).xy,(r1.xxxx).xy).xxxx)).y;
    // 70: movc r1.w, r0.z, l(0), r1.w
    r1.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 71: movc r3.xyz, r0.zzzz, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 72: mad r0.z, r0.x, r1.z, r1.w
    r0.z = ((r0.xxxx)*(r1.zzzz)+(r1.wwww)).z;
    // 73: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 74: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 75: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 76: mad r0.z, r1.y, l(300.000000), r0.z
    r0.z = ((r1.yyyy)*(float4(300.000000,300.000000,300.000000,300.000000))+(r0.zzzz)).z;
    // 77: mad r1.yzw, r0.zzzz, v3.xxyz, r2.xxyz
    r1.yzw = ((r0.zzzz)*(v3.xxyz)+(r2.xxyz)).yzw;
    // 78: add r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)+(r3.xxyz)).yzw;
    // 79: add r1.yzw, r1.yyzw, cb0[1].xxyz
    r1.yzw = ((r1.yyzw)+(source[1].xxyz)).yzw;
    // 80: mad o0.xyz, r1.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r1.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 81: mad r0.x, r0.x, r1.x, r0.y
    r0.x = ((r0.xxxx)*(r1.xxxx)+(r0.yyyy)).x;
    // 82: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 83: add r0.y, v4.w, l(-1.000000)
    r0.y = ((v4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 84: max r0.y, r0.y, l(0.000010)
    r0.y = (max(r0.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 85: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 86: mad r0.y, -r0.w, r0.y, l(1.000000)
    r0.y = ((-(r0.wwww))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 87: mul_sat r0.y, r0.y, cb0[5].w
    r0.y = (saturate((r0.yyyy)*(source[5].wwww))).y;
    // 88: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 90: mul o0.w, r0.x, cb0[0].w
    output.w = ((r0.xxxx)*(source[0].wwww)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_r_pa_ring_09_10_ad: f2522047019b80478e44d73cb0ff84af; selected map c58e1e8dbee8b7f0facd4f7701f6dcdd1e14c1836718a3dff5dd0f2656f1fab8.
float4 ArtistNative3585(ARTIST_NATIVE_INPUT input)
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
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_localcrack_01_07_tr: 8b228c7b319b544781cca408746673a2; selected map cb7fea6335f99773642abba576c9c60a2c351f25a720c8f239556eb64c8886fd.
float4 ArtistNative3586(ARTIST_NATIVE_INPUT input)
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
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_118_fs_dt_ad: 305f52979299224e8136f5c440cb89d9; selected map 10f08d6f0e89becd42fc8396fc5a8febea9e88ebb347160baf0a60a97058f2a9.
float4 ArtistNative3587(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v6.w
    r0.x = ((r0.xxxx)+(-(v6.wwww))).x;
    // 10: add r0.y, -cb0[4].z, l(1.000000)
    r0.y = ((-(source[4].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v5.z
    r0.y = ((r0.yyyy)*(v5.zzzz)).y;
    // 16: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 17: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 18: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[4].w
    r0.z = ((r0.zzzz)*(source[4].wwww)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul_sat r0.z, r0.z, cb0[5].x
    r0.z = (saturate((r0.zzzz)*(source[5].xxxx))).z;
    // 22: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 23: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 24: add r0.y, -cb0[3].x, l(1.000000)
    r0.y = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 25: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mul_sat r0.y, r0.y, cb0[4].x
    r0.y = (saturate((r0.yyyy)*(source[4].xxxx))).y;
    // 27: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 28: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 29: mul r0.z, r0.z, cb0[4].y
    r0.z = ((r0.zzzz)*(source[4].yyyy)).z;
    // 30: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 31: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 32: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 33: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 34: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 35: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 36: mul r0.yzw, r0.yyzw, v4.wwww
    r0.yzw = ((r0.yyzw)*(v4.wwww)).yzw;
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
// fx_r_me_master_34_02_dt_fs_tr: be721e1393dd65428e755042d8c46f9e; selected map 62f745525b8f3787db8c18220218e9dd12447b4dd6a68fb9b0ecc5328005e8ab.
float4 ArtistNative3588(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    source[9].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 1: mul r0.xy, v4.xyxx, cb0[6].zwzz
    r0.xy = ((v4.xyxx)*(source[6].zwzz)).xy;
    // 2: mul r0.z, cb0[5].x, cb0[5].y
    r0.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[6].y, r0.x
    r1.x = ((r0.zzzz)*(source[6].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[7].x, r0.y
    r1.y = ((r0.zzzz)*(source[7].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[7].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[7].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[5].w
    r0.w = ((r0.xxxx)*(source[5].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[5].z, r0.w
    r1.x = ((r0.zzzz)*(source[5].zzzz)+(r0.wwww)).x;
    // 9: mul r0.w, r0.z, cb0[7].w
    r0.w = ((r0.zzzz)*(source[7].wwww)).w;
    // 10: mad r1.y, cb0[6].x, r0.y, r0.w
    r1.y = ((source[6].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 11: mul r0.xy, r0.xyxx, cb0[8].yzyy
    r0.xy = ((r0.xyxx)*(source[8].yzyy)).xy;
    // 12: mad r0.xy, r0.zzzz, cb0[8].xwxx, r0.xyxx
    r0.xy = ((r0.zzzz)*(source[8].xwxx)+(r0.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 16: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 18: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 19: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 20: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 21: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 22: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 23: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 24: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 26: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 27: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 28: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 30: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 31: mul r1.y, r1.x, cb0[9].z
    r1.y = ((r1.xxxx)*(source[9].zzzz)).y;
    // 32: mul r1.x, r1.x, cb0[10].w
    r1.x = ((r1.xxxx)*(source[10].wwww)).x;
    // 33: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 34: mul_sat r1.x, r1.x, cb0[11].x
    r1.x = (saturate((r1.xxxx)*(source[11].xxxx))).x;
    // 35: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 36: mul r1.y, r1.y, cb0[9].w
    r1.y = ((r1.yyyy)*(source[9].wwww)).y;
    // 37: movc r1.y, r0.w, l(0), r1.y
    r1.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 38: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 39: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 41: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 42: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 43: source device depth mapped to centimetre view depth; reconstruction at 45.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 45-48: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 49: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 50: add r0.y, -cb0[10].z, l(1.000000)
    r0.y = ((-(source[10].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 52: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 53: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 54: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 55: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 56: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 57: mul_sat r0.y, r0.y, cb0[10].x
    r0.y = (saturate((r0.yyyy)*(source[10].xxxx))).y;
    // 58: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 59: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 60: mul r0.z, r0.z, cb0[10].y
    r0.z = ((r0.zzzz)*(source[10].yyyy)).z;
    // 61: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 62: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 63: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 64: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 65: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_splitline_01_1_ad: 97638bedbf7a4b49ab33a1ee2c8a688b; selected map e26d460f95b0214c7c0301599fbe2a21161ff06bf3013bbbb31782640e256f02.
float4 ArtistNative3589(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
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
    // 10: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul_sat r0.w, r0.w, v6.z
    r0.w = (saturate((r0.wwww)*(v6.zzzz))).w;
    // 13: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 14: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 15: source device depth mapped to centimetre view depth; reconstruction at 17.
    r1.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 17-20: reconstructed view depth is supplied by the runtime adapter.
    r1.w = r1.w;
    // 21: add r1.w, r1.w, -v7.w
    r1.w = ((r1.wwww)+(-(v7.wwww))).w;
    // 22: mul_sat r1.w, r1.w, l(0.036364)
    r1.w = (saturate((r1.wwww)*(float4(0.036364,0.036364,0.036364,0.036364)))).w;
    // 23: mul r2.x, r0.w, v4.y
    r2.x = ((r0.wwww)*(v4.yyyy)).x;
    // 24: mul r2.x, r2.x, cb0[1].w
    r2.x = ((r2.xxxx)*(source[1].wwww)).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mul o0.w, r1.w, cb0[0].x
    output.w = ((r1.wwww)*(source[0].xxxx)).w;
    // 27: add r1.w, v4.x, l(-0.500000)
    r1.w = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 28: add r2.x, |r1.w|, |r1.w|
    r2.x = ((abs(r1.wwww))+(abs(r1.wwww))).x;
    // 29: lt r1.w, |r1.w|, l(0.000000)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 30: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 31: mul r2.x, r2.x, l(10.000000)
    r2.x = ((r2.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 32: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 33: mul r2.x, r2.x, l(0.035000)
    r2.x = ((r2.xxxx)*(float4(0.035000,0.035000,0.035000,0.035000))).x;
    // 34: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 35: mad r1.w, cb0[3].x, l(0.010000), r1.w
    r1.w = ((source[3].xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r1.wwww)).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 37: add r3.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 38: mul r3.xy, r3.xyxx, l(0.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(0.000000,1.000000,0.000000,0.000000))).xy;
    // 39: mul r1.w, r1.w, l(0.100000)
    r1.w = ((r1.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 40: mov r3.zw, r1.yyyx
    r3.zw = (r1.yyyx).zw;
    // 41: mov r4.x, r2.x
    r4.x = (r2.xxxx).x;
    // 42: mov r4.y, l(0)
    r4.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 43: loop
    [loop] while (true) {
    // 44: ge r2.w, r4.y, l(3.000000)
    r2.w = (asfloat((uint4)((r4.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 45: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 46: mad r3.zw, -r3.yyyx, r1.wwww, r3.zzzw
    r3.zw = ((-(r3.yyyx))*(r1.wwww)+(r3.zzzw)).zw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.wzww, t1.yzwx, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.wzww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 48: add r4.x, r2.w, r4.x
    r4.x = ((r2.wwww)+(r4.xxxx)).x;
    // 49: add r4.y, r4.y, l(1.000000)
    r4.y = ((r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 50: endloop
    }
    // 51: mov r2.w, r1.x
    r2.w = (r1.xxxx).w;
    // 52: mov r2.x, r3.z
    r2.x = (r3.zzzz).x;
    // 53: mov r5.x, r2.y
    r5.x = (r2.yyyy).x;
    // 54: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 55: loop
    [loop] while (true) {
    // 56: ge r1.y, r5.y, l(3.000000)
    r1.y = (asfloat((uint4)((r5.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).y;
    // 57: breakc_nz r1.y
    if ((asuint(r1.yyyy)).x != 0u) break;
    // 58: mad r2.xw, -r3.yyyx, r1.wwww, r2.xxxw
    r2.xw = ((-(r3.yyyx))*(r1.wwww)+(r2.xxxw)).xw;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.wxww, t1.xyzw, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.wxww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).y;
    // 60: add r5.x, r1.y, r5.x
    r5.x = ((r1.yyyy)+(r5.xxxx)).x;
    // 61: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 62: endloop
    }
    // 63: mov r4.y, r5.x
    r4.y = (r5.xxxx).y;
    // 64: mov r5.x, r1.x
    r5.x = (r1.xxxx).x;
    // 65: mov r5.y, r2.x
    r5.y = (r2.xxxx).y;
    // 66: mov r6.x, r2.z
    r6.x = (r2.zzzz).x;
    // 67: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 68: loop
    [loop] while (true) {
    // 69: ge r1.y, r6.y, l(3.000000)
    r1.y = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).y;
    // 70: breakc_nz r1.y
    if ((asuint(r1.yyyy)).x != 0u) break;
    // 71: mad r5.xy, -r3.xyxx, r1.wwww, r5.xyxx
    r5.xy = ((-(r3.xyxx))*(r1.wwww)+(r5.xyxx)).xy;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r5.xyxx, t1.xzyw, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 73: add r6.x, r1.y, r6.x
    r6.x = ((r1.yyyy)+(r6.xxxx)).x;
    // 74: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: endloop
    }
    // 76: mov r4.z, r6.x
    r4.z = (r6.xxxx).z;
    // 77: mul r1.xyw, r4.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000)
    r1.xyw = ((r4.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))).xyw;
    // 78: dp3 r2.x, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 79: mad r2.xyz, -r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xxxx
    r2.xyz = ((-(r4.xyzx))*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xxxx)).xyz;
    // 80: mad r1.xyw, cb0[3].yyyy, r2.xyxz, r1.xyxw
    r1.xyw = ((source[3].yyyy)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 81: lt r2.x, r0.w, l(0.000001)
    r2.x = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 82: mul r2.y, r0.w, r0.w
    r2.y = ((r0.wwww)*(r0.wwww)).y;
    // 83: mul r2.y, r0.w, r2.y
    r2.y = ((r0.wwww)*(r2.yyyy)).y;
    // 84: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: lt r2.z, r0.w, l(0.000001)
    r2.z = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 86: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 87: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 88: or r2.x, r2.x, r2.z
    r2.x = (asfloat(asuint(r2.xxxx) | asuint(r2.zzzz))).x;
    // 89: movc r0.w, r2.x, l(0), r0.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 90: add r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)+(r1.xyxw)).xyw;
    // 91: mad r1.xyw, cb0[1].xyxz, r1.xyxw, cb0[2].xyxz
    r1.xyw = ((source[1].xyxz)*(r1.xyxw)+(source[2].xyxz)).xyw;
    // 92: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_r_me_master_34_01_dt_fs_tr: be721e1393dd65428e755042d8c46f9e; selected map 62f745525b8f3787db8c18220218e9dd12447b4dd6a68fb9b0ecc5328005e8ab.
float4 ArtistNative3590(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[8u];
    source[3] = g_ArtistSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
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
    source[9].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[10].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 1: mul r0.xy, v4.xyxx, cb0[6].zwzz
    r0.xy = ((v4.xyxx)*(source[6].zwzz)).xy;
    // 2: mul r0.z, cb0[5].x, cb0[5].y
    r0.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[6].y, r0.x
    r1.x = ((r0.zzzz)*(source[6].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[7].x, r0.y
    r1.y = ((r0.zzzz)*(source[7].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[7].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[7].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[5].w
    r0.w = ((r0.xxxx)*(source[5].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[5].z, r0.w
    r1.x = ((r0.zzzz)*(source[5].zzzz)+(r0.wwww)).x;
    // 9: mul r0.w, r0.z, cb0[7].w
    r0.w = ((r0.zzzz)*(source[7].wwww)).w;
    // 10: mad r1.y, cb0[6].x, r0.y, r0.w
    r1.y = ((source[6].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 11: mul r0.xy, r0.xyxx, cb0[8].yzyy
    r0.xy = ((r0.xyxx)*(source[8].yzyy)).xy;
    // 12: mad r0.xy, r0.zzzz, cb0[8].xwxx, r0.xyxx
    r0.xy = ((r0.zzzz)*(source[8].xwxx)+(r0.xyxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyz = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 15: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 16: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 18: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 19: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 20: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 21: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 22: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 23: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 24: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 26: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 27: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 28: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 30: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 31: mul r1.y, r1.x, cb0[9].z
    r1.y = ((r1.xxxx)*(source[9].zzzz)).y;
    // 32: mul r1.x, r1.x, cb0[10].w
    r1.x = ((r1.xxxx)*(source[10].wwww)).x;
    // 33: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 34: mul_sat r1.x, r1.x, cb0[11].x
    r1.x = (saturate((r1.xxxx)*(source[11].xxxx))).x;
    // 35: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 36: mul r1.y, r1.y, cb0[9].w
    r1.y = ((r1.yyyy)*(source[9].wwww)).y;
    // 37: movc r1.y, r0.w, l(0), r1.y
    r1.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 38: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 39: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 41: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 42: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 43: source device depth mapped to centimetre view depth; reconstruction at 45.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 45-48: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 49: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 50: add r0.y, -cb0[10].z, l(1.000000)
    r0.y = ((-(source[10].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 52: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 53: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 54: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 55: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 56: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 57: mul_sat r0.y, r0.y, cb0[10].x
    r0.y = (saturate((r0.yyyy)*(source[10].xxxx))).y;
    // 58: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 59: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 60: mul r0.z, r0.z, cb0[10].y
    r0.z = ((r0.zzzz)*(source[10].yyyy)).z;
    // 61: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 62: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 63: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 64: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 65: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_090_ad: 98db55007c09f9409494fa72c2ea954f; selected map 12cdf7d1da244208f2cf879b5c033f60dff038f9380bc5abb993a918a146c35f.
float4 ArtistNative3591(ARTIST_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[3u];
    source[3] = g_ArtistSourceMaterialParameters[1u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v5.z
    r0.x = ((r0.xxxx)*(v5.zzzz)).x;
    // 4: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 6: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 7: mul r0.y, r0.y, cb0[5].z
    r0.y = ((r0.yyyy)*(source[5].zzzz)).y;
    // 8: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 9: mul_sat r0.y, r0.y, cb0[5].w
    r0.y = (saturate((r0.yyyy)*(source[5].wwww))).y;
    // 10: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 11: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 12: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 13: mul_sat r0.y, r0.y, cb0[5].x
    r0.y = (saturate((r0.yyyy)*(source[5].xxxx))).y;
    // 14: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 15: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 16: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 19: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 20: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 21: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 22: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 23: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 24: mul r0.yzw, r0.yyzw, v4.wwww
    r0.yzw = ((r0.yyzw)*(v4.wwww)).yzw;
    // 25: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 26: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_spritewave_01_16_tr: 1263f0ecd202cf498a97cd6ab0fbccb6; selected map 94c4bf49e73b703d7b7908e42460da77e29ae24cda44695056e3f629d6faf345.
float4 ArtistNative3592(ARTIST_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[10u];
    source[2] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[8u].xxxx,g_ArtistSourceMaterialParameters[8u].yyyy,1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].wwww,g_ArtistSourceMaterialParameters[4u].xxxx,1u);
    source[6] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ArtistSourceMaterialParameters[9u];
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
    source[13].z = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[14].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[14].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
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
    source[21].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 32: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 33: mul r1.x, v2.x, cb0[18].w
    r1.x = ((v2.xxxx)*(source[18].wwww)).x;
    // 34: mad r1.x, cb0[11].w, cb0[18].z, r1.x
    r1.x = ((source[11].wwww)*(source[18].zzzz)+(r1.xxxx)).x;
    // 35: mul r1.zw, cb0[11].wwww, cb0[19].yyyw
    r1.zw = ((source[11].wwww)*(source[19].yyyw)).zw;
    // 36: mad r1.y, cb0[19].x, v2.y, r1.z
    r1.y = ((source[19].xxxx)*(v2.yyyy)+(r1.zzzz)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 38: mad r0.zw, r1.xxxx, cb0[19].zzzz, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[19].zzzz)+(r0.zzzw)).zw;
    // 39: mad r1.y, cb0[17].x, r0.w, r1.w
    r1.y = ((source[17].xxxx)*(r0.wwww)+(r1.wwww)).y;
    // 40: mul r0.z, r0.z, cb0[16].w
    r0.z = ((r0.zzzz)*(source[16].wwww)).z;
    // 41: mad r1.x, cb0[11].w, cb0[16].z, r0.z
    r1.x = ((source[11].wwww)*(source[16].zzzz)+(r0.zzzz)).x;
    // 42: add r0.zw, r1.xxxy, cb0[20].xxxy
    r0.zw = ((r1.xxxy)+(source[20].xxxy)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s3, l(0.000000)
    r0.z = (ArtistNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 44: add r0.z, r0.z, l(0.100000)
    r0.z = ((r0.zzzz)+(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 45: add r0.w, v4.y, l(-1.000000)
    r0.w = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 46: add_sat r0.z, -r0.w, r0.z
    r0.z = (saturate((-(r0.wwww))+(r0.zzzz))).z;
    // 47: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 48: mul r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)*(source[20].wwww)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: mul_sat r0.w, r0.w, cb0[20].z
    r0.w = (saturate((r0.wwww)*(source[20].zzzz))).w;
    // 51: lt r1.x, r0.z, l(0.000001)
    r1.x = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 52: mul r0.z, r0.z, cb0[20].z
    r0.z = ((r0.zzzz)*(source[20].zzzz)).z;
    // 53: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 54: mov_sat r1.x, r0.z
    r1.x = (saturate(r0.zzzz)).x;
    // 55: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 56: mul r0.x, r0.x, cb0[21].x
    r0.x = ((r0.xxxx)*(source[21].xxxx)).x;
    // 57: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 58: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 59: add r0.x, r0.w, r1.x
    r0.x = ((r0.wwww)+(r1.xxxx)).x;
    // 60: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 61: mad r0.xyz, r0.xxxx, cb0[10].xyzx, r0.yyyy
    r0.xyz = ((r0.xxxx)*(source[10].xyzx)+(r0.yyyy)).xyz;
    // 62: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 63: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_circ_01_01_dt_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 ArtistNative3593(ARTIST_NATIVE_INPUT input)
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
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
// fx_c_pa_ring_06_ad: e91186b92721ed429a45977dbd4c5e7e; selected map 1994ac3e07582e978a16e8fdfc9b15127be2206134c65dde6db4fc3232563321.
float4 ArtistNative3594(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[0u];
    source[2].x = (((float4(1.70000005, 0.0, 0.0, 0.0)+sin((g_ArtistSourceMaterialTime.xxxx*float4(20.9439507, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))).x;
    source[2].y = ((((float4(1.70000005, 0.0, 0.0, 0.0)+sin((g_ArtistSourceMaterialTime.xxxx*float4(20.9439507, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))*((float4(1.70000005, 0.0, 0.0, 0.0)+cos((g_ArtistSourceMaterialTime.xxxx*float4(8.9759798, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0)))).x;
    source[2].z = (clamp((((float4(1.70000005, 0.0, 0.0, 0.0)+sin((g_ArtistSourceMaterialTime.xxxx*float4(20.9439507, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))*((float4(1.70000005, 0.0, 0.0, 0.0)+cos((g_ArtistSourceMaterialTime.xxxx*float4(8.9759798, 0.0, 0.0, 0.0))))*float4(0.600000024, 0.0, 0.0, 0.0))),float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
// fx_a_pa_gl_01_6_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ArtistNative3595(ARTIST_NATIVE_INPUT input)
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
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
// fx_j_rgbsplit_01_2_ad: d880c361f16370468cfdac8d4688d393; selected map e84de71c52a818919646772cd3b0a0181fe0758eaae2cdb6c89bc69d21826863.
float4 ArtistNative3596(ARTIST_NATIVE_INPUT input)
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
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_r_me_shine_05_01_ad: 220a27d436d0c149ae50a935ac278157; selected map 0b5b336819872cbee7f1bd00cd0d4982adbc8147ff99bf8739fa91f66a2b9a4f.
float4 ArtistNative3597(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[6u];
    source[3] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ArtistSourceMaterialParameters[5u];
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[7].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[11].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: add r0.zw, -r0.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 6: mul_sat r0.z, r0.x, r0.z
    r0.z = (saturate((r0.xxxx)*(r0.zzzz))).z;
    // 7: mul_sat r0.w, r0.w, cb0[10].y
    r0.w = (saturate((r0.wwww)*(source[10].yyyy))).w;
    // 8: mul r1.x, r0.z, l(4.000000)
    r1.x = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 9: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 10: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 11: mul r1.x, r1.x, cb0[9].x
    r1.x = ((r1.xxxx)*(source[9].xxxx)).x;
    // 12: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 13: mul r1.x, r1.x, cb0[9].y
    r1.x = ((r1.xxxx)*(source[9].yyyy)).x;
    // 14: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 15: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 16: mul r1.x, r1.x, cb0[9].z
    r1.x = ((r1.xxxx)*(source[9].zzzz)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[9].w
    r1.x = ((r1.xxxx)*(source[9].wwww)).x;
    // 19: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 20: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 21: mul_sat r0.z, r0.z, r1.x
    r0.z = (saturate((r0.zzzz)*(r1.xxxx))).z;
    // 22: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 23: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 24: source device depth mapped to centimetre view depth; reconstruction at 26.
    r1.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 26-29: reconstructed view depth is supplied by the runtime adapter.
    r1.x = r1.x;
    // 30: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 31: add r1.y, -cb0[11].y, l(1.000000)
    r1.y = ((-(source[11].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: mul r1.y, r1.y, l(100.000000)
    r1.y = ((r1.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 33: max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 34: div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // 35: log r1.y, r0.w
    r1.y = (log2(r0.wwww)).y;
    // 36: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 37: mul r1.y, r1.y, cb0[10].z
    r1.y = ((r1.yyyy)*(source[10].zzzz)).y;
    // 38: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 39: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 40: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 41: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 42: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 43: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 44: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 45: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 46: mul r1.y, r1.y, cb0[10].x
    r1.y = ((r1.yyyy)*(source[10].xxxx)).y;
    // 47: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 48: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 49: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 50: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 51: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 52: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 53: mul r1.xy, r0.xyxx, cb0[6].zwzz
    r1.xy = ((r0.xyxx)*(source[6].zwzz)).xy;
    // 54: mul r0.xy, r0.xyxx, cb0[8].xyxx
    r0.xy = ((r0.xyxx)*(source[8].xyxx)).xy;
    // 55: mad r2.x, cb0[6].y, cb0[6].x, r1.x
    r2.x = ((source[6].yyyy)*(source[6].xxxx)+(r1.xxxx)).x;
    // 56: mad r2.y, cb0[6].y, cb0[7].z, r1.y
    r2.y = ((source[6].yyyy)*(source[7].zzzz)+(r1.yyyy)).y;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 58: mad r2.x, cb0[6].y, cb0[7].w, r0.x
    r2.x = ((source[6].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 59: mad r2.y, cb0[6].y, cb0[8].z, r0.y
    r2.y = ((source[6].yyyy)*(source[8].zzzz)+(r0.yyyy)).y;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r2.xyxx, t2.xywz, s2, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 61: mul r2.xyz, r0.xywx, r1.xyzx
    r2.xyz = ((r0.xywx)*(r1.xyzx)).xyz;
    // 62: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 63: mad r0.xyw, -r1.xyxz, r0.xyxw, r1.wwww
    r0.xyw = ((-(r1.xyxz))*(r0.xyxw)+(r1.wwww)).xyw;
    // 64: mad r0.xyw, cb0[8].wwww, r0.xyxw, r2.xyxz
    r0.xyw = ((source[8].wwww)*(r0.xyxw)+(r2.xyxz)).xyw;
    // 65: mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 66: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 67: mad r0.xyw, r0.xyxw, cb0[1].xyxz, cb0[2].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)+(source[2].xyxz)).xyw;
    // 68: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 69: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 70: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative3598(ARTIST_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[2u];
    source[2] = g_ArtistSourceMaterialParameters[1u];
    source[3].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
// fx_o_de_condcircle_02_01_tr: ef9cad5cf42011438c3b2ff2ecf7baee; selected map d5372f699c5a6070975eda6e7a485a28046062641624495e35efd79dcd7962f3.
float4 ArtistNative3600(ARTIST_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source world position: neutral pre-view translation.
    source[1]=float4(input.decalProjection.xy,0.f,0.f);
    source[2]=input.color; // Original GroundEffect ActiveColorValue.
    source[3].x=input.decalProjection.z;
    source[4] = g_ArtistSourceMaterialParameters[2u];
    source[5] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[9] = g_ArtistSourceMaterialParameters[1u];
    source[10].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[10].y = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)).x;
    source[10].z = (((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[10].w = (ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[11].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[11].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].yyyy)))).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
    // 3: add r1.x, v4.w, -cb0[1].x
    r1.x = ((v4.wwww)+(-(source[1].xxxx))).x;
    // 4: add r0.z, r1.x, l(0.001000)
    r0.z = ((r1.xxxx)+(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 5: add r1.x, -v4.w, cb0[1].y
    r1.x = ((-(v4.wwww))+(source[1].yyyy)).x;
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
    // 11: mov r0.xw, l(0,0,0,0.010000)
    r0.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),0.010000)).xw;
    // 12: mov r0.yz, cb0[5].yyxy
    r0.yz = (source[5].yyxy).yz;
    // 13: add r1.xy, v7.xyxx, cb0[0].xyxx
    r1.xy = ((v7.xyxx)+(source[0].xyxx)).xy;
    // 14: mad r0.xy, r1.xyxx, l(0.002000, 0.002000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.xyxx)*(float4(0.002000,0.002000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 15: add r0.xy, r0.xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)+(r0.zwzz)).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 17: mad r0.yz, r1.xxyx, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[6].xxyx
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[6].xxyx)).yz;
    // 18: add r1.zw, r0.yyyz, l(0.000000, 0.000000, 0.418100, 0.364800)
    r1.zw = ((r0.yyyz)+(float4(0.000000,0.000000,0.418100,0.364800))).zw;
    // 19: add r2.xyzw, r0.yzyz, l(0.428100, 0.354800, 0.418100, 0.354800)
    r2.xyzw = ((r0.yzyz)+(float4(0.428100,0.354800,0.418100,0.354800))).xyzw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 21: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 22: mad r0.yz, r1.xxyx, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[7].xxyx
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[7].xxyx)).yz;
    // 23: add r1.zw, r0.yyyz, l(0.000000, 0.000000, 0.864861, 0.158384)
    r1.zw = ((r0.yyyz)+(float4(0.000000,0.000000,0.864861,0.158384))).zw;
    // 24: add r3.xyzw, r0.yzyz, l(0.874861, 0.148384, 0.864861, 0.148384)
    r3.xyzw = ((r0.yzyz)+(float4(0.874861,0.148384,0.864861,0.148384))).xyzw;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 26: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.002000, 0.002000), cb0[8].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.002000,0.002000))+(source[8].xxxy)).zw;
    // 27: add r1.zw, r0.zzzw, l(0.000000, 0.000000, 0.651340, 0.761638)
    r1.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.651340,0.761638))).zw;
    // 28: add r4.xyzw, r0.zwzw, l(0.661340, 0.751638, 0.651340, 0.751638)
    r4.xyzw = ((r0.zwzw)+(float4(0.661340,0.751638,0.651340,0.751638))).xyzw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 30: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 31: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 32: mad r0.yz, r1.xxyx, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[5].xxyx
    r0.yz = ((r1.xxyx)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[5].xxyx)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r3.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r3.xyxx, t0.yzxw, s0, l(0.000000)
    r1.z = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.zwzz, t0.yzwx, s0, l(0.000000)
    r1.w = (ArtistNativeSample0((r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.x = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 41: add r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)+(r2.xxxx)).z;
    // 42: add r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)+(r1.wwww)).z;
    // 43: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 44: mul r0.y, r0.y, l(0.250000)
    r0.y = ((r0.yyyy)*(float4(0.250000,0.250000,0.250000,0.250000))).y;
    // 45: mad r2.y, r0.x, l(0.250000), -r0.y
    r2.y = ((r0.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))+(-(r0.yyyy))).y;
    // 46: mov r3.xw, cb0[5].xxxy
    r3.xw = (source[5].xxxy).xw;
    // 47: mov r3.yz, l(0,0,0.010000,0)
    r3.yz = (float4(asfloat(0u),asfloat(0u),0.010000,asfloat(0u))).yz;
    // 48: mad r0.xz, r1.xxyx, l(0.002000, 0.000000, 0.002000, 0.000000), r3.xxyx
    r0.xz = ((r1.xxyx)*(float4(0.002000,0.000000,0.002000,0.000000))+(r3.xxyx)).xz;
    // 49: add r0.xz, r0.xxzx, r3.zzwz
    r0.xz = ((r0.xxzx)+(r3.zzwz)).xz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: add r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)+(r0.xxxx)).x;
    // 52: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 53: mad r2.x, r0.x, l(0.250000), -r0.y
    r2.x = ((r0.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))+(-(r0.yyyy))).x;
    // 54: mul r0.xyzw, r1.xyxy, l(0.003100, 0.003100, 0.007000, 0.007000)
    r0.xyzw = ((r1.xyxy)*(float4(0.003100,0.003100,0.007000,0.007000))).xyzw;
    // 55: mad r0.y, cb0[10].x, l(0.090000), r0.y
    r0.y = ((source[10].xxxx)*(float4(0.090000,0.090000,0.090000,0.090000))+(r0.yyyy)).y;
    // 56: mad r0.xy, r2.xyxx, l(-0.360000, -0.360000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r2.xyxx)*(float4(-0.360000,-0.360000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 58: mad r0.z, cb0[10].x, l(0.090000), r0.z
    r0.z = ((source[10].xxxx)*(float4(0.090000,0.090000,0.090000,0.090000))+(r0.zzzz)).z;
    // 59: mad r0.xy, r2.xyxx, l(-0.360000, -0.360000, 0.000000, 0.000000), r0.zwzz
    r0.xy = ((r2.xyxx)*(float4(-0.360000,-0.360000,0.000000,0.000000))+(r0.zwzz)).xy;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 61: add r0.xyzw, r0.xxyz, r1.xxyz
    r0.xyzw = ((r0.xxyz)+(r1.xxyz)).xyzw;
    // 62: mul r0.yzw, r0.yyzw, r0.yyzw
    r0.yzw = ((r0.yyzw)*(r0.yyzw)).yzw;
    // 63: dp2 r0.x, r0.xxxx, r0.xxxx
    r0.x = (dot((r0.xxxx).xy,(r0.xxxx).xy).xxxx).x;
    // 64: mad r0.x, r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)+(r0.xxxx)).x;
    // 65: add r0.yzw, r0.yyzw, r0.yyzw
    r0.yzw = ((r0.yyzw)+(r0.yyzw)).yzw;
    // 66: mad r0.yzw, r0.yyzw, r0.yyzw, r0.yyzw
    r0.yzw = ((r0.yyzw)*(r0.yyzw)+(r0.yyzw)).yzw;
    // 67: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 68: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 69: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 70: mad_sat r1.y, -r1.x, cb0[11].y, l(1.000000)
    r1.y = (saturate((-(r1.xxxx))*(source[11].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).y;
    // 71: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 72: mul r1.yw, r1.yyyy, l(0.000000, 10.000000, 0.000000, 300.000000)
    r1.yw = ((r1.yyyy)*(float4(0.000000,10.000000,0.000000,300.000000))).yw;
    // 73: min r1.yw, r1.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r1.yw = (min(r1.yyyw,float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 74: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 75: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 76: mul r2.xyz, r0.yzwy, r1.zzzz
    r2.xyz = ((r0.yzwy)*(r1.zzzz)).xyz;
    // 77: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 78: mul r0.xyzw, r0.xyzw, r0.xyzw
    r0.xyzw = ((r0.xyzw)*(r0.xyzw)).xyzw;
    // 79: mul r0.x, r0.x, l(10.000000)
    r0.x = ((r0.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 80: mul r2.w, r1.w, l(0.150000)
    r2.w = ((r1.wwww)*(float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 81: mad r2.xyz, r2.xyzx, l(0.100000, 0.100000, 0.100000, 0.000000), r2.wwww
    r2.xyz = ((r2.xyzx)*(float4(0.100000,0.100000,0.100000,0.000000))+(r2.wwww)).xyz;
    // 82: mad r1.z, r1.z, l(0.100000), r2.w
    r1.z = ((r1.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))+(r2.wwww)).z;
    // 83: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 84: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 85: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 86: mul r0.xyzw, r0.xyzw, r1.yyyy
    r0.xyzw = ((r0.xyzw)*(r1.yyyy)).xyzw;
    // 87: mad r0.x, r0.x, l(0.300000), r1.z
    r0.x = ((r0.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.zzzz)).x;
    // 88: mad r0.yzw, r0.yyzw, l(0.000000, 3.000000, 3.000000, 3.000000), r2.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,3.000000,3.000000,3.000000))+(r2.xxyz)).yzw;
    // 89: add r0.yzw, r0.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r0.yzw = ((r0.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 90: lt r1.y, l(0.000000), cb0[11].z
    r1.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(source[11].zzzz)) * 0xffffffffu)).y;
    // 91: movc r1.y, r1.y, cb0[11].z, l(1.000000)
    r1.y = ((asuint(r1.yyyy) != 0u) ? (source[11].zzzz) : (float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 92: ge r1.z, cb0[11].z, l(0.000000)
    r1.z = (asfloat((uint4)((source[11].zzzz)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 93: movc r1.y, r1.z, r1.y, cb0[11].z
    r1.y = ((asuint(r1.zzzz) != 0u) ? (r1.yyyy) : (source[11].zzzz)).y;
    // 94: div r1.y, l(0.003000), r1.y
    r1.y = ((float4(0.003000,0.003000,0.003000,0.003000))/(r1.yyyy)).y;
    // 95: add r1.y, -r1.y, l(0.500000)
    r1.y = ((-(r1.yyyy))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 96: max r1.y, r1.y, l(0.000010)
    r1.y = (max(r1.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 97: div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r1.y
    r1.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.yyyy)).y;
    // 98: mad r1.y, -r1.x, r1.y, l(1.000000)
    r1.y = ((-(r1.xxxx))*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 99: mad r1.x, -r1.x, l(2.000000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: mul_sat r1.yz, r1.yyxy, l(0.000000, 100000.000000, 100000.000000, 0.000000)
    r1.yz = (saturate((r1.yyxy)*(float4(0.000000,100000.000000,100000.000000,0.000000)))).yz;
    // 101: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 102: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 103: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 104: mad r0.yzw, r1.yyyy, l(0.000000, 5.000000, 5.000000, 5.000000), r0.yyzw
    r0.yzw = ((r1.yyyy)*(float4(0.000000,5.000000,5.000000,5.000000))+(r0.yyzw)).yzw;
    // 105: mul r1.yzw, cb0[2].xxyz, cb0[9].xxyz
    r1.yzw = ((source[2].xxyz)*(source[9].xxyz)).yzw;
    // 106: mad r0.yzw, r0.yyzw, r1.yyzw, cb0[4].xxyz
    r0.yzw = ((r0.yyzw)*(r1.yyzw)+(source[4].xxyz)).yzw;
    // 107: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 108: mul r0.y, r1.x, l(300.000000)
    r0.y = ((r1.xxxx)*(float4(300.000000,300.000000,300.000000,300.000000))).y;
    // 109: add r0.z, -r1.x, l(1.000000)
    r0.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 110: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 111: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 112: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 113: mul r0.w, r0.w, l(30.000000)
    r0.w = ((r0.wwww)*(float4(30.000000,30.000000,30.000000,30.000000))).w;
    // 114: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 115: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 116: mul r0.y, r0.y, l(0.200000)
    r0.y = ((r0.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 117: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 118: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 119: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 120: mul_sat r0.x, r0.x, cb0[11].w
    r0.x = (saturate((r0.xxxx)*(source[11].wwww))).x;
    // 121: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 122: add r0.y, -|v4.w|, cb0[1].y
    r0.y = ((-(abs(v4.wwww)))+(source[1].yyyy)).y;
    // 123: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 124: div_sat r0.y, r0.y, cb0[1].y
    r0.y = (saturate((r0.yyyy)/(source[1].yyyy))).y;
    // 125: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 126: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
// fx_o_de_condmondonut_02_01_tr: b123a95ca0a96e488268a58a2998fa43; selected map 199f3bfd7f0ea2dd0e0b415a95cf5634c2d386158bac72197ccecf05f98d35de.
float4 ArtistNative3601(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source world position: neutral pre-view translation.
    source[1]=float4(input.decalProjection.xy,0.f,0.f);
    source[2]=input.color; // Original GroundEffect ActiveColorValue.
    source[3].x=input.decalProjection.z;
    source[4] = g_ArtistSourceMaterialParameters[3u];
    source[5] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[9] = g_ArtistSourceMaterialParameters[2u];
    source[10].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[10].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)).x;
    source[10].w = (((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[11].x = ((float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[11].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[11].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)))).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[12].y = ((g_ArtistSourceMaterialParameters[0u].xxxx+float4(0.00249999994, 0.0, 0.0, 0.0))).x;
    source[12].z = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ArtistSourceMaterialParameters[0u].xxxx+float4(0.00249999994, 0.0, 0.0, 0.0)))).x;
    source[12].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
    // 3: add r1.x, v4.w, -cb0[1].x
    r1.x = ((v4.wwww)+(-(source[1].xxxx))).x;
    // 4: add r0.z, r1.x, l(0.001000)
    r0.z = ((r1.xxxx)+(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 5: add r1.x, -v4.w, cb0[1].y
    r1.x = ((-(v4.wwww))+(source[1].yyyy)).x;
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
    // 12: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 13: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 14: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 15: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 16: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 17: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 18: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 19: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 20: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 21: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 22: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 23: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 24: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 25: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 26: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 27: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 28: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 29: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 30: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 31: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 32: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 33: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 34: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 35: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 36: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 37: mul r0.y, r0.y, l(0.159155)
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))).y;
    // 38: mad r0.y, -|r0.y|, l(2.000000), l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: add r0.z, r0.y, -cb0[12].z
    r0.z = ((r0.yyyy)+(-(source[12].zzzz))).z;
    // 40: mul_sat r0.zw, r0.zzzz, l(0.000000, 0.000000, 10.000000, 2000.000000)
    r0.zw = (saturate((r0.zzzz)*(float4(0.000000,0.000000,10.000000,2000.000000)))).zw;
    // 41: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 43: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 44: mul r1.x, r0.z, r0.z
    r1.x = ((r0.zzzz)*(r0.zzzz)).x;
    // 45: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 47: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 48: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 49: mul r1.xy, cb0[10].xxxx, l(0.500000, 0.550000, 0.000000, 0.000000)
    r1.xy = ((source[10].xxxx)*(float4(0.500000,0.550000,0.000000,0.000000))).xy;
    // 50: max r1.xy, r1.yxyy, l(0.000010, 0.000010, 0.000000, 0.000000)
    r1.xy = (max(r1.yxyy,float4(0.000010,0.000010,0.000000,0.000000))).xy;
    // 51: div r1.xy, l(1.000000, 1.000000, 1.000000, 1.000000), r1.xyxx
    r1.xy = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xyxx)).xy;
    // 52: mad r1.xy, -r0.xxxx, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r0.xxxx))*(r1.xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 53: mul_sat r1.xy, r1.xyxx, l(4.000000, 100000.000000, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xyxx)*(float4(4.000000,100000.000000,0.000000,0.000000)))).xy;
    // 54: lt r1.z, l(0.000000), cb0[10].x
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(source[10].xxxx)) * 0xffffffffu)).z;
    // 55: and r1.w, r1.y, r1.z
    r1.w = (asfloat(asuint(r1.yyyy) & asuint(r1.zzzz))).w;
    // 56: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 57: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 58: mul r1.x, r1.x, l(2.500000)
    r1.x = ((r1.xxxx)*(float4(2.500000,2.500000,2.500000,2.500000))).x;
    // 59: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 60: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 61: and r1.x, r1.x, r1.z
    r1.x = (asfloat(asuint(r1.xxxx) & asuint(r1.zzzz))).x;
    // 62: ge r2.xy, cb0[10].xxxx, l(0.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((source[10].xxxx)>=(float4(0.000000,1.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 63: and r1.xy, r1.xwxx, r2.xxxx
    r1.xy = (asfloat(asuint(r1.xwxx) & asuint(r2.xxxx))).xy;
    // 64: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: mad r1.w, -r0.x, l(2.000000), l(1.000000)
    r1.w = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: max r2.z, r1.w, l(0.000000)
    r2.z = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 67: mul_sat r1.w, r1.w, l(100000.000000)
    r1.w = (saturate((r1.wwww)*(float4(100000.000000,100000.000000,100000.000000,100000.000000)))).w;
    // 68: mul r2.w, r2.z, l(300.000000)
    r2.w = ((r2.zzzz)*(float4(300.000000,300.000000,300.000000,300.000000))).w;
    // 69: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r3.x, r1.y, r2.w
    r3.x = ((r1.yyyy)*(r2.wwww)).x;
    // 72: mul r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)*(r3.xxxx)).z;
    // 73: log r3.y, r2.z
    r3.y = (log2(r2.zzzz)).y;
    // 74: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 75: mul r3.y, r3.y, l(30.000000)
    r3.y = ((r3.yyyy)*(float4(30.000000,30.000000,30.000000,30.000000))).y;
    // 76: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 77: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 78: movc r2.z, r2.z, l(0), r2.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 79: add r1.x, r1.x, r2.z
    r1.x = ((r1.xxxx)+(r2.zzzz)).x;
    // 80: mul r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 81: mad r0.z, r0.z, l(0.250000), r1.x
    r0.z = ((r0.zzzz)*(float4(0.250000,0.250000,0.250000,0.250000))+(r1.xxxx)).z;
    // 82: ge r2.z, cb0[12].x, l(1.000000)
    r2.z = (asfloat((uint4)((source[12].xxxx)>=(float4(1.000000,1.000000,1.000000,1.000000))) * 0xffffffffu)).z;
    // 83: movc r0.z, r2.z, r1.x, r0.z
    r0.z = ((asuint(r2.zzzz) != 0u) ? (r1.xxxx) : (r0.zzzz)).z;
    // 84: movc r0.z, r2.y, l(0), r0.z
    r0.z = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 85: mov r4.xw, l(0,0,0,0.010000)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),0.010000)).xw;
    // 86: mov r4.yz, cb0[5].yyxy
    r4.yz = (source[5].yyxy).yz;
    // 87: add r2.yz, v7.xxyx, cb0[0].xxyx
    r2.yz = ((v7.xxyx)+(source[0].xxyx)).yz;
    // 88: mad r3.yz, r2.yyzy, l(0.000000, 0.002000, 0.002000, 0.000000), r4.xxyx
    r3.yz = ((r2.yyzy)*(float4(0.000000,0.002000,0.002000,0.000000))+(r4.xxyx)).yz;
    // 89: add r3.yz, r3.yyzy, r4.zzwz
    r3.yz = ((r3.yyzy)+(r4.zzwz)).yz;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r3.yzyy, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r3.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 91: mad r3.yz, r2.yyzy, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[6].xxyx
    r3.yz = ((r2.yyzy)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[6].xxyx)).yz;
    // 92: add r4.xy, r3.yzyy, l(0.418100, 0.364800, 0.000000, 0.000000)
    r4.xy = ((r3.yzyy)+(float4(0.418100,0.364800,0.000000,0.000000))).xy;
    // 93: add r5.xyzw, r3.yzyz, l(0.428100, 0.354800, 0.418100, 0.354800)
    r5.xyzw = ((r3.yzyz)+(float4(0.428100,0.354800,0.418100,0.354800))).xyzw;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r4.xyxx, t0.yzwx, s0, l(0.000000)
    r2.w = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 95: add r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)+(r2.wwww)).x;
    // 96: mad r3.yz, r2.yyzy, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[7].xxyx
    r3.yz = ((r2.yyzy)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[7].xxyx)).yz;
    // 97: add r4.xy, r3.yzyy, l(0.864861, 0.158384, 0.000000, 0.000000)
    r4.xy = ((r3.yzyy)+(float4(0.864861,0.158384,0.000000,0.000000))).xy;
    // 98: add r6.xyzw, r3.yzyz, l(0.874861, 0.148384, 0.864861, 0.148384)
    r6.xyzw = ((r3.yzyz)+(float4(0.874861,0.148384,0.864861,0.148384))).xyzw;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r4.xyxx, t0.yzwx, s0, l(0.000000)
    r2.w = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 100: mad r3.yz, r2.yyzy, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[8].xxyx
    r3.yz = ((r2.yyzy)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[8].xxyx)).yz;
    // 101: add r4.xy, r3.yzyy, l(0.651340, 0.761638, 0.000000, 0.000000)
    r4.xy = ((r3.yzyy)+(float4(0.651340,0.761638,0.000000,0.000000))).xy;
    // 102: add r7.xyzw, r3.yzyz, l(0.661340, 0.751638, 0.651340, 0.751638)
    r7.xyzw = ((r3.yzyz)+(float4(0.661340,0.751638,0.651340,0.751638))).xyzw;
    // 103: sample_b_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s0, l(0.000000)
    r3.y = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 104: add r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)+(r3.yyyy)).w;
    // 105: add r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)+(r2.wwww)).x;
    // 106: mad r3.yz, r2.yyzy, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[5].xxyx
    r3.yz = ((r2.yyzy)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[5].xxyx)).yz;
    // 107: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.yzyy, t0.yzwx, s0, l(0.000000)
    r2.w = (ArtistNativeSample0((r3.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 108: sample_b_indexable(texture2d)(float,float,float,float) r3.y, r5.zwzz, t0.yxzw, s0, l(0.000000)
    r3.y = (ArtistNativeSample0((r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 109: sample_b_indexable(texture2d)(float,float,float,float) r3.z, r5.xyxx, t0.yzxw, s0, l(0.000000)
    r3.z = (ArtistNativeSample0((r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 110: add r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)+(r3.yyyy)).w;
    // 111: sample_b_indexable(texture2d)(float,float,float,float) r3.y, r6.zwzz, t0.yxzw, s0, l(0.000000)
    r3.y = (ArtistNativeSample0((r6.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r3.w, r6.xyxx, t0.yzwx, s0, l(0.000000)
    r3.w = (ArtistNativeSample0((r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 113: sample_b_indexable(texture2d)(float,float,float,float) r4.x, r7.zwzz, t0.xyzw, s0, l(0.000000)
    r4.x = (ArtistNativeSample0((r7.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 114: sample_b_indexable(texture2d)(float,float,float,float) r4.y, r7.xyxx, t0.yxzw, s0, l(0.000000)
    r4.y = (ArtistNativeSample0((r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 115: add r3.yw, r3.yyyw, r4.xxxy
    r3.yw = ((r3.yyyw)+(r4.xxxy)).yw;
    // 116: add r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)+(r3.yyyy)).w;
    // 117: mul r2.w, r2.w, l(0.250000)
    r2.w = ((r2.wwww)*(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 118: mad r4.y, r1.x, l(0.250000), -r2.w
    r4.y = ((r1.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))+(-(r2.wwww))).y;
    // 119: mov r5.xw, cb0[5].xxxy
    r5.xw = (source[5].xxxy).xw;
    // 120: mov r5.yz, l(0,0,0.010000,0)
    r5.yz = (float4(asfloat(0u),asfloat(0u),0.010000,asfloat(0u))).yz;
    // 121: mad r4.zw, r2.yyyz, l(0.000000, 0.000000, 0.002000, 0.002000), r5.xxxy
    r4.zw = ((r2.yyyz)*(float4(0.000000,0.000000,0.002000,0.002000))+(r5.xxxy)).zw;
    // 122: add r4.zw, r4.zzzw, r5.zzzw
    r4.zw = ((r4.zzzw)+(r5.zzzw)).zw;
    // 123: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r4.zwzz, t0.xyzw, s0, l(0.000000)
    r1.x = (ArtistNativeSample0((r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 124: add r1.x, r3.z, r1.x
    r1.x = ((r3.zzzz)+(r1.xxxx)).x;
    // 125: add r1.x, r3.w, r1.x
    r1.x = ((r3.wwww)+(r1.xxxx)).x;
    // 126: mad r4.x, r1.x, l(0.250000), -r2.w
    r4.x = ((r1.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))+(-(r2.wwww))).x;
    // 127: mul r3.yz, r2.yyzy, l(0.000000, 0.003100, 0.003100, 0.000000)
    r3.yz = ((r2.yyzy)*(float4(0.000000,0.003100,0.003100,0.000000))).yz;
    // 128: mul r2.yz, r2.yyzy, l(0.000000, 0.007000, 0.007000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,0.007000,0.007000,0.000000))).yz;
    // 129: mad r3.z, cb0[10].y, l(0.090000), r3.z
    r3.z = ((source[10].yyyy)*(float4(0.090000,0.090000,0.090000,0.090000))+(r3.zzzz)).z;
    // 130: mad r3.yz, r4.xxyx, l(0.000000, -0.360000, -0.360000, 0.000000), r3.yyzy
    r3.yz = ((r4.xxyx)*(float4(0.000000,-0.360000,-0.360000,0.000000))+(r3.yyzy)).yz;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r3.yzw, r3.yzyy, t1.wxyz, s1, l(0.000000)
    r3.yzw = (ArtistNativeSample1((r3.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 132: mad r2.y, cb0[10].y, l(0.090000), r2.y
    r2.y = ((source[10].yyyy)*(float4(0.090000,0.090000,0.090000,0.090000))+(r2.yyyy)).y;
    // 133: mad r2.yz, r4.xxyx, l(0.000000, -0.360000, -0.360000, 0.000000), r2.yyzy
    r2.yz = ((r4.xxyx)*(float4(0.000000,-0.360000,-0.360000,0.000000))+(r2.yyzy)).yz;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, r2.yzyy, t1.wxyz, s1, l(0.000000)
    r2.yzw = (ArtistNativeSample1((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 135: add r4.xyzw, r2.yyzw, r3.yyzw
    r4.xyzw = ((r2.yyzw)+(r3.yyzw)).xyzw;
    // 136: dp2 r1.x, r4.xxxx, r4.xxxx
    r1.x = (dot((r4.xxxx).xy,(r4.xxxx).xy).xxxx).x;
    // 137: mul r2.yzw, r4.yyzw, r4.yyzw
    r2.yzw = ((r4.yyzw)*(r4.yyzw)).yzw;
    // 138: add r2.yzw, r2.yyzw, r2.yyzw
    r2.yzw = ((r2.yyzw)+(r2.yyzw)).yzw;
    // 139: mad r2.yzw, r2.yyzw, r2.yyzw, r2.yyzw
    r2.yzw = ((r2.yyzw)*(r2.yyzw)+(r2.yyzw)).yzw;
    // 140: mad r1.x, r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)+(r1.xxxx)).x;
    // 141: mad_sat r3.y, -r0.x, cb0[11].z, l(1.000000)
    r3.y = (saturate((-(r0.xxxx))*(source[11].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000)))).y;
    // 142: mul r3.z, r3.y, l(300.000000)
    r3.z = ((r3.yyyy)*(float4(300.000000,300.000000,300.000000,300.000000))).z;
    // 143: add r3.yw, -r3.yyyy, l(0.000000, 1.000000, 0.000000, 0.020000)
    r3.yw = ((-(r3.yyyy))+(float4(0.000000,1.000000,0.000000,0.020000))).yw;
    // 144: min r3.z, r3.z, l(1.000000)
    r3.z = (min(r3.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 145: mul r4.x, r1.y, r3.z
    r4.x = ((r1.yyyy)*(r3.zzzz)).x;
    // 146: mul r3.w, r3.w, l(10.000000)
    r3.w = ((r3.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 147: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 148: mul r3.yz, r3.yyzy, r3.yywy
    r3.yz = ((r3.yyzy)*(r3.yywy)).yz;
    // 149: mul r3.y, r4.x, r3.y
    r3.y = ((r4.xxxx)*(r3.yyyy)).y;
    // 150: mul r3.w, r4.x, l(0.150000)
    r3.w = ((r4.xxxx)*(float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 151: mul r3.y, r3.y, r3.y
    r3.y = ((r3.yyyy)*(r3.yyyy)).y;
    // 152: mul r4.x, r1.x, r3.y
    r4.x = ((r1.xxxx)*(r3.yyyy)).x;
    // 153: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 154: mul r1.x, r1.x, l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 155: mul r1.x, r1.x, r3.z
    r1.x = ((r1.xxxx)*(r3.zzzz)).x;
    // 156: mul r1.x, r1.x, r3.x
    r1.x = ((r1.xxxx)*(r3.xxxx)).x;
    // 157: mul r4.yzw, r2.yyzw, r3.yyyy
    r4.yzw = ((r2.yyzw)*(r3.yyyy)).yzw;
    // 158: mul r2.yzw, r2.yyzw, r2.yyzw
    r2.yzw = ((r2.yyzw)*(r2.yyzw)).yzw;
    // 159: mul r2.yzw, r2.yyzw, r3.zzzz
    r2.yzw = ((r2.yyzw)*(r3.zzzz)).yzw;
    // 160: mul r2.yzw, r2.yyzw, r3.xxxx
    r2.yzw = ((r2.yyzw)*(r3.xxxx)).yzw;
    // 161: mad r3.xyz, r4.yzwy, l(0.100000, 0.100000, 0.100000, 0.000000), r3.wwww
    r3.xyz = ((r4.yzwy)*(float4(0.100000,0.100000,0.100000,0.000000))+(r3.wwww)).xyz;
    // 162: mad r3.w, r4.x, l(0.100000), r3.w
    r3.w = ((r4.xxxx)*(float4(0.100000,0.100000,0.100000,0.100000))+(r3.wwww)).w;
    // 163: mad r1.x, r1.x, l(0.800000), r3.w
    r1.x = ((r1.xxxx)*(float4(0.800000,0.800000,0.800000,0.800000))+(r3.wwww)).x;
    // 164: add r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)+(r1.xxxx)).z;
    // 165: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 166: mul r0.z, r0.z, cb0[2].w
    r0.z = ((r0.zzzz)*(source[2].wwww)).z;
    // 167: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 168: mul r0.z, r0.z, cb0[3].x
    r0.z = ((r0.zzzz)*(source[3].xxxx)).z;
    // 169: mad r2.yzw, r2.yyzw, l(0.000000, 8.000000, 8.000000, 8.000000), r3.xxyz
    r2.yzw = ((r2.yyzw)*(float4(0.000000,8.000000,8.000000,8.000000))+(r3.xxyz)).yzw;
    // 170: add r2.yzw, r2.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r2.yzw = ((r2.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 171: add r1.x, -|v4.w|, cb0[1].y
    r1.x = ((-(abs(v4.wwww)))+(source[1].yyyy)).x;
    // 172: mul r1.x, r1.x, l(5.000000)
    r1.x = ((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 173: div_sat r1.x, r1.x, cb0[1].y
    r1.x = (saturate((r1.xxxx)/(source[1].yyyy))).x;
    // 174: mul r1.x, r1.x, v4.z
    r1.x = ((r1.xxxx)*(v4.zzzz)).x;
    // 175: mul o0.w, r0.z, r1.x
    output.w = ((r0.zzzz)*(r1.xxxx)).w;
    // 176: lt r0.z, l(0.000000), cb0[11].w
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(source[11].wwww)) * 0xffffffffu)).z;
    // 177: movc r0.z, r0.z, cb0[11].w, l(1.000000)
    r0.z = ((asuint(r0.zzzz) != 0u) ? (source[11].wwww) : (float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 178: ge r1.x, cb0[11].w, l(0.000000)
    r1.x = (asfloat((uint4)((source[11].wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 179: movc r0.z, r1.x, r0.z, cb0[11].w
    r0.z = ((asuint(r1.xxxx) != 0u) ? (r0.zzzz) : (source[11].wwww)).z;
    // 180: div r3.xy, l(0.003000, 0.001500, 0.000000, 0.000000), r0.zzzz
    r3.xy = ((float4(0.003000,0.001500,0.000000,0.000000))/(r0.zzzz)).xy;
    // 181: mad r0.z, cb0[10].x, l(0.500000), r3.y
    r0.z = ((source[10].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r3.yyyy)).z;
    // 182: max r0.z, r0.z, l(0.000010)
    r0.z = (max(r0.zzzz,float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 183: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 184: mad r0.z, -r0.x, r0.z, l(1.000000)
    r0.z = ((-(r0.xxxx))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 185: mul_sat r0.z, r0.z, l(100000.000000)
    r0.z = (saturate((r0.zzzz)*(float4(100000.000000,100000.000000,100000.000000,100000.000000)))).z;
    // 186: and r0.z, r0.z, r1.z
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r1.zzzz))).z;
    // 187: and r0.z, r0.z, r2.x
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r2.xxxx))).z;
    // 188: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 189: add r1.x, -r3.x, l(0.500000)
    r1.x = ((-(r3.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 190: add r1.y, -r3.y, cb0[12].x
    r1.y = ((-(r3.yyyy))+(source[12].xxxx)).y;
    // 191: add r1.y, r1.y, l(0.002500)
    r1.y = ((r1.yyyy)+(float4(0.002500,0.002500,0.002500,0.002500))).y;
    // 192: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 193: add r0.y, r0.y, -r1.y
    r0.y = ((r0.yyyy)+(-(r1.yyyy))).y;
    // 194: mul_sat r0.y, r0.y, l(2000.000000)
    r0.y = (saturate((r0.yyyy)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).y;
    // 195: add r0.y, -r0.y, r0.w
    r0.y = ((-(r0.yyyy))+(r0.wwww)).y;
    // 196: max r0.w, r1.x, l(0.000010)
    r0.w = (max(r1.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 197: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 198: mad r0.x, -r0.x, r0.w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 199: mul_sat r0.x, r0.x, l(100000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(100000.000000,100000.000000,100000.000000,100000.000000)))).x;
    // 200: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 201: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 202: mul r0.x, r0.x, l(5.000000)
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 203: mad r0.x, r0.z, l(5.000000), r0.x
    r0.x = ((r0.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.xxxx)).x;
    // 204: mad r0.x, r0.y, l(5.000000), r0.x
    r0.x = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.xxxx)).x;
    // 205: add r0.xyz, r0.xxxx, r2.yzwy
    r0.xyz = ((r0.xxxx)+(r2.yzwy)).xyz;
    // 206: mul r1.xyz, cb0[2].xyzx, cb0[9].xyzx
    r1.xyz = ((source[2].xyzx)*(source[9].xyzx)).xyz;
    // 207: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[4].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[4].xyzx)).xyz;
    // 208: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
// fx_o_de_condmonfan_02_01_tr: 76df7d394e7b4242b700d26cf69db77b; selected map 75a71a8578c7bb07083149b2a812c9496cb999d4462d666e3bd6deb1b33a7a42.
float4 ArtistNative3602(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source world position: neutral pre-view translation.
    source[1]=float4(input.decalProjection.xy,0.f,0.f);
    source[2]=input.color; // Original GroundEffect ActiveColorValue.
    source[3].x=input.decalProjection.z;
    source[4] = g_ArtistSourceMaterialParameters[2u];
    source[5] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[9] = g_ArtistSourceMaterialParameters[1u];
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))),1u);
    source[11] = ArtistNativeAppend(cos(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(3.1400001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(3.1400001, 0.0, 0.0, 0.0)))),1u);
    source[12] = ArtistNativeAppend(sin(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(3.1400001, 0.0, 0.0, 0.0))),cos(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(3.1400001, 0.0, 0.0, 0.0))),1u);
    source[13] = ArtistNativeAppend(cos(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(-3.1400001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(-3.1400001, 0.0, 0.0, 0.0)))),1u);
    source[14] = ArtistNativeAppend(sin(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(-3.1400001, 0.0, 0.0, 0.0))),cos(((float4(0.5, 0.0, 0.0, 0.0)+g_ArtistSourceMaterialParameters[0u].xxxx)*float4(-3.1400001, 0.0, 0.0, 0.0))),1u);
    source[15] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].xxxx*float4(-3.1400001, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].xxxx*float4(-3.1400001, 0.0, 0.0, 0.0))),1u);
    source[16].x = (g_ArtistSourceMaterialTime.xxxx).x;
    source[16].y = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)).x;
    source[16].z = (((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[16].w = (ArtistNativePeriodic(((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[17].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[17].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(float4(0.5, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].zzzz)))).x;
    source[17].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[18].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[18].y = ((g_ArtistSourceMaterialParameters[0u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))).x;
    source[18].z = (sin((g_ArtistSourceMaterialParameters[0u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0)))).x;
    source[18].w = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))))).x;
    source[19].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].xxxx*float4(-3.1400001, 0.0, 0.0, 0.0))))).x;
    source[19].y = (cos((g_ArtistSourceMaterialParameters[0u].xxxx*float4(-3.1400001, 0.0, 0.0, 0.0)))).x;
    source[19].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
    // 3: add r1.x, v4.w, -cb0[1].x
    r1.x = ((v4.wwww)+(-(source[1].xxxx))).x;
    // 4: add r0.z, r1.x, l(0.001000)
    r0.z = ((r1.xxxx)+(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 5: add r1.x, -v4.w, cb0[1].y
    r1.x = ((-(v4.wwww))+(source[1].yyyy)).x;
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
    // 12: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 13: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 14: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 15: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 16: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 17: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 18: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 19: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 20: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 21: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 22: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 23: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 24: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 25: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 26: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 27: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 28: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 29: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 30: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 31: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 32: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 33: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 34: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 35: mul r0.w, r0.z, l(0.159155)
    r0.w = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 36: mad r0.z, r0.z, l(0.159155), l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 37: add r0.z, -r0.z, l(0.500000)
    r0.z = ((-(r0.zzzz))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 38: ge r1.x, r0.z, l(0.000000)
    r1.x = (asfloat((uint4)((r0.zzzz)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 39: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 40: movc r0.z, r1.x, r0.z, r0.w
    r0.z = ((asuint(r1.xxxx) != 0u) ? (r0.zzzz) : (r0.wwww)).z;
    // 41: mad_sat r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = (saturate((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000)))).z;
    // 42: add r0.w, r0.z, -cb0[18].x
    r0.w = ((r0.zzzz)+(-(source[18].xxxx))).w;
    // 43: mul_sat r0.w, r0.w, l(2000.000000)
    r0.w = (saturate((r0.wwww)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).w;
    // 44: dp2 r1.x, r0.xyxx, r0.xyxx
    r1.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 45: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 46: mad r1.y, -r1.x, l(2.000000), l(1.000000)
    r1.y = ((-(r1.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 47: max r1.z, r1.y, l(0.000000)
    r1.z = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 48: mul_sat r1.y, r1.y, l(100000.000000)
    r1.y = (saturate((r1.yyyy)*(float4(100000.000000,100000.000000,100000.000000,100000.000000)))).y;
    // 49: mul r1.w, r1.z, l(300.000000)
    r1.w = ((r1.zzzz)*(float4(300.000000,300.000000,300.000000,300.000000))).w;
    // 50: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r2.x, r0.w, r1.w
    r2.x = ((r0.wwww)*(r1.wwww)).x;
    // 53: mul r2.x, r1.z, r2.x
    r2.x = ((r1.zzzz)*(r2.xxxx)).x;
    // 54: dp2 r3.x, cb0[13].xyxx, r0.xyxx
    r3.x = (dot((source[13].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 55: dp2 r3.y, cb0[14].xyxx, r0.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 56: add r2.yz, r3.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r3.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r2.yzyy, t0.yxzw, s2, l(0.000000)
    r2.y = (ArtistNativeSample2((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 58: dp2 r2.z, cb0[15].xyxx, r0.xyxx
    r2.z = (dot((source[15].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 59: mul_sat r2.z, r2.z, l(200.000000)
    r2.z = (saturate((r2.zzzz)*(float4(200.000000,200.000000,200.000000,200.000000)))).z;
    // 60: mul r2.y, r2.z, r2.y
    r2.y = ((r2.zzzz)*(r2.yyyy)).y;
    // 61: dp2 r3.x, cb0[11].xyxx, r0.xyxx
    r3.x = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 62: dp2 r3.y, cb0[12].xyxx, r0.xyxx
    r3.y = (dot((source[12].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 63: dp2 r0.x, cb0[10].xyxx, r0.xyxx
    r0.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 64: mul_sat r0.x, r0.x, l(200.000000)
    r0.x = (saturate((r0.xxxx)*(float4(200.000000,200.000000,200.000000,200.000000)))).x;
    // 65: add r2.zw, r3.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r3.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.zwzz, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 67: mad_sat r0.x, r0.x, r0.y, r2.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(r2.yyyy))).x;
    // 68: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 69: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 70: log r0.y, r1.z
    r0.y = (log2(r1.zzzz)).y;
    // 71: lt r1.z, r1.z, l(0.000001)
    r1.z = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 72: mul r0.y, r0.y, l(30.000000)
    r0.y = ((r0.yyyy)*(float4(30.000000,30.000000,30.000000,30.000000))).y;
    // 73: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 74: mul r0.y, r1.w, r0.y
    r0.y = ((r1.wwww)*(r0.yyyy)).y;
    // 75: movc r0.y, r1.z, l(0), r0.y
    r0.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 76: mad r0.x, r2.x, r0.x, r0.y
    r0.x = ((r2.xxxx)*(r0.xxxx)+(r0.yyyy)).x;
    // 77: mov r2.xw, l(0,0,0,0.010000)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),0.010000)).xw;
    // 78: mov r2.yz, cb0[5].yyxy
    r2.yz = (source[5].yyxy).yz;
    // 79: add r1.zw, v7.xxxy, cb0[0].xxxy
    r1.zw = ((v7.xxxy)+(source[0].xxxy)).zw;
    // 80: mad r2.xy, r1.zwzz, l(0.002000, 0.002000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r1.zwzz)*(float4(0.002000,0.002000,0.000000,0.000000))+(r2.xyxx)).xy;
    // 81: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 83: mad r2.xy, r1.zwzz, l(0.002000, 0.002000, 0.000000, 0.000000), cb0[6].xyxx
    r2.xy = ((r1.zwzz)*(float4(0.002000,0.002000,0.000000,0.000000))+(source[6].xyxx)).xy;
    // 84: add r2.zw, r2.xxxy, l(0.000000, 0.000000, 0.418100, 0.364800)
    r2.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.418100,0.364800))).zw;
    // 85: add r3.xyzw, r2.xyxy, l(0.428100, 0.354800, 0.418100, 0.354800)
    r3.xyzw = ((r2.xyxy)+(float4(0.428100,0.354800,0.418100,0.354800))).xyzw;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t1.xyzw, s0, l(0.000000)
    r2.x = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 87: add r0.y, r0.y, r2.x
    r0.y = ((r0.yyyy)+(r2.xxxx)).y;
    // 88: mad r2.xy, r1.zwzz, l(0.002000, 0.002000, 0.000000, 0.000000), cb0[7].xyxx
    r2.xy = ((r1.zwzz)*(float4(0.002000,0.002000,0.000000,0.000000))+(source[7].xyxx)).xy;
    // 89: add r2.zw, r2.xxxy, l(0.000000, 0.000000, 0.864861, 0.158384)
    r2.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.864861,0.158384))).zw;
    // 90: add r4.xyzw, r2.xyxy, l(0.874861, 0.148384, 0.864861, 0.148384)
    r4.xyzw = ((r2.xyxy)+(float4(0.874861,0.148384,0.864861,0.148384))).xyzw;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t1.xyzw, s0, l(0.000000)
    r2.x = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 92: mad r2.yz, r1.zzwz, l(0.000000, 0.002000, 0.002000, 0.000000), cb0[8].xxyx
    r2.yz = ((r1.zzwz)*(float4(0.000000,0.002000,0.002000,0.000000))+(source[8].xxyx)).yz;
    // 93: add r5.xy, r2.yzyy, l(0.651340, 0.761638, 0.000000, 0.000000)
    r5.xy = ((r2.yzyy)+(float4(0.651340,0.761638,0.000000,0.000000))).xy;
    // 94: add r6.xyzw, r2.yzyz, l(0.661340, 0.751638, 0.651340, 0.751638)
    r6.xyzw = ((r2.yzyz)+(float4(0.661340,0.751638,0.651340,0.751638))).xyzw;
    // 95: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r5.xyxx, t1.yxzw, s0, l(0.000000)
    r2.y = (ArtistNativeSample0((r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 96: add r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 97: add r0.y, r0.y, r2.x
    r0.y = ((r0.yyyy)+(r2.xxxx)).y;
    // 98: mad r2.xy, r1.zwzz, l(0.002000, 0.002000, 0.000000, 0.000000), cb0[5].xyxx
    r2.xy = ((r1.zwzz)*(float4(0.002000,0.002000,0.000000,0.000000))+(source[5].xyxx)).xy;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xyxx, t1.xyzw, s0, l(0.000000)
    r2.x = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r3.zwzz, t1.yxzw, s0, l(0.000000)
    r2.y = (ArtistNativeSample0((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 101: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r3.xyxx, t1.yzxw, s0, l(0.000000)
    r2.z = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 102: add r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 103: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r4.zwzz, t1.yxzw, s0, l(0.000000)
    r2.y = (ArtistNativeSample0((r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 104: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r4.xyxx, t1.yzwx, s0, l(0.000000)
    r2.w = (ArtistNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r6.zwzz, t1.xyzw, s0, l(0.000000)
    r3.x = (ArtistNativeSample0((r6.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r3.y, r6.xyxx, t1.yxzw, s0, l(0.000000)
    r3.y = (ArtistNativeSample0((r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 107: add r2.yw, r2.yyyw, r3.xxxy
    r2.yw = ((r2.yyyw)+(r3.xxxy)).yw;
    // 108: add r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 109: mul r2.x, r2.x, l(0.250000)
    r2.x = ((r2.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 110: mad r3.y, r0.y, l(0.250000), -r2.x
    r3.y = ((r0.yyyy)*(float4(0.250000,0.250000,0.250000,0.250000))+(-(r2.xxxx))).y;
    // 111: mov r4.xw, cb0[5].xxxy
    r4.xw = (source[5].xxxy).xw;
    // 112: mov r4.yz, l(0,0,0.010000,0)
    r4.yz = (float4(asfloat(0u),asfloat(0u),0.010000,asfloat(0u))).yz;
    // 113: mad r3.zw, r1.zzzw, l(0.000000, 0.000000, 0.002000, 0.002000), r4.xxxy
    r3.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.002000,0.002000))+(r4.xxxy)).zw;
    // 114: add r3.zw, r3.zzzw, r4.zzzw
    r3.zw = ((r3.zzzw)+(r4.zzzw)).zw;
    // 115: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r3.zwzz, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 116: add r0.y, r2.z, r0.y
    r0.y = ((r2.zzzz)+(r0.yyyy)).y;
    // 117: add r0.y, r2.w, r0.y
    r0.y = ((r2.wwww)+(r0.yyyy)).y;
    // 118: mad r3.x, r0.y, l(0.250000), -r2.x
    r3.x = ((r0.yyyy)*(float4(0.250000,0.250000,0.250000,0.250000))+(-(r2.xxxx))).x;
    // 119: mul r2.xy, r1.zwzz, l(0.003100, 0.003100, 0.000000, 0.000000)
    r2.xy = ((r1.zwzz)*(float4(0.003100,0.003100,0.000000,0.000000))).xy;
    // 120: mul r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.007000, 0.007000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.007000,0.007000))).zw;
    // 121: mad r2.y, cb0[16].x, l(0.090000), r2.y
    r2.y = ((source[16].xxxx)*(float4(0.090000,0.090000,0.090000,0.090000))+(r2.yyyy)).y;
    // 122: mad r2.xy, r3.xyxx, l(-0.360000, -0.360000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r3.xyxx)*(float4(-0.360000,-0.360000,0.000000,0.000000))+(r2.xyxx)).xy;
    // 123: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 124: mad r1.z, cb0[16].x, l(0.090000), r1.z
    r1.z = ((source[16].xxxx)*(float4(0.090000,0.090000,0.090000,0.090000))+(r1.zzzz)).z;
    // 125: mad r1.zw, r3.xxxy, l(0.000000, 0.000000, -0.360000, -0.360000), r1.zzzw
    r1.zw = ((r3.xxxy)*(float4(0.000000,0.000000,-0.360000,-0.360000))+(r1.zzzw)).zw;
    // 126: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r1.zwzz, t2.xyzw, s1, l(0.000000)
    r3.xyz = (ArtistNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 127: add r2.xyzw, r2.xxyz, r3.xxyz
    r2.xyzw = ((r2.xxyz)+(r3.xxyz)).xyzw;
    // 128: mul r2.xyzw, r2.xyzw, r2.xyzw
    r2.xyzw = ((r2.xyzw)*(r2.xyzw)).xyzw;
    // 129: mul r2.xyzw, r2.xyzw, l(2.000000, 2.000000, 2.000000, 2.000000)
    r2.xyzw = ((r2.xyzw)*(float4(2.000000,2.000000,2.000000,2.000000))).xyzw;
    // 130: mad r2.xyzw, r2.xyzw, r2.xyzw, r2.xyzw
    r2.xyzw = ((r2.xyzw)*(r2.xyzw)+(r2.xyzw)).xyzw;
    // 131: mad r0.y, -r1.x, cb0[17].y, l(1.000000)
    r0.y = ((-(r1.xxxx))*(source[17].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 132: mov_sat r1.z, r0.y
    r1.z = (saturate(r0.yyyy)).z;
    // 133: mul_sat r0.y, r0.y, l(100000.000000)
    r0.y = (saturate((r0.yyyy)*(float4(100000.000000,100000.000000,100000.000000,100000.000000)))).y;
    // 134: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 135: mul r1.w, r1.z, r1.z
    r1.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 136: mul r1.zw, r0.yyyy, r1.zzzw
    r1.zw = ((r0.yyyy)*(r1.zzzw)).zw;
    // 137: mul r0.y, r0.y, l(0.150000)
    r0.y = ((r0.yyyy)*(float4(0.150000,0.150000,0.150000,0.150000))).y;
    // 138: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 139: mul r3.xyzw, r2.xyzw, r1.wwww
    r3.xyzw = ((r2.xyzw)*(r1.wwww)).xyzw;
    // 140: mul r2.xyzw, r2.xyzw, r2.xyzw
    r2.xyzw = ((r2.xyzw)*(r2.xyzw)).xyzw;
    // 141: mad r3.xyzw, r3.xyzw, l(0.200000, 0.200000, 0.200000, 0.200000), r0.yyyy
    r3.xyzw = ((r3.xyzw)*(float4(0.200000,0.200000,0.200000,0.200000))+(r0.yyyy)).xyzw;
    // 142: mul r0.y, r2.x, l(10.000000)
    r0.y = ((r2.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 143: log r1.w, r1.z
    r1.w = (log2(r1.zzzz)).w;
    // 144: lt r1.z, r1.z, l(0.000001)
    r1.z = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 145: mul r1.w, r1.w, l(30.000000)
    r1.w = ((r1.wwww)*(float4(30.000000,30.000000,30.000000,30.000000))).w;
    // 146: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 147: movc r1.z, r1.z, l(0), r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 148: mul r0.y, r0.y, r1.z
    r0.y = ((r0.yyyy)*(r1.zzzz)).y;
    // 149: mul r2.xyz, r2.yzwy, r1.zzzz
    r2.xyz = ((r2.yzwy)*(r1.zzzz)).xyz;
    // 150: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), r3.yzwy
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(r3.yzwy)).xyz;
    // 151: mad r0.y, r0.y, l(0.200000), r3.x
    r0.y = ((r0.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(r3.xxxx)).y;
    // 152: mad r0.x, r0.x, l(0.200000), r0.y
    r0.x = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))+(r0.yyyy)).x;
    // 153: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 154: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 155: mul_sat r0.x, r0.x, cb0[19].z
    r0.x = (saturate((r0.xxxx)*(source[19].zzzz))).x;
    // 156: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 157: add r2.xyz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 158: add r0.y, -|v4.w|, cb0[1].y
    r0.y = ((-(abs(v4.wwww)))+(source[1].yyyy)).y;
    // 159: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 160: div_sat r0.y, r0.y, cb0[1].y
    r0.y = (saturate((r0.yyyy)/(source[1].yyyy))).y;
    // 161: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 162: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    // 163: lt r0.x, l(0.000000), cb0[17].z
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))<(source[17].zzzz)) * 0xffffffffu)).x;
    // 164: movc r0.x, r0.x, cb0[17].z, l(1.000000)
    r0.x = ((asuint(r0.xxxx) != 0u) ? (source[17].zzzz) : (float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 165: ge r0.y, cb0[17].z, l(0.000000)
    r0.y = (asfloat((uint4)((source[17].zzzz)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 166: movc r0.x, r0.y, r0.x, cb0[17].z
    r0.x = ((asuint(r0.yyyy) != 0u) ? (r0.xxxx) : (source[17].zzzz)).x;
    // 167: div r0.xy, l(0.003000, 0.002000, 0.000000, 0.000000), r0.xxxx
    r0.xy = ((float4(0.003000,0.002000,0.000000,0.000000))/(r0.xxxx)).xy;
    // 168: add r0.y, -r0.y, cb0[17].w
    r0.y = ((-(r0.yyyy))+(source[17].wwww)).y;
    // 169: add r0.xy, -r0.xyxx, l(0.500000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(0.500000,1.000000,0.000000,0.000000))).xy;
    // 170: add r0.y, -r0.y, r0.z
    r0.y = ((-(r0.yyyy))+(r0.zzzz)).y;
    // 171: mul_sat r0.y, r0.y, l(2000.000000)
    r0.y = (saturate((r0.yyyy)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).y;
    // 172: add r0.y, -r0.y, r0.w
    r0.y = ((-(r0.yyyy))+(r0.wwww)).y;
    // 173: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 174: max r0.x, r0.x, l(0.000010)
    r0.x = (max(r0.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 175: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 176: mad r0.x, -r1.x, r0.x, l(1.000000)
    r0.x = ((-(r1.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 177: mul_sat r0.x, r0.x, l(100000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(100000.000000,100000.000000,100000.000000,100000.000000)))).x;
    // 178: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 179: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 180: mad r0.x, r0.x, l(5.000000), r0.y
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.yyyy)).x;
    // 181: add r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)+(r2.xyzx)).xyz;
    // 182: mul r1.xyz, cb0[2].xyzx, cb0[9].xyzx
    r1.xyz = ((source[2].xyzx)*(source[9].xyzx)).xyz;
    // 183: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[4].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[4].xyzx)).xyz;
    // 184: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
