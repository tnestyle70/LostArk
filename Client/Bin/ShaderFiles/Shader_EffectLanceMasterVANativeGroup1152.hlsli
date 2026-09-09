// Recovered LanceMaster source programs 1152..1215.
#ifndef LANCE_VA_NATIVE_MODEL_ONLY
float4 LanceVANative1200(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[10u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].xxxx,g_LanceVASourceMaterialParameters[5u].yyyy,1u);
    source[4] = input.dynamicParameter;
    source[5] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].zzzz,g_LanceVASourceMaterialParameters[1u].wwww,1u);
    source[8].x = (cos((g_LanceVASourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[8].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[8].z = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[8].w = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[9].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[9].z = (g_LanceVASourceMaterialParameters[7u].wwww).x;
    source[9].w = (g_LanceVASourceMaterialParameters[8u].xxxx).x;
    source[10].x = (g_LanceVASourceMaterialParameters[8u].yyyy).x;
    source[10].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[10].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[10].w = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[11].x = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[11].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[11].w = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[12].x = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[12].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[13].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[13].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[14].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[14].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[15].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[15].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[16].y = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
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
    // 1: mul r0.xy, v4.xyxx, cb0[9].yzyy
    r0.xy = ((v4.xyxx)*(source[9].yzyy)).xy;
    // 2: mad r1.x, cb0[4].z, cb0[9].w, r0.x
    r1.x = ((source[4].zzzz)*(source[9].wwww)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[4].z, cb0[10].x, r0.y
    r1.y = ((source[4].zzzz)*(source[10].xxxx)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 6: mul r0.x, r0.x, cb0[4].y
    r0.x = ((r0.xxxx)*(source[4].yyyy)).x;
    // 7: mad r0.yz, v4.xxyx, cb0[12].zzwz, cb0[7].xxyx
    r0.yz = ((v4.xxyx)*(source[12].zzwz)+(source[7].xxyx)).yz;
    // 8: mad r0.yz, cb0[13].zzzz, r0.xxxx, r0.yyzy
    r0.yz = ((source[13].zzzz)*(r0.xxxx)+(r0.yyzy)).yz;
    // 9: add r1.xy, cb0[4].xwxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[4].xwxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mad r2.x, r1.x, cb0[13].w, r0.y
    r2.x = ((r1.xxxx)*(source[13].wwww)+(r0.yyyy)).x;
    // 11: mad r2.y, r1.x, cb0[14].x, r0.z
    r2.y = ((r1.xxxx)*(source[14].xxxx)+(r0.zzzz)).y;
    // 12: add r0.yz, r2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 13: dp2 r2.x, cb0[5].xyxx, r0.yzyy
    r2.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 14: dp2 r2.y, cb0[6].xyxx, r0.yzyy
    r2.y = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 15: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s3, l(0.000000)
    r0.y = (LanceVANativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 17: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 18: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 19: mul r0.z, r0.z, cb0[14].y
    r0.z = ((r0.zzzz)*(source[14].yyyy)).z;
    // 20: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 21: mul_sat r0.z, r0.z, cb0[14].z
    r0.z = (saturate((r0.zzzz)*(source[14].zzzz))).z;
    // 22: mul r1.zw, cb0[4].zzzz, cb0[15].zzzw
    r1.zw = ((source[4].zzzz)*(source[15].zzzw)).zw;
    // 23: mad r1.zw, cb0[15].xxxy, v4.xxxy, r1.zzzw
    r1.zw = ((source[15].xxxy)*(v4.xxxy)+(r1.zzzw)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t2.yzwx, s4, l(0.000000)
    r0.w = (LanceVANativeSample4((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 25: add r0.w, -r1.y, r0.w
    r0.w = ((-(r1.yyyy))+(r0.wwww)).w;
    // 26: mul_sat r0.w, r0.w, cb0[16].x
    r0.w = (saturate((r0.wwww)*(source[16].xxxx))).w;
    // 27: add r1.y, -v4.y, l(1.000000)
    r1.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r1.y, r1.y, v4.y
    r1.y = ((r1.yyyy)*(v4.yyyy)).y;
    // 29: mul_sat r1.y, r1.y, cb0[14].w
    r1.y = (saturate((r1.yyyy)*(source[14].wwww))).y;
    // 30: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 31: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 32: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 33: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 34: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 35: mul r0.z, r0.z, v6.z
    r0.z = ((r0.zzzz)*(v6.zzzz)).z;
    // 36: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 37: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 38: mul r0.w, r0.w, cb0[16].y
    r0.w = ((r0.wwww)*(source[16].yyyy)).w;
    // 39: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 40: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 41: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 42: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 43: movc o0.w, r0.z, l(0), r0.y
    output.w = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 44: mad r0.yz, v4.xxyx, cb0[8].yyzy, cb0[3].xxyx
    r0.yz = ((v4.xxyx)*(source[8].yyzy)+(source[3].xxyx)).yz;
    // 45: mad r0.xy, r0.xxxx, l(0.600000, 0.600000, 0.000000, 0.000000), r0.yzyy
    r0.xy = ((r0.xxxx)*(float4(0.600000,0.600000,0.000000,0.000000))+(r0.yzyy)).xy;
    // 46: mad r0.xy, r1.xxxx, cb0[10].yzyy, r0.xyxx
    r0.xy = ((r1.xxxx)*(source[10].yzyy)+(r0.xyxx)).xy;
    // 47: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 48: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 49: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 50: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 52: mul r0.yz, cb0[4].zzzz, cb0[11].yyzy
    r0.yz = ((source[4].zzzz)*(source[11].yyzy)).yz;
    // 53: mad r1.x, v4.x, cb0[10].w, r0.y
    r1.x = ((v4.xxxx)*(source[10].wwww)+(r0.yyyy)).x;
    // 54: mad r1.y, v4.y, cb0[11].x, r0.z
    r1.y = ((v4.yyyy)*(source[11].xxxx)+(r0.zzzz)).y;
    // 55: add r0.yz, r1.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 56: dp2 r1.x, cb0[5].xyxx, r0.yzyy
    r1.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 57: dp2 r1.y, cb0[6].xyxx, r0.yzyy
    r1.y = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 58: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 60: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 61: sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // 62: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 63: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 64: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 65: mul r0.z, r0.z, cb0[11].w
    r0.z = ((r0.zzzz)*(source[11].wwww)).z;
    // 66: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 67: mul r0.z, r0.z, cb0[12].x
    r0.z = ((r0.zzzz)*(source[12].xxxx)).z;
    // 68: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 69: mad r0.x, r0.y, cb0[12].y, r0.x
    r0.x = ((r0.yyyy)*(source[12].yyyy)+(r0.xxxx)).x;
    // 70: mad r0.xyz, r0.xxxx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 71: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1201(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[9u];
    source[2] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[5u].wwww,g_LanceVASourceMaterialParameters[6u].xxxx,1u);
    source[5] = LanceVANativeAppend(cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = LanceVANativeAppend(sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].wwww,g_LanceVASourceMaterialParameters[3u].xxxx,1u);
    source[8] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].yyyy,g_LanceVASourceMaterialParameters[4u].zzzz,1u);
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(6.28000021, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((float4(6.28000021, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[6u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[9].w = ((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].x = (((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[10].y = (((g_LanceVASourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].z = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[10].w = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[11].x = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[7u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[12].x = (g_LanceVASourceMaterialParameters[8u].xxxx).x;
    source[12].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[12].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[13].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[13].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[14].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[14].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[14].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[15].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[15].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[16].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[16].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[16].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[16].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[17].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[17].y = (g_LanceVASourceMaterialParameters[8u].yyyy).x;
    source[17].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[17].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.x, v2.x, cb0[12].w
    r0.x = ((v2.xxxx)*(source[12].wwww)).x;
    // 2: mad r0.x, cb0[9].z, cb0[12].z, r0.x
    r0.x = ((source[9].zzzz)*(source[12].zzzz)+(r0.xxxx)).x;
    // 3: mul r0.z, v2.y, cb0[13].x
    r0.z = ((v2.yyyy)*(source[13].xxxx)).z;
    // 4: mad r0.y, cb0[9].z, cb0[13].y, r0.z
    r0.y = ((source[9].zzzz)*(source[13].yyyy)+(r0.zzzz)).y;
    // 5: add r0.xy, r0.xyxx, cb0[7].xyxx
    r0.xy = ((r0.xyxx)+(source[7].xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 7: mul r0.yz, v2.xxyx, cb0[14].yyzy
    r0.yz = ((v2.xxyx)*(source[14].yyzy)).yz;
    // 8: mad r0.yz, cb0[9].zzzz, cb0[14].xxwx, r0.yyzy
    r0.yz = ((source[9].zzzz)*(source[14].xxwx)+(r0.yyzy)).yz;
    // 9: add r0.yz, r0.yyzy, cb0[8].xxyx
    r0.yz = ((r0.yyzy)+(source[8].xxyx)).yz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s3, l(0.000000)
    r0.y = (LanceVANativeSample3((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 11: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 12: mul r0.y, r0.y, cb0[17].y
    r0.y = ((r0.yyyy)*(source[17].yyyy)).y;
    // 13: mad r0.yz, r0.yyyy, l(0.000000, 0.300000, 0.300000, 0.000000), v2.xxyx
    r0.yz = ((r0.yyyy)*(float4(0.000000,0.300000,0.300000,0.000000))+(v2.xxyx)).yz;
    // 14: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 15: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 16: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mul r0.w, r0.w, cb0[15].z
    r0.w = ((r0.wwww)*(source[15].zzzz)).w;
    // 18: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 19: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 20: log r0.w, r0.x
    r0.w = (log2(r0.xxxx)).w;
    // 21: mul r0.w, r0.w, cb0[15].w
    r0.w = ((r0.wwww)*(source[15].wwww)).w;
    // 22: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 23: mul r0.w, r0.w, cb0[16].x
    r0.w = ((r0.wwww)*(source[16].xxxx)).w;
    // 24: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 25: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 26: mad r0.x, r0.x, cb0[16].y, r0.w
    r0.x = ((r0.xxxx)*(source[16].yyyy)+(r0.wwww)).x;
    // 27: mad r1.x, cb0[10].z, v2.x, cb0[2].x
    r1.x = ((source[10].zzzz)*(v2.xxxx)+(source[2].xxxx)).x;
    // 28: mad r1.y, cb0[10].w, v2.y, cb0[3].y
    r1.y = ((source[10].wwww)*(v2.yyyy)+(source[3].yyyy)).y;
    // 29: add r1.xy, r1.xyxx, cb0[4].xyxx
    r1.xy = ((r1.xyxx)+(source[4].xyxx)).xy;
    // 30: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 31: dp2 r2.x, cb0[5].xyxx, r1.xyxx
    r2.x = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 32: dp2 r2.y, cb0[6].xyxx, r1.xyxx
    r2.y = (dot((source[6].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 33: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 34: mul r1.zw, v2.xxxy, cb0[11].yyyz
    r1.zw = ((v2.xxxy)*(source[11].yyyz)).zw;
    // 35: mad r1.zw, cb0[9].zzzz, cb0[11].xxxw, r1.zzzw
    r1.zw = ((source[9].zzzz)*(source[11].xxxw)+(r1.zzzw)).zw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t0.zwxy, s0, l(0.000000)
    r1.zw = (LanceVANativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 37: mad r1.xy, cb0[12].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[12].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 39: mad r0.x, r0.w, cb0[12].y, r0.x
    r0.x = ((r0.wwww)*(source[12].yyyy)+(r0.xxxx)).x;
    // 40: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 41: mad r1.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 42: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 43: mul r0.x, r0.y, cb0[16].w
    r0.x = ((r0.yyyy)*(source[16].wwww)).x;
    // 44: mul r0.y, r0.z, cb0[17].x
    r0.y = ((r0.zzzz)*(source[17].xxxx)).y;
    // 45: mad r1.y, cb0[9].z, cb0[17].z, r0.y
    r1.y = ((source[9].zzzz)*(source[17].zzzz)+(r0.yyyy)).y;
    // 46: mad r1.x, cb0[9].z, cb0[16].z, r0.x
    r1.x = ((source[9].zzzz)*(source[16].zzzz)+(r0.xxxx)).x;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r0.x = (LanceVANativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: add r0.y, v3.w, l(-1.000000)
    r0.y = ((v3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 49: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 50: add r0.x, r0.x, l(0.200000)
    r0.x = ((r0.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 51: mul_sat r0.x, r0.x, cb0[17].w
    r0.x = (saturate((r0.xxxx)*(source[17].wwww))).x;
    // 52: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 53: mul_sat r0.x, r0.x, cb0[18].x
    r0.x = (saturate((r0.xxxx)*(source[18].xxxx))).x;
    // 54: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

float4 LanceVANative1202(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[9u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[7u].zzzz,g_LanceVASourceMaterialParameters[7u].wwww,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[4] = LanceVANativeAppend(cos(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_LanceVASourceMaterialParameters[8u];
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((g_LanceVASourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[9].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[10].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[10].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[10].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[10].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[12].x = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_LanceVASourceMaterialParameters[7u].wwww).x;
    source[12].z = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[12].w = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[13].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[13].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[13].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[14].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[15].x = (cos((g_LanceVASourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[16].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[16].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[16].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[16].w = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[17].x = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
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
    // 29: mul r0.w, r0.w, v4.w
    r0.w = ((r0.wwww)*(v4.wwww)).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: lt r1.z, r0.z, l(0.000001)
    r1.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 34: mul r0.z, r0.z, cb0[16].z
    r0.z = ((r0.zzzz)*(source[16].zzzz)).z;
    // 35: max r0.z, r0.z, cb0[17].x
    r0.z = (max(r0.zzzz,source[17].xxxx)).z;
    // 36: min r0.z, r0.z, cb0[16].w
    r0.z = (min(r0.zzzz,source[16].wwww)).z;
    // 37: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 38: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 39: mad r2.x, cb0[9].w, cb0[9].z, r1.x
    r2.x = ((source[9].wwww)*(source[9].zzzz)+(r1.xxxx)).x;
    // 40: mad r2.y, cb0[9].w, cb0[10].z, r1.y
    r2.y = ((source[9].wwww)*(source[10].zzzz)+(r1.yyyy)).y;
    // 41: mul r1.x, v4.x, cb0[10].w
    r1.x = ((v4.xxxx)*(source[10].wwww)).x;
    // 42: mul r1.y, v4.x, cb0[11].x
    r1.y = ((v4.xxxx)*(source[11].xxxx)).y;
    // 43: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r1.xyxx
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 44: mul r1.zw, v2.xxxy, cb0[11].zzzw
    r1.zw = ((v2.xxxy)*(source[11].zzzw)).zw;
    // 45: mad r2.x, cb0[9].w, cb0[11].y, r1.z
    r2.x = ((source[9].wwww)*(source[11].yyyy)+(r1.zzzz)).x;
    // 46: mad r2.y, cb0[9].w, cb0[12].x, r1.w
    r2.y = ((source[9].wwww)*(source[12].xxxx)+(r1.wwww)).y;
    // 47: add r1.zw, r2.xxxy, cb0[2].xxxy
    r1.zw = ((r2.xxxy)+(source[2].xxxy)).zw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.zwzz, t0.yzwx, s1, l(0.000000)
    r0.w = (LanceVANativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: add r1.z, v4.z, cb0[12].w
    r1.z = ((v4.zzzz)+(source[12].wwww)).z;
    // 50: mad r1.zw, r0.wwww, r1.zzzz, cb0[3].xxxy
    r1.zw = ((r0.wwww)*(r1.zzzz)+(source[3].xxxy)).zw;
    // 51: add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // 52: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 53: dp2 r2.x, cb0[4].xyxx, r1.xyxx
    r2.x = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 54: dp2 r2.y, cb0[5].xyxx, r1.xyxx
    r2.y = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 55: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (LanceVANativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 57: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 58: mul r1.x, r1.x, cb0[13].z
    r1.x = ((r1.xxxx)*(source[13].zzzz)).x;
    // 59: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 60: mul r1.x, r1.x, cb0[13].w
    r1.x = ((r1.xxxx)*(source[13].wwww)).x;
    // 61: lt r1.y, |r0.w|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 62: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 63: mad r1.x, r0.w, cb0[14].x, r1.x
    r1.x = ((r0.wwww)*(source[14].xxxx)+(r1.xxxx)).x;
    // 64: dp2 r2.x, cb0[6].xyxx, r0.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[7].xyxx, r0.xyxx
    r2.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 66: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: mul r0.xy, r0.xyxx, cb0[14].zwzz
    r0.xy = ((r0.xyxx)*(source[14].zwzz)).xy;
    // 68: mad r2.x, cb0[9].w, cb0[14].y, r0.x
    r2.x = ((source[9].wwww)*(source[14].yyyy)+(r0.xxxx)).x;
    // 69: mad r2.y, cb0[9].w, cb0[15].y, r0.y
    r2.y = ((source[9].wwww)*(source[15].yyyy)+(r0.yyyy)).y;
    // 70: add r0.xy, r2.xyxx, cb0[15].zwzz
    r0.xy = ((r2.xyxx)+(source[15].zwzz)).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 72: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 73: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 74: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 75: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 76: mul r0.y, r0.y, cb0[16].y
    r0.y = ((r0.yyyy)*(source[16].yyyy)).y;
    // 77: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 78: mul_sat r0.y, r0.y, cb0[16].x
    r0.y = (saturate((r0.yyyy)*(source[16].xxxx))).y;
    // 79: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: mul r0.x, r0.x, cb0[16].x
    r0.x = ((r0.xxxx)*(source[16].xxxx)).x;
    // 81: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 82: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 83: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 84: mul r0.x, r0.x, cb0[17].y
    r0.x = ((r0.xxxx)*(source[17].yyyy)).x;
    // 85: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 86: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 87: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 88: add r0.x, r0.y, r1.y
    r0.x = ((r0.yyyy)+(r1.yyyy)).x;
    // 89: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 90: mad r0.xyz, r0.xxxx, cb0[8].xyzx, r1.xxxx
    r0.xyz = ((r0.xxxx)*(source[8].xyzx)+(r1.xxxx)).xyz;
    // 91: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 92: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1203(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.x, cb0[2].x, v4.x, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.x, cb0[2].x
    r0.y = ((v4.xxxx)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

float4 LanceVANative1204(LANCE_VA_NATIVE_INPUT input)
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

float4 LanceVANative1205(LANCE_VA_NATIVE_INPUT input)
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

float4 LanceVANative1206(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[3].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: add r0.zw, -r0.xxxy, v2.xxxy
    r0.zw = ((-(r0.xxxy))+(v2.xxxy)).zw;
    // 3: mad r0.xy, v4.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((v4.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 4: mul r0.zw, r0.xxxy, cb0[2].zzzz
    r0.zw = ((r0.xxxy)*(source[2].zzzz)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.wxyz, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 6: mul_sat r0.x, r0.x, cb0[2].x
    r0.x = (saturate((r0.xxxx)*(source[2].xxxx))).x;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.zwzz, t3.yxzw, s3, l(0.000000)
    r0.y = (LanceVANativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 8: mul_sat r0.y, r0.y, cb0[2].w
    r0.y = (saturate((r0.yyyy)*(source[2].wwww))).y;
    // 9: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 10: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 11: mul r0.z, r0.z, cb0[3].x
    r0.z = ((r0.zzzz)*(source[3].xxxx)).z;
    // 12: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 13: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, cb0[2].y
    r0.z = ((r0.zzzz)*(source[2].yyyy)).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: mul r0.z, r0.y, r0.x
    r0.z = ((r0.yyyy)*(r0.xxxx)).z;
    // 20: mad r0.x, -r0.x, r0.y, r0.x
    r0.x = ((-(r0.xxxx))*(r0.yyyy)+(r0.xxxx)).x;
    // 21: mad r0.x, v4.z, r0.x, r0.z
    r0.x = ((v4.zzzz)*(r0.xxxx)+(r0.zzzz)).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 23: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 24: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 25: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 26: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 27: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1207(LANCE_VA_NATIVE_INPUT input)
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

float4 LanceVANative1208(LANCE_VA_NATIVE_INPUT input)
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

float4 LanceVANative1209(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[0u].zzzz*g_LanceVASourceMaterialTime.xxxx)*float4(-1.29999995, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[0u].zzzz*g_LanceVASourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[0u].zzzz*g_LanceVASourceMaterialTime.xxxx)*float4(-0.5, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[0u].zzzz*g_LanceVASourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[4] = LanceVANativeAppend(LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[0u].zzzz*g_LanceVASourceMaterialTime.xxxx)*float4(-0.349999994, 0.0, 0.0, 0.0))),LanceVANativePeriodic(((g_LanceVASourceMaterialParameters[0u].zzzz*g_LanceVASourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].xxxx,g_LanceVASourceMaterialParameters[0u].yyyy,1u);
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
    // 1: mad r0.xy, v2.xyxx, l(0.500000, 2.000000, 0.000000, 0.000000), cb0[3].xyxx
    r0.xy = ((v2.xyxx)*(float4(0.500000,2.000000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mad r0.yz, v2.xxyx, l(0.000000, 1.000000, 2.000000, 0.000000), cb0[4].xxyx
    r0.yz = ((v2.xxyx)*(float4(0.000000,1.000000,2.000000,0.000000))+(source[4].xxyx)).yz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s0, l(0.000000)
    r0.y = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 5: mul r0.z, r0.y, r0.x
    r0.z = ((r0.yyyy)*(r0.xxxx)).z;
    // 6: add r1.xy, v2.xyxx, cb0[5].xyxx
    r1.xy = ((v2.xyxx)+(source[5].xyxx)).xy;
    // 7: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.070000, 0.070000), r1.xxxy
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.070000,0.070000))+(r1.xxxy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: add r0.zw, v2.xxxy, cb0[2].xxxy
    r0.zw = ((v2.xxxy)+(source[2].xxxy)).zw;
    // 10: mad r0.xy, r0.xxxx, r0.yyyy, r0.zwzz
    r0.xy = ((r0.xxxx)*(r0.yyyy)+(r0.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 13: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 14: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 15: mul o0.xyz, r0.xyzx, cb0[0].xxxx
    output.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 16: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 LanceVANative1210(LANCE_VA_NATIVE_INPUT input)
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
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.zwzz, t0.wxyz, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.xwyz, s1, l(0.000000)
    r0.y = (LanceVANativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
    // 3: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 4: mad r0.x, v0.w, r0.x, r0.y
    r0.x = ((v0.wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 5: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 6: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 7: mul r0.xy, v2.xyxx, l(8.000000, 4.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(8.000000,4.000000,0.000000,0.000000))).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xyz = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: mul r1.xyz, v3.xyzx, l(0.359551, 0.330854, 0.274713, 0.000000)
    r1.xyz = ((v3.xyzx)*(float4(0.359551,0.330854,0.274713,0.000000))).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 11: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1211(LANCE_VA_NATIVE_INPUT input)
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

float4 LanceVANative1212(LANCE_VA_NATIVE_INPUT input)
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
    float4 r0=0.f;
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
    // 13: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 14: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 15: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 16: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 17: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 LanceVANative1213(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.x, cb0[2].x, v4.x, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.x, cb0[2].x
    r0.y = ((v4.xxxx)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

float4 LanceVANative1214(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
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
    // 13: mad r0.y, cb0[2].x, v4.z, l(-1.000000)
    r0.y = ((source[2].xxxx)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 14: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 15: mul r0.z, v4.z, cb0[2].x
    r0.z = ((v4.zzzz)*(source[2].xxxx)).z;
    // 16: mad r0.yz, r0.zzzz, v2.xxyx, -r0.yyyy
    r0.yz = ((r0.zzzz)*(v2.xxyx)+(-(r0.yyyy))).yz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s1, l(0.000000)
    r1.xyzw = (LanceVANativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

float4 LanceVANative1215(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2] = g_LanceVASourceMaterialParameters[0u];
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (LanceVANativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 3: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 4: mul r0.y, r0.y, v4.x
    r0.y = ((r0.yyyy)*(v4.xxxx)).y;
    // 5: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 6: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 7: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 8: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 10: mul r0.y, r1.w, l(3.000000)
    r0.y = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 11: mad r1.xyz, cb0[2].xyzx, r1.xyzx, r1.xxxx
    r1.xyz = ((source[2].xyzx)*(r1.xyzx)+(r1.xxxx)).xyz;
    // 12: mad r1.xyz, v3.xyzx, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((v3.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 13: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 14: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 15: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 16: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 17: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
