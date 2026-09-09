// Single source owner for LanceVANative profiles 576..639.
#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_atta_10_05_ds_ad: 5302b738e52f7244969833048f8196db; selected map 5a42b67096426feec50af003e5c1f1dc13f9ebca11acf2ba140585714426d762.
float4 LanceVANative576(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[2].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[2].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[2].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[3].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[3].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xy = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_h_me_movepanlerp_01_2_ad: 79950b9981542c4587c2313e2866e5ac; selected map dd28fee51e89e66a4fdb11e90222259370a314bf6bd2824af80d6b3d9e2a7a2f.
float4 LanceVANative577(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[0u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0700000003, 0.0, 0.0, 0.0))),1u);
    source[8] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v4.xyxx, l(2.000000, 3.000000, 0.000000, 0.000000), cb0[7].xyxx
    r0.xy = ((v4.xyxx)*(float4(2.000000,3.000000,0.000000,0.000000))+(source[7].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: mad r0.xy, v4.xyxx, l(4.000000, 13.000000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v4.xyxx)*(float4(4.000000,13.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 5: add r0.xy, r0.xyxx, cb0[8].xyxx
    r0.xy = ((r0.xyxx)+(source[8].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mad r0.yz, v4.xxyx, l(0.000000, 8.000000, 8.000000, 0.000000), cb0[6].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,8.000000,8.000000,0.000000))+(source[6].xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.yz = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
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
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.z = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_34_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 LanceVANative578(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_23_tr: f77f84bb00757048a8cbf965b1d6f276; selected map 004a31e283bc7194223ff241d8722eaf6bc6ab70cdf452f4d336436cc61fcd58.
float4 LanceVANative579(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_21_tr: f77f84bb00757048a8cbf965b1d6f276; selected map 004a31e283bc7194223ff241d8722eaf6bc6ab70cdf452f4d336436cc61fcd58.
float4 LanceVANative580(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_10_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 LanceVANative581(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_pa_fd_01_05_tr: 7afa6d855bf8b548b5f2494a4bc8e70e; selected map 07ab742e1ef4991de3355893c0df10fc2865d099a8ddedbc4392e23407c3bb3b.
float4 LanceVANative583(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2] = g_LanceVASourceMaterialParameters[2u];
    source[3].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 10: add r0.y, -cb0[4].w, l(1.000000)
    r0.y = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 28: mad r0.z, cb0[3].w, v4.z, l(-1.000000)
    r0.z = ((source[3].wwww)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 29: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 30: mul r0.w, v4.z, cb0[3].w
    r0.w = ((v4.zzzz)*(source[3].wwww)).w;
    // 31: mad r0.zw, r0.wwww, v2.xxxy, -r0.zzzz
    r0.zw = ((r0.wwww)*(v2.xxxy)+(-(r0.zzzz))).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t0.xyzw, s2, l(0.000000)
    r1.xyz = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 33: log r0.z, |r1.x|
    r0.z = (log2(abs(r1.xxxx))).z;
    // 34: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 37: lt r0.w, |r1.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 38: movc r0.z, r0.w, l(0), |r0.z|
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.zzzz))).z;
    // 39: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 40: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 41: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 42: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 43: mul r0.w, r0.y, r0.w
    r0.w = ((r0.yyyy)*(r0.wwww)).w;
    // 44: ge r0.y, l(0.250000), r0.y
    r0.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.yyyy)) * 0xffffffffu)).y;
    // 45: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 46: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 48: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 49: mul r0.xzw, r1.xxyz, cb0[4].xxxx
    r0.xzw = ((r1.xxyz)*(source[4].xxxx)).xzw;
    // 50: dp3 r1.w, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: mad r1.xyz, -cb0[4].xxxx, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[4].xxxx))*(r1.xyzx)+(r1.wwww)).xyz;
    // 52: mad r0.xzw, cb0[4].yyyy, r1.xxyz, r0.xxzw
    r0.xzw = ((source[4].yyyy)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 53: mul r0.xzw, r0.xxzw, v3.xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)).xzw;
    // 54: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 55: movc r0.xyz, r0.yyyy, r1.xyzx, r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (r1.xyzx) : (r0.xzwx)).xyz;
    // 56: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 57: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 LanceVANative584(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2] = g_LanceVASourceMaterialParameters[1u];
    source[3].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xyzw = (LanceVANativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_j_pa_lbeam_01_1_ad: e916b8b428ce2d4f9e6c9c1f921545f6; selected map c6dd09439718068dd0b1df175b964425adda3531defec8efa86787f044491394.
float4 LanceVANative585(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_a_pa_ht_01_1_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 LanceVANative587(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2] = g_LanceVASourceMaterialParameters[2u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].wwww,g_LanceVASourceMaterialParameters[1u].xxxx,1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].zzzz,g_LanceVASourceMaterialParameters[1u].wwww,1u);
    source[5].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xy = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_03_tr: f77f84bb00757048a8cbf965b1d6f276; selected map 004a31e283bc7194223ff241d8722eaf6bc6ab70cdf452f4d336436cc61fcd58.
float4 LanceVANative588(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_flamesurface_01_01_ma: aceafb65292efd47b0bac9f86ac7a96e; selected map c1c3000834b25b638a3d7c6d863317badcd14a6b65ab07a2ff28250c8f15e6d3.
float4 LanceVANative589(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=input.color; // These native masked/local-VF variants place particle color and opacity in prefix row 0.
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.125, 0.0, 0.0, 0.0))),1u);
    source[3] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[4] = g_LanceVASourceMaterialParameters[3u];
    source[5] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7] = g_LanceVASourceMaterialParameters[2u];
    source[8] = input.dynamicParameter;
    source[9].x = (LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[9].z = ((g_LanceVASourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[10].x = (LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[10].z = ((g_LanceVASourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[10].w = ((g_LanceVASourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))).x;
    source[11].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[11].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[11].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[12].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: mul r1.xy, v4.xyxx, cb0[9].wwww
    r1.xy = ((v4.xyxx)*(source[9].wwww)).xy;
    // 5: mad r1.xy, r1.xyxx, l(0.800000, 0.800000, 0.000000, 0.000000), cb0[3].xyxx
    r1.xy = ((r1.xyxx)*(float4(0.800000,0.800000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
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
    // 13: mul r1.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 15: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 17: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 19: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 20: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: mad r1.xyz, r2.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))+(r1.xyzx)).xyz;
    // 22: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 23: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 24: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 25: dp3_sat r0.x, r0.xyzx, r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 26: mul r0.y, r0.x, cb0[11].z
    r0.y = ((r0.xxxx)*(source[11].zzzz)).y;
    // 27: add r0.z, r0.x, l(1.000000)
    r0.z = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 29: mad r0.z, r0.z, l(5.500000), l(-5.000000)
    r0.z = ((r0.zzzz)*(float4(5.500000,5.500000,5.500000,5.500000))+(float4(-5.000000,-5.000000,-5.000000,-5.000000))).z;
    // 30: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 32: add r0.w, cb0[8].x, cb0[12].x
    r0.w = ((source[8].xxxx)+(source[12].xxxx)).w;
    // 33: mul r1.xy, v4.xyxx, cb0[11].wwww
    r1.xy = ((v4.xyxx)*(source[11].wwww)).xy;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s4, l(0.000000)
    r1.x = (LanceVANativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 36: add r1.x, r1.x, cb0[8].x
    r1.x = ((r1.xxxx)+(source[8].xxxx)).x;
    // 37: mul r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)*(r1.xxxx)).x;
    // 38: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 39: round_ni_sat r0.y, r0.y
    r0.y = (saturate(floor(r0.yyyy))).y;
    // 40: round_ni_sat r0.w, r1.x
    r0.w = (saturate(floor(r1.xxxx))).w;
    // 41: add r0.w, -r0.w, r0.y
    r0.w = ((-(r0.wwww))+(r0.yyyy)).w;
    // 42: add r0.y, r0.y, l(-0.333300)
    r0.y = ((r0.yyyy)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 43: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 44: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) clip(-1.f);
    // 45: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 46: mul r1.xy, r2.xyxx, l(0.650000, 0.650000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(0.650000,0.650000,0.000000,0.000000))).xy;
    // 47: mad r1.zw, v4.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), r1.xxxy
    r1.zw = ((v4.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(r1.xxxy)).zw;
    // 48: mad r1.xy, v4.xyxx, l(1.500000, 1.500000, 0.000000, 0.000000), r1.xyxx
    r1.xy = ((v4.xyxx)*(float4(1.500000,1.500000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 49: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t4.yxzw, s3, l(0.000000)
    r0.y = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 51: add r1.xy, r1.zwzz, cb0[5].xyxx
    r1.xy = ((r1.zwzz)+(source[5].xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t4.xyzw, s3, l(0.000000)
    r1.x = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 53: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 54: mad r1.xy, v4.xyxx, l(1.500000, 1.500000, 0.000000, 0.000000), cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(float4(1.500000,1.500000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 56: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 57: mul r1.xyz, r1.xyzx, cb0[9].yyyy
    r1.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 58: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 59: mul r3.xyz, r0.yyyy, r3.xyzx
    r3.xyz = ((r0.yyyy)*(r3.xyzx)).xyz;
    // 60: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 61: mad r3.xyz, cb0[4].xyzx, l(0.000100, 0.000100, 0.000100, 0.000000), r3.xyzx
    r3.xyz = ((source[4].xyzx)*(float4(0.000100,0.000100,0.000100,0.000000))+(r3.xyzx)).xyz;
    // 62: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 63: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 64: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 65: mul r0.x, r0.x, l(1.850000)
    r0.x = ((r0.xxxx)*(float4(1.850000,1.850000,1.850000,1.850000))).x;
    // 66: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 67: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 68: log r0.x, r0.y
    r0.x = (log2(r0.yyyy)).x;
    // 69: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 70: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 71: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 72: mul r0.x, r0.x, cb0[11].y
    r0.x = ((r0.xxxx)*(source[11].yyyy)).x;
    // 73: mul r4.xyz, r0.xxxx, cb0[4].xyzx
    r4.xyz = ((r0.xxxx)*(source[4].xyzx)).xyz;
    // 74: movc r0.xyz, r0.yyyy, l(0,0,0,0), r4.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 75: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 76: add r1.w, -r2.z, l(1.000000)
    r1.w = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul r1.w, r1.w, l(2.500000)
    r1.w = ((r1.wwww)*(float4(2.500000,2.500000,2.500000,2.500000))).w;
    // 78: mul r2.w, |r1.w|, |r1.w|
    r2.w = ((abs(r1.wwww))*(abs(r1.wwww))).w;
    // 79: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 80: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 81: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 82: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 84: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 85: mad r0.xyz, r0.wwww, cb0[7].xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(source[7].xyzx)+(r0.xyzx)).xyz;
    // 86: mad o0.xyz, cb0[0].xyzx, r0.xyzx, cb0[1].xyzx
    output.xyz = ((source[0].xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_26_tr: f77f84bb00757048a8cbf965b1d6f276; selected map 004a31e283bc7194223ff241d8722eaf6bc6ab70cdf452f4d336436cc61fcd58.
float4 LanceVANative590(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_pa_flowmask_01_03_tr: a7f2f31eaeb6e240b8de40abdd7e5bae; selected map 5ef6b688a34a724e45145f48c375dddb8ad2172fe22a6672107272840e9e3973.
float4 LanceVANative591(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[7u];
    source[2] = g_LanceVASourceMaterialParameters[6u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[0u].wwww,1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].zzzz,g_LanceVASourceMaterialParameters[3u].wwww,1u);
    source[5] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),1u);
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].zzzz,g_LanceVASourceMaterialParameters[1u].wwww,1u);
    source[8] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[9] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].xxxx,g_LanceVASourceMaterialParameters[5u].yyyy,1u);
    source[10] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[4u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[12].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.x = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r2.zwzz, t0.xyzw, s0, l(0.000000)
    r1.x = (LanceVANativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r1.z = (LanceVANativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 29: mad r2.xy, v2.xyxx, cb0[3].xyxx, r1.xyxx
    r2.xy = ((v2.xyxx)*(source[3].xyxx)+(r1.xyxx)).xy;
    // 30: mov r0.w, v4.w
    r0.w = (v4.wwww).w;
    // 31: add r0.zw, r0.zzzw, r2.xxxy
    r0.zw = ((r0.zzzw)+(r2.xxxy)).zw;
    // 32: add r0.zw, r0.zzzw, cb0[6].xxxy
    r0.zw = ((r0.zzzw)+(source[6].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s1, l(0.000000)
    r2.xyz = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mad r0.zw, v2.xxxy, cb0[7].xxxy, r1.xxxy
    r0.zw = ((v2.xxxy)*(source[7].xxxy)+(r1.xxxy)).zw;
    // 35: add r0.zw, r0.zzzw, cb0[8].xxxy
    r0.zw = ((r0.zzzw)+(source[8].xxxy)).zw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.zwzz, t4.xyzw, s2, l(0.000000)
    r3.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.y = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_40_tr: 9765660da7e1414994a02fe197e8e364; selected map 63f30317cf581b133c3d9d7bb73869eefa79ea7a22b9497e57433c216f8431cc.
float4 LanceVANative592(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].wwww),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].wwww,g_LanceVASourceMaterialParameters[6u].xxxx,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos((g_LanceVASourceMaterialParameters[5u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[14].z = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[15].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_h_me_watertrail_01_2_tr: 70bf2a6e9bf4f0478cecbfc43c4e160f; selected map 54a52a78d4c82bd42962193bcb3a64e28cce1275eb69c9affb1d3006478abcc3.
float4 LanceVANative593(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[7].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[8].x = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[10].w = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[12].x = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[12].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[14].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[14].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[14].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[15].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r1.zw = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 27: mad r1.xy, r0.wwww, r1.zwzz, r1.xyxx
    r1.xy = ((r0.wwww)*(r1.zwzz)+(r1.xyxx)).xy;
    // 28: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 29: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 30: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.y = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_03_03_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 LanceVANative594(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[5] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[6] = input.dynamicParameter;
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz),1u);
    source[8] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz),1u);
    source[9] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[10] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[13].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[15].x = (cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[16].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r2.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_02_22_tr: f77f84bb00757048a8cbf965b1d6f276; selected map 004a31e283bc7194223ff241d8722eaf6bc6ab70cdf452f4d336436cc61fcd58.
float4 LanceVANative595(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[9] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[12].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[13].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_pa_trail_01_4_tr: 1946dbc9412ed54793435682a2c4fd1b; selected map 911f2043f65893d736a03e9ffd561354f4f4d547f3df0a2e543554d7a84a1945.
float4 LanceVANative596(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[4u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].wwww,g_LanceVASourceMaterialParameters[2u].xxxx,1u);
    source[3] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz))).x;
    source[8].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[9].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: mul r1.y, r0.z, cb0[6].y
    r1.y = ((r0.zzzz)*(source[6].yyyy)).y;
    // 30: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 31: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 32: mul r0.z, r0.z, l(0.800000)
    r0.z = ((r0.zzzz)*(float4(0.800000,0.800000,0.800000,0.800000))).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: lt r0.w, r0.x, l(0.000001)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 36: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 37: mul r1.zw, r0.zzzz, cb0[6].xxxz
    r1.zw = ((r0.zzzz)*(source[6].xxxz)).zw;
    // 38: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, 0.800000, -0.400000), r1.yyyw
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,0.800000,-0.400000))+(r1.yyyw)).zw;
    // 39: mad r1.yw, v4.zzzz, l(0.000000, 0.500000, 0.000000, -0.200000), r1.yyyw
    r1.yw = ((v4.zzzz)*(float4(0.000000,0.500000,0.000000,-0.200000))+(r1.yyyw)).yw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.ywyy, t0.yxzw, s0, l(0.000000)
    r1.y = (LanceVANativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: add r0.z, r1.y, r0.z
    r0.z = ((r1.yyyy)+(r0.zzzz)).z;
    // 43: mul r0.w, r1.y, cb0[6].w
    r0.w = ((r1.yyyy)*(source[6].wwww)).w;
    // 44: mad r1.yz, r0.wwww, v4.wwww, r1.xxzx
    r1.yz = ((r0.wwww)*(v4.wwww)+(r1.xxzx)).yz;
    // 45: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 46: mad r1.xw, v4.zzzz, l(0.400000, 0.000000, 0.000000, -0.600000), r1.yyyz
    r1.xw = ((v4.zzzz)*(float4(0.400000,0.000000,0.000000,-0.600000))+(r1.yyyz)).xw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xwxx, t2.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample2((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 48: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 49: mad r0.z, r0.z, l(0.500000), r0.w
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).z;
    // 50: mad r1.x, -r0.x, l(2.000000), l(1.000000)
    r1.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, -r0.x, cb0[8].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[8].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[9].y
    r0.x = (saturate((r0.xxxx)*(source[9].yyyy))).x;
    // 53: mul r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)*(source[9].zzzz)).x;
    // 54: mul r1.x, r1.x, l(0.666667)
    r1.x = ((r1.xxxx)*(float4(0.666667,0.666667,0.666667,0.666667))).x;
    // 55: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 56: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 57: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 58: add r1.xw, v4.xxxy, l(-1.000000, 0.000000, 0.000000, -1.000000)
    r1.xw = ((v4.xxxy)+(float4(-1.000000,0.000000,0.000000,-1.000000))).xw;
    // 59: mad r0.y, r0.z, r0.y, -r1.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(-(r1.wwww))).y;
    // 60: mad r1.xy, r1.xxxx, l(0.500000, -0.400000, 0.000000, 0.000000), r1.yzyy
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.400000,0.000000,0.000000))+(r1.yzyy)).xy;
    // 61: add r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)+(source[2].xyxx)).xy;
    // 62: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 64: dp2 r2.x, cb0[3].xyxx, r1.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[4].xyxx, r1.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 66: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
    r0.z = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 68: mul_sat r1.x, r0.z, l(20.000000)
    r1.x = (saturate((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000)))).x;
    // 69: mad_sat r0.x, r1.x, r0.y, -r0.x
    r0.x = (saturate((r1.xxxx)*(r0.yyyy)+(-(r0.xxxx)))).x;
    // 70: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 71: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 72: mul r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)*(r0.zzzz)).x;
    // 73: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 74: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 76: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 77: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 78: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 79: mad r0.x, r0.z, cb0[7].z, r0.x
    r0.x = ((r0.zzzz)*(source[7].zzzz)+(r0.xxxx)).x;
    // 80: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_o_me_ringmaster_01_02_ad: 3b6a039a9e0d654db11a318efadfeb87; selected map 591c1968f37783ec12eae8ea0fe9faf0c359c96420759b59ad35886d6cb18512.
float4 LanceVANative597(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[4u];
    source[3] = g_LanceVASourceMaterialParameters[3u];
    source[4] = LanceVANativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz))).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].wwww)).x;
    source[7].y = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[8].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].xxxx))).x;
    source[8].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[9].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[10].x = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[10].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[10].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 3: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 4: mad r0.w, -r0.z, cb0[6].z, l(1.000000)
    r0.w = ((-(r0.zzzz))*(source[6].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 5: mad r0.z, -r0.z, cb0[8].y, l(1.000000)
    r0.z = ((-(r0.zzzz))*(source[8].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 6: mul_sat r0.z, r0.z, cb0[9].y
    r0.z = (saturate((r0.zzzz)*(source[9].yyyy))).z;
    // 7: mul_sat r0.w, r0.w, cb0[7].z
    r0.w = (saturate((r0.wwww)*(source[7].zzzz))).w;
    // 8: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 10: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 11: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 12: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 13: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 14: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 15: dp2 r1.x, cb0[4].xyxx, r0.xyxx
    r1.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 16: dp2 r1.y, cb0[5].xyxx, r0.xyxx
    r1.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 17: mad r0.xy, cb0[10].yyyy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[10].yyyy)*(r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 19: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 20: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 21: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 22: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 23: mul r0.y, r0.y, cb0[10].w
    r0.y = ((r0.yyyy)*(source[10].wwww)).y;
    // 24: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 25: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 26: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 27: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 28: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 29: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 30: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 31: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 32: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_me_makeflow_03_22_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 LanceVANative598(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[5] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[6] = input.dynamicParameter;
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz),1u);
    source[8] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz),1u);
    source[9] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[10] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[13].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[15].x = (cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[16].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r2.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_pa_skull_02_15_tr: 456fc57bd455014b93b97e18c9390a4f; selected map 45f9f31688792848e09af99f5a64410bef1bac4520e1f2c9707f5c31e5b7fdc4.
float4 LanceVANative599(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[8u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[7u].yyyy,g_LanceVASourceMaterialParameters[7u].zzzz,1u);
    source[3] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))),1u);
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].yyyy,g_LanceVASourceMaterialParameters[4u].zzzz,1u);
    source[6] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(cos(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = LanceVANativeAppend(sin(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[0u].wwww,1u);
    source[10].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[10].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[10].z = ((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)).x;
    source[10].w = (((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))).x;
    source[11].x = (((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[11].y = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].z = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[12].x = (((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[12].y = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[12].z = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[12].w = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0)))).x;
    source[13].x = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].w = ((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[15].y = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[15].z = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[15].w = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[16].x = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[16].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[16].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[17].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[1u].xxxx))).x;
    source[17].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[17].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[17].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[18].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[18].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[18].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[19].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[19].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[19].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[19].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 1: mad r0.xy, v2.xyxx, cb0[11].yzyy, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[11].yzyy)+(source[2].xyxx)).xy;
    // 2: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, v2.xxyx, cb0[12].yyzy, cb0[4].xxyx
    r0.yz = ((v2.xxyx)*(source[12].yyzy)+(source[4].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, cb0[13].y
    r0.x = ((r0.xxxx)*(source[13].yyyy)).x;
    // 8: mad r0.yz, v2.xxyx, cb0[14].xxyx, cb0[5].xxyx
    r0.yz = ((v2.xxyx)*(source[14].xxyx)+(source[5].xxyx)).yz;
    // 9: mad r1.xy, cb0[15].xxxx, r0.xxxx, r0.yzyy
    r1.xy = ((source[15].xxxx)*(r0.xxxx)+(r0.yzyy)).xy;
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
    r0.w = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: mul r1.x, r0.x, cb0[15].x
    r1.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 17: mad r0.yz, r1.xxxx, cb0[15].wwww, r0.yyzy
    r0.yz = ((r1.xxxx)*(source[15].wwww)+(r0.yyzy)).yz;
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
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 38: mad r0.x, -r0.x, cb0[17].x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[17].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mul_sat r0.x, r0.x, cb0[18].x
    r0.x = (saturate((r0.xxxx)*(source[18].xxxx))).x;
    // 40: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 41: mul_sat r0.x, r0.x, cb0[18].y
    r0.x = (saturate((r0.xxxx)*(source[18].yyyy))).x;
    // 42: mad r0.x, r0.y, cb0[16].w, r0.x
    r0.x = ((r0.yyyy)*(source[16].wwww)+(r0.xxxx)).x;
    // 43: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 44: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 45: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 47: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 48: mul r0.y, r0.y, cb0[16].y
    r0.y = ((r0.yyyy)*(source[16].yyyy)).y;
    // 49: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 50: mad r0.y, r0.w, cb0[16].z, r0.y
    r0.y = ((r0.wwww)*(source[16].zzzz)+(r0.yyyy)).y;
    // 51: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 52: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 53: mul r0.y, v2.x, cb0[18].w
    r0.y = ((v2.xxxx)*(source[18].wwww)).y;
    // 54: mad r1.x, cb0[10].x, cb0[18].z, r0.y
    r1.x = ((source[10].xxxx)*(source[18].zzzz)+(r0.yyyy)).x;
    // 55: mul r0.y, v2.y, cb0[19].x
    r0.y = ((v2.yyyy)*(source[19].xxxx)).y;
    // 56: mad r1.y, cb0[10].x, cb0[19].y, r0.y
    r1.y = ((source[10].xxxx)*(source[19].yyyy)+(r0.yyyy)).y;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 58: add r0.z, v4.x, cb0[19].z
    r0.z = ((v4.xxxx)+(source[19].zzzz)).z;
    // 59: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 60: mul_sat r0.y, r0.y, cb0[19].w
    r0.y = (saturate((r0.yyyy)*(source[19].wwww))).y;
    // 61: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 62: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_pa_makeflow_02_04_tr: 6681761729fba24896145fb070705f70; selected map fdfa3146587d6e058328ddecec678833d5f6bda76940ba9f8bd2a35254a53ed4.
float4 LanceVANative601(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[7u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[5] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[6] = g_LanceVASourceMaterialParameters[6u];
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].yyyy,g_LanceVASourceMaterialParameters[5u].zzzz,1u);
    source[8] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[10].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[11].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[11].z = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[11].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[12].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[12].w = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)).x;
    source[13].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[14].x = (cos((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.w = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 34: mov r3.y, cb0[10].w
    r3.y = (source[10].wwww).y;
    // 35: mov r3.xz, l(0,0,0,0)
    r3.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 36: add r4.xyzw, r0.yzyz, r3.yxxy
    r4.xyzw = ((r0.yzyz)+(r3.yxxy)).xyzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r4.xyxx, t1.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r4.zwzz, t1.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r2.xyz = (LanceVANativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r3.xyz = (LanceVANativeSample2((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.y = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.z = (LanceVANativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_j_circleshine_01_1_ad: 0e6b5d5714d4eb4bb64a4cd1462e2fcc; selected map 5d170fe316c6dcdedebb7f152742380fca555e8bdbf338d52f0b82a8143a0086.
float4 LanceVANative602(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 27: mad r0.y, r0.y, l(0.159155), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 28: dp2 r1.x, r0.yyyy, v4.wwww
    r1.x = (dot((r0.yyyy).xy,(v4.wwww).xy).xxxx).x;
    // 29: mov r0.z, l(0)
    r0.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 30: mul r1.y, r0.x, l(0.040000)
    r1.y = ((r0.xxxx)*(float4(0.040000,0.040000,0.040000,0.040000))).y;
    // 31: mul r2.xy, v4.xzxx, cb0[2].xyxx
    r2.xy = ((v4.xzxx)*(source[2].xyxx)).xy;
    // 32: mul r0.w, r2.x, l(0.500000)
    r0.w = ((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 33: max r0.y, r2.y, l(0.000010)
    r0.y = (max(r2.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 34: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 35: mad r0.y, -r0.x, r0.y, l(1.000000)
    r0.y = ((-(r0.xxxx))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul_sat r0.y, r0.y, l(9.999998)
    r0.y = (saturate((r0.yyyy)*(float4(9.999998,9.999998,9.999998,9.999998)))).y;
    // 37: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: add r1.zw, r0.zzzw, r1.xxxy
    r1.zw = ((r0.zzzw)+(r1.xxxy)).zw;
    // 39: add r2.xyzw, r1.xwxw, l(-5.000000, 0.200000, -2.300000, 0.200000)
    r2.xyzw = ((r1.xwxw)+(float4(-5.000000,0.200000,-2.300000,0.200000))).xyzw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 41: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 42: mul r1.xyzw, r2.xyzw, l(2.500000, 1.000000, 1.500000, 1.000000)
    r1.xyzw = ((r2.xyzw)*(float4(2.500000,1.000000,1.500000,1.000000))).xyzw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t0.xyzw, s0, l(0.000000)
    r1.x = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 45: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 46: mul r0.w, r0.w, l(6.500000)
    r0.w = ((r0.wwww)*(float4(6.500000,6.500000,6.500000,6.500000))).w;
    // 47: max r1.x, v4.y, l(0.000010)
    r1.x = (max(v4.yyyy,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 48: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 49: mad r1.x, -r0.x, r1.x, l(1.000000)
    r1.x = ((-(r0.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 51: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 52: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 53: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 54: mul r1.y, r0.y, r1.y
    r1.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 55: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 56: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 57: add r1.x, v4.y, l(-0.100000)
    r1.x = ((v4.yyyy)+(float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 58: max r1.x, r1.x, l(0.000010)
    r1.x = (max(r1.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 59: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 60: mad r0.x, -r0.x, r1.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 61: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 62: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 63: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 64: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 65: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 66: mad r0.z, r0.z, r0.x, r0.z
    r0.z = ((r0.zzzz)*(r0.xxxx)+(r0.zzzz)).z;
    // 67: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 68: mad_sat r0.x, r0.z, r0.x, r0.w
    r0.x = (saturate((r0.zzzz)*(r0.xxxx)+(r0.wwww))).x;
    // 69: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 70: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 71: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 72: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 73: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_c_pa_lensflare_01_03_ad: 55883598a583ae4389d79bb0fdf1d392; selected map ae9ed651c317374306f77e3e999680cc10ae6be4f2b89b76263a55721100b6de.
float4 LanceVANative603(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[2].z = ((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))).x;
    source[2].w = (((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[3].x = (sin(((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].w = (((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].x = (sin(((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[4].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[4].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.yzw = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 LanceVANative604(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2] = g_LanceVASourceMaterialParameters[1u];
    source[3].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xyzw = (LanceVANativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_e_pa_ht_18_4_tr: a14d57c96465f346be3437ad63c9ad10; selected map 0716202d132c7fa3cb9c994bb5370a1db2710298fb3e58040e580c23ad3e1c1b.
float4 LanceVANative605(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[6u];
    source[2] = g_LanceVASourceMaterialParameters[5u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].wwww,g_LanceVASourceMaterialParameters[3u].xxxx,1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[1u].xxxx,1u);
    source[5] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].wwww,g_LanceVASourceMaterialParameters[1u].yyyy,1u);
    source[8] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[10].w = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)).x;
    source[11].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[11].w = (LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].x = (LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[13].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[3u].zzzz)).x;
    source[13].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[3u].zzzz))).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xy = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, cb0[11].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[11].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 4: mad r1.x, cb0[4].x, r0.x, cb0[5].x
    r1.x = ((source[4].xxxx)*(r0.xxxx)+(source[5].xxxx)).x;
    // 5: mad r1.y, cb0[4].y, r0.y, cb0[6].y
    r1.y = ((source[4].yyyy)*(r0.yyyy)+(source[6].yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mad r1.x, cb0[7].x, r0.x, cb0[8].x
    r1.x = ((source[7].xxxx)*(r0.xxxx)+(source[8].xxxx)).x;
    // 8: mad r1.y, cb0[7].y, r0.y, cb0[9].y
    r1.y = ((source[7].yyyy)*(r0.yyyy)+(source[9].yyyy)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_frag_07_02_tr: 21310dd53f047b4c9c6d215723e659ae; selected map 9a5065a684482b39573cad842bafa7252825dd4ce943af6e75781d622d09ea24.
float4 LanceVANative607(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[7]=float4(input.skyUpperColor,0.f);
    source[8]=float4(input.skyLowerColor,0.f);
    source[9]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].xxxx,g_LanceVASourceMaterialParameters[1u].yyyy,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].wwww,g_LanceVASourceMaterialParameters[2u].xxxx,1u);
    source[4].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[4].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xy = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[4].xxxx, cb0[3].xxxy, v2.xxxy
    r0.zw = ((source[4].xxxx)*(source[3].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[5].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_a_pa_db_02_1_tr: 9e4781052a66534b9646434db8d482db; selected map 4bdc6bcaf84fc1de676b6c243995d7aec03cd12fbb6a44cdf8836763e5c834ca.
float4 LanceVANative608(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[3]=float4(input.skyUpperColor,0.f);
    source[4]=float4(input.skyLowerColor,0.f);
    source[5]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r1.xyzw = (LanceVANativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_e_me_ht_03_4_ma: 3a96e00bdfda46489bb6aa32ae1ac89c; selected map 16becdfced0f83608f64db928c38304217f35b32aa207cf6a8f62437780e32c5.
float4 LanceVANative609(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0].w=1.f;
    source[6]=float4(input.skyUpperColor,0.f);
    source[7]=float4(input.skyLowerColor,0.f);
    source[8]=float4(input.ambientColor,input.skyIntensity);
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=input.color; // These native masked/local-VF variants place particle color and opacity in prefix row 0.
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3] = g_LanceVASourceMaterialParameters[1u];
    source[4].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
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
    // 8: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 9: mul r1.xy, v4.xyxx, cb0[4].xxxx
    r1.xy = ((v4.xyxx)*(source[4].xxxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 12: mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 14: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r0.w
    r1.w = (rsqrt(r0.wwww)).w;
    // 20: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 21: div r3.xyz, r1.xyzx, r0.wwww
    r3.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 22: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 23: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 24: mul r4.xyz, r0.wwww, r1.xyzx
    r4.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 25: mad r0.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 26: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 27: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 28: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 29: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 30: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 31: mul r5.xyz, r0.wwww, v0.xyzx
    r5.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 32: mul r6.xyz, r4.zxyz, r5.yzxy
    r6.xyz = ((r4.zxyz)*(r5.yzxy)).xyz;
    // 33: mad r6.xyz, r4.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r4.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 34: mul r6.xyz, r6.xzyx, v1.wwww
    r6.xyz = ((r6.xzyx)*(v1.wwww)).xyz;
    // 35: dp3 r7.y, r6.xzyx, r0.xyzx
    r7.y = (dot((r6.xzyx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 36: dp3 r7.x, r5.xyzx, r0.xyzx
    r7.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 37: dp3 r7.z, r4.xyzx, r0.xyzx
    r7.z = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 38: dp3 r0.y, r6.xzyx, r1.xyzx
    r0.y = (dot((r6.xzyx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 39: mov r6.x, r5.z
    r6.x = (r5.zzzz).x;
    // 40: dp3 r0.x, r5.xyzx, r1.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 41: mov r6.z, r4.z
    r6.z = (r4.zzzz).z;
    // 42: dp3 r0.z, r4.xyzx, r1.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 43: dp3_sat r0.w, r7.xyzx, r6.xyzx
    r0.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 44: dp3_sat r1.w, r3.xyzx, r6.xyzx
    r1.w = (saturate(dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 45: mul r3.x, r1.w, l(10.000000)
    r3.x = ((r1.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 46: mul r3.yzw, r1.wwww, cb0[3].xxyz
    r3.yzw = ((r1.wwww)*(source[3].xxyz)).yzw;
    // 47: min r1.w, r3.x, l(1.000000)
    r1.w = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 49: log r1.w, r0.w
    r1.w = (log2(r0.wwww)).w;
    // 50: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 51: mul r1.w, r1.w, cb0[4].y
    r1.w = ((r1.wwww)*(source[4].yyyy)).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: mul r1.w, r1.w, cb0[4].z
    r1.w = ((r1.wwww)*(source[4].zzzz)).w;
    // 54: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 55: mul r2.xyz, r2.xyzx, cb0[0].xyzx
    r2.xyz = ((r2.xyzx)*(source[0].xyzx)).xyz;
    // 56: mul r3.xyz, r3.yzwy, r2.xyzx
    r3.xyz = ((r3.yzwy)*(r2.xyzx)).xyz;
    // 57: mul r4.xyz, cb0[2].xyzx, cb0[2].wwww
    r4.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 58: mad r2.xyz, r4.xyzx, r2.xyzx, r3.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 59: mad r2.xyz, r2.wwww, r0.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r0.wwww)+(r2.xyzx)).xyz;
    // 60: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 62: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 63: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 66: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 67: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 68: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 69: mul r1.yzw, r1.yyyy, cb0[7].xxyz
    r1.yzw = ((r1.yyyy)*(source[7].xxyz)).yzw;
    // 70: mad r1.xyz, r1.xxxx, cb0[6].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[6].xyzx)+(r1.yzwy)).xyz;
    // 71: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 72: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[1].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[1].xyzx)).xyz;
    // 73: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 75: mad o0.xyz, r2.xyzx, cb0[8].xyzx, r3.xyzx
    output.xyz = ((r2.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // 77: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_pa_makeflow_03_03_tr: b7fba3d7f5a5dd4bac4b55216b447962; selected map cc98d515357c137c204955f6aab619bb64a9b46b3720cfe06ed57a959156e3d6.
float4 LanceVANative610(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[6u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[0u].wwww,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].zzzz,g_LanceVASourceMaterialParameters[3u].wwww,1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),1u);
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].xxxx,g_LanceVASourceMaterialParameters[0u].yyyy,1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].zzzz,g_LanceVASourceMaterialParameters[1u].wwww,1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].yyyy,g_LanceVASourceMaterialParameters[4u].zzzz,1u);
    source[9].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[9].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[9].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[10].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)).x;
    source[10].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[10].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[10].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[11].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[12].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r1.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 31: mov r2.w, cb0[9].z
    r2.w = (source[9].zzzz).w;
    // 32: mov r2.yz, l(0,0,0,0)
    r2.yz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yz;
    // 33: add r3.xyzw, r0.zwzw, r2.wzzw
    r3.xyzw = ((r0.zwzw)+(r2.wzzw)).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r3.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.zwzz, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: add r3.xy, -r1.xxxx, r0.zwzz
    r3.xy = ((-(r1.xxxx))+(r0.zwzz)).xy;
    // 37: mul r1.xy, r3.xyxx, l(8.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = ((r3.xyxx)*(float4(8.000000,8.000000,0.000000,0.000000))).xy;
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
    // 46: mov r3.x, v4.w
    r3.x = (v4.wwww).x;
    // 47: mov r3.y, l(0)
    r3.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 48: add r1.xy, r1.xyxx, r3.xyxx
    r1.xy = ((r1.xyxx)+(r3.xyxx)).xy;
    // 49: add r1.xy, r1.xyxx, cb0[5].xyxx
    r1.xy = ((r1.xyxx)+(source[5].xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 51: mad r2.zw, r0.xxxy, cb0[6].xxxy, r0.zzzw
    r2.zw = ((r0.xxxy)*(source[6].xxxy)+(r0.zzzw)).zw;
    // 52: add r2.zw, r2.zzzw, cb0[7].xxxy
    r2.zw = ((r2.zzzw)+(source[7].xxxy)).zw;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.zwzz, t3.xyzw, s2, l(0.000000)
    r3.xyz = (LanceVANativeSample2((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 54: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 55: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: mad r1.xyz, -r1.xyzx, r3.xyzx, r1.wwww
    r1.xyz = ((-(r1.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 57: mad r1.xyz, cb0[10].zzzz, r1.xyzx, r4.xyzx
    r1.xyz = ((source[10].zzzz)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 58: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 59: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 60: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 61: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 62: mul r1.xyz, r1.xyzx, cb0[11].xxxx
    r1.xyz = ((r1.xyzx)*(source[11].xxxx)).xyz;
    // 63: mad r1.xyz, v3.xyzx, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((v3.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 64: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 65: mov r2.x, v4.y
    r2.x = (v4.yyyy).x;
    // 66: mad r1.xy, r0.xyxx, cb0[8].xyxx, r2.xyxx
    r1.xy = ((r0.xyxx)*(source[8].xyxx)+(r2.xyxx)).xy;
    // 67: mad r0.yz, cb0[11].wwww, r0.zzwz, r1.xxyx
    r0.yz = ((source[11].wwww)*(r0.zzwz)+(r1.xxyx)).yz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s3, l(0.000000)
    r0.y = (LanceVANativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 69: max r0.y, |r0.y|, l(0.000001)
    r0.y = (max(abs(r0.yyyy),float4(0.000001,0.000001,0.000001,0.000001))).y;
    // 70: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 71: mul r0.y, r0.y, cb0[12].x
    r0.y = ((r0.yyyy)*(source[12].xxxx)).y;
    // 72: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 73: add r0.z, -|r0.x|, l(1.000000)
    r0.z = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: mul r0.x, r0.z, |r0.x|
    r0.x = ((r0.zzzz)*(abs(r0.xxxx))).x;
    // 75: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 76: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 77: mul r0.xy, r0.xyxx, cb0[12].zyzz
    r0.xy = ((r0.xyxx)*(source[12].zyzz)).xy;
    // 78: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 79: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 80: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 81: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 82: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_circ_01_2_tr: 653a5bb9cb1a754193f542b0fda91232; selected map b9a00a284058a01b21ea0d6fd6eec705cebc4a6f9f17beb3a1ba17da3210361f.
float4 LanceVANative611(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_LanceVASourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_LanceVASourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_LanceVASourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_LanceVASourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_LanceVASourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_me_movepanlerp_01_ad: 67258247381fa148892b36789bcdfd79; selected map 08f155ba2544fa6d28bf95b35bf75c831665a2e52d6a227c467c0964f973ffd4.
float4 LanceVANative612(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0700000003, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(0.0299999993, 0.0, 0.0, 0.0))),1u);
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v4.xyxx, l(2.000000, 3.000000, 0.000000, 0.000000), cb0[5].xyxx
    r0.xy = ((v4.xyxx)*(float4(2.000000,3.000000,0.000000,0.000000))+(source[5].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: mad r0.xy, v4.xyxx, l(4.000000, 13.000000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v4.xyxx)*(float4(4.000000,13.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 5: add r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mad r0.yz, v4.xxyx, l(0.000000, 8.000000, 8.000000, 0.000000), cb0[4].xxyx
    r0.yz = ((v4.xxyx)*(float4(0.000000,8.000000,8.000000,0.000000))+(source[4].xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.yz = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
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
    // 23: dp2 r1.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.yzyy
    r1.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).x;
    // 24: dp2 r1.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.yzyy
    r1.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.yzyy).xy).xxxx).y;
    // 25: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.z = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_n_pa_trail_01_08_tr: 1946dbc9412ed54793435682a2c4fd1b; selected map 911f2043f65893d736a03e9ffd561354f4f4d547f3df0a2e543554d7a84a1945.
float4 LanceVANative613(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[4u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].wwww,g_LanceVASourceMaterialParameters[2u].xxxx,1u);
    source[3] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_LanceVASourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz))).x;
    source[8].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[9].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: mul r1.y, r0.z, cb0[6].y
    r1.y = ((r0.zzzz)*(source[6].yyyy)).y;
    // 30: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 31: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 32: mul r0.z, r0.z, l(0.800000)
    r0.z = ((r0.zzzz)*(float4(0.800000,0.800000,0.800000,0.800000))).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: lt r0.w, r0.x, l(0.000001)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 36: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 37: mul r1.zw, r0.zzzz, cb0[6].xxxz
    r1.zw = ((r0.zzzz)*(source[6].xxxz)).zw;
    // 38: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, 0.800000, -0.400000), r1.yyyw
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,0.800000,-0.400000))+(r1.yyyw)).zw;
    // 39: mad r1.yw, v4.zzzz, l(0.000000, 0.500000, 0.000000, -0.200000), r1.yyyw
    r1.yw = ((v4.zzzz)*(float4(0.000000,0.500000,0.000000,-0.200000))+(r1.yyyw)).yw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.ywyy, t0.yxzw, s0, l(0.000000)
    r1.y = (LanceVANativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: add r0.z, r1.y, r0.z
    r0.z = ((r1.yyyy)+(r0.zzzz)).z;
    // 43: mul r0.w, r1.y, cb0[6].w
    r0.w = ((r1.yyyy)*(source[6].wwww)).w;
    // 44: mad r1.yz, r0.wwww, v4.wwww, r1.xxzx
    r1.yz = ((r0.wwww)*(v4.wwww)+(r1.xxzx)).yz;
    // 45: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 46: mad r1.xw, v4.zzzz, l(0.400000, 0.000000, 0.000000, -0.600000), r1.yyyz
    r1.xw = ((v4.zzzz)*(float4(0.400000,0.000000,0.000000,-0.600000))+(r1.yyyz)).xw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xwxx, t2.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample2((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 48: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 49: mad r0.z, r0.z, l(0.500000), r0.w
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).z;
    // 50: mad r1.x, -r0.x, l(2.000000), l(1.000000)
    r1.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, -r0.x, cb0[8].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[8].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[9].y
    r0.x = (saturate((r0.xxxx)*(source[9].yyyy))).x;
    // 53: mul r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)*(source[9].zzzz)).x;
    // 54: mul r1.x, r1.x, l(0.666667)
    r1.x = ((r1.xxxx)*(float4(0.666667,0.666667,0.666667,0.666667))).x;
    // 55: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 56: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 57: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 58: add r1.xw, v4.xxxy, l(-1.000000, 0.000000, 0.000000, -1.000000)
    r1.xw = ((v4.xxxy)+(float4(-1.000000,0.000000,0.000000,-1.000000))).xw;
    // 59: mad r0.y, r0.z, r0.y, -r1.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(-(r1.wwww))).y;
    // 60: mad r1.xy, r1.xxxx, l(0.500000, -0.400000, 0.000000, 0.000000), r1.yzyy
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.400000,0.000000,0.000000))+(r1.yzyy)).xy;
    // 61: add r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)+(source[2].xyxx)).xy;
    // 62: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 64: dp2 r2.x, cb0[3].xyxx, r1.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[4].xyxx, r1.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 66: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
    r0.z = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 68: mul_sat r1.x, r0.z, l(20.000000)
    r1.x = (saturate((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000)))).x;
    // 69: mad_sat r0.x, r1.x, r0.y, -r0.x
    r0.x = (saturate((r1.xxxx)*(r0.yyyy)+(-(r0.xxxx)))).x;
    // 70: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 71: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 72: mul r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)*(r0.zzzz)).x;
    // 73: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 74: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 76: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 77: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 78: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 79: mad r0.x, r0.z, cb0[7].z, r0.x
    r0.x = ((r0.zzzz)*(source[7].zzzz)+(r0.xxxx)).x;
    // 80: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_me_watertrail_01_39_tr: 94047f2453098c4babc74fdc1dbdcee4; selected map a142ee98604f07e857616aa30a2f6d3477654d32bc73de97f2c50c2a79b55695.
float4 LanceVANative614(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[7].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[8].x = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[11].x = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[11].y = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].z = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[11].w = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[12].x = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[12].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[14].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[14].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[14].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[15].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    // 6: mul r0.y, r0.y, cb0[15].y
    r0.y = ((r0.yyyy)*(source[15].yyyy)).y;
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
    r1.zw = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 27: mad r1.xy, r0.wwww, r1.zwzz, r1.xyxx
    r1.xy = ((r0.wwww)*(r1.zwzz)+(r1.xyxx)).xy;
    // 28: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 29: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 30: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.y = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 44: add r0.y, -r1.w, r0.y
    r0.y = ((-(r1.wwww))+(r0.yyyy)).y;
    // 45: mul_sat r0.y, r0.y, cb0[15].x
    r0.y = (saturate((r0.yyyy)*(source[15].xxxx))).y;
    // 46: mul r0.y, r0.y, r3.x
    r0.y = ((r0.yyyy)*(r3.xxxx)).y;
    // 47: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 48: mul_sat r0.x, r0.x, cb0[15].z
    r0.x = (saturate((r0.xxxx)*(source[15].zzzz))).x;
    // 49: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 50: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 51: mad r0.xyz, -cb0[1].xyzx, r1.xyzx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 52: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 53: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 54: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 56: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 57: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 58: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 59: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_e_me_ap_02_5_ts_tr: f3e012370cd62d4f9a62c9c2d507b775; selected map 3f47e48618d77fb4eb098e3fb3db46f5a5252affc813ce1120fd377c1569187b.
float4 LanceVANative615(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.xyzw = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_e_pa_fd_04_1_tr: a40cba34e2e4df4a88cb86692b690da3; selected map 075338ae49df4f3c7cf9d0e65f2455cc6070f44a45cb22739d8a2b49d724cf80.
float4 LanceVANative616(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2] = g_LanceVASourceMaterialParameters[2u];
    source[3].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 10: add r0.y, -cb0[4].w, l(1.000000)
    r0.y = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 28: mad r0.z, cb0[3].w, v4.z, l(-1.000000)
    r0.z = ((source[3].wwww)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 29: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 30: mul r0.w, v4.z, cb0[3].w
    r0.w = ((v4.zzzz)*(source[3].wwww)).w;
    // 31: mad r0.zw, r0.wwww, v2.xxxy, -r0.zzzz
    r0.zw = ((r0.wwww)*(v2.xxxy)+(-(r0.zzzz))).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t0.xyzw, s2, l(0.000000)
    r1.xyzw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 33: log r0.z, |r1.w|
    r0.z = (log2(abs(r1.wwww))).z;
    // 34: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 35: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 36: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 37: lt r0.w, |r1.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 38: movc r0.z, r0.w, l(0), |r0.z|
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.zzzz))).z;
    // 39: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 40: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 41: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 42: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 43: mul r0.w, r0.y, r0.w
    r0.w = ((r0.yyyy)*(r0.wwww)).w;
    // 44: ge r0.y, l(0.250000), r0.y
    r0.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.yyyy)) * 0xffffffffu)).y;
    // 45: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 46: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 48: movc o0.w, r0.z, l(0), r0.x
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 49: mul r0.xzw, r1.xxyz, cb0[4].xxxx
    r0.xzw = ((r1.xxyz)*(source[4].xxxx)).xzw;
    // 50: dp3 r1.w, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: mad r1.xyz, -cb0[4].xxxx, r1.xyzx, r1.wwww
    r1.xyz = ((-(source[4].xxxx))*(r1.xyzx)+(r1.wwww)).xyz;
    // 52: mad r0.xzw, cb0[4].yyyy, r1.xxyz, r0.xxzw
    r0.xzw = ((source[4].yyyy)*(r1.xxyz)+(r0.xxzw)).xzw;
    // 53: mul r0.xzw, r0.xxzw, v3.xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)).xzw;
    // 54: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 55: movc r0.xyz, r0.yyyy, r1.xyzx, r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (r1.xyzx) : (r0.xzwx)).xyz;
    // 56: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 57: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_o_me_windtrail_01_13_tr: eeb3bc7670738945a302b87117e6d641; selected map aa6b1a2668d37423f06c36da873710db9f87e4ae36140622dd3a77d8094bdf32.
float4 LanceVANative617(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[7].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[8].x = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].xxxx)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[10].w = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[12].x = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[12].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[13].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[14].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[15].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[16].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 10: mul_sat r0.x, r0.x, l(0.007143)
    r0.x = (saturate((r0.xxxx)*(float4(0.007143,0.007143,0.007143,0.007143)))).x;
    // 11: mul_sat r0.x, r0.x, cb0[16].x
    r0.x = (saturate((r0.xxxx)*(source[16].xxxx))).x;
    // 12: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 13: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 14: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 15: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 16: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 17: mul r0.z, r0.z, cb0[15].z
    r0.z = ((r0.zzzz)*(source[15].zzzz)).z;
    // 18: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 19: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 20: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 21: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 22: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 23: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 24: mul r1.x, r0.z, cb0[7].w
    r1.x = ((r0.zzzz)*(source[7].wwww)).x;
    // 25: mad r1.x, cb0[7].z, cb0[7].y, r1.x
    r1.x = ((source[7].zzzz)*(source[7].yyyy)+(r1.xxxx)).x;
    // 26: mul r1.z, r0.w, cb0[8].x
    r1.z = ((r0.wwww)*(source[8].xxxx)).z;
    // 27: mad r1.y, cb0[7].z, cb0[9].w, r1.z
    r1.y = ((source[7].zzzz)*(source[9].wwww)+(r1.zzzz)).y;
    // 28: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 29: add r1.z, cb0[3].w, cb0[12].x
    r1.z = ((source[3].wwww)+(source[12].xxxx)).z;
    // 30: add r1.z, r1.z, l(-1.000000)
    r1.z = ((r1.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 31: mul r2.xy, r0.zwzz, cb0[11].xyxx
    r2.xy = ((r0.zwzz)*(source[11].xyxx)).xy;
    // 32: mul r0.zw, r0.zzzw, cb0[14].xxxy
    r0.zw = ((r0.zzzw)*(source[14].xxxy)).zw;
    // 33: mad r1.w, cb0[7].z, cb0[10].w, r2.x
    r1.w = ((source[7].zzzz)*(source[10].wwww)+(r2.xxxx)).w;
    // 34: mad r2.x, cb0[7].z, cb0[11].z, r2.y
    r2.x = ((source[7].zzzz)*(source[11].zzzz)+(r2.yyyy)).x;
    // 35: mad r2.y, cb0[3].y, cb0[11].w, r2.x
    r2.y = ((source[3].yyyy)*(source[11].wwww)+(r2.xxxx)).y;
    // 36: mad r2.x, cb0[3].y, cb0[10].z, r1.w
    r2.x = ((source[3].yyyy)*(source[10].zzzz)+(r1.wwww)).x;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 38: mad r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((r1.zzzz)*(r2.xyxx)+(r1.xyxx)).xy;
    // 39: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 40: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 41: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r3.xyzw = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 44: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: mul r2.xyzw, r3.wxyz, cb0[1].wxyz
    r2.xyzw = ((r3.wxyz)*(source[1].wxyz)).xyzw;
    // 46: mad r0.z, cb0[7].z, cb0[13].w, r0.z
    r0.z = ((source[7].zzzz)*(source[13].wwww)+(r0.zzzz)).z;
    // 47: mad r0.w, cb0[7].z, cb0[14].z, r0.w
    r0.w = ((source[7].zzzz)*(source[14].zzzz)+(r0.wwww)).w;
    // 48: mad r4.y, cb0[3].y, cb0[14].w, r0.w
    r4.y = ((source[3].yyyy)*(source[14].wwww)+(r0.wwww)).y;
    // 49: mad r4.x, cb0[3].y, cb0[13].z, r0.z
    r4.x = ((source[3].yyyy)*(source[13].zzzz)+(r0.zzzz)).x;
    // 50: add r0.zw, r4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 51: dp2 r4.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.zwzz
    r4.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).x;
    // 52: dp2 r4.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.zwzz
    r4.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).y;
    // 53: add r0.zw, r4.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r4.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 55: add r0.z, -r1.w, r0.z
    r0.z = ((-(r1.wwww))+(r0.zzzz)).z;
    // 56: mul_sat r0.z, r0.z, cb0[15].x
    r0.z = (saturate((r0.zzzz)*(source[15].xxxx))).z;
    // 57: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 58: add r0.w, -v4.y, l(1.000000)
    r0.w = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 60: mul_sat r0.w, r0.w, cb0[15].y
    r0.w = (saturate((r0.wwww)*(source[15].yyyy))).w;
    // 61: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 62: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 63: mul_sat r0.y, r0.y, cb0[15].w
    r0.y = (saturate((r0.yyyy)*(source[15].wwww))).y;
    // 64: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 65: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 66: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 67: mad r0.xyz, -cb0[1].xyzx, r3.xyzx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r3.xyzx)+(r0.xxxx)).xyz;
    // 68: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r2.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r2.yzwy)).xyz;
    // 69: mul r2.xyz, r0.xyzx, cb0[13].yyyy
    r2.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 70: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 71: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 72: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 73: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 74: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 75: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 76: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_a_pa_db_01_2_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 LanceVANative618(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2] = g_LanceVASourceMaterialParameters[1u];
    source[3].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xyzw = (LanceVANativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_pa_ap_01_2_tr: 9bd32611087e674eb9772288fd730bed; selected map 668cc569785a03268ecb8380a49eff747caff479be009fad05ecfcee0ffc813c.
float4 LanceVANative619(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[7u];
    source[2] = g_LanceVASourceMaterialParameters[6u];
    source[3] = LanceVANativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = LanceVANativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[6].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[7].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[8].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[10].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[10].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[10].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[11].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx))).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.zw = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 10: mad r0.zw, cb0[7].zzzz, r0.zzzw, r0.xxxy
    r0.zw = ((source[7].zzzz)*(r0.zzzw)+(r0.xxxy)).zw;
    // 11: mul r1.xy, r0.zwzz, cb0[5].zwzz
    r1.xy = ((r0.zwzz)*(source[5].zwzz)).xy;
    // 12: mul r0.zw, r0.zzzw, cb0[8].yyyz
    r0.zw = ((r0.zzzw)*(source[8].yyyz)).zw;
    // 13: mad r0.zw, cb0[5].yyyy, cb0[8].xxxw, r0.zzzw
    r0.zw = ((source[5].yyyy)*(source[8].xxxw)+(r0.zzzw)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 15: mad r2.x, cb0[5].y, cb0[5].x, r1.x
    r2.x = ((source[5].yyyy)*(source[5].xxxx)+(r1.xxxx)).x;
    // 16: mad r2.y, cb0[5].y, cb0[7].w, r1.y
    r2.y = ((source[5].yyyy)*(source[7].wwww)+(r1.yyyy)).y;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_o_me_ring_05_6_ts_tr: 134755c6675ce84fae8ab2556db1fcdd; selected map 84eac35599ce44da0bad3fd3ca813fca5326f0a8844be9eb4f29516af097bbed.
float4 LanceVANative620(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3] = g_LanceVASourceMaterialParameters[2u];
    source[4] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(-0.200000003, 0.0, 0.0, 0.0))),1u);
    source[5] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(-0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)).x;
    source[7].y = (((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[7].z = (((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(-0.200000003, 0.0, 0.0, 0.0))).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].y = (LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(-0.200000003, 0.0, 0.0, 0.0)))).x;
    source[8].z = (LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[8].w = (((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].x = (LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)*float4(-0.100000001, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, -v4.x, l(1.000000)
    r0.x = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: mul r0.y, r0.x, cb0[8].x
    r0.y = ((r0.xxxx)*(source[8].xxxx)).y;
    // 3: mul r0.x, v4.y, cb0[7].w
    r0.x = ((v4.yyyy)*(source[7].wwww)).x;
    // 4: add r0.zw, r0.xxxy, cb0[4].xxxy
    r0.zw = ((r0.xxxy)+(source[4].xxxy)).zw;
    // 5: add r0.xy, r0.xyxx, cb0[5].xyxx
    r0.xy = ((r0.xyxx)+(source[5].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t0.yxzw, s1, l(0.000000)
    r0.y = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 8: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 9: mul r0.x, r0.x, cb0[9].y
    r0.x = ((r0.xxxx)*(source[9].yyyy)).x;
    // 10: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 11: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 16: mul_sat r0.y, r0.y, cb0[9].w
    r0.y = (saturate((r0.yyyy)*(source[9].wwww))).y;
    // 17: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 18: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 19: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 20: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s0, l(0.000000)
    r0.xyz = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 22: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 23: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 24: mul r0.xyz, r0.xyzx, cb0[6].xxxx
    r0.xyz = ((r0.xyzx)*(source[6].xxxx)).xyz;
    // 25: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 26: mul_sat r0.xyz, r0.xyzx, cb0[6].yyyy
    r0.xyz = (saturate((r0.xyzx)*(source[6].yyyy))).xyz;
    // 27: mad r0.xyz, r0.xyzx, cb0[3].xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[3].xyzx)+(source[1].xyzx)).xyz;
    // 28: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 29: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_helix_03_05_tr: dbbc5f8f7cdc3a44a98a801f6009e5d2; selected map f197c0d3db2137a8f7d09b413c54660e59eea0003bea6b2506e644eb22802810.
float4 LanceVANative621(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[4u];
    source[2] = LanceVANativeAppend(cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = LanceVANativeAppend(sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos(((float4(0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[8].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[8].y = ((float4(0.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].zzzz)).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 1: mul_sat r0.x, v4.x, l(0.660000)
    r0.x = (saturate((v4.xxxx)*(float4(0.660000,0.660000,0.660000,0.660000)))).x;
    // 2: add r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 3: dp2 r0.y, cb0[3].xyxx, r1.zwzz
    r0.y = (dot((source[3].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 4: dp4 r0.z, cb0[2].xyxy, r1.xyzw
    r0.z = (dot((source[2].xyxy).xyzw,(r1.xyzw).xyzw).xxxx).z;
    // 5: add r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 6: add r0.w, r0.y, v4.y
    r0.w = ((r0.yyyy)+(v4.yyyy)).w;
    // 7: mul r0.w, r0.w, cb0[5].z
    r0.w = ((r0.wwww)*(source[5].zzzz)).w;
    // 8: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 9: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 10: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 11: mul r0.x, r0.x, l(0.250000)
    r0.x = ((r0.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))).x;
    // 12: add r0.w, -r0.y, l(1.000000)
    r0.w = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: mul r1.x, r0.w, r0.y
    r1.x = ((r0.wwww)*(r0.yyyy)).x;
    // 14: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 15: mad r0.x, r0.x, l(4.000000), r0.z
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))+(r0.zzzz)).x;
    // 16: mul r0.zw, r0.xxxw, r0.xxxw
    r0.zw = ((r0.xxxw)*(r0.xxxw)).zw;
    // 17: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 18: mul r1.x, r1.x, v4.w
    r1.x = ((r1.xxxx)*(v4.wwww)).x;
    // 19: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 20: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 21: mad r0.y, r0.y, v4.z, l(-0.500000)
    r0.y = ((r0.yyyy)*(v4.zzzz)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 22: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 23: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 24: div r0.xz, r0.xxzx, r1.xxyx
    r0.xz = ((r0.xxzx)/(r1.xxyx)).xz;
    // 25: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 26: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mul r0.z, r1.x, r0.z
    r0.z = ((r1.xxxx)*(r0.zzzz)).z;
    // 28: mad_sat r0.x, r0.x, l(0.500000), l(0.500000)
    r0.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 29: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 30: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 31: mul r0.w, r1.x, cb0[5].w
    r0.w = ((r1.xxxx)*(source[5].wwww)).w;
    // 32: mad r1.xyzw, r0.wwww, l(0.050000, 0.050000, -0.050000, -0.050000), l(-0.025000, -0.025000, -0.025000, -0.025000)
    r1.xyzw = ((r0.wwww)*(float4(0.050000,0.050000,-0.050000,-0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).xyzw;
    // 33: mul r2.xyz, r0.xxxx, l(0.500000, 3.141593, -0.500000, 0.000000)
    r2.xyz = ((r0.xxxx)*(float4(0.500000,3.141593,-0.500000,0.000000))).xyz;
    // 34: sincos null, r0.w, r2.y
    r0.w = (cos(r2.yyyy)).w;
    // 35: mad r2.yz, r0.yyyy, l(0.000000, 2.000000, 2.000000, 0.000000), r2.xxzx
    r2.yz = ((r0.yyyy)*(float4(0.000000,2.000000,2.000000,0.000000))+(r2.xxzx)).yz;
    // 36: add r0.y, r0.w, l(1.000000)
    r0.y = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 37: mad r0.y, -r0.y, l(0.500000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: add r0.y, -r0.y, r0.x
    r0.y = ((-(r0.yyyy))+(r0.xxxx)).y;
    // 39: mad r2.x, r0.y, l(0.050000), r0.x
    r2.x = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))+(r0.xxxx)).x;
    // 40: add r2.w, -r2.x, l(1.000000)
    r2.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 42: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 43: mul r3.xyzw, r0.xxxx, v6.xyxy
    r3.xyzw = ((r0.xxxx)*(v6.xyxy)).xyzw;
    // 44: mad r0.xy, r1.zwzz, r3.zwzz, r2.wzww
    r0.xy = ((r1.zwzz)*(r3.zwzz)+(r2.wzww)).xy;
    // 45: mad r1.xy, r1.xyxx, r3.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)*(r3.xyxx)+(r2.xyxx)).xy;
    // 46: mul r1.zw, r0.xxxy, cb0[4].zzzw
    r1.zw = ((r0.xxxy)*(source[4].zzzw)).zw;
    // 47: mul r0.xy, r0.xyxx, cb0[6].zwzz
    r0.xy = ((r0.xyxx)*(source[6].zwzz)).xy;
    // 48: mad r2.x, cb0[4].y, cb0[4].x, r1.z
    r2.x = ((source[4].yyyy)*(source[4].xxxx)+(r1.zzzz)).x;
    // 49: mad r2.y, cb0[4].y, cb0[6].x, r1.w
    r2.y = ((source[4].yyyy)*(source[6].xxxx)+(r1.wwww)).y;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 51: mad r2.x, cb0[4].y, cb0[6].y, r0.x
    r2.x = ((source[4].yyyy)*(source[6].yyyy)+(r0.xxxx)).x;
    // 52: mad r2.y, cb0[4].y, cb0[7].x, r0.y
    r2.y = ((source[4].yyyy)*(source[7].xxxx)+(r0.yyyy)).y;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 54: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 55: mul r0.x, r0.x, cb0[7].y
    r0.x = ((r0.xxxx)*(source[7].yyyy)).x;
    // 56: mul r0.yw, r1.xxxy, cb0[4].zzzw
    r0.yw = ((r1.xxxy)*(source[4].zzzw)).yw;
    // 57: mul r1.xy, r1.xyxx, cb0[6].zwzz
    r1.xy = ((r1.xyxx)*(source[6].zwzz)).xy;
    // 58: mad r2.x, cb0[4].y, cb0[4].x, r0.y
    r2.x = ((source[4].yyyy)*(source[4].xxxx)+(r0.yyyy)).x;
    // 59: mad r2.y, cb0[4].y, cb0[6].x, r0.w
    r2.y = ((source[4].yyyy)*(source[6].xxxx)+(r0.wwww)).y;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 61: mad r2.x, cb0[4].y, cb0[6].y, r1.x
    r2.x = ((source[4].yyyy)*(source[6].yyyy)+(r1.xxxx)).x;
    // 62: mad r2.y, cb0[4].y, cb0[7].x, r1.y
    r2.y = ((source[4].yyyy)*(source[7].xxxx)+(r1.yyyy)).y;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 64: mad r0.x, r0.y, r0.w, r0.x
    r0.x = ((r0.yyyy)*(r0.wwww)+(r0.xxxx)).x;
    // 65: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 66: max r0.x, r0.x, l(0.000001)
    r0.x = (max(r0.xxxx,float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 67: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 68: mul r0.x, r0.x, cb0[7].z
    r0.x = ((r0.xxxx)*(source[7].zzzz)).x;
    // 69: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 70: mad r0.x, cb0[7].w, r0.x, cb0[8].y
    r0.x = ((source[7].wwww)*(r0.xxxx)+(source[8].yyyy)).x;
    // 71: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 72: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 73: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 74: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_x_me_wind_04_03_ts_ad: 39367451901e2c45998dfab345ebe5bd; selected map b7f851ed8f76b46c929ae92c10eb2ae104e5005eeb9781a4709d31b42da5a3dc.
float4 LanceVANative622(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[6u];
    source[3] = LanceVANativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = LanceVANativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[5].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[7].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[8].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[8].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[9].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[10].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[10].z = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[11].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx))).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[3].xyxx, r0.xyxx
    r1.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.xyxx
    r1.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 4: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: add r0.z, -r0.x, l(1.000000)
    r0.z = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 6: mul_sat r0.z, r0.x, r0.z
    r0.z = (saturate((r0.xxxx)*(r0.zzzz))).z;
    // 7: mul r0.w, r0.z, l(4.000000)
    r0.w = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 8: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 9: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 10: mul r0.w, r0.w, cb0[5].z
    r0.w = ((r0.wwww)*(source[5].zzzz)).w;
    // 11: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 12: mul r0.w, r0.w, cb0[5].w
    r0.w = ((r0.wwww)*(source[5].wwww)).w;
    // 13: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 14: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 15: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 16: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 17: mul r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)*(source[6].yyyy)).w;
    // 18: lt r1.x, |r0.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 20: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 21: mul r0.w, r0.x, cb0[7].w
    r0.w = ((r0.xxxx)*(source[7].wwww)).w;
    // 22: mad r1.x, cb0[6].w, cb0[7].z, r0.w
    r1.x = ((source[6].wwww)*(source[7].zzzz)+(r0.wwww)).x;
    // 23: mul r0.w, cb0[6].w, cb0[8].y
    r0.w = ((source[6].wwww)*(source[8].yyyy)).w;
    // 24: mad r1.y, cb0[8].x, r0.y, r0.w
    r1.y = ((source[8].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 26: mad r0.xy, cb0[8].zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((source[8].zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 27: mul r1.xy, r0.xyxx, cb0[9].yzyy
    r1.xy = ((r0.xyxx)*(source[9].yzyy)).xy;
    // 28: mul r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)*(source[7].xyxx)).xy;
    // 29: mad r1.xy, cb0[6].wwww, cb0[9].xwxx, r1.xyxx
    r1.xy = ((source[6].wwww)*(source[9].xwxx)+(r1.xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 31: mad r1.x, cb0[6].w, cb0[6].z, r0.x
    r1.x = ((source[6].wwww)*(source[6].zzzz)+(r0.xxxx)).x;
    // 32: mad r1.y, cb0[6].w, cb0[8].w, r0.y
    r1.y = ((source[6].wwww)*(source[8].wwww)+(r0.yyyy)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 34: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 35: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 36: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 37: mul r0.x, r0.x, cb0[10].x
    r0.x = ((r0.xxxx)*(source[10].xxxx)).x;
    // 38: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 39: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 40: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 41: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 42: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 43: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 44: source device depth mapped to centimetre view depth; reconstruction at 46.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 46-49: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 50: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 51: add r0.z, -cb0[11].y, l(1.000000)
    r0.z = ((-(source[11].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 53: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 54: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 55: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 56: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 57: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 58: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 59: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 60: mul r0.w, r0.w, cb0[10].z
    r0.w = ((r0.wwww)*(source[10].zzzz)).w;
    // 61: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 62: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 63: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 64: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 65: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 66: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 67: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 68: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 69: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 70: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_n_me_ap_07_5_ts_tr: f3e012370cd62d4f9a62c9c2d507b775; selected map 3f47e48618d77fb4eb098e3fb3db46f5a5252affc813ce1120fd377c1569187b.
float4 LanceVANative623(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.xyzw = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_y_me_makeflow_03_21_tr: 5db7b7be4bce824eb486c9d7ae062e1f; selected map 5a1d0845cce484fa6b92bf759529b3193bb35f872d747fe030f96a967ee97f46.
float4 LanceVANative624(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[5] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[6] = input.dynamicParameter;
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz),1u);
    source[8] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz),1u);
    source[9] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].zzzz,g_LanceVASourceMaterialParameters[5u].wwww,1u);
    source[10] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[12].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[13].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[15].x = (cos((g_LanceVASourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[15].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[16].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.x = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r2.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_c_pa_glitter_01_tr: bd8398a6efa91243bb7de7f4bed27970; selected map 41c5e36935706c430baa850216169d8ff6ba9cb558897004f6967d0a0c2bf07e.
float4 LanceVANative625(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[2].w = ((g_LanceVASourceMaterialParameters[0u].zzzz*float4(3.5, 0.0, 0.0, 0.0))).x;
    source[3].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 1: mul r0.xyzw, v2.xyxy, l(0.500000, 0.500000, 0.700000, 0.700000)
    r0.xyzw = ((v2.xyxy)*(float4(0.500000,0.500000,0.700000,0.700000))).xyzw;
    // 2: mad r0.xy, v4.wwww, l(0.100000, -0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v4.wwww)*(float4(0.100000,-0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, r0.xxxx, l(0.000000, 0.400000, 0.400000, 0.000000), r0.zzwz
    r0.yz = ((r0.xxxx)*(float4(0.000000,0.400000,0.400000,0.000000))+(r0.zzwz)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s4, l(0.000000)
    r0.w = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s1, l(0.000000)
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: add r0.z, r0.w, r0.w
    r0.z = ((r0.wwww)+(r0.wwww)).z;
    // 8: mul r0.w, r0.x, l(0.200000)
    r0.w = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 9: add r1.xy, r0.xxxx, v2.xyxx
    r1.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t5.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.xy, v2.xyxx, l(0.950000, 0.950000, 0.000000, 0.000000), r0.wwww
    r1.xy = ((v2.xyxx)*(float4(0.950000,0.950000,0.000000,0.000000))+(r0.wwww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (LanceVANativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 13: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 14: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 15: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 16: mul r1.x, r0.z, l(5.000000)
    r1.x = ((r0.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 17: mad r0.w, r0.w, l(150.000000), r1.x
    r0.w = ((r0.wwww)*(float4(150.000000,150.000000,150.000000,150.000000))+(r1.xxxx)).w;
    // 18: mul r1.xyzw, v2.xyxy, cb0[2].zzww
    r1.xyzw = ((v2.xyxy)*(source[2].zzww)).xyzw;
    // 19: mad r1.xy, cb0[2].yyyy, cb0[2].xxxx, r1.xyxx
    r1.xy = ((source[2].yyyy)*(source[2].xxxx)+(r1.xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t3.yzxw, s3, l(0.000000)
    r1.z = (LanceVANativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.x = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 22: mul r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)*(r1.xxxx)).x;
    // 23: mul r1.x, r1.x, cb0[3].x
    r1.x = ((r1.xxxx)*(source[3].xxxx)).x;
    // 24: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 25: mul r1.x, r0.x, r1.x
    r1.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 26: mad r0.y, r1.x, l(30.000000), r0.y
    r0.y = ((r1.xxxx)*(float4(30.000000,30.000000,30.000000,30.000000))+(r0.yyyy)).y;
    // 27: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 28: mad r1.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 29: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 30: mul r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)*(r0.zzzz)).x;
    // 31: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 32: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_h_me_fd_02_1_ts_tr: 6f7d48813f113e4f9e74351373f7d1e3; selected map ca7bd339e01a327c65bba9fc0db7e33c191e6200b995be7f47c35be66d8957e1.
float4 LanceVANative626(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    // 28: mul r1.xy, r0.yyyy, cb0[4].wyww
    r1.xy = ((r0.yyyy)*(source[4].wyww)).xy;
    // 29: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 30: mul r0.yz, r0.yyyy, l(0.000000, 0.200000, 0.400000, 0.000000)
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.200000,0.400000,0.000000))).yz;
    // 31: exp r0.yz, r0.yyzy
    r0.yz = (exp2(r0.yyzy)).yz;
    // 32: mul r0.z, r0.z, cb0[4].z
    r0.z = ((r0.zzzz)*(source[4].zzzz)).z;
    // 33: mul r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)*(source[5].xxxx)).y;
    // 34: lt r0.w, r0.x, l(0.000001)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 36: mad r2.xy, -r0.xxxx, l(4.000000, 5.000000, 0.000000, 0.000000), l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r0.xxxx))*(float4(4.000000,5.000000,0.000000,0.000000))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 37: mul r2.xy, r2.xyxx, l(0.400000, 0.200000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(0.400000,0.200000,0.000000,0.000000))).xy;
    // 38: max r2.xy, r2.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (max(r2.xyxx,float4(0.000000,0.000000,0.000000,0.000000))).xy;
    // 39: movc r1.zw, r0.wwww, l(0,0,0,0), r0.zzzy
    r1.zw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzy)).zw;
    // 40: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 41: mul r0.z, cb0[3].x, cb0[4].x
    r0.z = ((source[3].xxxx)*(source[4].xxxx)).z;
    // 42: mov r0.y, -r0.z
    r0.y = (-(r0.zzzz)).y;
    // 43: add r0.xy, r1.yzyy, r0.xyxx
    r0.xy = ((r1.yzyy)+(r0.xyxx)).xy;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).x;
    // 45: mad r1.xyzw, r0.xxxx, l(0.500000, 0.500000, 0.500000, 0.500000), r1.xwxw
    r1.xyzw = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(r1.xwxw)).xyzw;
    // 46: mad r1.xyzw, r0.zzzz, l(-0.300000, -0.700000, -0.010000, -0.500000), r1.xyzw
    r1.xyzw = ((r0.zzzz)*(float4(-0.300000,-0.700000,-0.010000,-0.500000))+(r1.xyzw)).xyzw;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r1.zwzz, t1.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yzxw).z;
    // 49: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 50: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 51: add_sat r0.z, -r2.y, r0.x
    r0.z = (saturate((-(r2.yyyy))+(r0.xxxx))).z;
    // 52: mul r0.z, r0.z, l(0.200000)
    r0.z = ((r0.zzzz)*(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 53: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 1.200000, 1.200000), r0.zzzz
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,1.200000,1.200000))+(r0.zzzz)).zw;
    // 54: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.100000, -0.100000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.100000,-0.100000))).zw;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.xyzw, s1, l(0.000000)
    r0.z = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
    // 56: mul r0.xw, r0.zzzz, r0.xxxy
    r0.xw = ((r0.zzzz)*(r0.xxxy)).xw;
    // 57: mul r0.z, |r0.w|, |r0.w|
    r0.z = ((abs(r0.wwww))*(abs(r0.wwww))).z;
    // 58: mul r0.z, r0.z, |r0.w|
    r0.z = ((r0.zzzz)*(abs(r0.wwww))).z;
    // 59: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 60: mul r0.z, r0.z, l(2000.000000)
    r0.z = ((r0.zzzz)*(float4(2000.000000,2000.000000,2000.000000,2000.000000))).z;
    // 61: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 62: mad r0.x, r0.x, l(3.000000), r0.z
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))+(r0.zzzz)).x;
    // 63: add_sat r0.z, -r2.y, r0.y
    r0.z = (saturate((-(r2.yyyy))+(r0.yyyy))).z;
    // 64: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 65: mad r0.xzw, r0.xxxx, cb0[1].xxyz, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[1].xxyz)+(source[2].xxyz)).xzw;
    // 66: mad o0.xyz, r0.xzwx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xzwx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 67: mul r0.x, r0.y, l(10.000000)
    r0.x = ((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 68: mad r0.yz, r0.yyyy, l(0.000000, 0.600000, 0.600000, 0.000000), v4.xxyx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.600000,0.600000,0.000000))+(v4.xxyx)).yz;
    // 69: add r0.yz, r0.yyzy, l(0.000000, -0.530000, -0.520000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.530000,-0.520000,0.000000))).yz;
    // 70: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 71: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 72: mad r0.y, -r0.y, l(2.857143), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.857143,2.857143,2.857143,2.857143))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 73: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 74: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 75: mul r0.y, r0.y, l(20.000000)
    r0.y = ((r0.yyyy)*(float4(20.000000,20.000000,20.000000,20.000000))).y;
    // 76: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 77: mad r0.x, r2.x, l(10.000000), r0.x
    r0.x = ((r2.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 78: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 79: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 80: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 81: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 82: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 83: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 84: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r0.z, r0.z, l(1.050000)
    r0.z = ((r0.zzzz)*(float4(1.050000,1.050000,1.050000,1.050000))).z;
    // 86: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 87: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 88: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 89: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t2.yxzw, s3, l(0.000000)
    r0.y = (LanceVANativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 90: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 91: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_me_flow_02_05_tr: 1a34f909fe3faf478223c87a92ffdc9c; selected map cbbc0273b92e5b1b9403cf8dda28c267142b352f90fbfc34a6ecd4e4f558e517.
float4 LanceVANative627(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[6u];
    source[3] = LanceVANativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = LanceVANativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = input.dynamicParameter;
    source[6] = g_LanceVASourceMaterialParameters[4u];
    source[7].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[8].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[9].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[9].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[10].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[10].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[10].w = ((float4(0.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.x, cb0[5].z, cb0[8].z
    r0.x = ((source[5].zzzz)*(source[8].zzzz)).x;
    // 2: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: dp2 r1.y, cb0[4].xyxx, r0.yzyy
    r1.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 4: dp2 r1.x, cb0[3].xyxx, r0.yzyy
    r1.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 5: mad_sat r0.y, r1.y, r0.x, l(0.500000)
    r0.y = (saturate((r1.yyyy)*(r0.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000)))).y;
    // 6: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: add r0.x, r1.x, l(0.500000)
    r0.x = ((r1.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 8: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 9: add r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)+(r0.zzzz)).z;
    // 10: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 11: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 12: mul r1.xy, r0.xyxx, cb0[7].zwzz
    r1.xy = ((r0.xyxx)*(source[7].zwzz)).xy;
    // 13: mad r2.x, cb0[7].y, cb0[7].x, r1.x
    r2.x = ((source[7].yyyy)*(source[7].xxxx)+(r1.xxxx)).x;
    // 14: mad r2.y, cb0[7].y, cb0[8].w, r1.y
    r2.y = ((source[7].yyyy)*(source[8].wwww)+(r1.yyyy)).y;
    // 15: mul r1.xy, r2.xyxx, l(1.660000, 1.660000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(1.660000,1.660000,0.000000,0.000000))).xy;
    // 16: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s0, l(-1.000000)
    r2.xyz = (LanceVANativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 17: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s0, l(-1.000000)
    r1.xyz = (LanceVANativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 18: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 19: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: mad r1.xyz, -r2.xyzx, r1.xyzx, r0.wwww
    r1.xyz = ((-(r2.xyzx))*(r1.xyzx)+(r0.wwww)).xyz;
    // 21: mad r1.xyz, cb0[9].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[9].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 22: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 23: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 24: mul r1.xyz, r1.xyzx, cb0[9].yyyy
    r1.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 25: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 26: mul r2.xyz, cb0[6].xyzx, cb0[6].wwww
    r2.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 27: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 28: add r0.w, cb0[5].y, l(-1.000000)
    r0.w = ((source[5].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 29: add r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)+(r0.wwww)).x;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.xyxx, t2.yzwx, s1, l(0.000000)
    r0.w = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 31: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(-1.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 32: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 33: mul_sat r0.x, r0.x, cb0[10].x
    r0.x = (saturate((r0.xxxx)*(source[10].xxxx))).x;
    // 34: log r0.y, |r0.w|
    r0.y = (log2(abs(r0.wwww))).y;
    // 35: lt r0.z, |r0.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 36: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 37: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 38: mul r0.y, r0.y, cb0[9].w
    r0.y = ((r0.yyyy)*(source[9].wwww)).y;
    // 39: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 40: mad r0.yzw, r0.yyyy, r1.xxyz, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)+(r1.xxyz)).yzw;
    // 41: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 42: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 43: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 44: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 45: mul r0.y, r0.y, cb0[10].y
    r0.y = ((r0.yyyy)*(source[10].yyyy)).y;
    // 46: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 47: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 48: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 49: add r0.x, r0.x, cb0[10].w
    r0.x = ((r0.xxxx)+(source[10].wwww)).x;
    // 50: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_master_01_004_tr: 34cbd1a06701bd41885b14be1d72fa8c; selected map 70e27fe90eea57776e7c7e9fa3b9b34164e261d72a655851bbf87160df47065e.
float4 LanceVANative628(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[4u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].yyyy,g_LanceVASourceMaterialParameters[2u].zzzz,1u);
    source[3].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[3].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[5].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].wwww)).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)).x;
    source[6].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xy = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.z = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: mad r0.w, cb0[3].y, cb0[6].y, cb0[6].z
    r0.w = ((source[3].yyyy)*(source[6].yyyy)+(source[6].zzzz)).w;
    // 14: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
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
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    // 34: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 35: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 36: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_pa_spritewave_01_99_tr: 1b614321d13a674ba81d24973bb6b184; selected map 14be505a134258ff472c4ecfa430347520d50d720964438b3b672c805ef17b2f.
float4 LanceVANative629(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[11u];
    source[2] = LanceVANativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = LanceVANativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[8u].wwww,g_LanceVASourceMaterialParameters[9u].xxxx,1u);
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[6] = LanceVANativeAppend(cos(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = LanceVANativeAppend(sin(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_LanceVASourceMaterialParameters[10u];
    source[11].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[11].y = (cos(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[11].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[12].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[13].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[7u].wwww).x;
    source[13].w = (g_LanceVASourceMaterialParameters[8u].yyyy).x;
    source[14].x = (g_LanceVASourceMaterialParameters[8u].zzzz).x;
    source[14].y = (g_LanceVASourceMaterialParameters[8u].xxxx).x;
    source[14].z = (g_LanceVASourceMaterialParameters[9u].xxxx).x;
    source[14].w = (g_LanceVASourceMaterialParameters[8u].wwww).x;
    source[15].x = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[15].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[15].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[16].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[16].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[16].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[17].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[17].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[17].z = ((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[17].w = (sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[18].y = (cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].z = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[18].w = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[19].x = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[19].y = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[19].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[19].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[20].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[20].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[20].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[20].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[21].x = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[21].y = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[21].z = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[21].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.x = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
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
    r0.w = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.w = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_atta_02_04_ts_tr: 17c9d4932226e946bf2e955856dfa6d6; selected map 9ca95df33d6b6533f7bd157a3f6115897d9ab654c558a7dcb663677617bee9b0.
float4 LanceVANative630(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[2].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[2].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[3].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xy = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 12: mul r0.w, r0.w, cb0[4].y
    r0.w = ((r0.wwww)*(source[4].yyyy)).w;
    // 13: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 14: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 15: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: movc r0.w, r1.x, l(0), |r0.w|
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.wwww))).w;
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
    // 28: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 29: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_o_pa_shine_01_tr: 42c49d5d2ba0d7438739d445ed28e5bf; selected map 3b705a5ce391e75ff80d59d8d33c9fe4af4d747356c35cb757724fd5c0f3dca7.
float4 LanceVANative631(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = g_LanceVASourceMaterialParameters[4u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].wwww,g_LanceVASourceMaterialParameters[3u].xxxx,1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[1u].xxxx,1u);
    source[5] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].wwww,g_LanceVASourceMaterialParameters[1u].yyyy,1u);
    source[8] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[10].w = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)).x;
    source[11].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[11].w = (LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].x = (LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 3: mad r0.xy, cb0[11].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[11].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 4: mad r1.x, cb0[4].x, r0.x, cb0[5].x
    r1.x = ((source[4].xxxx)*(r0.xxxx)+(source[5].xxxx)).x;
    // 5: mad r1.y, cb0[4].y, r0.y, cb0[6].y
    r1.y = ((source[4].yyyy)*(r0.yyyy)+(source[6].yyyy)).y;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
    r0.z = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mad r1.x, cb0[7].x, r0.x, cb0[8].x
    r1.x = ((source[7].xxxx)*(r0.xxxx)+(source[8].xxxx)).x;
    // 8: mad r1.y, cb0[7].y, r0.y, cb0[9].y
    r1.y = ((source[7].yyyy)*(r0.yyyy)+(source[9].yyyy)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    // 32: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 34: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 35: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 36: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 37: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 38: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 39: mad r0.xyz, r0.xxxx, r0.yzwy, v3.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(v3.xyzx)).xyz;
    // 40: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_j_swipemirr_01_ad: c59c57d1f4b8df42ab03e74cd9f4c57c; selected map feb0c08a00f248186a97db5dfeced62214e361814219ce9837bdb3d3f37566ab.
float4 LanceVANative632(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[0u];
    source[2] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[3] = LanceVANativeAppend(LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.200000003, 0.0, 0.0, 0.0))),LanceVANativePeriodic((g_LanceVASourceMaterialTime.xxxx*float4(-0.300000012, 0.0, 0.0, 0.0))),1u);
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.350000, -0.500000, -0.450000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.350000,-0.500000,-0.450000,-0.500000))).xyzw;
    // 2: mul r1.xy, r0.yyyy, l(2.500000, -2.500000, 0.000000, 0.000000)
    r1.xy = ((r0.yyyy)*(float4(2.500000,-2.500000,0.000000,0.000000))).xy;
    // 3: mov_sat r1.yz, r1.xxyx
    r1.yz = (saturate(r1.xxyx)).yz;
    // 4: add r1.y, r1.z, -r1.y
    r1.y = ((r1.zzzz)+(-(r1.yyyy))).y;
    // 5: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 6: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 7: mul r1.z, r1.z, l(0.400000)
    r1.z = ((r1.zzzz)*(float4(0.400000,0.400000,0.400000,0.400000))).z;
    // 8: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 9: movc r2.y, r1.y, l(0), r1.z
    r2.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 10: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 11: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 12: sqrt r0.xy, r0.xyxx
    r0.xy = (sqrt(r0.xyxx)).xy;
    // 13: mad r0.y, -r0.y, l(2.500000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.500000,2.500000,2.500000,2.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: mul_sat r0.y, r0.y, l(1.111111)
    r0.y = (saturate((r0.yyyy)*(float4(1.111111,1.111111,1.111111,1.111111)))).y;
    // 15: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 16: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 17: mad r0.x, -r0.x, l(1.666667), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(1.666667,1.666667,1.666667,1.666667))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 18: mul_sat r0.x, r0.x, l(1.111111)
    r0.x = (saturate((r0.xxxx)*(float4(1.111111,1.111111,1.111111,1.111111)))).x;
    // 19: mad r2.x, r0.x, l(0.850000), v2.x
    r2.x = ((r0.xxxx)*(float4(0.850000,0.850000,0.850000,0.850000))+(v2.xxxx)).x;
    // 20: mad r0.xz, r2.xxyx, l(1.000000, 0.000000, 0.700000, 0.000000), cb0[2].xxyx
    r0.xz = ((r2.xxyx)*(float4(1.000000,0.000000,0.700000,0.000000))+(source[2].xxyx)).xz;
    // 21: mad r1.xy, r2.xyxx, l(1.000000, 0.700000, 0.000000, 0.000000), cb0[3].xyxx
    r1.xy = ((r2.xyxx)*(float4(1.000000,0.700000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xzxx, t1.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 24: add r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)+(r0.xxxx)).x;
    // 25: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 26: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 27: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 28: mul r0.x, |r0.x|, |r0.x|
    r0.x = ((abs(r0.xxxx))*(abs(r0.xxxx))).x;
    // 29: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 31: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 32: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 33: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 34: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 35: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 36: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_e_pa_ht_01_1_tr: b03cd6e82a0c7c4f9c13d5c0bf2467a0; selected map 4e0d2751353057f17c700ccc9bccdf4f3970128651458a41480588e25fcfe33b.
float4 LanceVANative633(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[3].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 10: add r0.y, -cb0[3].z, l(1.000000)
    r0.y = ((-(source[3].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    r1.xyzw = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 18: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 19: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
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
    // 36: mul r1.xyz, r0.xyzx, cb0[2].wwww
    r1.xyz = ((r0.xyzx)*(source[2].wwww)).xyz;
    // 37: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 38: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 39: mul r1.xyz, r1.xyzx, cb0[3].xxxx
    r1.xyz = ((r1.xyzx)*(source[3].xxxx)).xyz;
    // 40: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 41: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 42: add r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)+(r0.wwww)).xyz;
    // 43: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 44: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_pa_skull_02_23_tr: 456fc57bd455014b93b97e18c9390a4f; selected map 45f9f31688792848e09af99f5a64410bef1bac4520e1f2c9707f5c31e5b7fdc4.
float4 LanceVANative634(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[8u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[7u].yyyy,g_LanceVASourceMaterialParameters[7u].zzzz,1u);
    source[3] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))),1u);
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].yyyy,g_LanceVASourceMaterialParameters[4u].zzzz,1u);
    source[6] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(cos(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = LanceVANativeAppend(sin(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[0u].wwww,1u);
    source[10].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[10].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[10].z = ((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)).x;
    source[10].w = (((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(-0.200000003, 0.0, 0.0, 0.0))).x;
    source[11].x = (((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[11].y = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].z = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[12].x = (((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[12].y = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[12].z = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[12].w = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.400000006, 0.0, 0.0, 0.0)))).x;
    source[13].x = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(0.0500000007, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].w = ((g_LanceVASourceMaterialParameters[4u].xxxx*float4(3.1400001, 0.0, 0.0, 0.0))).x;
    source[14].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[14].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[14].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[15].y = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[15].z = (LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[3u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[15].w = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[16].x = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[16].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[16].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[17].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[1u].xxxx))).x;
    source[17].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[17].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[17].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[18].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[18].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[18].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[19].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[19].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[19].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[19].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 1: mad r0.xy, v2.xyxx, cb0[11].yzyy, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[11].yzyy)+(source[2].xyxx)).xy;
    // 2: add r0.xy, r0.xyxx, cb0[3].xyxx
    r0.xy = ((r0.xyxx)+(source[3].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, v2.xxyx, cb0[12].yyzy, cb0[4].xxyx
    r0.yz = ((v2.xxyx)*(source[12].yyzy)+(source[4].xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, cb0[13].y
    r0.x = ((r0.xxxx)*(source[13].yyyy)).x;
    // 8: mad r0.yz, v2.xxyx, cb0[14].xxyx, cb0[5].xxyx
    r0.yz = ((v2.xxyx)*(source[14].xxyx)+(source[5].xxyx)).yz;
    // 9: mad r1.xy, cb0[15].xxxx, r0.xxxx, r0.yzyy
    r1.xy = ((source[15].xxxx)*(r0.xxxx)+(r0.yzyy)).xy;
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
    r0.w = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 16: mul r1.x, r0.x, cb0[15].x
    r1.x = ((r0.xxxx)*(source[15].xxxx)).x;
    // 17: mad r0.yz, r1.xxxx, cb0[15].wwww, r0.yyzy
    r0.yz = ((r1.xxxx)*(source[15].wwww)+(r0.yyzy)).yz;
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
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    // 38: mad r0.x, -r0.x, cb0[17].x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[17].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mul_sat r0.x, r0.x, cb0[18].x
    r0.x = (saturate((r0.xxxx)*(source[18].xxxx))).x;
    // 40: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 41: mul_sat r0.x, r0.x, cb0[18].y
    r0.x = (saturate((r0.xxxx)*(source[18].yyyy))).x;
    // 42: mad r0.x, r0.y, cb0[16].w, r0.x
    r0.x = ((r0.yyyy)*(source[16].wwww)+(r0.xxxx)).x;
    // 43: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 44: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 45: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 47: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 48: mul r0.y, r0.y, cb0[16].y
    r0.y = ((r0.yyyy)*(source[16].yyyy)).y;
    // 49: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 50: mad r0.y, r0.w, cb0[16].z, r0.y
    r0.y = ((r0.wwww)*(source[16].zzzz)+(r0.yyyy)).y;
    // 51: mad r0.yzw, r0.yyyy, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 52: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 53: mul r0.y, v2.x, cb0[18].w
    r0.y = ((v2.xxxx)*(source[18].wwww)).y;
    // 54: mad r1.x, cb0[10].x, cb0[18].z, r0.y
    r1.x = ((source[10].xxxx)*(source[18].zzzz)+(r0.yyyy)).x;
    // 55: mul r0.y, v2.y, cb0[19].x
    r0.y = ((v2.yyyy)*(source[19].xxxx)).y;
    // 56: mad r1.y, cb0[10].x, cb0[19].y, r0.y
    r1.y = ((source[10].xxxx)*(source[19].yyyy)+(r0.yyyy)).y;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t2.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 58: add r0.z, v4.x, cb0[19].z
    r0.z = ((v4.xxxx)+(source[19].zzzz)).z;
    // 59: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 60: mul_sat r0.y, r0.y, cb0[19].w
    r0.y = (saturate((r0.yyyy)*(source[19].wwww))).y;
    // 61: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 62: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_pa_turbulence_01_tr: f2ecc1c1e40aa34d879ceeabe82dfc21; selected map c345cefde3ac6ecbf5923057310970d247e2eee9b09817d3af90d16e36f482f6.
float4 LanceVANative635(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[4u];
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[4].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.xy = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r1.xyzw, -r0.xyxy, v2.xyxy
    r1.xyzw = ((-(r0.xyxy))+(v2.xyxy)).xyzw;
    // 3: mad r0.xyzw, v4.xxyy, r1.xyzw, r0.xyxy
    r0.xyzw = ((v4.xxyy)*(r1.xyzw)+(r0.xyxy)).xyzw;
    // 4: mul r1.xy, r0.xyxx, cb0[5].yyyy
    r1.xy = ((r0.xyxx)*(source[5].yyyy)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r1.x = (LanceVANativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
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
    r0.w = (LanceVANativeSample2((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_j_pa_smokes_1_tr: a83e5c536b16af489329afa9f08e8de4; selected map 9ab44cf37664f698acb3a1e6470e7836b159cd3c89dd93e97df3816c02760646.
float4 LanceVANative636(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[0u];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 15: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 16: mul_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)*(r0.yyyy))).x;
    // 17: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 18: mul r0.xy, v2.xyxx, l(6.000000, 6.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(6.000000,6.000000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 20: mul r0.xyz, r0.xxxx, v3.xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_me_master_01_018_ad: c9cf79250d787c4b8faa16cdb042f143; selected map e8bae060e5bc8394b3e0755ed4d89824b43afc42dcdbf4d111327e2960a7236f.
float4 LanceVANative637(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[6u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].yyyy,g_LanceVASourceMaterialParameters[3u].zzzz,1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[5].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].wwww)).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[8].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[9].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[9].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].wwww)).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[10].x = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[10].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
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
    r0.yz = (LanceVANativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
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
    r0.x = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 20: mul r1.xy, r0.yzyy, cb0[8].yzyy
    r1.xy = ((r0.yzyy)*(source[8].yzyy)).xy;
    // 21: mad r1.xy, r0.wwww, cb0[8].xwxx, r1.xyxx
    r1.xy = ((r0.wwww)*(source[8].xwxx)+(r1.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.zxyw, s2, l(0.000000)
    r1.x = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
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
    r0.y = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_o_pa_circledisort_01_01_ad: 36a0459be7616a4bacc60514830f1037; selected map 2449918b9aefcd38dfba19a96c9200cd10fb6ac36a0aaaf38df7a2ecaaa60233.
float4 LanceVANative638(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[0u];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    // 1: mad r0.x, -v4.y, l(0.500000), l(1.000000)
    r0.x = ((-(v4.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: max r0.x, r0.x, l(0.000010)
    r0.x = (max(r0.xxxx,float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 3: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.x
    r0.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.xxxx)).x;
    // 4: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 5: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 6: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 7: mad r0.z, -r0.y, l(2.000000), l(1.000000)
    r0.z = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: mad r0.y, -r0.y, l(1.923077), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(1.923077,1.923077,1.923077,1.923077))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 9: mul_sat r0.y, r0.y, l(1.428571)
    r0.y = (saturate((r0.yyyy)*(float4(1.428571,1.428571,1.428571,1.428571)))).y;
    // 10: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 11: add_sat r0.z, r0.z, r0.z
    r0.z = (saturate((r0.zzzz)+(r0.zzzz))).z;
    // 12: add r0.xy, -r0.xyxx, r0.zzzz
    r0.xy = ((-(r0.xyxx))+(r0.zzzz)).xy;
    // 13: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: mad_sat r0.x, r0.y, l(0.300000), r0.x
    r0.x = (saturate((r0.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.xxxx))).x;
    // 20: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 21: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 22: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 23: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 24: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_k_pa_makeflow_02_07_tr: 6681761729fba24896145fb070705f70; selected map fdfa3146587d6e058328ddecec678833d5f6bda76940ba9f8bd2a35254a53ed4.
float4 LanceVANative639(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[7u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].yyyy),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[3u].zzzz),1u);
    source[4] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy),1u);
    source[5] = LanceVANativeAppend((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy),1u);
    source[6] = g_LanceVASourceMaterialParameters[6u];
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].yyyy,g_LanceVASourceMaterialParameters[5u].zzzz,1u);
    source[8] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[10].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[11].y = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[11].z = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[11].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[12].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[12].w = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].yyyy)).x;
    source[13].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[13].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[14].x = (cos((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
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
    r0.w = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 34: mov r3.y, cb0[10].w
    r3.y = (source[10].wwww).y;
    // 35: mov r3.xz, l(0,0,0,0)
    r3.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 36: add r4.xyzw, r0.yzyz, r3.yxxy
    r4.xyzw = ((r0.yzyz)+(r3.yxxy)).xyzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r4.xyxx, t1.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r4.zwzz, t1.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r2.xyz = (LanceVANativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r3.xyz = (LanceVANativeSample2((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.y = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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
    r0.z = (LanceVANativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
