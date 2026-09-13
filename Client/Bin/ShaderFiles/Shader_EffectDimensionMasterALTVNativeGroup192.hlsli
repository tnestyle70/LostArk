// Single source owner for ALTVNative profiles 192..255.
// fx_k_pa_circlenoise_01_tr: dae27e0258d6624199078efc591c9303; selected map cd64395a8c8060785ad7f7f3f11e8a38ebd8ced1904a127df8c675d8c8648b40.
float4 ALTVNative192(ALTV_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 1.700000, 1.700000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,1.700000,1.700000))).zw;
    // 3: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 4: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 5: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 7: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 8: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 9: mul r0.z, r0.z, v4.x
    r0.z = ((r0.zzzz)*(v4.xxxx)).z;
    // 10: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 11: movc r1.y, r0.y, l(0), r0.z
    r1.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 12: add r0.y, -r1.y, l(1.000000)
    r0.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 13: max r0.z, |r0.w|, |r0.y|
    r0.z = (max(abs(r0.wwww),abs(r0.yyyy))).z;
    // 14: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 15: min r1.z, |r0.w|, |r0.y|
    r1.z = (min(abs(r0.wwww),abs(r0.yyyy))).z;
    // 16: mul r0.z, r0.z, r1.z
    r0.z = ((r0.zzzz)*(r1.zzzz)).z;
    // 17: mul r1.z, r0.z, r0.z
    r1.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 18: mad r1.w, r1.z, l(0.020835), l(-0.085133)
    r1.w = ((r1.zzzz)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).w;
    // 19: mad r1.w, r1.z, r1.w, l(0.180141)
    r1.w = ((r1.zzzz)*(r1.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 20: mad r1.w, r1.z, r1.w, l(-0.330299)
    r1.w = ((r1.zzzz)*(r1.wwww)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).w;
    // 21: mad r1.z, r1.z, r1.w, l(0.999866)
    r1.z = ((r1.zzzz)*(r1.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 22: mul r1.w, r0.z, r1.z
    r1.w = ((r0.zzzz)*(r1.zzzz)).w;
    // 23: mad r1.w, r1.w, l(-2.000000), l(1.570796)
    r1.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).w;
    // 24: lt r2.x, |r0.w|, |r0.y|
    r2.x = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.yyyy))) * 0xffffffffu)).x;
    // 25: and r1.w, r1.w, r2.x
    r1.w = (asfloat(asuint(r1.wwww) & asuint(r2.xxxx))).w;
    // 26: mad r0.z, r0.z, r1.z, r1.w
    r0.z = ((r0.zzzz)*(r1.zzzz)+(r1.wwww)).z;
    // 27: lt r1.z, r0.w, -r0.w
    r1.z = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).z;
    // 28: and r1.z, r1.z, l(0xc0490fdb)
    r1.z = (asfloat(asuint(r1.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 29: add r0.z, r0.z, r1.z
    r0.z = ((r0.zzzz)+(r1.zzzz)).z;
    // 30: min r1.z, r0.w, r0.y
    r1.z = (min(r0.wwww,r0.yyyy)).z;
    // 31: max r0.y, r0.w, r0.y
    r0.y = (max(r0.wwww,r0.yyyy)).y;
    // 32: ge r0.y, r0.y, -r0.y
    r0.y = (asfloat((uint4)((r0.yyyy)>=(-(r0.yyyy))) * 0xffffffffu)).y;
    // 33: lt r0.w, r1.z, -r1.z
    r0.w = (asfloat((uint4)((r1.zzzz)<(-(r1.zzzz))) * 0xffffffffu)).w;
    // 34: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 35: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 36: mul r1.x, r0.y, l(0.530517)
    r1.x = ((r0.yyyy)*(float4(0.530517,0.530517,0.530517,0.530517))).x;
    // 37: mov r2.x, l(0)
    r2.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 38: mul r2.y, v4.y, l(-0.500000)
    r2.y = ((v4.yyyy)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 39: add r0.yz, r1.xxyx, r2.xxyx
    r0.yz = ((r1.xxyx)+(r2.xxyx)).yz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ALTVNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 41: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 42: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 43: mad r0.yzw, r1.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r0.yyzw
    r0.yzw = ((r1.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r0.yyzw)).yzw;
    // 44: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 45: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 46: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 47: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 48: mul r0.y, r0.y, l(50.000000)
    r0.y = ((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000))).y;
    // 49: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 50: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 51: rsq r0.y, r0.x
    r0.y = (rsqrt(r0.xxxx)).y;
    // 52: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 53: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 54: mul r0.x, r0.x, l(25.000000)
    r0.x = ((r0.xxxx)*(float4(25.000000,25.000000,25.000000,25.000000))).x;
    // 55: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 56: movc r0.y, r0.z, l(-0.000000), -r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 57: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 58: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 59: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_m_swp_cam_line_02-3_tr: cb00f25c542bff4e80cadb10ef536113; selected map 03b9b81443d762a972c3e6405ebd2127f5f0a2bf3d63c1b89c057bf31963bdc6.
float4 ALTVNative193(ALTV_NATIVE_INPUT input)
{
    float4 source[24]; [unroll] for (uint i=0u; i<24u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[19u];
    source[2].x = (g_ALTVSourceMaterialParameters[18u].yyyy).x;
    source[2].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[2].z = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[2].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[3].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[3].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].yyyy)).x;
    source[3].w = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[4].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((g_ALTVSourceMaterialParameters[7u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[5].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[5].w = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[6].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[6].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].zzzz)).x;
    source[6].w = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[7].x = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[7u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[17u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[9].x = (g_ALTVSourceMaterialParameters[15u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[9].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].wwww)).x;
    source[10].x = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[10].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[11].x = ((g_ALTVSourceMaterialParameters[7u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[11].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[18u].xxxx).x;
    source[12].x = (g_ALTVSourceMaterialParameters[16u].wwww).x;
    source[12].y = (g_ALTVSourceMaterialParameters[17u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[13].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[14].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].xxxx)).x;
    source[14].y = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[14].w = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[15].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[15].z = ((g_ALTVSourceMaterialParameters[7u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[15].w = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[16u].xxxx).x;
    source[16].y = (g_ALTVSourceMaterialParameters[17u].yyyy).x;
    source[16].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[17].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[17].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[17].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[17].w = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[18].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[18].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[18].w = ((g_ALTVSourceMaterialParameters[8u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[19].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[19].y = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[19].z = (g_ALTVSourceMaterialParameters[16u].yyyy).x;
    source[19].w = (g_ALTVSourceMaterialParameters[17u].zzzz).x;
    source[20].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[20].y = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[20].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[20].w = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[21].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].xxxx)).x;
    source[21].y = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[21].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[21].w = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[22].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[22].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[22].z = ((g_ALTVSourceMaterialParameters[8u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[22].w = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[23].x = (g_ALTVSourceMaterialParameters[16u].zzzz).x;
    source[23].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[23].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
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
    // 28: mad r0.z, v4.y, cb0[10].y, cb0[10].z
    r0.z = ((v4.yyyy)*(source[10].yyyy)+(source[10].zzzz)).z;
    // 29: add r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)+(source[11].xxxx)).z;
    // 30: mad r0.z, cb0[11].y, r0.y, r0.z
    r0.z = ((source[11].yyyy)*(r0.yyyy)+(r0.zzzz)).z;
    // 31: mad r0.w, v4.y, cb0[9].x, cb0[9].y
    r0.w = ((v4.yyyy)*(source[9].xxxx)+(source[9].yyyy)).w;
    // 32: add r0.w, r0.w, cb0[9].w
    r0.w = ((r0.wwww)+(source[9].wwww)).w;
    // 33: mad r0.w, r0.x, cb0[8].w, r0.w
    r0.w = ((r0.xxxx)*(source[8].wwww)+(r0.wwww)).w;
    // 34: mad r1.x, cb0[10].x, r0.w, r0.z
    r1.x = ((source[10].xxxx)*(r0.wwww)+(r0.zzzz)).x;
    // 35: mad r1.y, r0.z, cb0[11].z, r0.w
    r1.y = ((r0.zzzz)*(source[11].zzzz)+(r0.wwww)).y;
    // 36: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t1.zwxy, s2, cb0[2].x
    r0.zw = (ALTVNativeSample2((r1.xyxx).xy, (source[2].xxxx).x, true).zwxy).zw;
    // 37: mul r0.zw, r0.zzzw, cb0[11].wwww
    r0.zw = ((r0.zzzw)*(source[11].wwww)).zw;
    // 38: mad r1.x, v4.y, cb0[7].x, cb0[7].y
    r1.x = ((v4.yyyy)*(source[7].xxxx)+(source[7].yyyy)).x;
    // 39: add r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)+(source[7].wwww)).x;
    // 40: mad r1.x, cb0[8].x, r0.y, r1.x
    r1.x = ((source[8].xxxx)*(r0.yyyy)+(r1.xxxx)).x;
    // 41: mad r1.y, v4.y, cb0[5].w, cb0[6].x
    r1.y = ((v4.yyyy)*(source[5].wwww)+(source[6].xxxx)).y;
    // 42: add r1.y, r1.y, cb0[6].z
    r1.y = ((r1.yyyy)+(source[6].zzzz)).y;
    // 43: mad r1.y, r0.x, cb0[5].z, r1.y
    r1.y = ((r0.xxxx)*(source[5].zzzz)+(r1.yyyy)).y;
    // 44: mad r2.x, cb0[6].w, r1.y, r1.x
    r2.x = ((source[6].wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 45: mad r2.y, r1.x, cb0[8].y, r1.y
    r2.y = ((r1.xxxx)*(source[8].yyyy)+(r1.yyyy)).y;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s1, cb0[2].x
    r1.xy = (ALTVNativeSample1((r2.xyxx).xy, (source[2].xxxx).x, true).xyzw).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, r0.zzzw
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(r0.zzzw)).zw;
    // 48: mul r0.zw, r0.zzzw, v4.zzzz
    r0.zw = ((r0.zzzw)*(v4.zzzz)).zw;
    // 49: mad r1.x, v4.x, cb0[14].w, cb0[15].x
    r1.x = ((v4.xxxx)*(source[14].wwww)+(source[15].xxxx)).x;
    // 50: mad r1.x, cb0[15].y, r0.y, r1.x
    r1.x = ((source[15].yyyy)*(r0.yyyy)+(r1.xxxx)).x;
    // 51: add r1.x, r1.x, cb0[15].z
    r1.x = ((r1.xxxx)+(source[15].zzzz)).x;
    // 52: mad r1.y, v4.x, cb0[13].y, cb0[13].z
    r1.y = ((v4.xxxx)*(source[13].yyyy)+(source[13].zzzz)).y;
    // 53: mad r1.y, r0.x, cb0[13].x, r1.y
    r1.y = ((r0.xxxx)*(source[13].xxxx)+(r1.yyyy)).y;
    // 54: add r1.y, r1.y, cb0[14].x
    r1.y = ((r1.yyyy)+(source[14].xxxx)).y;
    // 55: mad r2.x, cb0[14].y, r1.y, r1.x
    r2.x = ((source[14].yyyy)*(r1.yyyy)+(r1.xxxx)).x;
    // 56: mad r2.y, r1.x, cb0[15].w, r1.y
    r2.y = ((r1.xxxx)*(source[15].wwww)+(r1.yyyy)).y;
    // 57: mad r1.xy, cb0[16].xxxx, r0.zwzz, r2.xyxx
    r1.xy = ((source[16].xxxx)*(r0.zwzz)+(r2.xyxx)).xy;
    // 58: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s3, cb0[2].x
    r1.x = (ALTVNativeSample3((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).x;
    // 59: mul r1.x, r1.x, cb0[16].y
    r1.x = ((r1.xxxx)*(source[16].yyyy)).x;
    // 60: mad r1.y, v4.x, cb0[18].y, cb0[18].z
    r1.y = ((v4.xxxx)*(source[18].yyyy)+(source[18].zzzz)).y;
    // 61: add r1.y, r1.y, cb0[18].w
    r1.y = ((r1.yyyy)+(source[18].wwww)).y;
    // 62: mad r1.y, cb0[19].x, r0.y, r1.y
    r1.y = ((source[19].xxxx)*(r0.yyyy)+(r1.yyyy)).y;
    // 63: mad r1.z, v4.x, cb0[16].w, cb0[17].x
    r1.z = ((v4.xxxx)*(source[16].wwww)+(source[17].xxxx)).z;
    // 64: add r1.z, r1.z, cb0[17].z
    r1.z = ((r1.zzzz)+(source[17].zzzz)).z;
    // 65: mad r1.z, r0.x, cb0[16].z, r1.z
    r1.z = ((r0.xxxx)*(source[16].zzzz)+(r1.zzzz)).z;
    // 66: mad r2.x, cb0[17].w, r1.z, r1.y
    r2.x = ((source[17].wwww)*(r1.zzzz)+(r1.yyyy)).x;
    // 67: mad r2.y, r1.y, cb0[19].y, r1.z
    r2.y = ((r1.yyyy)*(source[19].yyyy)+(r1.zzzz)).y;
    // 68: mad r1.yz, cb0[19].zzzz, r0.zzwz, r2.xxyx
    r1.yz = ((source[19].zzzz)*(r0.zzwz)+(r2.xxyx)).yz;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t3.yxzw, s4, cb0[2].x
    r1.y = (ALTVNativeSample4((r1.yzyy).xy, (source[2].xxxx).x, true).yxzw).y;
    // 70: mul r1.y, r1.y, cb0[19].w
    r1.y = ((r1.yyyy)*(source[19].wwww)).y;
    // 71: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 72: mad r1.y, v4.x, cb0[21].w, cb0[22].x
    r1.y = ((v4.xxxx)*(source[21].wwww)+(source[22].xxxx)).y;
    // 73: mad r1.y, cb0[22].y, r0.y, r1.y
    r1.y = ((source[22].yyyy)*(r0.yyyy)+(r1.yyyy)).y;
    // 74: add r1.y, r1.y, cb0[22].z
    r1.y = ((r1.yyyy)+(source[22].zzzz)).y;
    // 75: mad r1.z, v4.x, cb0[20].y, cb0[20].z
    r1.z = ((v4.xxxx)*(source[20].yyyy)+(source[20].zzzz)).z;
    // 76: mad r1.z, r0.x, cb0[20].x, r1.z
    r1.z = ((r0.xxxx)*(source[20].xxxx)+(r1.zzzz)).z;
    // 77: add r1.z, r1.z, cb0[21].x
    r1.z = ((r1.zzzz)+(source[21].xxxx)).z;
    // 78: mad r2.x, cb0[21].y, r1.z, r1.y
    r2.x = ((source[21].yyyy)*(r1.zzzz)+(r1.yyyy)).x;
    // 79: mad r2.y, r1.y, cb0[22].w, r1.z
    r2.y = ((r1.yyyy)*(source[22].wwww)+(r1.zzzz)).y;
    // 80: mad r1.yz, cb0[23].xxxx, r0.zzwz, r2.xxyx
    r1.yz = ((source[23].xxxx)*(r0.zzwz)+(r2.xxyx)).yz;
    // 81: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t4.yxzw, s5, cb0[2].x
    r1.y = (ALTVNativeSample5((r1.yzyy).xy, (source[2].xxxx).x, true).yxzw).y;
    // 82: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 83: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 84: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 85: mul r1.y, v4.w, cb0[23].y
    r1.y = ((v4.wwww)*(source[23].yyyy)).y;
    // 86: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 87: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 88: mul_sat r1.x, r1.x, cb0[23].z
    r1.x = (saturate((r1.xxxx)*(source[23].zzzz))).x;
    // 89: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 90: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 91: mad r1.x, v4.x, cb0[4].y, cb0[4].z
    r1.x = ((v4.xxxx)*(source[4].yyyy)+(source[4].zzzz)).x;
    // 92: mad r0.y, cb0[4].w, r0.y, r1.x
    r0.y = ((source[4].wwww)*(r0.yyyy)+(r1.xxxx)).y;
    // 93: add r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)+(source[5].xxxx)).y;
    // 94: mad r1.x, v4.x, cb0[2].z, cb0[2].w
    r1.x = ((v4.xxxx)*(source[2].zzzz)+(source[2].wwww)).x;
    // 95: mad r0.x, r0.x, cb0[2].y, r1.x
    r0.x = ((r0.xxxx)*(source[2].yyyy)+(r1.xxxx)).x;
    // 96: add r0.x, r0.x, cb0[3].z
    r0.x = ((r0.xxxx)+(source[3].zzzz)).x;
    // 97: mad r1.x, cb0[3].w, r0.x, r0.y
    r1.x = ((source[3].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 98: mad r1.y, r0.y, cb0[5].y, r0.x
    r1.y = ((r0.yyyy)*(source[5].yyyy)+(r0.xxxx)).y;
    // 99: mad r0.xy, cb0[12].xxxx, r0.zwzz, r1.xyxx
    r0.xy = ((source[12].xxxx)*(r0.zwzz)+(r1.xyxx)).xy;
    // 100: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s0, cb0[2].x
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 101: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 103: mad r0.xyz, cb0[12].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[12].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 104: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 105: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 106: mul r0.xyz, r0.xyzx, cb0[12].zzzz
    r0.xyz = ((r0.xyzx)*(source[12].zzzz)).xyz;
    // 107: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 108: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 109: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 110: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_a_pa_ht_01_2_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ALTVNative194(ALTV_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[3u];
    source[2] = g_ALTVSourceMaterialParameters[2u];
    source[3] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[0u].wwww,g_ALTVSourceMaterialParameters[1u].xxxx,1u);
    source[4] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[1u].zzzz,g_ALTVSourceMaterialParameters[1u].wwww,1u);
    source[5].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[6].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[6].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[7].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, cb0[5].xxxx, cb0[3].xyxx, v2.xyxx
    r0.xy = ((source[5].xxxx)*(source[3].xyxx)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[5].wwww
    r0.xy = ((r0.xyxx)*(source[5].wwww)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 4: mad r0.zw, cb0[5].xxxx, cb0[4].xxxy, v2.xxxy
    r0.zw = ((source[5].xxxx)*(source[4].xxxy)+(v2.xxxy)).zw;
    // 5: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

// fx_d_post3ch_02_01_tr: 1d73a749ec48cf4098dce279345dfcbc; selected map f1ac659a839a9eb4d7004a8747ef1244188118666e52f6f5e4330911ee15dbb3.
float4 ALTVNative195(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[3u];
    source[2] = g_ALTVSourceMaterialParameters[0u];
    source[3] = g_ALTVSourceMaterialParameters[1u];
    source[4] = g_ALTVSourceMaterialParameters[2u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mad r0.zw, v4.yyyy, cb0[2].xxxy, r0.xxxy
    r0.zw = ((v4.yyyy)*(source[2].xxxy)+(r0.xxxy)).zw;
    // 4: sample_indexable(texture2d)(float,float,float,float) r1.x, r0.zwzz, t0.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r1.x = (Read_EffectSceneColor(LinearClampUVSampler, (r0.zwzz).xy).xyzw).x;
    // 5: mad r0.zw, v4.yyyy, cb0[3].xxxy, r0.xxxy
    r0.zw = ((v4.yyyy)*(source[3].xxxy)+(r0.xxxy)).zw;
    // 6: mad r0.xy, v4.yyyy, cb0[4].xyxx, r0.xyxx
    r0.xy = ((v4.yyyy)*(source[4].xyxx)+(r0.xyxx)).xy;
    // 7: sample_indexable(texture2d)(float,float,float,float) r1.z, r0.xyxx, t0.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r1.z = (Read_EffectSceneColor(LinearClampUVSampler, (r0.xyxx).xy).xyzw).z;
    // 8: sample_indexable(texture2d)(float,float,float,float) r1.y, r0.zwzz, t0.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r1.y = (Read_EffectSceneColor(LinearClampUVSampler, (r0.zwzz).xy).xyzw).y;
    // 9: mad r0.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 10: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 11: mul o0.w, v3.w, cb0[0].x
    output.w = ((v3.wwww)*(source[0].xxxx)).w;
    return output;
}

// bfx_d_pa_shine_02_ad_inst32535: e916b8b428ce2d4f9e6c9c1f921545f6; selected map 50be7092d8903a418d7eb61946212425360615ce93db44db2ffd3508fed4fce4.
float4 ALTVNative196(ALTV_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
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

// fx_m_pa_highpixelring_01_05-02_ad: d9f3da526f61294bbc774b9fcc81634f; selected map b55a9a321fafcb0509636bbdd7eeb87bdf6366483544abaa428daba4e9ecd346.
float4 ALTVNative197(ALTV_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[7u];
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = ALTVNativeAppend(ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[4].w = ((g_ALTVSourceMaterialParameters[6u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[5].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[5].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].zzzz)).x;
    source[5].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[6].y = ((g_ALTVSourceMaterialParameters[4u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[6].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[6].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[7].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[4u].wwww)).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[8].z = ((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[8].w = ((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].x = (ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[9].y = (ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[10].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[10].y = ((g_ALTVSourceMaterialParameters[2u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[2u].yyyy)).x;
    source[11].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[11].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[12].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[12].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[13].z = ((g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[13].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mad r0.x, cb0[6].x, v2.x, cb0[6].y
    r0.x = ((source[6].xxxx)*(v2.xxxx)+(source[6].yyyy)).x;
    // 2: mad r0.y, cb0[6].z, v2.y, cb0[7].x
    r0.y = ((source[6].zzzz)*(v2.yyyy)+(source[7].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, v4.zzzz
    r0.xy = ((r0.xyxx)+(v4.zzzz)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mad r1.x, cb0[4].z, v2.x, cb0[4].w
    r1.x = ((source[4].zzzz)*(v2.xxxx)+(source[4].wwww)).x;
    // 7: mad r1.y, v2.y, cb0[5].x, cb0[5].z
    r1.y = ((v2.yyyy)*(source[5].xxxx)+(source[5].zzzz)).y;
    // 8: add r0.zw, r1.xxxy, v4.zzzz
    r0.zw = ((r1.xxxy)+(v4.zzzz)).zw;
    // 9: mad r0.xy, cb0[7].yyyy, r0.xyxx, r0.zwzz
    r0.xy = ((source[7].yyyy)*(r0.xyxx)+(r0.zwzz)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 12: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)*(source[7].zzzz)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 16: add r0.y, v4.y, cb0[7].w
    r0.y = ((v4.yyyy)+(source[7].wwww)).y;
    // 17: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 18: mul_sat r0.xy, r0.xxxx, l(10.000000, 8.000000, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xxxx)*(float4(10.000000,8.000000,0.000000,0.000000)))).xy;
    // 19: add r0.y, -r0.y, r0.x
    r0.y = ((-(r0.yyyy))+(r0.xxxx)).y;
    // 20: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 21: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 22: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: mul r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)*(source[8].yyyy)).z;
    // 24: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 25: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 26: mul r1.xyz, v3.xyzx, cb0[2].xyzx
    r1.xyz = ((v3.xyzx)*(source[2].xyzx)).xyz;
    // 27: mad r0.zw, v2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), cb0[3].xxxy
    r0.zw = ((v2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(source[3].xxxy)).zw;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(0.000000)
    r0.z = (ALTVNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 29: mul r0.w, |r0.z|, |r0.z|
    r0.w = ((abs(r0.zzzz))*(abs(r0.zzzz))).w;
    // 30: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 31: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 32: movc r1.xyz, r0.zzzz, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 33: mad r0.yzw, r0.yyyy, r1.xxyz, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r0.yyyy)*(r1.xxyz)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 34: mad r1.xy, v2.xyxx, cb0[9].zwzz, cb0[10].ywyy
    r1.xy = ((v2.xyxx)*(source[9].zwzz)+(source[10].ywyy)).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.x = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 36: mad r1.xy, r1.xxxx, cb0[11].xxxx, v2.xyxx
    r1.xy = ((r1.xxxx)*(source[11].xxxx)+(v2.xyxx)).xy;
    // 37: mad r2.x, r1.x, cb0[11].y, cb0[11].w
    r2.x = ((r1.xxxx)*(source[11].yyyy)+(source[11].wwww)).x;
    // 38: mad r2.y, r1.y, cb0[11].z, cb0[12].x
    r2.y = ((r1.yyyy)*(source[11].zzzz)+(source[12].xxxx)).y;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = (ALTVNativeSample4((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 40: mul r1.xyz, r1.xyzx, cb0[12].yyyy
    r1.xyz = ((r1.xyzx)*(source[12].yyyy)).xyz;
    // 41: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 42: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 43: mul r1.xyz, r1.xyzx, cb0[12].zzzz
    r1.xyz = ((r1.xyzx)*(source[12].zzzz)).xyz;
    // 44: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 45: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 47: mad r1.xyz, cb0[12].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[12].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 48: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 49: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 50: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 51: mul r1.x, v4.x, cb0[13].w
    r1.x = ((v4.xxxx)*(source[13].wwww)).x;
    // 52: mul r1.x, r1.x, cb0[14].x
    r1.x = ((r1.xxxx)*(source[14].xxxx)).x;
    // 53: mul r1.yz, v2.xxyx, cb0[13].xxxx
    r1.yz = ((v2.xxyx)*(source[13].xxxx)).yz;
    // 54: frc r1.yz, r1.yyzy
    r1.yz = (frac(r1.yyzy)).yz;
    // 55: div r2.xy, r1.yzyy, r1.xxxx
    r2.xy = ((r1.yzyy)/(r1.xxxx)).xy;
    // 56: add r2.zw, -r1.yyyz, l(0.000000, 0.000000, 1.000000, 1.000000)
    r2.zw = ((-(r1.yyyz))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 57: div r1.yz, r1.yyzy, cb0[13].zzzz
    r1.yz = ((r1.yyzy)/(source[13].zzzz)).yz;
    // 58: round_ni r1.yz, r1.yyzy
    r1.yz = (floor(r1.yyzy)).yz;
    // 59: div r1.xw, r2.zzzw, r1.xxxx
    r1.xw = ((r2.zzzw)/(r1.xxxx)).xw;
    // 60: div r2.zw, r2.zzzw, cb0[13].zzzz
    r2.zw = ((r2.zzzw)/(source[13].zzzz)).zw;
    // 61: round_ni r2.xyzw, r2.xyzw
    r2.xyzw = (floor(r2.xyzw)).xyzw;
    // 62: mul r1.yz, r1.yyzy, r2.zzwz
    r1.yz = ((r1.yyzy)*(r2.zzwz)).yz;
    // 63: mul_sat r1.y, r1.z, r1.y
    r1.y = (saturate((r1.zzzz)*(r1.yyyy))).y;
    // 64: round_ni r1.xz, r1.xxwx
    r1.xz = (floor(r1.xxwx)).xz;
    // 65: mul r1.xz, r1.xxzx, r2.xxyx
    r1.xz = ((r1.xxzx)*(r2.xxyx)).xz;
    // 66: mul_sat r1.x, r1.z, r1.x
    r1.x = (saturate((r1.zzzz)*(r1.xxxx))).x;
    // 67: add r1.x, -r1.x, r1.y
    r1.x = ((-(r1.xxxx))+(r1.yyyy)).x;
    // 68: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 69: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 70: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 71: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 72: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 73: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_e_pa_ht_18_6_tr: 7d08e12e1633b740853481874caab82f; selected map acba13b00769908b86fcfbc037a5d05d502810885976df98a43dff8e6f1fca2f.
float4 ALTVNative198(ALTV_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[6u];
    source[2] = g_ALTVSourceMaterialParameters[5u];
    source[3] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[2u].wwww,g_ALTVSourceMaterialParameters[3u].xxxx,1u);
    source[4] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[0u].zzzz,g_ALTVSourceMaterialParameters[1u].xxxx,1u);
    source[5] = ALTVNativeAppend(ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[6] = ALTVNativeAppend(ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[2u].xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[2u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[0u].wwww,g_ALTVSourceMaterialParameters[1u].yyyy,1u);
    source[8] = ALTVNativeAppend(ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].wwww)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[9] = ALTVNativeAppend(ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[2u].yyyy)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[10].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[10].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].zzzz)).x;
    source[11].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[11].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].zzzz)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].x = (ALTVNativePeriodic(((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[2u].yyyy)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[13].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[3u].zzzz)).x;
    source[13].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[3u].zzzz))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
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
    // 10: add r0.y, -cb0[13].w, l(1.000000)
    r0.y = ((-(source[13].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 12: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 13: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 14: add r0.y, v2.x, l(-0.500000)
    r0.y = ((v2.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 15: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 16: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 17: mul r0.z, r0.y, v2.y
    r0.z = ((r0.yyyy)*(v2.yyyy)).z;
    // 18: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mad r0.y, r0.y, l(0.500000), r0.z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.zzzz)).y;
    // 20: sqrt r0.z, v2.y
    r0.z = (sqrt(v2.yyyy)).z;
    // 21: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 22: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 23: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 24: mul r0.z, r0.z, cb0[10].x
    r0.z = ((r0.zzzz)*(source[10].xxxx)).z;
    // 25: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 26: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 27: mul_sat r0.z, r0.y, cb0[13].x
    r0.z = (saturate((r0.yyyy)*(source[13].xxxx))).z;
    // 28: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 29: add r0.z, -v2.y, l(1.000000)
    r0.z = ((-(v2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 31: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 32: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 33: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 34: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 35: mad r0.xz, cb0[3].xxyx, v2.xxyx, v4.xxyx
    r0.xz = ((source[3].xxyx)*(v2.xxyx)+(v4.xxyx)).xz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.xz, r0.xzxx, t1.xzyw, s1, l(0.000000)
    r0.xz = (ALTVNativeSample0((r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).xz;
    // 37: mad r0.xz, cb0[11].zzzz, r0.xxzx, v2.xxyx
    r0.xz = ((source[11].zzzz)*(r0.xxzx)+(v2.xxyx)).xz;
    // 38: mad r1.x, cb0[4].x, r0.x, cb0[5].x
    r1.x = ((source[4].xxxx)*(r0.xxxx)+(source[5].xxxx)).x;
    // 39: mad r1.y, cb0[4].y, r0.z, cb0[6].y
    r1.y = ((source[4].yyyy)*(r0.zzzz)+(source[6].yyyy)).y;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 41: mad r1.x, cb0[7].x, r0.x, cb0[8].x
    r1.x = ((source[7].xxxx)*(r0.xxxx)+(source[8].xxxx)).x;
    // 42: mad r1.y, cb0[7].y, r0.z, cb0[9].y
    r1.y = ((source[7].yyyy)*(r0.zzzz)+(source[9].yyyy)).y;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r0.x = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 44: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 45: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 46: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 47: mul r0.z, r0.z, cb0[12].y
    r0.z = ((r0.zzzz)*(source[12].yyyy)).z;
    // 48: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 49: mul r0.z, r0.z, cb0[12].z
    r0.z = ((r0.zzzz)*(source[12].zzzz)).z;
    // 50: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 51: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
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

// fx_m_swp_boxcrack_basemaster_01-3_ad: 1fe38ae0c402e44bb0a9abf699e1096d; selected map 0961a5d1989f4fb20981d70a1a6d4783c919f5808ceeab25c0242a2c7666efd0.
float4 ALTVNative199(ALTV_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[17u];
    source[3] = g_ALTVSourceMaterialParameters[15u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[5].y = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[5].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].zzzz)).x;
    source[6].z = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[6].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[6u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[8].z = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[9].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[10].y = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[10].w = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[11].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[12].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].wwww)).x;
    source[13].x = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[13].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[14].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[14].y = ((g_ALTVSourceMaterialParameters[6u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[14].z = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[15].x = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[15].z = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[15].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[16].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].yyyy)).x;
    source[16].z = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[17].x = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[17].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[17].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[17].w = ((g_ALTVSourceMaterialParameters[7u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[18].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[18].z = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[18].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[19].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[19].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[19].z = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[19].w = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[20].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[20].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[20].z = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[20].w = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[21].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[21].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mad r0.x, cb0[4].x, cb0[15].z, cb0[15].w
    r0.x = ((source[4].xxxx)*(source[15].zzzz)+(source[15].wwww)).x;
    // 2: mad r0.x, v4.y, cb0[15].y, r0.x
    r0.x = ((v4.yyyy)*(source[15].yyyy)+(r0.xxxx)).x;
    // 3: add r0.x, r0.x, cb0[16].y
    r0.x = ((r0.xxxx)+(source[16].yyyy)).x;
    // 4: mad r0.y, cb0[4].x, cb0[17].x, cb0[17].y
    r0.y = ((source[4].xxxx)*(source[17].xxxx)+(source[17].yyyy)).y;
    // 5: mad r0.y, cb0[17].z, v4.x, r0.y
    r0.y = ((source[17].zzzz)*(v4.xxxx)+(r0.yyyy)).y;
    // 6: add r0.y, r0.y, cb0[17].w
    r0.y = ((r0.yyyy)+(source[17].wwww)).y;
    // 7: mad r1.x, cb0[16].z, r0.x, r0.y
    r1.x = ((source[16].zzzz)*(r0.xxxx)+(r0.yyyy)).x;
    // 8: mad r1.y, r0.y, cb0[18].x, r0.x
    r1.y = ((r0.yyyy)*(source[18].xxxx)+(r0.xxxx)).y;
    // 9: mad r0.x, cb0[4].y, cb0[8].z, cb0[8].w
    r0.x = ((source[4].yyyy)*(source[8].zzzz)+(source[8].wwww)).x;
    // 10: mad r0.x, v4.y, cb0[8].y, r0.x
    r0.x = ((v4.yyyy)*(source[8].yyyy)+(r0.xxxx)).x;
    // 11: mad r0.y, cb0[4].y, cb0[9].y, cb0[9].z
    r0.y = ((source[4].yyyy)*(source[9].yyyy)+(source[9].zzzz)).y;
    // 12: mad r0.y, cb0[9].w, v4.x, r0.y
    r0.y = ((source[9].wwww)*(v4.xxxx)+(r0.yyyy)).y;
    // 13: mad r2.x, cb0[9].x, r0.x, r0.y
    r2.x = ((source[9].xxxx)*(r0.xxxx)+(r0.yyyy)).x;
    // 14: mad r2.y, r0.y, cb0[10].x, r0.x
    r2.y = ((r0.yyyy)*(source[10].xxxx)+(r0.xxxx)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 16: mul r0.xy, r0.xyxx, cb0[10].yyyy
    r0.xy = ((r0.xyxx)*(source[10].yyyy)).xy;
    // 17: mul r0.xy, r0.xyxx, cb0[4].zzzz
    r0.xy = ((r0.xyxx)*(source[4].zzzz)).xy;
    // 18: mad r0.zw, cb0[18].yyyy, r0.xxxy, r1.xxxy
    r0.zw = ((source[18].yyyy)*(r0.xxxy)+(r1.xxxy)).zw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (ALTVNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 20: mul r0.z, r0.z, cb0[18].z
    r0.z = ((r0.zzzz)*(source[18].zzzz)).z;
    // 21: mad r0.w, cb0[4].x, cb0[12].x, cb0[12].y
    r0.w = ((source[4].xxxx)*(source[12].xxxx)+(source[12].yyyy)).w;
    // 22: mad r0.w, v4.y, cb0[11].w, r0.w
    r0.w = ((v4.yyyy)*(source[11].wwww)+(r0.wwww)).w;
    // 23: add r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)+(source[12].wwww)).w;
    // 24: mad r1.x, cb0[4].x, cb0[13].z, cb0[13].w
    r1.x = ((source[4].xxxx)*(source[13].zzzz)+(source[13].wwww)).x;
    // 25: mad r1.x, cb0[14].x, v4.x, r1.x
    r1.x = ((source[14].xxxx)*(v4.xxxx)+(r1.xxxx)).x;
    // 26: add r1.x, r1.x, cb0[14].y
    r1.x = ((r1.xxxx)+(source[14].yyyy)).x;
    // 27: mad r2.x, cb0[13].x, r0.w, r1.x
    r2.x = ((source[13].xxxx)*(r0.wwww)+(r1.xxxx)).x;
    // 28: mad r2.y, r1.x, cb0[14].z, r0.w
    r2.y = ((r1.xxxx)*(source[14].zzzz)+(r0.wwww)).y;
    // 29: mad r1.xy, cb0[14].wwww, r0.xyxx, r2.xyxx
    r1.xy = ((source[14].wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 31: mad r0.z, cb0[15].x, r0.w, r0.z
    r0.z = ((source[15].xxxx)*(r0.wwww)+(r0.zzzz)).z;
    // 32: mad r0.w, cb0[4].x, cb0[19].x, cb0[19].y
    r0.w = ((source[4].xxxx)*(source[19].xxxx)+(source[19].yyyy)).w;
    // 33: mad r0.w, v4.y, cb0[18].w, r0.w
    r0.w = ((v4.yyyy)*(source[18].wwww)+(r0.wwww)).w;
    // 34: mad r1.x, cb0[4].x, cb0[19].w, cb0[20].x
    r1.x = ((source[4].xxxx)*(source[19].wwww)+(source[20].xxxx)).x;
    // 35: mad r1.x, cb0[20].y, v4.x, r1.x
    r1.x = ((source[20].yyyy)*(v4.xxxx)+(r1.xxxx)).x;
    // 36: mad r2.x, cb0[19].z, r0.w, r1.x
    r2.x = ((source[19].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 37: mad r2.y, r1.x, cb0[20].z, r0.w
    r2.y = ((r1.xxxx)*(source[20].zzzz)+(r0.wwww)).y;
    // 38: mad r1.xy, cb0[20].wwww, r0.xyxx, r2.xyxx
    r1.xy = ((source[20].wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s4, l(0.000000)
    r0.w = (ALTVNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 40: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 41: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 42: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 43: mul r0.w, cb0[4].w, cb0[21].x
    r0.w = ((source[4].wwww)*(source[21].xxxx)).w;
    // 44: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 45: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 46: mul_sat r0.z, r0.z, cb0[21].y
    r0.z = (saturate((r0.zzzz)*(source[21].yyyy))).z;
    // 47: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 48: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 49: mad r0.w, cb0[4].x, cb0[5].y, cb0[5].z
    r0.w = ((source[4].xxxx)*(source[5].yyyy)+(source[5].zzzz)).w;
    // 50: mad r0.w, v4.y, cb0[5].x, r0.w
    r0.w = ((v4.yyyy)*(source[5].xxxx)+(r0.wwww)).w;
    // 51: add r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)+(source[6].yyyy)).w;
    // 52: mad r1.x, cb0[4].x, cb0[7].x, cb0[7].y
    r1.x = ((source[4].xxxx)*(source[7].xxxx)+(source[7].yyyy)).x;
    // 53: mad r1.x, cb0[7].z, v4.x, r1.x
    r1.x = ((source[7].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 54: add r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)+(source[7].wwww)).x;
    // 55: mad r2.x, cb0[6].z, r0.w, r1.x
    r2.x = ((source[6].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 56: mad r2.y, r1.x, cb0[8].x, r0.w
    r2.y = ((r1.xxxx)*(source[8].xxxx)+(r0.wwww)).y;
    // 57: mad r0.xy, cb0[10].zzzz, r0.xyxx, r2.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(r2.xyxx)).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t4.xywz, s1, l(0.000000)
    r0.xyw = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 59: dp3 r1.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 60: add r1.xyz, -r0.xywx, r1.xxxx
    r1.xyz = ((-(r0.xywx))+(r1.xxxx)).xyz;
    // 61: mad r0.xyw, cb0[10].wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((source[10].wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 62: max r0.xyw, |r0.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r0.xyw = (max(abs(r0.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 63: log r0.xyw, r0.xyxw
    r0.xyw = (log2(r0.xyxw)).xyw;
    // 64: mul r0.xyw, r0.xyxw, cb0[11].xxxx
    r0.xyw = ((r0.xyxw)*(source[11].xxxx)).xyw;
    // 65: exp r0.xyw, r0.xyxw
    r0.xyw = (exp2(r0.xyxw)).xyw;
    // 66: mad r0.xyw, cb0[11].yyyy, r0.xyxw, cb0[11].zzzz
    r0.xyw = ((source[11].yyyy)*(r0.xyxw)+(source[11].zzzz)).xyw;
    // 67: mul r0.xyw, r0.xyxw, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)).xyw;
    // 68: mad r0.xyw, cb0[3].xyxz, r0.xyxw, cb0[2].xyxz
    r0.xyw = ((source[3].xyxz)*(r0.xyxw)+(source[2].xyxz)).xyw;
    // 69: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 70: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 71: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_fsm_rgbsplit_01_4_ad: 07c909538ced5d438a51037ad2e95200; selected map 059383a9d7a75a34bf8fc4fff304e255bcf21cd7089dd63bb8536adafc931b30.
float4 ALTVNative200(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[0u].wwww,g_ALTVSourceMaterialParameters[1u].xxxx,1u);
    source[3].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.xy, v2.xyxx, cb0[2].xyxx
    r0.xy = ((v2.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.xyz, r0.xxxx, l(0.100000, 0.500000, 0.100000, 0.000000)
    r0.xyz = ((r0.xxxx)*(float4(0.100000,0.500000,0.100000,0.000000))).xyz;
    // 4: mul r1.xz, v4.xxxx, l(0.100000, 0.000000, -0.100000, 0.000000)
    r1.xz = ((v4.xxxx)*(float4(0.100000,0.000000,-0.100000,0.000000))).xz;
    // 5: mov r1.yw, l(0,0,0,0)
    r1.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yw;
    // 6: mad r1.xyzw, v2.xyxy, cb0[2].xyxy, r1.xyzw
    r1.xyzw = ((v2.xyxy)*(source[2].xyxy)+(r1.xyzw)).xyzw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s1, l(0.000000)
    r0.w = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t1.xyzw, s1, l(0.000000)
    r1.x = (ALTVNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    // 17: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 18: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 19: source device depth mapped to centimetre view depth; reconstruction at 21.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 21-24: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 25: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 26: add r1.x, -cb0[4].x, l(1.000000)
    r1.x = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 28: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 29: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 30: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 31: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 32: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_whkf_depth_shine_04_03_ad: e5cc0e7e816841418dbb0ca414d52ae7; selected map 303732d7099bb502524a120300fd58270b9c0b4b59114424a1f79c7aed4cb5f5.
float4 ALTVNative201(ALTV_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[5u];
    source[2] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[5].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[5].w = (clamp(g_ALTVSourceMaterialParameters[4u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ALTVSourceMaterialParameters[4u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[6].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[7].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[8].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[9].w = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[10].x = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx))).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
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
    // 7: mul_sat r0.y, r0.y, cb0[9].x
    r0.y = (saturate((r0.yyyy)*(source[9].xxxx))).y;
    // 8: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 9: div r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)/(r0.zzzz)).x;
    // 10: mad_sat r1.x, r0.x, l(0.500000), l(0.500000)
    r1.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 11: add r0.x, -r1.x, l(1.000000)
    r0.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)*(r0.xxxx)).x;
    // 13: add r0.zw, r1.xxxy, v4.xxxx
    r0.zw = ((r1.xxxy)+(v4.xxxx)).zw;
    // 14: mul r1.x, r0.x, l(4.000000)
    r1.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 15: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 16: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 17: mul r1.x, r1.x, cb0[8].x
    r1.x = ((r1.xxxx)*(source[8].xxxx)).x;
    // 18: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 19: mul r1.x, r1.x, cb0[8].y
    r1.x = ((r1.xxxx)*(source[8].yyyy)).x;
    // 20: movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 21: log r1.x, |r1.y|
    r1.x = (log2(abs(r1.yyyy))).x;
    // 22: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: mul r1.x, r1.x, cb0[8].z
    r1.x = ((r1.xxxx)*(source[8].zzzz)).x;
    // 24: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 25: mul r1.x, r1.x, cb0[8].w
    r1.x = ((r1.xxxx)*(source[8].wwww)).x;
    // 26: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 27: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 28: mul r1.x, cb0[4].y, cb0[6].z
    r1.x = ((source[4].yyyy)*(source[6].zzzz)).x;
    // 29: mad r1.x, cb0[6].w, r0.z, r1.x
    r1.x = ((source[6].wwww)*(r0.zzzz)+(r1.xxxx)).x;
    // 30: mul r1.z, r0.w, cb0[7].x
    r1.z = ((r0.wwww)*(source[7].xxxx)).z;
    // 31: mul r0.zw, r0.zzzw, cb0[4].zzzw
    r0.zw = ((r0.zzzw)*(source[4].zzzw)).zw;
    // 32: mad r1.y, cb0[4].y, cb0[7].y, r1.z
    r1.y = ((source[4].yyyy)*(source[7].yyyy)+(r1.zzzz)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 34: mad r2.x, cb0[4].y, cb0[4].x, r0.z
    r2.x = ((source[4].yyyy)*(source[4].xxxx)+(r0.zzzz)).x;
    // 35: mad r2.y, cb0[4].y, cb0[6].y, r0.w
    r2.y = ((source[4].yyyy)*(source[6].yyyy)+(r0.wwww)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 37: mul r0.z, r1.x, r0.z
    r0.z = ((r1.xxxx)*(r0.zzzz)).z;
    // 38: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 39: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 40: mul r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)*(source[7].zzzz)).z;
    // 41: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 42: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 43: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 44: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 45: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 46: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 47: source device depth mapped to centimetre view depth; reconstruction at 49.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 49-52: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 53: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 54: add r0.w, -cb0[10].x, l(1.000000)
    r0.w = ((-(source[10].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 56: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 57: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 58: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 59: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 60: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 61: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 62: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 63: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 64: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 65: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 66: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 67: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 68: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 69: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 70: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 71: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_q_pa_shine_01_ad: a15674efb9ac3743b395931c760fdffc; selected map 793a1a4a641f2eb4649d38a6f49f87f90de3bbda83d829d4857c2ecd320d253a.
float4 ALTVNative202(ALTV_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[6u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[5u].xxxx,g_ALTVSourceMaterialParameters[5u].yyyy,1u);
    source[3] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[6].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)).x;
    source[6].w = (((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[7].x = (clamp(g_ALTVSourceMaterialParameters[4u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ALTVSourceMaterialParameters[4u].zzzz,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[8].w = (g_ALTVSourceMaterialTime.xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[9].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[10].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[10].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].w = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp4 r0.x, cb0[3].xyxy, r0.xyzw
    r0.x = (dot((source[3].xyxy).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 3: dp2 r0.y, cb0[4].xyxx, r0.zwzz
    r0.y = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: add r1.y, r0.y, l(0.500000)
    r1.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 5: add r0.y, -r1.y, l(1.000000)
    r0.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 6: mad r0.z, r0.y, cb0[7].x, cb0[7].y
    r0.z = ((r0.yyyy)*(source[7].xxxx)+(source[7].yyyy)).z;
    // 7: mul_sat r0.y, r0.y, cb0[11].z
    r0.y = (saturate((r0.yyyy)*(source[11].zzzz))).y;
    // 8: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 9: div r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)/(r0.zzzz)).x;
    // 10: mad_sat r1.x, r0.x, l(0.500000), l(0.500000)
    r1.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 11: add r0.x, -r1.x, l(1.000000)
    r0.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)*(r0.xxxx)).x;
    // 13: mul r0.z, r0.x, l(4.000000)
    r0.z = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).z;
    // 14: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 15: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 16: mul r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)*(source[7].zzzz)).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 19: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 20: log r0.z, |r1.y|
    r0.z = (log2(abs(r1.yyyy))).z;
    // 21: mul r0.z, r0.z, cb0[8].x
    r0.z = ((r0.zzzz)*(source[8].xxxx)).z;
    // 22: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 23: mul r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)*(source[8].yyyy)).z;
    // 24: lt r0.w, |r1.y|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 25: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 26: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 27: mul r0.zw, r1.xxxy, cb0[9].xxxy
    r0.zw = ((r1.xxxy)*(source[9].xxxy)).zw;
    // 28: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 29: mad r2.x, cb0[8].w, cb0[8].z, r0.z
    r2.x = ((source[8].wwww)*(source[8].zzzz)+(r0.zzzz)).x;
    // 30: mad r2.y, cb0[8].w, cb0[9].z, r0.w
    r2.y = ((source[8].wwww)*(source[9].zzzz)+(r0.wwww)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mad r2.x, cb0[8].w, cb0[9].w, r1.x
    r2.x = ((source[8].wwww)*(source[9].wwww)+(r1.xxxx)).x;
    // 33: mad r2.y, cb0[8].w, cb0[10].z, r1.y
    r2.y = ((source[8].wwww)*(source[10].zzzz)+(r1.yyyy)).y;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 35: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 36: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 37: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 38: mul r0.z, r0.z, cb0[10].w
    r0.z = ((r0.zzzz)*(source[10].wwww)).z;
    // 39: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 40: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 41: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 42: mul_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)*(r0.xxxx))).x;
    // 43: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 44: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[11].w
    r0.z = ((r0.zzzz)*(source[11].wwww)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 48: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 49: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 50: mul r1.xyz, r0.zzzz, v6.xyzx
    r1.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 51: log r0.z, |r1.z|
    r0.z = (log2(abs(r1.zzzz))).z;
    // 52: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 53: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 54: lt r0.w, |r1.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)*(source[2].xyxx)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s0, l(0.000000)
    r1.xyz = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 57: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 58: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 59: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 60: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 61: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 62: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 63: add r0.yzw, -r1.xxyz, r0.yyyy
    r0.yzw = ((-(r1.xxyz))+(r0.yyyy)).yzw;
    // 64: mad r0.yzw, cb0[5].zzzz, r0.yyzw, r1.xxyz
    r0.yzw = ((source[5].zzzz)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 65: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 66: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 67: mul r0.yzw, r0.yyzw, cb0[5].wwww
    r0.yzw = ((r0.yyzw)*(source[5].wwww)).yzw;
    // 68: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 69: mul r0.yzw, r0.yyzw, cb0[6].xxxx
    r0.yzw = ((r0.yyzw)*(source[6].xxxx)).yzw;
    // 70: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 71: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 72: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 73: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_pa_basic_01_ad: 5ea6abf36654c94eae8b523062def76d; selected map ddfa6780fd232822c32a57bef702012872510d7c744aec89366ef16885fdc498.
float4 ALTVNative203(ALTV_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[2].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t0.yzxw, s1, l(0.000000)
    r0.z = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t1.xywz, s0, l(0.000000)
    r0.xyw = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 7: log r1.x, |r0.z|
    r1.x = (log2(abs(r0.zzzz))).x;
    // 8: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 9: mul r1.x, r1.x, cb0[2].w
    r1.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 10: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 11: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 12: movc r0.z, r0.z, l(0), |r1.x|
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).z;
    // 13: log r1.x, r0.z
    r1.x = (log2(r0.zzzz)).x;
    // 14: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 15: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 16: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 17: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 18: movc r0.z, r0.z, l(0), r1.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).z;
    // 19: mul r1.xyz, r0.xywx, cb0[2].yyyy
    r1.xyz = ((r0.xywx)*(source[2].yyyy)).xyz;
    // 20: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyw, -cb0[2].yyyy, r0.xyxw, r1.wwww
    r0.xyw = ((-(source[2].yyyy))*(r0.xyxw)+(r1.wwww)).xyw;
    // 22: mad r0.xyw, cb0[2].zzzz, r0.xyxw, r1.xyxz
    r0.xyw = ((source[2].zzzz)*(r0.xyxw)+(r1.xyxz)).xyw;
    // 23: mad r0.xyw, r0.xyxw, v3.xyxz, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(v3.xyxz)+(source[1].xyxz)).xyw;
    // 24: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 25: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 26: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_pa_swp_cubeshine_03_ad: 0d2d84dda92c764e90d9dc2c020b436d; selected map 7647226c4b4ccaca10c3773c760ebe0517ee7bde315c7f82bf7f4c2dbd69e896.
float4 ALTVNative204(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[12u];
    source[2].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[2].y = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[2].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[2].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[3].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].zzzz)).x;
    source[3].z = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[3].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[4].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[4].w = ((g_ALTVSourceMaterialParameters[5u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[6].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[7].w = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[9].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[9].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[5u].wwww)).x;
    source[9].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[10].x = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[10].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[11].x = ((g_ALTVSourceMaterialParameters[5u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[11].y = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[12].x = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[12].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].yyyy)).x;
    source[13].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[13].z = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[13].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[14].y = ((g_ALTVSourceMaterialParameters[6u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[14].z = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[15].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
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
    // 28: mad r0.z, v4.y, cb0[6].y, cb0[6].z
    r0.z = ((v4.yyyy)*(source[6].yyyy)+(source[6].zzzz)).z;
    // 29: mad r0.z, cb0[6].w, r0.y, r0.z
    r0.z = ((source[6].wwww)*(r0.yyyy)+(r0.zzzz)).z;
    // 30: mad r0.w, v4.y, cb0[5].z, cb0[5].w
    r0.w = ((v4.yyyy)*(source[5].zzzz)+(source[5].wwww)).w;
    // 31: mad r0.w, r0.x, cb0[5].y, r0.w
    r0.w = ((r0.xxxx)*(source[5].yyyy)+(r0.wwww)).w;
    // 32: mad r1.x, cb0[6].x, r0.w, r0.z
    r1.x = ((source[6].xxxx)*(r0.wwww)+(r0.zzzz)).x;
    // 33: mad r1.y, r0.z, cb0[7].x, r0.w
    r1.y = ((r0.zzzz)*(source[7].xxxx)+(r0.wwww)).y;
    // 34: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s2, l(-1.000000)
    r0.zw = (ALTVNativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zwxy).zw;
    // 35: mul r0.zw, r0.zzzw, cb0[7].yyyy
    r0.zw = ((r0.zzzw)*(source[7].yyyy)).zw;
    // 36: mul r0.zw, r0.zzzw, v4.zzzz
    r0.zw = ((r0.zzzw)*(v4.zzzz)).zw;
    // 37: mad r1.x, v4.x, cb0[10].y, cb0[10].z
    r1.x = ((v4.xxxx)*(source[10].yyyy)+(source[10].zzzz)).x;
    // 38: mad r1.x, cb0[10].w, r0.y, r1.x
    r1.x = ((source[10].wwww)*(r0.yyyy)+(r1.xxxx)).x;
    // 39: add r1.x, r1.x, cb0[11].x
    r1.x = ((r1.xxxx)+(source[11].xxxx)).x;
    // 40: mad r1.y, v4.x, cb0[8].w, cb0[9].x
    r1.y = ((v4.xxxx)*(source[8].wwww)+(source[9].xxxx)).y;
    // 41: mad r1.y, r0.x, cb0[8].z, r1.y
    r1.y = ((r0.xxxx)*(source[8].zzzz)+(r1.yyyy)).y;
    // 42: add r1.y, r1.y, cb0[9].z
    r1.y = ((r1.yyyy)+(source[9].zzzz)).y;
    // 43: mad r2.x, cb0[9].w, r1.y, r1.x
    r2.x = ((source[9].wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 44: mad r2.y, r1.x, cb0[11].y, r1.y
    r2.y = ((r1.xxxx)*(source[11].yyyy)+(r1.yyyy)).y;
    // 45: mad r1.xy, cb0[11].zzzz, r0.zwzz, r2.xyxx
    r1.xy = ((source[11].zzzz)*(r0.zwzz)+(r2.xyxx)).xy;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s3, l(-1.000000)
    r1.x = (ALTVNativeSample2((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 47: mad r1.y, v4.x, cb0[13].z, cb0[13].w
    r1.y = ((v4.xxxx)*(source[13].zzzz)+(source[13].wwww)).y;
    // 48: mad r1.y, cb0[14].x, r0.y, r1.y
    r1.y = ((source[14].xxxx)*(r0.yyyy)+(r1.yyyy)).y;
    // 49: add r1.y, r1.y, cb0[14].y
    r1.y = ((r1.yyyy)+(source[14].yyyy)).y;
    // 50: mad r1.z, v4.x, cb0[12].x, cb0[12].y
    r1.z = ((v4.xxxx)*(source[12].xxxx)+(source[12].yyyy)).z;
    // 51: mad r1.z, r0.x, cb0[11].w, r1.z
    r1.z = ((r0.xxxx)*(source[11].wwww)+(r1.zzzz)).z;
    // 52: add r1.z, r1.z, cb0[12].w
    r1.z = ((r1.zzzz)+(source[12].wwww)).z;
    // 53: mad r2.x, cb0[13].x, r1.z, r1.y
    r2.x = ((source[13].xxxx)*(r1.zzzz)+(r1.yyyy)).x;
    // 54: mad r2.y, r1.y, cb0[14].z, r1.z
    r2.y = ((r1.yyyy)*(source[14].zzzz)+(r1.zzzz)).y;
    // 55: mad r1.yz, cb0[14].wwww, r0.zzwz, r2.xxyx
    r1.yz = ((source[14].wwww)*(r0.zzwz)+(r2.xxyx)).yz;
    // 56: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t2.yxzw, s4, l(-1.000000)
    r1.y = (ALTVNativeSample3((r1.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 57: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 58: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 59: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 60: mul r1.y, v4.w, cb0[15].x
    r1.y = ((v4.wwww)*(source[15].xxxx)).y;
    // 61: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 62: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 63: mul_sat r1.x, r1.x, cb0[15].y
    r1.x = (saturate((r1.xxxx)*(source[15].yyyy))).x;
    // 64: div r1.yz, v7.xxyx, v7.wwww
    r1.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 65: mad r1.yz, r1.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r1.yz = ((r1.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 66: source device depth mapped to centimetre view depth; reconstruction at 68.
    r1.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.yzyy).xy, 0.f).y * 100000.f;
    // Native 68-71: reconstructed view depth is supplied by the runtime adapter.
    r1.y = r1.y;
    // 72: add r1.y, r1.y, -v7.w
    r1.y = ((r1.yyyy)+(-(v7.wwww))).y;
    // 73: add r1.z, -cb0[15].z, l(1.000000)
    r1.z = ((-(source[15].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 75: div_sat r1.y, r1.y, r1.z
    r1.y = (saturate((r1.yyyy)/(r1.zzzz))).y;
    // 76: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 77: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 78: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 79: mad r1.y, v4.x, cb0[4].x, cb0[4].y
    r1.y = ((v4.xxxx)*(source[4].xxxx)+(source[4].yyyy)).y;
    // 80: mad r0.y, cb0[4].z, r0.y, r1.y
    r0.y = ((source[4].zzzz)*(r0.yyyy)+(r1.yyyy)).y;
    // 81: add r0.y, r0.y, cb0[4].w
    r0.y = ((r0.yyyy)+(source[4].wwww)).y;
    // 82: mad r1.y, v4.x, cb0[2].y, cb0[2].z
    r1.y = ((v4.xxxx)*(source[2].yyyy)+(source[2].zzzz)).y;
    // 83: mad r0.x, r0.x, cb0[2].x, r1.y
    r0.x = ((r0.xxxx)*(source[2].xxxx)+(r1.yyyy)).x;
    // 84: add r0.x, r0.x, cb0[3].y
    r0.x = ((r0.xxxx)+(source[3].yyyy)).x;
    // 85: mad r2.x, cb0[3].z, r0.x, r0.y
    r2.x = ((source[3].zzzz)*(r0.xxxx)+(r0.yyyy)).x;
    // 86: mad r2.y, r0.y, cb0[5].x, r0.x
    r2.y = ((r0.yyyy)*(source[5].xxxx)+(r0.xxxx)).y;
    // 87: mad r0.xy, cb0[7].zzzz, r0.zwzz, r2.xyxx
    r0.xy = ((source[7].zzzz)*(r0.zwzz)+(r2.xyxx)).xy;
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(-1.000000)
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 89: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r1.yzw, -r0.xxyz, r0.wwww
    r1.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 91: mad r0.xyz, cb0[7].wwww, r1.yzwy, r0.xyzx
    r0.xyz = ((source[7].wwww)*(r1.yzwy)+(r0.xyzx)).xyz;
    // 92: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 93: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 94: mul r0.xyz, r0.xyzx, cb0[8].xxxx
    r0.xyz = ((r0.xyzx)*(source[8].xxxx)).xyz;
    // 95: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 96: mul r0.xyz, r0.xyzx, cb0[8].yyyy
    r0.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 97: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 98: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 99: mul o0.xyz, r1.xxxx, r0.xyzx
    output.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 100: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_pa_swp_cubeshine_05_a4: f09b8dda9387e745b0a64ee73072beac; selected map 108fbba71459f54210f79de1d25fc9105465c21c98de83cb09d0f7a477cb6ca6.
float4 ALTVNative205(ALTV_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[16u];
    source[2].x = (g_ALTVSourceMaterialParameters[15u].wwww).x;
    source[2].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[2].z = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[2].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[3].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[3].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].yyyy)).x;
    source[3].w = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[4].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[5].x = ((g_ALTVSourceMaterialParameters[6u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[5].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[5].w = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[6].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[6].w = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[7].x = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[7].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[6u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[8].z = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[10].x = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[10].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].xxxx)).x;
    source[11].x = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[12].y = ((g_ALTVSourceMaterialParameters[6u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[12].z = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[12].w = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[13].z = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[14].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[14].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].zzzz)).x;
    source[14].z = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[15].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[15].z = ((g_ALTVSourceMaterialParameters[7u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[15].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[16].y = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[16].z = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[16].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[17].x = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[17].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[17].z = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[17].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].xxxx)).x;
    source[18].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[18].z = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[18].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[19].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[19].y = ((g_ALTVSourceMaterialParameters[7u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[19].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[19].w = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[20].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[20].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[20].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
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
    // 28: mad r0.z, v4.x, cb0[11].z, cb0[11].w
    r0.z = ((v4.xxxx)*(source[11].zzzz)+(source[11].wwww)).z;
    // 29: mad r0.z, cb0[12].x, r0.y, r0.z
    r0.z = ((source[12].xxxx)*(r0.yyyy)+(r0.zzzz)).z;
    // 30: add r0.z, r0.z, cb0[12].y
    r0.z = ((r0.zzzz)+(source[12].yyyy)).z;
    // 31: mad r0.w, v4.x, cb0[10].x, cb0[10].y
    r0.w = ((v4.xxxx)*(source[10].xxxx)+(source[10].yyyy)).w;
    // 32: mad r0.w, r0.x, cb0[9].w, r0.w
    r0.w = ((r0.xxxx)*(source[9].wwww)+(r0.wwww)).w;
    // 33: add r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)+(source[10].wwww)).w;
    // 34: mad r1.x, cb0[11].x, r0.w, r0.z
    r1.x = ((source[11].xxxx)*(r0.wwww)+(r0.zzzz)).x;
    // 35: mad r1.y, r0.z, cb0[12].z, r0.w
    r1.y = ((r0.zzzz)*(source[12].zzzz)+(r0.wwww)).y;
    // 36: mad r0.z, v4.y, cb0[5].w, cb0[6].x
    r0.z = ((v4.yyyy)*(source[5].wwww)+(source[6].xxxx)).z;
    // 37: add r0.z, r0.z, cb0[6].z
    r0.z = ((r0.zzzz)+(source[6].zzzz)).z;
    // 38: mad r0.z, v2.y, cb0[5].z, r0.z
    r0.z = ((v2.yyyy)*(source[5].zzzz)+(r0.zzzz)).z;
    // 39: mad r0.w, v4.y, cb0[7].x, cb0[7].y
    r0.w = ((v4.yyyy)*(source[7].xxxx)+(source[7].yyyy)).w;
    // 40: add r0.w, r0.w, cb0[7].w
    r0.w = ((r0.wwww)+(source[7].wwww)).w;
    // 41: mad r0.w, cb0[8].x, v2.x, r0.w
    r0.w = ((source[8].xxxx)*(v2.xxxx)+(r0.wwww)).w;
    // 42: mad r2.x, cb0[6].w, r0.z, r0.w
    r2.x = ((source[6].wwww)*(r0.zzzz)+(r0.wwww)).x;
    // 43: mad r2.y, r0.w, cb0[8].y, r0.z
    r2.y = ((r0.wwww)*(source[8].yyyy)+(r0.zzzz)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s2, cb0[2].x
    r0.zw = (ALTVNativeSample1((r2.xyxx).xy, (source[2].xxxx).x, true).zwxy).zw;
    // 45: mul r0.zw, r0.zzzw, cb0[8].zzzz
    r0.zw = ((r0.zzzw)*(source[8].zzzz)).zw;
    // 46: mul r0.zw, r0.zzzw, v4.zzzz
    r0.zw = ((r0.zzzw)*(v4.zzzz)).zw;
    // 47: mad r1.xy, cb0[12].wwww, r0.zwzz, r1.xyxx
    r1.xy = ((source[12].wwww)*(r0.zwzz)+(r1.xyxx)).xy;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s3, cb0[2].x
    r1.x = (ALTVNativeSample2((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).x;
    // 49: mul r1.x, r1.x, cb0[13].x
    r1.x = ((r1.xxxx)*(source[13].xxxx)).x;
    // 50: mad r1.y, v4.x, cb0[15].x, cb0[15].y
    r1.y = ((v4.xxxx)*(source[15].xxxx)+(source[15].yyyy)).y;
    // 51: add r1.y, r1.y, cb0[15].z
    r1.y = ((r1.yyyy)+(source[15].zzzz)).y;
    // 52: mad r1.y, cb0[15].w, r0.y, r1.y
    r1.y = ((source[15].wwww)*(r0.yyyy)+(r1.yyyy)).y;
    // 53: mad r1.z, v4.x, cb0[13].z, cb0[13].w
    r1.z = ((v4.xxxx)*(source[13].zzzz)+(source[13].wwww)).z;
    // 54: add r1.z, r1.z, cb0[14].y
    r1.z = ((r1.zzzz)+(source[14].yyyy)).z;
    // 55: mad r1.z, r0.x, cb0[13].y, r1.z
    r1.z = ((r0.xxxx)*(source[13].yyyy)+(r1.zzzz)).z;
    // 56: mad r2.x, cb0[14].z, r1.z, r1.y
    r2.x = ((source[14].zzzz)*(r1.zzzz)+(r1.yyyy)).x;
    // 57: mad r2.y, r1.y, cb0[16].x, r1.z
    r2.y = ((r1.yyyy)*(source[16].xxxx)+(r1.zzzz)).y;
    // 58: mad r1.yz, cb0[16].yyyy, r0.zzwz, r2.xxyx
    r1.yz = ((source[16].yyyy)*(r0.zzwz)+(r2.xxyx)).yz;
    // 59: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t2.yxzw, s4, cb0[2].x
    r1.y = (ALTVNativeSample3((r1.yzyy).xy, (source[2].xxxx).x, true).yxzw).y;
    // 60: mul r1.y, r1.y, cb0[16].z
    r1.y = ((r1.yyyy)*(source[16].zzzz)).y;
    // 61: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 62: mad r1.y, v4.x, cb0[18].z, cb0[18].w
    r1.y = ((v4.xxxx)*(source[18].zzzz)+(source[18].wwww)).y;
    // 63: mad r1.y, cb0[19].x, r0.y, r1.y
    r1.y = ((source[19].xxxx)*(r0.yyyy)+(r1.yyyy)).y;
    // 64: add r1.y, r1.y, cb0[19].y
    r1.y = ((r1.yyyy)+(source[19].yyyy)).y;
    // 65: mad r1.z, v4.x, cb0[17].x, cb0[17].y
    r1.z = ((v4.xxxx)*(source[17].xxxx)+(source[17].yyyy)).z;
    // 66: mad r1.z, r0.x, cb0[16].w, r1.z
    r1.z = ((r0.xxxx)*(source[16].wwww)+(r1.zzzz)).z;
    // 67: add r1.z, r1.z, cb0[17].w
    r1.z = ((r1.zzzz)+(source[17].wwww)).z;
    // 68: mad r2.x, cb0[18].x, r1.z, r1.y
    r2.x = ((source[18].xxxx)*(r1.zzzz)+(r1.yyyy)).x;
    // 69: mad r2.y, r1.y, cb0[19].z, r1.z
    r2.y = ((r1.yyyy)*(source[19].zzzz)+(r1.zzzz)).y;
    // 70: mad r1.yz, cb0[19].wwww, r0.zzwz, r2.xxyx
    r1.yz = ((source[19].wwww)*(r0.zzwz)+(r2.xxyx)).yz;
    // 71: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t3.yxzw, s5, cb0[2].x
    r1.y = (ALTVNativeSample4((r1.yzyy).xy, (source[2].xxxx).x, true).yxzw).y;
    // 72: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 73: max r1.x, |r1.x|, l(0.000001)
    r1.x = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 74: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 75: mul r1.y, v4.w, cb0[20].x
    r1.y = ((v4.wwww)*(source[20].xxxx)).y;
    // 76: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 77: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 78: mul_sat r1.x, r1.x, cb0[20].y
    r1.x = (saturate((r1.xxxx)*(source[20].yyyy))).x;
    // 79: div r1.yz, v7.xxyx, v7.wwww
    r1.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 80: mad r1.yz, r1.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r1.yz = ((r1.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 81: source device depth mapped to centimetre view depth; reconstruction at 83.
    r1.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.yzyy).xy, 0.f).y * 100000.f;
    // Native 83-86: reconstructed view depth is supplied by the runtime adapter.
    r1.y = r1.y;
    // 87: add r1.y, r1.y, -v7.w
    r1.y = ((r1.yyyy)+(-(v7.wwww))).y;
    // 88: add r1.z, -cb0[20].z, l(1.000000)
    r1.z = ((-(source[20].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 89: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 90: div_sat r1.y, r1.y, r1.z
    r1.y = (saturate((r1.yyyy)/(r1.zzzz))).y;
    // 91: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 92: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 93: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 94: mad r1.x, v4.x, cb0[4].y, cb0[4].z
    r1.x = ((v4.xxxx)*(source[4].yyyy)+(source[4].zzzz)).x;
    // 95: mad r0.y, cb0[4].w, r0.y, r1.x
    r0.y = ((source[4].wwww)*(r0.yyyy)+(r1.xxxx)).y;
    // 96: add r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)+(source[5].xxxx)).y;
    // 97: mad r1.x, v4.x, cb0[2].z, cb0[2].w
    r1.x = ((v4.xxxx)*(source[2].zzzz)+(source[2].wwww)).x;
    // 98: mad r0.x, r0.x, cb0[2].y, r1.x
    r0.x = ((r0.xxxx)*(source[2].yyyy)+(r1.xxxx)).x;
    // 99: add r0.x, r0.x, cb0[3].z
    r0.x = ((r0.xxxx)+(source[3].zzzz)).x;
    // 100: mad r1.x, cb0[3].w, r0.x, r0.y
    r1.x = ((source[3].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 101: mad r1.y, r0.y, cb0[5].y, r0.x
    r1.y = ((r0.yyyy)*(source[5].yyyy)+(r0.xxxx)).y;
    // 102: mad r0.xy, cb0[8].wwww, r0.zwzz, r1.xyxx
    r0.xy = ((source[8].wwww)*(r0.zwzz)+(r1.xyxx)).xy;
    // 103: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s1, cb0[2].x
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 104: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 106: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 107: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 108: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 109: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 110: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 111: mul r0.xyz, r0.xyzx, cb0[9].zzzz
    r0.xyz = ((r0.xyzx)*(source[9].zzzz)).xyz;
    // 112: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 113: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
// Physical profile group 192-255 keeps unrelated native groups outside this FXC dependency.
