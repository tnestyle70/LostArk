// Single source owner for LanceVANative profiles 640..703.
#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_atta_05_14_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 LanceVANative640(LANCE_VA_NATIVE_INPUT input)
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
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_me_watertrail_01_46_tr: 0c986668f7b6b8438ad4e59533ac0d95; selected map 04f23ee5f526fb7f1b8f245b449c842f3c96ad9cc0aef03ea1bde8686efad069.
float4 LanceVANative641(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].zzzz,g_LanceVASourceMaterialParameters[3u].wwww,1u);
    source[7].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[11].x = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[11].y = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[14].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[14].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
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
    r1.yz = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 19: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 20: add r1.xy, cb0[3].xzxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[3].xzxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mad r2.x, r1.x, cb0[7].x, r0.z
    r2.x = ((r1.xxxx)*(source[7].xxxx)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[12].y, r0.w
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r2.xyxx, t2.xwyz, s2, l(0.000000)
    r1.xzw = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_s_pa_flow_01_08_tr: d5264a6e7ea1394690402346afbc0de6; selected map 7f81ef1d8d362e88193717f346d5f3bc9143e81b03f25b065f212b6f5346b98d.
float4 LanceVANative642(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[8u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[0u].wwww,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].yyyy,g_LanceVASourceMaterialParameters[2u].zzzz,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[5u].zzzz),1u);
    source[5] = g_LanceVASourceMaterialParameters[6u];
    source[6] = g_LanceVASourceMaterialParameters[7u];
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].wwww,g_LanceVASourceMaterialParameters[5u].xxxx,1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].yyyy,g_LanceVASourceMaterialParameters[3u].zzzz,1u);
    source[9].x = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[10].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[5u].zzzz)).x;
    source[10].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[10].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[12].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[12].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[13].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
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
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mov r1.w, cb0[9].w
    r1.w = (source[9].wwww).w;
    // 33: mov r1.xz, l(0,0,0,0)
    r1.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 34: mad r2.xyzw, r0.xyxy, cb0[3].xyxy, r1.wzzw
    r2.xyzw = ((r0.xyxy)*(source[3].xyxy)+(r1.wzzw)).xyzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s0, l(0.000000)
    r1.z = (LanceVANativeSample0((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.x = (LanceVANativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t3.wxyz, s2, l(0.000000)
    r1.yzw = (LanceVANativeSample2((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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
    r0.w = (sin(r0.wwww)).w;
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
    r1.x = (LanceVANativeSample3((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_o_me_makeflow_02_22_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 LanceVANative643(LANCE_VA_NATIVE_INPUT input)
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
// fx_b_pa_cd_02_tr: 1e3a9f3a0de95641b964f0aaf35e4b5d; selected map b433fb5c4ba29bf90206e515a49916cf6562ca427a6fdc075113e28bb5b8b421.
float4 LanceVANative644(LANCE_VA_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.xy, v4.yyyy, l(0.050000, 0.100000, 0.000000, 0.000000)
    r0.xy = ((v4.yyyy)*(float4(0.050000,0.100000,0.000000,0.000000))).xy;
    // 2: mad r0.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, v4.yyyy, l(0.000000, -0.070000, -0.030000, 0.000000), v2.xxyx
    r0.yz = ((v4.yyyy)*(float4(0.000000,-0.070000,-0.030000,0.000000))+(v2.xxyx)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.xyzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 6: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 7: mad r0.xy, r0.xxxx, l(0.150000, 0.150000, 0.000000, 0.000000), v2.xyxx
    r0.xy = ((r0.xxxx)*(float4(0.150000,0.150000,0.000000,0.000000))+(v2.xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t2.yzxw, s4, l(0.000000)
    r0.z = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: add r0.y, -r0.x, r0.z
    r0.y = ((-(r0.xxxx))+(r0.zzzz)).y;
    // 11: mad r0.x, v4.x, r0.y, r0.x
    r0.x = ((v4.xxxx)*(r0.yyyy)+(r0.xxxx)).x;
    // 12: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 13: mul r0.x, r0.x, l(2.500000)
    r0.x = ((r0.xxxx)*(float4(2.500000,2.500000,2.500000,2.500000))).x;
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
    // 23: mul_sat r0.y, r0.y, l(0.100000)
    r0.y = (saturate((r0.yyyy)*(float4(0.100000,0.100000,0.100000,0.100000)))).y;
    // 24: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 25: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 27: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 28: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_m_me_floorstrm_1_ad: 00cd866295cc494dba563a698dea14df; selected map 25dad5f905785b3aa0b8fbe349a5a17d05495ab568518506e1e61c8b6a29d983.
float4 LanceVANative645(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[6].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[9].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[10].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].xxxx)).x;
    source[10].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[10].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[7].zwzz
    r0.xy = ((v4.xyxx)*(source[7].zwzz)).xy;
    // 2: mul r0.z, cb0[6].x, cb0[6].y
    r0.z = ((source[6].xxxx)*(source[6].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[7].y, r0.x
    r1.x = ((r0.zzzz)*(source[7].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[8].x, r0.y
    r1.y = ((r0.zzzz)*(source[8].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
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
    r1.xyz = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_atta_10_06_ad: 5302b738e52f7244969833048f8196db; selected map e475066cd144e8ad9b41e407d3331eac30174ee17ccf8cd7ad491e7577052930.
float4 LanceVANative646(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[2].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[2].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[2].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[3].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[3].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[3].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
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
// fx_d_pa_afterburn_01_37_dt_tr: 461f29f977b1c241bca533ae89e12229; selected map c4a929c9743b7944f0da5d3ecee3fa714e385c37e49b60b2f61e9740403a64cb.
float4 LanceVANative647(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3] = g_LanceVASourceMaterialParameters[4u];
    source[4] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(cos(((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[7] = LanceVANativeAppend(sin(((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[8] = LanceVANativeAppend(cos((((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[9] = LanceVANativeAppend(sin((((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos((((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (cos((g_LanceVASourceMaterialParameters[2u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[10].w = ((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)).x;
    source[11].x = (cos((((g_LanceVASourceMaterialParameters[2u].yyyy+g_LanceVASourceMaterialParameters[2u].zzzz)+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[11].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[12].y = ((g_LanceVASourceMaterialParameters[1u].zzzz*float4(3.0, 0.0, 0.0, 0.0))).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[12].w = ((g_LanceVASourceMaterialParameters[1u].wwww*float4(20.0, 0.0, 0.0, 0.0))).x;
    source[13].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[13].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[14].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy))).x;
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
    // 1: mul r0.x, v4.w, cb0[10].y
    r0.x = ((v4.wwww)*(source[10].yyyy)).x;
    // 2: mad r0.yz, r0.xxxx, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.xxxx)*(v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: mul r1.xyzw, r0.xxxx, l(1.330000, 1.330000, 1.768900, 1.768900)
    r1.xyzw = ((r0.xxxx)*(float4(1.330000,1.330000,1.768900,1.768900))).xyzw;
    // 4: mad r1.xyzw, r1.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r1.xyzw = ((r1.xyzw)*(v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 5: dp2 r0.x, cb0[5].xyxx, r0.yzyy
    r0.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 6: dp2 r2.x, cb0[4].xyxx, r0.yzyy
    r2.x = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 7: mad r2.y, v4.x, l(0.020000), r0.x
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.xxxx)).y;
    // 8: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 10: dp2 r0.y, cb0[7].xyxx, r1.xyxx
    r0.y = (dot((source[7].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 11: mad r2.y, v4.x, l(0.020000), r0.y
    r2.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 12: dp2 r2.x, cb0[6].xyxx, r1.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 13: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, l(0.333300)
    r0.y = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))).y;
    // 16: mad r0.x, r0.x, l(0.333300), r0.y
    r0.x = ((r0.xxxx)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.yyyy)).x;
    // 17: dp2 r0.y, cb0[9].xyxx, r1.zwzz
    r0.y = (dot((source[9].xyxx).xy,(r1.zwzz).xy).xxxx).y;
    // 18: dp2 r1.x, cb0[8].xyxx, r1.zwzz
    r1.x = (dot((source[8].xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 19: mad r1.y, v4.x, l(0.020000), r0.y
    r1.y = ((v4.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))+(r0.yyyy)).y;
    // 20: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 22: mad r0.x, r0.y, l(0.333300), r0.x
    r0.x = ((r0.yyyy)*(float4(0.333300,0.333300,0.333300,0.333300))+(r0.xxxx)).x;
    // 23: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 24: mul r0.y, r0.y, cb0[12].y
    r0.y = ((r0.yyyy)*(source[12].yyyy)).y;
    // 25: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 26: mul r0.y, r0.y, cb0[12].w
    r0.y = ((r0.yyyy)*(source[12].wwww)).y;
    // 27: mov_sat r0.z, v4.z
    r0.z = (saturate(v4.zzzz)).z;
    // 28: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 30: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 31: movc r0.y, r0.z, l(-0.000000), -r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 32: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 33: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 34: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 35: mad r0.z, -r0.z, l(1.428571), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(1.428571,1.428571,1.428571,1.428571))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 36: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 37: add r0.w, r0.z, r0.x
    r0.w = ((r0.zzzz)+(r0.xxxx)).w;
    // 38: mul_sat r0.x, r0.x, cb0[11].y
    r0.x = (saturate((r0.xxxx)*(source[11].yyyy))).x;
    // 39: add r0.w, r0.w, -cb0[11].w
    r0.w = ((r0.wwww)+(-(source[11].wwww))).w;
    // 40: mad r0.y, r0.w, r0.z, r0.y
    r0.y = ((r0.wwww)*(r0.zzzz)+(r0.yyyy)).y;
    // 41: mul_sat r0.y, r0.y, cb0[13].x
    r0.y = (saturate((r0.yyyy)*(source[13].xxxx))).y;
    // 42: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 43: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 44: mul r0.z, r0.z, cb0[13].y
    r0.z = ((r0.zzzz)*(source[13].yyyy)).z;
    // 45: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 46: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 47: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 48: source device depth mapped to centimetre view depth; reconstruction at 50.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 50-53: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 54: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 55: add r1.x, -cb0[14].x, l(1.000000)
    r1.x = ((-(source[14].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 56: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 57: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 58: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 59: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 60: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 61: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 62: movc o0.w, r0.y, l(0), r0.z
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 63: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 64: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 65: mul r0.y, r0.y, cb0[11].z
    r0.y = ((r0.yyyy)*(source[11].zzzz)).y;
    // 66: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 67: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 68: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 69: mad r1.xyz, cb0[3].wwww, cb0[3].xyzx, -r0.yzwy
    r1.xyz = ((source[3].wwww)*(source[3].xyzx)+(-(r0.yzwy))).xyz;
    // 70: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 71: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_s_pa_windtrail_01_1_ts_tr: 13eb0c448d109c44b667c502c4e3ac5f; selected map 2f122762b41a7ef111d8a7ffad58d067a2bd4f1efd0d1eea9c67cd00c2d111aa.
float4 LanceVANative648(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[3u].zzzz,1u);
    source[5] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].yyyy,g_LanceVASourceMaterialParameters[1u].xxxx,1u);
    source[8].x = (cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[8].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[8].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[9].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[9].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[9].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[9].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[10].x = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[10].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[10].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[10].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[12].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[12].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[13].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[13].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
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
    // 1: mul r0.x, v4.x, cb0[11].w
    r0.x = ((v4.xxxx)*(source[11].wwww)).x;
    // 2: mul r0.y, v4.y, cb0[12].x
    r0.y = ((v4.yyyy)*(source[12].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)+(source[7].xyxx)).xy;
    // 4: mul r0.zw, cb0[3].zzzz, l(0.000000, 0.000000, 0.080000, -0.100000)
    r0.zw = ((source[3].zzzz)*(float4(0.000000,0.000000,0.080000,-0.100000))).zw;
    // 5: mad r0.zw, v4.xxxy, cb0[9].yyyz, r0.zzzw
    r0.zw = ((v4.xxxy)*(source[9].yyyz)+(r0.zzzw)).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 7: mov r1.xz, r0.zzzz
    r1.xz = (r0.zzzz).xz;
    // 8: mul r2.xyzw, v4.xyxy, cb0[10].xyzw
    r2.xyzw = ((v4.xyxy)*(source[10].xyzw)).xyzw;
    // 9: mad r0.zw, cb0[3].zzzz, l(0.000000, 0.000000, -0.050000, -0.050000), r2.xxxy
    r0.zw = ((source[3].zzzz)*(float4(0.000000,0.000000,-0.050000,-0.050000))+(r2.xxxy)).zw;
    // 10: mad r2.xy, cb0[3].xxxx, l(0.080000, 3.000000, 0.000000, 0.000000), r2.zwzz
    r2.xy = ((source[3].xxxx)*(float4(0.080000,3.000000,0.000000,0.000000))+(r2.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.x = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 12: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.xzyw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).z;
    // 14: mov r1.yw, r0.zzzz
    r1.yw = (r0.zzzz).yw;
    // 15: mad r0.z, v4.y, cb0[9].z, cb0[9].w
    r0.z = ((v4.yyyy)*(source[9].zzzz)+(source[9].wwww)).z;
    // 16: mul r1.xyzw, r1.xyzw, r0.zzzz
    r1.xyzw = ((r1.xyzw)*(r0.zzzz)).xyzw;
    // 17: mul r1.xyzw, r1.xyzw, cb0[3].yyyy
    r1.xyzw = ((r1.xyzw)*(source[3].yyyy)).xyzw;
    // 18: mad r0.xy, r1.xyxx, l(0.400000, 0.400000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.xyxx)*(float4(0.400000,0.400000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 19: add r1.xy, cb0[3].xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[3].xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 20: mad r0.z, r1.x, l(0.300000), r0.y
    r0.z = ((r1.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy)).z;
    // 21: add r0.xy, r0.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 22: dp2 r3.x, cb0[5].xyxx, r0.xyxx
    r3.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: dp2 r3.y, cb0[6].xyxx, r0.xyxx
    r3.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 24: add r0.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 26: mul_sat r0.x, r0.x, cb0[12].w
    r0.x = (saturate((r0.xxxx)*(source[12].wwww))).x;
    // 27: mul r0.yz, v4.xxyx, cb0[13].zzwz
    r0.yz = ((v4.xxyx)*(source[13].zzwz)).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 29: add r0.z, -v4.y, l(1.000000)
    r0.z = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: mad r0.y, r0.z, cb0[13].y, r0.y
    r0.y = ((r0.zzzz)*(source[13].yyyy)+(r0.yyyy)).y;
    // 31: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 32: mul_sat r0.z, r0.z, cb0[13].x
    r0.z = (saturate((r0.zzzz)*(source[13].xxxx))).z;
    // 33: add r0.y, -r1.y, r0.y
    r0.y = ((-(r1.yyyy))+(r0.yyyy)).y;
    // 34: mul r0.w, r1.x, cb0[8].y
    r0.w = ((r1.xxxx)*(source[8].yyyy)).w;
    // 35: mul_sat r0.y, r0.y, cb0[14].x
    r0.y = (saturate((r0.yyyy)*(source[14].xxxx))).y;
    // 36: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 37: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 38: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 39: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 40: mad r0.xy, v4.xyxx, cb0[8].zyzz, cb0[4].xyxx
    r0.xy = ((v4.xyxx)*(source[8].zyzz)+(source[4].xyxx)).xy;
    // 41: mad r0.xy, r1.zwzz, l(0.600000, 0.600000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.zwzz)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 42: mad r0.z, r0.w, l(0.300000), r0.y
    r0.z = ((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy)).z;
    // 43: add r0.xy, r0.xzxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 44: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 45: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 46: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: mul r0.y, r2.x, r0.x
    r0.y = ((r2.xxxx)*(r0.xxxx)).y;
    // 49: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 50: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 51: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 52: mul r0.y, r0.y, cb0[11].x
    r0.y = ((r0.yyyy)*(source[11].xxxx)).y;
    // 53: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 54: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 55: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 56: mad r0.x, r0.x, cb0[11].z, r0.y
    r0.x = ((r0.xxxx)*(source[11].zzzz)+(r0.yyyy)).x;
    // 57: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 58: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_ribbonflow_02_107_ad: a4f518d2180ca24996d08b4a0e1d5d32; selected map b4e8ac243545216c4516b53b07981fee4e532d07f5efb5af6579b404621b8734.
float4 LanceVANative649(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[6u];
    source[2] = g_LanceVASourceMaterialParameters[5u];
    source[3] = LanceVANativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = LanceVANativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].yyyy,g_LanceVASourceMaterialParameters[3u].zzzz,1u);
    source[6].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[7].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[8].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[9].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[9].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[10].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
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
    // 1: mul r0.x, cb0[6].y, cb0[8].z
    r0.x = ((source[6].yyyy)*(source[8].zzzz)).x;
    // 2: mul r0.yz, v2.xxyx, cb0[7].yyzy
    r0.yz = ((v2.xxyx)*(source[7].yyzy)).yz;
    // 3: mad r0.yz, cb0[6].yyyy, cb0[7].xxwx, r0.yyzy
    r0.yz = ((source[6].yyyy)*(source[7].xxwx)+(r0.yyzy)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t1.zxyw, s0, l(0.000000)
    r0.yz = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 5: mad r0.yz, cb0[8].xxxx, r0.yyzy, v2.xxyx
    r0.yz = ((source[8].xxxx)*(r0.yyzy)+(v2.xxyx)).yz;
    // 6: mad r1.x, cb0[8].w, r0.y, r0.x
    r1.x = ((source[8].wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 7: mul r0.x, r0.z, cb0[9].x
    r0.x = ((r0.zzzz)*(source[9].xxxx)).x;
    // 8: mul r0.yz, r0.yyzy, cb0[6].zzwz
    r0.yz = ((r0.yyzy)*(source[6].zzwz)).yz;
    // 9: mad r1.y, cb0[6].y, cb0[9].y, r0.x
    r1.y = ((source[6].yyyy)*(source[9].yyyy)+(r0.xxxx)).y;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: mad r0.x, cb0[6].y, cb0[6].x, r0.y
    r0.x = ((source[6].yyyy)*(source[6].xxxx)+(r0.yyyy)).x;
    // 12: mad r0.y, cb0[6].y, cb0[8].y, r0.z
    r0.y = ((source[6].yyyy)*(source[8].yyyy)+(r0.zzzz)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 15: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 16: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 17: mad r0.xyz, cb0[9].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 18: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 19: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 20: mul r0.xyz, r0.xyzx, cb0[9].wwww
    r0.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 21: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 22: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 23: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 26: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 27: dp2 r2.x, cb0[3].xyxx, r1.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 28: dp2 r2.y, cb0[4].xyxx, r1.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 29: add r0.w, r1.y, r1.y
    r0.w = ((r1.yyyy)+(r1.yyyy)).w;
    // 30: mad r0.w, -|r0.w|, |r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))*(abs(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: mad r1.xy, r2.xyxx, cb0[5].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(source[5].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t0.xyzw, s3, l(0.000000)
    r1.xyz = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 33: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 34: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 35: mul r1.x, r1.x, l(0.333330)
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 36: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 37: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 38: mul r1.y, r1.y, cb0[10].x
    r1.y = ((r1.yyyy)*(source[10].xxxx)).y;
    // 39: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 40: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 41: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 42: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 43: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 44: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 45: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_me_master_01_010_ts_ad: f262a6c09f212d4b80a156c9f4f8a57c; selected map fa582e54b5c807862f5e60ba55b4dfe8ed1ef42d313d401635529bd4d1dfcda4.
float4 LanceVANative650(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[5] = g_LanceVASourceMaterialParameters[5u];
    source[6].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[7].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[8].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].xxxx)).x;
    source[9].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[10].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
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
    // 1: add r0.x, cb0[3].y, l(-1.000000)
    r0.x = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[6].y, cb0[8].w, cb0[9].x
    r0.y = ((source[6].yyyy)*(source[8].wwww)+(source[9].xxxx)).y;
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: mul r0.w, v4.x, cb0[6].w
    r0.w = ((v4.xxxx)*(source[6].wwww)).w;
    // 15: mul r1.x, cb0[6].x, cb0[6].y
    r1.x = ((source[6].xxxx)*(source[6].yyyy)).x;
    // 16: mad r2.x, r1.x, cb0[6].z, r0.w
    r2.x = ((r1.xxxx)*(source[6].zzzz)+(r0.wwww)).x;
    // 17: mul r1.yz, v4.yyxy, cb0[7].xxwx
    r1.yz = ((v4.yyxy)*(source[7].xxwx)).yz;
    // 18: mad r2.yz, r1.xxxx, cb0[7].yyzy, r1.yyzy
    r2.yz = ((r1.xxxx)*(source[7].yyzy)+(r1.yyzy)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r2.xyxx, t0.wxyz, s0, l(0.000000)
    r1.yzw = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 20: mul r0.w, v4.y, cb0[8].x
    r0.w = ((v4.yyyy)*(source[8].xxxx)).w;
    // 21: mad r2.w, r1.x, cb0[8].y, r0.w
    r2.w = ((r1.xxxx)*(source[8].yyyy)+(r0.wwww)).w;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.zwzz, t1.xyzw, s1, l(0.000000)
    r2.xyz = (LanceVANativeSample1((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 23: mul r1.xyz, r1.yzwy, r2.xyzx
    r1.xyz = ((r1.yzwy)*(r2.xyzx)).xyz;
    // 24: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 27: mad r0.xyz, cb0[9].wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((source[9].wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 28: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 29: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 30: mul r0.xyz, r0.xyzx, cb0[10].xxxx
    r0.xyz = ((r0.xyzx)*(source[10].xxxx)).xyz;
    // 31: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 32: mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 33: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 34: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 35: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 36: add r0.w, -cb0[3].x, l(1.000000)
    r0.w = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 38: mul_sat r0.w, r0.w, cb0[10].y
    r0.w = (saturate((r0.wwww)*(source[10].yyyy))).w;
    // 39: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 40: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 41: mul r1.x, r1.x, cb0[10].z
    r1.x = ((r1.xxxx)*(source[10].zzzz)).x;
    // 42: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 43: mul_sat r1.x, r1.x, cb0[1].w
    r1.x = (saturate((r1.xxxx)*(source[1].wwww))).x;
    // 44: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 45: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 46: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 47: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bfx_j_pa_lightdust_01_1_tr: 8a5cc23116d92a4d894fa89a15d5c64f; selected map 6575f98d15daef884390d85ac13cadd021cdff94e5225e70729808a38c568908.
float4 LanceVANative651(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].xxxx,g_LanceVASourceMaterialParameters[1u].yyyy,1u);
    source[3].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
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
    // 1: mov r0.xz, l(0,0,0,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 2: mov r0.yw, v4.zzzx
    r0.yw = (v4.zzzx).yw;
    // 3: add r0.xyzw, r0.xyzw, v2.xyxy
    r0.xyzw = ((r0.xyzw)+(v2.xyxy)).xyzw;
    // 4: mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s1, l(0.000000)
    r0.z = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyzw = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_glow_01_05_dt_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 LanceVANative652(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
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
    source[4].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[5].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[1u].xxxx))).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_t_me_master_01_ph_01_msk: 3b18265f1649424d9c83b0a824f4033b; selected map b164df527a5a1eddf6ad5b0081025721c9060cdd7b808d68643f99398b30da93.
float4 LanceVANative653(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=input.color; // These native masked/local-VF variants place particle color and opacity in prefix row 0.
    source[1] = g_LanceVASourceMaterialParameters[11u];
    source[2] = LanceVANativeAppend(LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].wwww,g_LanceVASourceMaterialParameters[4u].wwww,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].xxxx,g_LanceVASourceMaterialParameters[2u].xxxx,1u);
    source[4] = g_LanceVASourceMaterialParameters[8u];
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].yyyy,g_LanceVASourceMaterialParameters[3u].wwww,1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].wwww,g_LanceVASourceMaterialParameters[1u].wwww,1u);
    source[8] = g_LanceVASourceMaterialParameters[9u];
    source[9] = g_LanceVASourceMaterialParameters[7u];
    source[10].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[13].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[13].z = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[15].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[15].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mov_sat r0.x, cb0[0].w
    r0.x = (saturate(source[0].wwww)).x;
    // 2: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: mul r0.yz, v4.xxyx, cb0[15].yyzy
    r0.yz = ((v4.xxyx)*(source[15].yyzy)).yz;
    // 4: mad r0.yz, cb0[10].yyyy, cb0[15].xxwx, r0.yyzy
    r0.yz = ((source[10].yyyy)*(source[15].xxwx)+(r0.yyzy)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s4, l(0.000000)
    r0.y = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: add_sat r0.x, -r0.x, r0.y
    r0.x = (saturate((-(r0.xxxx))+(r0.yyyy))).x;
    // 7: mul_sat r0.y, r0.y, cb0[16].x
    r0.y = (saturate((r0.yyyy)*(source[16].xxxx))).y;
    // 8: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 9: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 10: mul r0.z, r0.z, cb0[16].y
    r0.z = ((r0.zzzz)*(source[16].yyyy)).z;
    // 11: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 12: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 13: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 15: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 16: or r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.yyyy))).x;
    // 17: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 18: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 19: mul r0.xy, v4.xyxx, cb0[7].xyxx
    r0.xy = ((v4.xyxx)*(source[7].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 22: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 23: mad r0.xyz, cb0[13].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[13].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 24: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 26: mul r0.xyz, r0.xyzx, cb0[13].zzzz
    r0.xyz = ((r0.xyzx)*(source[13].zzzz)).xyz;
    // 27: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 28: mul r1.xy, v4.xyxx, cb0[10].zwzz
    r1.xy = ((v4.xyxx)*(source[10].zwzz)).xy;
    // 29: mad r2.x, cb0[10].y, cb0[10].x, r1.x
    r2.x = ((source[10].yyyy)*(source[10].xxxx)+(r1.xxxx)).x;
    // 30: mad r2.y, cb0[10].y, cb0[11].x, r1.y
    r2.y = ((source[10].yyyy)*(source[11].xxxx)+(r1.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 32: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 33: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 34: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 37: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 39: mul r2.xyz, cb0[8].xyzx, cb0[8].wwww
    r2.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 40: mad r3.xy, r0.xyxx, r2.xyxx, r1.xyxx
    r3.xy = ((r0.xyxx)*(r2.xyxx)+(r1.xyxx)).xy;
    // 41: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 42: mul r2.xy, r3.xyxx, cb0[13].wwww
    r2.xy = ((r3.xyxx)*(source[13].wwww)).xy;
    // 43: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 44: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 45: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 46: mad r2.xy, r3.xyxx, cb0[6].xyxx, r2.xyxx
    r2.xy = ((r3.xyxx)*(source[6].xyxx)+(r2.xyxx)).xy;
    // 47: add r0.w, -|r3.z|, l(1.000000)
    r0.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyz = (LanceVANativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 49: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 50: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 51: mad r2.xyz, cb0[14].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[14].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 52: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 53: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 54: mul r2.xyz, r2.xyzx, cb0[14].yyyy
    r2.xyz = ((r2.xyzx)*(source[14].yyyy)).xyz;
    // 55: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 56: mul r3.xyz, cb0[9].xyzx, cb0[9].wwww
    r3.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 57: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 58: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 59: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 60: mad r2.w, v1.z, r1.w, l(1.000000)
    r2.w = ((v1.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: mul r3.xyz, r1.wwww, v1.xyzx
    r3.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 62: mul_sat r1.w, r2.w, l(0.500000)
    r1.w = (saturate((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 63: mad r1.w, r1.w, l(0.950000), l(0.050000)
    r1.w = ((r1.wwww)*(float4(0.950000,0.950000,0.950000,0.950000))+(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 64: dp3 r1.w, r1.xyzx, r1.wwww
    r1.w = (dot((r1.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 65: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 66: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 67: mul r2.w, r2.w, cb0[14].z
    r2.w = ((r2.wwww)*(source[14].zzzz)).w;
    // 68: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 69: mul r2.w, r2.w, cb0[14].w
    r2.w = ((r2.wwww)*(source[14].wwww)).w;
    // 70: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 71: mad r0.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 72: mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 73: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 74: mad r2.xy, r2.xyxx, cb0[3].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(source[3].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t2.xzwy, s1, l(0.000000)
    r1.w = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzwy).w;
    // 76: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 77: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 78: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 79: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 83: mul r2.xyz, r2.xyzx, cb0[5].xxxx
    r2.xyz = ((r2.xyzx)*(source[5].xxxx)).xyz;
    // 84: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 85: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 86: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_t_me_master_01_ph_02_msk: 3b18265f1649424d9c83b0a824f4033b; selected map b164df527a5a1eddf6ad5b0081025721c9060cdd7b808d68643f99398b30da93.
float4 LanceVANative654(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[0]=input.color; // These native masked/local-VF variants place particle color and opacity in prefix row 0.
    source[1] = g_LanceVASourceMaterialParameters[11u];
    source[2] = LanceVANativeAppend(LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].wwww,g_LanceVASourceMaterialParameters[4u].wwww,1u),float4(1.0, 0.0, 0.0, 0.0),2u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].xxxx,g_LanceVASourceMaterialParameters[2u].xxxx,1u);
    source[4] = g_LanceVASourceMaterialParameters[8u];
    source[5] = input.dynamicParameter;
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].yyyy,g_LanceVASourceMaterialParameters[3u].wwww,1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].wwww,g_LanceVASourceMaterialParameters[1u].wwww,1u);
    source[8] = g_LanceVASourceMaterialParameters[9u];
    source[9] = g_LanceVASourceMaterialParameters[7u];
    source[10].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[10].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[12].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[13].y = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[13].z = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[14].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[14].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[15].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[15].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mov_sat r0.x, cb0[0].w
    r0.x = (saturate(source[0].wwww)).x;
    // 2: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 3: mul r0.yz, v4.xxyx, cb0[15].yyzy
    r0.yz = ((v4.xxyx)*(source[15].yyzy)).yz;
    // 4: mad r0.yz, cb0[10].yyyy, cb0[15].xxwx, r0.yyzy
    r0.yz = ((source[10].yyyy)*(source[15].xxwx)+(r0.yyzy)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s4, l(0.000000)
    r0.y = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 6: add_sat r0.x, -r0.x, r0.y
    r0.x = (saturate((-(r0.xxxx))+(r0.yyyy))).x;
    // 7: mul_sat r0.y, r0.y, cb0[16].x
    r0.y = (saturate((r0.yyyy)*(source[16].xxxx))).y;
    // 8: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 9: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 10: mul r0.z, r0.z, cb0[16].y
    r0.z = ((r0.zzzz)*(source[16].yyyy)).z;
    // 11: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 12: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 13: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: add r0.x, r0.x, l(-0.166000)
    r0.x = ((r0.xxxx)+(float4(-0.166000,-0.166000,-0.166000,-0.166000))).x;
    // 15: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 16: or r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.yyyy))).x;
    // 17: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 18: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 19: mul r0.xy, v4.xyxx, cb0[7].xyxx
    r0.xy = ((v4.xyxx)*(source[7].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 22: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 23: mad r0.xyz, cb0[13].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[13].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 24: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 26: mul r0.xyz, r0.xyzx, cb0[13].zzzz
    r0.xyz = ((r0.xyzx)*(source[13].zzzz)).xyz;
    // 27: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 28: mul r1.xy, v4.xyxx, cb0[10].zwzz
    r1.xy = ((v4.xyxx)*(source[10].zwzz)).xy;
    // 29: mad r2.x, cb0[10].y, cb0[10].x, r1.x
    r2.x = ((source[10].yyyy)*(source[10].xxxx)+(r1.xxxx)).x;
    // 30: mad r2.y, cb0[10].y, cb0[11].x, r1.y
    r2.y = ((source[10].yyyy)*(source[11].xxxx)+(r1.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 32: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 33: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 34: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 37: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 39: mul r2.xyz, cb0[8].xyzx, cb0[8].wwww
    r2.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 40: mad r3.xy, r0.xyxx, r2.xyxx, r1.xyxx
    r3.xy = ((r0.xyxx)*(r2.xyxx)+(r1.xyxx)).xy;
    // 41: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 42: mul r2.xy, r3.xyxx, cb0[13].wwww
    r2.xy = ((r3.xyxx)*(source[13].wwww)).xy;
    // 43: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 44: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 45: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 46: mad r2.xy, r3.xyxx, cb0[6].xyxx, r2.xyxx
    r2.xy = ((r3.xyxx)*(source[6].xyxx)+(r2.xyxx)).xy;
    // 47: add r0.w, -|r3.z|, l(1.000000)
    r0.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyz = (LanceVANativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 49: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 50: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 51: mad r2.xyz, cb0[14].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[14].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 52: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 53: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 54: mul r2.xyz, r2.xyzx, cb0[14].yyyy
    r2.xyz = ((r2.xyzx)*(source[14].yyyy)).xyz;
    // 55: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 56: mul r3.xyz, cb0[9].xyzx, cb0[9].wwww
    r3.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 57: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 58: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 59: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 60: mad r2.w, v1.z, r1.w, l(1.000000)
    r2.w = ((v1.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: mul r3.xyz, r1.wwww, v1.xyzx
    r3.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 62: mul_sat r1.w, r2.w, l(0.500000)
    r1.w = (saturate((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000)))).w;
    // 63: mad r1.w, r1.w, l(0.950000), l(0.050000)
    r1.w = ((r1.wwww)*(float4(0.950000,0.950000,0.950000,0.950000))+(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 64: dp3 r1.w, r1.xyzx, r1.wwww
    r1.w = (dot((r1.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 65: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 66: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 67: mul r2.w, r2.w, cb0[14].z
    r2.w = ((r2.wwww)*(source[14].zzzz)).w;
    // 68: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 69: mul r2.w, r2.w, cb0[14].w
    r2.w = ((r2.wwww)*(source[14].wwww)).w;
    // 70: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 71: mad r0.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 72: mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 73: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 74: mad r2.xy, r2.xyxx, cb0[3].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(source[3].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t2.xzwy, s1, l(0.000000)
    r1.w = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzwy).w;
    // 76: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 77: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 78: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 79: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 83: mul r2.xyz, r2.xyzx, cb0[5].xxxx
    r2.xyz = ((r2.xyzx)*(source[5].xxxx)).xyz;
    // 84: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 85: mad o0.xyz, r0.xyzx, cb0[0].xyzx, cb0[1].xyzx
    output.xyz = ((r0.xyzx)*(source[0].xyzx)+(source[1].xyzx)).xyz;
    // 86: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_a_pa_gl_01_6_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 LanceVANative655(LANCE_VA_NATIVE_INPUT input)
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
// fx_c_pa_lensflare_01_11_dt5_ad: 55883598a583ae4389d79bb0fdf1d392; selected map ae9ed651c317374306f77e3e999680cc10ae6be4f2b89b76263a55721100b6de.
float4 LanceVANative656(LANCE_VA_NATIVE_INPUT input)
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
// fx_o_pa_spritewave_01_29_tr: 896de46482584f40beae5d174ab6a67c; selected map 656abb016151e52a25ea0326da9bfc1b2fa6381531ff0286f6b5b2a2364259da.
float4 LanceVANative657(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[23]; [unroll] for (uint i=0u; i<23u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[12u];
    source[2] = LanceVANativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = LanceVANativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].zzzz,g_LanceVASourceMaterialParameters[4u].wwww,1u);
    source[5] = LanceVANativeAppend(cos(((g_LanceVASourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = LanceVANativeAppend(sin(((g_LanceVASourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_LanceVASourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = g_LanceVASourceMaterialParameters[11u];
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((g_LanceVASourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[10].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[11].x = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[11].y = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[11].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[12].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_LanceVASourceMaterialParameters[9u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[10u].yyyy).x;
    source[13].x = (g_LanceVASourceMaterialParameters[10u].zzzz).x;
    source[13].y = (g_LanceVASourceMaterialParameters[8u].wwww).x;
    source[13].z = (g_LanceVASourceMaterialParameters[9u].wwww).x;
    source[13].w = (g_LanceVASourceMaterialParameters[10u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[9u].xxxx).x;
    source[14].y = (g_LanceVASourceMaterialParameters[8u].yyyy).x;
    source[14].z = (g_LanceVASourceMaterialParameters[9u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[15].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[8u].zzzz).x;
    source[15].z = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[16].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[16].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[17].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[17].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[17].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[17].w = ((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].x = (sin((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[18].z = (cos((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].w = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[19].x = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[19].y = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[19].z = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[19].w = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[20].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[20].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[20].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[20].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[21].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[21].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[21].z = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[21].w = (g_LanceVASourceMaterialParameters[7u].wwww).x;
    source[22].x = (g_LanceVASourceMaterialParameters[8u].xxxx).x;
    source[22].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[19].xyxx
    r0.xy = ((v2.xyxx)*(source[19].xyxx)).xy;
    // 2: mad r1.x, cb0[10].w, cb0[18].w, r0.x
    r1.x = ((source[10].wwww)*(source[18].wwww)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[10].w, cb0[19].z, r0.y
    r1.y = ((source[10].wwww)*(source[19].zzzz)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r0.x = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 6: dp2 r1.x, cb0[7].xyxx, r0.yzyy
    r1.x = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 7: dp2 r1.y, cb0[8].xyxx, r0.yzyy
    r1.y = (dot((source[8].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 8: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: mad r0.xw, r0.xxxx, cb0[19].wwww, r1.xxxy
    r0.xw = ((r0.xxxx)*(source[19].wwww)+(r1.xxxy)).xw;
    // 10: mul r0.xw, r0.xxxw, cb0[17].xxxy
    r0.xw = ((r0.xxxw)*(source[17].xxxy)).xw;
    // 11: mad r1.x, cb0[10].w, cb0[16].w, r0.x
    r1.x = ((source[10].wwww)*(source[16].wwww)+(r0.xxxx)).x;
    // 12: mad r1.y, cb0[10].w, cb0[20].x, r0.w
    r1.y = ((source[10].wwww)*(source[20].xxxx)+(r0.wwww)).y;
    // 13: add r0.xw, r1.xxxy, cb0[20].yyyz
    r0.xw = ((r1.xxxy)+(source[20].yyyz)).xw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xwxx, t4.xyzw, s4, l(0.000000)
    r0.x = (LanceVANativeSample4((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 15: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 16: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 17: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 18: mul r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)*(source[20].wwww)).w;
    // 19: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 20: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 21: add r0.w, v4.y, l(-1.000000)
    r0.w = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 22: add_sat r0.x, -r0.w, r0.x
    r0.x = (saturate((-(r0.wwww))+(r0.xxxx))).x;
    // 23: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 24: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 25: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 26: mul_sat r0.w, r0.w, cb0[21].x
    r0.w = (saturate((r0.wwww)*(source[21].xxxx))).w;
    // 27: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 28: mul r0.x, r0.x, cb0[21].x
    r0.x = ((r0.xxxx)*(source[21].xxxx)).x;
    // 29: movc r0.w, r1.x, l(-0.000000), -r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.wwww))).w;
    // 30: mov_sat r1.x, r0.x
    r1.x = (saturate(r0.xxxx)).x;
    // 31: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 32: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: mul r1.xyz, r0.wwww, cb0[9].xyzx
    r1.xyz = ((r0.wwww)*(source[9].xyzx)).xyz;
    // 34: mul r2.xy, v2.xyxx, cb0[13].zwzz
    r2.xy = ((v2.xyxx)*(source[13].zwzz)).xy;
    // 35: mad r3.x, cb0[10].w, cb0[13].y, r2.x
    r3.x = ((source[10].wwww)*(source[13].yyyy)+(r2.xxxx)).x;
    // 36: mad r3.y, cb0[10].w, cb0[14].x, r2.y
    r3.y = ((source[10].wwww)*(source[14].xxxx)+(r2.yyyy)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (LanceVANativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 38: mad r2.xy, r0.wwww, cb0[14].yyyy, v2.xyxx
    r2.xy = ((r0.wwww)*(source[14].yyyy)+(v2.xyxx)).xy;
    // 39: mul r0.w, cb0[10].w, cb0[12].z
    r0.w = ((source[10].wwww)*(source[12].zzzz)).w;
    // 40: mad r3.x, cb0[12].w, r2.x, r0.w
    r3.x = ((source[12].wwww)*(r2.xxxx)+(r0.wwww)).x;
    // 41: mul r0.w, cb0[10].w, cb0[14].z
    r0.w = ((source[10].wwww)*(source[14].zzzz)).w;
    // 42: mad r3.y, cb0[13].x, r2.y, r0.w
    r3.y = ((source[13].xxxx)*(r2.yyyy)+(r0.wwww)).y;
    // 43: mul r2.x, v4.w, cb0[14].w
    r2.x = ((v4.wwww)*(source[14].wwww)).x;
    // 44: mul r2.y, v4.w, cb0[15].x
    r2.y = ((v4.wwww)*(source[15].xxxx)).y;
    // 45: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 47: add r1.w, v4.z, cb0[15].y
    r1.w = ((v4.zzzz)+(source[15].yyyy)).w;
    // 48: mad r2.xy, r0.wwww, r1.wwww, cb0[4].xyxx
    r2.xy = ((r0.wwww)*(r1.wwww)+(source[4].xyxx)).xy;
    // 49: dp2 r3.x, cb0[2].xyxx, r0.yzyy
    r3.x = (dot((source[2].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 50: dp2 r3.y, cb0[3].xyxx, r0.yzyy
    r3.y = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 51: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 52: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 53: mad r0.y, -r0.y, l(2.000000), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 55: mul r0.y, r0.y, cb0[21].z
    r0.y = ((r0.yyyy)*(source[21].zzzz)).y;
    // 56: max r0.y, r0.y, cb0[22].x
    r0.y = (max(r0.yyyy,source[22].xxxx)).y;
    // 57: min r0.y, r0.y, cb0[21].w
    r0.y = (min(r0.yyyy,source[21].wwww)).y;
    // 58: mad r0.zw, cb0[11].zzzw, v4.xxxx, r3.xxxy
    r0.zw = ((source[11].zzzw)*(v4.xxxx)+(r3.xxxy)).zw;
    // 59: add r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 60: mul r0.zw, r0.zzzw, cb0[11].xxxy
    r0.zw = ((r0.zzzw)*(source[11].xxxy)).zw;
    // 61: mad r3.x, cb0[10].w, cb0[10].z, r0.z
    r3.x = ((source[10].wwww)*(source[10].zzzz)+(r0.zzzz)).x;
    // 62: mad r3.y, cb0[10].w, cb0[12].y, r0.w
    r3.y = ((source[10].wwww)*(source[12].yyyy)+(r0.wwww)).y;
    // 63: add r0.zw, r2.xxxy, r3.xxxy
    r0.zw = ((r2.xxxy)+(r3.xxxy)).zw;
    // 64: add r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 65: dp2 r2.x, cb0[5].xyxx, r0.zwzz
    r2.x = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 66: dp2 r2.y, cb0[6].xyxx, r0.zwzz
    r2.y = (dot((source[6].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 67: add r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r2.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 68: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s0, l(-1.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 69: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 70: mul r0.w, r0.w, cb0[16].x
    r0.w = ((r0.wwww)*(source[16].xxxx)).w;
    // 71: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 72: mul r0.w, r0.w, cb0[16].y
    r0.w = ((r0.wwww)*(source[16].yyyy)).w;
    // 73: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 75: mad r0.w, r0.z, cb0[16].z, r0.w
    r0.w = ((r0.zzzz)*(source[16].zzzz)+(r0.wwww)).w;
    // 76: mad r1.xyz, r0.zzzz, r1.xyzx, r0.wwww
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r0.wwww)).xyz;
    // 77: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 78: mul r0.x, r0.x, cb0[22].y
    r0.x = ((r0.xxxx)*(source[22].yyyy)).x;
    // 79: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 80: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 81: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 82: mad r0.yzw, r1.xxyz, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r1.xxyz)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 83: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 84: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 85: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_a_pa_gl_01_6_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 LanceVANative658(LANCE_VA_NATIVE_INPUT input)
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
// fx_c_pa_filmnoise_01_tr: 6790453a10072947b6462e623f3de668; selected map 81cb7e05da05b927190032925551963afe1ca014e05f50352838f726f0bea752.
float4 LanceVANative659(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[3u];
    source[2] = LanceVANativeAppend(LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = (g_LanceVASourceMaterialParameters[0u].zzzz*float4(0.0, 0.0, 0.0, 0.0));
    source[4] = LanceVANativeAppend(LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))),1u);
    source[5] = (g_LanceVASourceMaterialParameters[0u].zzzz*float4(0.00200000009, 0.0, 0.0, 0.0));
    source[6] = (g_LanceVASourceMaterialParameters[0u].zzzz*float4(-0.00200000009, 0.0, 0.0, 0.0));
    source[7] = LanceVANativeAppend(LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(-0.5, 0.0, 0.0, 0.0))),1u);
    source[8] = (g_LanceVASourceMaterialParameters[0u].zzzz*float4(0.00100000005, 0.0, 0.0, 0.0));
    source[9] = (g_LanceVASourceMaterialParameters[0u].zzzz*float4(-0.00100000005, 0.0, 0.0, 0.0));
    source[10].x = (LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0)))).x;
    source[10].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].z = (((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*g_LanceVASourceMaterialParameters[2u].wwww)).x;
    source[10].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[11].z = (((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*g_LanceVASourceMaterialParameters[1u].wwww)).x;
    source[11].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[12].x = (((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(-0.5, 0.0, 0.0, 0.0))).x;
    source[12].y = (LanceVANativePeriodic(((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(-0.5, 0.0, 0.0, 0.0)))).x;
    source[12].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[12].w = (((sin(((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))*sin((((g_LanceVASourceMaterialParameters[2u].xxxx*g_LanceVASourceMaterialTime.xxxx)*float4(0.600000024, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[13].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[13].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
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
    // 1: mad r0.xy, v2.xyxx, l(1.000000, 3.000000, 0.000000, 0.000000), cb0[4].xyxx
    r0.xy = ((v2.xyxx)*(float4(1.000000,3.000000,0.000000,0.000000))+(source[4].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, r0.xxxx, cb0[11].zzzz, cb0[6].xxyx
    r0.yz = ((r0.xxxx)*(source[11].zzzz)+(source[6].xxyx)).yz;
    // 4: mad r0.xw, r0.xxxx, cb0[11].zzzz, cb0[5].xxxy
    r0.xw = ((r0.xxxx)*(source[11].zzzz)+(source[5].xxxy)).xw;
    // 5: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 6: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 7: add r0.xyzw, r0.xyzw, r1.xxyy
    r0.xyzw = ((r0.xyzw)+(r1.xxyy)).xyzw;
    // 8: add r0.yz, -r0.xxwx, r0.yyzy
    r0.yz = ((-(r0.xxwx))+(r0.yyzy)).yz;
    // 9: mad r0.xy, v2.xxxx, r0.yzyy, r0.xwxx
    r0.xy = ((v2.xxxx)*(r0.yzyy)+(r0.xwxx)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.yxzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.x = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r0.xyxx).xy).yxzw).x;
    // 11: mul r0.y, r0.x, cb0[11].w
    r0.y = ((r0.xxxx)*(source[11].wwww)).y;
    // 12: add r1.zw, v2.xxxy, cb0[7].xxxy
    r1.zw = ((v2.xxxy)+(source[7].xxxy)).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t2.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 14: mad r1.zw, r0.wwww, cb0[12].wwww, cb0[9].xxxy
    r1.zw = ((r0.wwww)*(source[12].wwww)+(source[9].xxxy)).zw;
    // 15: mad r2.xy, r0.wwww, cb0[12].wwww, cb0[8].xyxx
    r2.xy = ((r0.wwww)*(source[12].wwww)+(source[8].xyxx)).xy;
    // 16: add r2.xy, r1.xyxx, r2.xyxx
    r2.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 17: add r1.zw, r1.zzzw, r1.xxxy
    r1.zw = ((r1.zzzw)+(r1.xxxy)).zw;
    // 18: add r1.zw, -r2.xxxy, r1.zzzw
    r1.zw = ((-(r2.xxxy))+(r1.zzzw)).zw;
    // 19: mad r1.zw, v2.xxxx, r1.zzzw, r2.xxxy
    r1.zw = ((v2.xxxx)*(r1.zzzw)+(r2.xxxy)).zw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t1.xywz, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.w = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r1.zwzz).xy).xywz).w;
    // 21: mul r0.z, r0.w, cb0[13].x
    r0.z = ((r0.wwww)*(source[13].xxxx)).z;
    // 22: mad r1.zw, v2.xxxy, l(0.000000, 0.000000, 5.000000, 5.000000), cb0[2].xxxy
    r1.zw = ((v2.xxxy)*(float4(0.000000,0.000000,5.000000,5.000000))+(source[2].xxxy)).zw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.xywz, s1, l(0.000000)
    r0.w = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).w;
    // 24: mad r1.zw, r0.wwww, cb0[10].zzzz, cb0[3].xxxy
    r1.zw = ((r0.wwww)*(source[10].zzzz)+(source[3].xxxy)).zw;
    // 25: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 26: sample_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.w = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r1.xyxx).xy).yzwx).w;
    // 27: mul r0.x, r0.w, cb0[11].x
    r0.x = ((r0.wwww)*(source[11].xxxx)).x;
    // 28: mul r1.xyz, r0.xyzx, v3.xyzx
    r1.xyz = ((r0.xyzx)*(v3.xyzx)).xyz;
    // 29: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: mad r0.xyz, -r0.xyzx, v3.xyzx, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(v3.xyzx)+(r0.wwww)).xyz;
    // 31: mad r0.xyz, cb0[13].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 32: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 33: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 34: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 35: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 36: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 37: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 38: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 39: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 40: mul r0.y, r0.y, cb0[13].z
    r0.y = ((r0.yyyy)*(source[13].zzzz)).y;
    // 41: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 42: mul_sat r0.y, r0.y, cb0[13].w
    r0.y = (saturate((r0.yyyy)*(source[13].wwww))).y;
    // 43: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 44: mul r0.y, r0.y, cb0[14].x
    r0.y = ((r0.yyyy)*(source[14].xxxx)).y;
    // 45: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 46: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}
#endif
