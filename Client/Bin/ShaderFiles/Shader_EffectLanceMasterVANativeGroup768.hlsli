// Single source owner for LanceVANative profiles 768..831.
#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_l_pa_ap_23_1_tr: c9e2873553a59d42857bb5e72fae97c3; selected map 10507a3a0c024475158714574bdf0b83d9009d1538f833f7cc434126ffd9fcec.
float4 LanceVANative768(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[6u];
    source[2] = LanceVANativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = LanceVANativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[5].w = (clamp(g_LanceVASourceMaterialParameters[5u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_LanceVASourceMaterialParameters[5u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[7].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[7].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[9].x = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[10].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[10].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[10].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[11].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx))).x;
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
    r0.xz = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).xz;
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
    r0.w = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 21: mad r2.x, cb0[4].y, cb0[7].w, r0.x
    r2.x = ((source[4].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 22: mad r2.y, cb0[4].y, cb0[8].z, r0.z
    r2.y = ((source[4].yyyy)*(source[8].zzzz)+(r0.zzzz)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_n_pa_shine_08_tr: a14d57c96465f346be3437ad63c9ad10; selected map 0716202d132c7fa3cb9c994bb5370a1db2710298fb3e58040e580c23ad3e1c1b.
float4 LanceVANative769(LANCE_VA_NATIVE_INPUT input)
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
// fx_k_pa_turbulence_01_tr: f2ecc1c1e40aa34d879ceeabe82dfc21; selected map c345cefde3ac6ecbf5923057310970d247e2eee9b09817d3af90d16e36f482f6.
float4 LanceVANative770(LANCE_VA_NATIVE_INPUT input)
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
// bfx_d_pa_master_01_01_ad: e042827679e152468e9af203098ff8ae; selected map ac7b3b6c6c96f877181de9750eb9017464049052c612774604d7d9cc5ad38649.
float4 LanceVANative771(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[6u];
    source[2] = g_LanceVASourceMaterialParameters[5u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].xxxx,g_LanceVASourceMaterialParameters[2u].yyyy,1u);
    source[4].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[4].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[5].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[6].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[7].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[2u].zzzz)).x;
    source[7].w = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[8].y = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
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
    // 1: add r0.x, v4.y, l(-1.000000)
    r0.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[4].y, cb0[7].y, cb0[7].z
    r0.y = ((source[4].yyyy)*(source[7].yyyy)+(source[7].zzzz)).y;
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
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 14: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 16: mul_sat r0.x, r0.x, cb0[8].y
    r0.x = (saturate((r0.xxxx)*(source[8].yyyy))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, cb0[8].z
    r0.y = ((r0.yyyy)*(source[8].zzzz)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 22: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 23: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 24: mul r0.y, v2.x, cb0[4].w
    r0.y = ((v2.xxxx)*(source[4].wwww)).y;
    // 25: mul r0.z, cb0[4].x, cb0[4].y
    r0.z = ((source[4].xxxx)*(source[4].yyyy)).z;
    // 26: mad r1.x, r0.z, cb0[4].z, r0.y
    r1.x = ((r0.zzzz)*(source[4].zzzz)+(r0.yyyy)).x;
    // 27: mul r0.yw, v2.yyyx, cb0[5].xxxw
    r0.yw = ((v2.yyyx)*(source[5].xxxw)).yw;
    // 28: mad r1.yz, r0.zzzz, cb0[5].yyzy, r0.yywy
    r1.yz = ((r0.zzzz)*(source[5].yyzy)+(r0.yywy)).yz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s0, l(0.000000)
    r2.xyz = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 30: mul r0.y, v2.y, cb0[6].x
    r0.y = ((v2.yyyy)*(source[6].xxxx)).y;
    // 31: mad r1.w, r0.z, cb0[6].y, r0.y
    r1.w = ((r0.zzzz)*(source[6].yyyy)+(r0.yyyy)).w;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.zwzz, t2.wxyz, s1, l(0.000000)
    r0.yzw = (LanceVANativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 33: mul r1.xyz, r0.yzwy, r2.xyzx
    r1.xyz = ((r0.yzwy)*(r2.xyzx)).xyz;
    // 34: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: mad r0.yzw, -r2.xxyz, r0.yyzw, r1.wwww
    r0.yzw = ((-(r2.xxyz))*(r0.yyzw)+(r1.wwww)).yzw;
    // 36: mad r0.yzw, cb0[6].zzzz, r0.yyzw, r1.xxyz
    r0.yzw = ((source[6].zzzz)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 37: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 38: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 39: mul r0.yzw, r0.yyzw, cb0[6].wwww
    r0.yzw = ((r0.yyzw)*(source[6].wwww)).yzw;
    // 40: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 41: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 42: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 43: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 44: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 45: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 46: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

// sk_flm_hor_01_01_mi_dead: 89e0f405daa79e449a4a77b9816624f0; selected map a179bb957eee797ffbbf952f49b12a920e37c7ee2697d98e0a43e8a5b8dd61ee.
float4 LanceVANative772(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[38]; [unroll] for (uint i=0u; i<38u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing project scene adapters: absolute source-cm origin, current scene hemispherical lighting.
    source[0]=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[1]=float4(0.f,1.f,0.f,1.f); // View fade identity; no scene camera distance fade is authored for this cue.
    // Unbound native environment-cube/SH contribution stays zero, matching the existing SourceCharacter environment boundary.
    source[35]=float4(input.skyUpperColor,0.f);
    source[36]=float4(input.skyLowerColor,0.f);
    source[37]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_LanceVASourceMaterialParameters[16u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = g_LanceVASourceMaterialParameters[9u];
    source[5] = g_LanceVASourceMaterialParameters[10u];
    source[6] = g_LanceVASourceMaterialParameters[11u];
    source[7] = g_LanceVASourceMaterialParameters[18u];
    source[8] = g_LanceVASourceMaterialParameters[13u];
    source[9] = g_LanceVASourceMaterialParameters[12u];
    source[10] = g_LanceVASourceMaterialParameters[19u];
    source[11] = g_LanceVASourceMaterialParameters[7u];
    source[12] = g_LanceVASourceMaterialParameters[8u];
    source[13] = g_LanceVASourceMaterialParameters[14u];
    source[14] = g_LanceVASourceMaterialParameters[15u];
    source[15] = LanceVANativeAppend(g_LanceVASourceMaterialTime.xxxx,g_LanceVASourceMaterialTime.xxxx,1u);
    source[16] = g_LanceVASourceMaterialParameters[17u];
    source[17].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[17].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[17].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[17].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[18].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[18].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[18].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[19].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[19].y = ((g_LanceVASourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[19].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[19].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[20].x = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialParameters[0u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[20].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[20].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[20].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[21].x = (clamp(g_LanceVASourceMaterialParameters[2u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[21].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[21].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[21].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[22].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[22].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[22].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[22].w = ((g_LanceVASourceMaterialParameters[4u].xxxx*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[23].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[23].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[23].z = ((g_LanceVASourceMaterialParameters[4u].yyyy*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[23].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[24].x = ((g_LanceVASourceMaterialParameters[4u].zzzz*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[24].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[21].zzzz
    r0.xy = ((v4.xyxx)*(source[21].zzzz)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s5, l(0.000000)
    r0.x = (LanceVANativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[21].w
    r0.x = ((r0.xxxx)+(-(source[21].wwww))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: mul_sat r0.x, r0.x, cb0[22].x
    r0.x = (saturate((r0.xxxx)*(source[22].xxxx))).x;
    // 8: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 9: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 11: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 12: add r0.x, -cb0[7].w, l(1.000000)
    r0.x = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 14: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 15: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 16: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mul r0.y, cb0[7].z, l(1.500000)
    r0.y = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 18: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 19: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 20: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 21: mul r2.x, r0.y, l(0.125000)
    r2.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 22: mul r3.y, cb0[7].y, cb0[15].y
    r3.y = ((source[7].yyyy)*(source[15].yyyy)).y;
    // 23: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 24: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 25: add r0.yz, r2.xxyx, r3.xxyx
    r0.yz = ((r2.xxyx)+(r3.xxyx)).yz;
    // 26: frc r0.w, cb0[7].x
    r0.w = (frac(source[7].xxxx)).w;
    // 27: add r1.w, -r0.w, cb0[7].x
    r1.w = ((-(r0.wwww))+(source[7].xxxx)).w;
    // 28: mul r3.z, r1.w, l(0.125000)
    r3.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 29: add r0.yz, r0.yyzy, r3.zzwz
    r0.yz = ((r0.yyzy)+(r3.zzwz)).yz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t5.xyzw, s4, l(0.000000)
    r2.xyzw = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 31: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 32: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 33: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 35: mad r2.xyz, cb0[17].wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((source[17].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 36: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 38: mad r2.xyz, cb0[18].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[18].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 39: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 40: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 41: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 42: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 43: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 44: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 47: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 48: mul r1.w, r6.y, cb0[17].y
    r1.w = ((r6.yyyy)*(source[17].yyyy)).w;
    // 49: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 50: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 52: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 53: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 54: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 55: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 56: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 57: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 58: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 59: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 60: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r7.xyz = (LanceVANativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 62: mad r3.xyz, r7.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r7.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 63: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 64: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 65: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 66: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 67: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 68: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 69: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 70: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 71: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 72: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 73: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 74: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 75: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 76: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 78: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 79: mul_sat r8.w, r1.w, cb2[3].w
    r8.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 80: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 81: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 82: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 84: mad r4.xyz, cb0[17].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[17].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 85: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 86: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 87: add r3.xyz, -r4.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 88: mad r3.xyz, cb0[18].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[18].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 89: mad r4.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 90: mad r9.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 91: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 92: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 93: mul r9.xyz, r2.xyzx, r3.xyzx
    r9.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 94: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: mad r10.xyz, -r3.xyzx, r2.xyzx, r1.wwww
    r10.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 96: mad r2.xyz, r3.xyzx, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = ((r3.xyzx)*(r2.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 97: mad r3.xyz, cb0[17].wwww, r10.xyzx, r9.xyzx
    r3.xyz = ((source[17].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 98: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 99: add r9.xyz, -r3.xyzx, r1.wwww
    r9.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 100: mad r3.xyz, cb0[18].xxxx, r9.xyzx, r3.xyzx
    r3.xyz = ((source[18].xxxx)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 101: mul r3.xyz, r4.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 102: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 103: mad r0.xyz, r0.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 104: mul r0.w, r6.x, cb0[20].y
    r0.w = ((r6.xxxx)*(source[20].yyyy)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[20].z
    r0.w = (saturate((r0.wwww)+(source[20].zzzz))).w;
    // 108: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r3.xyz, r1.wwww, cb0[14].xyzx
    r3.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 110: mul r4.xyz, r0.xyzx, r3.xyzx
    r4.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 111: mad r0.xyz, -r3.xyzx, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r3.xyzx))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 112: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 113: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 114: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 115: mad_sat r3.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 116: mad r0.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r0.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 117: mad r4.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 118: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 119: mad r4.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 120: mad r0.xyz, r0.xyzx, r0.wwww, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // 121: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 122: max r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (max(r0.xyzx,r0.wwww)).xyz;
    // 123: mov_sat r3.w, cb0[21].y
    r3.w = (saturate(source[21].yyyy)).w;
    // 124: mad r4.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r4.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 125: mul r1.w, r3.w, l(0.080000)
    r1.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 127: mad r4.xyz, r8.wwww, r4.xyzx, r1.wwww
    r4.xyz = ((r8.wwww)*(r4.xyzx)+(r1.wwww)).xyz;
    // 128: add r1.w, cb0[22].w, -cb0[23].x
    r1.w = ((source[22].wwww)+(-(source[23].xxxx))).w;
    // 129: mad r1.w, r7.x, r1.w, cb0[23].x
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[23].xxxx)).w;
    // 130: add r2.w, -r1.w, cb0[23].z
    r2.w = ((-(r1.wwww))+(source[23].zzzz)).w;
    // 131: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 132: add r2.w, -r1.w, cb0[24].x
    r2.w = ((-(r1.wwww))+(source[24].xxxx)).w;
    // 133: mad r1.w, r7.z, r2.w, r1.w
    r1.w = ((r7.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 134: mul r1.w, r6.z, r1.w
    r1.w = ((r6.zzzz)*(r1.wwww)).w;
    // 135: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 136: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: movc r1.w, r5.z, l(0), r1.w
    r1.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 138: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 139: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 141: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 142: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 143: mul r5.xy, r5.xyxx, cb0[17].xxxx
    r5.xy = ((r5.xyxx)*(source[17].xxxx)).xy;
    // 144: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 147: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 148: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 149: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 150: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 151: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 152: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 153: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 154: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 156: mul r7.xyz, r1.wwww, v5.xyzx
    r7.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 158: deriv_rtx_coarse r8.x, r1.w
    r8.x = (ddx_coarse(r1.wwww)).x;
    // 159: deriv_rty_coarse r8.y, r1.w
    r8.y = (ddy_coarse(r1.wwww)).y;
    // 160: dp2 r2.w, r8.xyxx, r8.xyxx
    r2.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 161: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 162: mad r2.w, r2.w, l(0.300000), r8.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz)).w;
    // 164: min r8.y, r2.w, l(1.000000)
    r8.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r9.xyz, r4.xyzx, r2.wwww
    r9.xyz = (max(r4.xyzx,r2.wwww)).xyz;
    // 167: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 168: mul_sat r2.w, r4.y, l(50.000000)
    r2.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 170: mul r10.xyz, r1.wwww, r6.xyzx
    r10.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 171: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 172: add r2.w, r10.z, l(1.000000)
    r2.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 176: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 177: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 178: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 179: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 180: add_sat r8.x, -r2.w, r3.w
    r8.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 181: sample_indexable(texture2d)(float,float,float,float) r11.xy, r8.xyxx, t7.xyzw, s8 (unbound native scene environment contribution)
    r11.xy = (float4(0.f,0.f,0.f,0.f)).xy;
    // 182: add r2.w, r0.w, r8.x
    r2.w = ((r0.wwww)+(r8.xxxx)).w;
    // 183: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 184: mul r12.xyz, r4.xyzx, r11.yyyy
    r12.xyz = ((r4.xyzx)*(r11.yyyy)).xyz;
    // 185: mad r9.xyz, r9.xyzx, r11.xxxx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xxxx)+(r12.xyzx)).xyz;
    // 186: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r11.yyyy)).w;
    // 187: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 188: mad r11.xyz, r4.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r4.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 189: dp3 r3.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 190: mad r4.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 191: mad r12.xyz, -r9.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 193: mul r11.xyz, r3.xyzx, r12.xyzx
    r11.xyz = ((r3.xyzx)*(r12.xyzx)).xyz;
    // 194: add r3.w, -r8.w, l(1.000000)
    r3.w = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: mul r11.xyz, r3.wwww, r11.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)).xyz;
    // 196: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 197: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 198: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 199: mul r13.xyz, r5.wwww, v1.xyzx
    r13.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 200: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 201: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 202: mul r14.xyz, r5.wwww, v0.xyzx
    r14.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 203: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 204: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 205: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 206: dp3 r16.y, r15.xyzx, r6.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 207: dp3 r15.y, r15.xyzx, r10.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 208: dp3 r16.x, r14.xyzx, r6.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 209: dp3 r15.x, r14.xyzx, r10.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 210: dp2 r14.z, r16.xyxx, cb0[26].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 211: mul r8.xz, cb0[26].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[26].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 212: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 213: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 214: dp2 r17.z, r15.xyxx, cb0[26].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 215: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 216: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 217: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 218: dp4 r13.x, cb0[27].xyzw, r14.xyzw
    r13.x = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 219: dp4 r13.y, cb0[28].xyzw, r14.xyzw
    r13.y = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 220: dp4 r13.z, cb0[29].xyzw, r14.xyzw
    r13.z = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 221: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 222: dp4 r18.x, cb0[30].xyzw, r15.xyzw
    r18.x = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 223: dp4 r18.y, cb0[31].xyzw, r15.xyzw
    r18.y = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 224: dp4 r18.z, cb0[32].xyzw, r15.xyzw
    r18.z = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 225: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 226: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 227: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 228: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 229: mad r13.xyz, cb0[33].xyzx, r5.wwww, r13.xyzx
    r13.xyz = ((source[33].xyzx)*(r5.wwww)+(r13.xyzx)).xyz;
    // 230: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 231: mul r13.xyz, r13.xyzx, cb0[25].xyzx
    r13.xyz = ((r13.xyzx)*(source[25].xyzx)).xyz;
    // 232: mul r13.xyz, r13.xyzx, cb0[26].zzzz
    r13.xyz = ((r13.xyzx)*(source[26].zzzz)).xyz;
    // 233: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[25].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[25].wwww)).xyz;
    // 234: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 235: add r13.xyz, -r5.wwww, r13.xyzx
    r13.xyz = ((-(r5.wwww))+(r13.xyzx)).xyz;
    // 236: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 237: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 238: mad r6.w, r8.y, l(2.000000), l(2.000000)
    r6.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 239: div r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)/(r6.wwww)).w;
    // 240: mad r5.w, r4.w, l(5.000000), r5.w
    r5.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 241: add_sat r5.w, r8.w, r5.w
    r5.w = (saturate((r8.wwww)+(r5.wwww))).w;
    // 242: mad r7.w, r5.w, l(-2.000000), l(3.000000)
    r7.w = ((r5.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 243: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 244: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 245: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 246: mul r5.w, r5.w, l(1.500000)
    r5.w = ((r5.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 247: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 248: mul r13.xyz, r5.wwww, r13.xyzx
    r13.xyz = ((r5.wwww)*(r13.xyzx)).xyz;
    // 249: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 250: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 251: mul r11.xyz, r0.xyzx, r11.xyzx
    r11.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 252: mul r5.w, r8.y, l(5.000000)
    r5.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 253: mul r7.w, r8.y, r8.y
    r7.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 254: mul r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)*(r7.wwww)).w;
    // 255: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 256: add r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)+(r2.wwww)).w;
    // 258: add_sat r0.w, r2.w, l(-1.000000)
    r0.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 259: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t8.xyzw, s7, r5.w (unbound native scene environment contribution)
    r13.xyzw = (float4(0.f,0.f,0.f,0.f)).xyzw;
    // 260: mul r8.xyz, r13.xyzx, r13.wwww
    r8.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 261: mul r8.xyz, r8.xyzx, cb0[25].xyzx
    r8.xyz = ((r8.xyzx)*(source[25].xyzx)).xyz;
    // 262: mul r8.xyz, r8.xyzx, cb0[26].zzzz
    r8.xyz = ((r8.xyzx)*(source[26].zzzz)).xyz;
    // 263: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[25].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[25].wwww)).xyz;
    // 264: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 265: add r8.xyz, -r2.wwww, r8.xyzx
    r8.xyz = ((-(r2.wwww))+(r8.xyzx)).xyz;
    // 266: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 267: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 268: div r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)/(r6.wwww)).w;
    // 269: mad r2.w, r4.w, l(5.000000), r2.w
    r2.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 270: add_sat r2.w, r8.w, r2.w
    r2.w = (saturate((r8.wwww)+(r2.wwww))).w;
    // 271: mad r4.w, r2.w, l(-2.000000), l(3.000000)
    r4.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 272: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 273: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 274: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 275: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 276: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 277: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 278: mul r13.xyz, r8.xyzx, r9.xyzx
    r13.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 279: mad r2.w, r0.w, r4.x, r4.y
    r2.w = ((r0.wwww)*(r4.xxxx)+(r4.yyyy)).w;
    // 280: mad r2.w, r2.w, r0.w, r4.z
    r2.w = ((r2.wwww)*(r0.wwww)+(r4.zzzz)).w;
    // 281: mul r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)*(r2.wwww)).w;
    // 282: max r0.w, r0.w, r2.w
    r0.w = (max(r0.wwww,r2.wwww)).w;
    // 283: mad r4.xyz, r13.xyzx, r0.wwww, r11.xyzx
    r4.xyz = ((r13.xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 284: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 285: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 286: mul r11.xyz, r2.wwww, v6.xyzx
    r11.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 287: dp3 r2.w, r11.xyzx, r6.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 288: dp3 r4.w, -r11.xyzx, r6.xyzx
    r4.w = (dot((-(r11.xyzx)).xyz,(r6.xyzx).xyz).xxxx).w;
    // 289: dp3 r5.w, r11.xyzx, r10.xyzx
    r5.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 290: mad r6.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 291: mad r6.zw, r4.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r4.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 292: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 293: mad r10.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 294: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 295: mul r10.yzw, r10.yyyy, cb0[36].xxyz
    r10.yzw = ((r10.yyyy)*(source[36].xxyz)).yzw;
    // 296: mad r10.xyz, r10.xxxx, cb0[35].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[35].xyzx)+(r10.yzwy)).xyz;
    // 297: mul r10.xyz, r10.xyzx, cb0[37].wwww
    r10.xyz = ((r10.xyzx)*(source[37].wwww)).xyz;
    // 298: mul r10.xyz, r3.xyzx, r10.xyzx
    r10.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 299: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 300: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 301: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 302: mad r0.xyz, -r0.xyzx, r8.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r8.wwww)+(r0.xyzx)).xyz;
    // 303: mad r0.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 304: mul r4.xyz, r6.yyyy, cb0[36].xyzx
    r4.xyz = ((r6.yyyy)*(source[36].xyzx)).xyz;
    // 305: mad r4.xyz, cb0[35].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[35].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 306: mul r4.xyz, r4.xyzx, cb0[37].wwww
    r4.xyz = ((r4.xyzx)*(source[37].wwww)).xyz;
    // 307: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 308: mul r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)*(r4.xyzx)).xyz;
    // 309: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 310: mad r0.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 311: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 313: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 314: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 315: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 316: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 317: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 318: add r2.xyz, r2.xyzx, -r4.xyzx
    r2.xyz = ((r2.xyzx)+(-(r4.xyzx))).xyz;
    // 319: dp3 r0.w, r5.xyzx, r7.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 320: mul_sat r2.w, r0.w, cb0[18].y
    r2.w = (saturate((r0.wwww)*(source[18].yyyy))).w;
    // 321: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 322: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 323: mul_sat r4.x, r7.z, cb0[18].y
    r4.x = (saturate((r7.zzzz)*(source[18].yyyy))).x;
    // 324: add r4.y, -|r7.z|, l(1.000000)
    r4.y = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 325: mul r0.w, r0.w, r4.y
    r0.w = ((r0.wwww)*(r4.yyyy)).w;
    // 326: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 327: add_sat r4.x, r4.x, -cb0[18].z
    r4.x = (saturate((r4.xxxx)+(-(source[18].zzzz)))).x;
    // 328: log r4.y, r4.x
    r4.y = (log2(r4.xxxx)).y;
    // 329: lt r4.x, r4.x, l(0.000001)
    r4.x = (asfloat((uint4)((r4.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 330: mul r4.y, r4.y, cb0[18].w
    r4.y = ((r4.yyyy)*(source[18].wwww)).y;
    // 331: exp r4.y, r4.y
    r4.y = (exp2(r4.yyyy)).y;
    // 332: mul r2.w, r2.w, r4.y
    r2.w = ((r2.wwww)*(r4.yyyy)).w;
    // 333: movc r2.w, r4.x, l(0), r2.w
    r2.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 334: mul r4.xyz, cb0[12].xyzx, cb0[19].yyyy
    r4.xyz = ((source[12].xyzx)*(source[19].yyyy)).xyz;
    // 335: mul r4.xyz, r4.xyzx, cb0[20].xxxx
    r4.xyz = ((r4.xyzx)*(source[20].xxxx)).xyz;
    // 336: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 337: mad r5.xyz, r2.wwww, cb0[11].xyzx, -cb0[11].xyzx
    r5.xyz = ((r2.wwww)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 338: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 339: mad r2.w, cb0[10].w, r2.w, l(1.000000)
    r2.w = ((source[10].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 340: mad r5.xyz, cb0[11].wwww, r5.xyzx, cb0[11].xyzx
    r5.xyz = ((source[11].wwww)*(r5.xyzx)+(source[11].xyzx)).xyz;
    // 341: mad r2.xyz, r2.xyzx, r4.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 342: mad r2.xyz, r2.wwww, cb0[10].xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(source[10].xyzx)+(r2.xyzx)).xyz;
    // 343: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 344: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 345: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 346: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 347: mul r4.xyz, r2.wwww, cb0[13].xyzx
    r4.xyz = ((r2.wwww)*(source[13].xyzx)).xyz;
    // 348: movc r4.xyz, r0.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 349: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 350: mad r1.xyz, cb0[17].zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((source[17].zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 351: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 352: mul r2.xyz, r6.wwww, cb0[36].xyzx
    r2.xyz = ((r6.wwww)*(source[36].xyzx)).xyz;
    // 353: mad r2.xyz, r6.zzzz, cb0[35].xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(source[35].xyzx)+(r2.xyzx)).xyz;
    // 354: mul r2.xyz, r2.xyzx, cb0[37].wwww
    r2.xyz = ((r2.xyzx)*(source[37].wwww)).xyz;
    // 355: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t6.yzwx, s6, l(0.000000)
    r0.w = (LanceVANativeSample6((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 356: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 357: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 358: mul_sat r4.xyz, cb0[16].xyzx, cb0[16].wwww
    r4.xyz = (saturate((source[16].xyzx)*(source[16].wwww))).xyz;
    // 359: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 360: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 362: mul r4.xyz, r3.wwww, r5.xyzx
    r4.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 363: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 364: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 365: mad r1.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 366: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 368: mad o0.xyz, r3.xyzx, cb0[37].xyzx, r1.xyzx
    output.xyz = ((r3.xyzx)*(source[37].xyzx)+(r1.xyzx)).xyz;
    // 369: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}

// sk_flm_hor_01_02_mi_dead: 7f1aa7738f054740a70276b015d53b0d; selected map 4d7cc38c7353a107b9734344feb4ae8f775c502922a9c0fcbe7f6e8c1f8deae4.
float4 LanceVANative773(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[38]; [unroll] for (uint i=0u; i<38u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing project scene adapters: absolute source-cm origin, current scene hemispherical lighting.
    source[0]=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[1]=float4(0.f,1.f,0.f,1.f); // View fade identity; no scene camera distance fade is authored for this cue.
    // Unbound native environment-cube/SH contribution stays zero, matching the existing SourceCharacter environment boundary.
    source[35]=float4(input.skyUpperColor,0.f);
    source[36]=float4(input.skyLowerColor,0.f);
    source[37]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_LanceVASourceMaterialParameters[16u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = g_LanceVASourceMaterialParameters[9u];
    source[5] = g_LanceVASourceMaterialParameters[10u];
    source[6] = g_LanceVASourceMaterialParameters[11u];
    source[7] = g_LanceVASourceMaterialParameters[18u];
    source[8] = g_LanceVASourceMaterialParameters[13u];
    source[9] = g_LanceVASourceMaterialParameters[12u];
    source[10] = g_LanceVASourceMaterialParameters[19u];
    source[11] = g_LanceVASourceMaterialParameters[7u];
    source[12] = g_LanceVASourceMaterialParameters[8u];
    source[13] = g_LanceVASourceMaterialParameters[14u];
    source[14] = g_LanceVASourceMaterialParameters[15u];
    source[15] = LanceVANativeAppend(g_LanceVASourceMaterialTime.xxxx,g_LanceVASourceMaterialTime.xxxx,1u);
    source[16] = g_LanceVASourceMaterialParameters[17u];
    source[17].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[17].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[17].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[17].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[18].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[18].y = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[18].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[18].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[19].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[19].y = ((g_LanceVASourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[19].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[19].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[20].x = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialParameters[0u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[20].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[20].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[20].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[21].x = (clamp(g_LanceVASourceMaterialParameters[2u].wwww,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[21].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[21].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[21].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[22].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[22].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[22].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[22].w = ((g_LanceVASourceMaterialParameters[4u].xxxx*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[23].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[23].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[23].z = ((g_LanceVASourceMaterialParameters[4u].yyyy*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[23].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[24].x = ((g_LanceVASourceMaterialParameters[4u].zzzz*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[24].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[21].zzzz
    r0.xy = ((v4.xyxx)*(source[21].zzzz)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s5, l(0.000000)
    r0.x = (LanceVANativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[21].w
    r0.x = ((r0.xxxx)+(-(source[21].wwww))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: mul_sat r0.x, r0.x, cb0[22].x
    r0.x = (saturate((r0.xxxx)*(source[22].xxxx))).x;
    // 8: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 9: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 11: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 12: add r0.x, -cb0[7].w, l(1.000000)
    r0.x = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 14: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 15: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 16: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mul r0.y, cb0[7].z, l(1.500000)
    r0.y = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 18: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 19: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 20: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 21: mul r2.x, r0.y, l(0.125000)
    r2.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 22: mul r3.y, cb0[7].y, cb0[15].y
    r3.y = ((source[7].yyyy)*(source[15].yyyy)).y;
    // 23: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 24: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 25: add r0.yz, r2.xxyx, r3.xxyx
    r0.yz = ((r2.xxyx)+(r3.xxyx)).yz;
    // 26: frc r0.w, cb0[7].x
    r0.w = (frac(source[7].xxxx)).w;
    // 27: add r1.w, -r0.w, cb0[7].x
    r1.w = ((-(r0.wwww))+(source[7].xxxx)).w;
    // 28: mul r3.z, r1.w, l(0.125000)
    r3.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 29: add r0.yz, r0.yyzy, r3.zzwz
    r0.yz = ((r0.yyzy)+(r3.zzwz)).yz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t5.xyzw, s4, l(0.000000)
    r2.xyzw = (LanceVANativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 31: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 32: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 33: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 35: mad r2.xyz, cb0[17].wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((source[17].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 36: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 38: mad r2.xyz, cb0[18].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[18].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 39: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 40: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 41: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 42: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 43: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 44: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 46: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 47: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 48: mul r1.w, r6.y, cb0[17].y
    r1.w = ((r6.yyyy)*(source[17].yyyy)).w;
    // 49: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 50: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 52: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 53: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 54: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 55: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 56: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 57: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 58: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 59: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 60: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r7.xyz = (LanceVANativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 62: mad r3.xyz, r7.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r7.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 63: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 64: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 65: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 66: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 67: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 68: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 69: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 70: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 71: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 72: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 73: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 74: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 75: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 76: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 78: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 79: mul_sat r8.w, r1.w, cb2[3].w
    r8.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 80: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 81: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 82: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 84: mad r4.xyz, cb0[17].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[17].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 85: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 86: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 87: add r3.xyz, -r4.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 88: mad r3.xyz, cb0[18].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[18].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 89: mad r4.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 90: mad r9.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 91: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 92: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 93: mul r9.xyz, r2.xyzx, r3.xyzx
    r9.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 94: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: mad r10.xyz, -r3.xyzx, r2.xyzx, r1.wwww
    r10.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 96: mad r2.xyz, r3.xyzx, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = ((r3.xyzx)*(r2.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 97: mad r3.xyz, cb0[17].wwww, r10.xyzx, r9.xyzx
    r3.xyz = ((source[17].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 98: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 99: add r9.xyz, -r3.xyzx, r1.wwww
    r9.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 100: mad r3.xyz, cb0[18].xxxx, r9.xyzx, r3.xyzx
    r3.xyz = ((source[18].xxxx)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 101: mul r3.xyz, r4.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 102: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 103: mad r0.xyz, r0.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 104: mul r0.w, r6.x, cb0[20].y
    r0.w = ((r6.xxxx)*(source[20].yyyy)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[20].z
    r0.w = (saturate((r0.wwww)+(source[20].zzzz))).w;
    // 108: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r3.xyz, r1.wwww, cb0[14].xyzx
    r3.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 110: mul r4.xyz, r0.xyzx, r3.xyzx
    r4.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 111: mad r0.xyz, -r3.xyzx, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r3.xyzx))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 112: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 113: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 114: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 115: mad_sat r3.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 116: mad r0.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r0.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 117: mad r4.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 118: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 119: mad r4.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 120: mad r0.xyz, r0.xyzx, r0.wwww, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // 121: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 122: max r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (max(r0.xyzx,r0.wwww)).xyz;
    // 123: mov_sat r3.w, cb0[21].y
    r3.w = (saturate(source[21].yyyy)).w;
    // 124: mad r4.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r4.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 125: mul r1.w, r3.w, l(0.080000)
    r1.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 127: mad r4.xyz, r8.wwww, r4.xyzx, r1.wwww
    r4.xyz = ((r8.wwww)*(r4.xyzx)+(r1.wwww)).xyz;
    // 128: add r1.w, cb0[22].w, -cb0[23].x
    r1.w = ((source[22].wwww)+(-(source[23].xxxx))).w;
    // 129: mad r1.w, r7.x, r1.w, cb0[23].x
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[23].xxxx)).w;
    // 130: add r2.w, -r1.w, cb0[23].z
    r2.w = ((-(r1.wwww))+(source[23].zzzz)).w;
    // 131: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 132: add r2.w, -r1.w, cb0[24].x
    r2.w = ((-(r1.wwww))+(source[24].xxxx)).w;
    // 133: mad r1.w, r7.z, r2.w, r1.w
    r1.w = ((r7.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 134: mul r1.w, r6.z, r1.w
    r1.w = ((r6.zzzz)*(r1.wwww)).w;
    // 135: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 136: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: movc r1.w, r5.z, l(0), r1.w
    r1.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 138: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 139: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 141: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 142: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 143: mul r5.xy, r5.xyxx, cb0[17].xxxx
    r5.xy = ((r5.xyxx)*(source[17].xxxx)).xy;
    // 144: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 147: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 148: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 149: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 150: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 151: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 152: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 153: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 154: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 156: mul r7.xyz, r1.wwww, v5.xyzx
    r7.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 158: deriv_rtx_coarse r8.x, r1.w
    r8.x = (ddx_coarse(r1.wwww)).x;
    // 159: deriv_rty_coarse r8.y, r1.w
    r8.y = (ddy_coarse(r1.wwww)).y;
    // 160: dp2 r2.w, r8.xyxx, r8.xyxx
    r2.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 161: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 162: mad r2.w, r2.w, l(0.300000), r8.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz)).w;
    // 164: min r8.y, r2.w, l(1.000000)
    r8.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r9.xyz, r4.xyzx, r2.wwww
    r9.xyz = (max(r4.xyzx,r2.wwww)).xyz;
    // 167: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 168: mul_sat r2.w, r4.y, l(50.000000)
    r2.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 170: mul r10.xyz, r1.wwww, r6.xyzx
    r10.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 171: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 172: add r2.w, r10.z, l(1.000000)
    r2.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 176: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 177: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 178: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 179: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 180: mul r1.w, r1.w, cb0[24].y
    r1.w = ((r1.wwww)*(source[24].yyyy)).w;
    // 181: add_sat r8.x, -r2.w, r3.w
    r8.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r11.xy, r8.xyxx, t6.xyzw, s7 (unbound native scene environment contribution)
    r11.xy = (float4(0.f,0.f,0.f,0.f)).xy;
    // 183: add r2.w, r0.w, r8.x
    r2.w = ((r0.wwww)+(r8.xxxx)).w;
    // 184: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 185: mul r12.xyz, r4.xyzx, r11.yyyy
    r12.xyz = ((r4.xyzx)*(r11.yyyy)).xyz;
    // 186: mad r9.xyz, r9.xyzx, r11.xxxx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xxxx)+(r12.xyzx)).xyz;
    // 187: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r11.yyyy)).w;
    // 188: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 189: mad r11.xyz, r4.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r4.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 190: dp3 r3.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 191: mad r4.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r12.xyz, -r9.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 194: mul r11.xyz, r3.xyzx, r12.xyzx
    r11.xyz = ((r3.xyzx)*(r12.xyzx)).xyz;
    // 195: add r3.w, -r8.w, l(1.000000)
    r3.w = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 196: mul r11.xyz, r3.wwww, r11.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)).xyz;
    // 197: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 199: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 200: mul r13.xyz, r5.wwww, v1.xyzx
    r13.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 201: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 202: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 203: mul r14.xyz, r5.wwww, v0.xyzx
    r14.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 204: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 205: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 206: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 207: dp3 r16.y, r15.xyzx, r6.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 208: dp3 r15.y, r15.xyzx, r10.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 209: dp3 r16.x, r14.xyzx, r6.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 210: dp3 r15.x, r14.xyzx, r10.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 211: dp2 r14.z, r16.xyxx, cb0[26].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 212: mul r8.xz, cb0[26].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[26].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 213: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 214: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 215: dp2 r17.z, r15.xyxx, cb0[26].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 216: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 217: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 218: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 219: dp4 r13.x, cb0[27].xyzw, r14.xyzw
    r13.x = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 220: dp4 r13.y, cb0[28].xyzw, r14.xyzw
    r13.y = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 221: dp4 r13.z, cb0[29].xyzw, r14.xyzw
    r13.z = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 222: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 223: dp4 r18.x, cb0[30].xyzw, r15.xyzw
    r18.x = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 224: dp4 r18.y, cb0[31].xyzw, r15.xyzw
    r18.y = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 225: dp4 r18.z, cb0[32].xyzw, r15.xyzw
    r18.z = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 226: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 227: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 228: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 229: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 230: mad r13.xyz, cb0[33].xyzx, r5.wwww, r13.xyzx
    r13.xyz = ((source[33].xyzx)*(r5.wwww)+(r13.xyzx)).xyz;
    // 231: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 232: mul r13.xyz, r13.xyzx, cb0[25].xyzx
    r13.xyz = ((r13.xyzx)*(source[25].xyzx)).xyz;
    // 233: mul r13.xyz, r13.xyzx, cb0[26].zzzz
    r13.xyz = ((r13.xyzx)*(source[26].zzzz)).xyz;
    // 234: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[25].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[25].wwww)).xyz;
    // 235: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: add r13.xyz, -r5.wwww, r13.xyzx
    r13.xyz = ((-(r5.wwww))+(r13.xyzx)).xyz;
    // 237: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 238: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 239: mad r6.w, r8.y, l(2.000000), l(2.000000)
    r6.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 240: div r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)/(r6.wwww)).w;
    // 241: mad r5.w, r4.w, l(5.000000), r5.w
    r5.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 242: add_sat r5.w, r8.w, r5.w
    r5.w = (saturate((r8.wwww)+(r5.wwww))).w;
    // 243: mad r7.w, r5.w, l(-2.000000), l(3.000000)
    r7.w = ((r5.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 244: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 245: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 246: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 247: mul r5.w, r5.w, l(1.500000)
    r5.w = ((r5.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 248: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 249: mul r13.xyz, r5.wwww, r13.xyzx
    r13.xyz = ((r5.wwww)*(r13.xyzx)).xyz;
    // 250: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 251: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 252: mul r11.xyz, r0.xyzx, r11.xyzx
    r11.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 253: mul r5.w, r8.y, l(5.000000)
    r5.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 254: mul r7.w, r8.y, r8.y
    r7.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 255: mul r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)*(r7.wwww)).w;
    // 256: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 257: add r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)+(r2.wwww)).w;
    // 259: add_sat r0.w, r2.w, l(-1.000000)
    r0.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t7.xyzw, s6, r5.w (unbound native scene environment contribution)
    r13.xyzw = (float4(0.f,0.f,0.f,0.f)).xyzw;
    // 261: mul r8.xyz, r13.xyzx, r13.wwww
    r8.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 262: mul r8.xyz, r8.xyzx, cb0[25].xyzx
    r8.xyz = ((r8.xyzx)*(source[25].xyzx)).xyz;
    // 263: mul r8.xyz, r8.xyzx, cb0[26].zzzz
    r8.xyz = ((r8.xyzx)*(source[26].zzzz)).xyz;
    // 264: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[25].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[25].wwww)).xyz;
    // 265: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r8.xyz, -r2.wwww, r8.xyzx
    r8.xyz = ((-(r2.wwww))+(r8.xyzx)).xyz;
    // 267: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 268: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)/(r6.wwww)).w;
    // 270: mad r2.w, r4.w, l(5.000000), r2.w
    r2.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 271: add_sat r2.w, r8.w, r2.w
    r2.w = (saturate((r8.wwww)+(r2.wwww))).w;
    // 272: mad r4.w, r2.w, l(-2.000000), l(3.000000)
    r4.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 274: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 275: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 276: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 278: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 279: mul r13.xyz, r8.xyzx, r9.xyzx
    r13.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 280: mad r2.w, r0.w, r4.x, r4.y
    r2.w = ((r0.wwww)*(r4.xxxx)+(r4.yyyy)).w;
    // 281: mad r2.w, r2.w, r0.w, r4.z
    r2.w = ((r2.wwww)*(r0.wwww)+(r4.zzzz)).w;
    // 282: mul r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)*(r2.wwww)).w;
    // 283: max r0.w, r0.w, r2.w
    r0.w = (max(r0.wwww,r2.wwww)).w;
    // 284: mad r4.xyz, r13.xyzx, r0.wwww, r11.xyzx
    r4.xyz = ((r13.xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 285: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 287: mul r11.xyz, r2.wwww, v6.xyzx
    r11.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r2.w, r11.xyzx, r6.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 289: dp3 r4.w, -r11.xyzx, r6.xyzx
    r4.w = (dot((-(r11.xyzx)).xyz,(r6.xyzx).xyz).xxxx).w;
    // 290: dp3 r5.w, r11.xyzx, r10.xyzx
    r5.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 291: mad r6.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 292: mad r6.zw, r4.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r4.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 293: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 294: mad r10.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 296: mul r10.yzw, r10.yyyy, cb0[36].xxyz
    r10.yzw = ((r10.yyyy)*(source[36].xxyz)).yzw;
    // 297: mad r10.xyz, r10.xxxx, cb0[35].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[35].xyzx)+(r10.yzwy)).xyz;
    // 298: mul r10.xyz, r10.xyzx, cb0[37].wwww
    r10.xyz = ((r10.xyzx)*(source[37].wwww)).xyz;
    // 299: mul r10.xyz, r3.xyzx, r10.xyzx
    r10.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 300: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 301: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 302: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 303: mad r0.xyz, -r0.xyzx, r8.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r8.wwww)+(r0.xyzx)).xyz;
    // 304: mad r0.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 305: mul r4.xyz, r6.yyyy, cb0[36].xyzx
    r4.xyz = ((r6.yyyy)*(source[36].xyzx)).xyz;
    // 306: mad r4.xyz, cb0[35].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[35].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 307: mul r4.xyz, r4.xyzx, cb0[37].wwww
    r4.xyz = ((r4.xyzx)*(source[37].wwww)).xyz;
    // 308: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 309: mul r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)*(r4.xyzx)).xyz;
    // 310: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 311: mad r0.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 312: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 314: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 315: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 316: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 317: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 318: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 319: add r2.xyz, r2.xyzx, -r4.xyzx
    r2.xyz = ((r2.xyzx)+(-(r4.xyzx))).xyz;
    // 320: dp3 r0.w, r5.xyzx, r7.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 321: mul_sat r2.w, r0.w, cb0[18].y
    r2.w = (saturate((r0.wwww)*(source[18].yyyy))).w;
    // 322: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 323: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 324: mul_sat r4.x, r7.z, cb0[18].y
    r4.x = (saturate((r7.zzzz)*(source[18].yyyy))).x;
    // 325: add r4.y, -|r7.z|, l(1.000000)
    r4.y = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 326: mul r0.w, r0.w, r4.y
    r0.w = ((r0.wwww)*(r4.yyyy)).w;
    // 327: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 328: add_sat r4.x, r4.x, -cb0[18].z
    r4.x = (saturate((r4.xxxx)+(-(source[18].zzzz)))).x;
    // 329: log r4.y, r4.x
    r4.y = (log2(r4.xxxx)).y;
    // 330: lt r4.x, r4.x, l(0.000001)
    r4.x = (asfloat((uint4)((r4.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 331: mul r4.y, r4.y, cb0[18].w
    r4.y = ((r4.yyyy)*(source[18].wwww)).y;
    // 332: exp r4.y, r4.y
    r4.y = (exp2(r4.yyyy)).y;
    // 333: mul r2.w, r2.w, r4.y
    r2.w = ((r2.wwww)*(r4.yyyy)).w;
    // 334: movc r2.w, r4.x, l(0), r2.w
    r2.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 335: mul r4.xyz, cb0[12].xyzx, cb0[19].yyyy
    r4.xyz = ((source[12].xyzx)*(source[19].yyyy)).xyz;
    // 336: mul r4.xyz, r4.xyzx, cb0[20].xxxx
    r4.xyz = ((r4.xyzx)*(source[20].xxxx)).xyz;
    // 337: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 338: mad r5.xyz, r2.wwww, cb0[11].xyzx, -cb0[11].xyzx
    r5.xyz = ((r2.wwww)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 339: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 340: mad r2.w, cb0[10].w, r2.w, l(1.000000)
    r2.w = ((source[10].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 341: mad r5.xyz, cb0[11].wwww, r5.xyzx, cb0[11].xyzx
    r5.xyz = ((source[11].wwww)*(r5.xyzx)+(source[11].xyzx)).xyz;
    // 342: mad r2.xyz, r2.xyzx, r4.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 343: mad r2.xyz, r2.wwww, cb0[10].xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(source[10].xyzx)+(r2.xyzx)).xyz;
    // 344: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 345: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 346: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 347: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 348: mul r4.xyz, r2.wwww, cb0[13].xyzx
    r4.xyz = ((r2.wwww)*(source[13].xyzx)).xyz;
    // 349: movc r4.xyz, r0.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 350: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 351: mad r1.xyz, cb0[17].zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((source[17].zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 352: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 353: mul r2.xyz, r6.wwww, cb0[36].xyzx
    r2.xyz = ((r6.wwww)*(source[36].xyzx)).xyz;
    // 354: mad r2.xyz, r6.zzzz, cb0[35].xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(source[35].xyzx)+(r2.xyzx)).xyz;
    // 355: mul r2.xyz, r2.xyzx, cb0[37].wwww
    r2.xyz = ((r2.xyzx)*(source[37].wwww)).xyz;
    // 356: mul_sat r4.xyz, cb0[16].xyzx, cb0[16].wwww
    r4.xyz = (saturate((source[16].xyzx)*(source[16].wwww))).xyz;
    // 357: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 358: mul r4.xyz, r4.xyzx, cb0[24].yyyy
    r4.xyz = ((r4.xyzx)*(source[24].yyyy)).xyz;
    // 360: mul r4.xyz, r3.wwww, r5.xyzx
    r4.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 361: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 362: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 363: mad r1.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 364: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 366: mad o0.xyz, r3.xyzx, cb0[37].xyzx, r1.xyzx
    output.xyz = ((r3.xyzx)*(source[37].xyzx)+(r1.xyzx)).xyz;
    // 367: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}

// sk_flm_hor_01_03_dead: fff2e8a629416a4fb1392c738ff27e03; selected map 80e7e99c3062286bb9ce895c7adf1cd795cfd54771bc01f6472a536506f29e43.
float4 LanceVANative774(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[30]; [unroll] for (uint i=0u; i<30u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing project scene adapters: absolute source-cm origin, current scene hemispherical lighting.
    source[0]=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[1]=float4(input.sourceCameraPosition,0.f);
    source[27]=float4(input.skyUpperColor,0.f);
    source[28]=float4(input.skyLowerColor,0.f);
    source[29]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_LanceVASourceMaterialParameters[18u];
    source[3] = g_LanceVASourceMaterialParameters[19u];
    source[4] = g_LanceVASourceMaterialParameters[9u];
    source[5] = g_LanceVASourceMaterialParameters[10u];
    source[6] = g_LanceVASourceMaterialParameters[11u];
    source[7] = g_LanceVASourceMaterialParameters[12u];
    source[8] = g_LanceVASourceMaterialParameters[14u];
    source[9] = g_LanceVASourceMaterialParameters[13u];
    source[10] = g_LanceVASourceMaterialParameters[16u];
    source[11] = g_LanceVASourceMaterialParameters[17u];
    source[12] = g_LanceVASourceMaterialParameters[21u];
    source[13] = g_LanceVASourceMaterialParameters[7u];
    source[14] = g_LanceVASourceMaterialParameters[8u];
    source[15] = g_LanceVASourceMaterialParameters[15u];
    source[16] = LanceVANativeAppend(g_LanceVASourceMaterialTime.xxxx,g_LanceVASourceMaterialTime.xxxx,1u);
    source[17] = g_LanceVASourceMaterialParameters[20u];
    source[18] = LanceVANativeAppend(cos((g_LanceVASourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))),1u);
    source[19] = LanceVANativeAppend(sin((g_LanceVASourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[20].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[20].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[20].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[20].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[21].x = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[21].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[21].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[21].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[22].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[22].y = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[22].z = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[22].w = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[23].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[23].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[23].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[23].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[24].x = ((g_LanceVASourceMaterialParameters[0u].zzzz*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[24].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[24].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[24].w = ((g_LanceVASourceMaterialParameters[0u].wwww*g_LanceVASourceMaterialTime.xxxx)).x;
    source[25].x = (((g_LanceVASourceMaterialParameters[0u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[25].y = (sin(((g_LanceVASourceMaterialParameters[0u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[25].z = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialParameters[0u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[25].w = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialParameters[0u].wwww*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[26].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[26].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[26].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[26].w = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[26].xxxx
    r0.xy = ((v4.xyxx)*(source[26].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s6, l(0.000000)
    r0.x = (LanceVANativeSample6((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[26].y
    r0.x = ((r0.xxxx)+(-(source[26].yyyy))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xyzw = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: mul_sat r0.x, r0.x, cb0[26].z
    r0.x = (saturate((r0.xxxx)*(source[26].zzzz))).x;
    // 8: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 9: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 11: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 12: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 13: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 14: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 15: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 16: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 17: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 18: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 19: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 20: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 21: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 22: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 23: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 24: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 25: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 26: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = (LanceVANativeSample3((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 28: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 30: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 31: mul r0.w, r0.w, cb0[21].x
    r0.w = ((r0.wwww)*(source[21].xxxx)).w;
    // 32: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 33: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 35: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 36: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 37: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 38: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 39: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 40: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxwy).yw;
    // 42: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 43: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 44: mul r3.xy, r0.ywyy, cb0[20].xxxx
    r3.xy = ((r0.ywyy)*(source[20].xxxx)).xy;
    // 45: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 46: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 47: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 48: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 49: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 50: mad r4.xyz, cb0[20].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 51: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 52: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 53: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 54: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 55: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 56: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 57: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 58: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 59: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 60: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 61: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 62: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 63: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 64: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 65: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 66: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 67: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 68: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 69: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 70: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 71: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 72: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 73: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 74: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 75: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 76: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 77: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 78: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 79: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 80: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 81: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 82: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 83: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 84: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = (LanceVANativeSample4((r0.ywyy).xy, (r0.xxxx).x, true).xywz).xyw;
    // 85: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 86: rcp r1.w, cb0[21].z
    r1.w = (1.0/(source[21].zzzz)).w;
    // 87: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 88: mul r6.xyz, r6.xyzx, cb0[21].zzzz
    r6.xyz = ((r6.xyzx)*(source[21].zzzz)).xyz;
    // 89: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 90: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 91: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 92: mad r6.xyz, r6.xyzx, cb0[21].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[21].zzzz)+(r10.xyzx)).xyz;
    // 93: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 94: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 95: add r1.w, cb0[21].z, l(1.000000)
    r1.w = ((source[21].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 97: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 98: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 99: mad r6.xyz, r2.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 100: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 101: mul r0.xyw, r0.xyxw, cb0[21].wwww
    r0.xyw = ((r0.xyxw)*(source[21].wwww)).xyw;
    // 102: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 103: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 104: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 105: dp3 r1.w, r3.xyzx, r4.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 106: mul_sat r2.w, r1.w, cb0[22].y
    r2.w = (saturate((r1.wwww)*(source[22].yyyy))).w;
    // 107: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul_sat r3.w, r4.z, cb0[22].y
    r3.w = (saturate((r4.zzzz)*(source[22].yyyy))).w;
    // 110: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: add_sat r3.w, r3.w, -cb0[22].z
    r3.w = (saturate((r3.wwww)+(-(source[22].zzzz)))).w;
    // 112: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 113: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 114: mul r4.w, r4.w, cb0[22].w
    r4.w = ((r4.wwww)*(source[22].wwww)).w;
    // 115: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 116: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 117: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 118: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 120: mad r2.xyz, cb0[20].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 121: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 123: mad r2.xyz, cb0[20].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 124: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 126: mul r6.xyz, r6.xyzx, cb0[22].xxxx
    r6.xyz = ((r6.xyzx)*(source[22].xxxx)).xyz;
    // 127: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r10.xyzw = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 128: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 129: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 130: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 131: mad r2.xyz, r3.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 132: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 133: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 134: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 135: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 136: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 138: mul r3.w, r3.w, cb0[23].x
    r3.w = ((r3.wwww)*(source[23].xxxx)).w;
    // 139: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 140: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 143: div r4.w, cb0[23].y, r4.w
    r4.w = ((source[23].yyyy)/(r4.wwww)).w;
    // 144: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 145: mul r6.xyz, r0.xywx, r4.wwww
    r6.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 146: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 147: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 148: mad r1.xyz, cb0[20].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 149: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 151: mad r1.xyz, cb0[20].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 152: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 153: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 154: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 155: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 156: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 157: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r10.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r10.xywx))).xyz;
    // 158: mad r10.xyz, r10.zzzz, r11.xyzx, r10.xywx
    r10.xyz = ((r10.zzzz)*(r11.xyzx)+(r10.xywx)).xyz;
    // 159: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 160: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 161: mad r10.xyz, cb0[20].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 162: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 164: mad r10.xyz, cb0[20].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 165: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 166: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 168: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 169: mul r12.xyz, r1.xyzx, r10.xyzx
    r12.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 170: mad r1.xyz, r10.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 171: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 172: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 173: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: mul r4.w, r4.w, cb0[23].z
    r4.w = ((r4.wwww)*(source[23].zzzz)).w;
    // 175: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 176: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 177: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 178: mul r6.xyz, r0.xywx, r2.yyyy
    r6.xyz = ((r0.xywx)*(r2.yyyy)).xyz;
    // 179: dp3 r2.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 180: mad r10.xyz, -r2.yyyy, r0.xywx, r2.zzzz
    r10.xyz = ((-(r2.yyyy))*(r0.xywx)+(r2.zzzz)).xyz;
    // 181: mad r6.xyz, cb0[20].yyyy, r10.xyzx, r6.xyzx
    r6.xyz = ((source[20].yyyy)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 182: dp3 r2.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 183: add r10.xyz, -r6.xyzx, r2.yyyy
    r10.xyz = ((-(r6.xyzx))+(r2.yyyy)).xyz;
    // 184: mad r6.xyz, cb0[20].zzzz, r10.xyzx, r6.xyzx
    r6.xyz = ((source[20].zzzz)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 185: dp3 r2.y, r1.xyzx, r1.xyzx
    r2.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 186: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 187: div r1.xyz, r1.xyzx, r2.yyyy
    r1.xyz = ((r1.xyzx)/(r2.yyyy)).xyz;
    // 188: dp3 r2.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 189: add r10.xyz, -r1.xyzx, r2.yyyy
    r10.xyz = ((-(r1.xyzx))+(r2.yyyy)).xyz;
    // 190: add r1.xyz, r1.xyzx, -r10.xyzx
    r1.xyz = ((r1.xyzx)+(-(r10.xyzx))).xyz;
    // 191: mul r10.xyz, cb0[14].xyzx, cb0[24].xxxx
    r10.xyz = ((source[14].xyzx)*(source[24].xxxx)).xyz;
    // 192: mul r10.xyz, r10.xyzx, cb0[25].wwww
    r10.xyz = ((r10.xyzx)*(source[25].wwww)).xyz;
    // 193: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 194: mad r13.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r13.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 195: add r2.y, r2.w, l(-1.000000)
    r2.y = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 196: mad r2.y, cb0[12].w, r2.y, l(1.000000)
    r2.y = ((source[12].wwww)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 197: mad r13.xyz, cb0[13].wwww, r13.xyzx, cb0[13].xyzx
    r13.xyz = ((source[13].wwww)*(r13.xyzx)+(source[13].xyzx)).xyz;
    // 198: mad r1.xyz, r1.xyzx, r10.xyzx, r13.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)+(r13.xyzx)).xyz;
    // 199: mad r1.xyz, r2.yyyy, cb0[12].xyzx, r1.xyzx
    r1.xyz = ((r2.yyyy)*(source[12].xyzx)+(r1.xyzx)).xyz;
    // 200: mad r1.xyz, r6.xyzx, r11.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 201: add r2.y, -|r4.z|, l(1.000000)
    r2.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 202: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 203: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 204: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 205: mul r2.y, r2.y, l(1.500000)
    r2.y = ((r2.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 206: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 207: mul r2.yzw, r2.yyyy, cb0[15].xxyz
    r2.yzw = ((r2.yyyy)*(source[15].xxyz)).yzw;
    // 208: movc r2.yzw, r1.wwww, l(0,0,0,0), r2.yyzw
    r2.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyzw)).yzw;
    // 209: add r1.xyz, r1.xyzx, r2.yzwy
    r1.xyz = ((r1.xyzx)+(r2.yzwy)).xyz;
    // 210: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 211: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 212: sqrt r2.y, r1.w
    r2.y = (sqrt(r1.wwww)).y;
    // 213: div r2.yzw, r9.xxyz, r2.yyyy
    r2.yzw = ((r9.xxyz)/(r2.yyyy)).yzw;
    // 214: dp3 r2.y, r2.yzwy, r4.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).y;
    // 215: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 216: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 217: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 218: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 219: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 220: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 221: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 222: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 223: div_sat r1.w, r2.y, r1.w
    r1.w = (saturate((r2.yyyy)/(r1.wwww))).w;
    // 224: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 225: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 226: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 227: mad r0.xyz, r3.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 228: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 230: mad r0.xyz, cb0[20].yyyy, r2.yzwy, r0.xyzx
    r0.xyz = ((source[20].yyyy)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 231: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 232: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 233: mad r0.xyz, cb0[20].zzzz, r2.yzwy, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 234: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 235: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 236: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 237: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 238: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 239: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 240: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 241: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 242: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 243: add r1.w, -r2.x, cb0[3].x
    r1.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 244: mul r4.z, r1.w, l(0.125000)
    r4.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 245: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 246: mul r6.x, r1.w, l(0.125000)
    r6.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 247: mul r4.y, cb0[3].y, cb0[16].y
    r4.y = ((source[3].yyyy)*(source[16].yyyy)).y;
    // 248: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 249: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 250: add r2.yz, r4.xxyx, r6.xxyx
    r2.yz = ((r4.xxyx)+(r6.xxyx)).yz;
    // 251: add r2.yz, r2.yyzy, r4.zzwz
    r2.yz = ((r2.yyzy)+(r4.zzwz)).yz;
    // 252: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.yzyy, t6.xyzw, s5, l(0.000000)
    r4.xyzw = (LanceVANativeSample5((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 253: mul r2.yzw, r0.wwww, r4.xxyz
    r2.yzw = ((r0.wwww)*(r4.xxyz)).yzw;
    // 254: mul r0.w, r2.x, r4.w
    r0.w = ((r2.xxxx)*(r4.wwww)).w;
    // 255: mad r2.xyz, r2.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 256: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 257: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 258: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 259: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 260: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 261: mad r2.xy, cb0[17].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 262: mul r0.w, cb0[17].y, cb0[24].y
    r0.w = ((source[17].yyyy)*(source[24].yyyy)).w;
    // 263: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 264: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 265: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 266: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 267: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 268: mul r1.w, cb0[17].x, l(0.001000)
    r1.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 269: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 270: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 271: dp2 r1.w, cb0[18].xyxx, r2.xyxx
    r1.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 272: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 273: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 274: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r2.xyzw = (LanceVANativeSample5((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 276: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 277: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 278: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 279: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 280: mad r4.xyz, cb0[17].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[17].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 281: mul r2.xyz, r2.xyzx, cb0[17].zzzz
    r2.xyz = ((r2.xyzx)*(source[17].zzzz)).xyz;
    // 282: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 283: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 284: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 285: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 286: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 287: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 288: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 289: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 290: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 291: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 292: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 293: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 294: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 295: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 296: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 297: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 298: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 299: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 301: mad o0.xyz, r0.xyzx, cb0[29].xyzx, r1.xyzx
    output.xyz = ((r0.xyzx)*(source[29].xyzx)+(r1.xyzx)).xyz;
    // 303: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}

// sk_flm_hor_01_04_mi_dead: a54025e4f4b1364bbb87fbfc241cd843; selected map a1363800a349ca8451ec1d4b622ce2a6399b90d2b9953a3407d909a1dd258c97.
float4 LanceVANative775(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[24]; [unroll] for (uint i=0u; i<24u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    // Existing project scene adapters: absolute source-cm origin, current scene hemispherical lighting.
    source[0]=0.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i) projection[i]=input.sourceProjection[i];
    source[0]=float4(1.f,0.f,0.f,0.f);
    source[1].x=input.color.a;
    source[21]=float4(input.skyUpperColor,0.f);
    source[22]=float4(input.skyLowerColor,0.f);
    source[23]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_LanceVASourceMaterialParameters[11u];
    source[3] = g_LanceVASourceMaterialParameters[14u];
    source[4] = g_LanceVASourceMaterialParameters[15u];
    source[5] = g_LanceVASourceMaterialParameters[8u];
    source[6] = g_LanceVASourceMaterialParameters[7u];
    source[7] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[1u].wwww,2u);
    source[8] = g_LanceVASourceMaterialParameters[9u];
    source[9] = g_LanceVASourceMaterialParameters[6u];
    source[10] = g_LanceVASourceMaterialParameters[13u];
    source[11] = g_LanceVASourceMaterialParameters[12u];
    source[12] = LanceVANativeAppend(g_LanceVASourceMaterialTime.xxxx,g_LanceVASourceMaterialTime.xxxx,1u);
    source[13].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[13].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[13].w = ((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialParameters[5u].yyyy)).x;
    source[14].x = (clamp((g_LanceVASourceMaterialParameters[5u].yyyy*g_LanceVASourceMaterialParameters[5u].yyyy),float4(0.100000001, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[14].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[14].z = (clamp(g_LanceVASourceMaterialParameters[5u].yyyy,float4(0.100000001, 0.0, 0.0, 0.0),float4(0.5, 0.0, 0.0, 0.0))).x;
    source[14].w = (clamp(g_LanceVASourceMaterialParameters[5u].xxxx,float4(0.200000003, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[15].x = ((float4(0.5, 0.0, 0.0, 0.0)*clamp(g_LanceVASourceMaterialParameters[5u].xxxx,float4(0.200000003, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[15].z = ((g_LanceVASourceMaterialParameters[4u].wwww*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[15].w = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[16].x = ((g_LanceVASourceMaterialParameters[4u].zzzz*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[16].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[16].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[16].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[17].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[17].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[17].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[17].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[3u].wwww)).x;
    source[18].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[18].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[18].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[18].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[19].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[19].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[19].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[19].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[20].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[20].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[20].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,input.uv1.yx); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v8 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: add r0.x, v4.w, l(0.500000)
    r0.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 2: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 3: mad r0.z, cb0[13].y, l(-3.500000), l(5.000000)
    r0.z = ((source[13].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 4: mul r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)*(source[14].xxxx)).z;
    // 5: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 6: add r1.x, -r0.w, v4.z
    r1.x = ((-(r0.wwww))+(v4.zzzz)).x;
    // 7: mad r0.w, cb0[14].y, r1.x, r0.w
    r0.w = ((source[14].yyyy)*(r1.xxxx)+(r0.wwww)).w;
    // 8: mul r1.x, r0.w, cb0[14].z
    r1.x = ((r0.wwww)*(source[14].zzzz)).x;
    // 9: mad r0.w, r1.x, l(0.750000), r0.w
    r0.w = ((r1.xxxx)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 10: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 11: mad_sat r0.w, cb0[15].x, r0.w, r0.w
    r0.w = (saturate((source[15].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 13: add r2.x, -r1.z, l(1.000000)
    r2.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 15: mad r3.xy, r2.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r2.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r2.y, r3.x, r1.x, l(0.200000)
    r2.y = ((r3.xxxx)*(r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 17: add r2.x, -r2.y, r2.x
    r2.x = ((-(r2.yyyy))+(r2.xxxx)).x;
    // 18: mad r2.z, cb0[16].x, r2.x, r2.y
    r2.z = ((source[16].xxxx)*(r2.xxxx)+(r2.yyyy)).z;
    // 19: mad r2.x, cb0[15].z, r2.x, r2.y
    r2.x = ((source[15].zzzz)*(r2.xxxx)+(r2.yyyy)).x;
    // 20: add r2.xy, -r0.wwww, r2.xzxx
    r2.xy = ((-(r0.wwww))+(r2.xzxx)).xy;
    // 21: mul r2.z, cb0[14].w, l(0.700000)
    r2.z = ((source[14].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).z;
    // 22: mad r2.y, r2.z, r2.y, r0.w
    r2.y = ((r2.zzzz)*(r2.yyyy)+(r0.wwww)).y;
    // 23: mad r0.w, r2.z, r2.x, r0.w
    r0.w = ((r2.zzzz)*(r2.xxxx)+(r0.wwww)).w;
    // 24: div r0.w, r0.w, cb0[15].y
    r0.w = ((r0.wwww)/(source[15].yyyy)).w;
    // 25: add r0.yw, -r0.xxxw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = ((-(r0.xxxw))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 26: mul r0.w, r0.w, r0.z
    r0.w = ((r0.wwww)*(r0.zzzz)).w;
    // 27: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 28: mul_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)*(r0.wwww))).x;
    // 29: div r0.w, r2.y, cb0[15].w
    r0.w = ((r2.yyyy)/(source[15].wwww)).w;
    // 30: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 32: mul r0.z, r0.z, l(4.000000)
    r0.z = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).z;
    // 33: mul_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)*(r0.zzzz))).y;
    // 34: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 35: add r0.x, -r1.z, r0.x
    r0.x = ((-(r1.zzzz))+(r0.xxxx)).x;
    // 36: mad r0.x, cb0[16].y, r0.x, r1.z
    r0.x = ((source[16].yyyy)*(r0.xxxx)+(r1.zzzz)).x;
    // 37: add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // 38: mul r0.yzw, r0.yyzw, cb0[13].xxxx
    r0.yzw = ((r0.yyzw)*(source[13].xxxx)).yzw;
    // 39: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r0.xyz, cb0[16].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 43: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 45: mad r0.xyz, cb0[16].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[16].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 46: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 47: mad r4.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 48: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 49: mad r4.xyz, r0.xyzx, r2.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r4.xyz = ((r0.xyzx)*(r2.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 50: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 51: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 52: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 53: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 54: div r2.xyz, r4.xyzx, r0.wwww
    r2.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 55: mul r1.xyz, r1.yyyy, r2.xyzx
    r1.xyz = ((r1.yyyy)*(r2.xyzx)).xyz;
    // 56: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 57: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 58: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 59: mov_sat r0.w, r2.z
    r0.w = (saturate(r2.zzzz)).w;
    // 60: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 61: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 62: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 63: mul r1.xyz, r1.xyzx, cb0[17].xxxx
    r1.xyz = ((r1.xyzx)*(source[17].xxxx)).xyz;
    // 64: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 65: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 67: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 68: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 69: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 70: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 71: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 72: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 73: mov_sat r2.x, r0.w
    r2.x = (saturate(r0.wwww)).x;
    // 74: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: mul_sat r2.y, r2.z, cb0[18].z
    r2.y = (saturate((r2.zzzz)*(source[18].zzzz))).y;
    // 76: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 77: mad_sat r2.y, r2.y, cb0[18].z, -cb0[18].w
    r2.y = (saturate((r2.yyyy)*(source[18].zzzz)+(-(source[18].wwww)))).y;
    // 78: log r2.w, r2.y
    r2.w = (log2(r2.yyyy)).w;
    // 79: lt r2.y, r2.y, l(0.000001)
    r2.y = (asfloat((uint4)((r2.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: mul r2.w, r2.w, cb0[19].x
    r2.w = ((r2.wwww)*(source[19].xxxx)).w;
    // 81: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 82: mul r2.x, r2.w, r2.x
    r2.x = ((r2.wwww)*(r2.xxxx)).x;
    // 83: movc r2.x, r2.y, l(0), r2.x
    r2.x = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 84: mad r4.xyz, r2.xxxx, cb0[9].xyzx, -cb0[9].xyzx
    r4.xyz = ((r2.xxxx)*(source[9].xyzx)+(-(source[9].xyzx))).xyz;
    // 85: mul r2.x, r2.x, cb0[10].w
    r2.x = ((r2.xxxx)*(source[10].wwww)).x;
    // 86: mad r4.xyz, cb0[9].wwww, r4.xyzx, cb0[9].xyzx
    r4.xyz = ((source[9].wwww)*(r4.xyzx)+(source[9].xyzx)).xyz;
    // 87: mad r4.xyz, r0.wwww, cb0[8].xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(source[8].xyzx)+(r4.xyzx)).xyz;
    // 88: mad r2.xyw, r2.xxxx, cb0[10].xyxz, r4.xyxz
    r2.xyw = ((r2.xxxx)*(source[10].xyxz)+(r4.xyxz)).xyw;
    // 89: add r4.xyz, v8.xyzx, cb0[0].yzwy
    r4.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 90: add r4.xyz, -r4.xyzx, cb0[0].yzwy
    r4.xyz = ((-(r4.xyzx))+(source[0].yzwy)).xyz;
    // 91: mul r5.xyz, r2.zzzz, r4.xyzx
    r5.xyz = ((r2.zzzz)*(r4.xyzx)).xyz;
    // 92: mad r4.xyz, r5.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r4.xyzx
    r4.xyz = ((r5.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r4.xyzx)).xyz;
    // 93: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 94: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 95: div r0.w, r4.z, r0.w
    r0.w = ((r4.zzzz)/(r0.wwww)).w;
    // 96: add r0.w, r0.w, cb0[7].z
    r0.w = ((r0.wwww)+(source[7].zzzz)).w;
    // 97: dp3 r2.z, v1.xyzx, v1.xyzx
    r2.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 98: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 99: mul r4.xyz, r2.zzzz, v1.xyzx
    r4.xyz = ((r2.zzzz)*(v1.xyzx)).xyz;
    // 100: dp3 r2.z, r4.xyzx, r3.xyzx
    r2.z = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 101: add r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)+(r2.zzzz)).w;
    // 102: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 103: add r0.w, r0.w, l(-0.500000)
    r0.w = ((r0.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 104: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 105: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: log r2.z, r0.w
    r2.z = (log2(r0.wwww)).z;
    // 107: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 108: mad r3.w, cb0[17].w, l(4.500000), l(0.500000)
    r3.w = ((source[17].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 109: mul r3.w, r3.w, cb0[18].x
    r3.w = ((r3.wwww)*(source[18].xxxx)).w;
    // 110: mul r3.w, r3.w, l(0.050000)
    r3.w = ((r3.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 111: mul r2.z, r2.z, r3.w
    r2.z = ((r2.zzzz)*(r3.wwww)).z;
    // 112: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 113: mul r2.z, r2.z, cb0[18].y
    r2.z = ((r2.zzzz)*(source[18].yyyy)).z;
    // 114: movc r0.w, r0.w, l(0), r2.z
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 115: mad r1.xyz, r0.wwww, r1.xyzx, r2.xywx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xywx)).xyz;
    // 116: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 117: add r0.w, -cb0[11].w, l(1.000000)
    r0.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r0.w, r0.w, cb0[20].x
    r0.w = ((r0.wwww)*(source[20].xxxx)).w;
    // 119: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 120: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 121: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r2.x, cb0[11].z, l(1.500000)
    r2.x = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 123: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 124: mad r0.w, r0.w, l(0.500000), cb0[11].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 125: mul r2.y, cb0[11].y, cb0[12].y
    r2.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 126: mul r5.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r5.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 127: frc r3.w, r5.x
    r3.w = (frac(r5.xxxx)).w;
    // 128: mul r5.y, r3.w, l(0.125000)
    r5.y = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 129: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 130: add r2.xy, r2.xyxx, r5.yzyy
    r2.xy = ((r2.xyxx)+(r5.yzyy)).xy;
    // 131: frc r3.w, cb0[11].x
    r3.w = (frac(source[11].xxxx)).w;
    // 132: add r4.w, -r3.w, cb0[11].x
    r4.w = ((-(r3.wwww))+(source[11].xxxx)).w;
    // 133: mul r2.z, r4.w, l(0.125000)
    r2.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 134: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = (LanceVANativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 136: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 137: mul r0.w, r3.w, r2.w
    r0.w = ((r3.wwww)*(r2.wwww)).w;
    // 138: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 139: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 140: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 141: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 142: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 143: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 144: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 145: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 146: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 147: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 148: mul r2.xyz, r2.xyzx, cb0[0].xxxx
    r2.xyz = ((r2.xyzx)*(source[0].xxxx)).xyz;
    // 149: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 150: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 151: mul r3.yzw, r3.yyyy, cb0[22].xxyz
    r3.yzw = ((r3.yyyy)*(source[22].xxyz)).yzw;
    // 152: mad r3.xyz, r3.xxxx, cb0[21].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[21].xyzx)+(r3.yzwy)).xyz;
    // 153: mul r3.xyz, r3.xyzx, cb0[23].wwww
    r3.xyz = ((r3.xyzx)*(source[23].wwww)).xyz;
    // 154: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 155: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 157: mad r1.xyz, r0.xyzx, cb0[23].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 159: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 160: mul r0.xy, v4.xyxx, cb0[19].yyyy
    r0.xy = ((v4.xyxx)*(source[19].yyyy)).xy;
    // 161: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 162: add r0.x, r0.x, -cb0[19].z
    r0.x = ((r0.xxxx)+(-(source[19].zzzz))).x;
    // 163: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 164: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 165: mul o0.w, r0.x, cb0[1].x
    output.w = ((r0.xxxx)*(source[1].xxxx)).w;
    return output;
}

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_z_pa_grad_01_1_tr: c4e3bde2576c2b46b3dd6ac9b693f4ed; selected map 154931681653007185176db8267f10aa3331472347eae2285d7d0b44dd35daf8.
float4 LanceVANative776(LANCE_VA_NATIVE_INPUT input)
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
    // 1: mad r0.x, cb0[2].x, v4.z, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.z, cb0[2].x
    r0.y = ((v4.zzzz)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_ringmaster_01_25_ad: 8a1e117816d4fd4d97ca1bb64457dd34; selected map 9edb9210536945be53e89959523e27c70a6c386821ad2a98095623b39523a05c.
float4 LanceVANative777(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[6u];
    source[2] = g_LanceVASourceMaterialParameters[5u];
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[4] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].xxxx,g_LanceVASourceMaterialParameters[3u].yyyy,1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[5].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[1u].zzzz)).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[1u].zzzz))).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[1u].wwww)).x;
    source[8].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[1u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[1u].wwww),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[9].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].xxxx))).x;
    source[9].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[10].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[10].y = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[10].z = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[10].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
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
    // 27: mad r1.x, r0.y, l(0.159155), l(0.500000)
    r1.x = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 28: mul r0.yz, cb0[4].xxyx, cb0[5].yyyy
    r0.yz = ((source[4].xxyx)*(source[5].yyyy)).yz;
    // 29: mul r0.w, v4.z, cb0[6].y
    r0.w = ((v4.zzzz)*(source[6].yyyy)).w;
    // 30: add r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)+(r0.xxxx)).z;
    // 31: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 32: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 33: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 34: lt r1.z, r0.x, l(0.000000)
    r1.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 35: movc r1.y, r1.z, l(0), r0.w
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 36: mad r0.yz, r1.xxyx, cb0[3].xxyx, r0.yyzy
    r0.yz = ((r1.xxyx)*(source[3].xxyx)+(r0.yyzy)).yz;
    // 37: mul r0.w, r1.x, cb0[5].w
    r0.w = ((r1.xxxx)*(source[5].wwww)).w;
    // 38: mul r1.x, r1.y, cb0[6].x
    r1.x = ((r1.yyyy)*(source[6].xxxx)).x;
    // 39: mul r0.yz, r0.yyzy, cb0[5].xxxx
    r0.yz = ((r0.yyzy)*(source[5].xxxx)).yz;
    // 40: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(-1.000000)
    r0.y = (LanceVANativeSample1((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 41: mad r0.z, -r0.x, cb0[7].w, l(1.000000)
    r0.z = ((-(r0.xxxx))*(source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: mad r0.x, -r0.x, cb0[9].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[9].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mul_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)*(source[10].zzzz))).x;
    // 44: mul_sat r0.z, r0.z, cb0[8].w
    r0.z = (saturate((r0.zzzz)*(source[8].wwww))).z;
    // 45: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 47: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 48: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 49: mul r0.x, r0.x, cb0[10].w
    r0.x = ((r0.xxxx)*(source[10].wwww)).x;
    // 50: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 51: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 52: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 53: mul_sat r0.x, r0.x, cb0[11].x
    r0.x = (saturate((r0.xxxx)*(source[11].xxxx))).x;
    // 54: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 55: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 57: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 58: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 59: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 60: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 61: mul r0.y, cb0[5].x, cb0[5].y
    r0.y = ((source[5].xxxx)*(source[5].yyyy)).y;
    // 62: mad r2.x, r0.y, cb0[5].z, r0.w
    r2.x = ((r0.yyyy)*(source[5].zzzz)+(r0.wwww)).x;
    // 63: mad r2.y, r0.y, cb0[6].z, r1.x
    r2.y = ((r0.yyyy)*(source[6].zzzz)+(r1.xxxx)).y;
    // 64: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t1.wxyz, s0, l(-1.000000)
    r0.yzw = (LanceVANativeSample0((r2.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 65: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 66: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 67: mad r0.yzw, cb0[6].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[6].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 68: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 69: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 70: mul r0.yzw, r0.yyzw, cb0[7].xxxx
    r0.yzw = ((r0.yyzw)*(source[7].xxxx)).yzw;
    // 71: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 72: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 73: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 74: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 75: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 76: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 77: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// fx_d_pa_atta_05_18_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 LanceVANative778(LANCE_VA_NATIVE_INPUT input)
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
// fx_j_pa_flare_01_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 LanceVANative779(LANCE_VA_NATIVE_INPUT input)
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
// lv_fat_ocastle_matte_01_mi: 38a74c088c723a4094ea288f0a1b4014; selected map 4bd09fe79b27410630f15a48876afaff90a0e467d3bdfe9cad80cc58c6dd2f58.
float4 LanceVANative781(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[0] = g_LanceVASourceMaterialParameters[6u];
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3] = g_LanceVASourceMaterialParameters[5u];
    source[4] = g_LanceVASourceMaterialParameters[4u];
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[0u].zzzz,g_LanceVASourceMaterialParameters[0u].wwww,1u);
    source[6].x = (g_LanceVASourceMaterialTime.xxxx).x;
    source[6].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[7].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].yyyy))).x;
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
    // 1: mul r0.xyz, cb0[6].yzwy, cb0[6].xxxx
    r0.xyz = ((source[6].yzwy)*(source[6].xxxx)).xyz;
    // 2: mul r1.xyzw, r0.xxyy, l(6.283185, 0.610000, 6.283185, 0.610000)
    r1.xyzw = ((r0.xxyy)*(float4(6.283185,0.610000,6.283185,0.610000))).xyzw;
    // 3: mul r0.xy, r0.zzzz, l(6.283185, 0.610000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(float4(6.283185,0.610000,0.000000,0.000000))).xy;
    // 4: sincos r2.x, r3.x, r1.z
    r2.x = (sin(r1.zzzz)).x; r3.x = (cos(r1.zzzz)).x;
    // 5: mad r0.z, r3.x, l(0.500000), l(0.500000)
    r0.z = ((r3.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 6: mad r0.w, r2.x, l(0.500000), l(0.500000)
    r0.w = ((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 7: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 8: frc r1.yz, r1.yywy
    r1.yz = (frac(r1.yywy)).yz;
    // 9: sincos r1.x, r2.x, r1.x
    r1.x = (sin(r1.xxxx)).x; r2.x = (cos(r1.xxxx)).x;
    // 10: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 11: mul r0.z, r0.z, r1.z
    r0.z = ((r0.zzzz)*(r1.zzzz)).z;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r2.yzw = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 13: add r0.w, r2.z, r2.y
    r0.w = ((r2.zzzz)+(r2.yyyy)).w;
    // 14: add r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)+(r0.wwww)).w;
    // 15: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 16: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xyzw = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 18: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 19: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 20: mul r0.z, r0.z, r3.y
    r0.z = ((r0.zzzz)*(r3.yyyy)).z;
    // 21: mul r4.xyz, cb0[2].xyzx, cb0[2].wwww
    r4.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 22: mul r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = ((r0.zzzz)*(r4.xyzx)).xyz;
    // 23: mad r0.z, r2.x, l(0.500000), l(0.500000)
    r0.z = ((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 24: mad r0.w, r1.x, l(0.500000), l(0.500000)
    r0.w = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 25: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 26: mul r0.z, r1.y, r0.z
    r0.z = ((r1.yyyy)*(r0.zzzz)).z;
    // 27: mul r0.z, r3.x, r0.z
    r0.z = ((r3.xxxx)*(r0.zzzz)).z;
    // 28: mul r1.xyz, cb0[1].xyzx, cb0[1].wwww
    r1.xyz = ((source[1].xyzx)*(source[1].wwww)).xyz;
    // 29: mad r1.xyz, r0.zzzz, r1.xyzx, r4.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 30: sincos r0.x, r2.x, r0.x
    r0.x = (sin(r0.xxxx)).x; r2.x = (cos(r0.xxxx)).x;
    // 31: frc r0.y, r0.y
    r0.y = (frac(r0.yyyy)).y;
    // 32: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: mad r0.z, r2.x, l(0.500000), l(0.500000)
    r0.z = ((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 34: mad r0.x, r0.x, l(0.500000), l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 35: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 36: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 37: mul r0.x, r3.z, r0.x
    r0.x = ((r3.zzzz)*(r0.xxxx)).x;
    // 38: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 39: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 40: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 41: dp3 r0.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 42: add r3.xyz, -r2.yzwy, r0.wwww
    r3.xyz = ((-(r2.yzwy))+(r0.wwww)).xyz;
    // 43: mad r2.xyz, cb0[7].xxxx, r3.xyzx, r2.yzwy
    r2.xyz = ((source[7].xxxx)*(r3.xyzx)+(r2.yzwy)).xyz;
    // 44: mad r0.xyz, r2.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 45: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 46: mul r1.xy, r1.xyxx, cb0[5].xyxx
    r1.xy = ((r1.xyxx)*(source[5].xyxx)).xy;
    // 47: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 48: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 49: mad_sat r0.w, -r0.w, cb0[8].y, l(1.000000)
    r0.w = (saturate((-(r0.wwww))*(source[8].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 50: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 51: mad o0.xyz, r0.wwww, r0.xyzx, cb0[0].xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)+(source[0].xyzx)).xyz;
    // 52: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_lut_stone_basic02a_mi_rain: 599d51a203649a49b997057baea2bbcb; selected map 7b25eef2018f612f792b6cca0c6264434d12626126470dff052e2e6581eae6d7.
float4 LanceVANative782(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[9].xxxx
    r1.xyz = ((r1.xyzx)*(source[9].xxxx)).xyz;
    // 54: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 55: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 56: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 57: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 58: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 60: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 61: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 65: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 68: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 69: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 70: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 71: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 74: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 75: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 76: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 77: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 78: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 80: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_atm_etc_ruins11a_sm_mi_rain: b9f1a2c03fedbf46bc76d418220fd2a8; selected map da66252f2703f6e40120f6a6e586b9f1e3a1d4a0289dbbb1f53dc21619b90c38.
float4 LanceVANative783(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].wwww,g_LanceVASourceMaterialParameters[3u].xxxx,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].zzzz),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].zzzz)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 54: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 55: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 56: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 58: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 59: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 60: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 61: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 62: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 63: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 66: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 67: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 68: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 69: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 72: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 73: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 74: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 75: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 76: add r6.xyz, -r1.xyzx, r0.wwww
    r6.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 77: mad r1.xyz, cb0[9].yyyy, r6.xyzx, r1.xyzx
    r1.xyz = ((source[9].yyyy)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 78: mul r1.xyz, r1.xyzx, cb0[9].zzzz
    r1.xyz = ((r1.xyzx)*(source[9].zzzz)).xyz;
    // 79: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 80: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 81: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 83: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 85: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_kur_stone_rock03d_mi_rain: 599d51a203649a49b997057baea2bbcb; selected map 7b25eef2018f612f792b6cca0c6264434d12626126470dff052e2e6581eae6d7.
float4 LanceVANative784(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[9].xxxx
    r1.xyz = ((r1.xyzx)*(source[9].xxxx)).xyz;
    // 54: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 55: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 56: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 57: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 58: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 60: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 61: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 65: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 68: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 69: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 70: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 71: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 74: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 75: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 76: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 77: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 78: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 80: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_lut_pofbattle_lance01a02_mi_rain: b9f1a2c03fedbf46bc76d418220fd2a8; selected map da66252f2703f6e40120f6a6e586b9f1e3a1d4a0289dbbb1f53dc21619b90c38.
float4 LanceVANative785(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].wwww,g_LanceVASourceMaterialParameters[3u].xxxx,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].zzzz),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].zzzz)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 54: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 55: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 56: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 58: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 59: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 60: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 61: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 62: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 63: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 66: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 67: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 68: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 69: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 72: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 73: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 74: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 75: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 76: add r6.xyz, -r1.xyzx, r0.wwww
    r6.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 77: mad r1.xyz, cb0[9].yyyy, r6.xyzx, r1.xyzx
    r1.xyz = ((source[9].yyyy)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 78: mul r1.xyz, r1.xyzx, cb0[9].zzzz
    r1.xyz = ((r1.xyzx)*(source[9].zzzz)).xyz;
    // 79: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 80: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 81: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 83: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 85: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_kur_stone_rock01a_mi_hhk: b29b1d29a2878e47ace0bf77236d5e8f; selected map f106257aebd19afb920109886e2d8648c09380fe24570774c78208f9befadc9a.
float4 LanceVANative786(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[3]=float4(input.skyUpperColor,0.f);
    source[4]=float4(input.skyLowerColor,0.f);
    source[5]=float4(input.ambientColor,input.skyIntensity);
    source[0] = g_LanceVASourceMaterialParameters[2u];
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: mul r0.xy, r0.xyxx, cb0[2].xxxx
    r0.xy = ((r0.xyxx)*(source[2].xxxx)).xy;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
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
    // 12: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 13: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 16: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 19: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 20: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 22: mul r1.yzw, r1.yyyy, cb0[4].xxyz
    r1.yzw = ((r1.yyyy)*(source[4].xxyz)).yzw;
    // 23: mad r1.xyz, r1.xxxx, cb0[3].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[3].xyzx)+(r1.yzwy)).xyz;
    // 24: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 26: mul r3.xyz, cb0[1].xyzx, cb0[2].yyyy
    r3.xyz = ((source[1].xyzx)*(source[2].yyyy)).xyz;
    // 27: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 28: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 29: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[0].xyzx)).xyz;
    // 30: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 32: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r3.xyzx
    output.xyz = ((r2.xyzx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // 34: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_lut_stone_basic04a_mi_rain: 599d51a203649a49b997057baea2bbcb; selected map 7b25eef2018f612f792b6cca0c6264434d12626126470dff052e2e6581eae6d7.
float4 LanceVANative787(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[9].xxxx
    r1.xyz = ((r1.xyzx)*(source[9].xxxx)).xyz;
    // 54: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 55: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 56: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 57: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 58: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 60: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 61: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 65: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 68: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 69: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 70: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 71: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 74: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 75: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 76: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 77: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 78: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 80: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_lut_stone_basic05a_mi_rain: 599d51a203649a49b997057baea2bbcb; selected map 7b25eef2018f612f792b6cca0c6264434d12626126470dff052e2e6581eae6d7.
float4 LanceVANative788(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[9].xxxx
    r1.xyz = ((r1.xyzx)*(source[9].xxxx)).xyz;
    // 54: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 55: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 56: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 57: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 58: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 60: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 61: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 65: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 68: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 69: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 70: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 71: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 74: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 75: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 76: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 77: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 78: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 80: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_lut_stone_basic02b_mi_rain: 599d51a203649a49b997057baea2bbcb; selected map 7b25eef2018f612f792b6cca0c6264434d12626126470dff052e2e6581eae6d7.
float4 LanceVANative789(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[9].xxxx
    r1.xyz = ((r1.xyzx)*(source[9].xxxx)).xyz;
    // 54: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 55: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 56: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 57: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 58: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 60: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 61: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 65: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 68: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 69: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 70: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 71: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 74: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 75: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 76: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 77: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 78: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 80: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_lut_stone_basic03a_mi_rain: b9f1a2c03fedbf46bc76d418220fd2a8; selected map da66252f2703f6e40120f6a6e586b9f1e3a1d4a0289dbbb1f53dc21619b90c38.
float4 LanceVANative790(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].wwww,g_LanceVASourceMaterialParameters[3u].xxxx,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].zzzz),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].zzzz)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 54: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 55: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 56: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 58: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 59: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 60: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 61: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 62: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 63: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 66: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 67: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 68: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 69: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 72: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 73: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 74: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 75: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 76: add r6.xyz, -r1.xyzx, r0.wwww
    r6.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 77: mad r1.xyz, cb0[9].yyyy, r6.xyzx, r1.xyzx
    r1.xyz = ((source[9].yyyy)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 78: mul r1.xyz, r1.xyzx, cb0[9].zzzz
    r1.xyz = ((r1.xyzx)*(source[9].zzzz)).xyz;
    // 79: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 80: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 81: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 83: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 85: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_fat_caves_wall01_mi_ksr_rain: 599d51a203649a49b997057baea2bbcb; selected map 7b25eef2018f612f792b6cca0c6264434d12626126470dff052e2e6581eae6d7.
float4 LanceVANative791(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].wwww,g_LanceVASourceMaterialParameters[4u].xxxx,1u);
    source[3] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[4] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy),1u);
    source[5].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].x = ((g_LanceVASourceMaterialTime.xxxx*g_LanceVASourceMaterialParameters[2u].yyyy)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.tangentUp,0.f); // native texcoord7
    float4 v7 = float4(input.sourceWorldPosition,1.f); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xyzw, v7.xzyz, cb0[0].xzyz
    r0.xyzw = ((v7.xzyz)+(source[0].xzyz)).xyzw;
    // 2: mul r1.xy, r0.xzxx, cb0[7].wwww
    r1.xy = ((r0.xzxx)*(source[7].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xy = (LanceVANativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 6: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 7: mul r2.xyz, r0.xzwx, cb0[5].zzzz
    r2.xyz = ((r0.xzwx)*(source[5].zzzz)).xyz;
    // 8: mad r0.xyzw, r0.xyzw, cb0[3].xyxy, cb0[4].xyxy
    r0.xyzw = ((r0.xyzw)*(source[3].xyxy)+(source[4].xyxy)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r1.w = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 10: mul r2.w, r1.w, cb0[8].y
    r2.w = ((r1.wwww)*(source[8].yyyy)).w;
    // 11: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 12: mad r2.w, cb0[8].x, r1.z, r2.w
    r2.w = ((source[8].xxxx)*(r1.zzzz)+(r2.wwww)).w;
    // 13: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 14: mad r1.z, cb0[7].z, cb0[6].w, r2.w
    r1.z = ((source[7].zzzz)*(source[6].wwww)+(r2.wwww)).z;
    // 15: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 16: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 17: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 20: mul r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)*(r1.zzzz)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 23: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: add r0.zw, -r0.xxxy, r0.zzzw
    r0.zw = ((-(r0.xxxy))+(r0.zzzw)).zw;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 30: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 31: mul r4.xyz, r1.zzzz, v0.xyzx
    r4.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 32: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 33: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 34: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 35: mov r4.w, r5.z
    r4.w = (r5.zzzz).w;
    // 36: dp2 r1.z, r4.zwzz, r4.zwzz
    r1.z = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).z;
    // 37: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 38: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 39: mad r0.xy, |r1.zzzz|, r0.zwzz, r0.xyxx
    r0.xy = ((abs(r1.zzzz))*(r0.zwzz)+(r0.xyxx)).xy;
    // 40: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.yzyy, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xzxx, t0.yzwx, s0, l(0.000000)
    r0.w = (LanceVANativeSample0((r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 44: mad r0.z, |r1.z|, r0.z, r0.w
    r0.z = ((abs(r1.zzzz))*(r0.zzzz)+(r0.wwww)).z;
    // 45: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 46: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, -r0.xxxy
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(-(r0.xxxy))).zw;
    // 48: mov_sat r1.x, r3.z
    r1.x = (saturate(r3.zzzz)).x;
    // 49: mad r0.xy, r1.xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((r1.xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 50: mad r0.xy, v4.xyxx, cb0[2].xyxx, r0.xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)+(r0.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s3, l(0.000000)
    r0.zw = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (LanceVANativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[9].xxxx
    r1.xyz = ((r1.xyzx)*(source[9].xxxx)).xyz;
    // 54: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 55: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 56: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 57: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 58: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 60: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 61: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 65: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 68: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 69: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 70: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 71: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 74: mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // 75: mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // 76: mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // 77: mad r6.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r6.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 78: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 80: mad o0.xyz, r1.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r1.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 82: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_rhd_zenail_floor01_mi_ksr: 982a797d2c642a40a5229388cf148df2; selected map bbc92e408f3ddc00778068f7b16b46ea6c023723f376ba0658ac539d1b1489aa.
float4 LanceVANative792(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[9]=float4(input.skyUpperColor,0.f);
    source[10]=float4(input.skyLowerColor,0.f);
    source[11]=float4(input.ambientColor,input.skyIntensity);
    source[0] = g_LanceVASourceMaterialParameters[9u];
    source[1] = g_LanceVASourceMaterialParameters[8u];
    source[2] = g_LanceVASourceMaterialParameters[11u];
    source[3] = g_LanceVASourceMaterialParameters[4u];
    source[4] = g_LanceVASourceMaterialParameters[5u];
    source[5].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[5].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].w = ((g_LanceVASourceMaterialParameters[0u].xxxx*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[7].x = (sin(((g_LanceVASourceMaterialParameters[0u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[7].y = ((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialParameters[0u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[7].z = (((float4(1.5, 0.0, 0.0, 0.0)+sin(((g_LanceVASourceMaterialParameters[0u].yyyy*g_LanceVASourceMaterialTime.xxxx)*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.400000006, 0.0, 0.0, 0.0))).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[8].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) clip(-1.f);
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 9: mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // 10: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 18: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 21: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 22: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 25: mad_sat r1.w, r0.w, cb0[5].w, -cb0[6].x
    r1.w = (saturate((r0.wwww)*(source[5].wwww)+(-(source[6].xxxx)))).w;
    // 26: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 27: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: mul r2.x, r2.x, cb0[6].y
    r2.x = ((r2.xxxx)*(source[6].yyyy)).x;
    // 29: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 30: mul r2.xyz, r2.xxxx, cb0[2].xyzx
    r2.xyz = ((r2.xxxx)*(source[2].xyzx)).xyz;
    // 31: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 32: add r2.xyz, r2.xyzx, -cb0[2].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[2].xyzx))).xyz;
    // 33: mad r2.xyz, cb0[2].wwww, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((source[2].wwww)*(r2.xyzx)+(source[2].xyzx)).xyz;
    // 34: mul r3.xyz, cb0[3].xyzx, cb0[6].wwww
    r3.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 35: mul r3.xyz, r3.xyzx, cb0[7].zzzz
    r3.xyz = ((r3.xyzx)*(source[7].zzzz)).xyz;
    // 36: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 37: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 38: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 39: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 40: mul r3.xyz, r3.xyzx, cb0[7].wwww
    r3.xyz = ((r3.xyzx)*(source[7].wwww)).xyz;
    // 41: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 42: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 44: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 45: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: mul r3.xyz, r1.wwww, cb0[1].xyzx
    r3.xyz = ((r1.wwww)*(source[1].xyzx)).xyz;
    // 49: movc r3.xyz, r0.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 50: mad r2.xyz, r2.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r3.xyzx
    r2.xyz = ((r2.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r3.xyzx)).xyz;
    // 51: add r2.xyz, r2.xyzx, cb0[0].xyzx
    r2.xyz = ((r2.xyzx)+(source[0].xyzx)).xyz;
    // 52: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 54: mad r0.xyz, cb0[8].yyyy, r3.xyzx, r0.xyzx
    r0.xyz = ((source[8].yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 55: mul r3.xyz, cb0[4].xyzx, cb0[8].zzzz
    r3.xyz = ((source[4].xyzx)*(source[8].zzzz)).xyz;
    // 56: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 57: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 58: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 59: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 60: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 61: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 62: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 63: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 65: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 66: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 67: mul r3.yzw, r3.yyyy, cb0[10].xxyz
    r3.yzw = ((r3.yyyy)*(source[10].xxyz)).yzw;
    // 68: mad r3.xyz, r3.xxxx, cb0[9].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[9].xyzx)+(r3.yzwy)).xyz;
    // 69: mul r3.xyz, r3.xyzx, cb0[11].wwww
    r3.xyz = ((r3.xyzx)*(source[11].wwww)).xyz;
    // 70: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 71: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 73: mad o0.xyz, r0.xyzx, cb0[11].xyzx, r2.xyzx
    output.xyz = ((r0.xyzx)*(source[11].xyzx)+(r2.xyzx)).xyz;
    // 75: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_fat_caves_wall01b_mi_ksr: 9becf638b83d6542aef7b19bed3bb609; selected map 7743f9c290425b4505aa55f781beef781407d06b5dec99f4eb1fe19f39c670b2.
float4 LanceVANative793(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[3]=float4(input.skyUpperColor,0.f);
    source[4]=float4(input.skyLowerColor,0.f);
    source[5]=float4(input.ambientColor,input.skyIntensity);
    source[0] = g_LanceVASourceMaterialParameters[2u];
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 8: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 9: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 10: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 12: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 13: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 14: dp3 r0.x, r0.xyzx, r1.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 17: mul r0.yzw, r0.yyyy, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(source[4].xxyz)).yzw;
    // 18: mad r0.xyz, r0.xxxx, cb0[3].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[3].xyzx)+(r0.yzwy)).xyz;
    // 19: mul r0.xyz, r0.xyzx, cb0[5].wwww
    r0.xyz = ((r0.xyzx)*(source[5].wwww)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 21: mul r3.xyz, cb0[1].xyzx, cb0[2].xxxx
    r3.xyz = ((source[1].xyzx)*(source[2].xxxx)).xyz;
    // 22: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 23: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 24: mad r3.xyz, r0.xyzx, r2.xyzx, cb0[0].xyzx
    r3.xyz = ((r0.xyzx)*(r2.xyzx)+(source[0].xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 27: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r3.xyzx
    output.xyz = ((r2.xyzx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // 29: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_fat_caves_wall01a_mi_ksr: ce44998f541797478b813791892af61d; selected map d9ac4a3ace351a156b6288572c6d912d7df05248a968d2e697ba9a342114ca1e.
float4 LanceVANative794(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[3]=float4(input.skyUpperColor,0.f);
    source[4]=float4(input.skyLowerColor,0.f);
    source[5]=float4(input.ambientColor,input.skyIntensity);
    source[0] = g_LanceVASourceMaterialParameters[2u];
    source[1] = g_LanceVASourceMaterialParameters[1u];
    source[2].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) clip(-1.f);
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 7: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 8: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 19: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 22: mul r1.yzw, r1.yyyy, cb0[4].xxyz
    r1.yzw = ((r1.yyyy)*(source[4].xxyz)).yzw;
    // 23: mad r1.xyz, r1.xxxx, cb0[3].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[3].xyzx)+(r1.yzwy)).xyz;
    // 24: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 25: mul r3.xyz, cb0[1].xyzx, cb0[2].xxxx
    r3.xyz = ((source[1].xyzx)*(source[2].xxxx)).xyz;
    // 26: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 27: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 28: mad r3.xyz, r1.xyzx, r0.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r0.xyzx)+(source[0].xyzx)).xyz;
    // 29: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 31: mad o0.xyz, r0.xyzx, cb0[5].xyzx, r3.xyzx
    output.xyz = ((r0.xyzx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // 33: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_gdogods_swordfloor02_mi_alchemy: ef961e393dfade4c84fd1b0e773ee4ca; selected map b8f1239bcc910ca8da66fe2f93e44b9117fa6da829dfb44710ad316d159fc296.
float4 LanceVANative795(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[17]=float4(input.skyUpperColor,0.f);
    source[18]=float4(input.skyLowerColor,0.f);
    source[19]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = g_LanceVASourceMaterialParameters[4u];
    source[3] = g_LanceVASourceMaterialParameters[3u];
    source[4].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[6].x = (clamp(g_LanceVASourceMaterialParameters[2u].xxxx,float4(0.0, 0.0, 0.0, 0.0),float4(100.0, 0.0, 0.0, 0.0))).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 3: mul r0.xy, r0.yxyy, cb0[6].xzxx
    r0.xy = ((r0.yxyy)*(source[6].xzxx)).xy;
    // 4: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 5: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 6: mul r0.w, r0.w, cb0[5].y
    r0.w = ((r0.wwww)*(source[5].yyyy)).w;
    // 7: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 8: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 9: min r0.w, r0.z, l(1.000000)
    r0.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: mul_sat r1.w, r0.z, cb2[3].w
    r1.w = (saturate((r0.zzzz)*(passValues[3].wwww))).w;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r3.xyz, cb0[3].xyzx, cb0[4].yyyy
    r3.xyz = ((source[3].xyzx)*(source[4].yyyy)).xyz;
    // 13: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 14: mul r3.xyz, r2.xyzx, cb0[4].zzzz
    r3.xyz = ((r2.xyzx)*(source[4].zzzz)).xyz;
    // 15: mad r2.xyz, cb0[4].wwww, r2.xyzx, -r3.xyzx
    r2.xyz = ((source[4].wwww)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // 16: mad r2.xyz, r0.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 17: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 18: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 19: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 20: mad r3.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 21: mad r4.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 22: mad r5.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 23: log r0.zw, |r0.xxxy|
    r0.zw = (log2(abs(r0.xxxy))).zw;
    // 24: lt r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((abs(r0.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 25: mul r0.zw, r0.zzzw, cb0[6].yyyw
    r0.zw = ((r0.zzzw)*(source[6].yyyw)).zw;
    // 26: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 27: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 29: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 30: min r1.z, r0.x, l(1.000000)
    r1.z = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: mad r0.xzw, r0.yyyy, r4.xxyz, r5.xxyz
    r0.xzw = ((r0.yyyy)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 32: mad r0.xzw, r0.xxzw, r0.yyyy, r3.xxyz
    r0.xzw = ((r0.xxzw)*(r0.yyyy)+(r3.xxyz)).xzw;
    // 33: mul r0.xzw, r0.yyyy, r0.xxzw
    r0.xzw = ((r0.yyyy)*(r0.xxzw)).xzw;
    // 34: max r0.xzw, r0.xxzw, r0.yyyy
    r0.xzw = (max(r0.xxzw,r0.yyyy)).xzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 36: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: dp2 r3.x, r1.xyxx, r1.xyxx
    r3.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 38: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 39: mul r4.xy, r1.xyxx, v2.wwww
    r4.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 40: add r1.x, -r3.x, l(1.000000)
    r1.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 45: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 46: div r3.xyz, r4.xyzx, r1.xxxx
    r3.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 47: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 48: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 49: mul r4.xyz, r1.xxxx, r3.xyzx
    r4.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 50: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 51: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 52: mul r5.xyz, r1.xxxx, v6.xyzx
    r5.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // 53: dp3 r1.x, r5.xyzx, r4.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 54: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 55: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 56: mul r6.xyz, r1.yyyy, cb0[18].xyzx
    r6.xyz = ((r1.yyyy)*(source[18].xyzx)).xyz;
    // 57: mad r6.xyz, r1.xxxx, cb0[17].xyzx, r6.xyzx
    r6.xyz = ((r1.xxxx)*(source[17].xyzx)+(r6.xyzx)).xyz;
    // 58: mul r6.xyz, r6.xyzx, cb0[19].wwww
    r6.xyz = ((r6.xyzx)*(source[19].wwww)).xyz;
    // 59: mul r6.xyz, r2.xyzx, r6.xyzx
    r6.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 60: mul r0.xzw, r0.xxzw, r6.xxyz
    r0.xzw = ((r0.xxzw)*(r6.xxyz)).xzw;
    // 61: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 62: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 63: mul r6.xyz, r1.xxxx, v1.xyzx
    r6.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 64: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 65: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 66: mul r7.xyz, r1.xxxx, v0.xyzx
    r7.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 67: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 68: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 69: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 70: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 71: dp3 r9.x, r7.xyzx, r4.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 72: dp2 r10.z, r9.xyxx, cb0[8].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[8].xyxx).xy).xxxx).z;
    // 73: mul r1.xy, cb0[8].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[8].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 74: dp2 r10.x, r9.xyxx, r1.xyxx
    r10.x = (dot((r9.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 75: dp3 r10.y, r6.xyzx, r4.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 76: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 77: dp4 r11.x, cb0[9].xyzw, r10.xyzw
    r11.x = (dot((source[9].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 78: dp4 r11.y, cb0[10].xyzw, r10.xyzw
    r11.y = (dot((source[10].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 79: dp4 r11.z, cb0[11].xyzw, r10.xyzw
    r11.z = (dot((source[11].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 80: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 81: dp4 r13.x, cb0[12].xyzw, r12.xyzw
    r13.x = (dot((source[12].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 82: dp4 r13.y, cb0[13].xyzw, r12.xyzw
    r13.y = (dot((source[13].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 83: dp4 r13.z, cb0[14].xyzw, r12.xyzw
    r13.z = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 84: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 85: mul r3.w, r10.y, r10.y
    r3.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 86: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 87: mad r3.w, r10.x, r10.x, -r3.w
    r3.w = ((r10.xxxx)*(r10.xxxx)+(-(r3.wwww))).w;
    // 88: mad r10.xyz, cb0[15].xyzx, r3.wwww, r11.xyzx
    r10.xyz = ((source[15].xyzx)*(r3.wwww)+(r11.xyzx)).xyz;
    // 89: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: mul r10.xyz, r10.xyzx, cb0[7].xyzx
    r10.xyz = ((r10.xyzx)*(source[7].xyzx)).xyz;
    // 91: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[7].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[7].wwww)).xyz;
    // 92: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 93: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 94: mul r11.xyz, r3.wwww, v5.xyzx
    r11.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 95: dp3 r3.w, r4.xyzx, r11.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 96: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 97: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 98: deriv_rtx_coarse r12.x, r3.w
    r12.x = (ddx_coarse(r3.wwww)).x;
    // 99: deriv_rty_coarse r12.y, r3.w
    r12.y = (ddy_coarse(r3.wwww)).y;
    // 100: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: dp2 r4.w, r12.xyxx, r12.xyxx
    r4.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 102: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 103: mad_sat r12.y, r4.w, l(0.300000), r1.z
    r12.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.zzzz))).y;
    // 105: add r1.z, -r12.y, l(1.000000)
    r1.z = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: mov_sat r2.w, cb0[5].z
    r2.w = (saturate(source[5].zzzz)).w;
    // 107: mad r13.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r13.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 108: mul r4.w, r2.w, l(0.080000)
    r4.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 110: mad r13.xyz, r1.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r1.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 111: max r14.xyz, r1.zzzz, r13.xyzx
    r14.xyz = (max(r1.zzzz,r13.xyzx)).xyz;
    // 112: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 113: mul_sat r1.z, r13.y, l(50.000000)
    r1.z = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 114: mul r14.xyz, r1.zzzz, r14.xyzx
    r14.xyz = ((r1.zzzz)*(r14.xyzx)).xyz;
    // 115: add r1.z, r4.z, l(1.000000)
    r1.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 116: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 117: add_sat r12.x, -r1.z, r3.w
    r12.x = (saturate((-(r1.zzzz))+(r3.wwww))).x;
    // 118: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t3.zwxy, s4 (unbound native scene environment contribution)
    r12.zw = (float4(0.f,0.f,0.f,0.f)).zw;
    // 119: add r1.z, r0.y, r12.x
    r1.z = ((r0.yyyy)+(r12.xxxx)).z;
    // 120: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 121: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 122: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 123: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.wwww)).w;
    // 124: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 125: mad r12.xzw, r13.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 126: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 127: mad r13.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 128: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 129: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 130: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 131: mul r0.xzw, r0.xxzw, r10.xxyz
    r0.xzw = ((r0.xxzw)*(r10.xxyz)).xzw;
    // 132: mad r0.xzw, -r0.xxzw, r1.wwww, r0.xxzw
    r0.xzw = ((-(r0.xxzw))*(r1.wwww)+(r0.xxzw)).xzw;
    // 133: mul r1.w, r12.y, l(5.000000)
    r1.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 134: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 135: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 136: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 137: add r1.z, r0.y, r1.z
    r1.z = ((r0.yyyy)+(r1.zzzz)).z;
    // 139: add_sat r0.y, r1.z, l(-1.000000)
    r0.y = (saturate((r1.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 140: dp3 r7.x, r7.xyzx, r4.xyzx
    r7.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 141: dp3 r7.y, r8.xyzx, r4.xyzx
    r7.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 142: dp2 r1.x, r7.xyxx, r1.xyxx
    r1.x = (dot((r7.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 143: dp2 r1.z, r7.xyxx, cb0[8].xyxx
    r1.z = (dot((r7.xyxx).xy,(source[8].xyxx).xy).xxxx).z;
    // 144: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 145: dp3 r2.w, r5.xyzx, r4.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 146: mad r4.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 147: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 148: sample_l_indexable(texturecube)(float,float,float,float) r1.xyzw, r1.xyzx, t4.xyzw, s3, r1.w (unbound native scene environment contribution)
    r1.xyzw = (float4(0.f,0.f,0.f,0.f)).xyzw;
    // 149: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 150: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 151: mad r1.xyz, r1.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[7].wwww)).xyz;
    // 152: mad r1.w, r0.y, r13.x, r13.y
    r1.w = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).w;
    // 153: mad r1.w, r1.w, r0.y, r13.z
    r1.w = ((r1.wwww)*(r0.yyyy)+(r13.zzzz)).w;
    // 154: mul r1.w, r0.y, r1.w
    r1.w = ((r0.yyyy)*(r1.wwww)).w;
    // 155: max r0.y, r0.y, r1.w
    r0.y = (max(r0.yyyy,r1.wwww)).y;
    // 156: mul r4.yzw, r4.yyyy, cb0[18].xxyz
    r4.yzw = ((r4.yyyy)*(source[18].xxyz)).yzw;
    // 157: mad r4.xyz, cb0[17].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[17].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 158: mul r4.xyz, r4.xyzx, cb0[19].wwww
    r4.xyz = ((r4.xyzx)*(source[19].wwww)).xyz;
    // 159: mul r4.xyz, r0.yyyy, r4.xyzx
    r4.xyz = ((r0.yyyy)*(r4.xyzx)).xyz;
    // 160: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 161: mad r0.xyz, r1.xyzx, r12.xzwx, r0.xzwx
    r0.xyz = ((r1.xyzx)*(r12.xzwx)+(r0.xzwx)).xyz;
    // 162: mul r1.xyz, r12.xzwx, r1.xyzx
    r1.xyz = ((r12.xzwx)*(r1.xyzx)).xyz;
    // 164: dp3 r0.w, r3.xyzx, r11.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 165: add r1.x, -|r11.z|, l(1.000000)
    r1.x = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 166: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 168: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 169: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 170: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 171: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 172: mul r1.yzw, r0.wwww, cb0[2].xxyz
    r1.yzw = ((r0.wwww)*(source[2].xxyz)).yzw;
    // 173: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 174: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 175: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 177: mad o0.xyz, r2.xyzx, cb0[19].xyzx, r1.xyzx
    output.xyz = ((r2.xyzx)*(source[19].xyzx)+(r1.xyzx)).xyz;
    // 178: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_gdogods_swordfloor02a_mi_alchemy: ef961e393dfade4c84fd1b0e773ee4ca; selected map b8f1239bcc910ca8da66fe2f93e44b9117fa6da829dfb44710ad316d159fc296.
float4 LanceVANative796(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[17]=float4(input.skyUpperColor,0.f);
    source[18]=float4(input.skyLowerColor,0.f);
    source[19]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = g_LanceVASourceMaterialParameters[4u];
    source[3] = g_LanceVASourceMaterialParameters[3u];
    source[4].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[6].x = (clamp(g_LanceVASourceMaterialParameters[2u].xxxx,float4(0.0, 0.0, 0.0, 0.0),float4(100.0, 0.0, 0.0, 0.0))).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 3: mul r0.xy, r0.yxyy, cb0[6].xzxx
    r0.xy = ((r0.yxyy)*(source[6].xzxx)).xy;
    // 4: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 5: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 6: mul r0.w, r0.w, cb0[5].y
    r0.w = ((r0.wwww)*(source[5].yyyy)).w;
    // 7: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 8: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 9: min r0.w, r0.z, l(1.000000)
    r0.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: mul_sat r1.w, r0.z, cb2[3].w
    r1.w = (saturate((r0.zzzz)*(passValues[3].wwww))).w;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r3.xyz, cb0[3].xyzx, cb0[4].yyyy
    r3.xyz = ((source[3].xyzx)*(source[4].yyyy)).xyz;
    // 13: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 14: mul r3.xyz, r2.xyzx, cb0[4].zzzz
    r3.xyz = ((r2.xyzx)*(source[4].zzzz)).xyz;
    // 15: mad r2.xyz, cb0[4].wwww, r2.xyzx, -r3.xyzx
    r2.xyz = ((source[4].wwww)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // 16: mad r2.xyz, r0.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 17: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 18: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 19: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 20: mad r3.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 21: mad r4.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 22: mad r5.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 23: log r0.zw, |r0.xxxy|
    r0.zw = (log2(abs(r0.xxxy))).zw;
    // 24: lt r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((abs(r0.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 25: mul r0.zw, r0.zzzw, cb0[6].yyyw
    r0.zw = ((r0.zzzw)*(source[6].yyyw)).zw;
    // 26: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 27: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 29: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 30: min r1.z, r0.x, l(1.000000)
    r1.z = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: mad r0.xzw, r0.yyyy, r4.xxyz, r5.xxyz
    r0.xzw = ((r0.yyyy)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 32: mad r0.xzw, r0.xxzw, r0.yyyy, r3.xxyz
    r0.xzw = ((r0.xxzw)*(r0.yyyy)+(r3.xxyz)).xzw;
    // 33: mul r0.xzw, r0.yyyy, r0.xxzw
    r0.xzw = ((r0.yyyy)*(r0.xxzw)).xzw;
    // 34: max r0.xzw, r0.xxzw, r0.yyyy
    r0.xzw = (max(r0.xxzw,r0.yyyy)).xzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 36: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: dp2 r3.x, r1.xyxx, r1.xyxx
    r3.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 38: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 39: mul r4.xy, r1.xyxx, v2.wwww
    r4.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 40: add r1.x, -r3.x, l(1.000000)
    r1.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 45: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 46: div r3.xyz, r4.xyzx, r1.xxxx
    r3.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 47: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 48: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 49: mul r4.xyz, r1.xxxx, r3.xyzx
    r4.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 50: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 51: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 52: mul r5.xyz, r1.xxxx, v6.xyzx
    r5.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // 53: dp3 r1.x, r5.xyzx, r4.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 54: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 55: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 56: mul r6.xyz, r1.yyyy, cb0[18].xyzx
    r6.xyz = ((r1.yyyy)*(source[18].xyzx)).xyz;
    // 57: mad r6.xyz, r1.xxxx, cb0[17].xyzx, r6.xyzx
    r6.xyz = ((r1.xxxx)*(source[17].xyzx)+(r6.xyzx)).xyz;
    // 58: mul r6.xyz, r6.xyzx, cb0[19].wwww
    r6.xyz = ((r6.xyzx)*(source[19].wwww)).xyz;
    // 59: mul r6.xyz, r2.xyzx, r6.xyzx
    r6.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 60: mul r0.xzw, r0.xxzw, r6.xxyz
    r0.xzw = ((r0.xxzw)*(r6.xxyz)).xzw;
    // 61: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 62: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 63: mul r6.xyz, r1.xxxx, v1.xyzx
    r6.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 64: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 65: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 66: mul r7.xyz, r1.xxxx, v0.xyzx
    r7.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 67: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 68: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 69: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 70: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 71: dp3 r9.x, r7.xyzx, r4.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 72: dp2 r10.z, r9.xyxx, cb0[8].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[8].xyxx).xy).xxxx).z;
    // 73: mul r1.xy, cb0[8].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[8].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 74: dp2 r10.x, r9.xyxx, r1.xyxx
    r10.x = (dot((r9.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 75: dp3 r10.y, r6.xyzx, r4.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 76: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 77: dp4 r11.x, cb0[9].xyzw, r10.xyzw
    r11.x = (dot((source[9].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 78: dp4 r11.y, cb0[10].xyzw, r10.xyzw
    r11.y = (dot((source[10].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 79: dp4 r11.z, cb0[11].xyzw, r10.xyzw
    r11.z = (dot((source[11].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 80: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 81: dp4 r13.x, cb0[12].xyzw, r12.xyzw
    r13.x = (dot((source[12].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 82: dp4 r13.y, cb0[13].xyzw, r12.xyzw
    r13.y = (dot((source[13].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 83: dp4 r13.z, cb0[14].xyzw, r12.xyzw
    r13.z = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 84: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 85: mul r3.w, r10.y, r10.y
    r3.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 86: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 87: mad r3.w, r10.x, r10.x, -r3.w
    r3.w = ((r10.xxxx)*(r10.xxxx)+(-(r3.wwww))).w;
    // 88: mad r10.xyz, cb0[15].xyzx, r3.wwww, r11.xyzx
    r10.xyz = ((source[15].xyzx)*(r3.wwww)+(r11.xyzx)).xyz;
    // 89: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: mul r10.xyz, r10.xyzx, cb0[7].xyzx
    r10.xyz = ((r10.xyzx)*(source[7].xyzx)).xyz;
    // 91: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[7].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[7].wwww)).xyz;
    // 92: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 93: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 94: mul r11.xyz, r3.wwww, v5.xyzx
    r11.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 95: dp3 r3.w, r4.xyzx, r11.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 96: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 97: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 98: deriv_rtx_coarse r12.x, r3.w
    r12.x = (ddx_coarse(r3.wwww)).x;
    // 99: deriv_rty_coarse r12.y, r3.w
    r12.y = (ddy_coarse(r3.wwww)).y;
    // 100: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: dp2 r4.w, r12.xyxx, r12.xyxx
    r4.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 102: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 103: mad_sat r12.y, r4.w, l(0.300000), r1.z
    r12.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.zzzz))).y;
    // 105: add r1.z, -r12.y, l(1.000000)
    r1.z = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: mov_sat r2.w, cb0[5].z
    r2.w = (saturate(source[5].zzzz)).w;
    // 107: mad r13.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r13.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 108: mul r4.w, r2.w, l(0.080000)
    r4.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 110: mad r13.xyz, r1.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r1.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 111: max r14.xyz, r1.zzzz, r13.xyzx
    r14.xyz = (max(r1.zzzz,r13.xyzx)).xyz;
    // 112: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 113: mul_sat r1.z, r13.y, l(50.000000)
    r1.z = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 114: mul r14.xyz, r1.zzzz, r14.xyzx
    r14.xyz = ((r1.zzzz)*(r14.xyzx)).xyz;
    // 115: add r1.z, r4.z, l(1.000000)
    r1.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 116: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 117: add_sat r12.x, -r1.z, r3.w
    r12.x = (saturate((-(r1.zzzz))+(r3.wwww))).x;
    // 118: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t3.zwxy, s4 (unbound native scene environment contribution)
    r12.zw = (float4(0.f,0.f,0.f,0.f)).zw;
    // 119: add r1.z, r0.y, r12.x
    r1.z = ((r0.yyyy)+(r12.xxxx)).z;
    // 120: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 121: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 122: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 123: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.wwww)).w;
    // 124: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 125: mad r12.xzw, r13.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 126: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 127: mad r13.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 128: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 129: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 130: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 131: mul r0.xzw, r0.xxzw, r10.xxyz
    r0.xzw = ((r0.xxzw)*(r10.xxyz)).xzw;
    // 132: mad r0.xzw, -r0.xxzw, r1.wwww, r0.xxzw
    r0.xzw = ((-(r0.xxzw))*(r1.wwww)+(r0.xxzw)).xzw;
    // 133: mul r1.w, r12.y, l(5.000000)
    r1.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 134: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 135: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 136: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 137: add r1.z, r0.y, r1.z
    r1.z = ((r0.yyyy)+(r1.zzzz)).z;
    // 139: add_sat r0.y, r1.z, l(-1.000000)
    r0.y = (saturate((r1.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 140: dp3 r7.x, r7.xyzx, r4.xyzx
    r7.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 141: dp3 r7.y, r8.xyzx, r4.xyzx
    r7.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 142: dp2 r1.x, r7.xyxx, r1.xyxx
    r1.x = (dot((r7.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 143: dp2 r1.z, r7.xyxx, cb0[8].xyxx
    r1.z = (dot((r7.xyxx).xy,(source[8].xyxx).xy).xxxx).z;
    // 144: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 145: dp3 r2.w, r5.xyzx, r4.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 146: mad r4.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 147: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 148: sample_l_indexable(texturecube)(float,float,float,float) r1.xyzw, r1.xyzx, t4.xyzw, s3, r1.w (unbound native scene environment contribution)
    r1.xyzw = (float4(0.f,0.f,0.f,0.f)).xyzw;
    // 149: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 150: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 151: mad r1.xyz, r1.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[7].wwww)).xyz;
    // 152: mad r1.w, r0.y, r13.x, r13.y
    r1.w = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).w;
    // 153: mad r1.w, r1.w, r0.y, r13.z
    r1.w = ((r1.wwww)*(r0.yyyy)+(r13.zzzz)).w;
    // 154: mul r1.w, r0.y, r1.w
    r1.w = ((r0.yyyy)*(r1.wwww)).w;
    // 155: max r0.y, r0.y, r1.w
    r0.y = (max(r0.yyyy,r1.wwww)).y;
    // 156: mul r4.yzw, r4.yyyy, cb0[18].xxyz
    r4.yzw = ((r4.yyyy)*(source[18].xxyz)).yzw;
    // 157: mad r4.xyz, cb0[17].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[17].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 158: mul r4.xyz, r4.xyzx, cb0[19].wwww
    r4.xyz = ((r4.xyzx)*(source[19].wwww)).xyz;
    // 159: mul r4.xyz, r0.yyyy, r4.xyzx
    r4.xyz = ((r0.yyyy)*(r4.xyzx)).xyz;
    // 160: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 161: mad r0.xyz, r1.xyzx, r12.xzwx, r0.xzwx
    r0.xyz = ((r1.xyzx)*(r12.xzwx)+(r0.xzwx)).xyz;
    // 162: mul r1.xyz, r12.xzwx, r1.xyzx
    r1.xyz = ((r12.xzwx)*(r1.xyzx)).xyz;
    // 164: dp3 r0.w, r3.xyzx, r11.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 165: add r1.x, -|r11.z|, l(1.000000)
    r1.x = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 166: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 168: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 169: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 170: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 171: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 172: mul r1.yzw, r0.wwww, cb0[2].xxyz
    r1.yzw = ((r0.wwww)*(source[2].xxyz)).yzw;
    // 173: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 174: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 175: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 177: mad o0.xyz, r2.xyzx, cb0[19].xyzx, r1.xyzx
    output.xyz = ((r2.xyzx)*(source[19].xyzx)+(r1.xyzx)).xyz;
    // 178: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_gdogods_swordfloor01a_mi_alchemy: ef961e393dfade4c84fd1b0e773ee4ca; selected map b8f1239bcc910ca8da66fe2f93e44b9117fa6da829dfb44710ad316d159fc296.
float4 LanceVANative797(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[17]=float4(input.skyUpperColor,0.f);
    source[18]=float4(input.skyLowerColor,0.f);
    source[19]=float4(input.ambientColor,input.skyIntensity);
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = g_LanceVASourceMaterialParameters[4u];
    source[3] = g_LanceVASourceMaterialParameters[3u];
    source[4].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[4].y = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[5].z = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[5].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[6].x = (clamp(g_LanceVASourceMaterialParameters[2u].xxxx,float4(0.0, 0.0, 0.0, 0.0),float4(100.0, 0.0, 0.0, 0.0))).x;
    source[6].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: mul r0.z, r0.z, cb0[5].x
    r0.z = ((r0.zzzz)*(source[5].xxxx)).z;
    // 3: mul r0.xy, r0.yxyy, cb0[6].xzxx
    r0.xy = ((r0.yxyy)*(source[6].xzxx)).xy;
    // 4: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 5: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 6: mul r0.w, r0.w, cb0[5].y
    r0.w = ((r0.wwww)*(source[5].yyyy)).w;
    // 7: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 8: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 9: min r0.w, r0.z, l(1.000000)
    r0.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: mul_sat r1.w, r0.z, cb2[3].w
    r1.w = (saturate((r0.zzzz)*(passValues[3].wwww))).w;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = (LanceVANativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 12: mul r3.xyz, cb0[3].xyzx, cb0[4].yyyy
    r3.xyz = ((source[3].xyzx)*(source[4].yyyy)).xyz;
    // 13: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 14: mul r3.xyz, r2.xyzx, cb0[4].zzzz
    r3.xyz = ((r2.xyzx)*(source[4].zzzz)).xyz;
    // 15: mad r2.xyz, cb0[4].wwww, r2.xyzx, -r3.xyzx
    r2.xyz = ((source[4].wwww)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // 16: mad r2.xyz, r0.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 17: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 18: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 19: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 20: mad r3.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 21: mad r4.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 22: mad r5.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 23: log r0.zw, |r0.xxxy|
    r0.zw = (log2(abs(r0.xxxy))).zw;
    // 24: lt r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((abs(r0.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 25: mul r0.zw, r0.zzzw, cb0[6].yyyw
    r0.zw = ((r0.zzzw)*(source[6].yyyw)).zw;
    // 26: exp r0.zw, r0.zzzw
    r0.zw = (exp2(r0.zzzw)).zw;
    // 27: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: movc r0.xy, r0.xyxx, l(0,0,0,0), r0.zwzz
    r0.xy = ((asuint(r0.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zwzz)).xy;
    // 29: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 30: min r1.z, r0.x, l(1.000000)
    r1.z = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: mad r0.xzw, r0.yyyy, r4.xxyz, r5.xxyz
    r0.xzw = ((r0.yyyy)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 32: mad r0.xzw, r0.xxzw, r0.yyyy, r3.xxyz
    r0.xzw = ((r0.xxzw)*(r0.yyyy)+(r3.xxyz)).xzw;
    // 33: mul r0.xzw, r0.yyyy, r0.xxzw
    r0.xzw = ((r0.yyyy)*(r0.xxzw)).xzw;
    // 34: max r0.xzw, r0.xxzw, r0.yyyy
    r0.xzw = (max(r0.xxzw,r0.yyyy)).xzw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 36: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: dp2 r3.x, r1.xyxx, r1.xyxx
    r3.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 38: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 39: mul r4.xy, r1.xyxx, v2.wwww
    r4.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 40: add r1.x, -r3.x, l(1.000000)
    r1.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 45: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 46: div r3.xyz, r4.xyzx, r1.xxxx
    r3.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 47: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 48: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 49: mul r4.xyz, r1.xxxx, r3.xyzx
    r4.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 50: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 51: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 52: mul r5.xyz, r1.xxxx, v6.xyzx
    r5.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // 53: dp3 r1.x, r5.xyzx, r4.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 54: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 55: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 56: mul r6.xyz, r1.yyyy, cb0[18].xyzx
    r6.xyz = ((r1.yyyy)*(source[18].xyzx)).xyz;
    // 57: mad r6.xyz, r1.xxxx, cb0[17].xyzx, r6.xyzx
    r6.xyz = ((r1.xxxx)*(source[17].xyzx)+(r6.xyzx)).xyz;
    // 58: mul r6.xyz, r6.xyzx, cb0[19].wwww
    r6.xyz = ((r6.xyzx)*(source[19].wwww)).xyz;
    // 59: mul r6.xyz, r2.xyzx, r6.xyzx
    r6.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 60: mul r0.xzw, r0.xxzw, r6.xxyz
    r0.xzw = ((r0.xxzw)*(r6.xxyz)).xzw;
    // 61: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 62: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 63: mul r6.xyz, r1.xxxx, v1.xyzx
    r6.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 64: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 65: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 66: mul r7.xyz, r1.xxxx, v0.xyzx
    r7.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 67: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 68: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 69: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 70: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 71: dp3 r9.x, r7.xyzx, r4.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 72: dp2 r10.z, r9.xyxx, cb0[8].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[8].xyxx).xy).xxxx).z;
    // 73: mul r1.xy, cb0[8].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[8].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 74: dp2 r10.x, r9.xyxx, r1.xyxx
    r10.x = (dot((r9.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 75: dp3 r10.y, r6.xyzx, r4.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 76: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 77: dp4 r11.x, cb0[9].xyzw, r10.xyzw
    r11.x = (dot((source[9].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 78: dp4 r11.y, cb0[10].xyzw, r10.xyzw
    r11.y = (dot((source[10].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 79: dp4 r11.z, cb0[11].xyzw, r10.xyzw
    r11.z = (dot((source[11].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 80: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 81: dp4 r13.x, cb0[12].xyzw, r12.xyzw
    r13.x = (dot((source[12].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 82: dp4 r13.y, cb0[13].xyzw, r12.xyzw
    r13.y = (dot((source[13].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 83: dp4 r13.z, cb0[14].xyzw, r12.xyzw
    r13.z = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 84: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 85: mul r3.w, r10.y, r10.y
    r3.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 86: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 87: mad r3.w, r10.x, r10.x, -r3.w
    r3.w = ((r10.xxxx)*(r10.xxxx)+(-(r3.wwww))).w;
    // 88: mad r10.xyz, cb0[15].xyzx, r3.wwww, r11.xyzx
    r10.xyz = ((source[15].xyzx)*(r3.wwww)+(r11.xyzx)).xyz;
    // 89: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: mul r10.xyz, r10.xyzx, cb0[7].xyzx
    r10.xyz = ((r10.xyzx)*(source[7].xyzx)).xyz;
    // 91: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[7].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[7].wwww)).xyz;
    // 92: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 93: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 94: mul r11.xyz, r3.wwww, v5.xyzx
    r11.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 95: dp3 r3.w, r4.xyzx, r11.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 96: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 97: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 98: deriv_rtx_coarse r12.x, r3.w
    r12.x = (ddx_coarse(r3.wwww)).x;
    // 99: deriv_rty_coarse r12.y, r3.w
    r12.y = (ddy_coarse(r3.wwww)).y;
    // 100: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: dp2 r4.w, r12.xyxx, r12.xyxx
    r4.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 102: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 103: mad_sat r12.y, r4.w, l(0.300000), r1.z
    r12.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.zzzz))).y;
    // 105: add r1.z, -r12.y, l(1.000000)
    r1.z = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: mov_sat r2.w, cb0[5].z
    r2.w = (saturate(source[5].zzzz)).w;
    // 107: mad r13.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r13.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 108: mul r4.w, r2.w, l(0.080000)
    r4.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 110: mad r13.xyz, r1.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r1.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 111: max r14.xyz, r1.zzzz, r13.xyzx
    r14.xyz = (max(r1.zzzz,r13.xyzx)).xyz;
    // 112: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 113: mul_sat r1.z, r13.y, l(50.000000)
    r1.z = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 114: mul r14.xyz, r1.zzzz, r14.xyzx
    r14.xyz = ((r1.zzzz)*(r14.xyzx)).xyz;
    // 115: add r1.z, r4.z, l(1.000000)
    r1.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 116: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 117: add_sat r12.x, -r1.z, r3.w
    r12.x = (saturate((-(r1.zzzz))+(r3.wwww))).x;
    // 118: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t3.zwxy, s4 (unbound native scene environment contribution)
    r12.zw = (float4(0.f,0.f,0.f,0.f)).zw;
    // 119: add r1.z, r0.y, r12.x
    r1.z = ((r0.yyyy)+(r12.xxxx)).z;
    // 120: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 121: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 122: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 123: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.wwww)).w;
    // 124: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 125: mad r12.xzw, r13.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 126: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 127: mad r13.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 128: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 129: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 130: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 131: mul r0.xzw, r0.xxzw, r10.xxyz
    r0.xzw = ((r0.xxzw)*(r10.xxyz)).xzw;
    // 132: mad r0.xzw, -r0.xxzw, r1.wwww, r0.xxzw
    r0.xzw = ((-(r0.xxzw))*(r1.wwww)+(r0.xxzw)).xzw;
    // 133: mul r1.w, r12.y, l(5.000000)
    r1.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 134: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 135: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 136: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 137: add r1.z, r0.y, r1.z
    r1.z = ((r0.yyyy)+(r1.zzzz)).z;
    // 139: add_sat r0.y, r1.z, l(-1.000000)
    r0.y = (saturate((r1.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 140: dp3 r7.x, r7.xyzx, r4.xyzx
    r7.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 141: dp3 r7.y, r8.xyzx, r4.xyzx
    r7.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 142: dp2 r1.x, r7.xyxx, r1.xyxx
    r1.x = (dot((r7.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 143: dp2 r1.z, r7.xyxx, cb0[8].xyxx
    r1.z = (dot((r7.xyxx).xy,(source[8].xyxx).xy).xxxx).z;
    // 144: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 145: dp3 r2.w, r5.xyzx, r4.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 146: mad r4.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 147: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 148: sample_l_indexable(texturecube)(float,float,float,float) r1.xyzw, r1.xyzx, t4.xyzw, s3, r1.w (unbound native scene environment contribution)
    r1.xyzw = (float4(0.f,0.f,0.f,0.f)).xyzw;
    // 149: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 150: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 151: mad r1.xyz, r1.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[7].wwww)).xyz;
    // 152: mad r1.w, r0.y, r13.x, r13.y
    r1.w = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).w;
    // 153: mad r1.w, r1.w, r0.y, r13.z
    r1.w = ((r1.wwww)*(r0.yyyy)+(r13.zzzz)).w;
    // 154: mul r1.w, r0.y, r1.w
    r1.w = ((r0.yyyy)*(r1.wwww)).w;
    // 155: max r0.y, r0.y, r1.w
    r0.y = (max(r0.yyyy,r1.wwww)).y;
    // 156: mul r4.yzw, r4.yyyy, cb0[18].xxyz
    r4.yzw = ((r4.yyyy)*(source[18].xxyz)).yzw;
    // 157: mad r4.xyz, cb0[17].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[17].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 158: mul r4.xyz, r4.xyzx, cb0[19].wwww
    r4.xyz = ((r4.xyzx)*(source[19].wwww)).xyz;
    // 159: mul r4.xyz, r0.yyyy, r4.xyzx
    r4.xyz = ((r0.yyyy)*(r4.xyzx)).xyz;
    // 160: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 161: mad r0.xyz, r1.xyzx, r12.xzwx, r0.xzwx
    r0.xyz = ((r1.xyzx)*(r12.xzwx)+(r0.xzwx)).xyz;
    // 162: mul r1.xyz, r12.xzwx, r1.xyzx
    r1.xyz = ((r12.xzwx)*(r1.xyzx)).xyz;
    // 164: dp3 r0.w, r3.xyzx, r11.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 165: add r1.x, -|r11.z|, l(1.000000)
    r1.x = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 166: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 168: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 169: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 170: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 171: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 172: mul r1.yzw, r0.wwww, cb0[2].xxyz
    r1.yzw = ((r0.wwww)*(source[2].xxyz)).yzw;
    // 173: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 174: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 175: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 177: mad o0.xyz, r2.xyzx, cb0[19].xyzx, r1.xyzx
    output.xyz = ((r2.xyzx)*(source[19].xyzx)+(r1.xyzx)).xyz;
    // 178: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_gdogods_swordfloor01_mi_alchemy: 7af4912b3f3201498ff32236c2388f23; selected map 8b707a2d7420228ad371b6ad5638e649b4bb55d1fa95743c45dbec84573850d6.
float4 LanceVANative798(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[25]; [unroll] for (uint i=0u; i<25u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[22]=float4(input.skyUpperColor,0.f);
    source[23]=float4(input.skyLowerColor,0.f);
    source[24]=float4(input.ambientColor,input.skyIntensity);
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = g_LanceVASourceMaterialParameters[5u];
    source[5] = g_LanceVASourceMaterialParameters[7u];
    source[6].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[6].y = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[6].w = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[7].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].z = ((float4(-0.5, 0.0, 0.0, 0.0)+g_LanceVASourceMaterialParameters[2u].zzzz)).x;
    source[7].w = (((float4(-0.5, 0.0, 0.0, 0.0)+g_LanceVASourceMaterialParameters[2u].zzzz)*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[8].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[8].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[8].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[8].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[9].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[3u].xxxx)).x;
    source[9].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[9].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[9].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[10].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[10].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[10].w = (clamp(g_LanceVASourceMaterialParameters[4u].xxxx,float4(0.0, 0.0, 0.0, 0.0),float4(100.0, 0.0, 0.0, 0.0))).x;
    source[11].x = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[7].xxxx
    r0.xy = ((source[0].xyxx)*(source[7].xxxx)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mul r1.xy, v4.xyxx, cb0[6].yyyy
    r1.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: mul r1.xy, r1.xyxx, cb0[6].zzzz
    r1.xy = ((r1.xyxx)*(source[6].zzzz)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 9: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: mad r1.xy, cb0[6].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[6].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 11: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 12: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 23: dp3 r3.x, r2.xyzx, r1.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 24: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 27: dp3 r3.z, r4.xyzx, r1.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 28: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 29: mul r5.xyz, r2.yzxy, r4.zxyz
    r5.xyz = ((r2.yzxy)*(r4.zxyz)).xyz;
    // 30: mad r5.xyz, r4.yzxy, r2.zxyz, -r5.xyzx
    r5.xyz = ((r4.yzxy)*(r2.zxyz)+(-(r5.xyzx))).xyz;
    // 31: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 34: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mad r0.x, r0.x, l(0.500000), cb0[7].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].wwww)).x;
    // 36: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 38: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.zw, v4.xxxy, cb0[8].xxxx
    r0.zw = ((v4.xxxy)*(source[8].xxxx)).zw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = (LanceVANativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 42: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[6].w, l(0.000000)
    r0.z = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 46: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 47: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 48: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 51: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: mul r0.yzw, cb0[5].xxyz, cb0[8].zzzz
    r0.yzw = ((source[5].xxyz)*(source[8].zzzz)).yzw;
    // 55: mul r7.xyz, r6.xyzx, r0.yzwy
    r7.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 56: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: mad r0.yzw, -r0.yyzw, r6.xxyz, r1.wwww
    r0.yzw = ((-(r0.yyzw))*(r6.xxyz)+(r1.wwww)).yzw;
    // 58: mad r0.yzw, cb0[9].xxxx, r0.yyzw, r7.xxyz
    r0.yzw = ((source[9].xxxx)*(r0.yyzw)+(r7.xxyz)).yzw;
    // 59: mul r6.xyz, cb0[4].xyzx, cb0[8].yyyy
    r6.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 60: mad r0.yzw, -r3.xxyz, r6.xxyz, r0.yyzw
    r0.yzw = ((-(r3.xxyz))*(r6.xxyz)+(r0.yyzw)).yzw;
    // 61: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 62: mad r0.yzw, r0.xxxx, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.xxxx)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 63: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 64: mul r3.xyz, r0.yzwy, cb0[9].yyyy
    r3.xyz = ((r0.yzwy)*(source[9].yyyy)).xyz;
    // 65: mad r0.yzw, cb0[9].zzzz, r0.yyzw, -r3.xxyz
    r0.yzw = ((source[9].zzzz)*(r0.yyzw)+(-(r3.xxyz))).yzw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = (LanceVANativeSample4((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 67: mul r1.w, r6.z, cb0[9].w
    r1.w = ((r6.zzzz)*(source[9].wwww)).w;
    // 68: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 69: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 70: mul r2.w, r2.w, cb0[10].x
    r2.w = ((r2.wwww)*(source[10].xxxx)).w;
    // 71: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 72: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 73: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 75: mad r0.yzw, r2.wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((r2.wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 76: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 78: mad_sat r3.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 79: mad r0.yzw, r3.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r3.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 80: mad r7.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 81: mad r8.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 82: mul r1.w, r6.x, cb0[11].y
    r1.w = ((r6.xxxx)*(source[11].yyyy)).w;
    // 83: mul r2.w, r6.y, cb0[10].w
    r2.w = ((r6.yyyy)*(source[10].wwww)).w;
    // 84: log r4.w, |r1.w|
    r4.w = (log2(abs(r1.wwww))).w;
    // 85: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 86: mul r4.w, r4.w, cb0[11].z
    r4.w = ((r4.wwww)*(source[11].zzzz)).w;
    // 87: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 88: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: movc r1.w, r1.w, l(0), r4.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 90: mad r7.xyz, r1.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 91: mad r0.yzw, r7.xxyz, r1.wwww, r0.yyzw
    r0.yzw = ((r7.xxyz)*(r1.wwww)+(r0.yyzw)).yzw;
    // 92: mul r0.yzw, r1.wwww, r0.yyzw
    r0.yzw = ((r1.wwww)*(r0.yyzw)).yzw;
    // 93: max r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = (max(r0.yyzw,r1.wwww)).yzw;
    // 94: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 95: mad r1.xyz, r0.xxxx, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 96: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 97: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 98: mul r7.xyz, r0.xxxx, r1.xyzx
    r7.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 99: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 100: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 101: mul r8.xyz, r0.xxxx, v6.xyzx
    r8.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 102: dp3 r0.x, r8.xyzx, r7.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 103: mad r6.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 104: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 105: mul r9.xyz, r6.yyyy, cb0[23].xyzx
    r9.xyz = ((r6.yyyy)*(source[23].xyzx)).xyz;
    // 106: mad r9.xyz, r6.xxxx, cb0[22].xyzx, r9.xyzx
    r9.xyz = ((r6.xxxx)*(source[22].xyzx)+(r9.xyzx)).xyz;
    // 107: mul r9.xyz, r9.xyzx, cb0[24].wwww
    r9.xyz = ((r9.xyzx)*(source[24].wwww)).xyz;
    // 108: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 109: mul r0.xyz, r0.yzwy, r9.xyzx
    r0.xyz = ((r0.yzwy)*(r9.xyzx)).xyz;
    // 110: dp3 r9.x, r2.xyzx, r7.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 111: dp3 r9.y, r5.xyzx, r7.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 112: dp2 r10.z, r9.xyxx, cb0[13].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 113: dp3 r10.y, r4.xyzx, r7.xyzx
    r10.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 114: mul r6.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 115: dp2 r10.x, r9.xyxx, r6.xyxx
    r10.x = (dot((r9.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 116: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 117: dp4 r11.x, cb0[14].xyzw, r10.xyzw
    r11.x = (dot((source[14].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 118: dp4 r11.y, cb0[15].xyzw, r10.xyzw
    r11.y = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 119: dp4 r11.z, cb0[16].xyzw, r10.xyzw
    r11.z = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 120: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 121: dp4 r13.x, cb0[17].xyzw, r12.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 122: dp4 r13.y, cb0[18].xyzw, r12.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 123: dp4 r13.z, cb0[19].xyzw, r12.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 124: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 125: mul r0.w, r10.y, r10.y
    r0.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 126: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 127: mad r0.w, r10.x, r10.x, -r0.w
    r0.w = ((r10.xxxx)*(r10.xxxx)+(-(r0.wwww))).w;
    // 128: mad r10.xyz, cb0[20].xyzx, r0.wwww, r11.xyzx
    r10.xyz = ((source[20].xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 129: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 130: mul r10.xyz, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((r10.xyzx)*(source[12].xyzx)).xyz;
    // 131: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 132: mov_sat r3.w, cb0[10].y
    r3.w = (saturate(source[10].yyyy)).w;
    // 133: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 134: mul r0.w, r3.w, l(0.080000)
    r0.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 136: mad r11.xyz, r6.wwww, r11.xyzx, r0.wwww
    r11.xyz = ((r6.wwww)*(r11.xyzx)+(r0.wwww)).xyz;
    // 137: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 138: log r3.w, |r2.w|
    r3.w = (log2(abs(r2.wwww))).w;
    // 139: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 140: mul r3.w, r3.w, cb0[11].x
    r3.w = ((r3.wwww)*(source[11].xxxx)).w;
    // 141: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 142: movc r2.w, r2.w, l(0), r3.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 143: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 144: min r6.z, r2.w, l(1.000000)
    r6.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 145: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 146: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 147: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 148: dp3 r2.w, r7.xyzx, r12.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 149: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 150: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 151: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 152: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 153: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 155: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 156: mad_sat r13.y, r3.w, l(0.300000), r6.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 158: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 160: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 161: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 162: add r0.w, r7.z, l(1.000000)
    r0.w = ((r7.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: add_sat r13.x, -r0.w, r2.w
    r13.x = (saturate((-(r0.wwww))+(r2.wwww))).x;
    // 165: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6 (unbound native scene environment contribution)
    r13.zw = (float4(0.f,0.f,0.f,0.f)).zw;
    // 166: add r0.w, r1.w, r13.x
    r0.w = ((r1.wwww)+(r13.xxxx)).w;
    // 167: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 168: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 169: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 170: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r13.wwww)).w;
    // 171: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 172: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 173: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 174: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 175: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 176: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 177: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 178: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 179: mad r0.xyz, -r0.xyzx, r6.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r6.wwww)+(r0.xyzx)).xyz;
    // 180: dp3 r2.x, r2.xyzx, r7.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 181: dp3 r2.y, r5.xyzx, r7.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 182: dp2 r5.x, r2.xyxx, r6.xyxx
    r5.x = (dot((r2.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 183: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 184: mul r2.x, r13.y, l(5.000000)
    r2.x = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 185: mul r2.y, r13.y, r13.y
    r2.y = ((r13.yyyy)*(r13.yyyy)).y;
    // 186: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 187: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 188: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 190: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 191: dp3 r5.y, r4.xyzx, r7.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 192: dp3 r1.w, r8.xyzx, r7.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 193: mad r2.yz, r1.wwww, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r1.wwww)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 194: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 195: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r5.xyzx, t6.xyzw, s5, r2.x (unbound native scene environment contribution)
    r4.xyzw = (float4(0.f,0.f,0.f,0.f)).xyzw;
    // 196: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 197: mul r4.xyz, r4.xyzx, cb0[12].xyzx
    r4.xyz = ((r4.xyzx)*(source[12].xyzx)).xyz;
    // 198: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 199: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 200: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 201: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 202: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 203: mul r2.xzw, r2.zzzz, cb0[23].xxyz
    r2.xzw = ((r2.zzzz)*(source[23].xxyz)).xzw;
    // 204: mad r2.xyz, cb0[22].xyzx, r2.yyyy, r2.xzwx
    r2.xyz = ((source[22].xyzx)*(r2.yyyy)+(r2.xzwx)).xyz;
    // 205: mul r2.xyz, r2.xyzx, cb0[24].wwww
    r2.xyz = ((r2.xyzx)*(source[24].wwww)).xyz;
    // 206: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 207: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 208: mad r0.xyz, r2.xyzx, r13.xzwx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r13.xzwx)+(r0.xyzx)).xyz;
    // 209: mul r2.xyz, r13.xzwx, r2.xyzx
    r2.xyz = ((r13.xzwx)*(r2.xyzx)).xyz;
    // 211: dp3 r0.w, r1.xyzx, r12.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 212: add r1.x, -|r12.z|, l(1.000000)
    r1.x = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 214: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 215: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 216: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 217: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 218: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 219: mul r1.yzw, r0.wwww, cb0[3].xxyz
    r1.yzw = ((r0.wwww)*(source[3].xxyz)).yzw;
    // 220: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 221: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 222: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 224: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r1.xyzx
    output.xyz = ((r3.xyzx)*(source[24].xyzx)+(r1.xyzx)).xyz;
    // 225: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_rhd_stone_rock12_mi_ksr: 2173c0c1bbd8d54e8b78df0014d7848a; selected map 3a3ae10d9d74a2790099b3aff4f48010cde1c9e1e4407a0c6f2f8807022ca0d0.
float4 LanceVANative799(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[8]=float4(input.skyUpperColor,0.f);
    source[9]=float4(input.skyLowerColor,0.f);
    source[10]=float4(input.ambientColor,input.skyIntensity);
    source[0] = g_LanceVASourceMaterialParameters[6u];
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3] = g_LanceVASourceMaterialParameters[4u];
    source[4].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[5].y = ((float4(-0.5, 0.0, 0.0, 0.0)+g_LanceVASourceMaterialParameters[0u].wwww)).x;
    source[5].z = (((float4(-0.5, 0.0, 0.0, 0.0)+g_LanceVASourceMaterialParameters[0u].wwww)*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[5].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[6].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[6].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[1u].zzzz)).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
    // 1: max r0.xyz, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[1].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 6: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 7: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 8: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 10: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 11: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 15: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 16: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 17: mul r2.xyz, r0.wwww, v1.xyzx
    r2.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 18: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 21: mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // 22: mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 23: mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 24: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 25: dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 26: dp3 r5.z, r2.xyzx, r1.xyzx
    r5.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 27: dp3 r0.x, r5.xyzx, r0.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 28: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: mad r0.x, r0.x, l(0.500000), cb0[5].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).x;
    // 30: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 32: mul_sat r0.y, r0.y, r5.w
    r0.y = (saturate((r0.yyyy)*(r5.wwww))).y;
    // 33: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: mul r0.zw, v4.xxxy, cb0[4].yyyy
    r0.zw = ((v4.xxxy)*(source[4].yyyy)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = (LanceVANativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 37: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 38: mul r1.w, r6.w, r6.w
    r1.w = ((r6.wwww)*(r6.wwww)).w;
    // 39: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 40: max r1.w, cb0[4].w, l(0.000000)
    r1.w = (max(source[4].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 43: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 44: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 45: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 47: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 48: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 49: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 50: dp3 r0.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 51: add r7.xyz, -r5.xyzx, r0.yyyy
    r7.xyz = ((-(r5.xyzx))+(r0.yyyy)).xyz;
    // 52: mad r5.xyz, cb0[6].xxxx, r7.xyzx, r5.xyzx
    r5.xyz = ((source[6].xxxx)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 53: mul r7.xyz, cb0[3].xyzx, cb0[6].zzzz
    r7.xyz = ((source[3].xyzx)*(source[6].zzzz)).xyz;
    // 54: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 55: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 56: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 57: mad r6.xyz, cb0[7].xxxx, r6.xyzx, r8.xyzx
    r6.xyz = ((source[7].xxxx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 58: mul r7.xyz, cb0[2].xyzx, cb0[6].yyyy
    r7.xyz = ((source[2].xyzx)*(source[6].yyyy)).xyz;
    // 59: mad r6.xyz, -r5.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 60: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 61: mad r5.xyz, r0.xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 62: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 63: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 64: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 65: mul r6.xy, r0.zwzz, cb0[4].zzzz
    r6.xy = ((r0.zwzz)*(source[4].zzzz)).xy;
    // 66: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 68: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 69: add r6.z, r0.y, l(0.000010)
    r6.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 70: add r0.yzw, -r1.xxyz, r6.xxyz
    r0.yzw = ((-(r1.xxyz))+(r6.xxyz)).yzw;
    // 71: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 72: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 73: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 74: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 75: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 76: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 77: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 78: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 79: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 80: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 81: mul r1.yzw, r1.yyyy, cb0[9].xxyz
    r1.yzw = ((r1.yyyy)*(source[9].xxyz)).yzw;
    // 82: mad r1.xyz, r1.xxxx, cb0[8].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[8].xyzx)+(r1.yzwy)).xyz;
    // 83: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 84: mad r6.xyz, r1.xyzx, r5.xyzx, cb0[0].xyzx
    r6.xyz = ((r1.xyzx)*(r5.xyzx)+(source[0].xyzx)).xyz;
    // 85: mul r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r1.xyzx)).xyz;
    // 87: mad o0.xyz, r5.xyzx, cb0[10].xyzx, r6.xyzx
    output.xyz = ((r5.xyzx)*(source[10].xyzx)+(r6.xyzx)).xyz;
    // 89: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif

#ifndef LANCE_VA_NATIVE_MODEL_ONLY
// bg_ocn_stone_rock01_mi_ksr: 9d53e0395be73e4a9ace11b65529f6e5; selected map 31c13ef65c468422e9f7ea319a5182e2d6c5e4bfe3038460c557fa8c53ee17b7.
float4 LanceVANative800(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[0]=0.f; // Absolute source-centimetre positions use zero pre-view translation.
    source[10]=float4(input.skyUpperColor,0.f);
    source[11]=float4(input.skyLowerColor,0.f);
    source[12]=float4(input.ambientColor,input.skyIntensity);
    source[0] = g_LanceVASourceMaterialParameters[7u];
    source[1] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[3u].zzzz,(g_LanceVASourceMaterialParameters[3u].zzzz*g_LanceVASourceMaterialParameters[3u].wwww),1u);
    source[2] = g_LanceVASourceMaterialParameters[6u];
    source[3] = g_LanceVASourceMaterialParameters[4u];
    source[4] = g_LanceVASourceMaterialParameters[5u];
    source[5].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[5].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[5].z = ((g_LanceVASourceMaterialParameters[3u].zzzz*g_LanceVASourceMaterialParameters[3u].wwww)).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].y = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[7].z = ((float4(-0.5, 0.0, 0.0, 0.0)+g_LanceVASourceMaterialParameters[1u].yyyy)).x;
    source[7].w = (((float4(-0.5, 0.0, 0.0, 0.0)+g_LanceVASourceMaterialParameters[1u].yyyy)*float4(2.0, 0.0, 0.0, 0.0))).x;
    source[8].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].wwww)).x;
    source[8].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[2u].xxxx)).x;
    source[9].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[1].xyxx
    r1.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 4: mul r1.zw, r1.xxxy, cb0[6].xxxx
    r1.zw = ((r1.xxxy)*(source[6].xxxx)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = (LanceVANativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 6: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mul r1.zw, r1.zzzw, cb0[6].yyyy
    r1.zw = ((r1.zzzw)*(source[6].yyyy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 10: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: mad r1.zw, cb0[5].wwww, r1.xxxy, r1.zzzw
    r1.zw = ((source[5].wwww)*(r1.xxxy)+(r1.zzzw)).zw;
    // 12: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 18: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 20: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 21: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 22: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 23: mul r2.xyz, r0.wwww, v1.xyzx
    r2.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 24: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r4.xyz, r0.wwww, v0.xyzx
    r4.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 27: mul r5.xyz, r2.zxyz, r4.yzxy
    r5.xyz = ((r2.zxyz)*(r4.yzxy)).xyz;
    // 28: mad r5.xyz, r2.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r2.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 29: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r6.y, r5.xyzx, r1.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r6.x, r4.xyzx, r1.xyzx
    r6.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 32: dp3 r6.z, r2.xyzx, r1.xyzx
    r6.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 33: dp3 r0.x, r6.xyzx, r0.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 34: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mad r0.x, r0.x, l(0.500000), cb0[7].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].wwww)).x;
    // 36: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 37: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 38: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: mul r0.zw, v4.xxxy, cb0[6].zzzz
    r0.zw = ((v4.xxxy)*(source[6].zzzz)).zw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t4.xyzw, s4, l(0.000000)
    r6.xyzw = (LanceVANativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = (LanceVANativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 42: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 43: mul r1.w, r6.w, r6.w
    r1.w = ((r6.wwww)*(r6.wwww)).w;
    // 44: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 45: max r1.w, cb0[7].x, l(0.000000)
    r1.w = (max(source[7].xxxx,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 46: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 47: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 48: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 49: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 50: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 52: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 53: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 54: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 55: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 56: add r7.xyz, -r3.xyzx, r0.yyyy
    r7.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 57: mad r3.xyz, cb0[8].yyyy, r7.xyzx, r3.xyzx
    r3.xyz = ((source[8].yyyy)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 58: mul r7.xyz, cb0[4].xyzx, cb0[8].wwww
    r7.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 59: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 60: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 61: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 62: mad r6.xyz, cb0[9].yyyy, r6.xyzx, r8.xyzx
    r6.xyz = ((source[9].yyyy)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 63: mul r7.xyz, cb0[3].xyzx, cb0[8].zzzz
    r7.xyz = ((source[3].xyzx)*(source[8].zzzz)).xyz;
    // 64: mad r6.xyz, -r3.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 65: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 66: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 67: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 68: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 69: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 70: mul r6.xy, r0.zwzz, cb0[6].wwww
    r6.xy = ((r0.zwzz)*(source[6].wwww)).xy;
    // 71: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 72: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 73: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 74: add r6.z, r0.y, l(0.000010)
    r6.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 75: add r0.yzw, -r1.xxyz, r6.xxyz
    r0.yzw = ((-(r1.xxyz))+(r6.xxyz)).yzw;
    // 76: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 77: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 78: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 79: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 80: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 81: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 82: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 83: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 84: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 85: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 86: mul r1.yzw, r1.yyyy, cb0[11].xxyz
    r1.yzw = ((r1.yyyy)*(source[11].xxyz)).yzw;
    // 87: mad r1.xyz, r1.xxxx, cb0[10].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[10].xyzx)+(r1.yzwy)).xyz;
    // 88: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 89: mad r6.xyz, r1.xyzx, r3.xyzx, cb0[0].xyzx
    r6.xyz = ((r1.xyzx)*(r3.xyzx)+(source[0].xyzx)).xyz;
    // 90: mul r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.xyzx)).xyz;
    // 92: mad o0.xyz, r3.xyzx, cb0[12].xyzx, r6.xyzx
    output.xyz = ((r3.xyzx)*(source[12].xyzx)+(r6.xyzx)).xyz;
    // 94: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=input.color.a; // Native opaque RT0 alpha is not coverage; exact source discard already ran.
    return output;
}
#endif
