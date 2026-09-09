// Single source owner for ArtistNative profiles 768..831.
#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_shine_01_09_ad: eed5a0bfbb11fb4cbe047cc2cefd45f5; selected map 813c7a9653d5d31479688e52db86c7fc08a10cb679d4957d37544db222fd06a2.
float4 ArtistNative820(ARTIST_NATIVE_INPUT input)
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

#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_d_pa_flare_02_02_dt_ad: 31a56416afc48b48975d5f75c4d52157; selected map 4d77068b2fd78f85fcdac963e7906195871e4e3e5ba7a45f5fe9ea5bb462c047.
float4 ArtistNative821(ARTIST_NATIVE_INPUT input)
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

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_pa_flowergarden_flare_depth_02_02_dt_ad: 31a56416afc48b48975d5f75c4d52157; selected map 4d77068b2fd78f85fcdac963e7906195871e4e3e5ba7a45f5fe9ea5bb462c047.
float4 ArtistNative822(ARTIST_NATIVE_INPUT input)
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

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_118_fs_dt_ad: 305f52979299224e8136f5c440cb89d9; selected map 10f08d6f0e89becd42fc8396fc5a8febea9e88ebb347160baf0a60a97058f2a9.
float4 ArtistNative823(ARTIST_NATIVE_INPUT input)
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

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_pa_sqc_02_01_tr: b1b503130ec03c409bd93d0361be062a; selected map 624707a6baae9b157d1d5f22930ae4bb00d0c60932d770eeb513ad7fd3af8266.
float4 ArtistNative824(ARTIST_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ArtistSourceMaterialParameters[1u];
    source[2].x = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
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
    // 10: add r0.y, -cb0[2].x, l(1.000000)
    r0.y = ((-(source[2].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.zwzz, t0.yxzw, s1, l(0.000000)
    r0.y = (ArtistNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v2.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ArtistNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 16: mad r0.y, v0.w, r0.y, r0.z
    r0.y = ((v0.wwww)*(r0.yyyy)+(r0.zzzz)).y;
    // 17: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 18: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 19: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 20: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_245_ts_tr: fd5e4ab195b9e347955f676c347c71a3; selected map 4e79a51bfa7cfeb96317f910323adeefb66052aaa013b46ee660be37099c8abc.
float4 ArtistNative825(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[5].zwzz
    r0.xy = ((v4.xyxx)*(source[5].zwzz)).xy;
    // 2: mul r0.z, cb0[4].x, cb0[4].y
    r0.z = ((source[4].xxxx)*(source[4].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[5].y, r0.x
    r1.x = ((r0.zzzz)*(source[5].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[6].x, r0.y
    r1.y = ((r0.zzzz)*(source[6].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[6].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[6].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 7: mul r0.x, r0.x, cb0[4].w
    r0.x = ((r0.xxxx)*(source[4].wwww)).x;
    // 8: mad r1.x, r0.z, cb0[4].z, r0.x
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.xxxx)).x;
    // 9: mul r0.x, r0.z, cb0[6].w
    r0.x = ((r0.zzzz)*(source[6].wwww)).x;
    // 10: mad r1.y, cb0[5].x, r0.y, r0.x
    r1.y = ((source[5].xxxx)*(r0.yyyy)+(r0.xxxx)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 13: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 14: add r0.y, -cb0[3].x, l(1.000000)
    r0.y = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mad r0.x, r0.x, l(0.333330), -r0.y
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))+(-(r0.yyyy))).x;
    // 16: mul_sat r0.x, r0.x, cb0[7].x
    r0.x = (saturate((r0.xxxx)*(source[7].xxxx))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 22: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 23: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 24: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 25: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.w, r0.w, cb0[7].z
    r0.w = ((r0.wwww)*(source[7].zzzz)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul_sat r0.w, r0.w, cb0[7].w
    r0.w = (saturate((r0.wwww)*(source[7].wwww))).w;
    // 29: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 30: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 31: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 32: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 33: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 34: add r0.xyz, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)+(source[2].xyzx)).xyz;
    // 35: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_d_me_master_01_246_ts_tr: fd5e4ab195b9e347955f676c347c71a3; selected map 4e79a51bfa7cfeb96317f910323adeefb66052aaa013b46ee660be37099c8abc.
float4 ArtistNative826(ARTIST_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ArtistSourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[4].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[4].z = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[6].y = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[0u].wwww)).x;
    source[6].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[7].z = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[5].zwzz
    r0.xy = ((v4.xyxx)*(source[5].zwzz)).xy;
    // 2: mul r0.z, cb0[4].x, cb0[4].y
    r0.z = ((source[4].xxxx)*(source[4].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[5].y, r0.x
    r1.x = ((r0.zzzz)*(source[5].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[6].x, r0.y
    r1.y = ((r0.zzzz)*(source[6].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ArtistNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[6].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[6].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // 7: mul r0.x, r0.x, cb0[4].w
    r0.x = ((r0.xxxx)*(source[4].wwww)).x;
    // 8: mad r1.x, r0.z, cb0[4].z, r0.x
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.xxxx)).x;
    // 9: mul r0.x, r0.z, cb0[6].w
    r0.x = ((r0.zzzz)*(source[6].wwww)).x;
    // 10: mad r1.y, cb0[5].x, r0.y, r0.x
    r1.y = ((source[5].xxxx)*(r0.yyyy)+(r0.xxxx)).y;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ArtistNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 13: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 14: add r0.y, -cb0[3].x, l(1.000000)
    r0.y = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mad r0.x, r0.x, l(0.333330), -r0.y
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))+(-(r0.yyyy))).x;
    // 16: mul_sat r0.x, r0.x, cb0[7].x
    r0.x = (saturate((r0.xxxx)*(source[7].xxxx))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 22: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 23: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 24: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 25: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 26: mul r0.w, r0.w, cb0[7].z
    r0.w = ((r0.wwww)*(source[7].zzzz)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul_sat r0.w, r0.w, cb0[7].w
    r0.w = (saturate((r0.wwww)*(source[7].wwww))).w;
    // 29: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 30: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 31: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 32: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 33: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 34: add r0.xyz, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)+(source[2].xyzx)).xyz;
    // 35: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef ARTIST_NATIVE_MODEL_ONLY
// bfx_j_pa_lightdust_01_1_tr: 8a5cc23116d92a4d894fa89a15d5c64f; selected map 6575f98d15daef884390d85ac13cadd021cdff94e5225e70729808a38c568908.
float4 ArtistNative827(ARTIST_NATIVE_INPUT input)
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

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative828(ARTIST_NATIVE_INPUT input)
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

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_2_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ArtistNative829(ARTIST_NATIVE_INPUT input)
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

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_flowergarden_petal_01: d363ca0a56ab3549a00ddb9515e32efe; selected map 354af6192a9037300ddbc92e615e76fe8ea46591a20f629d15d318c6d01e043e.
float4 ArtistNative830(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[11u];
    source[2] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[4u].wwww,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[2u].xxxx,1u);
    source[4] = g_ArtistSourceMaterialParameters[8u];
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = g_ArtistSourceMaterialParameters[9u];
    source[9] = g_ArtistSourceMaterialParameters[7u];
    source[10] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[13].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[18].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[18].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[18].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[19].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 12: mul r0.yz, v4.xxyx, cb0[18].yyzy
    r0.yz = ((v4.xxyx)*(source[18].yyzy)).yz;
    // 13: mad r0.yz, cb0[13].yyyy, cb0[18].xxwx, r0.yyzy
    r0.yz = ((source[13].yyyy)*(source[18].xxwx)+(r0.yyzy)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 16: dp2 r1.x, cb0[10].xyxx, r0.zwzz
    r1.x = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 17: dp2 r1.y, cb0[11].xyxx, r0.zwzz
    r1.y = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 18: mad r0.zw, r0.zzzw, cb0[3].xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(source[3].xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.xzyw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 20: mad r1.xy, r1.xyxx, cb0[12].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(source[12].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (ArtistNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 22: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 23: mul_sat r0.y, r0.y, cb0[19].x
    r0.y = (saturate((r0.yyyy)*(source[19].xxxx))).y;
    // 24: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 25: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r0.w, r0.w, cb0[19].y
    r0.w = ((r0.wwww)*(source[19].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 29: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 31: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 32: or r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.yyyy))).x;
    // 33: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 34: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 35: log r0.x, |r0.z|
    r0.x = (log2(abs(r0.zzzz))).x;
    // 36: lt r0.y, |r0.z|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 37: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 38: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 39: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 40: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 41: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 42: mul r0.yz, r0.yyyy, v5.xxyx
    r0.yz = ((r0.yyyy)*(v5.xxyx)).yz;
    // 43: mul r1.xy, v4.xyxx, cb0[7].xyxx
    r1.xy = ((v4.xyxx)*(source[7].xyxx)).xy;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 45: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 47: mad r1.xyz, cb0[16].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 48: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 49: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 50: mul r1.xyz, r1.xyzx, cb0[16].zzzz
    r1.xyz = ((r1.xyzx)*(source[16].zzzz)).xyz;
    // 51: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 52: mul r2.xy, v4.xyxx, cb0[13].zwzz
    r2.xy = ((v4.xyxx)*(source[13].zwzz)).xy;
    // 53: mad r3.x, cb0[13].y, cb0[13].x, r2.x
    r3.x = ((source[13].yyyy)*(source[13].xxxx)+(r2.xxxx)).x;
    // 54: mad r3.y, cb0[13].y, cb0[14].x, r2.y
    r3.y = ((source[13].yyyy)*(source[14].xxxx)+(r2.yyyy)).y;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r3.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 56: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 58: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 61: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 63: mul r3.xyz, cb0[8].xyzx, cb0[8].wwww
    r3.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 64: mad r4.xy, r1.xyxx, r3.xyxx, r2.xyxx
    r4.xy = ((r1.xyxx)*(r3.xyxx)+(r2.xyxx)).xy;
    // 65: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 66: mul r3.xy, r4.xyxx, cb0[16].wwww
    r3.xy = ((r4.xyxx)*(source[16].wwww)).xy;
    // 67: mad r0.yz, r0.yyzy, cb0[6].xxyx, r3.xxyx
    r0.yz = ((r0.yyzy)*(source[6].xxyx)+(r3.xxyx)).yz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s3, l(0.000000)
    r0.yzw = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 69: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: add r3.xyz, -r0.yzwy, r1.wwww
    r3.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 71: mad r0.yzw, cb0[17].xxxx, r3.xxyz, r0.yyzw
    r0.yzw = ((source[17].xxxx)*(r3.xxyz)+(r0.yyzw)).yzw;
    // 72: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 73: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 74: mul r0.yzw, r0.yyzw, cb0[17].yyyy
    r0.yzw = ((r0.yyzw)*(source[17].yyyy)).yzw;
    // 75: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 76: mul r3.xyz, cb0[9].xyzx, cb0[9].wwww
    r3.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 77: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 78: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 79: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 80: mad r2.w, v1.z, r1.w, l(1.000000)
    r2.w = ((v1.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r3.xyz, r1.wwww, v1.xyzx
    r3.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 82: mul_sat r1.w, r2.w, l(0.500000)
    r1.w = (saturate((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 83: mad r1.w, r1.w, l(0.950000), l(0.050000)
    r1.w = ((r1.wwww)*(float4(0.950000,0.950000,0.950000,0.950000))+(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 84: dp3 r1.w, r2.xyzx, r1.wwww
    r1.w = (dot((r2.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 85: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 86: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 87: mul r2.w, r2.w, cb0[17].z
    r2.w = ((r2.wwww)*(source[17].zzzz)).w;
    // 88: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 89: mul r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)*(source[17].wwww)).w;
    // 90: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 91: mad r0.yzw, r1.wwww, r0.yyzw, r1.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 92: mul r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = ((r0.yyzw)*(r1.wwww)).yzw;
    // 93: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 94: mul r1.xyz, r1.xyzx, cb0[5].zzzz
    r1.xyz = ((r1.xyzx)*(source[5].zzzz)).xyz;
    // 95: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 96: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 97: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef ARTIST_NATIVE_MODEL_ONLY
// fx_m_flowergarden_petal_02: d363ca0a56ab3549a00ddb9515e32efe; selected map 354af6192a9037300ddbc92e615e76fe8ea46591a20f629d15d318c6d01e043e.
float4 ArtistNative831(ARTIST_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[1] = g_ArtistSourceMaterialParameters[11u];
    source[2] = ArtistNativeAppend(ArtistNativeAppend(g_ArtistSourceMaterialParameters[4u].wwww,g_ArtistSourceMaterialParameters[4u].wwww,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[1u].xxxx,g_ArtistSourceMaterialParameters[2u].xxxx,1u);
    source[4] = g_ArtistSourceMaterialParameters[8u];
    source[5] = input.dynamicParameter;
    source[6] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[3u].yyyy,g_ArtistSourceMaterialParameters[3u].wwww,1u);
    source[7] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[0u].wwww,g_ArtistSourceMaterialParameters[1u].wwww,1u);
    source[8] = g_ArtistSourceMaterialParameters[9u];
    source[9] = g_ArtistSourceMaterialParameters[7u];
    source[10] = ArtistNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[11] = ArtistNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ArtistSourceMaterialParameters[5u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[12] = ArtistNativeAppend(g_ArtistSourceMaterialParameters[5u].yyyy,g_ArtistSourceMaterialParameters[5u].zzzz,1u);
    source[13].x = (g_ArtistSourceMaterialParameters[2u].wwww).x;
    source[13].y = (g_ArtistSourceMaterialTime.xxxx).x;
    source[13].z = (g_ArtistSourceMaterialParameters[0u].yyyy).x;
    source[13].w = (g_ArtistSourceMaterialParameters[1u].zzzz).x;
    source[14].x = (g_ArtistSourceMaterialParameters[3u].zzzz).x;
    source[14].y = (g_ArtistSourceMaterialParameters[4u].wwww).x;
    source[14].z = (g_ArtistSourceMaterialParameters[2u].xxxx).x;
    source[14].w = (g_ArtistSourceMaterialParameters[1u].xxxx).x;
    source[15].x = (g_ArtistSourceMaterialParameters[5u].xxxx).x;
    source[15].y = (g_ArtistSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_ArtistSourceMaterialParameters[3u].yyyy).x;
    source[15].w = (g_ArtistSourceMaterialParameters[1u].wwww).x;
    source[16].x = (g_ArtistSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_ArtistSourceMaterialParameters[6u].zzzz).x;
    source[16].z = (g_ArtistSourceMaterialParameters[6u].wwww).x;
    source[16].w = (g_ArtistSourceMaterialParameters[0u].zzzz).x;
    source[17].x = (g_ArtistSourceMaterialParameters[4u].yyyy).x;
    source[17].y = (g_ArtistSourceMaterialParameters[4u].zzzz).x;
    source[17].z = (g_ArtistSourceMaterialParameters[3u].xxxx).x;
    source[17].w = (g_ArtistSourceMaterialParameters[4u].xxxx).x;
    source[18].x = (g_ArtistSourceMaterialParameters[2u].yyyy).x;
    source[18].y = (g_ArtistSourceMaterialParameters[0u].xxxx).x;
    source[18].z = (g_ArtistSourceMaterialParameters[1u].yyyy).x;
    source[18].w = (g_ArtistSourceMaterialParameters[2u].zzzz).x;
    source[19].x = (g_ArtistSourceMaterialParameters[6u].xxxx).x;
    source[19].y = (g_ArtistSourceMaterialParameters[6u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 12: mul r0.yz, v4.xxyx, cb0[18].yyzy
    r0.yz = ((v4.xxyx)*(source[18].yyzy)).yz;
    // 13: mad r0.yz, cb0[13].yyyy, cb0[18].xxwx, r0.yyzy
    r0.yz = ((source[13].yyyy)*(source[18].xxwx)+(r0.yyzy)).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s4, l(0.000000)
    r0.y = (ArtistNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 16: dp2 r1.x, cb0[10].xyxx, r0.zwzz
    r1.x = (dot((source[10].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 17: dp2 r1.y, cb0[11].xyxx, r0.zwzz
    r1.y = (dot((source[11].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 18: mad r0.zw, r0.zzzw, cb0[3].xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(source[3].xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.xzyw, s1, l(0.000000)
    r0.z = (ArtistNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 20: mad r1.xy, r1.xyxx, cb0[12].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(source[12].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (ArtistNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 22: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 23: mul_sat r0.y, r0.y, cb0[19].x
    r0.y = (saturate((r0.yyyy)*(source[19].xxxx))).y;
    // 24: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 25: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r0.w, r0.w, cb0[19].y
    r0.w = ((r0.wwww)*(source[19].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 29: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 31: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 32: or r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.yyyy))).x;
    // 33: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 34: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 35: log r0.x, |r0.z|
    r0.x = (log2(abs(r0.zzzz))).x;
    // 36: lt r0.y, |r0.z|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 37: mul r0.x, r0.x, cb0[15].x
    r0.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 38: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 39: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 40: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 41: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 42: mul r0.yz, r0.yyyy, v5.xxyx
    r0.yz = ((r0.yyyy)*(v5.xxyx)).yz;
    // 43: mul r1.xy, v4.xyxx, cb0[7].xyxx
    r1.xy = ((v4.xyxx)*(source[7].xyxx)).xy;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s2, l(0.000000)
    r1.xyz = (ArtistNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 45: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 47: mad r1.xyz, cb0[16].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 48: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 49: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 50: mul r1.xyz, r1.xyzx, cb0[16].zzzz
    r1.xyz = ((r1.xyzx)*(source[16].zzzz)).xyz;
    // 51: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 52: mul r2.xy, v4.xyxx, cb0[13].zwzz
    r2.xy = ((v4.xyxx)*(source[13].zwzz)).xy;
    // 53: mad r3.x, cb0[13].y, cb0[13].x, r2.x
    r3.x = ((source[13].yyyy)*(source[13].xxxx)+(r2.xxxx)).x;
    // 54: mad r3.y, cb0[13].y, cb0[14].x, r2.y
    r3.y = ((source[13].yyyy)*(source[14].xxxx)+(r2.yyyy)).y;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r3.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (ArtistNativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 56: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 58: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 61: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 63: mul r3.xyz, cb0[8].xyzx, cb0[8].wwww
    r3.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 64: mad r4.xy, r1.xyxx, r3.xyxx, r2.xyxx
    r4.xy = ((r1.xyxx)*(r3.xyxx)+(r2.xyxx)).xy;
    // 65: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 66: mul r3.xy, r4.xyxx, cb0[16].wwww
    r3.xy = ((r4.xyxx)*(source[16].wwww)).xy;
    // 67: mad r0.yz, r0.yyzy, cb0[6].xxyx, r3.xxyx
    r0.yz = ((r0.yyzy)*(source[6].xxyx)+(r3.xxyx)).yz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s3, l(0.000000)
    r0.yzw = (ArtistNativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 69: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: add r3.xyz, -r0.yzwy, r1.wwww
    r3.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 71: mad r0.yzw, cb0[17].xxxx, r3.xxyz, r0.yyzw
    r0.yzw = ((source[17].xxxx)*(r3.xxyz)+(r0.yyzw)).yzw;
    // 72: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 73: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 74: mul r0.yzw, r0.yyzw, cb0[17].yyyy
    r0.yzw = ((r0.yyzw)*(source[17].yyyy)).yzw;
    // 75: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 76: mul r3.xyz, cb0[9].xyzx, cb0[9].wwww
    r3.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 77: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 78: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 79: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 80: mad r2.w, v1.z, r1.w, l(1.000000)
    r2.w = ((v1.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r3.xyz, r1.wwww, v1.xyzx
    r3.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 82: mul_sat r1.w, r2.w, l(0.500000)
    r1.w = (saturate((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 83: mad r1.w, r1.w, l(0.950000), l(0.050000)
    r1.w = ((r1.wwww)*(float4(0.950000,0.950000,0.950000,0.950000))+(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 84: dp3 r1.w, r2.xyzx, r1.wwww
    r1.w = (dot((r2.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 85: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 86: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 87: mul r2.w, r2.w, cb0[17].z
    r2.w = ((r2.wwww)*(source[17].zzzz)).w;
    // 88: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 89: mul r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)*(source[17].wwww)).w;
    // 90: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 91: mad r0.yzw, r1.wwww, r0.yyzw, r1.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 92: mul r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = ((r0.yyzw)*(r1.wwww)).yzw;
    // 93: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 94: mul r1.xyz, r1.xyzx, cb0[5].zzzz
    r1.xyz = ((r1.xyzx)*(source[5].zzzz)).xyz;
    // 95: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 96: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 97: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
