// Original Kouku material programs 4288..4351; native IDs and expressions are unchanged.
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_waterflow_01_19_tr: d5264a6e7ea1394690402346afbc0de6; selected map 7f81ef1d8d362e88193717f346d5f3bc9143e81b03f25b065f212b6f5346b98d.
float4 ArtistNative4288(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[8u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[4] = ArtistNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz),1u);
    source[5] = g_ArtistSourceMaterialParameters[6u];
    source[6] = g_ArtistSourceMaterialParameters[7u];
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[5u].xxxx,1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[9].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    // 28: mul r0.w, r0.y, cb0[9].x
    r0.w = ((r0.yyyy)*(source[9].xxxx)).w;
    // 29: mad r0.x, r0.z, l(0.500000), r0.w
    r0.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).x;
    // 30: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mov r1.w, cb0[9].w
    r1.w = (source[9].wwww).w;
    // 33: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 34: mad r2.xyzw, r0.xyxy, cb0[3].xyxy, r1.wzzw
    r2.xyzw = ((r0.xyxy)*(source[3].xyxy)+(r1.wzzw)).xyzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s0, l(0.000000)
    r1.z = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: add r2.y, -r0.z, r1.z
    r2.y = ((-(r0.zzzz))+(r1.zzzz)).y;
    // 38: add r2.x, -r0.z, r0.w
    r2.x = ((-(r0.zzzz))+(r0.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 44: div r0.zw, r2.xxxy, r0.zzzz
    r0.zw = ((r2.xxxy)/(r0.zzzz)).zw;
    // 45: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: mov r1.y, v4.w
    r1.y = (v4.wwww).y;
    // 47: mad r1.xy, r0.xyxx, cb0[8].xyxx, r1.xyxx
    r1.xy = ((r0.xyxx)*(source[8].xyxx)+(r1.xyxx)).xy;
    // 48: mad r1.xy, v4.xxxx, r0.zwzz, r1.xyxx
    r1.xy = ((v4.xxxx)*(r0.zwzz)+(r1.xyxx)).xy;
    // 49: mul r0.zw, r0.zzzw, v4.xxxx
    r0.zw = ((r0.zzzw)*(v4.xxxx)).zw;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 52: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 53: mul r1.x, r1.x, cb0[12].z
    r1.x = ((r1.xxxx)*(source[12].zzzz)).x;
    // 54: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 55: mad r1.yz, r0.xxyx, cb0[7].xxyx, r0.zzwz
    r1.yz = ((r0.xxyx)*(source[7].xxyx)+(r0.zzwz)).yz;
    // 56: mad r0.xy, r0.xyxx, cb0[2].xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(source[2].xyxx)+(r0.zwzz)).xy;
    // 57: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t3.wxyz, s2, l(0.000000)
    r1.yzw = (ArtistNativeSample2((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 60: add r0.w, r1.y, cb0[12].w
    r0.w = ((r1.yyyy)+(source[12].wwww)).w;
    // 61: max r1.yzw, |r1.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r1.yzw = (max(abs(r1.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 62: log r1.yzw, r1.yyzw
    r1.yzw = (log2(r1.yyzw)).yzw;
    // 63: mul r1.yzw, r1.yyzw, cb0[11].zzzz
    r1.yzw = ((r1.yyzw)*(source[11].zzzz)).yzw;
    // 64: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 65: mad r1.yzw, -cb0[11].wwww, r1.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r1.yzw = ((-(source[11].wwww))*(r1.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 66: mul r0.w, r0.w, cb0[13].x
    r0.w = ((r0.wwww)*(source[13].xxxx)).w;
    // 67: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 68: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 69: sincos r0.w, null, r0.w
    { const float4 sourceAngle = r0.wwww; r0.w = (sin(sourceAngle)).w; }
    // 70: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 71: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 72: mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // 73: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 74: mul_sat r0.w, r0.w, cb0[13].z
    r0.w = (saturate((r0.wwww)*(source[13].zzzz))).w;
    // 75: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 76: mul r0.w, r0.w, cb0[13].w
    r0.w = ((r0.wwww)*(source[13].wwww)).w;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t0.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 78: mul_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)*(r1.xxxx))).w;
    // 79: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 80: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 81: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 83: mad r0.xyz, cb0[10].yyyy, r2.xyzx, r0.xyzx
    r0.xyz = ((source[10].yyyy)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 84: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 85: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 86: mul r0.xyz, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 87: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 88: mul r0.xyz, r0.xyzx, cb0[10].wwww
    r0.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 89: add r2.xyz, -cb0[5].xyzx, cb0[6].xyzx
    r2.xyz = ((-(source[5].xyzx))+(source[6].xyzx)).xyz;
    // 90: mad r1.xyz, r1.yzwy, r2.xyzx, cb0[5].xyzx
    r1.xyz = ((r1.yzwy)*(r2.xyzx)+(source[5].xyzx)).xyz;
    // 91: mad r0.xyz, r0.xyzx, v3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(r1.xyzx)).xyz;
    // 92: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 93: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4288Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[5u].xxxx,1u);
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: add r0.xy, v1.yxyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.yxyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
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
    // 28: mul r0.w, r0.y, cb0[3].x
    r0.w = ((r0.yyyy)*(source[3].xxxx)).w;
    // 29: mad r0.x, r0.z, l(0.500000), r0.w
    r0.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).x;
    // 30: mul r0.zw, r0.xxxy, cb0[0].xxxy
    r0.zw = ((r0.xxxy)*(source[0].xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mov r1.w, cb0[3].w
    r1.w = (source[3].wwww).w;
    // 33: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 34: mad r2.xyzw, r0.xyxy, cb0[0].xyxy, r1.wzzw
    r2.xyzw = ((r0.xyxy)*(source[0].xyxy)+(r1.wzzw)).xyzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s1, l(0.000000)
    r1.z = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: add r2.y, -r0.z, r1.z
    r2.y = ((-(r0.zzzz))+(r1.zzzz)).y;
    // 38: add r2.x, -r0.z, r0.w
    r2.x = ((-(r0.zzzz))+(r0.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 44: div r0.zw, r2.xxxy, r0.zzzz
    r0.zw = ((r2.xxxy)/(r0.zzzz)).zw;
    // 45: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: mov r1.y, v3.w
    r1.y = (v3.wwww).y;
    // 47: mad r1.xy, r0.xyxx, cb0[2].xyxx, r1.xyxx
    r1.xy = ((r0.xyxx)*(source[2].xyxx)+(r1.xyxx)).xy;
    // 48: mad r1.xy, v3.xxxx, r0.zwzz, r1.xyxx
    r1.xy = ((v3.xxxx)*(r0.zwzz)+(r1.xyxx)).xy;
    // 49: mul r0.zw, r0.zzzw, v3.xxxx
    r0.zw = ((r0.zzzw)*(v3.xxxx)).zw;
    // 50: mad r0.xy, r0.xyxx, cb0[1].xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(source[1].xyxx)+(r0.zwzz)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 52: add r0.xyzw, r0.xyxy, cb0[4].wwww
    r0.xyzw = ((r0.xyxy)+(source[4].wwww)).xyzw;
    // 53: mul r0.xyzw, r0.xyzw, cb0[5].xxxx
    r0.xyzw = ((r0.xyzw)*(source[5].xxxx)).xyzw;
    // 54: mul r0.xyzw, r0.xyzw, v3.zzzz
    r0.xyzw = ((r0.xyzw)*(v3.zzzz)).xyzw;
    // 55: mul r0.xyzw, r0.xyzw, l(6.283185, 6.283185, 6.283185, 6.283185)
    r0.xyzw = ((r0.xyzw)*(float4(6.283185,6.283185,6.283185,6.283185))).xyzw;
    // 56: sincos r0.xyzw, null, r0.xyzw
    { const float4 sourceAngle = r0.xyzw; r0.xyzw = (sin(sourceAngle)).xyzw; }
    // 57: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 58: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 59: mul r0.xyzw, r0.xyzw, cb0[5].yyyy
    r0.xyzw = ((r0.xyzw)*(source[5].yyyy)).xyzw;
    // 60: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 61: mul_sat r0.xyzw, r0.xyzw, cb0[5].zzzz
    r0.xyzw = (saturate((r0.xyzw)*(source[5].zzzz))).xyzw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s4, l(0.000000)
    r1.xy = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 63: max r1.xyzw, |r1.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 64: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 65: mul r1.xyzw, r1.xyzw, cb0[4].zzzz
    r1.xyzw = ((r1.xyzw)*(source[4].zzzz)).xyzw;
    // 66: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 67: mul r0.xyzw, r0.xyzw, r1.xyzw
    r0.xyzw = ((r0.xyzw)*(r1.xyzw)).xyzw;
    // 68: mul r0.xyzw, r0.xyzw, cb0[5].wwww
    r0.xyzw = ((r0.xyzw)*(source[5].wwww)).xyzw;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v1.xyxx, t0.xyzw, s3, l(0.000000)
    r1.xy = (ArtistNativeSample3((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 70: mul_sat r0.xyzw, r0.xyzw, r1.xyxy
    r0.xyzw = (saturate((r0.xyzw)*(r1.xyxy))).xyzw;
    // 71: mul r0.xyzw, r0.xyzw, v2.wwww
    r0.xyzw = ((r0.xyzw)*(v2.wwww)).xyzw;
    // 72: mul r0.xyzw, r0.xyzw, cb0[6].xxxx
    r0.xyzw = ((r0.xyzw)*(source[6].xxxx)).xyzw;
    // 73: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 74: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 75: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 76: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 77: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 78: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 79: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 80: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 81: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 82: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 83: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 84: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 85: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 86: source device depth mapped to centimetre view depth; reconstruction at 88.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 88-91: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 92: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 93: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 94: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 95: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 96: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_twirl_01_01_ad: 0f7eab3760884747a6788150e2a3c640; selected map c58c1cb0c02b327bc071a559aa0a7b85d600bdcdc099fdccbcd868d9754f62db.
float4 ArtistNative4289(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = g_ArtistSourceMaterialParameters[7u];
    source[5].x = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[7].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[10].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[11].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[11].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[11].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[11].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[12].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[13].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[13].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ArtistSourceMaterialParameters[0u].zzzz))).x;
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
    // 32: mul r1.x, cb0[3].w, cb0[7].z
    r1.x = ((source[3].wwww)*(source[7].zzzz)).x;
    // 33: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 34: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 35: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 36: mul r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)*(source[7].wwww)).x;
    // 37: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 38: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 39: mad r1.x, r0.y, l(0.318310), r0.w
    r1.x = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.wwww)).x;
    // 40: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 41: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 42: mul r0.y, r0.y, cb0[3].z
    r0.y = ((r0.yyyy)*(source[3].zzzz)).y;
    // 43: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 44: lt r0.w, r0.x, l(0.000000)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 45: mad r0.x, -r0.x, cb0[11].x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[11].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: mul_sat r0.x, r0.x, cb0[12].x
    r0.x = (saturate((r0.xxxx)*(source[12].xxxx))).x;
    // 47: movc r1.y, r0.w, l(0), r0.y
    r1.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 48: mul r0.yw, v4.xxxy, cb0[6].zzzw
    r0.yw = ((v4.xxxy)*(source[6].zzzw)).yw;
    // 49: mul r1.z, cb0[5].x, cb0[5].y
    r1.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 50: mad r2.x, r1.z, cb0[6].y, r0.y
    r2.x = ((r1.zzzz)*(source[6].yyyy)+(r0.yyyy)).x;
    // 51: mad r2.y, r1.z, cb0[7].x, r0.w
    r2.y = ((r1.zzzz)*(source[7].xxxx)+(r0.wwww)).y;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 53: mad r0.yw, r0.yyyy, cb0[7].yyyy, r1.xxxy
    r0.yw = ((r0.yyyy)*(source[7].yyyy)+(r1.xxxy)).yw;
    // 54: mul r1.x, r0.y, cb0[5].w
    r1.x = ((r0.yyyy)*(source[5].wwww)).x;
    // 55: mad r1.x, r1.z, cb0[5].z, r1.x
    r1.x = ((r1.zzzz)*(source[5].zzzz)+(r1.xxxx)).x;
    // 56: mul r1.w, r1.z, cb0[8].x
    r1.w = ((r1.zzzz)*(source[8].xxxx)).w;
    // 57: mad r1.y, cb0[6].x, r0.w, r1.w
    r1.y = ((source[6].xxxx)*(r0.wwww)+(r1.wwww)).y;
    // 58: mul r0.yw, r0.yyyw, cb0[8].zzzw
    r0.yw = ((r0.yyyw)*(source[8].zzzw)).yw;
    // 59: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r1.xyxx, t1.xywz, s1, l(0.000000)
    r1.xyw = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xywz).xyw;
    // 60: mad r2.x, r1.z, cb0[8].y, r0.y
    r2.x = ((r1.zzzz)*(source[8].yyyy)+(r0.yyyy)).x;
    // 61: mad r2.y, r1.z, cb0[9].x, r0.w
    r2.y = ((r1.zzzz)*(source[9].xxxx)+(r0.wwww)).y;
    // 62: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyz = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 63: add r0.y, -r1.x, r2.x
    r0.y = ((-(r1.xxxx))+(r2.xxxx)).y;
    // 64: mad r0.y, r0.y, l(0.500000), r1.x
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xxxx)).y;
    // 65: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mul r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)*(r0.wwww)).w;
    // 67: mul_sat r0.z, r0.z, cb0[10].x
    r0.z = (saturate((r0.zzzz)*(source[10].xxxx))).z;
    // 68: mul r0.w, r0.w, r0.y
    r0.w = ((r0.wwww)*(r0.yyyy)).w;
    // 69: log r1.z, r0.z
    r1.z = (log2(r0.zzzz)).z;
    // 70: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 71: mul r1.z, r1.z, cb0[10].y
    r1.z = ((r1.zzzz)*(source[10].yyyy)).z;
    // 72: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 73: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 74: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 75: mad_sat r0.x, r0.w, cb0[9].w, r0.x
    r0.x = (saturate((r0.wwww)*(source[9].wwww)+(r0.xxxx))).x;
    // 76: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 77: mul r0.z, r0.z, cb0[12].y
    r0.z = ((r0.zzzz)*(source[12].yyyy)).z;
    // 78: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 79: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 80: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 81: mad_sat r0.y, v6.z, r0.w, r0.y
    r0.y = (saturate((v6.zzzz)*(r0.wwww)+(r0.yyyy))).y;
    // 82: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 83: lt r0.xy, r0.xyxx, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 84: mul r0.w, r0.w, cb0[12].z
    r0.w = ((r0.wwww)*(source[12].zzzz)).w;
    // 85: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 86: mul_sat r0.w, r0.w, cb0[12].w
    r0.w = (saturate((r0.wwww)*(source[12].wwww))).w;
    // 87: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 88: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 89: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 90: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 91: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 92: source device depth mapped to centimetre view depth; reconstruction at 94.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 94-97: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 98: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 99: add r0.w, -cb0[13].z, l(1.000000)
    r0.w = ((-(source[13].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 101: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 102: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 103: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 104: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 105: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 106: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 107: mul r0.yzw, r1.xxyw, r2.xxyz
    r0.yzw = ((r1.xxyw)*(r2.xxyz)).yzw;
    // 108: dp3 r1.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 109: mad r1.xyz, -r1.xywx, r2.xyzx, r1.zzzz
    r1.xyz = ((-(r1.xywx))*(r2.xyzx)+(r1.zzzz)).xyz;
    // 110: mad r0.yzw, cb0[9].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[9].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 111: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 112: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 113: mul r0.yzw, r0.yyzw, cb0[9].zzzz
    r0.yzw = ((r0.yyzw)*(source[9].zzzz)).yzw;
    // 114: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 115: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 116: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 117: mad r0.yzw, cb0[1].xxyz, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)*(r0.yyzw)+(source[2].xxyz)).yzw;
    // 118: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 119: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 120: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4289Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_k_me_trail_02_23_tr: 8a207c75b72d7a43a73607666b39ad11; selected map 21cff576e7c7cefcc9ed0dd1cec0552a9c9f98faf45bc5d08481bcf1c8863cb6.
float4 ArtistNative4290(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[12u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].wwww,g_ArtistSourceMaterialParameters[7u].xxxx,1u);
    source[4] = input.dynamicParameter;
    source[5] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].xxxx,g_ArtistSourceMaterialParameters[3u].yyyy,1u);
    source[9].x = (cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[11].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[11].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[12].x = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[16].y = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[16].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[16].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[17].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[18].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[18].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[19].y = (g_ArtistSourceMaterialParameters[8u].wwww).x;
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
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.yz, v4.xxyx, cb0[10].yyzy
    r0.yz = ((v4.xxyx)*(source[10].yyzy)).yz;
    // 8: mad r0.yz, cb0[4].zzzz, cb0[11].xxyx, r0.yyzy
    r0.yz = ((source[4].zzzz)*(source[11].xxyx)+(r0.yyzy)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s4, l(0.000000)
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
    // 33: mul r1.zw, cb0[4].zzzz, cb0[18].zzzw
    r1.zw = ((source[4].zzzz)*(source[18].zzzw)).zw;
    // 34: mad r1.zw, cb0[18].xxxy, v4.xxxy, r1.zzzw
    r1.zw = ((source[18].xxxy)*(v4.xxxy)+(r1.zzzw)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t3.yzwx, s5, l(0.000000)
    r0.w = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: add r0.w, -r1.y, r0.w
    r0.w = ((-(r1.yyyy))+(r0.wwww)).w;
    // 37: mul_sat r0.w, r0.w, cb0[19].x
    r0.w = (saturate((r0.wwww)*(source[19].xxxx))).w;
    // 38: add r1.y, -v4.y, l(1.000000)
    r1.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: mul r1.y, r1.y, v4.y
    r1.y = ((r1.yyyy)*(v4.yyyy)).y;
    // 40: mul_sat r1.y, r1.y, cb0[17].w
    r1.y = (saturate((r1.yyyy)*(source[17].wwww))).y;
    // 41: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 42: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 43: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 44: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 45: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 46: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 47: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 48: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 49: mul r0.w, r0.w, cb0[19].y
    r0.w = ((r0.wwww)*(source[19].yyyy)).w;
    // 50: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 51: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 52: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 53: div r1.yz, v7.xxyx, v7.wwww
    r1.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 54: mad r1.yz, r1.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r1.yz = ((r1.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 55: source device depth mapped to centimetre view depth; reconstruction at 57.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.yzyy).xy, 0.f).y * 100000.f;
    // Native 57-60: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 61: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 62: mul_sat r0.w, r0.w, l(0.007143)
    r0.w = (saturate((r0.wwww)*(float4(0.007143,0.007143,0.007143,0.007143)))).w;
    // 63: mul_sat r0.x, r0.w, r0.x
    r0.x = (saturate((r0.wwww)*(r0.xxxx))).x;
    // 64: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 65: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 66: mad r0.xz, v4.xxyx, cb0[9].yyzy, cb0[3].xxyx
    r0.xz = ((v4.xxyx)*(source[9].yyzy)+(source[3].xxyx)).xz;
    // 67: mad r0.xy, r0.yyyy, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xzxx
    r0.xy = ((r0.yyyy)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xzxx)).xy;
    // 68: mad r0.xy, r1.xxxx, cb0[11].zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[11].zwzz)+(r0.xyxx)).xy;
    // 69: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 70: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 71: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 72: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 74: mul r0.yz, cb0[4].zzzz, cb0[12].zzwz
    r0.yz = ((source[4].zzzz)*(source[12].zzwz)).yz;
    // 75: mad r0.yz, v4.xxyx, cb0[12].xxyx, r0.yyzy
    r0.yz = ((v4.xxyx)*(source[12].xxyx)+(r0.yyzy)).yz;
    // 76: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 77: dp2 r1.x, cb0[5].xyxx, r0.yzyy
    r1.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 78: dp2 r1.y, cb0[6].xyxx, r0.yzyy
    r1.y = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 79: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 81: add r0.y, -r0.x, r0.y
    r0.y = ((-(r0.xxxx))+(r0.yyyy)).y;
    // 82: mad r0.x, r0.y, l(0.500000), r0.x
    r0.x = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.xxxx)).x;
    // 83: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 84: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 85: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 86: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 87: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 88: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 89: mul r0.z, r0.z, cb0[13].y
    r0.z = ((r0.zzzz)*(source[13].yyyy)).z;
    // 90: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 91: mad r0.x, r0.y, cb0[13].z, r0.x
    r0.x = ((r0.yyyy)*(source[13].zzzz)+(r0.xxxx)).x;
    // 92: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 93: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_me_spritewave_01_08_ad: 7a247f168ab39940a18326ffb9bed3c2; selected map 6f18cd790230668b14555a0aa2e5b214620affefbe85030bc5b5e0f030f85f46.
float4 ArtistNative4291(ARTIST_NATIVE_INPUT input)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[13u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].wwww,g_ArtistSourceMaterialParameters[6u].xxxx,1u);
    source[7] = ArtistNativeAppend(cos(((g_ArtistSourceMaterialParameters[6u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[6u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = ArtistNativeAppend(sin(((g_ArtistSourceMaterialParameters[6u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ArtistSourceMaterialParameters[6u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = g_ArtistSourceMaterialParameters[11u];
    source[12].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ArtistSourceMaterialParameters[6u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[12].y = (cos(((g_ArtistSourceMaterialParameters[6u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[12].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[13].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[6u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[14].z = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[14].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[16].y = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[16].z = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[18].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[18].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[18].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[19].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[19].z = ((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[19].w = (sin((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[20].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[20].y = (cos((g_ArtistSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[20].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[20].w = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[21].x = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[21].y = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[21].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[21].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[22].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[22].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[22].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[22].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[23].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[23].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[23].z = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[23].w = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[24].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[24].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
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
    // 1: mul r0.x, v4.x, cb0[20].w
    r0.x = ((v4.xxxx)*(source[20].wwww)).x;
    // 2: mad r0.x, cb0[12].w, cb0[20].z, r0.x
    r0.x = ((source[12].wwww)*(source[20].zzzz)+(r0.xxxx)).x;
    // 3: mul r0.zw, cb0[12].wwww, cb0[21].yyyw
    r0.zw = ((source[12].wwww)*(source[21].yyyw)).zw;
    // 4: mad r0.y, cb0[21].x, v4.y, r0.z
    r0.y = ((source[21].xxxx)*(v4.yyyy)+(r0.zzzz)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 7: dp2 r1.x, cb0[9].xyxx, r0.yzyy
    r1.x = (dot((source[9].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 8: dp2 r1.y, cb0[10].xyxx, r0.yzyy
    r1.y = (dot((source[10].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 9: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 10: mad r1.xy, r0.xxxx, cb0[21].zzzz, r1.xyxx
    r1.xy = ((r0.xxxx)*(source[21].zzzz)+(r1.xyxx)).xy;
    // 11: mad r2.y, cb0[19].x, r1.y, r0.w
    r2.y = ((source[19].xxxx)*(r1.yyyy)+(r0.wwww)).y;
    // 12: mul r0.x, r1.x, cb0[18].w
    r0.x = ((r1.xxxx)*(source[18].wwww)).x;
    // 13: mad r2.x, cb0[12].w, cb0[18].z, r0.x
    r2.x = ((source[12].wwww)*(source[18].zzzz)+(r0.xxxx)).x;
    // 14: add r0.xw, r2.xxxy, cb0[22].xxxy
    r0.xw = ((r2.xxxy)+(source[22].xxxy)).xw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t3.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 16: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 17: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 18: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.w, r0.w, cb0[22].z
    r0.w = ((r0.wwww)*(source[22].zzzz)).w;
    // 20: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 21: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 22: add r0.w, cb0[3].y, l(-1.000000)
    r0.w = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 23: add_sat r0.x, -r0.w, r0.x
    r0.x = (saturate((-(r0.wwww))+(r0.xxxx))).x;
    // 24: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 25: mul r0.w, r0.w, cb0[23].x
    r0.w = ((r0.wwww)*(source[23].xxxx)).w;
    // 26: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 27: mul_sat r0.w, r0.w, cb0[22].w
    r0.w = (saturate((r0.wwww)*(source[22].wwww))).w;
    // 28: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.x, r0.x, cb0[22].w
    r0.x = ((r0.xxxx)*(source[22].wwww)).x;
    // 30: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 31: mov_sat r1.x, r0.x
    r1.x = (saturate(r0.xxxx)).x;
    // 32: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 33: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: mul r1.xyz, r0.wwww, cb0[11].xyzx
    r1.xyz = ((r0.wwww)*(source[11].xyzx)).xyz;
    // 35: mul r0.w, cb0[12].w, cb0[14].z
    r0.w = ((source[12].wwww)*(source[14].zzzz)).w;
    // 36: mad r2.x, cb0[14].w, v4.x, r0.w
    r2.x = ((source[14].wwww)*(v4.xxxx)+(r0.wwww)).x;
    // 37: mul r0.w, v4.y, cb0[15].x
    r0.w = ((v4.yyyy)*(source[15].xxxx)).w;
    // 38: mad r2.y, cb0[12].w, cb0[15].y, r0.w
    r2.y = ((source[12].wwww)*(source[15].yyyy)+(r0.wwww)).y;
    // 39: mad r2.xy, cb0[3].wwww, cb0[15].zwzz, r2.xyxx
    r2.xy = ((source[3].wwww)*(source[15].zwzz)+(r2.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 41: add r1.w, cb0[3].z, cb0[16].x
    r1.w = ((source[3].zzzz)+(source[16].xxxx)).w;
    // 42: mad r2.xy, r0.wwww, r1.wwww, cb0[6].xyxx
    r2.xy = ((r0.wwww)*(r1.wwww)+(source[6].xyxx)).xy;
    // 43: dp2 r3.x, cb0[4].xyxx, r0.yzyy
    r3.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 44: dp2 r3.y, cb0[5].xyxx, r0.yzyy
    r3.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 45: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 46: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 47: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 49: mul r0.y, r0.y, cb0[23].z
    r0.y = ((r0.yyyy)*(source[23].zzzz)).y;
    // 50: max r0.y, r0.y, cb0[24].x
    r0.y = (max(r0.yyyy,source[24].xxxx)).y;
    // 51: min r0.y, r0.y, cb0[23].w
    r0.y = (min(r0.yyyy,source[23].wwww)).y;
    // 52: mad r0.zw, cb0[13].zzzw, cb0[3].xxxx, r3.xxxy
    r0.zw = ((source[13].zzzw)*(source[3].xxxx)+(r3.xxxy)).zw;
    // 53: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 54: mul r0.zw, r0.zzzw, cb0[13].xxxy
    r0.zw = ((r0.zzzw)*(source[13].xxxy)).zw;
    // 55: mad r3.x, cb0[12].w, cb0[12].z, r0.z
    r3.x = ((source[12].wwww)*(source[12].zzzz)+(r0.zzzz)).x;
    // 56: mad r3.y, cb0[12].w, cb0[14].y, r0.w
    r3.y = ((source[12].wwww)*(source[14].yyyy)+(r0.wwww)).y;
    // 57: add r0.zw, r2.xxxy, r3.xxxy
    r0.zw = ((r2.xxxy)+(r3.xxxy)).zw;
    // 58: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 59: dp2 r2.x, cb0[7].xyxx, r0.zwzz
    r2.x = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 60: dp2 r2.y, cb0[8].xyxx, r0.zwzz
    r2.y = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 61: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 62: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s0, l(-1.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 63: mul r2.xy, v4.xyxx, cb0[17].xyxx
    r2.xy = ((v4.xyxx)*(source[17].xyxx)).xy;
    // 64: mad r3.x, cb0[12].w, cb0[16].w, r2.x
    r3.x = ((source[12].wwww)*(source[16].wwww)+(r2.xxxx)).x;
    // 65: mad r3.y, cb0[12].w, cb0[17].z, r2.y
    r3.y = ((source[12].wwww)*(source[17].zzzz)+(r2.yyyy)).y;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.xyxx, t4.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 67: mul r0.xw, r0.xxxw, r0.zzzz
    r0.xw = ((r0.xxxw)*(r0.zzzz)).xw;
    // 68: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 69: mul r1.w, r1.w, cb0[17].w
    r1.w = ((r1.wwww)*(source[17].wwww)).w;
    // 70: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 71: mul r1.w, r1.w, cb0[18].x
    r1.w = ((r1.wwww)*(source[18].xxxx)).w;
    // 72: lt r2.x, |r0.w|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 73: movc r1.w, r2.x, l(0), r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 74: mad r0.w, r0.w, cb0[18].y, r1.w
    r0.w = ((r0.wwww)*(source[18].yyyy)+(r1.wwww)).w;
    // 75: mad r1.xyz, r0.zzzz, r1.xyzx, r0.wwww
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r0.wwww)).xyz;
    // 76: mul r0.x, r0.x, cb0[24].y
    r0.x = ((r0.xxxx)*(source[24].yyyy)).x;
    // 77: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 78: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 79: mad r0.yzw, r1.xxyz, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r1.xxyz)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 80: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 81: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 82: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 83: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 84: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 85: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 86: mul r1.y, r1.y, cb0[23].y
    r1.y = ((r1.yyyy)*(source[23].yyyy)).y;
    // 87: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 88: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 89: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 90: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 91: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 92: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_flow_013_1_tr: d5264a6e7ea1394690402346afbc0de6; selected map 7f81ef1d8d362e88193717f346d5f3bc9143e81b03f25b065f212b6f5346b98d.
float4 ArtistNative4292(ARTIST_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[8u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[4] = ArtistNativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz),1u);
    source[5] = g_ArtistSourceMaterialParameters[6u];
    source[6] = g_ArtistSourceMaterialParameters[7u];
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[5u].xxxx,1u);
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[9].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[10].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].zzzz)).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[11].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
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
    // 28: mul r0.w, r0.y, cb0[9].x
    r0.w = ((r0.yyyy)*(source[9].xxxx)).w;
    // 29: mad r0.x, r0.z, l(0.500000), r0.w
    r0.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).x;
    // 30: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s0, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mov r1.w, cb0[9].w
    r1.w = (source[9].wwww).w;
    // 33: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 34: mad r2.xyzw, r0.xyxy, cb0[3].xyxy, r1.wzzw
    r2.xyzw = ((r0.xyxy)*(source[3].xyxy)+(r1.wzzw)).xyzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s0, l(0.000000)
    r1.z = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: add r2.y, -r0.z, r1.z
    r2.y = ((-(r0.zzzz))+(r1.zzzz)).y;
    // 38: add r2.x, -r0.z, r0.w
    r2.x = ((-(r0.zzzz))+(r0.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 44: div r0.zw, r2.xxxy, r0.zzzz
    r0.zw = ((r2.xxxy)/(r0.zzzz)).zw;
    // 45: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: mov r1.y, v4.w
    r1.y = (v4.wwww).y;
    // 47: mad r1.xy, r0.xyxx, cb0[8].xyxx, r1.xyxx
    r1.xy = ((r0.xyxx)*(source[8].xyxx)+(r1.xyxx)).xy;
    // 48: mad r1.xy, v4.xxxx, r0.zwzz, r1.xyxx
    r1.xy = ((v4.xxxx)*(r0.zwzz)+(r1.xyxx)).xy;
    // 49: mul r0.zw, r0.zzzw, v4.xxxx
    r0.zw = ((r0.zzzw)*(v4.xxxx)).zw;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s4, l(0.000000)
    r1.x = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 51: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 52: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 53: mul r1.x, r1.x, cb0[12].z
    r1.x = ((r1.xxxx)*(source[12].zzzz)).x;
    // 54: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 55: mad r1.yz, r0.xxyx, cb0[7].xxyx, r0.zzwz
    r1.yz = ((r0.xxyx)*(source[7].xxyx)+(r0.zzwz)).yz;
    // 56: mad r0.xy, r0.xyxx, cb0[2].xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(source[2].xyxx)+(r0.zwzz)).xy;
    // 57: add r0.xy, r0.xyxx, cb0[4].xyxx
    r0.xy = ((r0.xyxx)+(source[4].xyxx)).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t3.wxyz, s2, l(0.000000)
    r1.yzw = (ArtistNativeSample2((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 60: add r0.w, r1.y, cb0[12].w
    r0.w = ((r1.yyyy)+(source[12].wwww)).w;
    // 61: max r1.yzw, |r1.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r1.yzw = (max(abs(r1.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 62: log r1.yzw, r1.yyzw
    r1.yzw = (log2(r1.yyzw)).yzw;
    // 63: mul r1.yzw, r1.yyzw, cb0[11].zzzz
    r1.yzw = ((r1.yyzw)*(source[11].zzzz)).yzw;
    // 64: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 65: mad r1.yzw, -cb0[11].wwww, r1.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r1.yzw = ((-(source[11].wwww))*(r1.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 66: mul r0.w, r0.w, cb0[13].x
    r0.w = ((r0.wwww)*(source[13].xxxx)).w;
    // 67: mul r0.w, r0.w, v4.z
    r0.w = ((r0.wwww)*(v4.zzzz)).w;
    // 68: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 69: sincos r0.w, null, r0.w
    { const float4 sourceAngle = r0.wwww; r0.w = (sin(sourceAngle)).w; }
    // 70: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 71: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 72: mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // 73: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 74: mul_sat r0.w, r0.w, cb0[13].z
    r0.w = (saturate((r0.wwww)*(source[13].zzzz))).w;
    // 75: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 76: mul r0.w, r0.w, cb0[13].w
    r0.w = ((r0.wwww)*(source[13].wwww)).w;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t0.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 78: mul_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)*(r1.xxxx))).w;
    // 79: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 80: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 81: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 83: mad r0.xyz, cb0[10].yyyy, r2.xyzx, r0.xyzx
    r0.xyz = ((source[10].yyyy)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 84: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 85: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 86: mul r0.xyz, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 87: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 88: mul r0.xyz, r0.xyzx, cb0[10].wwww
    r0.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 89: add r2.xyz, -cb0[5].xyzx, cb0[6].xyzx
    r2.xyz = ((-(source[5].xyzx))+(source[6].xyzx)).xyz;
    // 90: mad r1.xyz, r1.yzwy, r2.xyzx, cb0[5].xyzx
    r1.xyz = ((r1.yzwy)*(r2.xyzx)+(source[5].xyzx)).xyz;
    // 91: mad r0.xyz, r0.xyzx, v3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(r1.xyzx)).xyz;
    // 92: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 93: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4292Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[1] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[5u].xxxx,1u);
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
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
    // 1: add r0.xy, v1.yxyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v1.yxyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
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
    // 28: mul r0.w, r0.y, cb0[3].x
    r0.w = ((r0.yyyy)*(source[3].xxxx)).w;
    // 29: mad r0.x, r0.z, l(0.500000), r0.w
    r0.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).x;
    // 30: mul r0.zw, r0.xxxy, cb0[0].xxxy
    r0.zw = ((r0.xxxy)*(source[0].xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mov r1.w, cb0[3].w
    r1.w = (source[3].wwww).w;
    // 33: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 34: mad r2.xyzw, r0.xyxy, cb0[0].xyxy, r1.wzzw
    r2.xyzw = ((r0.xyxy)*(source[0].xyxy)+(r1.wzzw)).xyzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ArtistNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s1, l(0.000000)
    r1.z = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: add r2.y, -r0.z, r1.z
    r2.y = ((-(r0.zzzz))+(r1.zzzz)).y;
    // 38: add r2.x, -r0.z, r0.w
    r2.x = ((-(r0.zzzz))+(r0.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 44: div r0.zw, r2.xxxy, r0.zzzz
    r0.zw = ((r2.xxxy)/(r0.zzzz)).zw;
    // 45: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: mov r1.y, v3.w
    r1.y = (v3.wwww).y;
    // 47: mad r1.xy, r0.xyxx, cb0[2].xyxx, r1.xyxx
    r1.xy = ((r0.xyxx)*(source[2].xyxx)+(r1.xyxx)).xy;
    // 48: mad r1.xy, v3.xxxx, r0.zwzz, r1.xyxx
    r1.xy = ((v3.xxxx)*(r0.zwzz)+(r1.xyxx)).xy;
    // 49: mul r0.zw, r0.zzzw, v3.xxxx
    r0.zw = ((r0.zzzw)*(v3.xxxx)).zw;
    // 50: mad r0.xy, r0.xyxx, cb0[1].xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(source[1].xyxx)+(r0.zwzz)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xy = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 52: add r0.xyzw, r0.xyxy, cb0[4].wwww
    r0.xyzw = ((r0.xyxy)+(source[4].wwww)).xyzw;
    // 53: mul r0.xyzw, r0.xyzw, cb0[5].xxxx
    r0.xyzw = ((r0.xyzw)*(source[5].xxxx)).xyzw;
    // 54: mul r0.xyzw, r0.xyzw, v3.zzzz
    r0.xyzw = ((r0.xyzw)*(v3.zzzz)).xyzw;
    // 55: mul r0.xyzw, r0.xyzw, l(6.283185, 6.283185, 6.283185, 6.283185)
    r0.xyzw = ((r0.xyzw)*(float4(6.283185,6.283185,6.283185,6.283185))).xyzw;
    // 56: sincos r0.xyzw, null, r0.xyzw
    { const float4 sourceAngle = r0.xyzw; r0.xyzw = (sin(sourceAngle)).xyzw; }
    // 57: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 58: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 59: mul r0.xyzw, r0.xyzw, cb0[5].yyyy
    r0.xyzw = ((r0.xyzw)*(source[5].yyyy)).xyzw;
    // 60: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 61: mul_sat r0.xyzw, r0.xyzw, cb0[5].zzzz
    r0.xyzw = (saturate((r0.xyzw)*(source[5].zzzz))).xyzw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s4, l(0.000000)
    r1.xy = (ArtistNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 63: max r1.xyzw, |r1.xyxy|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyxy),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 64: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 65: mul r1.xyzw, r1.xyzw, cb0[4].zzzz
    r1.xyzw = ((r1.xyzw)*(source[4].zzzz)).xyzw;
    // 66: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 67: mul r0.xyzw, r0.xyzw, r1.xyzw
    r0.xyzw = ((r0.xyzw)*(r1.xyzw)).xyzw;
    // 68: mul r0.xyzw, r0.xyzw, cb0[5].wwww
    r0.xyzw = ((r0.xyzw)*(source[5].wwww)).xyzw;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v1.xyxx, t0.xyzw, s3, l(0.000000)
    r1.xy = (ArtistNativeSample3((v1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 70: mul_sat r0.xyzw, r0.xyzw, r1.xyxy
    r0.xyzw = (saturate((r0.xyzw)*(r1.xyxy))).xyzw;
    // 71: mul r0.xyzw, r0.xyzw, v2.wwww
    r0.xyzw = ((r0.xyzw)*(v2.wwww)).xyzw;
    // 72: mul r0.xyzw, r0.xyzw, cb0[6].xxxx
    r0.xyzw = ((r0.xyzw)*(source[6].xxxx)).xyzw;
    // 73: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 74: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 75: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 76: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 77: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 78: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 79: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 80: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 81: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 82: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 83: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 84: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 85: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 86: source device depth mapped to centimetre view depth; reconstruction at 88.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 88-91: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 92: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 93: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 94: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 95: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 96: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_de_master_01_16_tr: aafa1b2d458b5746b5f1db2d070d99a4; selected map 4666168621a50b9d884941ea185df1faa5afb1e8b92cd9c220da7a9154af0b96.
float4 ArtistNative4293(ARTIST_NATIVE_INPUT input)
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
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    // 55: add r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)+(r0.zzzz)).w;
    // 56: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 57: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 58: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 59: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 60: mul r0.y, r0.y, cb0[11].z
    r0.y = ((r0.yyyy)*(source[11].zzzz)).y;
    // 61: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 62: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 63: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 64: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 65: movc r0.yzw, r0.zzzz, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 66: mad r0.yzw, r1.xxyz, l(0.000000, 0.150000, 0.150000, 0.150000), r0.yyzw
    r0.yzw = ((r1.xxyz)*(float4(0.000000,0.150000,0.150000,0.150000))+(r0.yyzw)).yzw;
    // 67: mul r1.xyz, r1.xyzx, cb2[3].wwww
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)).xyz;
    // 68: mad r1.xyz, r1.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 69: add r0.yzw, r0.yyzw, cb0[3].xxyz
    r0.yzw = ((r0.yyzw)+(source[3].xxyz)).yzw;
    // 70: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 71: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 72: mul r1.w, r1.w, v7.z
    r1.w = ((r1.wwww)*(v7.zzzz)).w;
    // 73: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 74: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 75: mul r2.yzw, r2.yyyy, cb0[16].xxyz
    r2.yzw = ((r2.yyyy)*(source[16].xxyz)).yzw;
    // 76: mad r2.xyz, r2.xxxx, cb0[15].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[15].xyzx)+(r2.yzwy)).xyz;
    // 77: mul r2.xyz, r2.xyzx, cb0[17].wwww
    r2.xyz = ((r2.xyzx)*(source[17].wwww)).xyz;
    // 78: mad r0.yzw, r2.xxyz, r1.xxyz, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 79: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 81: mad r0.yzw, r1.xxyz, cb0[17].xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(source[17].xxyz)+(r0.yyzw)).yzw;
    // 83: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 84: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 85: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 86: mul r0.y, r0.y, cb0[12].x
    r0.y = ((r0.yyyy)*(source[12].xxxx)).y;
    // 87: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 88: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 89: mul r0.yz, v4.xxyx, cb0[12].yyyy
    r0.yz = ((v4.xxyx)*(source[12].yyyy)).yz;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s5, l(0.000000)
    r0.y = (ArtistNativeSample5((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 91: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 92: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 93: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 94: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 95: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 96: mul r0.x, r0.x, cb0[2].x
    r0.x = ((r0.xxxx)*(source[2].xxxx)).x;
    // 97: add r0.y, -|v4.w|, cb0[0].y
    r0.y = ((-(abs(v4.wwww)))+(source[0].yyyy)).y;
    // 98: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 99: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 100: mul r0.y, r0.y, v4.z
    r0.y = ((r0.yyyy)*(v4.zzzz)).y;
    // 101: mul o0.w, r0.y, r0.x
    output.w = ((r0.yyyy)*(r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_h_de_crack_03_2_tr: 87e3ec1a518e5b478b308c5308ce7773; selected map 83b896c99c7980613b1d34231d989223568290120b09e7e5a5659ffc32fa6962.
float4 ArtistNative4294(ARTIST_NATIVE_INPUT input)
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
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s4, l(0.000000)
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
// fx_k_me_dissolve_041_14_ma: 3fdc451142ff60449bcad09ea75935ed; selected map 46952ce70555ece4342562f06c2e5c2ae54092a56926a1d6efe7803d2bfe0117.
float4 ArtistNative4295(ARTIST_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=input.color; // Native masked mesh particle RGBA prefix.
    source[1] = g_ArtistSourceMaterialParameters[10u];
    source[2] = input.dynamicParameter;
    source[3] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[4u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].xxxx,g_ArtistSourceMaterialParameters[5u].yyyy,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].xxxx,g_ArtistSourceMaterialParameters[4u].zzzz,1u);
    source[7] = g_ArtistSourceMaterialParameters[8u];
    source[8] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].zzzz,g_ArtistSourceMaterialParameters[2u].xxxx,1u);
    source[9] = g_ArtistSourceMaterialParameters[7u];
    source[10] = g_ArtistSourceMaterialParameters[6u];
    source[11] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[12] = ArtistNativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[13] = ArtistNativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[14].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[16].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[16].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[17].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[17].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[18].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[18].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[19].x = (cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[19].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[19].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy)).x;
    source[19].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].yyyy))).x;
    source[20].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.y, cb0[13].xyxx, r0.xyxx
    r1.y = (dot((source[13].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 3: dp2 r1.x, cb0[12].xyxx, r0.xyxx
    r1.x = (dot((source[12].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 4: add r2.xyzw, r1.xyxy, l(0.500000, 0.000000, -0.500000, 0.000000)
    r2.xyzw = ((r1.xyxy)+(float4(0.500000,0.000000,-0.500000,0.000000))).xyzw;
    // 5: mul_sat r0.z, r1.x, l(1000.000000)
    r0.z = (saturate((r1.xxxx)*(float4(1000.000000,1000.000000,1000.000000,1000.000000)))).z;
    // 6: dp2 r0.w, r2.zwzz, r2.zwzz
    r0.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 7: dp2 r1.x, r2.xyxx, r2.xyxx
    r1.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 8: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 9: mad_sat r1.x, -r1.x, cb0[19].w, l(1.000000)
    r1.x = (saturate((-(r1.xxxx))*(source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 10: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 11: mad_sat r0.w, -r0.w, cb0[19].w, l(1.000000)
    r0.w = (saturate((-(r0.wwww))*(source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 12: add r0.w, -r1.x, r0.w
    r0.w = ((-(r1.xxxx))+(r0.wwww)).w;
    // 13: mad_sat r0.z, r0.z, r0.w, r1.x
    r0.z = (saturate((r0.zzzz)*(r0.wwww)+(r1.xxxx))).z;
    // 14: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 15: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 16: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 17: mul r0.w, r0.x, cb0[17].w
    r0.w = ((r0.xxxx)*(source[17].wwww)).w;
    // 18: mad r1.x, cb0[17].z, cb0[17].y, r0.w
    r1.x = ((source[17].zzzz)*(source[17].yyyy)+(r0.wwww)).x;
    // 19: mul r0.w, r0.y, cb0[18].x
    r0.w = ((r0.yyyy)*(source[18].xxxx)).w;
    // 20: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 21: mul r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)*(source[6].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s0, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 23: mad r1.y, cb0[17].z, cb0[18].y, r0.w
    r1.y = ((source[17].zzzz)*(source[18].yyyy)+(r0.wwww)).y;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 25: mad r0.y, r0.y, cb0[18].z, r0.z
    r0.y = ((r0.yyyy)*(source[18].zzzz)+(r0.zzzz)).y;
    // 26: add r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 27: max r0.z, cb0[2].x, l(-0.250000)
    r0.z = (max(source[2].xxxx,float4(-0.250000,-0.250000,-0.250000,-0.250000))).z;
    // 28: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 31: add_sat r0.y, r0.y, cb0[20].x
    r0.y = (saturate((r0.yyyy)+(source[20].xxxx))).y;
    // 32: mul_sat r0.y, r0.y, cb0[0].w
    r0.y = (saturate((r0.yyyy)*(source[0].wwww))).y;
    // 33: add r0.y, r0.y, l(-0.333300)
    r0.y = ((r0.yyyy)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 34: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 35: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) clip(-1.f);
    // 36: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 37: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 38: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 39: mul r0.y, r0.y, cb0[14].z
    r0.y = ((r0.yyyy)*(source[14].zzzz)).y;
    // 40: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 41: mul r0.y, r0.y, cb0[14].w
    r0.y = ((r0.yyyy)*(source[14].wwww)).y;
    // 42: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 43: mul r1.xyz, cb0[7].xyzx, cb0[7].wwww
    r1.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 44: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 45: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 46: mul r1.xy, v4.xyxx, cb0[8].xyxx
    r1.xy = ((v4.xyxx)*(source[8].xyxx)).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 48: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 49: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 50: mad r1.xyz, cb0[15].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 51: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 52: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(source[15].wwww)).xyz;
    // 54: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 55: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 56: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 57: mul r4.xyz, cb0[10].xyzx, cb0[10].wwww
    r4.xyz = ((source[10].xyzx)*(source[10].wwww)).xyz;
    // 58: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 59: mad r1.xyz, r1.xyzx, r2.xyzx, -r3.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // 60: mul r2.xy, v4.xyxx, cb0[11].xyxx
    r2.xy = ((v4.xyxx)*(source[11].xyxx)).xy;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xy = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 62: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 63: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 64: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 66: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 67: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 68: mad r2.xyz, cb0[16].zzzz, r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((source[16].zzzz)*(r2.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 69: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 70: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 71: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 72: dp3_sat r0.w, r4.xyzx, r2.xyzx
    r0.w = (saturate(dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx)).w;
    // 73: log r1.w, r0.w
    r1.w = (log2(r0.wwww)).w;
    // 74: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 75: mul r1.w, r1.w, cb0[16].w
    r1.w = ((r1.wwww)*(source[16].wwww)).w;
    // 76: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 77: mul_sat r1.w, r1.w, cb0[17].x
    r1.w = (saturate((r1.wwww)*(source[17].xxxx))).w;
    // 78: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 79: mad r1.xyz, r0.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 80: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_me_trail_02_4_tr: a197c8691bb49c459070791db5bac738; selected map 333926f5c6bdc25402d3ec373cacfb94e26cac229d874cc9e6899a47d7742c33.
float4 ArtistNative4296(ARTIST_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[12u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[8u].xxxx,g_ArtistSourceMaterialParameters[8u].yyyy,1u);
    source[3] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].xxxx,g_ArtistSourceMaterialParameters[2u].yyyy,1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[7].x = (cos((g_ArtistSourceMaterialParameters[4u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[7].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
    source[9].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[9].w = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[10].z = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[11].x = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[11].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[12].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[13].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[14].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[14].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[15].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[16].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[16].w = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[17].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
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
    // 1: mul r0.x, v2.x, cb0[11].w
    r0.x = ((v2.xxxx)*(source[11].wwww)).x;
    // 2: mul r0.y, v2.y, cb0[12].x
    r0.y = ((v2.yyyy)*(source[12].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 4: mad r1.x, v4.z, cb0[12].w, r0.x
    r1.x = ((v4.zzzz)*(source[12].wwww)+(r0.xxxx)).x;
    // 5: mad r1.y, v4.z, cb0[13].x, r0.y
    r1.y = ((v4.zzzz)*(source[13].xxxx)+(r0.yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.yz, v2.xxyx, cb0[8].yyzy
    r0.yz = ((v2.xxyx)*(source[8].yyzy)).yz;
    // 8: mad r0.yz, v4.zzzz, cb0[9].xxyx, r0.yyzy
    r0.yz = ((v4.zzzz)*(source[9].xxyx)+(r0.yyzy)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mad r0.z, v2.y, cb0[8].z, cb0[8].w
    r0.z = ((v2.yyyy)*(source[8].zzzz)+(source[8].wwww)).z;
    // 11: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 12: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 13: mad r0.zw, v2.xxxy, cb0[13].yyyz, cb0[6].xxxy
    r0.zw = ((v2.xxxy)*(source[13].yyyz)+(source[6].xxxy)).zw;
    // 14: mad r0.zw, cb0[14].yyyy, r0.yyyy, r0.zzzw
    r0.zw = ((source[14].yyyy)*(r0.yyyy)+(r0.zzzw)).zw;
    // 15: add r1.xy, v4.xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r0.zw, r1.xxxx, cb0[14].zzzw, r0.zzzw
    r0.zw = ((r1.xxxx)*(source[14].zzzw)+(r0.zzzw)).zw;
    // 17: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 18: dp2 r2.x, cb0[3].xyxx, r0.zwzz
    r2.x = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 19: dp2 r2.y, cb0[4].xyxx, r0.zwzz
    r2.y = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).y;
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
    // 25: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 26: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 27: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 28: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r0.z, r0.z, cb0[15].y
    r0.z = ((r0.zzzz)*(source[15].yyyy)).z;
    // 31: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 32: mul_sat r0.z, r0.z, cb0[15].z
    r0.z = (saturate((r0.zzzz)*(source[15].zzzz))).z;
    // 33: mul r1.zw, v4.zzzz, cb0[17].zzzw
    r1.zw = ((v4.zzzz)*(source[17].zzzw)).zw;
    // 34: mad r1.zw, cb0[17].xxxy, v2.xxxy, r1.zzzw
    r1.zw = ((source[17].xxxy)*(v2.xxxy)+(r1.zzzw)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t3.yzwx, s4, l(0.000000)
    r0.w = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: mul r1.zw, v4.zzzz, cb0[16].zzzw
    r1.zw = ((v4.zzzz)*(source[16].zzzw)).zw;
    // 37: mad r1.zw, cb0[16].xxxy, v2.xxxy, r1.zzzw
    r1.zw = ((source[16].xxxy)*(v2.xxxy)+(r1.zzzw)).zw;
    // 38: mad r1.zw, r0.wwww, cb0[18].xxxx, r1.zzzw
    r1.zw = ((r0.wwww)*(source[18].xxxx)+(r1.zzzw)).zw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t4.yzwx, s5, l(0.000000)
    r0.w = (ArtistNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: add r0.w, -r1.y, r0.w
    r0.w = ((-(r1.yyyy))+(r0.wwww)).w;
    // 41: mul_sat r0.w, r0.w, cb0[18].y
    r0.w = (saturate((r0.wwww)*(source[18].yyyy))).w;
    // 42: add r1.y, -v2.y, l(1.000000)
    r1.y = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: mul r1.y, r1.y, v2.y
    r1.y = ((r1.yyyy)*(v2.yyyy)).y;
    // 44: mul_sat r1.y, r1.y, cb0[15].w
    r1.y = (saturate((r1.yyyy)*(source[15].wwww))).y;
    // 45: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 46: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 47: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 48: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 49: movc o0.w, r0.x, l(0), r0.z
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 50: mad r0.xz, v2.xxyx, cb0[7].yyzy, cb0[2].xxyx
    r0.xz = ((v2.xxyx)*(source[7].yyzy)+(source[2].xxyx)).xz;
    // 51: mad r0.xy, r0.yyyy, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xzxx
    r0.xy = ((r0.yyyy)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xzxx)).xy;
    // 52: mad r0.xy, r1.xxxx, cb0[9].zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[9].zwzz)+(r0.xyxx)).xy;
    // 53: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 54: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 55: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 56: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 58: mul r0.yz, v4.zzzz, cb0[10].zzwz
    r0.yz = ((v4.zzzz)*(source[10].zzwz)).yz;
    // 59: mad r0.yz, v2.xxyx, cb0[10].xxyx, r0.yyzy
    r0.yz = ((v2.xxyx)*(source[10].xxyx)+(r0.yyzy)).yz;
    // 60: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 61: dp2 r1.x, cb0[3].xyxx, r0.yzyy
    r1.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 62: dp2 r1.y, cb0[4].xyxx, r0.yzyy
    r1.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 63: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 65: add r0.y, -r0.x, r0.y
    r0.y = ((-(r0.xxxx))+(r0.yyyy)).y;
    // 66: mad r0.x, r0.y, l(0.500000), r0.x
    r0.x = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.xxxx)).x;
    // 67: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 68: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 69: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 70: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 72: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 73: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 74: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 75: mad r0.x, r0.y, cb0[11].z, r0.x
    r0.x = ((r0.yyyy)*(source[11].zzzz)+(r0.xxxx)).x;
    // 76: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 77: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_circ_01_2_tr: 653a5bb9cb1a754193f542b0fda91232; selected map b9a00a284058a01b21ea0d6fd6eec705cebc4a6f9f17beb3a1ba17da3210361f.
float4 ArtistNative4297(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_r_pa_flashbang_03_02_tr: 58e1a4d7a3da5d40a9872a0c55cc295f; selected map 64ee4fe278d46b60b34e916f70633058783cf3d035e347377bff587d04bf7e41.
float4 ArtistNative4298(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative4298Distortion(ARTIST_NATIVE_INPUT input)
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
// bfx_d_pa_smoke_ulit_01_14_dt_tr: c09ef819991e34448b72ea36ee9d1242; selected map 0fa557db3ddb45f15aabefc9953ec128db20431b08a8a53f2bf6c1c3e973117b.
float4 ArtistNative4299(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_flowblur_01_3_tr: d7c41b166d92df4bbdbdd2d52910bb43; selected map cf5307bb357c63ef65726a3cf7a9af8c452377b63b4908657a4e9b8ae66dff59.
float4 ArtistNative4300(ARTIST_NATIVE_INPUT input)
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
    r1.xyw = (Read_EffectSceneColor(LinearClampUVSampler, (r2.xyxx).xy).xywz).xyw;
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
    r3.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
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
    r5.yzw = (Read_EffectSceneColorBias(LinearClampUVSampler, (r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).wxyz).yzw;
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
// fx_d_pa_master_01_013_ad: e253f2c3b158e843acc23d5fa6a0794e; selected map 466cafd8fbf0d743505ce2028974dcf96ad6804ee695c1c73c2501f2401a2c6e.
float4 ArtistNative4301(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[3].x = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[2u].wwww)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
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
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
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
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
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
    // 32: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 33: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 34: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 35: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
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
float4 ArtistNative4301Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_me_master_01_018_ad: c9cf79250d787c4b8faa16cdb042f143; selected map e8bae060e5bc8394b3e0755ed4d89824b43afc42dcdbf4d111327e2960a7236f.
float4 ArtistNative4302(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative4302Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_k_me_floorstrm_14_ad: 00cd866295cc494dba563a698dea14df; selected map 25dad5f905785b3aa0b8fbe349a5a17d05495ab568518506e1e61c8b6a29d983.
float4 ArtistNative4303(ARTIST_NATIVE_INPUT input)
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
float4 ArtistNative4303Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_z_pa_grad_01_1_tr: c4e3bde2576c2b46b3dd6ac9b693f4ed; selected map 154931681653007185176db8267f10aa3331472347eae2285d7d0b44dd35daf8.
float4 ArtistNative4336(ARTIST_NATIVE_INPUT input)
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
    // 1: mad r0.x, cb0[2].x, v4.z, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.z, cb0[2].x
    r0.y = ((v4.zzzz)*(source[2].xxxx)).y;
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
float4 ArtistNative4336Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_d_pa_ring_11_07_dt_ad: b07f6ad0a70bbf44a5ed8f1ad817c879; selected map eedffa0c57130ed172e5afae3088b3e8488984d2d288f52cab6713e6f713ed31.
float4 ArtistNative4337(ARTIST_NATIVE_INPUT input)
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
    // 38: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 39: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4337Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_linearwave_01_40_tr: 0ab678ead8004140be6413856cae2e82; selected map 494b8c5cb81aaac4d2b591cadda8382843f95f52af3672c3d9f2a3fa71e30354.
float4 ArtistNative4338(ARTIST_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[12u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ArtistSourceMaterialParameters[11u];
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[8].z = ((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[8].w = (sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[10].x = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[10].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[11].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[12].z = (floor(g_ArtistSourceMaterialParameters[8u].zzzz)).x;
    source[12].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[13].w = ((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[14].x = (sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[14].z = (cos((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].w = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[15].y = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[15].w = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[16].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[16].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[16].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[17].w = (floor(g_ArtistSourceMaterialParameters[7u].yyyy)).x;
    source[18].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[18].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[19].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[19].y = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[19].z = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[20].x = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 1: mul r0.z, v2.x, cb0[16].w
    r0.z = ((v2.xxxx)*(source[16].wwww)).z;
    // 2: mul r0.w, v2.y, cb0[17].x
    r0.w = ((v2.yyyy)*(source[17].xxxx)).w;
    // 3: add r0.xy, r0.zwzz, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t4.xyzw, s6, l(0.000000)
    r0.xy = (ArtistNativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 6: mad r1.xy, cb0[14].wwww, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[14].wwww)*(r0.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 7: mul r1.y, r1.y, cb0[15].y
    r1.y = ((r1.yyyy)*(source[15].yyyy)).y;
    // 8: mad r2.x, r1.x, cb0[15].x, cb0[15].z
    r2.x = ((r1.xxxx)*(source[15].xxxx)+(source[15].zzzz)).x;
    // 9: mad r2.y, cb0[15].w, v4.y, r1.y
    r2.y = ((source[15].wwww)*(v4.yyyy)+(r1.yyyy)).y;
    // 10: add r1.xy, r2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 11: dp2 r2.x, cb0[4].xyxx, r1.xyxx
    r2.x = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 12: dp2 r2.y, cb0[5].xyxx, r1.xyxx
    r2.y = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 13: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: mad r1.zw, v2.xxxy, cb0[16].xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((v2.xxxy)*(source[16].xxxy)+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s5, l(0.000000)
    r1.zw = (ArtistNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 16: mul r2.x, v4.w, cb0[16].z
    r2.x = ((v4.wwww)*(source[16].zzzz)).x;
    // 17: mad r1.xy, r2.xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((r2.xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 18: mad r0.xy, cb0[17].yyyy, r0.xyxx, r1.xyxx
    r0.xy = ((source[17].yyyy)*(r0.xyxx)+(r1.xyxx)).xy;
    // 19: mul r1.x, v4.z, cb0[13].y
    r1.x = ((v4.zzzz)*(source[13].yyyy)).x;
    // 20: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s4, r1.x
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (r1.xxxx).x, true).xyzw).x;
    // 21: mad r0.x, cb0[17].w, -r0.x, r0.x
    r0.x = ((source[17].wwww)*(-(r0.xxxx))+(r0.xxxx)).x;
    // 22: mul r0.x, r0.x, cb0[18].x
    r0.x = ((r0.xxxx)*(source[18].xxxx)).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.y, r0.y, cb0[18].y
    r0.y = ((r0.yyyy)*(source[18].yyyy)).y;
    // 26: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 27: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 28: mul r0.y, v4.x, cb0[10].z
    r0.y = ((v4.xxxx)*(source[10].zzzz)).y;
    // 29: mad r1.xy, cb0[9].zzzz, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[9].zzzz)*(r0.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 30: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 31: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: mad r2.y, r1.y, cb0[10].x, r0.y
    r2.y = ((r1.yyyy)*(source[10].xxxx)+(r0.yyyy)).y;
    // 33: mad r2.x, r1.x, cb0[9].w, cb0[10].y
    r2.x = ((r1.xxxx)*(source[9].wwww)+(source[10].yyyy)).x;
    // 34: add r0.yw, r2.xxxy, l(0.000000, -0.500000, 0.000000, -0.500000)
    r0.yw = ((r2.xxxy)+(float4(0.000000,-0.500000,0.000000,-0.500000))).yw;
    // 35: dp2 r1.x, cb0[2].xyxx, r0.ywyy
    r1.x = (dot((source[2].xyxx).xy,(r0.ywyy).xy).xxxx).x;
    // 36: dp2 r1.y, cb0[3].xyxx, r0.ywyy
    r1.y = (dot((source[3].xyxx).xy,(r0.ywyy).xy).xxxx).y;
    // 37: add r0.yw, r1.xxxy, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.xxxy)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 38: mul r1.x, v2.x, cb0[10].w
    r1.x = ((v2.xxxx)*(source[10].wwww)).x;
    // 39: mul r1.yzw, v2.yyxy, cb0[11].xxzw
    r1.yzw = ((v2.yyxy)*(source[11].xxzw)).yzw;
    // 40: add r1.xyzw, r1.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r1.xyzw = ((r1.xyzw)+(float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.xy = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s3, l(0.000000)
    r1.zw = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 43: mul r2.x, v4.w, cb0[11].y
    r2.x = ((v4.wwww)*(source[11].yyyy)).x;
    // 44: mad r0.yw, r2.xxxx, r1.xxxy, r0.yyyw
    r0.yw = ((r2.xxxx)*(r1.xxxy)+(r0.yyyw)).yw;
    // 45: mad r0.yw, cb0[12].xxxx, r1.zzzw, r0.yyyw
    r0.yw = ((source[12].xxxx)*(r1.zzzw)+(r0.yyyw)).yw;
    // 46: mul r1.x, v4.z, cb0[8].x
    r1.x = ((v4.zzzz)*(source[8].xxxx)).x;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t2.yxzw, s1, r1.x
    r0.y = (ArtistNativeSample1((r0.ywyy).xy, (r1.xxxx).x, true).yxzw).y;
    // 48: mad r0.y, cb0[12].z, -r0.y, r0.y
    r0.y = ((source[12].zzzz)*(-(r0.yyyy))+(r0.yyyy)).y;
    // 49: mul r0.y, r0.y, cb0[12].w
    r0.y = ((r0.yyyy)*(source[12].wwww)).y;
    // 50: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 51: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 52: mul r0.w, r0.w, cb0[13].x
    r0.w = ((r0.wwww)*(source[13].xxxx)).w;
    // 53: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 54: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 55: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 56: mul r0.x, r0.x, cb0[18].z
    r0.x = ((r0.xxxx)*(source[18].zzzz)).x;
    // 57: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 58: mul r0.y, r0.y, cb0[19].y
    r0.y = ((r0.yyyy)*(source[19].yyyy)).y;
    // 59: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 60: mul r0.y, r0.y, cb0[19].z
    r0.y = ((r0.yyyy)*(source[19].zzzz)).y;
    // 61: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 62: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 63: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 64: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 65: mad r1.x, cb0[19].w, l(10.000000), l(10.000000)
    r1.x = ((source[19].wwww)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 66: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 67: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 68: mul r0.w, r0.w, cb0[20].x
    r0.w = ((r0.wwww)*(source[20].xxxx)).w;
    // 69: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 70: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 71: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 72: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 73: mad r0.yz, v2.xxyx, cb0[7].yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((v2.xxyx)*(source[7].yyzy)+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 74: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t6.wxyz, s0, cb0[7].x
    r0.yzw = (ArtistNativeSample0((r0.yzyy).xy, (source[7].xxxx).x, true).wxyz).yzw;
    // 75: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 76: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 77: mad r0.yzw, cb0[7].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 78: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 79: mul r0.xyz, r0.xyzx, cb0[18].wwww
    r0.xyz = ((r0.xyzx)*(source[18].wwww)).xyz;
    // 80: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 81: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 82: mul r0.xyz, r0.xyzx, cb0[19].xxxx
    r0.xyz = ((r0.xyzx)*(source[19].xxxx)).xyz;
    // 83: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 84: mul_sat r0.xyz, r0.xyzx, cb0[6].xyzx
    r0.xyz = (saturate((r0.xyzx)*(source[6].xyzx))).xyz;
    // 85: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 86: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4338Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[4].z = ((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[4].w = (sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos((g_ArtistSourceMaterialParameters[3u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[5].w = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[8].z = (floor(g_ArtistSourceMaterialParameters[8u].zzzz)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[9].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[9].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[9].w = ((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[10].x = (sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].z = (cos((g_ArtistSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].w = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[11].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[12].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[13].w = (floor(g_ArtistSourceMaterialParameters[7u].yyyy)).x;
    source[14].x = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[14].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[15].z = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[15].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[16].x = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 1: mul r0.z, v1.x, cb0[12].w
    r0.z = ((v1.xxxx)*(source[12].wwww)).z;
    // 2: mul r0.w, v1.y, cb0[13].x
    r0.w = ((v1.yyyy)*(source[13].xxxx)).w;
    // 3: add r0.xy, r0.zwzz, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t4.xyzw, s6, l(0.000000)
    r0.xy = (ArtistNativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: add r0.zw, v1.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v1.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 6: mad r1.xy, cb0[10].wwww, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[10].wwww)*(r0.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 7: mul r1.y, r1.y, cb0[11].y
    r1.y = ((r1.yyyy)*(source[11].yyyy)).y;
    // 8: mad r2.x, r1.x, cb0[11].x, cb0[11].z
    r2.x = ((r1.xxxx)*(source[11].xxxx)+(source[11].zzzz)).x;
    // 9: mad r2.y, cb0[11].w, v3.y, r1.y
    r2.y = ((source[11].wwww)*(v3.yyyy)+(r1.yyyy)).y;
    // 10: add r1.xy, r2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 11: dp2 r2.x, cb0[2].xyxx, r1.xyxx
    r2.x = (dot((source[2].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 12: dp2 r2.y, cb0[3].xyxx, r1.xyxx
    r2.y = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 13: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: mad r1.zw, v1.xxxy, cb0[12].xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((v1.xxxy)*(source[12].xxxy)+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s5, l(0.000000)
    r1.zw = (ArtistNativeSample5((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 16: mul r2.x, v3.w, cb0[12].z
    r2.x = ((v3.wwww)*(source[12].zzzz)).x;
    // 17: mad r1.xy, r2.xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((r2.xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 18: mad r0.xy, cb0[13].yyyy, r0.xyxx, r1.xyxx
    r0.xy = ((source[13].yyyy)*(r0.xyxx)+(r1.xyxx)).xy;
    // 19: mul r1.x, v3.z, cb0[9].y
    r1.x = ((v3.zzzz)*(source[9].yyyy)).x;
    // 20: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s4, r1.x
    r0.x = (ArtistNativeSample4((r0.xyxx).xy, (r1.xxxx).x, true).xyzw).x;
    // 21: mad r0.x, cb0[13].w, -r0.x, r0.x
    r0.x = ((source[13].wwww)*(-(r0.xxxx))+(r0.xxxx)).x;
    // 22: mul r0.x, r0.x, cb0[14].x
    r0.x = ((r0.xxxx)*(source[14].xxxx)).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: mul r0.y, r0.y, cb0[14].y
    r0.y = ((r0.yyyy)*(source[14].yyyy)).y;
    // 26: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 27: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 28: mul r0.y, v3.x, cb0[6].z
    r0.y = ((v3.xxxx)*(source[6].zzzz)).y;
    // 29: mad r1.xy, cb0[5].zzzz, r0.zwzz, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((source[5].zzzz)*(r0.zwzz)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 30: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 31: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: mad r2.y, r1.y, cb0[6].x, r0.y
    r2.y = ((r1.yyyy)*(source[6].xxxx)+(r0.yyyy)).y;
    // 33: mad r2.x, r1.x, cb0[5].w, cb0[6].y
    r2.x = ((r1.xxxx)*(source[5].wwww)+(source[6].yyyy)).x;
    // 34: add r0.yw, r2.xxxy, l(0.000000, -0.500000, 0.000000, -0.500000)
    r0.yw = ((r2.xxxy)+(float4(0.000000,-0.500000,0.000000,-0.500000))).yw;
    // 35: dp2 r1.x, cb0[0].xyxx, r0.ywyy
    r1.x = (dot((source[0].xyxx).xy,(r0.ywyy).xy).xxxx).x;
    // 36: dp2 r1.y, cb0[1].xyxx, r0.ywyy
    r1.y = (dot((source[1].xyxx).xy,(r0.ywyy).xy).xxxx).y;
    // 37: add r0.yw, r1.xxxy, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.xxxy)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 38: mul r1.x, v1.x, cb0[6].w
    r1.x = ((v1.xxxx)*(source[6].wwww)).x;
    // 39: mul r1.yzw, v1.yyxy, cb0[7].xxzw
    r1.yzw = ((v1.yyxy)*(source[7].xxzw)).yzw;
    // 40: add r1.xyzw, r1.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r1.xyzw = ((r1.xyzw)+(float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.xy = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s3, l(0.000000)
    r1.zw = (ArtistNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 43: mul r2.x, v3.w, cb0[7].y
    r2.x = ((v3.wwww)*(source[7].yyyy)).x;
    // 44: mad r0.yw, r2.xxxx, r1.xxxy, r0.yyyw
    r0.yw = ((r2.xxxx)*(r1.xxxy)+(r0.yyyw)).yw;
    // 45: mad r0.yw, cb0[8].xxxx, r1.zzzw, r0.yyyw
    r0.yw = ((source[8].xxxx)*(r1.zzzw)+(r0.yyyw)).yw;
    // 46: mul r1.x, v3.z, cb0[4].x
    r1.x = ((v3.zzzz)*(source[4].xxxx)).x;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t2.yxzw, s1, r1.x
    r0.y = (ArtistNativeSample1((r0.ywyy).xy, (r1.xxxx).x, true).yxzw).y;
    // 48: mad r0.y, cb0[8].z, -r0.y, r0.y
    r0.y = ((source[8].zzzz)*(-(r0.yyyy))+(r0.yyyy)).y;
    // 49: mul r0.y, r0.y, cb0[8].w
    r0.y = ((r0.yyyy)*(source[8].wwww)).y;
    // 50: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 51: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 52: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 53: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 54: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 55: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 56: mul r0.x, r0.x, cb0[14].z
    r0.x = ((r0.xxxx)*(source[14].zzzz)).x;
    // 57: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 58: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 59: mul r0.y, r0.y, cb0[15].y
    r0.y = ((r0.yyyy)*(source[15].yyyy)).y;
    // 60: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 61: mul r0.y, r0.y, cb0[15].z
    r0.y = ((r0.yyyy)*(source[15].zzzz)).y;
    // 62: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 63: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 64: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 65: mad r0.w, cb0[15].w, l(10.000000), l(10.000000)
    r0.w = ((source[15].wwww)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 66: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 67: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 68: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 69: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 70: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 71: mul r0.x, r0.x, v2.w
    r0.x = ((r0.xxxx)*(v2.wwww)).x;
    // 72: mul r0.x, r0.x, cb0[16].y
    r0.x = ((r0.xxxx)*(source[16].yyyy)).x;
    // 73: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 74: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 75: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 76: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 77: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 78: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 79: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 80: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 81: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 82: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 83: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 84: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 85: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 86: source device depth mapped to centimetre view depth; reconstruction at 88.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 88-91: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 92: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 93: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 94: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 95: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 96: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_master_01_011_ds_tr: 83cd182fd8cbcb4089b301c2ef7a2abe; selected map 72ad261496577b67ba709c22ca2b41a958b443163ebe98a0ac211a38fe3c6e32.
float4 ArtistNative4339(ARTIST_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,g_ArtistSourceMaterialParameters[3u].xxxx,1u);
    source[4].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[8].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].yyyy)).x;
    source[8].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 9: mul r0.z, r0.z, cb0[6].w
    r0.z = ((r0.zzzz)*(source[6].wwww)).z;
    // 10: mad r1.y, cb0[5].x, r0.y, r0.z
    r1.y = ((source[5].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
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
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
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
    // 43: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 44: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 45: source device depth mapped to centimetre view depth; reconstruction at 47.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 47-50: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 51: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 52: add r0.w, -cb0[9].y, l(1.000000)
    r0.w = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 54: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 55: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 56: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 57: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 58: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4339Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].wwww,g_ArtistSourceMaterialParameters[3u].xxxx,1u);
    source[1].x = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[1].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[1].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[1].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[2].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[2].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[3].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[3].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[3].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[4].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[5].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[3u].yyyy)).x;
    source[5].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
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
    // 20: mul r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)*(source[6].zzzz)).x;
    // 21: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 22: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 23: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 24: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 25: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 26: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 27: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 28: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 29: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 31: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 32: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 33: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 34: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 35: source device depth mapped to centimetre view depth; reconstruction at 37.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 37-40: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 41: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 42: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 43: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 44: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 45: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_r_pa_twirl_03_02_ad: 1f377575d000c741acc21359a3b57bff; selected map 891633497c18c98704b26f55b0266657677b4f2793e6d9085d01e3f266d8838a.
float4 ArtistNative4340(ARTIST_NATIVE_INPUT input)
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

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4340Distortion(ARTIST_NATIVE_INPUT input)
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
// fx_k_pa_linearflow_01_01_tr: 01a927bf53ed8e46a35e8b10c8578ba5; selected map 339afa1673519302795061a9dabe1f9ce052aa315322bf0f578a6ae42fa6bbae.
float4 ArtistNative4341(ARTIST_NATIVE_INPUT input)
{
    float4 source[28]; [unroll] for (uint i=0u; i<28u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[15u];
    source[2] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[5] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)+g_ArtistSourceMaterialParameters[1u].xxxx),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)+g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[6] = g_ArtistSourceMaterialParameters[13u];
    source[7] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].zzzz,g_ArtistSourceMaterialParameters[6u].wwww,1u);
    source[10] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].wwww)+g_ArtistSourceMaterialParameters[5u].yyyy),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)+g_ArtistSourceMaterialParameters[5u].zzzz),1u);
    source[11] = g_ArtistSourceMaterialParameters[14u];
    source[12] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[13] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
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
    source[23].z = (g_ArtistSourceMaterialParameters[11u].wwww).x;
    source[23].w = (g_ArtistSourceMaterialParameters[12u].yyyy).x;
    source[24].x = (g_ArtistSourceMaterialParameters[12u].zzzz).x;
    source[24].y = (g_ArtistSourceMaterialParameters[12u].xxxx).x;
    source[24].z = (g_ArtistSourceMaterialParameters[11u].zzzz).x;
    source[24].w = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[25].x = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[25].y = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[25].z = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[25].w = ((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[26].x = (sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[26].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[26].z = (cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[26].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[27].x = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[27].y = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
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
    // 1: mad r0.xy, v2.xyxx, cb0[9].xyxx, cb0[10].xyxx
    r0.xy = ((v2.xyxx)*(source[9].xyxx)+(source[10].xyxx)).xy;
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
    // 14: add r0.z, v4.y, cb0[20].w
    r0.z = ((v4.yyyy)+(source[20].wwww)).z;
    // 15: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mad r1.zw, cb0[19].wwww, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[19].wwww)*(r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 17: mad r2.y, r1.w, cb0[20].y, r0.z
    r2.y = ((r1.wwww)*(source[20].yyyy)+(r0.zzzz)).y;
    // 18: mad r2.x, r1.z, cb0[20].x, cb0[20].z
    r2.x = ((r1.zzzz)*(source[20].xxxx)+(source[20].zzzz)).x;
    // 19: add r0.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 20: dp2 r2.x, cb0[7].xyxx, r0.zwzz
    r2.x = (dot((source[7].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 21: dp2 r2.y, cb0[8].xyxx, r0.zwzz
    r2.y = (dot((source[8].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 22: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 23: mul r1.z, v4.w, cb0[21].z
    r1.z = ((v4.wwww)*(source[21].zzzz)).z;
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
    // 32: mad r1.zw, v2.xxxy, cb0[4].xxxy, cb0[5].xxxy
    r1.zw = ((v2.xxxy)*(source[4].xxxy)+(source[5].xxxy)).zw;
    // 33: add r2.xyzw, r1.zwzw, l(0.200000, 0.000000, 0.000000, 0.200000)
    r2.xyzw = ((r1.zwzw)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t0.yzxw, s2, l(0.000000)
    r1.z = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s2, l(0.000000)
    r1.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t0.xyzw, s2, l(0.000000)
    r2.x = (ArtistNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: add r2.y, -r1.z, r2.x
    r2.y = ((-(r1.zzzz))+(r2.xxxx)).y;
    // 38: add r2.x, -r1.z, r1.w
    r2.x = ((-(r1.zzzz))+(r1.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 44: div r1.zw, r2.xxxy, r1.zzzz
    r1.zw = ((r2.xxxy)/(r1.zzzz)).zw;
    // 45: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: add r2.x, v4.x, cb0[16].x
    r2.x = ((v4.xxxx)+(source[16].xxxx)).x;
    // 47: mad r2.yz, cb0[15].xxxx, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((source[15].xxxx)*(r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 48: mad r3.y, r2.z, cb0[15].z, r2.x
    r3.y = ((r2.zzzz)*(source[15].zzzz)+(r2.xxxx)).y;
    // 49: mad r3.x, r2.y, cb0[15].y, cb0[15].w
    r3.x = ((r2.yyyy)*(source[15].yyyy)+(source[15].wwww)).x;
    // 50: add r2.xy, r3.xyxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 51: dp2 r3.x, cb0[2].xyxx, r2.xyxx
    r3.x = (dot((source[2].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 52: dp2 r3.y, cb0[3].xyxx, r2.xyxx
    r3.y = (dot((source[3].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 53: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mul r2.z, v4.z, cb0[17].x
    r2.z = ((v4.zzzz)*(source[17].xxxx)).z;
    // 55: mad r1.zw, r2.zzzz, r1.zzzw, r2.xxxy
    r1.zw = ((r2.zzzz)*(r1.zzzw)+(r2.xxxy)).zw;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t1.xyzw, s1, l(-1.000000)
    r2.xyz = (ArtistNativeSample1((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 57: mad r2.xyzw, cb0[17].zzzz, -r2.xxyz, r2.xxyz
    r2.xyzw = ((source[17].zzzz)*(-(r2.xxyz))+(r2.xxyz)).xyzw;
    // 58: mul r2.xyzw, r2.xyzw, cb0[17].wwww
    r2.xyzw = ((r2.xyzw)*(source[17].wwww)).xyzw;
    // 59: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 60: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 61: mul r2.xyzw, r2.xyzw, cb0[18].xxxx
    r2.xyzw = ((r2.xyzw)*(source[18].xxxx)).xyzw;
    // 62: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 63: add r0.x, r0.x, -r2.x
    r0.x = ((r0.xxxx)+(-(r2.xxxx))).x;
    // 64: mad r0.x, cb0[23].z, r0.x, r2.x
    r0.x = ((source[23].zzzz)*(r0.xxxx)+(r2.xxxx)).x;
    // 65: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 66: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 67: mul r0.x, r0.x, cb0[23].w
    r0.x = ((r0.xxxx)*(source[23].wwww)).x;
    // 68: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 69: mul r0.x, r0.x, cb0[24].x
    r0.x = ((r0.xxxx)*(source[24].xxxx)).x;
    // 70: mad r1.z, cb0[24].y, l(10.000000), l(10.000000)
    r1.z = ((source[24].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 71: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 72: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 74: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 75: mul r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)*(r2.xxxx)).z;
    // 76: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 77: mul r1.z, r1.z, cb0[24].z
    r1.z = ((r1.zzzz)*(source[24].zzzz)).z;
    // 78: movc r1.z, r1.w, l(0), r1.z
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 79: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 80: dp2 r3.x, cb0[12].xyxx, r1.xyxx
    r3.x = (dot((source[12].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 81: dp2 r3.y, cb0[13].xyxx, r1.xyxx
    r3.y = (dot((source[13].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 82: add r1.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 83: mul r1.xy, r1.xyxx, cb0[25].xyxx
    r1.xy = ((r1.xyxx)*(source[25].xyxx)).xy;
    // 84: mad r3.x, cb0[16].z, cb0[24].w, r1.x
    r3.x = ((source[16].zzzz)*(source[24].wwww)+(r1.xxxx)).x;
    // 85: mad r3.y, cb0[16].z, cb0[26].w, r1.y
    r3.y = ((source[16].zzzz)*(source[26].wwww)+(r1.yyyy)).y;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r3.xyxx, t4.xyzw, s5, l(0.000000)
    r1.x = (ArtistNativeSample5((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 87: add r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 88: add r1.y, -v3.w, l(1.000000)
    r1.y = ((-(v3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 90: mul_sat r1.x, r1.x, cb0[27].x
    r1.x = (saturate((r1.xxxx)*(source[27].xxxx))).x;
    // 91: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 92: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 93: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 94: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 95: add r1.xyz, -r2.yzwy, r0.xxxx
    r1.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 96: mad r1.xyz, cb0[18].yyyy, r1.xyzx, r2.yzwy
    r1.xyz = ((source[18].yyyy)*(r1.xyzx)+(r2.yzwy)).xyz;
    // 97: mad r2.xy, v2.xyxx, cb0[14].xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((v2.xyxx)*(source[14].xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 98: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t5.xyzw, s0, l(-1.000000)
    r2.xyz = (ArtistNativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 99: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 100: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 101: mad r2.xyz, cb0[14].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[14].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 102: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 103: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 104: add r3.xyz, -r0.yzwy, r0.xxxx
    r3.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 105: mad r0.xyz, cb0[22].wwww, r3.xyzx, r0.yzwy
    r0.xyz = ((source[22].wwww)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 106: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 107: mul r0.xyz, r0.xyzx, cb0[11].xyzx
    r0.xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 108: mad r0.xyz, r1.xyzx, cb0[6].xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(source[6].xyzx)+(r0.xyzx)).xyz;
    // 109: mul r0.xyz, r0.xyzx, cb0[23].xxxx
    r0.xyz = ((r0.xyzx)*(source[23].xxxx)).xyz;
    // 110: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 111: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 112: mul r0.xyz, r0.xyzx, cb0[23].yyyy
    r0.xyz = ((r0.xyzx)*(source[23].yyyy)).xyz;
    // 113: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 114: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 115: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4341Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[23]; [unroll] for (uint i=0u; i<23u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[1] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[3] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)+g_ArtistSourceMaterialParameters[1u].xxxx),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)+g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].zzzz,g_ArtistSourceMaterialParameters[6u].wwww,1u);
    source[7] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].wwww)+g_ArtistSourceMaterialParameters[5u].yyyy),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)+g_ArtistSourceMaterialParameters[5u].zzzz),1u);
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
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
    source[18].z = (g_ArtistSourceMaterialParameters[11u].wwww).x;
    source[18].w = (g_ArtistSourceMaterialParameters[12u].yyyy).x;
    source[19].x = (g_ArtistSourceMaterialParameters[12u].zzzz).x;
    source[19].y = (g_ArtistSourceMaterialParameters[12u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[11u].zzzz).x;
    source[19].w = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[20].x = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[20].y = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[20].z = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[20].w = ((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[21].x = (sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[21].z = (cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[22].x = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[22].y = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
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
    // 1: mad r0.xy, v1.xyxx, cb0[6].xyxx, cb0[7].xyxx
    r0.xy = ((v1.xyxx)*(source[6].xyxx)+(source[7].xyxx)).xy;
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
    // 14: add r0.z, v3.y, cb0[15].w
    r0.z = ((v3.yyyy)+(source[15].wwww)).z;
    // 15: add r1.xy, v1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mad r1.zw, cb0[14].wwww, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[14].wwww)*(r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 17: mad r2.y, r1.w, cb0[15].y, r0.z
    r2.y = ((r1.wwww)*(source[15].yyyy)+(r0.zzzz)).y;
    // 18: mad r2.x, r1.z, cb0[15].x, cb0[15].z
    r2.x = ((r1.zzzz)*(source[15].xxxx)+(source[15].zzzz)).x;
    // 19: add r0.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 20: dp2 r2.x, cb0[4].xyxx, r0.zwzz
    r2.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 21: dp2 r2.y, cb0[5].xyxx, r0.zwzz
    r2.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 22: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 23: mul r1.z, v3.w, cb0[16].z
    r1.z = ((v3.wwww)*(source[16].zzzz)).z;
    // 24: mad r0.xy, r1.zzzz, r0.xyxx, r0.zwzz
    r0.xy = ((r1.zzzz)*(r0.xyxx)+(r0.zwzz)).xy;
    // 25: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t3.xyzw, s3, l(-1.000000)
    r0.xy = (ArtistNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 26: mad r0.xyzw, cb0[17].xxxx, -r0.xyxy, r0.xyxy
    r0.xyzw = ((source[17].xxxx)*(-(r0.xyxy))+(r0.xyxy)).xyzw;
    // 27: mul r0.xyzw, r0.xyzw, cb0[17].yyyy
    r0.xyzw = ((r0.xyzw)*(source[17].yyyy)).xyzw;
    // 28: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 30: mul r0.xyzw, r0.xyzw, cb0[17].zzzz
    r0.xyzw = ((r0.xyzw)*(source[17].zzzz)).xyzw;
    // 31: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 32: mad r1.zw, v1.xxxy, cb0[2].xxxy, cb0[3].xxxy
    r1.zw = ((v1.xxxy)*(source[2].xxxy)+(source[3].xxxy)).zw;
    // 33: add r2.xyzw, r1.zwzw, l(0.200000, 0.000000, 0.000000, 0.200000)
    r2.xyzw = ((r1.zwzw)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t0.yzxw, s2, l(0.000000)
    r1.z = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s2, l(0.000000)
    r1.w = (ArtistNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t0.xyzw, s2, l(0.000000)
    r2.x = (ArtistNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: add r2.y, -r1.z, r2.x
    r2.y = ((-(r1.zzzz))+(r2.xxxx)).y;
    // 38: add r2.x, -r1.z, r1.w
    r2.x = ((-(r1.zzzz))+(r1.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 44: div r1.zw, r2.xxxy, r1.zzzz
    r1.zw = ((r2.xxxy)/(r1.zzzz)).zw;
    // 45: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: add r2.x, v3.x, cb0[11].x
    r2.x = ((v3.xxxx)+(source[11].xxxx)).x;
    // 47: mad r2.yz, cb0[10].xxxx, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((source[10].xxxx)*(r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 48: mad r3.y, r2.z, cb0[10].z, r2.x
    r3.y = ((r2.zzzz)*(source[10].zzzz)+(r2.xxxx)).y;
    // 49: mad r3.x, r2.y, cb0[10].y, cb0[10].w
    r3.x = ((r2.yyyy)*(source[10].yyyy)+(source[10].wwww)).x;
    // 50: add r2.xy, r3.xyxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 51: dp2 r3.x, cb0[0].xyxx, r2.xyxx
    r3.x = (dot((source[0].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 52: dp2 r3.y, cb0[1].xyxx, r2.xyxx
    r3.y = (dot((source[1].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 53: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mul r2.z, v3.z, cb0[12].x
    r2.z = ((v3.zzzz)*(source[12].xxxx)).z;
    // 55: mad r1.zw, r2.zzzz, r1.zzzw, r2.xxxy
    r1.zw = ((r2.zzzz)*(r1.zzzw)+(r2.xxxy)).zw;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(-1.000000)
    r1.zw = (ArtistNativeSample1((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zwxy).zw;
    // 57: mad r2.xyzw, cb0[12].zzzz, -r1.zwzw, r1.zwzw
    r2.xyzw = ((source[12].zzzz)*(-(r1.zwzw))+(r1.zwzw)).xyzw;
    // 58: mul r2.xyzw, r2.xyzw, cb0[12].wwww
    r2.xyzw = ((r2.xyzw)*(source[12].wwww)).xyzw;
    // 59: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 60: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 61: mul r2.xyzw, r2.xyzw, cb0[13].xxxx
    r2.xyzw = ((r2.xyzw)*(source[13].xxxx)).xyzw;
    // 62: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 63: add r0.xyzw, r0.xyzw, -r2.zwzw
    r0.xyzw = ((r0.xyzw)+(-(r2.zwzw))).xyzw;
    // 64: mad r0.xyzw, cb0[18].zzzz, r0.xyzw, r2.xyzw
    r0.xyzw = ((source[18].zzzz)*(r0.xyzw)+(r2.xyzw)).xyzw;
    // 65: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 66: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 67: mul r0.xyzw, r0.xyzw, cb0[18].wwww
    r0.xyzw = ((r0.xyzw)*(source[18].wwww)).xyzw;
    // 68: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 69: mul r0.xyzw, r0.xyzw, cb0[19].xxxx
    r0.xyzw = ((r0.xyzw)*(source[19].xxxx)).xyzw;
    // 70: mad r1.z, cb0[19].y, l(10.000000), l(10.000000)
    r1.z = ((source[19].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 71: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 72: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 74: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 75: mul r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)*(r2.xxxx)).z;
    // 76: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 77: mul r1.z, r1.z, cb0[19].z
    r1.z = ((r1.zzzz)*(source[19].zzzz)).z;
    // 78: movc r1.z, r1.w, l(0), r1.z
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 79: mul r0.xyzw, r0.xyzw, r1.zzzz
    r0.xyzw = ((r0.xyzw)*(r1.zzzz)).xyzw;
    // 80: dp2 r2.x, cb0[8].xyxx, r1.xyxx
    r2.x = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 81: dp2 r2.y, cb0[9].xyxx, r1.xyxx
    r2.y = (dot((source[9].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 82: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 83: mul r1.xy, r1.xyxx, cb0[20].xyxx
    r1.xy = ((r1.xyxx)*(source[20].xyxx)).xy;
    // 84: mad r2.x, cb0[11].z, cb0[19].w, r1.x
    r2.x = ((source[11].zzzz)*(source[19].wwww)+(r1.xxxx)).x;
    // 85: mad r2.y, cb0[11].z, cb0[21].w, r1.y
    r2.y = ((source[11].zzzz)*(source[21].wwww)+(r1.yyyy)).y;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t4.xyzw, s5, l(0.000000)
    r1.x = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 87: add r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 88: add r1.y, -v2.w, l(1.000000)
    r1.y = ((-(v2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 90: mul_sat r1.x, r1.x, cb0[22].x
    r1.x = (saturate((r1.xxxx)*(source[22].xxxx))).x;
    // 91: mul r1.x, r1.x, v2.w
    r1.x = ((r1.xxxx)*(v2.wwww)).x;
    // 92: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 93: mul r0.xyzw, r0.xyzw, cb0[22].yyyy
    r0.xyzw = ((r0.xyzw)*(source[22].yyyy)).xyzw;
    // 94: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 95: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 96: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 97: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 98: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 99: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 100: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 101: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 102: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 103: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 104: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 105: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 106: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 107: source device depth mapped to centimetre view depth; reconstruction at 109.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 109-112: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 113: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 114: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 115: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 116: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 117: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_ninjaflow_01_07_tr: 28c8301200413e449da688a3c218b53c; selected map 59c3ce6d2422c20c3d5f9a7f3ee8be3fc9e95ab17dc130fa4df0392babefe1a4.
float4 ArtistNative4342(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 4: mul r0.zw, v2.xxxy, cb0[3].xxxy
    r0.zw = ((v2.xxxy)*(source[3].xxxy)).zw;
    // 5: mad r1.xy, -r0.xyxx, cb0[5].zzzz, -r0.zwzz
    r1.xy = ((-(r0.xyxx))*(source[5].zzzz)+(-(r0.zwzz))).xy;
    // 6: mad r0.zw, v4.xxxx, r1.xxxy, r0.zzzw
    r0.zw = ((v4.xxxx)*(r1.xxxy)+(r0.zzzw)).zw;
    // 7: add r0.zw, r0.zzzw, v4.yyyy
    r0.zw = ((r0.zzzw)+(v4.yyyy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 9: mul r0.w, r0.z, cb0[8].x
    r0.w = ((r0.zzzz)*(source[8].xxxx)).w;
    // 10: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 11: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 12: mul r0.z, r0.z, cb0[7].x
    r0.z = ((r0.zzzz)*(source[7].xxxx)).z;
    // 13: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 14: mul r0.z, r0.z, cb0[7].y
    r0.z = ((r0.zzzz)*(source[7].yyyy)).z;
    // 15: mad r1.xy, v2.xyxx, cb0[4].xyxx, r0.wwww
    r1.xy = ((v2.xyxx)*(source[4].xyxx)+(r0.wwww)).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 17: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 18: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 19: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 20: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 21: mul r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)*(source[8].zzzz)).w;
    // 22: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 23: mad r1.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 25: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 26: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 27: mul r1.zw, r1.zzzw, cb0[8].wwww
    r1.zw = ((r1.zzzw)*(source[8].wwww)).zw;
    // 28: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 29: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 30: mul r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)*(r1.xxxx)).w;
    // 31: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 32: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 34: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 35: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 36: mul r0.zw, v2.xxxy, cb0[2].xxxy
    r0.zw = ((v2.xxxy)*(source[2].xxxy)).zw;
    // 37: mad r0.xy, -r0.xyxx, cb0[5].zzzz, -r0.zwzz
    r0.xy = ((-(r0.xyxx))*(source[5].zzzz)+(-(r0.zwzz))).xy;
    // 38: mad r0.xy, v4.xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((v4.xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r0.xyz, cb0[5].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 43: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 45: mul r0.xyz, r0.xyzx, cb0[6].xxxx
    r0.xyz = ((r0.xyzx)*(source[6].xxxx)).xyz;
    // 46: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 47: mul r0.xyz, r0.xyzx, cb0[6].yyyy
    r0.xyz = ((r0.xyzx)*(source[6].yyyy)).xyz;
    // 48: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 49: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_k_pa_ninjaflow_01_08_tr: 28c8301200413e449da688a3c218b53c; selected map 59c3ce6d2422c20c3d5f9a7f3ee8be3fc9e95ab17dc130fa4df0392babefe1a4.
float4 ArtistNative4343(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].xxxx,1u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].zzzz,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[4] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].zzzz,g_ArtistSourceMaterialParameters[2u].wwww,1u);
    source[5].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 4: mul r0.zw, v2.xxxy, cb0[3].xxxy
    r0.zw = ((v2.xxxy)*(source[3].xxxy)).zw;
    // 5: mad r1.xy, -r0.xyxx, cb0[5].zzzz, -r0.zwzz
    r1.xy = ((-(r0.xyxx))*(source[5].zzzz)+(-(r0.zwzz))).xy;
    // 6: mad r0.zw, v4.xxxx, r1.xxxy, r0.zzzw
    r0.zw = ((v4.xxxx)*(r1.xxxy)+(r0.zzzw)).zw;
    // 7: add r0.zw, r0.zzzw, v4.yyyy
    r0.zw = ((r0.zzzw)+(v4.yyyy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 9: mul r0.w, r0.z, cb0[8].x
    r0.w = ((r0.zzzz)*(source[8].xxxx)).w;
    // 10: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 11: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 12: mul r0.z, r0.z, cb0[7].x
    r0.z = ((r0.zzzz)*(source[7].xxxx)).z;
    // 13: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 14: mul r0.z, r0.z, cb0[7].y
    r0.z = ((r0.zzzz)*(source[7].yyyy)).z;
    // 15: mad r1.xy, v2.xyxx, cb0[4].xyxx, r0.wwww
    r1.xy = ((v2.xyxx)*(source[4].xyxx)+(r0.wwww)).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 17: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 18: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 19: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 20: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 21: mul r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)*(source[8].zzzz)).w;
    // 22: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 23: mad r1.xy, v2.yxyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.yxyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: add r1.xy, -|r1.xyxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(abs(r1.xyxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 25: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 26: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 27: mul r1.zw, r1.zzzw, cb0[8].wwww
    r1.zw = ((r1.zzzw)*(source[8].wwww)).zw;
    // 28: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 29: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 30: mul r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)*(r1.xxxx)).w;
    // 31: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 32: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 34: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 35: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 36: mul r0.zw, v2.xxxy, cb0[2].xxxy
    r0.zw = ((v2.xxxy)*(source[2].xxxy)).zw;
    // 37: mad r0.xy, -r0.xyxx, cb0[5].zzzz, -r0.zwzz
    r0.xy = ((-(r0.xyxx))*(source[5].zzzz)+(-(r0.zwzz))).xy;
    // 38: mad r0.xy, v4.xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((v4.xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r0.xyz, cb0[5].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 43: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 45: mul r0.xyz, r0.xyzx, cb0[6].xxxx
    r0.xyz = ((r0.xyzx)*(source[6].xxxx)).xyz;
    // 46: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 47: mul r0.xyz, r0.xyzx, cb0[6].yyyy
    r0.xyz = ((r0.xyzx)*(source[6].yyyy)).xyz;
    // 48: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 49: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_o_pa_ap_23_3_ad: 1f3e2fdc0f35504493c2d7fca9bd10b1; selected map 99fb0a09edfa19b9d94ab099ff0afebeefd112ee9e07786e44da96f7a5ba1180.
float4 ArtistNative4344(ARTIST_NATIVE_INPUT input)
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

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_spritetransition_01_10_tr: b1e4c19a96c4354a8fc13fa82c36f820; selected map 226bef295ab723129c70389fd700a996cd6b7858835fcf98a1278cfae5ab16ce.
float4 ArtistNative4345(ARTIST_NATIVE_INPUT input)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[9u];
    source[2] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[7u].xxxx,g_ArtistSourceMaterialParameters[7u].yyyy,1u);
    source[3] = ArtistNativeAppend((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx),(g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].yyyy),1u);
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].zzzz,g_ArtistSourceMaterialParameters[0u].wwww,1u);
    source[7] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[8] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ArtistSourceMaterialParameters[8u];
    source[11] = ArtistNativeAppend(ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].zzzz,g_ArtistSourceMaterialParameters[5u].wwww,1u);
    source[13] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].zzzz,g_ArtistSourceMaterialParameters[4u].wwww,1u);
    source[14] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[15] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[16] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[17] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[18] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].zzzz,1u);
    source[19] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].wwww*g_ArtistSourceMaterialTime.xxxx)*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[20].x = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[20].y = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[20].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[20].w = ((g_ArtistSourceMaterialParameters[0u].yyyy*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[21].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[21].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[21].z = (ArtistNativePeriodic((g_ArtistSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[22].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[22].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[22].z = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[22].w = ((g_ArtistSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[23].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[23].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialParameters[2u].xxxx*g_ArtistSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[23].z = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[23].w = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[24].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[24].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[24].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
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
    // 2: mul r0.y, v4.y, l(0.050000)
    r0.y = ((v4.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 3: add r0.xy, r0.xyxx, v2.xyxx
    r0.xy = ((r0.xyxx)+(v2.xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.xy, r0.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))).xy;
    // 7: mad r0.zw, cb0[2].xxxy, v2.xxxy, r0.xxxy
    r0.zw = ((source[2].xxxy)*(v2.xxxy)+(r0.xxxy)).zw;
    // 8: mad r0.xy, cb0[6].xyxx, v2.xyxx, r0.xyxx
    r0.xy = ((source[6].xyxx)*(v2.xyxx)+(r0.xyxx)).xy;
    // 9: add r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)+(source[7].xyxx)).xy;
    // 10: add r0.zw, r0.zzzw, cb0[3].xxxy
    r0.zw = ((r0.zzzw)+(source[3].xxxy)).zw;
    // 11: add r0.xyzw, r0.xyzw, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((r0.xyzw)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 12: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 13: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 14: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 16: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 17: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 18: mul r0.w, r0.w, cb0[20].x
    r0.w = ((r0.wwww)*(source[20].xxxx)).w;
    // 19: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 20: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 21: add r0.w, v4.x, cb0[20].y
    r0.w = ((v4.xxxx)+(source[20].yyyy)).w;
    // 22: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 23: dp2 r1.x, cb0[8].xyxx, r0.xyxx
    r1.x = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 24: dp2 r1.y, cb0[9].xyxx, r0.xyxx
    r1.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 25: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 28: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 29: mul r0.x, r0.x, cb0[21].w
    r0.x = ((r0.xxxx)*(source[21].wwww)).x;
    // 30: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 31: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 32: add r0.y, v4.z, cb0[22].x
    r0.y = ((v4.zzzz)+(source[22].xxxx)).y;
    // 33: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 34: add r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)+(r0.zzzz)).x;
    // 35: mul_sat r0.xy, r0.xxxx, l(10.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xxxx)*(float4(10.000000,8.000000,0.000000,0.000000)))).xy;
    // 36: mad r0.zw, v2.xxxy, cb0[14].xxxy, cb0[15].xxxy
    r0.zw = ((v2.xxxy)*(source[14].xxxy)+(source[15].xxxy)).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s4, l(0.000000)
    r0.z = (ArtistNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 38: mad r1.xy, v2.xyxx, cb0[12].xyxx, cb0[13].xyxx
    r1.xy = ((v2.xyxx)*(source[12].xyxx)+(source[13].xyxx)).xy;
    // 39: mad r0.zw, r0.zzzz, cb0[23].zzzz, r1.xxxy
    r0.zw = ((r0.zzzz)*(source[23].zzzz)+(r1.xxxy)).zw;
    // 40: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 41: dp2 r1.x, cb0[16].xyxx, r0.zwzz
    r1.x = (dot((source[16].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 42: dp2 r1.y, cb0[17].xyxx, r0.zwzz
    r1.y = (dot((source[17].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 43: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t4.yzxw, s5, l(0.000000)
    r0.z = (ArtistNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 45: mul r0.z, r0.z, cb0[23].w
    r0.z = ((r0.zzzz)*(source[23].wwww)).z;
    // 46: mul r0.z, r0.z, r0.x
    r0.z = ((r0.zzzz)*(r0.xxxx)).z;
    // 47: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 48: mul r0.x, r0.x, cb0[22].y
    r0.x = ((r0.xxxx)*(source[22].yyyy)).x;
    // 49: mul r0.y, r0.z, v3.w
    r0.y = ((r0.zzzz)*(v3.wwww)).y;
    // 50: add r0.zw, v2.xxxy, cb0[19].xxxy
    r0.zw = ((v2.xxxy)+(source[19].xxxy)).zw;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t5.yzxw, s6, l(0.000000)
    r0.z = (ArtistNativeSample6((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 52: add r1.xy, v2.xyxx, cb0[18].xyxx
    r1.xy = ((v2.xyxx)+(source[18].xyxx)).xy;
    // 53: mad r0.zw, r0.zzzz, cb0[24].xxxx, r1.xxxy
    r0.zw = ((r0.zzzz)*(source[24].xxxx)+(r1.xxxy)).zw;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t6.yzxw, s7, l(0.000000)
    r0.z = (ArtistNativeSample7((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 55: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 56: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 57: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 58: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 59: mul r0.w, r0.w, cb0[24].z
    r0.w = ((r0.wwww)*(source[24].zzzz)).w;
    // 60: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 61: mul_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)*(r0.yyyy))).y;
    // 62: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 63: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 64: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 65: mul r0.y, r0.y, cb0[22].z
    r0.y = ((r0.yyyy)*(source[22].zzzz)).y;
    // 66: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 67: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 68: mul r0.yzw, v3.xxyz, cb0[10].xxyz
    r0.yzw = ((v3.xxyz)*(source[10].xxyz)).yzw;
    // 69: mad r1.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), cb0[11].xyxx
    r1.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(source[11].xyxx)).xy;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t7.xyzw, s3, l(0.000000)
    r1.x = (ArtistNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 71: mul r1.y, |r1.x|, |r1.x|
    r1.y = ((abs(r1.xxxx))*(abs(r1.xxxx))).y;
    // 72: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 73: mul r0.yzw, r0.yyzw, r1.yyyy
    r0.yzw = ((r0.yyzw)*(r1.yyyy)).yzw;
    // 74: movc r0.yzw, r1.xxxx, l(0,0,0,0), r0.yyzw
    r0.yzw = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyzw)).yzw;
    // 75: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 76: mad r0.xyz, v3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000), r0.xyzx
    r0.xyz = ((v3.xyzx)*(float4(0.010000,0.010000,0.010000,0.000000))+(r0.xyzx)).xyz;
    // 77: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 78: add r0.xyz, r0.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 79: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_simpleaura_01_03_tr: f02128aceb072349977fac7d3f99d721; selected map 44db2c34358cc1fbc83961db105fb04b85cadc076ceac806f7cdb8bfe774ace5.
float4 ArtistNative4346(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[7u];
    source[2].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[2].w = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[3].x = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[3].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[3].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[3].w = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[5].w = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
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
    // 1: mul r0.xy, v4.zzzz, cb0[2].zwzz
    r0.xy = ((v4.zzzz)*(source[2].zwzz)).xy;
    // 2: mad r0.xy, v2.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v2.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (ArtistNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 9: mad r0.y, -r0.x, cb0[5].w, l(1.000000)
    r0.y = ((-(r0.xxxx))*(source[5].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 10: mad r0.yz, r0.yyyy, r0.yyyy, v2.xxyx
    r0.yz = ((r0.yyyy)*(r0.yyyy)+(v2.xxyx)).yz;
    // 11: mad r1.x, cb0[6].z, v4.y, cb0[6].w
    r1.x = ((source[6].zzzz)*(v4.yyyy)+(source[6].wwww)).x;
    // 12: mad r1.y, cb0[7].x, v4.y, cb0[7].y
    r1.y = ((source[7].xxxx)*(v4.yyyy)+(source[7].yyyy)).y;
    // 13: mad r0.yz, r0.yyzy, cb0[6].xxyx, r1.xxyx
    r0.yz = ((r0.yyzy)*(source[6].xxyx)+(r1.xxyx)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s3, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 16: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 17: mul r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)*(source[7].zzzz)).z;
    // 18: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 19: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 20: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 21: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 22: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 23: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 24: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 26: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 27: mad r1.x, cb0[5].z, l(10.000000), l(10.000000)
    r1.x = ((source[5].zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 28: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 29: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 30: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 31: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 32: mad r1.xy, r0.xxxx, cb0[3].yyyy, v2.xyxx
    r1.xy = ((r0.xxxx)*(source[3].yyyy)+(v2.xyxx)).xy;
    // 33: mad r1.zw, cb0[4].xxxz, v4.xxxx, cb0[4].yyyw
    r1.zw = ((source[4].xxxz)*(v4.xxxx)+(source[4].yyyw)).zw;
    // 34: mad r1.xy, r1.xyxx, cb0[3].zwzz, r1.zwzz
    r1.xy = ((r1.xyxx)*(source[3].zwzz)+(r1.zwzz)).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 37: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 38: mul r1.x, r1.x, cb0[5].x
    r1.x = ((r1.xxxx)*(source[5].xxxx)).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: mul_sat r1.x, r1.x, cb0[5].y
    r1.x = (saturate((r1.xxxx)*(source[5].yyyy))).x;
    // 41: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 42: dp2 r0.x, r0.xxxx, r0.wwww
    r0.x = (dot((r0.xxxx).xy,(r0.wwww).xy).xxxx).x;
    // 43: mul r0.w, r0.z, r0.x
    r0.w = ((r0.zzzz)*(r0.xxxx)).w;
    // 44: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 45: mul r0.y, r0.y, l(5.000000)
    r0.y = ((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 46: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 47: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 48: mul r0.w, r0.w, cb0[8].x
    r0.w = ((r0.wwww)*(source[8].xxxx)).w;
    // 49: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 50: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 51: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 52: mad r0.x, r0.x, r0.z, r0.y
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).x;
    // 53: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 54: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 55: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 56: source device depth mapped to centimetre view depth; reconstruction at 58.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 58-61: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 62: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 63: add r0.z, -cb0[8].z, l(1.000000)
    r0.z = ((-(source[8].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 64: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 65: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 66: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 67: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 68: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 69: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 70: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_fire_18_01_dt_tr: 17bc89b1218b4b41a6b98675ed3797d9; selected map c2f141e62e6ba08c656d0fc952842285c2661670c35801ba330f0e656a068e3b.
float4 ArtistNative4347(ARTIST_NATIVE_INPUT input)
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
// fx_o_pa_sy_08_2_ts_tr: 201c5765d89297459daec45853e82d62; selected map 5a23fdbd3f9a93a4ff0b80d0575a9d7d44caea27aa93dca573be998c8a730aca.
float4 ArtistNative4348(ARTIST_NATIVE_INPUT input)
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
    // 2: mul r1.xyz, r0.xyzx, cb0[3].xxxx
    r1.xyz = ((r0.xyzx)*(source[3].xxxx)).xyz;
    // 3: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 4: mad r0.yzw, -cb0[3].xxxx, r0.xxyz, r0.wwww
    r0.yzw = ((-(source[3].xxxx))*(r0.xxyz)+(r0.wwww)).yzw;
    // 5: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 6: mad r0.yzw, cb0[3].yyyy, r0.yyzw, r1.xxyz
    r0.yzw = ((source[3].yyyy)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 7: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 8: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 9: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 10: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 11: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 12: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 16: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 17: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_linearflow_02_07_tr: 3b833100bdaa7648bc65f846ace0dc55; selected map 233683ed70a4e669e0630774e5b2a5b9428ef83f6b460d85cb457077bafcd09a.
float4 ArtistNative4349(ARTIST_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[16u];
    source[3] = input.dynamicParameter;
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[2u].yyyy,g_ArtistSourceMaterialParameters[2u].zzzz,1u);
    source[7] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)+g_ArtistSourceMaterialParameters[1u].xxxx),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].wwww)+g_ArtistSourceMaterialParameters[1u].yyyy),1u);
    source[8] = g_ArtistSourceMaterialParameters[13u];
    source[9] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[6u].zzzz,g_ArtistSourceMaterialParameters[6u].wwww,1u);
    source[12] = ArtistNativeAppend(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].wwww)+g_ArtistSourceMaterialParameters[5u].yyyy),((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[6u].xxxx)+g_ArtistSourceMaterialParameters[5u].zzzz),1u);
    source[13] = g_ArtistSourceMaterialParameters[14u];
    source[14] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[15] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[16].x = (g_ArtistSourceMaterialParameters[9u].yyyy).x;
    source[16].y = (g_ArtistSourceMaterialParameters[9u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[9u].xxxx).x;
    source[16].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[17].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[17].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[18].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[18].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[18].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[18].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[19].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[19].z = (floor(g_ArtistSourceMaterialParameters[4u].yyyy)).x;
    source[19].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[20].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[20].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[20].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[20].w = ((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[21].x = (sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[21].z = (cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[21].w = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[22].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[22].y = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[22].z = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[22].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[23].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[23].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[23].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[23].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[24].x = (floor(g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[24].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[24].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[24].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[25].x = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[25].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[25].z = (g_ArtistSourceMaterialParameters[12u].yyyy).x;
    source[25].w = (g_ArtistSourceMaterialParameters[12u].zzzz).x;
    source[26].x = (g_ArtistSourceMaterialParameters[11u].wwww).x;
    source[26].y = (g_ArtistSourceMaterialParameters[11u].zzzz).x;
    source[26].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[26].w = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[27].x = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[27].y = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[27].z = ((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[27].w = (sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[28].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[28].y = (cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[28].z = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[28].w = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[29].x = (g_ArtistSourceMaterialParameters[12u].xxxx).x;
    source[29].y = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
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
    // 1: mad r0.xy, v4.wzww, cb0[6].xyxx, cb0[7].xyxx
    r0.xy = ((v4.wzww)*(source[6].xyxx)+(source[7].xyxx)).xy;
    // 2: add r1.xyzw, r0.xyxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r1.xyzw = ((r0.xyxy)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    // 14: add r0.z, cb0[3].x, cb0[18].x
    r0.z = ((source[3].xxxx)+(source[18].xxxx)).z;
    // 15: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mad r1.zw, cb0[17].xxxx, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[17].xxxx)*(r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 17: mad r2.y, r1.w, cb0[17].z, r0.z
    r2.y = ((r1.wwww)*(source[17].zzzz)+(r0.zzzz)).y;
    // 18: mad r2.x, r1.z, cb0[17].y, cb0[17].w
    r2.x = ((r1.zzzz)*(source[17].yyyy)+(source[17].wwww)).x;
    // 19: add r0.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 20: dp2 r2.x, cb0[4].xyxx, r0.zwzz
    r2.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 21: dp2 r2.y, cb0[5].xyxx, r0.zwzz
    r2.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 22: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 23: mul r1.z, cb0[3].z, cb0[19].x
    r1.z = ((source[3].zzzz)*(source[19].xxxx)).z;
    // 24: mad r0.xy, r1.zzzz, r0.xyxx, r0.zwzz
    r0.xy = ((r1.zzzz)*(r0.xyxx)+(r0.zwzz)).xy;
    // 25: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(-1.000000)
    r0.xyz = (ArtistNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 26: mad r0.xyzw, cb0[19].zzzz, -r0.xxyz, r0.xxyz
    r0.xyzw = ((source[19].zzzz)*(-(r0.xxyz))+(r0.xxyz)).xyzw;
    // 27: mul r0.xyzw, r0.xyzw, cb0[19].wwww
    r0.xyzw = ((r0.xyzw)*(source[19].wwww)).xyzw;
    // 28: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 30: mul r0.xyzw, r0.xyzw, cb0[20].xxxx
    r0.xyzw = ((r0.xyzw)*(source[20].xxxx)).xyzw;
    // 31: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 32: mad r1.zw, v4.wwwz, cb0[11].xxxy, cb0[12].xxxy
    r1.zw = ((v4.wwwz)*(source[11].xxxy)+(source[12].xxxy)).zw;
    // 33: add r2.xyzw, r1.zwzw, l(0.200000, 0.000000, 0.000000, 0.200000)
    r2.xyzw = ((r1.zwzw)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r1.z = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t2.yzwx, s4, l(0.000000)
    r1.w = (ArtistNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t2.xyzw, s4, l(0.000000)
    r2.x = (ArtistNativeSample4((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: add r2.y, -r1.z, r2.x
    r2.y = ((-(r1.zzzz))+(r2.xxxx)).y;
    // 38: add r2.x, -r1.z, r1.w
    r2.x = ((-(r1.zzzz))+(r1.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 44: div r1.zw, r2.xxxy, r1.zzzz
    r1.zw = ((r2.xxxy)/(r1.zzzz)).zw;
    // 45: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: add r2.x, cb0[3].y, cb0[22].z
    r2.x = ((source[3].yyyy)+(source[22].zzzz)).x;
    // 47: mad r2.yz, cb0[21].wwww, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((source[21].wwww)*(r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 48: mad r2.x, r2.y, cb0[22].x, r2.x
    r2.x = ((r2.yyyy)*(source[22].xxxx)+(r2.xxxx)).x;
    // 49: mad r2.y, r2.z, cb0[22].y, cb0[22].w
    r2.y = ((r2.zzzz)*(source[22].yyyy)+(source[22].wwww)).y;
    // 50: add r2.xy, r2.xyxx, l(-1.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(-1.500000,-0.500000,0.000000,0.000000))).xy;
    // 51: dp2 r3.x, cb0[9].xyxx, r2.xyxx
    r3.x = (dot((source[9].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 52: dp2 r3.y, cb0[10].xyxx, r2.xyxx
    r3.y = (dot((source[10].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 53: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mul r2.z, cb0[3].w, cb0[23].z
    r2.z = ((source[3].wwww)*(source[23].zzzz)).z;
    // 55: mad r1.zw, r2.zzzz, r1.zzzw, r2.xxxy
    r1.zw = ((r2.zzzz)*(r1.zzzw)+(r2.xxxy)).zw;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t3.xyzw, s3, l(-1.000000)
    r2.xyz = (ArtistNativeSample3((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 57: mad r2.xyzw, cb0[24].xxxx, -r2.xxyz, r2.xxyz
    r2.xyzw = ((source[24].xxxx)*(-(r2.xxyz))+(r2.xxyz)).xyzw;
    // 58: mul r2.xyzw, r2.xyzw, cb0[24].yyyy
    r2.xyzw = ((r2.xyzw)*(source[24].yyyy)).xyzw;
    // 59: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 60: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 61: mul r2.xyzw, r2.xyzw, cb0[24].zzzz
    r2.xyzw = ((r2.xyzw)*(source[24].zzzz)).xyzw;
    // 62: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 63: mul r0.x, r0.x, r2.x
    r0.x = ((r0.xxxx)*(r2.xxxx)).x;
    // 64: max r0.x, r0.x, l(0.000001)
    r0.x = (max(r0.xxxx,float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 65: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 66: mul r0.x, r0.x, cb0[25].z
    r0.x = ((r0.xxxx)*(source[25].zzzz)).x;
    // 67: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 68: mul r0.x, r0.x, cb0[25].w
    r0.x = ((r0.xxxx)*(source[25].wwww)).x;
    // 69: mad r1.z, cb0[26].x, l(10.000000), l(10.000000)
    r1.z = ((source[26].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 70: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 71: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 73: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: mul r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)*(r2.xxxx)).z;
    // 75: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 76: mul r1.z, r1.z, cb0[26].y
    r1.z = ((r1.zzzz)*(source[26].yyyy)).z;
    // 77: movc r1.z, r1.w, l(0), r1.z
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 78: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 79: dp2 r3.x, cb0[14].xyxx, r1.xyxx
    r3.x = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 80: dp2 r3.y, cb0[15].xyxx, r1.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 81: add r1.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 82: mul r1.x, r1.x, cb0[26].w
    r1.x = ((r1.xxxx)*(source[26].wwww)).x;
    // 83: mul r1.y, r1.y, cb0[27].x
    r1.y = ((r1.yyyy)*(source[27].xxxx)).y;
    // 84: mad r3.y, cb0[18].z, cb0[28].z, r1.y
    r3.y = ((source[18].zzzz)*(source[28].zzzz)+(r1.yyyy)).y;
    // 85: mad r3.x, cb0[18].z, cb0[26].z, r1.x
    r3.x = ((source[18].zzzz)*(source[26].zzzz)+(r1.xxxx)).x;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r3.xyxx, t4.xyzw, s5, l(0.000000)
    r1.x = (ArtistNativeSample5((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 87: add r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 88: add r1.y, -cb0[1].w, l(1.000000)
    r1.y = ((-(source[1].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 90: mul_sat r1.x, r1.x, cb0[28].w
    r1.x = (saturate((r1.xxxx)*(source[28].wwww))).x;
    // 91: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 92: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 93: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 94: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 95: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 96: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 97: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 98: mul r1.y, r1.y, cb0[29].x
    r1.y = ((r1.yyyy)*(source[29].xxxx)).y;
    // 99: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 100: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 101: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 102: movc o0.w, r1.x, l(0), r0.x
    output.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 103: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 104: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 105: mad r0.xyz, cb0[20].yyyy, r1.xyzx, r0.yzwy
    r0.xyz = ((source[20].yyyy)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 106: mad r1.xy, v4.xyxx, cb0[16].xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)*(source[16].xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 107: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s0, l(-1.000000)
    r1.xyz = (ArtistNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 108: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 110: mad r1.xyz, cb0[16].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 111: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 112: dp3 r0.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: add r3.xyz, -r2.yzwy, r0.wwww
    r3.xyz = ((-(r2.yzwy))+(r0.wwww)).xyz;
    // 114: mad r2.xyz, cb0[24].wwww, r3.xyzx, r2.yzwy
    r2.xyz = ((source[24].wwww)*(r3.xyzx)+(r2.yzwy)).xyz;
    // 115: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 116: mul r1.xyz, r1.xyzx, cb0[13].xyzx
    r1.xyz = ((r1.xyzx)*(source[13].xyzx)).xyz;
    // 117: mad r0.xyz, r0.xyzx, cb0[8].xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(source[8].xyzx)+(r1.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, cb0[25].xxxx
    r0.xyz = ((r0.xyzx)*(source[25].xxxx)).xyz;
    // 119: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 120: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 121: mul r0.xyz, r0.xyzx, cb0[25].yyyy
    r0.xyz = ((r0.xyzx)*(source[25].yyyy)).xyz;
    // 122: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 123: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 124: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4349Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
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
    source[10] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[12].z = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[12].w = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[13].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[13].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[14].z = (floor(g_ArtistSourceMaterialParameters[4u].yyyy)).x;
    source[14].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[15].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_ArtistSourceMaterialParameters[7u].zzzz).x;
    source[15].w = ((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[16].x = (sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[16].z = (cos((g_ArtistSourceMaterialParameters[7u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].w = (g_ArtistSourceMaterialParameters[7u].wwww).x;
    source[17].x = (g_ArtistSourceMaterialParameters[8u].xxxx).x;
    source[17].y = (g_ArtistSourceMaterialParameters[8u].yyyy).x;
    source[17].z = (g_ArtistSourceMaterialParameters[7u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[7u].yyyy).x;
    source[18].x = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[18].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[18].z = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[19].x = (floor(g_ArtistSourceMaterialParameters[0u].xxxx)).x;
    source[19].y = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[19].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[19].w = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[20].x = (g_ArtistSourceMaterialParameters[8u].wwww).x;
    source[20].y = (g_ArtistSourceMaterialParameters[8u].zzzz).x;
    source[20].z = (g_ArtistSourceMaterialParameters[12u].yyyy).x;
    source[20].w = (g_ArtistSourceMaterialParameters[12u].zzzz).x;
    source[21].x = (g_ArtistSourceMaterialParameters[11u].wwww).x;
    source[21].y = (g_ArtistSourceMaterialParameters[11u].zzzz).x;
    source[21].z = (g_ArtistSourceMaterialParameters[10u].xxxx).x;
    source[21].w = (g_ArtistSourceMaterialParameters[10u].wwww).x;
    source[22].x = (g_ArtistSourceMaterialParameters[11u].xxxx).x;
    source[22].y = (g_ArtistSourceMaterialParameters[10u].zzzz).x;
    source[22].z = ((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[22].w = (sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[23].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[23].y = (cos((g_ArtistSourceMaterialParameters[10u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[23].z = (g_ArtistSourceMaterialParameters[10u].yyyy).x;
    source[23].w = (g_ArtistSourceMaterialParameters[9u].wwww).x;
    source[24].x = (g_ArtistSourceMaterialParameters[12u].xxxx).x;
    source[24].y = (g_ArtistSourceMaterialParameters[11u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v4 = float4(input.tangentView,1.f); // native texcoord6
    float4 v5 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mad r0.xy, v2.wzww, cb0[4].xyxx, cb0[5].xyxx
    r0.xy = ((v2.wzww)*(source[4].xyxx)+(source[5].xyxx)).xy;
    // 2: add r1.xyzw, r0.xyxy, l(0.200000, 0.000000, 0.000000, 0.200000)
    r1.xyzw = ((r0.xyxy)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    // 14: add r0.z, cb0[1].x, cb0[13].x
    r0.z = ((source[1].xxxx)+(source[13].xxxx)).z;
    // 15: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: mad r1.zw, cb0[12].xxxx, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((source[12].xxxx)*(r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 17: mad r2.y, r1.w, cb0[12].z, r0.z
    r2.y = ((r1.wwww)*(source[12].zzzz)+(r0.zzzz)).y;
    // 18: mad r2.x, r1.z, cb0[12].y, cb0[12].w
    r2.x = ((r1.zzzz)*(source[12].yyyy)+(source[12].wwww)).x;
    // 19: add r0.zw, r2.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 20: dp2 r2.x, cb0[2].xyxx, r0.zwzz
    r2.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 21: dp2 r2.y, cb0[3].xyxx, r0.zwzz
    r2.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 22: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 23: mul r1.z, cb0[1].z, cb0[14].x
    r1.z = ((source[1].zzzz)*(source[14].xxxx)).z;
    // 24: mad r0.xy, r1.zzzz, r0.xyxx, r0.zwzz
    r0.xy = ((r1.zzzz)*(r0.xyxx)+(r0.zwzz)).xy;
    // 25: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(-1.000000)
    r0.xy = (ArtistNativeSample1((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 26: mad r0.xyzw, cb0[14].zzzz, -r0.xyxy, r0.xyxy
    r0.xyzw = ((source[14].zzzz)*(-(r0.xyxy))+(r0.xyxy)).xyzw;
    // 27: mul r0.xyzw, r0.xyzw, cb0[14].wwww
    r0.xyzw = ((r0.xyzw)*(source[14].wwww)).xyzw;
    // 28: max r0.xyzw, |r0.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(abs(r0.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 29: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 30: mul r0.xyzw, r0.xyzw, cb0[15].xxxx
    r0.xyzw = ((r0.xyzw)*(source[15].xxxx)).xyzw;
    // 31: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 32: mad r1.zw, v2.wwwz, cb0[8].xxxy, cb0[9].xxxy
    r1.zw = ((v2.wwwz)*(source[8].xxxy)+(source[9].xxxy)).zw;
    // 33: add r2.xyzw, r1.zwzw, l(0.200000, 0.000000, 0.000000, 0.200000)
    r2.xyzw = ((r1.zwzw)+(float4(0.200000,0.000000,0.000000,0.200000))).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t2.yzxw, s4, l(0.000000)
    r1.z = (ArtistNativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t2.yzwx, s4, l(0.000000)
    r1.w = (ArtistNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.zwzz, t2.xyzw, s4, l(0.000000)
    r2.x = (ArtistNativeSample4((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: add r2.y, -r1.z, r2.x
    r2.y = ((-(r1.zzzz))+(r2.xxxx)).y;
    // 38: add r2.x, -r1.z, r1.w
    r2.x = ((-(r1.zzzz))+(r1.wwww)).x;
    // 39: mul r2.xy, r2.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
    // 40: mov r2.z, l(0)
    r2.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: add r2.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 43: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 44: div r1.zw, r2.xxxy, r1.zzzz
    r1.zw = ((r2.xxxy)/(r1.zzzz)).zw;
    // 45: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 46: add r2.x, cb0[1].y, cb0[17].z
    r2.x = ((source[1].yyyy)+(source[17].zzzz)).x;
    // 47: mad r2.yz, cb0[16].wwww, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((source[16].wwww)*(r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 48: mad r2.x, r2.y, cb0[17].x, r2.x
    r2.x = ((r2.yyyy)*(source[17].xxxx)+(r2.xxxx)).x;
    // 49: mad r2.y, r2.z, cb0[17].y, cb0[17].w
    r2.y = ((r2.zzzz)*(source[17].yyyy)+(source[17].wwww)).y;
    // 50: add r2.xy, r2.xyxx, l(-1.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(-1.500000,-0.500000,0.000000,0.000000))).xy;
    // 51: dp2 r3.x, cb0[6].xyxx, r2.xyxx
    r3.x = (dot((source[6].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 52: dp2 r3.y, cb0[7].xyxx, r2.xyxx
    r3.y = (dot((source[7].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 53: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mul r2.z, cb0[1].w, cb0[18].z
    r2.z = ((source[1].wwww)*(source[18].zzzz)).z;
    // 55: mad r1.zw, r2.zzzz, r1.zzzw, r2.xxxy
    r1.zw = ((r2.zzzz)*(r1.zzzw)+(r2.xxxy)).zw;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s3, l(-1.000000)
    r1.zw = (ArtistNativeSample3((r1.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zwxy).zw;
    // 57: mad r2.xyzw, cb0[19].xxxx, -r1.zwzw, r1.zwzw
    r2.xyzw = ((source[19].xxxx)*(-(r1.zwzw))+(r1.zwzw)).xyzw;
    // 58: mul r2.xyzw, r2.xyzw, cb0[19].yyyy
    r2.xyzw = ((r2.xyzw)*(source[19].yyyy)).xyzw;
    // 59: max r2.xyzw, |r2.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r2.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 60: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 61: mul r2.xyzw, r2.xyzw, cb0[19].zzzz
    r2.xyzw = ((r2.xyzw)*(source[19].zzzz)).xyzw;
    // 62: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 63: mul r0.xyzw, r0.xyzw, r2.xyzw
    r0.xyzw = ((r0.xyzw)*(r2.xyzw)).xyzw;
    // 64: max r0.xyzw, r0.xyzw, l(0.000001, 0.000001, 0.000001, 0.000001)
    r0.xyzw = (max(r0.xyzw,float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 65: log r0.xyzw, r0.xyzw
    r0.xyzw = (log2(r0.xyzw)).xyzw;
    // 66: mul r0.xyzw, r0.xyzw, cb0[20].zzzz
    r0.xyzw = ((r0.xyzw)*(source[20].zzzz)).xyzw;
    // 67: exp r0.xyzw, r0.xyzw
    r0.xyzw = (exp2(r0.xyzw)).xyzw;
    // 68: mul r0.xyzw, r0.xyzw, cb0[20].wwww
    r0.xyzw = ((r0.xyzw)*(source[20].wwww)).xyzw;
    // 69: mad r1.z, cb0[21].x, l(10.000000), l(10.000000)
    r1.z = ((source[21].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 70: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 71: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 73: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: mul r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)*(r2.xxxx)).z;
    // 75: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 76: mul r1.z, r1.z, cb0[21].y
    r1.z = ((r1.zzzz)*(source[21].yyyy)).z;
    // 77: movc r1.z, r1.w, l(0), r1.z
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 78: mul r0.xyzw, r0.xyzw, r1.zzzz
    r0.xyzw = ((r0.xyzw)*(r1.zzzz)).xyzw;
    // 79: dp2 r2.x, cb0[10].xyxx, r1.xyxx
    r2.x = (dot((source[10].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 80: dp2 r2.y, cb0[11].xyxx, r1.xyxx
    r2.y = (dot((source[11].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 81: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 82: mul r1.x, r1.x, cb0[21].w
    r1.x = ((r1.xxxx)*(source[21].wwww)).x;
    // 83: mul r1.y, r1.y, cb0[22].x
    r1.y = ((r1.yyyy)*(source[22].xxxx)).y;
    // 84: mad r2.y, cb0[13].z, cb0[23].z, r1.y
    r2.y = ((source[13].zzzz)*(source[23].zzzz)+(r1.yyyy)).y;
    // 85: mad r2.x, cb0[13].z, cb0[21].z, r1.x
    r2.x = ((source[13].zzzz)*(source[21].zzzz)+(r1.xxxx)).x;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.xyxx, t4.xyzw, s5, l(0.000000)
    r1.x = (ArtistNativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 87: add r1.x, r1.x, l(0.200000)
    r1.x = ((r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 88: add r1.y, -cb0[0].w, l(1.000000)
    r1.y = ((-(source[0].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 90: mul_sat r1.x, r1.x, cb0[23].w
    r1.x = (saturate((r1.xxxx)*(source[23].wwww))).x;
    // 91: mul r1.x, r1.x, cb0[0].w
    r1.x = ((r1.xxxx)*(source[0].wwww)).x;
    // 92: mul r0.xyzw, r0.xyzw, r1.xxxx
    r0.xyzw = ((r0.xyzw)*(r1.xxxx)).xyzw;
    // 93: mul r0.xyzw, r0.xyzw, cb0[24].yyyy
    r0.xyzw = ((r0.xyzw)*(source[24].yyyy)).xyzw;
    // 94: mad r0.xyzw, r0.xyzw, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 95: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 96: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 97: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 98: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 99: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 100: div r1.xy, v3.xyxx, v3.wwww
    r1.xy = ((v3.xyxx)/(v3.wwww)).xy;
    // 101: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 102: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 103: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 104: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 105: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 106: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 107: source device depth mapped to centimetre view depth; reconstruction at 109.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 109-112: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 113: ge r0.x, v3.w, r0.x
    r0.x = (asfloat((uint4)((v3.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 114: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 115: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 116: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 117: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pa_simplesphere_01_04_tr: 709735f2ae768144ba2c7eb941573bf9; selected map f0c383ecd47ac677d4069c03913dd465cbacf93ea5bba8591898e311ec3b60bc.
float4 ArtistNative4350(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(0.300000012, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(0.75, 0.0, 0.0, 0.0))),1u);
    source[3] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(-0.150000006, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(-0.400000006, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[5].y = ((g_ArtistSourceMaterialParameters[2u].wwww*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[5].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialParameters[2u].wwww*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[5].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),(g_ArtistSourceMaterialParameters[2u].wwww*float4(0.5, 0.0, 0.0, 0.0))))).x;
    source[6].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[6].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[7].z = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)).x;
    source[7].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(0.300000012, 0.0, 0.0, 0.0))).x;
    source[8].x = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(0.75, 0.0, 0.0, 0.0))).x;
    source[8].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(0.75, 0.0, 0.0, 0.0)))).x;
    source[8].z = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(0.300000012, 0.0, 0.0, 0.0)))).x;
    source[8].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[9].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].z = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(-0.150000006, 0.0, 0.0, 0.0))).x;
    source[9].w = (((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(-0.400000006, 0.0, 0.0, 0.0))).x;
    source[10].x = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(-0.400000006, 0.0, 0.0, 0.0)))).x;
    source[10].y = (ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[2u].zzzz)*float4(-0.150000006, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
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
    // 4: mad r0.y, -r0.x, l(2.000000), l(1.000000)
    r0.y = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 5: mad r0.x, -r0.x, cb0[5].w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[5].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: mul_sat r0.xy, r0.xyxx, l(3.571429, 19.999996, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xyxx)*(float4(3.571429,19.999996,0.000000,0.000000)))).xy;
    // 7: add r0.y, -r0.x, r0.y
    r0.y = ((-(r0.xxxx))+(r0.yyyy)).y;
    // 8: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 9: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 10: mul r0.y, |r0.y|, |r0.y|
    r0.y = ((abs(r0.yyyy))*(abs(r0.yyyy))).y;
    // 11: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 12: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 13: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 14: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 15: mul r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)*(source[9].yyyy)).z;
    // 16: lt r0.w, r0.y, l(0.000001)
    r0.w = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 18: mul r1.xy, v2.xyxx, cb0[6].xxxx
    r1.xy = ((v2.xyxx)*(source[6].xxxx)).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 20: mul r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)*(source[6].yyyy)).w;
    // 21: mad r0.y, r0.y, l(0.500000), r0.w
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).y;
    // 22: mul r1.xy, v2.xyxx, cb0[4].wwww
    r1.xy = ((v2.xyxx)*(source[4].wwww)).xy;
    // 23: mad r1.zw, r1.xxxy, l(0.000000, 0.000000, 0.550000, 0.550000), r0.yyyy
    r1.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.550000,0.550000))+(r0.yyyy)).zw;
    // 24: mad r0.yw, r1.xxxy, l(0.000000, 0.550000, 0.000000, 0.550000), -r0.yyyy
    r0.yw = ((r1.xxxy)*(float4(0.000000,0.550000,0.000000,0.550000))+(-(r0.yyyy))).yw;
    // 25: add r0.yw, r0.yyyw, cb0[3].xxxy
    r0.yw = ((r0.yyyw)+(source[3].xxxy)).yw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample2((r0.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 27: add r1.xy, r1.zwzz, cb0[2].xyxx
    r1.xy = ((r1.zwzz)+(source[2].xyxx)).xy;
    // 28: mul r1.zw, r1.zzzw, cb0[4].zzzz
    r1.zw = ((r1.zzzw)*(source[4].zzzz)).zw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 30: mad r0.z, cb0[8].w, r0.w, r0.z
    r0.z = ((source[8].wwww)*(r0.wwww)+(r0.zzzz)).z;
    // 31: mad r0.y, r0.y, cb0[10].z, r0.z
    r0.y = ((r0.yyyy)*(source[10].zzzz)+(r0.zzzz)).y;
    // 32: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 33: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 34: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 35: mad r0.x, cb0[4].y, cb0[4].x, r1.z
    r0.x = ((source[4].yyyy)*(source[4].xxxx)+(r1.zzzz)).x;
    // 36: mad r0.y, cb0[4].y, cb0[6].z, r1.w
    r0.y = ((source[4].yyyy)*(source[6].zzzz)+(r1.wwww)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t2.xywz, s1, l(0.000000)
    r0.xyw = (ArtistNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 38: dp3 r1.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 39: add r1.xyz, -r0.xywx, r1.xxxx
    r1.xyz = ((-(r0.xywx))+(r1.xxxx)).xyz;
    // 40: mad r0.xyw, cb0[6].wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((source[6].wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 41: mul r0.xyw, r0.xyxw, cb0[7].xxxx
    r0.xyw = ((r0.xyxw)*(source[7].xxxx)).xyw;
    // 42: mul r0.xyz, r0.xywx, r0.zzzz
    r0.xyz = ((r0.xywx)*(r0.zzzz)).xyz;
    // 43: mad r0.xyz, v3.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 44: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_me_cam_01_03_tr: bce71bbf651cf94a972788154634e148; selected map 1d85152928454fdfdf75989c7cf04dfdd84adb7d5ee1bfcdce506c839dfe2980.
float4 ArtistNative4351(ARTIST_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[9u];
    source[3] = g_ArtistSourceMaterialParameters[7u];
    source[4] = g_ArtistSourceMaterialParameters[6u];
    source[5] = input.dynamicParameter;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[6].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[7].y = ((g_ArtistSourceMaterialParameters[4u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[7].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[7].w = ((g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[9].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].wwww)).x;
    source[9].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[12].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[12].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[12].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = input.color; // native texcoord1
    float4 v6 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v7 = float4(input.tangentView,1.f); // native texcoord6
    float4 v8 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v9 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.x, v5.x, cb0[10].w
    r0.x = ((v5.xxxx)*(source[10].wwww)).x;
    // 2: mul r0.y, v5.y, cb0[11].x
    r0.y = ((v5.yyyy)*(source[11].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[11].yzyy
    r0.xy = ((r0.xyxx)+(source[11].yzyy)).xy;
    // 4: add r0.z, -cb0[5].x, l(0.650000)
    r0.z = ((-(source[5].xxxx))+(float4(0.650000,0.650000,0.650000,0.650000))).z;
    // 5: add r0.xy, r0.zzzz, r0.xyxx
    r0.xy = ((r0.zzzz)+(r0.xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.x, r0.x, cb0[11].w
    r0.x = ((r0.xxxx)*(source[11].wwww)).x;
    // 8: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 9: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: mul r0.y, r0.y, cb0[12].x
    r0.y = ((r0.yyyy)*(source[12].xxxx)).y;
    // 11: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 12: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 13: mad r0.yz, v4.xxyx, cb0[6].yyzy, cb0[7].yywy
    r0.yz = ((v4.xxyx)*(source[6].yyzy)+(source[7].yywy)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mad r0.yz, r0.yyyy, cb0[8].xxxx, v4.xxyx
    r0.yz = ((r0.yyyy)*(source[8].xxxx)+(v4.xxyx)).yz;
    // 16: mul r1.x, r0.y, cb0[9].w
    r1.x = ((r0.yyyy)*(source[9].wwww)).x;
    // 17: mul r1.y, r0.z, cb0[10].x
    r1.y = ((r0.zzzz)*(source[10].xxxx)).y;
    // 18: add r0.w, cb0[5].x, cb0[9].x
    r0.w = ((source[5].xxxx)+(source[9].xxxx)).w;
    // 19: add r0.w, r0.w, cb0[5].z
    r0.w = ((r0.wwww)+(source[5].zzzz)).w;
    // 20: mad r1.xy, cb0[10].yzyy, r0.wwww, r1.xyxx
    r1.xy = ((source[10].yzyy)*(r0.wwww)+(r1.xyxx)).xy;
    // 21: mul r1.zw, r0.wwww, cb0[9].yyyz
    r1.zw = ((r0.wwww)*(source[9].yyyz)).zw;
    // 22: mad r0.yz, r0.yyzy, cb0[8].yyzy, r1.zzwz
    r0.yz = ((r0.yyzy)*(source[8].yyzy)+(r1.zzwz)).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 25: add r0.w, r0.z, r0.y
    r0.w = ((r0.zzzz)+(r0.yyyy)).w;
    // 26: mul r1.x, r0.x, r0.w
    r1.x = ((r0.xxxx)*(r0.wwww)).x;
    // 27: add r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)+(r0.wwww)).x;
    // 28: mad r0.y, r0.y, r0.z, r1.x
    r0.y = ((r0.yyyy)*(r0.zzzz)+(r1.xxxx)).y;
    // 29: mad r0.y, r0.y, l(0.500000), l(0.700000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.700000,0.700000,0.700000,0.700000))).y;
    // 30: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 31: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 32: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 33: mad r0.w, cb0[5].y, l(1.400000), l(0.200000)
    r0.w = ((source[5].yyyy)*(float4(1.400000,1.400000,1.400000,1.400000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 34: max r0.w, r0.w, l(0.000010)
    r0.w = (max(r0.wwww,float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 35: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 36: mad r0.z, -r0.z, r0.w, l(1.000000)
    r0.z = ((-(r0.zzzz))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: mad r0.w, -cb0[5].y, l(0.300000), l(1.000000)
    r0.w = ((-(source[5].yyyy))*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 38: max r0.w, r0.w, l(0.000010)
    r0.w = (max(r0.wwww,float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 39: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 40: mul_sat r0.z, r0.w, r0.z
    r0.z = (saturate((r0.wwww)*(r0.zzzz))).z;
    // 41: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 43: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 44: mul r0.w, r0.w, l(1.800000)
    r0.w = ((r0.wwww)*(float4(1.800000,1.800000,1.800000,1.800000))).w;
    // 45: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 46: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 47: mul r0.w, r0.z, cb0[1].w
    r0.w = ((r0.zzzz)*(source[1].wwww)).w;
    // 48: mul r0.xy, r0.xyxx, r0.zwzz
    r0.xy = ((r0.xyxx)*(r0.zwzz)).xy;
    // 49: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 50: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 51: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 52: mul r0.y, r0.y, cb0[12].y
    r0.y = ((r0.yyyy)*(source[12].yyyy)).y;
    // 53: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 54: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 56: dp3 r0.y, cb0[1].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((source[1].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 57: add r0.yzw, r0.yyyy, -cb0[1].xxyz
    r0.yzw = ((r0.yyyy)+(-(source[1].xxyz))).yzw;
    // 58: mad r0.yzw, cb0[6].xxxx, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((source[6].xxxx)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 59: mul r1.xyz, r0.yzwy, cb0[3].xyzx
    r1.xyz = ((r0.yzwy)*(source[3].xyzx)).xyz;
    // 60: mad r0.yzw, cb0[4].xxyz, r0.yyzw, -r1.xxyz
    r0.yzw = ((source[4].xxyz)*(r0.yyzw)+(-(r1.xxyz))).yzw;
    // 61: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 62: mad r0.xyz, cb0[12].zzzz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 63: mad o0.xyz, r0.xyzx, v6.wwww, v6.xyzx
    output.xyz = ((r0.xyzx)*(v6.wwww)+(v6.xyzx)).xyz;
    return output;
}
#endif
#endif

#if !defined(ARTIST_NATIVE_MODEL_ONLY) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
float4 ArtistNative4351Distortion(ARTIST_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = input.dynamicParameter;
    source[2].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[2].y = (g_ArtistSourceMaterialParameters[5u].yyyy).x;
    source[2].z = (g_ArtistSourceMaterialParameters[5u].zzzz).x;
    source[2].w = (g_ArtistSourceMaterialTime.xxxx).x;
    source[3].x = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[3].y = ((g_ArtistSourceMaterialParameters[4u].zzzz*g_ArtistSourceMaterialTime.xxxx)).x;
    source[3].z = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[3].w = ((g_ArtistSourceMaterialParameters[4u].wwww*g_ArtistSourceMaterialTime.xxxx)).x;
    source[4].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ArtistSourceMaterialParameters[5u].wwww).x;
    source[5].x = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[5u].wwww)).x;
    source[5].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[7].w = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = input.vertexColor; // native color0
    float4 v1 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v2 = float4(input.uv,input.uv1); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.x, v3.x, cb0[6].w
    r0.x = ((v3.xxxx)*(source[6].wwww)).x;
    // 2: mul r0.y, v3.y, cb0[7].x
    r0.y = ((v3.yyyy)*(source[7].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[7].yzyy
    r0.xy = ((r0.xyxx)+(source[7].yzyy)).xy;
    // 4: add r0.z, -cb0[1].x, l(0.650000)
    r0.z = ((-(source[1].xxxx))+(float4(0.650000,0.650000,0.650000,0.650000))).z;
    // 5: add r0.xy, r0.zzzz, r0.xyxx
    r0.xy = ((r0.zzzz)+(r0.xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ArtistNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 8: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 9: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 10: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 11: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 12: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 13: mad r0.yz, v2.xxyx, cb0[2].yyzy, cb0[3].yywy
    r0.yz = ((v2.xxyx)*(source[2].yyzy)+(source[3].yywy)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mad r0.yz, r0.yyyy, cb0[4].xxxx, v2.xxyx
    r0.yz = ((r0.yyyy)*(source[4].xxxx)+(v2.xxyx)).yz;
    // 16: mul r1.x, r0.y, cb0[5].w
    r1.x = ((r0.yyyy)*(source[5].wwww)).x;
    // 17: mul r1.y, r0.z, cb0[6].x
    r1.y = ((r0.zzzz)*(source[6].xxxx)).y;
    // 18: add r0.w, cb0[1].x, cb0[5].x
    r0.w = ((source[1].xxxx)+(source[5].xxxx)).w;
    // 19: add r0.w, r0.w, cb0[1].z
    r0.w = ((r0.wwww)+(source[1].zzzz)).w;
    // 20: mad r1.xy, cb0[6].yzyy, r0.wwww, r1.xyxx
    r1.xy = ((source[6].yzyy)*(r0.wwww)+(r1.xyxx)).xy;
    // 21: mul r1.zw, r0.wwww, cb0[5].yyyz
    r1.zw = ((r0.wwww)*(source[5].yyyz)).zw;
    // 22: mad r0.yz, r0.yyzy, cb0[4].yyzy, r1.zzwz
    r0.yz = ((r0.yyzy)*(source[4].yyzy)+(r1.zzwz)).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s2, l(0.000000)
    r0.y = (ArtistNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 25: add r0.w, r0.z, r0.y
    r0.w = ((r0.zzzz)+(r0.yyyy)).w;
    // 26: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 27: mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // 28: mad r0.x, r0.x, l(0.500000), l(0.700000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 29: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 30: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 31: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 32: mad r0.z, cb0[1].y, l(1.400000), l(0.200000)
    r0.z = ((source[1].yyyy)*(float4(1.400000,1.400000,1.400000,1.400000))+(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 33: max r0.z, r0.z, l(0.000010)
    r0.z = (max(r0.zzzz,float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 34: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 35: mad r0.y, -r0.y, r0.z, l(1.000000)
    r0.y = ((-(r0.yyyy))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mad r0.z, -cb0[1].y, l(0.300000), l(1.000000)
    r0.z = ((-(source[1].yyyy))*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: max r0.z, r0.z, l(0.000010)
    r0.z = (max(r0.zzzz,float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 39: mul_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)*(r0.yyyy))).y;
    // 40: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 42: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 43: mul r0.z, r0.z, l(1.800000)
    r0.z = ((r0.zzzz)*(float4(1.800000,1.800000,1.800000,1.800000))).z;
    // 44: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 45: mul r0.z, r0.z, cb0[0].w
    r0.z = ((r0.zzzz)*(source[0].wwww)).z;
    // 46: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // 49: mad r0.xyzw, r0.xxxx, l(2.000000, -2.000000, 2.000000, -2.000000), l(-1.000000, 1.000000, -1.000000, 1.000000)
    r0.xyzw = ((r0.xxxx)*(float4(2.000000,-2.000000,2.000000,-2.000000))+(float4(-1.000000,1.000000,-1.000000,1.000000))).xyzw;
    // 50: mad r0.xyzw, r0.xyzw, cb2[0].xyxy, cb2[0].wzwz
    r0.xyzw = ((r0.xyzw)*(passValues[0].xyxy)+(passValues[0].wzwz)).xyzw;
    // 51: dp2 r1.x, r0.zwzz, r0.zwzz
    r1.x = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).x;
    // 52: add r1.x, r1.x, l(-0.100000)
    r1.x = ((r1.xxxx)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 53: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 54: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) return 0.f;
    // 55: div r1.xy, v4.xyxx, v4.wwww
    r1.xy = ((v4.xyxx)/(v4.wwww)).xy;
    // 56: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 57: mad r0.xy, r0.xyxx, l(0.003922, -0.003922, 0.000000, 0.000000), r1.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.003922,-0.003922,0.000000,0.000000))+(r1.xyxx)).xy;
    // 58: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 4.000000, 4.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,4.000000,4.000000))).zw;
    // 59: max r0.zw, r0.zzzw, l(0.000000, 0.000000, -255.000000, -255.000000)
    r0.zw = (max(r0.zzzw,float4(0.000000,0.000000,-255.000000,-255.000000))).zw;
    // 60: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 255.000000, 255.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,255.000000,255.000000))).zw;
    // 61: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.003922, 0.003922)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.003922,0.003922))).zw;
    // Native 62: source device depth mapped to centimetre view depth; reconstruction at 64.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 64-67: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 68: ge r0.x, v4.w, r0.x
    r0.x = (asfloat((uint4)((v4.wwww)>=(r0.xxxx)) * 0xffffffffu)).x;
    // 69: movc r0.xy, r0.xxxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 70: max o0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    output.xy = (max(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 71: min r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 72: mov o0.zw, -r0.xxxy
    output.zw = (-(r0.xxxy)).zw;
    return output;
}
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_exposteffect_01: 599a6ad60ea6494a83850077e6cd3356; selected map 0adcb9ac7bfeb672a867821880774b930d2ef98a38f75c94db127e342a29f4f8.
float4 ArtistNative4304(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=g_ArtistSourceMaterialParameters[1].y; // Source action SkillValue opacity.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[4u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)*float4(-0.300000012, 0.0, 0.0, 0.0))),1u);
    source[3] = g_ArtistSourceMaterialParameters[3u];
    source[4].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[4].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ArtistSourceMaterialTime.xxxx).x;
    source[5].w = ((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[1u].zzzz)).x;
    source[6].x = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz)).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[0u].zzzz))).x;
    source[6].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[7].x = ((g_ArtistSourceMaterialParameters[0u].xxxx*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[7].y = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
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
    // 10: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 11: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 12: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 13: min r0.w, |r2.y|, |r2.x|
    r0.w = (min(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 14: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 15: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 16: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 17: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 18: mad r2.z, r1.w, l(0.020835), l(-0.085133)
    r2.z = ((r1.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 19: mad r2.z, r1.w, r2.z, l(0.180141)
    r2.z = ((r1.wwww)*(r2.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 20: mad r2.z, r1.w, r2.z, l(-0.330299)
    r2.z = ((r1.wwww)*(r2.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 21: mad r1.w, r1.w, r2.z, l(0.999866)
    r1.w = ((r1.wwww)*(r2.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 22: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 23: lt r2.w, |r2.y|, |r2.x|
    r2.w = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).w;
    // 24: mad r2.z, r2.z, l(-2.000000), l(1.570796)
    r2.z = ((r2.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 25: and r2.z, r2.w, r2.z
    r2.z = (asfloat(asuint(r2.wwww) & asuint(r2.zzzz))).z;
    // 26: mad r0.w, r0.w, r1.w, r2.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r2.zzzz)).w;
    // 27: lt r1.w, r2.y, -r2.y
    r1.w = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).w;
    // 28: and r1.w, r1.w, l(0xc0490fdb)
    r1.w = (asfloat(asuint(r1.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 29: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 30: min r1.w, r2.y, r2.x
    r1.w = (min(r2.yyyy,r2.xxxx)).w;
    // 31: max r2.z, r2.y, r2.x
    r2.z = (max(r2.yyyy,r2.xxxx)).z;
    // 32: lt r1.w, r1.w, -r1.w
    r1.w = (asfloat((uint4)((r1.wwww)<(-(r1.wwww))) * 0xffffffffu)).w;
    // 33: ge r2.z, r2.z, -r2.z
    r2.z = (asfloat((uint4)((r2.zzzz)>=(-(r2.zzzz))) * 0xffffffffu)).z;
    // 34: and r1.w, r1.w, r2.z
    r1.w = (asfloat(asuint(r1.wwww) & asuint(r2.zzzz))).w;
    // 35: movc r0.w, r1.w, -r0.w, r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 36: mad r0.w, r0.w, l(0.318310), l(1.000000)
    r0.w = ((r0.wwww)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r0.w, r0.w, cb0[4].x
    r0.w = ((r0.wwww)*(source[4].xxxx)).w;
    // 38: mul r3.x, r0.w, l(0.500000)
    r3.x = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 39: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 40: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 41: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 42: lt r2.z, r1.w, l(0.000001)
    r2.z = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 43: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 44: mul r1.w, r1.w, cb0[4].y
    r1.w = ((r1.wwww)*(source[4].yyyy)).w;
    // 45: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 46: mul r1.w, r1.w, cb0[4].z
    r1.w = ((r1.wwww)*(source[4].zzzz)).w;
    // 47: movc r3.w, r2.z, l(0), r1.w
    r3.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 48: add r2.zw, v4.xxxy, v4.xxxy
    r2.zw = ((v4.xxxy)+(v4.xxxy)).zw;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.zwzz, t0.yzwx, s2, l(0.000000)
    r1.w = (ArtistNativeSample1((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 50: mul r3.z, r1.w, cb0[4].w
    r3.z = ((r1.wwww)*(source[4].wwww)).z;
    // 51: lt r2.z, |cb0[5].x|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(source[5].xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 52: log r2.w, |cb0[5].x|
    r2.w = (log2(abs(source[5].xxxx))).w;
    // 53: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 54: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 55: movc r2.z, r2.z, l(0), r2.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 56: mad r4.y, r1.w, cb0[4].w, r2.z
    r4.y = ((r1.wwww)*(source[4].wwww)+(r2.zzzz)).y;
    // 57: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 58: add r2.zw, r3.zzzw, r4.xxxy
    r2.zw = ((r3.zzzw)+(r4.xxxy)).zw;
    // 59: mov r3.y, l(-0.350000)
    r3.y = (float4(-0.350000,-0.350000,-0.350000,-0.350000)).y;
    // 60: add r2.zw, r2.zzzw, r3.xxxy
    r2.zw = ((r2.zzzw)+(r3.xxxy)).zw;
    // 61: sample_l_indexable(texture2d)(float,float,float,float) r1.w, r2.zwzz, t1.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzwx).w;
    // 62: mul r1.w, r1.w, cb0[5].x
    r1.w = ((r1.wwww)*(source[5].xxxx)).w;
    // 63: mul r2.z, r1.w, l(0.250000)
    r2.z = ((r1.wwww)*(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 64: mad r3.yz, v4.xxyx, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((v4.xxyx)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 65: mad r1.xy, r2.zzzz, r3.yzyy, r1.xyxx
    r1.xy = ((r2.zzzz)*(r3.yzyy)+(r1.xyxx)).xy;
    // 66: add r2.zw, r3.xxxw, cb0[2].xxxy
    r2.zw = ((r3.xxxw)+(source[2].xxxy)).zw;
    // 67: sample_l_indexable(texture2d)(float,float,float,float) r2.z, r2.zwzz, t2.yzxw, s3, l(0.000000)
    r2.z = (ArtistNativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
    // 68: mad r0.w, -r0.w, cb0[6].z, l(1.000000)
    r0.w = ((-(r0.wwww))*(source[6].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: mul_sat r0.w, r0.w, l(1.333333)
    r0.w = (saturate((r0.wwww)*(float4(1.333333,1.333333,1.333333,1.333333)))).w;
    // 70: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r2.z, r0.w, r2.z
    r2.z = ((r0.wwww)*(r2.zzzz)).z;
    // 72: mul r2.z, r2.z, cb0[7].x
    r2.z = ((r2.zzzz)*(source[7].xxxx)).z;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r1.xyxx, t3.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 74: mul r2.z, r2.z, l(0.100000)
    r2.z = ((r2.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 75: mov r4.xy, r1.xyxx
    r4.xy = (r1.xyxx).xy;
    // 76: mov r5.x, r3.x
    r5.x = (r3.xxxx).x;
    // 77: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 78: loop
    [loop] while (true) {
    // 79: ge r2.w, r5.y, l(8.000000)
    r2.w = (asfloat((uint4)((r5.yyyy)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).w;
    // 80: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 81: mad r4.xy, -r2.xyxx, r2.zzzz, r4.xyxx
    r4.xy = ((-(r2.xyxx))*(r2.zzzz)+(r4.xyxx)).xy;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r4.xyxx, t3.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 83: add r5.x, r2.w, r5.x
    r5.x = ((r2.wwww)+(r5.xxxx)).x;
    // 84: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 85: endloop
    }
    // 86: mov r1.xy, r4.xyxx
    r1.xy = (r4.xyxx).xy;
    // 87: mov r6.x, r3.y
    r6.x = (r3.yyyy).x;
    // 88: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 89: loop
    [loop] while (true) {
    // 90: ge r2.w, r6.y, l(8.000000)
    r2.w = (asfloat((uint4)((r6.yyyy)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).w;
    // 91: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 92: mad r1.xy, -r2.xyxx, r2.zzzz, r1.xyxx
    r1.xy = ((-(r2.xyxx))*(r2.zzzz)+(r1.xyxx)).xy;
    // 93: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r1.xyxx, t3.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 94: add r6.x, r2.w, r6.x
    r6.x = ((r2.wwww)+(r6.xxxx)).x;
    // 95: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 96: endloop
    }
    // 97: mov r5.y, r6.x
    r5.y = (r6.xxxx).y;
    // 98: mov r3.xy, r1.xyxx
    r3.xy = (r1.xyxx).xy;
    // 99: mov r4.x, r3.z
    r4.x = (r3.zzzz).x;
    // 100: mov r4.y, l(0)
    r4.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 101: loop
    [loop] while (true) {
    // 102: ge r2.w, r4.y, l(8.000000)
    r2.w = (asfloat((uint4)((r4.yyyy)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).w;
    // 103: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 104: mad r3.xy, -r2.xyxx, r2.zzzz, r3.xyxx
    r3.xy = ((-(r2.xyxx))*(r2.zzzz)+(r3.xyxx)).xy;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t3.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 106: add r4.x, r2.w, r4.x
    r4.x = ((r2.wwww)+(r4.xxxx)).x;
    // 107: add r4.y, r4.y, l(1.000000)
    r4.y = ((r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 108: endloop
    }
    // 109: mov r5.z, r4.x
    r5.z = (r4.xxxx).z;
    // 110: mul r2.xyz, r5.xyzx, l(0.111111, 0.111111, 0.111111, 0.000000)
    r2.xyz = ((r5.xyzx)*(float4(0.111111,0.111111,0.111111,0.000000))).xyz;
    // 111: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: mad r3.xyz, -r5.xyzx, l(0.111111, 0.111111, 0.111111, 0.000000), r1.xxxx
    r3.xyz = ((-(r5.xyzx))*(float4(0.111111,0.111111,0.111111,0.000000))+(r1.xxxx)).xyz;
    // 113: mad r2.xyz, cb0[7].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[7].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 114: mul r1.xyw, r1.wwww, cb0[3].xyxz
    r1.xyw = ((r1.wwww)*(source[3].xyxz)).xyw;
    // 115: mad r1.xyw, r0.wwww, r1.xyxw, r2.xyxz
    r1.xyw = ((r0.wwww)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 116: add r1.xyw, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((r1.xyxw)+(source[1].xyxz)).xyw;
    // 117: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 118: mov r0.x, r1.z
    r0.x = (r1.zzzz).x;
    // 119: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 120: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 121: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 122: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 123: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 124: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 125: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 126: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 127: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 128: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 130: mov o0.w, cb0[0].x
    output.w = (source[0].xxxx).w;
    return output;
}
#endif
#endif

#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER)
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_j_pp_expost_04: 5a405d04c4e0fa42a1342f8da1615ca0; selected map c4a8a19abd4d206560f02a88612343962aa665362e57f28551e49b550042c557.
float4 ArtistNative4305(ARTIST_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=g_ArtistSourceMaterialParameters[3].w; // Source action SkillValue opacity.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[6u];
    source[2] = ArtistNativeAppend(ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ArtistNativePeriodic(((g_ArtistSourceMaterialTime.xxxx*g_ArtistSourceMaterialParameters[4u].xxxx)*float4(-0.300000012, 0.0, 0.0, 0.0))),1u);
    source[3] = g_ArtistSourceMaterialParameters[5u];
    source[4] = ArtistNativeAppend(cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ArtistNativeAppend(sin((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[6].z = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[6].w = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[7].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[8].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ArtistSourceMaterialParameters[2u].zzzz))).x;
    source[8].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[8].z = ((g_ArtistSourceMaterialParameters[0u].xxxx*g_ArtistSourceMaterialParameters[2u].wwww)).x;
    source[8].w = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[9].x = (cos((g_ArtistSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[9].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[9].w = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[10].x = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[10].y = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[10].z = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[10].w = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    float4 passValues[4]; [unroll] for(uint passIndex=0u;passIndex<4u;++passIndex) passValues[passIndex]=0.f;
    passValues[0]=float4(.5f,-.5f,.5f,.5f);
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
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
    // 10: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 11: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 12: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 13: min r0.w, |r2.y|, |r2.x|
    r0.w = (min(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 14: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 15: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 16: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 17: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 18: mad r2.z, r1.w, l(0.020835), l(-0.085133)
    r2.z = ((r1.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 19: mad r2.z, r1.w, r2.z, l(0.180141)
    r2.z = ((r1.wwww)*(r2.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 20: mad r2.z, r1.w, r2.z, l(-0.330299)
    r2.z = ((r1.wwww)*(r2.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 21: mad r1.w, r1.w, r2.z, l(0.999866)
    r1.w = ((r1.wwww)*(r2.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 22: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 23: lt r2.w, |r2.y|, |r2.x|
    r2.w = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).w;
    // 24: mad r2.z, r2.z, l(-2.000000), l(1.570796)
    r2.z = ((r2.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 25: and r2.z, r2.w, r2.z
    r2.z = (asfloat(asuint(r2.wwww) & asuint(r2.zzzz))).z;
    // 26: mad r0.w, r0.w, r1.w, r2.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r2.zzzz)).w;
    // 27: lt r1.w, r2.y, -r2.y
    r1.w = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).w;
    // 28: and r1.w, r1.w, l(0xc0490fdb)
    r1.w = (asfloat(asuint(r1.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 29: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 30: min r1.w, r2.y, r2.x
    r1.w = (min(r2.yyyy,r2.xxxx)).w;
    // 31: max r2.z, r2.y, r2.x
    r2.z = (max(r2.yyyy,r2.xxxx)).z;
    // 32: lt r1.w, r1.w, -r1.w
    r1.w = (asfloat((uint4)((r1.wwww)<(-(r1.wwww))) * 0xffffffffu)).w;
    // 33: ge r2.z, r2.z, -r2.z
    r2.z = (asfloat((uint4)((r2.zzzz)>=(-(r2.zzzz))) * 0xffffffffu)).z;
    // 34: and r1.w, r1.w, r2.z
    r1.w = (asfloat(asuint(r1.wwww) & asuint(r2.zzzz))).w;
    // 35: movc r0.w, r1.w, -r0.w, r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 36: mad r0.w, r0.w, l(0.318310), l(1.000000)
    r0.w = ((r0.wwww)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 38: mul r3.x, r0.w, l(0.500000)
    r3.x = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 39: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 40: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 41: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 42: lt r2.z, r1.w, l(0.000001)
    r2.z = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 43: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 44: mul r1.w, r1.w, cb0[6].y
    r1.w = ((r1.wwww)*(source[6].yyyy)).w;
    // 45: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 46: mul r1.w, r1.w, cb0[6].z
    r1.w = ((r1.wwww)*(source[6].zzzz)).w;
    // 47: movc r3.w, r2.z, l(0), r1.w
    r3.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 48: add r2.zw, v4.xxxy, v4.xxxy
    r2.zw = ((v4.xxxy)+(v4.xxxy)).zw;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.zwzz, t0.yzwx, s2, l(0.000000)
    r1.w = (ArtistNativeSample1((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 50: mul r3.z, r1.w, cb0[6].w
    r3.z = ((r1.wwww)*(source[6].wwww)).z;
    // 51: lt r2.z, |cb0[7].x|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(source[7].xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 52: log r2.w, |cb0[7].x|
    r2.w = (log2(abs(source[7].xxxx))).w;
    // 53: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 54: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 55: movc r2.z, r2.z, l(0), r2.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 56: mad r4.y, r1.w, cb0[6].w, r2.z
    r4.y = ((r1.wwww)*(source[6].wwww)+(r2.zzzz)).y;
    // 57: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 58: add r2.zw, r3.zzzw, r4.xxxy
    r2.zw = ((r3.zzzw)+(r4.xxxy)).zw;
    // 59: mov r3.y, l(-0.350000)
    r3.y = (float4(-0.350000,-0.350000,-0.350000,-0.350000)).y;
    // 60: add r2.zw, r2.zzzw, r3.xxxy
    r2.zw = ((r2.zzzw)+(r3.xxxy)).zw;
    // 61: sample_l_indexable(texture2d)(float,float,float,float) r1.w, r2.zwzz, t1.yzwx, s1, l(0.000000)
    r1.w = (ArtistNativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzwx).w;
    // 62: lt r2.z, |r1.w|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 63: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 64: mul r1.w, r1.w, cb0[7].y
    r1.w = ((r1.wwww)*(source[7].yyyy)).w;
    // 65: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 66: mul r1.w, r1.w, cb0[7].x
    r1.w = ((r1.wwww)*(source[7].xxxx)).w;
    // 67: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 68: mul r2.z, r1.w, cb0[7].z
    r2.z = ((r1.wwww)*(source[7].zzzz)).z;
    // 69: mad r3.yz, v4.xxyx, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((v4.xxyx)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 70: mad r1.xy, r2.zzzz, r3.yzyy, r1.xyxx
    r1.xy = ((r2.zzzz)*(r3.yzyy)+(r1.xyxx)).xy;
    // 71: add r3.xw, r3.xxxw, cb0[2].xxxy
    r3.xw = ((r3.xxxw)+(source[2].xxxy)).xw;
    // 72: sample_l_indexable(texture2d)(float,float,float,float) r2.w, r3.xwxx, t2.yzwx, s3, l(0.000000)
    r2.w = (ArtistNativeSample2((r3.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzwx).w;
    // 73: mad r0.w, -r0.w, cb0[8].x, l(1.000000)
    r0.w = ((-(r0.wwww))*(source[8].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: mul_sat r0.w, r0.w, l(1.333333)
    r0.w = (saturate((r0.wwww)*(float4(1.333333,1.333333,1.333333,1.333333)))).w;
    // 75: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)*(r2.wwww)).w;
    // 77: mul r2.w, r2.w, cb0[8].z
    r2.w = ((r2.wwww)*(source[8].zzzz)).w;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r1.xyxx, t3.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.xyz = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 79: mul r2.w, r2.w, l(0.100000)
    r2.w = ((r2.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 80: mov r3.xw, r1.xxxy
    r3.xw = (r1.xxxy).xw;
    // 81: mov r5.x, r4.x
    r5.x = (r4.xxxx).x;
    // 82: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 83: loop
    [loop] while (true) {
    // 84: ge r4.w, r5.y, l(8.000000)
    r4.w = (asfloat((uint4)((r5.yyyy)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).w;
    // 85: breakc_nz r4.w
    if ((asuint(r4.wwww)).x != 0u) break;
    // 86: mad r3.xw, -r2.xxxy, r2.wwww, r3.xxxw
    r3.xw = ((-(r2.xxxy))*(r2.wwww)+(r3.xxxw)).xw;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r4.w, r3.xwxx, t3.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 88: add r5.x, r4.w, r5.x
    r5.x = ((r4.wwww)+(r5.xxxx)).x;
    // 89: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 90: endloop
    }
    // 91: mov r1.xy, r3.xwxx
    r1.xy = (r3.xwxx).xy;
    // 92: mov r6.x, r4.y
    r6.x = (r4.yyyy).x;
    // 93: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 94: loop
    [loop] while (true) {
    // 95: ge r4.x, r6.y, l(8.000000)
    r4.x = (asfloat((uint4)((r6.yyyy)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).x;
    // 96: breakc_nz r4.x
    if ((asuint(r4.xxxx)).x != 0u) break;
    // 97: mad r1.xy, -r2.xyxx, r2.wwww, r1.xyxx
    r1.xy = ((-(r2.xyxx))*(r2.wwww)+(r1.xyxx)).xy;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r4.x, r1.xyxx, t3.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.x = (Read_EffectSceneColorBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 99: add r6.x, r4.x, r6.x
    r6.x = ((r4.xxxx)+(r6.xxxx)).x;
    // 100: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 101: endloop
    }
    // 102: mov r5.y, r6.x
    r5.y = (r6.xxxx).y;
    // 103: mov r3.xw, r1.xxxy
    r3.xw = (r1.xxxy).xw;
    // 104: mov r4.x, r4.z
    r4.x = (r4.zzzz).x;
    // 105: mov r4.y, l(0)
    r4.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 106: loop
    [loop] while (true) {
    // 107: ge r4.w, r4.y, l(8.000000)
    r4.w = (asfloat((uint4)((r4.yyyy)>=(float4(8.000000,8.000000,8.000000,8.000000))) * 0xffffffffu)).w;
    // 108: breakc_nz r4.w
    if ((asuint(r4.wwww)).x != 0u) break;
    // 109: mad r3.xw, -r2.xxxy, r2.wwww, r3.xxxw
    r3.xw = ((-(r2.xxxy))*(r2.wwww)+(r3.xxxw)).xw;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r4.w, r3.xwxx, t3.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.w = (Read_EffectSceneColorBias(LinearClampUVSampler, (r3.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 111: add r4.x, r4.w, r4.x
    r4.x = ((r4.wwww)+(r4.xxxx)).x;
    // 112: add r4.y, r4.y, l(1.000000)
    r4.y = ((r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 113: endloop
    }
    // 114: mov r5.z, r4.x
    r5.z = (r4.xxxx).z;
    // 115: mul r2.xyw, r5.xyxz, l(0.111111, 0.111111, 0.000000, 0.111111)
    r2.xyw = ((r5.xyxz)*(float4(0.111111,0.111111,0.000000,0.111111))).xyw;
    // 116: dp3 r1.x, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 117: mad r4.xyz, -r5.xyzx, l(0.111111, 0.111111, 0.111111, 0.000000), r1.xxxx
    r4.xyz = ((-(r5.xyzx))*(float4(0.111111,0.111111,0.111111,0.000000))+(r1.xxxx)).xyz;
    // 118: mad r2.xyw, cb0[8].wwww, r4.xyxz, r2.xyxw
    r2.xyw = ((source[8].wwww)*(r4.xyxz)+(r2.xyxw)).xyw;
    // 119: mul r1.xyw, r1.wwww, cb0[3].xyxz
    r1.xyw = ((r1.wwww)*(source[3].xyxz)).xyw;
    // 120: min r3.x, |r3.y|, |r3.z|
    r3.x = (min(abs(r3.yyyy),abs(r3.zzzz))).x;
    // 121: max r3.w, |r3.y|, |r3.z|
    r3.w = (max(abs(r3.yyyy),abs(r3.zzzz))).w;
    // 122: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r3.w
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r3.wwww)).w;
    // 123: mul r3.x, r3.w, r3.x
    r3.x = ((r3.wwww)*(r3.xxxx)).x;
    // 124: mul r3.w, r3.x, r3.x
    r3.w = ((r3.xxxx)*(r3.xxxx)).w;
    // 125: mad r4.x, r3.w, l(0.020835), l(-0.085133)
    r4.x = ((r3.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 126: mad r4.x, r3.w, r4.x, l(0.180141)
    r4.x = ((r3.wwww)*(r4.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 127: mad r4.x, r3.w, r4.x, l(-0.330299)
    r4.x = ((r3.wwww)*(r4.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 128: mad r3.w, r3.w, r4.x, l(0.999866)
    r3.w = ((r3.wwww)*(r4.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 129: mul r4.x, r3.w, r3.x
    r4.x = ((r3.wwww)*(r3.xxxx)).x;
    // 130: lt r4.y, |r3.y|, |r3.z|
    r4.y = (asfloat((uint4)((abs(r3.yyyy))<(abs(r3.zzzz))) * 0xffffffffu)).y;
    // 131: mad r4.x, r4.x, l(-2.000000), l(1.570796)
    r4.x = ((r4.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 132: and r4.x, r4.y, r4.x
    r4.x = (asfloat(asuint(r4.yyyy) & asuint(r4.xxxx))).x;
    // 133: mad r3.x, r3.x, r3.w, r4.x
    r3.x = ((r3.xxxx)*(r3.wwww)+(r4.xxxx)).x;
    // 134: lt r3.w, r3.y, -r3.y
    r3.w = (asfloat((uint4)((r3.yyyy)<(-(r3.yyyy))) * 0xffffffffu)).w;
    // 135: and r3.w, r3.w, l(0xc0490fdb)
    r3.w = (asfloat(asuint(r3.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 136: add r3.x, r3.w, r3.x
    r3.x = ((r3.wwww)+(r3.xxxx)).x;
    // 137: min r3.w, r3.y, r3.z
    r3.w = (min(r3.yyyy,r3.zzzz)).w;
    // 138: max r4.x, r3.y, r3.z
    r4.x = (max(r3.yyyy,r3.zzzz)).x;
    // 139: lt r3.w, r3.w, -r3.w
    r3.w = (asfloat((uint4)((r3.wwww)<(-(r3.wwww))) * 0xffffffffu)).w;
    // 140: ge r4.x, r4.x, -r4.x
    r4.x = (asfloat((uint4)((r4.xxxx)>=(-(r4.xxxx))) * 0xffffffffu)).x;
    // 141: and r3.w, r3.w, r4.x
    r3.w = (asfloat(asuint(r3.wwww) & asuint(r4.xxxx))).w;
    // 142: movc r3.x, r3.w, -r3.x, r3.x
    r3.x = ((asuint(r3.wwww) != 0u) ? (-(r3.xxxx)) : (r3.xxxx)).x;
    // 143: mul r3.x, r3.x, l(0.159155)
    r3.x = ((r3.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))).x;
    // 144: frc r3.x, r3.x
    r3.x = (frac(r3.xxxx)).x;
    // 145: mul r3.yz, r3.yyzy, r3.yyzy
    r3.yz = ((r3.yyzy)*(r3.yyzy)).yz;
    // 146: add r3.y, r3.z, r3.y
    r3.y = ((r3.zzzz)+(r3.yyyy)).y;
    // 147: sqrt r3.y, r3.y
    r3.y = (sqrt(r3.yyyy)).y;
    // 148: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 149: mad r4.x, r3.x, cb0[9].y, cb0[9].w
    r4.x = ((r3.xxxx)*(source[9].yyyy)+(source[9].wwww)).x;
    // 150: mad r4.y, r3.y, cb0[9].z, cb0[10].x
    r4.y = ((r3.yyyy)*(source[9].zzzz)+(source[10].xxxx)).y;
    // 151: add r3.xy, r4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 152: dp2 r4.x, cb0[4].xyxx, r3.xyxx
    r4.x = (dot((source[4].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 153: dp2 r4.y, cb0[5].xyxx, r3.xyxx
    r4.y = (dot((source[5].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 154: add r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 155: sample_l_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s4, l(-1.000000)
    r3.xyz = (ArtistNativeSample3((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 156: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 157: add r4.xyz, -r3.xyzx, r3.wwww
    r4.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 158: mad r3.xyz, cb0[10].yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((source[10].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 159: mul r3.xyz, r3.xyzx, cb0[10].zzzz
    r3.xyz = ((r3.xyzx)*(source[10].zzzz)).xyz;
    // 160: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 161: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 162: mul r3.xyz, r3.xyzx, cb0[10].wwww
    r3.xyz = ((r3.xyzx)*(source[10].wwww)).xyz;
    // 163: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 164: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 165: mad r1.xyw, r0.wwww, r1.xyxw, r3.xyxz
    r1.xyw = ((r0.wwww)*(r1.xyxw)+(r3.xyxz)).xyw;
    // 166: add r1.xyw, r1.xyxw, r2.xyxw
    r1.xyw = ((r1.xyxw)+(r2.xyxw)).xyw;
    // 167: add r1.xyw, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((r1.xyxw)+(source[1].xyxz)).xyw;
    // 168: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 169: mov r0.x, r1.z
    r0.x = (r1.zzzz).x;
    // 170: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 171: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 172: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 173: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 174: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 175: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 176: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 177: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 178: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 179: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 181: mov o0.w, cb0[0].x
    output.w = (source[0].xxxx).w;
    return output;
}
#endif
#endif
