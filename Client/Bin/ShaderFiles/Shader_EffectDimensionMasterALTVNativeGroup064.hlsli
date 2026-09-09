// Single source owner for ALTVNative profiles 64..127.
// bfx_d_pa_circ_01_02_ad: f223e0c7e9a78643ab33afec0a30fd52; selected map daf9dc99efc3b51b9abd1280ed355304a129abffa6febba2650d6878216675d4.
float4 ALTVNative80(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ALTVSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
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

// fx_m_pa_swp_boxedgeline_01_tr: fab7ed24c6f0434a9d7cc4ac984ffef4; selected map ff9d3981fec9e5d7c2fa3b67dcf58a9231e8b5c16f8a099eef41877fa593ea48.
float4 ALTVNative81(ALTV_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[13u];
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
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = ((g_ALTVSourceMaterialParameters[5u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[6].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].x = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[7].w = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[9].x = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[9].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[9].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[5u].wwww)).x;
    source[10].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[10].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[11].y = ((g_ALTVSourceMaterialParameters[5u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[11].z = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[12].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[12].z = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[12].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[13].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].yyyy)).x;
    source[13].z = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[13].w = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[14].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[14].w = ((g_ALTVSourceMaterialParameters[6u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[15].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[15].y = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[15].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mad r0.x, v4.y, cb0[5].z, cb0[5].w
    r0.x = ((v4.yyyy)*(source[5].zzzz)+(source[5].wwww)).x;
    // 2: mad r0.x, v2.y, cb0[5].y, r0.x
    r0.x = ((v2.yyyy)*(source[5].yyyy)+(r0.xxxx)).x;
    // 3: mad r0.y, v4.y, cb0[6].y, cb0[6].z
    r0.y = ((v4.yyyy)*(source[6].yyyy)+(source[6].zzzz)).y;
    // 4: mad r0.y, cb0[6].w, v2.x, r0.y
    r0.y = ((source[6].wwww)*(v2.xxxx)+(r0.yyyy)).y;
    // 5: mad r1.x, cb0[6].x, r0.x, r0.y
    r1.x = ((source[6].xxxx)*(r0.xxxx)+(r0.yyyy)).x;
    // 6: mad r1.y, r0.y, cb0[7].x, r0.x
    r1.y = ((r0.yyyy)*(source[7].xxxx)+(r0.xxxx)).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 8: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 9: mul r0.xy, r0.xyxx, v4.zzzz
    r0.xy = ((r0.xyxx)*(v4.zzzz)).xy;
    // 10: mad r0.z, v4.x, cb0[9].x, cb0[9].y
    r0.z = ((v4.xxxx)*(source[9].xxxx)+(source[9].yyyy)).z;
    // 11: mad r0.z, v2.y, cb0[8].w, r0.z
    r0.z = ((v2.yyyy)*(source[8].wwww)+(r0.zzzz)).z;
    // 12: add r0.z, r0.z, cb0[9].w
    r0.z = ((r0.zzzz)+(source[9].wwww)).z;
    // 13: mad r0.w, v4.x, cb0[10].z, cb0[10].w
    r0.w = ((v4.xxxx)*(source[10].zzzz)+(source[10].wwww)).w;
    // 14: mad r0.w, cb0[11].x, v2.x, r0.w
    r0.w = ((source[11].xxxx)*(v2.xxxx)+(r0.wwww)).w;
    // 15: add r0.w, r0.w, cb0[11].y
    r0.w = ((r0.wwww)+(source[11].yyyy)).w;
    // 16: mad r1.x, cb0[10].x, r0.z, r0.w
    r1.x = ((source[10].xxxx)*(r0.zzzz)+(r0.wwww)).x;
    // 17: mad r1.y, r0.w, cb0[11].z, r0.z
    r1.y = ((r0.wwww)*(source[11].zzzz)+(r0.zzzz)).y;
    // 18: mad r0.zw, cb0[11].wwww, r0.xxxy, r1.xxxy
    r0.zw = ((source[11].wwww)*(r0.xxxy)+(r1.xxxy)).zw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ALTVNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 20: mul r0.z, r0.z, cb0[12].x
    r0.z = ((r0.zzzz)*(source[12].xxxx)).z;
    // 21: mad r0.w, v4.x, cb0[12].z, cb0[12].w
    r0.w = ((v4.xxxx)*(source[12].zzzz)+(source[12].wwww)).w;
    // 22: mad r0.w, v2.y, cb0[12].y, r0.w
    r0.w = ((v2.yyyy)*(source[12].yyyy)+(r0.wwww)).w;
    // 23: add r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)+(source[13].yyyy)).w;
    // 24: mad r1.x, v4.x, cb0[14].x, cb0[14].y
    r1.x = ((v4.xxxx)*(source[14].xxxx)+(source[14].yyyy)).x;
    // 25: mad r1.x, cb0[14].z, v2.x, r1.x
    r1.x = ((source[14].zzzz)*(v2.xxxx)+(r1.xxxx)).x;
    // 26: add r1.x, r1.x, cb0[14].w
    r1.x = ((r1.xxxx)+(source[14].wwww)).x;
    // 27: mad r2.x, cb0[13].z, r0.w, r1.x
    r2.x = ((source[13].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 28: mad r2.y, r1.x, cb0[15].x, r0.w
    r2.y = ((r1.xxxx)*(source[15].xxxx)+(r0.wwww)).y;
    // 29: mad r1.xy, cb0[15].yyyy, r0.xyxx, r2.xyxx
    r1.xy = ((source[15].yyyy)*(r0.xyxx)+(r2.xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 31: mul r0.w, r0.w, cb0[15].z
    r0.w = ((r0.wwww)*(source[15].zzzz)).w;
    // 32: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 33: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 34: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 35: mul r0.w, v4.w, cb0[15].w
    r0.w = ((v4.wwww)*(source[15].wwww)).w;
    // 36: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 37: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 38: mul_sat r0.z, r0.z, cb0[16].x
    r0.z = (saturate((r0.zzzz)*(source[16].xxxx))).z;
    // 39: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 40: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 41: mad r0.z, v4.x, cb0[2].y, cb0[2].z
    r0.z = ((v4.xxxx)*(source[2].yyyy)+(source[2].zzzz)).z;
    // 42: mad r0.z, v2.y, cb0[2].x, r0.z
    r0.z = ((v2.yyyy)*(source[2].xxxx)+(r0.zzzz)).z;
    // 43: add r0.z, r0.z, cb0[3].y
    r0.z = ((r0.zzzz)+(source[3].yyyy)).z;
    // 44: mad r0.w, v4.x, cb0[4].x, cb0[4].y
    r0.w = ((v4.xxxx)*(source[4].xxxx)+(source[4].yyyy)).w;
    // 45: mad r0.w, cb0[4].z, v2.x, r0.w
    r0.w = ((source[4].zzzz)*(v2.xxxx)+(r0.wwww)).w;
    // 46: add r0.w, r0.w, cb0[4].w
    r0.w = ((r0.wwww)+(source[4].wwww)).w;
    // 47: mad r1.x, cb0[3].z, r0.z, r0.w
    r1.x = ((source[3].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 48: mad r1.y, r0.w, cb0[5].x, r0.z
    r1.y = ((r0.wwww)*(source[5].xxxx)+(r0.zzzz)).y;
    // 49: mad r0.xy, cb0[7].zzzz, r0.xyxx, r1.xyxx
    r0.xy = ((source[7].zzzz)*(r0.xyxx)+(r1.xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 51: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 53: mad r0.xyz, cb0[7].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[7].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 54: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 56: mul r0.xyz, r0.xyzx, cb0[8].xxxx
    r0.xyz = ((r0.xyzx)*(source[8].xxxx)).xyz;
    // 57: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 58: mad r0.xyz, cb0[8].yyyy, r0.xyzx, cb0[8].zzzz
    r0.xyz = ((source[8].yyyy)*(r0.xyzx)+(source[8].zzzz)).xyz;
    // 59: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_f_pa_ri_01_2_ad: 22fe1d84b183a24ba061efbe70ff88fe; selected map dcb4af1b0c51e126ebfcd50d3245e9fb4ef45276b0b47937288bd666ab550511.
float4 ALTVNative82(ALTV_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[2].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ALTVNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: max r1.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 3: mul r0.xyz, r0.xyzx, cb0[2].yyyy
    r0.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 4: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 5: mul r1.xyz, r1.xyzx, l(0.700000, 0.700000, 0.700000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.700000,0.700000,0.700000,0.000000))).xyz;
    // 6: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, l(50.000000, 50.000000, 50.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(50.000000,50.000000,50.000000,0.000000))).xyz;
    // 8: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 9: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 10: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 11: mul r0.w, r0.w, v4.w
    r0.w = ((r0.wwww)*(v4.wwww)).w;
    // 12: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 13: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 14: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 15: mul r1.x, v4.x, cb0[2].x
    r1.x = ((v4.xxxx)*(source[2].xxxx)).x;
    // 16: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 17: sincos r1.x, null, r1.x
    r1.x = (sin(r1.xxxx)).x;
    // 18: add r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)+(v4.yyyy)).x;
    // 19: mul r1.x, r1.x, v4.z
    r1.x = ((r1.xxxx)*(v4.zzzz)).x;
    // 20: mul r1.xyz, r0.xyzx, r1.xxxx
    r1.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 21: mad r0.xyz, cb0[2].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[2].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
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

// fx_d_pa_glow_02_02_dt15_ad: d57e37dea1c8d440a7971d2d3d9f4074; selected map f0c196d2bc8c3df44b9d36e2da636627dfe2ad544c5625471ff0aa17ded46bfe.
float4 ALTVNative83(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[2].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: max r0.y, r0.x, l(0.000000)
    r0.y = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 6: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 9: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 10: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 11: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 12: mul r0.z, r0.z, cb0[2].y
    r0.z = ((r0.zzzz)*(source[2].yyyy)).z;
    // 13: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)*(source[2].zzzz)).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 20: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 21: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 22: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 23: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 24: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 25: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 26: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 27: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 28: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 29: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 30: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 37: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 38: add r1.x, -cb0[3].y, l(1.000000)
    r1.x = ((-(source[3].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: max r1.x, -r1.x, l(0.001000)
    r1.x = (max(-(r1.xxxx),float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 40: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 41: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 42: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 43: mul r1.x, r1.x, v6.z
    r1.x = ((r1.xxxx)*(v6.zzzz)).x;
    // 44: mul r0.w, r0.w, |r1.x|
    r0.w = ((r0.wwww)*(abs(r1.xxxx))).w;
    // 45: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 46: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 47: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 48: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_cubesample_01_02_tr: f548f39885bbfe42ac405ccd46dc2919; selected map ef8970e4179030c31ce3c35fdd62acd0cd6f4dc450c6b815d0f88a5566280d1e.
float4 ALTVNative84(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[3u];
    source[3] = g_ALTVSourceMaterialParameters[1u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.z, cb0[4].x, l(0.100000)
    r0.z = ((source[4].xxxx)*(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 4: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 7: add r0.w, -|r1.z|, l(1.000000)
    r0.w = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 8: mul r2.xy, r0.wwww, r1.yxyy
    r2.xy = ((r0.wwww)*(r1.yxyy)).xy;
    // 9: mad r0.xy, r0.zzzz, r2.xyxx, r0.xyxx
    r0.xy = ((r0.zzzz)*(r2.xyxx)+(r0.xyxx)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.xyz = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r0.xyxx).xy).xyzw).xyz;
    // 11: max r2.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 12: mul r3.xyz, r2.xyzx, r2.xyzx
    r3.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 13: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 14: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 15: mad r0.xyz, -r2.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.xyzx))*(float4(5.000000,5.000000,5.000000,0.000000))+(r0.xyzx)).xyz;
    // 16: mul r2.xyz, r2.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(5.000000,5.000000,5.000000,0.000000))).xyz;
    // 17: mad r0.xyz, |r1.zzzz|, r0.xyzx, r2.xyzx
    r0.xyz = ((abs(r1.zzzz))*(r0.xyzx)+(r2.xyzx)).xyz;
    // 18: mul r1.xy, r1.xyxx, l(3.000000, 3.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(3.000000,3.000000,0.000000,0.000000))).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xyz = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 21: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 22: mul r1.w, r1.w, l(7.000000)
    r1.w = ((r1.wwww)*(float4(7.000000,7.000000,7.000000,7.000000))).w;
    // 23: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 24: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 25: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 26: mad r0.xyz, cb0[5].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[5].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 27: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 28: add r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)+(r1.xyxx)).xy;
    // 29: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 30: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 31: mul r1.zw, r1.zzzw, cb0[5].yyyy
    r1.zw = ((r1.zzzw)*(source[5].yyyy)).zw;
    // 32: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 33: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 34: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 35: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mul r0.w, r0.w, cb0[5].z
    r0.w = ((r0.wwww)*(source[5].zzzz)).w;
    // 37: mul r1.xyz, r0.wwww, cb0[1].xyzx
    r1.xyz = ((r0.wwww)*(source[1].xyzx)).xyz;
    // 38: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 39: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 40: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 42: mov_sat r0.x, cb0[1].w
    r0.x = (saturate(source[1].wwww)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_m_me_splitline_99_tr: 66c55112978a5443834bb8b277445abd; selected map 9febd4c1c60e56650427d330c5d24f93c7a1a5a254fe2b23928f6d1068c53f13.
float4 ALTVNative85(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4] = g_ALTVSourceMaterialParameters[1u];
    source[5].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 10: mul o0.w, cb0[0].x, cb0[1].w
    output.w = ((source[0].xxxx)*(source[1].wwww)).w;
    // 11: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: mul r2.xyz, cb0[3].wyzw, cb0[5].xyzx
    r2.xyz = ((source[3].wyzw)*(source[5].xyzx)).xyz;
    // 14: add r0.w, -v4.y, l(1.000000)
    r0.w = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 16: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 17: mul r0.w, r0.w, l(15.000000)
    r0.w = ((r0.wwww)*(float4(15.000000,15.000000,15.000000,15.000000))).w;
    // 18: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 19: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 20: lt r1.w, |v4.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(v4.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 21: log r2.w, |v4.y|
    r2.w = (log2(abs(v4.yyyy))).w;
    // 22: mul r2.w, r2.w, l(15.000000)
    r2.w = ((r2.wwww)*(float4(15.000000,15.000000,15.000000,15.000000))).w;
    // 23: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 24: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 25: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 26: add r1.w, v4.x, l(-0.500000)
    r1.w = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 27: add r2.w, |r1.w|, |r1.w|
    r2.w = ((abs(r1.wwww))+(abs(r1.wwww))).w;
    // 28: lt r1.w, |r1.w|, l(0.000000)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 29: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 30: mul r2.w, r2.w, l(10.000000)
    r2.w = ((r2.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 31: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 32: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 33: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 34: mul r1.w, r0.w, r2.x
    r1.w = ((r0.wwww)*(r2.xxxx)).w;
    // 35: mul r2.x, r1.w, l(0.100000)
    r2.x = ((r1.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 36: mad r3.xy, r1.wwww, l(0.050000, 0.050000, 0.000000, 0.000000), r1.xyxx
    r3.xy = ((r1.wwww)*(float4(0.050000,0.050000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 37: mad r1.w, cb0[3].x, l(0.010000), r2.x
    r1.w = ((source[3].xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r2.xxxx)).w;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 39: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 40: mul r1.xyw, r1.xyxw, l(0.000000, 1.000000, 0.000000, 0.100000)
    r1.xyw = ((r1.xyxw)*(float4(0.000000,1.000000,0.000000,0.100000))).xyw;
    // 41: mov r2.xw, r3.yyyx
    r2.xw = (r3.yyyx).xw;
    // 42: mov r5.x, r4.x
    r5.x = (r4.xxxx).x;
    // 43: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 44: loop
    [loop] while (true) {
    // 45: ge r3.z, r5.y, l(3.000000)
    r3.z = (asfloat((uint4)((r5.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).z;
    // 46: breakc_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) break;
    // 47: mad r2.xw, -r1.yyyx, r1.wwww, r2.xxxw
    r2.xw = ((-(r1.yyyx))*(r1.wwww)+(r2.xxxw)).xw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r3.z, r2.wxww, t0.yzxw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.wxww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzxw).z;
    // 49: add r5.x, r3.z, r5.x
    r5.x = ((r3.zzzz)+(r5.xxxx)).x;
    // 50: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: endloop
    }
    // 52: mov r3.z, r3.x
    r3.z = (r3.xxxx).z;
    // 53: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 54: mov r6.x, r4.y
    r6.x = (r4.yyyy).x;
    // 55: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 56: loop
    [loop] while (true) {
    // 57: ge r2.w, r6.y, l(3.000000)
    r2.w = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 58: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 59: mad r3.yz, -r1.yyxy, r1.wwww, r3.yyzy
    r3.yz = ((-(r1.yyxy))*(r1.wwww)+(r3.yyzy)).yz;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.zyzz, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 61: add r6.x, r2.w, r6.x
    r6.x = ((r2.wwww)+(r6.xxxx)).x;
    // 62: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 63: endloop
    }
    // 64: mov r5.y, r6.x
    r5.y = (r6.xxxx).y;
    // 65: mov r4.xy, r3.xyxx
    r4.xy = (r3.xyxx).xy;
    // 66: mov r6.x, r4.z
    r6.x = (r4.zzzz).x;
    // 67: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 68: loop
    [loop] while (true) {
    // 69: ge r2.x, r6.y, l(3.000000)
    r2.x = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).x;
    // 70: breakc_nz r2.x
    if ((asuint(r2.xxxx)).x != 0u) break;
    // 71: mad r4.xy, -r1.xyxx, r1.wwww, r4.xyxx
    r4.xy = ((-(r1.xyxx))*(r1.wwww)+(r4.xyxx)).xy;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r4.xyxx, t0.zxyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).zxyw).x;
    // 73: add r6.x, r2.x, r6.x
    r6.x = ((r2.xxxx)+(r6.xxxx)).x;
    // 74: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: endloop
    }
    // 76: mov r5.z, r6.x
    r5.z = (r6.xxxx).z;
    // 77: mul r1.xyw, r5.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000)
    r1.xyw = ((r5.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))).xyw;
    // 78: dp3 r2.x, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 79: mad r3.xyz, -r5.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xxxx
    r3.xyz = ((-(r5.xyzx))*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xxxx)).xyz;
    // 80: mad r1.xyw, r2.yyyy, r3.xyxz, r1.xyxw
    r1.xyw = ((r2.yyyy)*(r3.xyxz)+(r1.xyxw)).xyw;
    // 81: lt r2.x, r0.w, l(0.000001)
    r2.x = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 82: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 83: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 84: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 85: mul r2.yzw, r0.wwww, cb0[4].xxyz
    r2.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 86: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 87: add r1.xyw, r1.xyxw, r2.xyxz
    r1.xyw = ((r1.xyxw)+(r2.xyxz)).xyw;
    // 88: mad r1.xyw, cb0[1].xyxz, r1.xyxw, cb0[2].xyxz
    r1.xyw = ((source[1].xyxz)*(r1.xyxw)+(source[2].xyxz)).xyw;
    // 89: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_e_pa_gl_01_1_tr: 991ecc5522859c49bb034318a827af71; selected map fd85ade76eaa888dbc097dfdfc3900cacd64ae68812cae449a674d712986917f.
float4 ALTVNative86(ALTV_NATIVE_INPUT input)
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

// fx_m_me_swp_shine_02_02_ad: 28e05d1b40f5094585eb96d9747e78cc; selected map b28ac50cd6976551737fb2c0ac9e13f28ce365a7489c4ecf5167869d7910b8f1.
float4 ALTVNative87(ALTV_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = g_ALTVSourceMaterialParameters[5u];
    source[4] = g_ALTVSourceMaterialParameters[6u];
    source[5] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[6] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7] = input.dynamicParameter;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[10].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[11].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
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
    // 1: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r1.x, cb0[5].xyxx, r0.xyxx
    r1.x = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[6].xyxx, r0.xyxx
    r1.y = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).y;
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
    // 10: mul r0.w, r0.w, cb0[11].w
    r0.w = ((r0.wwww)*(source[11].wwww)).w;
    // 11: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 12: mul r0.w, r0.w, cb0[12].x
    r0.w = ((r0.wwww)*(source[12].xxxx)).w;
    // 13: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 14: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 15: mul r0.w, r0.w, cb0[12].y
    r0.w = ((r0.wwww)*(source[12].yyyy)).w;
    // 16: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 17: mul r0.w, r0.w, cb0[12].z
    r0.w = ((r0.wwww)*(source[12].zzzz)).w;
    // 18: lt r1.x, |r0.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: add r0.xy, r0.xyxx, cb0[7].xxxx
    r0.xy = ((r0.xyxx)+(source[7].xxxx)).xy;
    // 20: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 21: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 22: mul r1.xy, r0.xyxx, cb0[8].zwzz
    r1.xy = ((r0.xyxx)*(source[8].zwzz)).xy;
    // 23: mul r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)*(source[10].xyxx)).xy;
    // 24: mad r2.x, cb0[8].y, cb0[8].x, r1.x
    r2.x = ((source[8].yyyy)*(source[8].xxxx)+(r1.xxxx)).x;
    // 25: mad r2.y, cb0[8].y, cb0[9].z, r1.y
    r2.y = ((source[8].yyyy)*(source[9].zzzz)+(r1.yyyy)).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 27: mad r1.x, cb0[8].y, cb0[9].w, r0.x
    r1.x = ((source[8].yyyy)*(source[9].wwww)+(r0.xxxx)).x;
    // 28: mad r1.y, cb0[8].y, cb0[10].z, r0.y
    r1.y = ((source[8].yyyy)*(source[10].zzzz)+(r0.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 30: add r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)+(r0.wwww)).x;
    // 31: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 32: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 33: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 34: mul r0.w, r0.y, cb0[11].y
    r0.w = ((r0.yyyy)*(source[11].yyyy)).w;
    // 35: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 36: mul r0.w, r0.w, cb0[11].z
    r0.w = ((r0.wwww)*(source[11].zzzz)).w;
    // 37: movc r0.w, r0.x, l(0), r0.w
    r0.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 38: mul_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)*(r0.wwww))).z;
    // 39: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 40: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 41: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 42: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 43: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 44: mul r1.x, r1.x, cb0[12].w
    r1.x = ((r1.xxxx)*(source[12].wwww)).x;
    // 45: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 46: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 47: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 48: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 49: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 50: movc o0.w, r0.w, l(0), r0.z
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 51: mul r0.z, cb0[7].x, cb0[10].w
    r0.z = ((source[7].xxxx)*(source[10].wwww)).z;
    // 52: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 53: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 54: mul_sat r0.y, r0.y, cb0[11].x
    r0.y = (saturate((r0.yyyy)*(source[11].xxxx))).y;
    // 55: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 56: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 57: mad r1.xyz, cb0[4].wwww, cb0[4].xyzx, -r0.yzwy
    r1.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r0.yzwy))).xyz;
    // 58: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 59: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_pa_swp_spritewave_01_3_ad: 43a92aa4ee4d8043a21a7718ed2d4fe3; selected map 1a729403ec71fceff4b0e6cd8ef29a0aade186f0f621170a2d913e316bc43a71.
float4 ALTVNative88(ALTV_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[10u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[7u].wwww,g_ALTVSourceMaterialParameters[8u].xxxx,1u);
    source[3] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[4u].xxxx,g_ALTVSourceMaterialParameters[4u].yyyy,1u);
    source[4] = ALTVNativeAppend(cos(((g_ALTVSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin(((g_ALTVSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ALTVSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_ALTVSourceMaterialParameters[9u];
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((g_ALTVSourceMaterialParameters[5u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialTime.xxxx).x;
    source[10].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[11].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[13].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[14].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[15].x = (cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[15].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[16].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[16].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[16].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[17].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[17].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
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
    // 34: mul r0.z, r0.z, cb0[16].w
    r0.z = ((r0.zzzz)*(source[16].wwww)).z;
    // 35: max r0.z, r0.z, cb0[17].y
    r0.z = (max(r0.zzzz,source[17].yyyy)).z;
    // 36: min r0.z, r0.z, cb0[17].x
    r0.z = (min(r0.zzzz,source[17].xxxx)).z;
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
    r0.w = (ALTVNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.w = (ALTVNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
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
    r0.x = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 72: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 73: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 75: mul r0.x, r0.x, cb0[16].x
    r0.x = ((r0.xxxx)*(source[16].xxxx)).x;
    // 76: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 77: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 78: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 79: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 80: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 81: mul r0.y, r0.y, cb0[16].z
    r0.y = ((r0.yyyy)*(source[16].zzzz)).y;
    // 82: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 83: mul_sat r0.y, r0.y, cb0[16].y
    r0.y = (saturate((r0.yyyy)*(source[16].yyyy))).y;
    // 84: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r0.x, r0.x, cb0[16].y
    r0.x = ((r0.xxxx)*(source[16].yyyy)).x;
    // 86: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 87: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 88: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 89: mul r0.x, r0.x, cb0[17].z
    r0.x = ((r0.xxxx)*(source[17].zzzz)).x;
    // 90: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 91: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 92: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 93: add r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)+(r1.yyyy)).y;
    // 94: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 95: mad r0.yzw, r0.yyyy, cb0[8].xxyz, r1.xxxx
    r0.yzw = ((r0.yyyy)*(source[8].xxyz)+(r1.xxxx)).yzw;
    // 96: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 97: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 98: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 99: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_swp_spritewave_03_93_tr: e018e6159c0a384e8b988af6955c3ef5; selected map 726a838c15c4e761bc111314fee7647f341e76de114530cc6a09e65845541c9a.
float4 ALTVNative89(ALTV_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[10u];
    source[3] = input.dynamicParameter;
    source[4] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[7u].zzzz,g_ALTVSourceMaterialParameters[7u].wwww,1u);
    source[5] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[3u].wwww,g_ALTVSourceMaterialParameters[4u].xxxx,1u);
    source[6] = ALTVNativeAppend(cos(((g_ALTVSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ALTVNativeAppend(sin(((g_ALTVSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ALTVSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ALTVSourceMaterialParameters[8u];
    source[11].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[11].y = (cos(((g_ALTVSourceMaterialParameters[4u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialTime.xxxx).x;
    source[12].x = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[12].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[13].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[13].w = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[14].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[14].y = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[15].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[15].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[16].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[16].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[16].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[16].w = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[17].x = (cos((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[17].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[17].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[17].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[18].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[18].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[18].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[18].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[19].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[19].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
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
    // 34: mul r0.z, r0.z, cb0[18].z
    r0.z = ((r0.zzzz)*(source[18].zzzz)).z;
    // 35: max r0.z, r0.z, cb0[19].x
    r0.z = (max(r0.zzzz,source[19].xxxx)).z;
    // 36: min r0.z, r0.z, cb0[18].w
    r0.z = (min(r0.zzzz,source[18].wwww)).z;
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
    r0.w = (ALTVNativeSample1((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.w = (ALTVNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
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
    // 67: mul r0.xy, r0.xyxx, cb0[16].zwzz
    r0.xy = ((r0.xyxx)*(source[16].zwzz)).xy;
    // 68: mad r2.x, cb0[11].w, cb0[16].y, r0.x
    r2.x = ((source[11].wwww)*(source[16].yyyy)+(r0.xxxx)).x;
    // 69: mad r2.y, cb0[11].w, cb0[17].y, r0.y
    r2.y = ((source[11].wwww)*(source[17].yyyy)+(r0.yyyy)).y;
    // 70: add r0.xy, r2.xyxx, cb0[17].zwzz
    r0.xy = ((r2.xyxx)+(source[17].zwzz)).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 72: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 73: add r0.y, cb0[3].y, l(-1.000000)
    r0.y = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 74: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 75: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 76: mul r0.y, r0.y, cb0[18].y
    r0.y = ((r0.yyyy)*(source[18].yyyy)).y;
    // 77: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 78: mul_sat r0.y, r0.y, cb0[18].x
    r0.y = (saturate((r0.yyyy)*(source[18].xxxx))).y;
    // 79: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: mul r0.x, r0.x, cb0[18].x
    r0.x = ((r0.xxxx)*(source[18].xxxx)).x;
    // 81: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 82: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 83: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 84: mul r0.x, r0.x, cb0[19].y
    r0.x = ((r0.xxxx)*(source[19].yyyy)).x;
    // 85: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 86: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 87: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 88: add r0.x, r0.y, r1.y
    r0.x = ((r0.yyyy)+(r1.yyyy)).x;
    // 89: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 90: mad r0.xyz, r0.xxxx, cb0[10].xyzx, r1.xxxx
    r0.xyz = ((r0.xxxx)*(source[10].xyzx)+(r1.xxxx)).xyz;
    // 91: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 92: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_me_boxdirloop_01_tr: 9d9c19035009ea47b4df9e4708f9a4b7; selected map b202b71298360e91d7afcdadb50015ffc482449b10f8da4a5d1a99f5bacf87f3.
float4 ALTVNative90(ALTV_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[14u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[5].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].zzzz)).x;
    source[5].z = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[6].w = ((g_ALTVSourceMaterialParameters[5u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[7].x = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[8].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].xxxx)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[8].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[9].x = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[9].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[9].w = ((g_ALTVSourceMaterialParameters[6u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[10].y = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[12].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[12].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[5u].wwww)).x;
    source[13].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[13].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[14].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[14].y = ((g_ALTVSourceMaterialParameters[5u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[14].z = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[14].w = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[15].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[15].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[15].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[16].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].yyyy)).x;
    source[16].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[16].w = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[17].x = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[17].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[17].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[17].w = ((g_ALTVSourceMaterialParameters[6u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[18].x = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[18].y = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[18].z = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[18].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[19].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mad r0.x, cb0[3].y, cb0[7].z, cb0[7].w
    r0.x = ((source[3].yyyy)*(source[7].zzzz)+(source[7].wwww)).x;
    // 2: mad r0.x, v4.y, cb0[7].y, r0.x
    r0.x = ((v4.yyyy)*(source[7].yyyy)+(r0.xxxx)).x;
    // 3: add r0.x, r0.x, cb0[8].y
    r0.x = ((r0.xxxx)+(source[8].yyyy)).x;
    // 4: mad r0.y, cb0[3].y, cb0[9].x, cb0[9].y
    r0.y = ((source[3].yyyy)*(source[9].xxxx)+(source[9].yyyy)).y;
    // 5: mad r0.y, cb0[9].z, v4.x, r0.y
    r0.y = ((source[9].zzzz)*(v4.xxxx)+(r0.yyyy)).y;
    // 6: add r0.y, r0.y, cb0[9].w
    r0.y = ((r0.yyyy)+(source[9].wwww)).y;
    // 7: mad r1.x, cb0[8].z, r0.x, r0.y
    r1.x = ((source[8].zzzz)*(r0.xxxx)+(r0.yyyy)).x;
    // 8: mad r1.y, r0.y, cb0[10].x, r0.x
    r1.y = ((r0.yyyy)*(source[10].xxxx)+(r0.xxxx)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mul r0.xy, r0.xyxx, cb0[10].yyyy
    r0.xy = ((r0.xyxx)*(source[10].yyyy)).xy;
    // 11: mul r0.xy, r0.xyxx, cb0[3].zzzz
    r0.xy = ((r0.xyxx)*(source[3].zzzz)).xy;
    // 12: mad r0.z, cb0[3].x, cb0[12].x, cb0[12].y
    r0.z = ((source[3].xxxx)*(source[12].xxxx)+(source[12].yyyy)).z;
    // 13: mad r0.z, v4.y, cb0[11].w, r0.z
    r0.z = ((v4.yyyy)*(source[11].wwww)+(r0.zzzz)).z;
    // 14: add r0.z, r0.z, cb0[12].w
    r0.z = ((r0.zzzz)+(source[12].wwww)).z;
    // 15: mad r0.w, cb0[3].x, cb0[13].z, cb0[13].w
    r0.w = ((source[3].xxxx)*(source[13].zzzz)+(source[13].wwww)).w;
    // 16: mad r0.w, cb0[14].x, v4.x, r0.w
    r0.w = ((source[14].xxxx)*(v4.xxxx)+(r0.wwww)).w;
    // 17: add r0.w, r0.w, cb0[14].y
    r0.w = ((r0.wwww)+(source[14].yyyy)).w;
    // 18: mad r1.x, cb0[13].x, r0.z, r0.w
    r1.x = ((source[13].xxxx)*(r0.zzzz)+(r0.wwww)).x;
    // 19: mad r1.y, r0.w, cb0[14].z, r0.z
    r1.y = ((r0.wwww)*(source[14].zzzz)+(r0.zzzz)).y;
    // 20: mad r0.zw, cb0[14].wwww, r0.xxxy, r1.xxxy
    r0.zw = ((source[14].wwww)*(r0.xxxy)+(r1.xxxy)).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ALTVNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 22: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 23: mad r0.w, cb0[3].x, cb0[15].z, cb0[15].w
    r0.w = ((source[3].xxxx)*(source[15].zzzz)+(source[15].wwww)).w;
    // 24: mad r0.w, v4.y, cb0[15].y, r0.w
    r0.w = ((v4.yyyy)*(source[15].yyyy)+(r0.wwww)).w;
    // 25: add r0.w, r0.w, cb0[16].y
    r0.w = ((r0.wwww)+(source[16].yyyy)).w;
    // 26: mad r1.x, cb0[3].x, cb0[17].x, cb0[17].y
    r1.x = ((source[3].xxxx)*(source[17].xxxx)+(source[17].yyyy)).x;
    // 27: mad r1.x, cb0[17].z, v4.x, r1.x
    r1.x = ((source[17].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 28: add r1.x, r1.x, cb0[17].w
    r1.x = ((r1.xxxx)+(source[17].wwww)).x;
    // 29: mad r2.x, cb0[16].z, r0.w, r1.x
    r2.x = ((source[16].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 30: mad r2.y, r1.x, cb0[18].x, r0.w
    r2.y = ((r1.xxxx)*(source[18].xxxx)+(r0.wwww)).y;
    // 31: mad r1.xy, cb0[18].yyyy, r0.xyxx, r2.xyxx
    r1.xy = ((source[18].yyyy)*(r0.xyxx)+(r2.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 33: mul r0.w, r0.w, cb0[18].z
    r0.w = ((r0.wwww)*(source[18].zzzz)).w;
    // 34: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 35: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 36: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 37: mul r0.w, cb0[3].w, cb0[18].w
    r0.w = ((source[3].wwww)*(source[18].wwww)).w;
    // 38: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 39: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 40: mul_sat r0.z, r0.z, cb0[19].x
    r0.z = (saturate((r0.zzzz)*(source[19].xxxx))).z;
    // 41: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 42: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 43: mad r0.z, cb0[3].x, cb0[4].y, cb0[4].z
    r0.z = ((source[3].xxxx)*(source[4].yyyy)+(source[4].zzzz)).z;
    // 44: mad r0.z, v4.y, cb0[4].x, r0.z
    r0.z = ((v4.yyyy)*(source[4].xxxx)+(r0.zzzz)).z;
    // 45: add r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)+(source[5].yyyy)).z;
    // 46: mad r0.w, cb0[3].x, cb0[6].x, cb0[6].y
    r0.w = ((source[3].xxxx)*(source[6].xxxx)+(source[6].yyyy)).w;
    // 47: mad r0.w, cb0[6].z, v4.x, r0.w
    r0.w = ((source[6].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 48: add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // 49: mad r1.x, cb0[5].z, r0.z, r0.w
    r1.x = ((source[5].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 50: mad r1.y, r0.w, cb0[7].x, r0.z
    r1.y = ((r0.wwww)*(source[7].xxxx)+(r0.zzzz)).y;
    // 51: mad r0.xy, cb0[10].zzzz, r0.xyxx, r1.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(r1.xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 54: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 55: mad r0.xyz, cb0[10].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[10].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 56: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 57: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 58: mul r0.xyz, r0.xyzx, cb0[11].xxxx
    r0.xyz = ((r0.xyzx)*(source[11].xxxx)).xyz;
    // 59: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 60: mad r0.xyz, cb0[11].yyyy, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((source[11].yyyy)*(r0.xyzx)+(source[11].zzzz)).xyz;
    // 61: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 62: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_me_swp_master_01_110_ts_ad: 06b9e273db380241bdf0b6a88f9fbe47; selected map 5bb585cd057959578db269c4b9e70ea90a1531a6480f4e6c73bf9b92afbb659d.
float4 ALTVNative91(ALTV_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[9u];
    source[3] = g_ALTVSourceMaterialParameters[7u];
    source[4] = input.dynamicParameter;
    source[5] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[4u].yyyy,g_ALTVSourceMaterialParameters[4u].zzzz,1u);
    source[6].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[7].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].yyyy)).x;
    source[8].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[9].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[10].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[11].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[12].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].wwww)).x;
    source[12].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[12].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[13].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
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
    // 10: add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 11: mad r0.z, cb0[6].y, cb0[11].w, cb0[12].x
    r0.z = ((source[6].yyyy)*(source[11].wwww)+(source[12].xxxx)).z;
    // 12: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 13: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 14: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 15: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 16: mul r0.zw, v4.xxxy, cb0[7].zzzw
    r0.zw = ((v4.xxxy)*(source[7].zzzw)).zw;
    // 17: mul r1.x, cb0[6].x, cb0[6].y
    r1.x = ((source[6].xxxx)*(source[6].yyyy)).x;
    // 18: mad r2.x, r1.x, cb0[7].y, r0.z
    r2.x = ((r1.xxxx)*(source[7].yyyy)+(r0.zzzz)).x;
    // 19: mad r2.y, r1.x, cb0[8].x, r0.w
    r2.y = ((r1.xxxx)*(source[8].xxxx)+(r0.wwww)).y;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 21: mad r0.zw, cb0[8].zzzz, r0.zzzw, v4.xxxy
    r0.zw = ((source[8].zzzz)*(r0.zzzw)+(v4.xxxy)).zw;
    // 22: add r1.yz, r0.zzwz, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.zzwz)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 23: dp2 r1.w, r3.yxyy, r1.yzyy
    r1.w = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).w;
    // 24: dp2 r1.y, r3.zyzz, r1.yzyy
    r1.y = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).y;
    // 25: mul r2.z, r1.y, cb0[5].y
    r2.z = ((r1.yyyy)*(source[5].yyyy)).z;
    // 26: mad r2.x, r1.w, cb0[5].x, r0.y
    r2.x = ((r1.wwww)*(source[5].xxxx)+(r0.yyyy)).x;
    // 27: add r1.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t3.wxyz, s4, l(0.000000)
    r1.yzw = (ALTVNativeSample4((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 29: mul r0.y, r0.w, cb0[11].x
    r0.y = ((r0.wwww)*(source[11].xxxx)).y;
    // 30: mad r2.w, r1.x, cb0[11].y, r0.y
    r2.w = ((r1.xxxx)*(source[11].yyyy)+(r0.yyyy)).w;
    // 31: mul r3.xy, r0.wzww, cb0[10].xwxx
    r3.xy = ((r0.wzww)*(source[10].xwxx)).xy;
    // 32: mad r2.yz, r1.xxxx, cb0[10].yyzy, r3.xxyx
    r2.yz = ((r1.xxxx)*(source[10].yyzy)+(r3.xxyx)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.zwzz, t2.xyzw, s3, l(0.000000)
    r3.xyz = (ALTVNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 34: mul r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)*(r3.xxyz)).yzw;
    // 35: mul r0.y, r0.z, cb0[9].w
    r0.y = ((r0.zzzz)*(source[9].wwww)).y;
    // 36: mad r2.x, r1.x, cb0[9].z, r0.y
    r2.x = ((r1.xxxx)*(source[9].zzzz)+(r0.yyyy)).x;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyz = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 38: mul r1.yz, r1.yyzy, r2.xxyx
    r1.yz = ((r1.yyzy)*(r2.xxyx)).yz;
    // 39: add r0.y, r1.z, r1.y
    r0.y = ((r1.zzzz)+(r1.yyyy)).y;
    // 40: mad r0.y, r2.z, r1.w, r0.y
    r0.y = ((r2.zzzz)*(r1.wwww)+(r0.yyyy)).y;
    // 41: add r1.y, -cb0[4].x, l(1.000000)
    r1.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 42: mad r0.y, r0.y, l(0.333330), -r1.y
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))+(-(r1.yyyy))).y;
    // 43: mul_sat r0.y, r0.y, cb0[12].w
    r0.y = (saturate((r0.yyyy)*(source[12].wwww))).y;
    // 44: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 45: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 46: mul r1.y, r1.y, cb0[13].x
    r1.y = ((r1.yyyy)*(source[13].xxxx)).y;
    // 47: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 48: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 49: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 51: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 52: mul r0.y, r0.z, cb0[6].w
    r0.y = ((r0.zzzz)*(source[6].wwww)).y;
    // 53: mad r2.x, r1.x, cb0[6].z, r0.y
    r2.x = ((r1.xxxx)*(source[6].zzzz)+(r0.yyyy)).x;
    // 54: mul r0.y, r1.x, cb0[8].w
    r0.y = ((r1.xxxx)*(source[8].wwww)).y;
    // 55: mad r2.y, cb0[7].x, r0.w, r0.y
    r2.y = ((source[7].xxxx)*(r0.wwww)+(r0.yyyy)).y;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r2.xyxx, t4.wxyz, s1, l(0.000000)
    r0.yzw = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 57: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 58: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 59: mad r0.yzw, cb0[9].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[9].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 60: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 61: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 62: mul r0.yzw, r0.yyzw, cb0[9].yyyy
    r0.yzw = ((r0.yyzw)*(source[9].yyyy)).yzw;
    // 63: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 64: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 65: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 66: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 67: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 68: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 69: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_swp_expbox_04: 113b9c8384bcd14ea6b714658962c5fb; selected map 4a2cfa9da67d142ca575c731b9453a29200b17ebcbbe74200c90052b4ed53ce1.
float4 ALTVNative92(ALTV_NATIVE_INPUT input)
{
    float4 source[17]; [unroll] for (uint i=0u; i<17u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[13u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[5].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[5u].zzzz)).x;
    source[5].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[6].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[6].w = ((g_ALTVSourceMaterialParameters[5u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[7].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[9].y = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[9].w = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[10].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[11].x = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[11].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[12].x = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[12].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[13].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[14].y = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[15].y = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[15].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mad r0.x, cb0[3].y, cb0[7].z, cb0[7].w
    r0.x = ((source[3].yyyy)*(source[7].zzzz)+(source[7].wwww)).x;
    // 2: mad r0.x, v4.y, cb0[7].y, r0.x
    r0.x = ((v4.yyyy)*(source[7].yyyy)+(r0.xxxx)).x;
    // 3: mad r0.y, cb0[3].y, cb0[8].y, cb0[8].z
    r0.y = ((source[3].yyyy)*(source[8].yyyy)+(source[8].zzzz)).y;
    // 4: mad r0.y, cb0[8].w, v4.x, r0.y
    r0.y = ((source[8].wwww)*(v4.xxxx)+(r0.yyyy)).y;
    // 5: mad r1.x, cb0[8].x, r0.x, r0.y
    r1.x = ((source[8].xxxx)*(r0.xxxx)+(r0.yyyy)).x;
    // 6: mad r1.y, r0.y, cb0[9].x, r0.x
    r1.y = ((r0.yyyy)*(source[9].xxxx)+(r0.xxxx)).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 8: mul r0.xy, r0.xyxx, cb0[9].yyyy
    r0.xy = ((r0.xyxx)*(source[9].yyyy)).xy;
    // 9: mul r0.xy, r0.xyxx, cb0[3].zzzz
    r0.xy = ((r0.xyxx)*(source[3].zzzz)).xy;
    // 10: mad r0.z, cb0[3].x, cb0[11].x, cb0[11].y
    r0.z = ((source[3].xxxx)*(source[11].xxxx)+(source[11].yyyy)).z;
    // 11: mad r0.z, v4.y, cb0[10].w, r0.z
    r0.z = ((v4.yyyy)*(source[10].wwww)+(r0.zzzz)).z;
    // 12: mad r0.w, cb0[3].x, cb0[11].w, cb0[12].x
    r0.w = ((source[3].xxxx)*(source[11].wwww)+(source[12].xxxx)).w;
    // 13: mad r0.w, cb0[12].y, v4.x, r0.w
    r0.w = ((source[12].yyyy)*(v4.xxxx)+(r0.wwww)).w;
    // 14: mad r1.x, cb0[11].z, r0.z, r0.w
    r1.x = ((source[11].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 15: mad r1.y, r0.w, cb0[12].z, r0.z
    r1.y = ((r0.wwww)*(source[12].zzzz)+(r0.zzzz)).y;
    // 16: mad r0.zw, cb0[12].wwww, r0.xxxy, r1.xxxy
    r0.zw = ((source[12].wwww)*(r0.xxxy)+(r1.xxxy)).zw;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s2, l(0.000000)
    r0.z = (ALTVNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 18: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 19: mad r0.w, cb0[3].x, cb0[13].z, cb0[13].w
    r0.w = ((source[3].xxxx)*(source[13].zzzz)+(source[13].wwww)).w;
    // 20: mad r0.w, v4.y, cb0[13].y, r0.w
    r0.w = ((v4.yyyy)*(source[13].yyyy)+(r0.wwww)).w;
    // 21: mad r1.x, cb0[3].x, cb0[14].y, cb0[14].z
    r1.x = ((source[3].xxxx)*(source[14].yyyy)+(source[14].zzzz)).x;
    // 22: mad r1.x, cb0[14].w, v4.x, r1.x
    r1.x = ((source[14].wwww)*(v4.xxxx)+(r1.xxxx)).x;
    // 23: mad r2.x, cb0[14].x, r0.w, r1.x
    r2.x = ((source[14].xxxx)*(r0.wwww)+(r1.xxxx)).x;
    // 24: mad r2.y, r1.x, cb0[15].x, r0.w
    r2.y = ((r1.xxxx)*(source[15].xxxx)+(r0.wwww)).y;
    // 25: mad r1.xy, cb0[15].yyyy, r0.xyxx, r2.xyxx
    r1.xy = ((source[15].yyyy)*(r0.xyxx)+(r2.xyxx)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 27: mul r0.w, r0.w, cb0[15].z
    r0.w = ((r0.wwww)*(source[15].zzzz)).w;
    // 28: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 29: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 30: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 31: mul r0.w, cb0[3].w, cb0[15].w
    r0.w = ((source[3].wwww)*(source[15].wwww)).w;
    // 32: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: mul_sat r0.z, r0.z, cb0[16].x
    r0.z = (saturate((r0.zzzz)*(source[16].xxxx))).z;
    // 35: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 36: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 37: mad r0.z, cb0[3].x, cb0[4].y, cb0[4].z
    r0.z = ((source[3].xxxx)*(source[4].yyyy)+(source[4].zzzz)).z;
    // 38: mad r0.z, v4.y, cb0[4].x, r0.z
    r0.z = ((v4.yyyy)*(source[4].xxxx)+(r0.zzzz)).z;
    // 39: add r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)+(source[5].yyyy)).z;
    // 40: mad r0.w, cb0[3].x, cb0[6].x, cb0[6].y
    r0.w = ((source[3].xxxx)*(source[6].xxxx)+(source[6].yyyy)).w;
    // 41: mad r0.w, cb0[6].z, v4.x, r0.w
    r0.w = ((source[6].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 42: add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // 43: mad r1.x, cb0[5].z, r0.z, r0.w
    r1.x = ((source[5].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 44: mad r1.y, r0.w, cb0[7].x, r0.z
    r1.y = ((r0.wwww)*(source[7].xxxx)+(r0.zzzz)).y;
    // 45: mad r0.xy, cb0[9].zzzz, r0.xyxx, r1.xyxx
    r0.xy = ((source[9].zzzz)*(r0.xyxx)+(r1.xyxx)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 47: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 49: mad r0.xyz, cb0[9].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 50: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 51: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 52: mul r0.xyz, r0.xyzx, cb0[10].xxxx
    r0.xyz = ((r0.xyzx)*(source[10].xxxx)).xyz;
    // 53: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 54: mad r0.xyz, cb0[10].yyyy, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((source[10].yyyy)*(r0.xyzx)+(source[10].zzzz)).xyz;
    // 55: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 56: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_me_splitline_99_tr: 66c55112978a5443834bb8b277445abd; selected map 9febd4c1c60e56650427d330c5d24f93c7a1a5a254fe2b23928f6d1068c53f13.
float4 ALTVNative93(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4] = g_ALTVSourceMaterialParameters[1u];
    source[5].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 10: mul o0.w, cb0[0].x, cb0[1].w
    output.w = ((source[0].xxxx)*(source[1].wwww)).w;
    // 11: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: mul r2.xyz, cb0[3].wyzw, cb0[5].xyzx
    r2.xyz = ((source[3].wyzw)*(source[5].xyzx)).xyz;
    // 14: add r0.w, -v4.y, l(1.000000)
    r0.w = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 16: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 17: mul r0.w, r0.w, l(15.000000)
    r0.w = ((r0.wwww)*(float4(15.000000,15.000000,15.000000,15.000000))).w;
    // 18: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 19: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 20: lt r1.w, |v4.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(v4.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 21: log r2.w, |v4.y|
    r2.w = (log2(abs(v4.yyyy))).w;
    // 22: mul r2.w, r2.w, l(15.000000)
    r2.w = ((r2.wwww)*(float4(15.000000,15.000000,15.000000,15.000000))).w;
    // 23: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 24: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 25: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 26: add r1.w, v4.x, l(-0.500000)
    r1.w = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 27: add r2.w, |r1.w|, |r1.w|
    r2.w = ((abs(r1.wwww))+(abs(r1.wwww))).w;
    // 28: lt r1.w, |r1.w|, l(0.000000)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 29: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 30: mul r2.w, r2.w, l(10.000000)
    r2.w = ((r2.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 31: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 32: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 33: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 34: mul r1.w, r0.w, r2.x
    r1.w = ((r0.wwww)*(r2.xxxx)).w;
    // 35: mul r2.x, r1.w, l(0.100000)
    r2.x = ((r1.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 36: mad r3.xy, r1.wwww, l(0.050000, 0.050000, 0.000000, 0.000000), r1.xyxx
    r3.xy = ((r1.wwww)*(float4(0.050000,0.050000,0.000000,0.000000))+(r1.xyxx)).xy;
    // 37: mad r1.w, cb0[3].x, l(0.010000), r2.x
    r1.w = ((source[3].xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r2.xxxx)).w;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 39: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 40: mul r1.xyw, r1.xyxw, l(0.000000, 1.000000, 0.000000, 0.100000)
    r1.xyw = ((r1.xyxw)*(float4(0.000000,1.000000,0.000000,0.100000))).xyw;
    // 41: mov r2.xw, r3.yyyx
    r2.xw = (r3.yyyx).xw;
    // 42: mov r5.x, r4.x
    r5.x = (r4.xxxx).x;
    // 43: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 44: loop
    [loop] while (true) {
    // 45: ge r3.z, r5.y, l(3.000000)
    r3.z = (asfloat((uint4)((r5.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).z;
    // 46: breakc_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) break;
    // 47: mad r2.xw, -r1.yyyx, r1.wwww, r2.xxxw
    r2.xw = ((-(r1.yyyx))*(r1.wwww)+(r2.xxxw)).xw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r3.z, r2.wxww, t0.yzxw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.wxww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzxw).z;
    // 49: add r5.x, r3.z, r5.x
    r5.x = ((r3.zzzz)+(r5.xxxx)).x;
    // 50: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: endloop
    }
    // 52: mov r3.z, r3.x
    r3.z = (r3.xxxx).z;
    // 53: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 54: mov r6.x, r4.y
    r6.x = (r4.yyyy).x;
    // 55: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 56: loop
    [loop] while (true) {
    // 57: ge r2.w, r6.y, l(3.000000)
    r2.w = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 58: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 59: mad r3.yz, -r1.yyxy, r1.wwww, r3.yyzy
    r3.yz = ((-(r1.yyxy))*(r1.wwww)+(r3.yyzy)).yz;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.zyzz, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 61: add r6.x, r2.w, r6.x
    r6.x = ((r2.wwww)+(r6.xxxx)).x;
    // 62: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 63: endloop
    }
    // 64: mov r5.y, r6.x
    r5.y = (r6.xxxx).y;
    // 65: mov r4.xy, r3.xyxx
    r4.xy = (r3.xyxx).xy;
    // 66: mov r6.x, r4.z
    r6.x = (r4.zzzz).x;
    // 67: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 68: loop
    [loop] while (true) {
    // 69: ge r2.x, r6.y, l(3.000000)
    r2.x = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).x;
    // 70: breakc_nz r2.x
    if ((asuint(r2.xxxx)).x != 0u) break;
    // 71: mad r4.xy, -r1.xyxx, r1.wwww, r4.xyxx
    r4.xy = ((-(r1.xyxx))*(r1.wwww)+(r4.xyxx)).xy;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r4.xyxx, t0.zxyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).zxyw).x;
    // 73: add r6.x, r2.x, r6.x
    r6.x = ((r2.xxxx)+(r6.xxxx)).x;
    // 74: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: endloop
    }
    // 76: mov r5.z, r6.x
    r5.z = (r6.xxxx).z;
    // 77: mul r1.xyw, r5.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000)
    r1.xyw = ((r5.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))).xyw;
    // 78: dp3 r2.x, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 79: mad r3.xyz, -r5.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xxxx
    r3.xyz = ((-(r5.xyzx))*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xxxx)).xyz;
    // 80: mad r1.xyw, r2.yyyy, r3.xyxz, r1.xyxw
    r1.xyw = ((r2.yyyy)*(r3.xyxz)+(r1.xyxw)).xyw;
    // 81: lt r2.x, r0.w, l(0.000001)
    r2.x = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 82: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 83: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 84: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 85: mul r2.yzw, r0.wwww, cb0[4].xxyz
    r2.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 86: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 87: add r1.xyw, r1.xyxw, r2.xyxz
    r1.xyw = ((r1.xyxw)+(r2.xyxz)).xyw;
    // 88: mad r1.xyw, cb0[1].xyxz, r1.xyxw, cb0[2].xyxz
    r1.xyw = ((source[1].xyxz)*(r1.xyxw)+(source[2].xyxz)).xyw;
    // 89: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_d_pa_glow_01_05_dt15_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 ALTVNative94(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ALTVSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[1u].xxxx)).x;
    source[5].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[1u].xxxx))).x;
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

// fx_m_me_boxexp_01_ad: 3f643a9f2265c448896f752fb13f18fa; selected map 8cff1607270412e8518b2cd2676d023fc41948a97802415bb4f7887592f0a871.
float4 ALTVNative95(ALTV_NATIVE_INPUT input)
{
    float4 source[24]; [unroll] for (uint i=0u; i<24u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[18u];
    source[3] = g_ALTVSourceMaterialParameters[16u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].zzzz)).x;
    source[6].z = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[6].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[6u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[8].z = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[9].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[9].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].xxxx)).x;
    source[9].z = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[10].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[10].w = ((g_ALTVSourceMaterialParameters[7u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[11].x = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[12].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[12].w = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[13].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[13].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[14].x = ((g_ALTVSourceMaterialParameters[8u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[14].y = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[15].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[16].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[16].y = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[16].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[17].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].wwww)).x;
    source[17].y = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[17].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[17].w = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[18].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[18].z = ((g_ALTVSourceMaterialParameters[6u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[18].w = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[19].x = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[19].y = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[19].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[19].w = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[20].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[20].y = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[20].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].yyyy)).x;
    source[20].w = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[21].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[21].y = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[21].z = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[21].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[22].x = ((g_ALTVSourceMaterialParameters[7u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[22].y = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[22].z = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[22].w = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[23].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[23].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
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
    // 1: mad r0.x, cb0[4].y, cb0[11].w, cb0[12].x
    r0.x = ((source[4].yyyy)*(source[11].wwww)+(source[12].xxxx)).x;
    // 2: mad r0.x, v4.y, cb0[11].z, r0.x
    r0.x = ((v4.yyyy)*(source[11].zzzz)+(r0.xxxx)).x;
    // 3: add r0.x, r0.x, cb0[12].z
    r0.x = ((r0.xxxx)+(source[12].zzzz)).x;
    // 4: mad r0.y, cb0[4].y, cb0[13].y, cb0[13].z
    r0.y = ((source[4].yyyy)*(source[13].yyyy)+(source[13].zzzz)).y;
    // 5: mad r0.y, cb0[13].w, v4.x, r0.y
    r0.y = ((source[13].wwww)*(v4.xxxx)+(r0.yyyy)).y;
    // 6: add r0.y, r0.y, cb0[14].x
    r0.y = ((r0.yyyy)+(source[14].xxxx)).y;
    // 7: mad r1.x, cb0[12].w, r0.x, r0.y
    r1.x = ((source[12].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 8: mad r1.y, r0.y, cb0[14].y, r0.x
    r1.y = ((r0.yyyy)*(source[14].yyyy)+(r0.xxxx)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mul r0.xy, r0.xyxx, cb0[14].zzzz
    r0.xy = ((r0.xyxx)*(source[14].zzzz)).xy;
    // 11: mad r0.z, cb0[4].y, cb0[8].z, cb0[8].w
    r0.z = ((source[4].yyyy)*(source[8].zzzz)+(source[8].wwww)).z;
    // 12: mad r0.z, v4.y, cb0[8].y, r0.z
    r0.z = ((v4.yyyy)*(source[8].yyyy)+(r0.zzzz)).z;
    // 13: add r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)+(source[9].yyyy)).z;
    // 14: mad r0.w, cb0[4].y, cb0[10].x, cb0[10].y
    r0.w = ((source[4].yyyy)*(source[10].xxxx)+(source[10].yyyy)).w;
    // 15: mad r0.w, cb0[10].z, v4.x, r0.w
    r0.w = ((source[10].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 16: add r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)+(source[10].wwww)).w;
    // 17: mad r1.x, cb0[9].z, r0.z, r0.w
    r1.x = ((source[9].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 18: mad r1.y, r0.w, cb0[11].x, r0.z
    r1.y = ((r0.wwww)*(source[11].xxxx)+(r0.zzzz)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 20: mad r0.xy, cb0[11].yyyy, r0.zwzz, r0.xyxx
    r0.xy = ((source[11].yyyy)*(r0.zwzz)+(r0.xyxx)).xy;
    // 21: mul r0.xy, r0.xyxx, cb0[4].zzzz
    r0.xy = ((r0.xyxx)*(source[4].zzzz)).xy;
    // 22: mad r0.z, cb0[4].x, cb0[19].w, cb0[20].x
    r0.z = ((source[4].xxxx)*(source[19].wwww)+(source[20].xxxx)).z;
    // 23: mad r0.z, v4.y, cb0[19].z, r0.z
    r0.z = ((v4.yyyy)*(source[19].zzzz)+(r0.zzzz)).z;
    // 24: add r0.z, r0.z, cb0[20].z
    r0.z = ((r0.zzzz)+(source[20].zzzz)).z;
    // 25: mad r0.w, cb0[4].x, cb0[21].y, cb0[21].z
    r0.w = ((source[4].xxxx)*(source[21].yyyy)+(source[21].zzzz)).w;
    // 26: mad r0.w, cb0[21].w, v4.x, r0.w
    r0.w = ((source[21].wwww)*(v4.xxxx)+(r0.wwww)).w;
    // 27: add r0.w, r0.w, cb0[22].x
    r0.w = ((r0.wwww)+(source[22].xxxx)).w;
    // 28: mad r1.x, cb0[20].w, r0.z, r0.w
    r1.x = ((source[20].wwww)*(r0.zzzz)+(r0.wwww)).x;
    // 29: mad r1.y, r0.w, cb0[22].y, r0.z
    r1.y = ((r0.wwww)*(source[22].yyyy)+(r0.zzzz)).y;
    // 30: mad r0.zw, cb0[22].zzzz, r0.xxxy, r1.xxxy
    r0.zw = ((source[22].zzzz)*(r0.xxxy)+(r1.xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s4, l(0.000000)
    r0.z = (ALTVNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mul r0.z, r0.z, cb0[22].w
    r0.z = ((r0.zzzz)*(source[22].wwww)).z;
    // 33: mad r0.w, cb0[4].x, cb0[16].y, cb0[16].z
    r0.w = ((source[4].xxxx)*(source[16].yyyy)+(source[16].zzzz)).w;
    // 34: mad r0.w, v4.y, cb0[16].x, r0.w
    r0.w = ((v4.yyyy)*(source[16].xxxx)+(r0.wwww)).w;
    // 35: add r0.w, r0.w, cb0[17].x
    r0.w = ((r0.wwww)+(source[17].xxxx)).w;
    // 36: mad r1.x, cb0[4].x, cb0[17].w, cb0[18].x
    r1.x = ((source[4].xxxx)*(source[17].wwww)+(source[18].xxxx)).x;
    // 37: mad r1.x, cb0[18].y, v4.x, r1.x
    r1.x = ((source[18].yyyy)*(v4.xxxx)+(r1.xxxx)).x;
    // 38: add r1.x, r1.x, cb0[18].z
    r1.x = ((r1.xxxx)+(source[18].zzzz)).x;
    // 39: mad r2.x, cb0[17].y, r0.w, r1.x
    r2.x = ((source[17].yyyy)*(r0.wwww)+(r1.xxxx)).x;
    // 40: mad r2.y, r1.x, cb0[18].w, r0.w
    r2.y = ((r1.xxxx)*(source[18].wwww)+(r0.wwww)).y;
    // 41: mad r1.xy, cb0[19].xxxx, r0.xyxx, r2.xyxx
    r1.xy = ((source[19].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: mad r0.z, cb0[19].y, r0.w, r0.z
    r0.z = ((source[19].yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 44: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 45: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 46: mul r0.w, cb0[4].w, cb0[23].x
    r0.w = ((source[4].wwww)*(source[23].xxxx)).w;
    // 47: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 48: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 49: mul_sat r0.z, r0.z, cb0[23].y
    r0.z = (saturate((r0.zzzz)*(source[23].yyyy))).z;
    // 50: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 51: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 52: mad r0.w, cb0[4].x, cb0[5].y, cb0[5].z
    r0.w = ((source[4].xxxx)*(source[5].yyyy)+(source[5].zzzz)).w;
    // 53: mad r0.w, v4.y, cb0[5].x, r0.w
    r0.w = ((v4.yyyy)*(source[5].xxxx)+(r0.wwww)).w;
    // 54: add r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)+(source[6].yyyy)).w;
    // 55: mad r1.x, cb0[4].x, cb0[7].x, cb0[7].y
    r1.x = ((source[4].xxxx)*(source[7].xxxx)+(source[7].yyyy)).x;
    // 56: mad r1.x, cb0[7].z, v4.x, r1.x
    r1.x = ((source[7].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 57: add r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)+(source[7].wwww)).x;
    // 58: mad r2.x, cb0[6].z, r0.w, r1.x
    r2.x = ((source[6].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 59: mad r2.y, r1.x, cb0[8].x, r0.w
    r2.y = ((r1.xxxx)*(source[8].xxxx)+(r0.wwww)).y;
    // 60: mad r0.xy, cb0[14].wwww, r0.xyxx, r2.xyxx
    r0.xy = ((source[14].wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t4.xywz, s2, l(0.000000)
    r0.xyw = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 62: dp3 r1.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 63: add r1.xyz, -r0.xywx, r1.xxxx
    r1.xyz = ((-(r0.xywx))+(r1.xxxx)).xyz;
    // 64: mad r0.xyw, cb0[15].xxxx, r1.xyxz, r0.xyxw
    r0.xyw = ((source[15].xxxx)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 65: max r0.xyw, |r0.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r0.xyw = (max(abs(r0.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 66: log r0.xyw, r0.xyxw
    r0.xyw = (log2(r0.xyxw)).xyw;
    // 67: mul r0.xyw, r0.xyxw, cb0[15].yyyy
    r0.xyw = ((r0.xyxw)*(source[15].yyyy)).xyw;
    // 68: exp r0.xyw, r0.xyxw
    r0.xyw = (exp2(r0.xyxw)).xyw;
    // 69: mad r0.xyw, cb0[15].zzzz, r0.xyxw, cb0[15].wwww
    r0.xyw = ((source[15].zzzz)*(r0.xyxw)+(source[15].wwww)).xyw;
    // 70: mul r0.xyw, r0.xyxw, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)).xyw;
    // 71: mad r0.xyw, cb0[3].xyxz, r0.xyxw, cb0[2].xyxz
    r0.xyw = ((source[3].xyxz)*(r0.xyxw)+(source[2].xyxz)).xyw;
    // 72: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 73: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 74: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_boxlinelight_02_02_ts_ad: d16014b14d2fd446a90efaeceed55c9a; selected map a8d9c40af058c7a14d9a0d6d89dd0114dcdab8bd29569494c621aa4babbd3c80.
float4 ALTVNative96(ALTV_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[6u];
    source[3] = input.dynamicParameter;
    source[4] = g_ALTVSourceMaterialParameters[7u];
    source[5].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[5].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[6].z = ((g_ALTVSourceMaterialParameters[1u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[6].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[7].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].zzzz)).x;
    source[7].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[9].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[9].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[10].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[10].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.x, v4.x, cb0[5].z, cb0[6].z
    r0.x = ((v4.xxxx)*(source[5].zzzz)+(source[6].zzzz)).x;
    // 2: mad r0.y, v4.y, cb0[5].w, cb0[7].x
    r0.y = ((v4.yyyy)*(source[5].wwww)+(source[7].xxxx)).y;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.xy, r0.xxxx, cb0[7].yyyy, v4.xyxx
    r0.xy = ((r0.xxxx)*(source[7].yyyy)+(v4.xyxx)).xy;
    // 5: add r0.z, cb0[3].z, cb0[8].y
    r0.z = ((source[3].zzzz)+(source[8].yyyy)).z;
    // 6: mad r0.y, r0.y, cb0[7].w, r0.z
    r0.y = ((r0.yyyy)*(source[7].wwww)+(r0.zzzz)).y;
    // 7: mad r1.x, r0.x, cb0[7].z, cb0[8].x
    r1.x = ((r0.xxxx)*(source[7].zzzz)+(source[8].xxxx)).x;
    // 8: mov r0.x, l(-1.000000)
    r0.x = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x;
    // 9: add r1.y, r0.y, r0.x
    r1.y = ((r0.yyyy)+(r0.xxxx)).y;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 11: mul r0.xyz, r0.xyzx, cb0[8].zzzz
    r0.xyz = ((r0.xyzx)*(source[8].zzzz)).xyz;
    // 12: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 13: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 14: mul r0.xyz, r0.xyzx, cb0[8].wwww
    r0.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 15: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 16: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 18: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 19: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 21: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 22: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 23: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 24: add r1.x, cb0[3].y, cb0[9].w
    r1.x = ((source[3].yyyy)+(source[9].wwww)).x;
    // 25: add r0.w, r0.w, -r1.x
    r0.w = ((r0.wwww)+(-(r1.xxxx))).w;
    // 26: mul_sat r1.xy, r0.wwww, l(10.000000, 8.000000, 0.000000, 0.000000)
    r1.xy = (saturate((r0.wwww)*(float4(10.000000,8.000000,0.000000,0.000000)))).xy;
    // 27: add r0.w, -r1.y, r1.x
    r0.w = ((-(r1.yyyy))+(r1.xxxx)).w;
    // 28: mul r1.x, r1.x, cb0[1].w
    r1.x = ((r1.xxxx)*(source[1].wwww)).x;
    // 29: mul r0.w, r0.w, cb0[10].x
    r0.w = ((r0.wwww)*(source[10].xxxx)).w;
    // 30: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 31: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 32: mul r0.w, r0.w, cb0[10].y
    r0.w = ((r0.wwww)*(source[10].yyyy)).w;
    // 33: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 34: mul r1.yz, v4.xxyx, cb0[5].xxxx
    r1.yz = ((v4.xxyx)*(source[5].xxxx)).yz;
    // 35: mul r1.yz, r1.yyzy, l(0.000000, 6.283185, 6.283185, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,6.283185,6.283185,0.000000))).yz;
    // 36: sincos r1.yz, null, -r1.yyzy
    r1.yz = (sin(-(r1.yyzy))).yz;
    // 37: add r1.yz, r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((r1.yyzy)+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 38: log r2.xy, r1.yzyy
    r2.xy = (log2(r1.yzyy)).xy;
    // 39: lt r1.yz, r1.yyzy, l(0.000000, 0.000001, 0.000001, 0.000000)
    r1.yz = (asfloat((uint4)((r1.yyzy)<(float4(0.000000,0.000001,0.000001,0.000000))) * 0xffffffffu)).yz;
    // 40: mul r2.xy, r2.xyxx, cb0[3].xxxx
    r2.xy = ((r2.xyxx)*(source[3].xxxx)).xy;
    // 41: exp r2.xy, r2.xyxx
    r2.xy = (exp2(r2.xyxx)).xy;
    // 42: movc r1.yz, r1.yyzy, l(0,0,0,0), r2.xxyx
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyx)).yz;
    // 43: add r1.yz, r1.yyzy, -cb0[5].yyyy
    r1.yz = ((r1.yyzy)+(-(source[5].yyyy))).yz;
    // 44: add_sat r1.y, r1.z, r1.y
    r1.y = (saturate((r1.zzzz)+(r1.yyyy))).y;
    // 45: mul r2.xyzw, r0.xxyz, r1.yyyy
    r2.xyzw = ((r0.xxyz)*(r1.yyyy)).xyzw;
    // 46: mul r2.xyzw, r2.xyzw, cb0[9].yyyy
    r2.xyzw = ((r2.xyzw)*(source[9].yyyy)).xyzw;
    // 47: mul r0.xyz, cb0[1].xyzx, cb0[4].xyzx
    r0.xyz = ((source[1].xyzx)*(source[4].xyzx)).xyz;
    // 48: mad r0.xyz, r0.wwww, r0.xyzx, r2.yzwy
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.yzwy)).xyz;
    // 49: mul r0.w, r1.x, r2.x
    r0.w = ((r1.xxxx)*(r2.xxxx)).w;
    // 50: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 51: mad r0.xyz, cb0[1].xyzx, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((source[1].xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 52: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 53: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 54: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_swp_boxring_01_tr: 80f8105dbe956848ab2fdd5d11e876ac; selected map f103210b8469c30f7273c496b23f734ba00e1e59e844650a6f507bdccbc9c500.
float4 ALTVNative97(ALTV_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[11u];
    source[3] = input.dynamicParameter;
    source[4] = g_ALTVSourceMaterialParameters[12u];
    source[5] = ALTVNativeAppend(ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[6].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[6].z = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[6].w = ((g_ALTVSourceMaterialParameters[9u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[7].x = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[7].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].zzzz)).x;
    source[7].w = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[8].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[8].y = ((g_ALTVSourceMaterialParameters[7u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[8].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[9].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].wwww)).x;
    source[9].y = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[10].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[10].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].yyyy)).x;
    source[11].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].y = ((g_ALTVSourceMaterialParameters[1u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[12].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[13].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[14].y = ((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[14].z = ((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[14].w = (ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[15].x = (ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[15].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[15].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[15].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[16].x = ((g_ALTVSourceMaterialParameters[5u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[16].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[16].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[5u].yyyy)).x;
    source[16].w = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[17].x = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[17].y = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[17].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[17].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[18].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[18].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[18].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[18].w = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[19].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[19].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
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
    // 1: mad r0.x, cb0[8].x, v4.x, cb0[8].y
    r0.x = ((source[8].xxxx)*(v4.xxxx)+(source[8].yyyy)).x;
    // 2: mad r0.y, cb0[8].z, v4.y, cb0[9].x
    r0.y = ((source[8].zzzz)*(v4.yyyy)+(source[9].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[3].zzzz
    r0.xy = ((r0.xyxx)+(source[3].zzzz)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 5: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.zw, r0.xxxy, cb0[9].yyyy
    r0.zw = ((r0.xxxy)*(source[9].yyyy)).zw;
    // 7: mad r1.x, cb0[3].w, cb0[10].x, cb0[10].y
    r1.x = ((source[3].wwww)*(source[10].xxxx)+(source[10].yyyy)).x;
    // 8: mad r1.x, v4.y, cb0[9].w, r1.x
    r1.x = ((v4.yyyy)*(source[9].wwww)+(r1.xxxx)).x;
    // 9: add r1.x, r1.x, cb0[10].w
    r1.x = ((r1.xxxx)+(source[10].wwww)).x;
    // 10: mad r1.y, cb0[3].w, cb0[11].z, cb0[11].w
    r1.y = ((source[3].wwww)*(source[11].zzzz)+(source[11].wwww)).y;
    // 11: mad r1.y, cb0[12].x, v4.x, r1.y
    r1.y = ((source[12].xxxx)*(v4.xxxx)+(r1.yyyy)).y;
    // 12: add r1.y, r1.y, cb0[12].y
    r1.y = ((r1.yyyy)+(source[12].yyyy)).y;
    // 13: mad r2.x, cb0[11].x, r1.x, r1.y
    r2.x = ((source[11].xxxx)*(r1.xxxx)+(r1.yyyy)).x;
    // 14: mad r2.y, r1.y, cb0[12].z, r1.x
    r2.y = ((r1.yyyy)*(source[12].zzzz)+(r1.xxxx)).y;
    // 15: mad r0.zw, cb0[12].wwww, r0.zzzw, r2.xxxy
    r0.zw = ((source[12].wwww)*(r0.zzzw)+(r2.xxxy)).zw;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xyz = (ALTVNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 17: max r1.xyzw, |r1.xxyz|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xxyz),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 18: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 19: mul r1.xyzw, r1.xyzw, cb0[13].xxxx
    r1.xyzw = ((r1.xyzw)*(source[13].xxxx)).xyzw;
    // 20: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 21: mul r1.xyzw, r1.xyzw, cb0[13].yyyy
    r1.xyzw = ((r1.xyzw)*(source[13].yyyy)).xyzw;
    // 22: mad r2.x, cb0[6].z, v4.x, cb0[6].w
    r2.x = ((source[6].zzzz)*(v4.xxxx)+(source[6].wwww)).x;
    // 23: mad r2.y, v4.y, cb0[7].x, cb0[7].z
    r2.y = ((v4.yyyy)*(source[7].xxxx)+(source[7].zzzz)).y;
    // 24: add r0.zw, r2.xxxy, cb0[3].zzzz
    r0.zw = ((r2.xxxy)+(source[3].zzzz)).zw;
    // 25: mad r0.xy, cb0[9].yyyy, r0.xyxx, r0.zwzz
    r0.xy = ((source[9].yyyy)*(r0.xyxx)+(r0.zwzz)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 27: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 28: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 30: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 31: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 32: add r0.y, cb0[3].y, cb0[13].z
    r0.y = ((source[3].yyyy)+(source[13].zzzz)).y;
    // 33: mad r0.xyzw, r0.xxxx, r1.xyzw, -r0.yyyy
    r0.xyzw = ((r0.xxxx)*(r1.xyzw)+(-(r0.yyyy))).xyzw;
    // 34: mul_sat r1.xyz, r0.yzwy, l(8.000000, 8.000000, 8.000000, 0.000000)
    r1.xyz = (saturate((r0.yzwy)*(float4(8.000000,8.000000,8.000000,0.000000)))).xyz;
    // 35: mul_sat r0.xyzw, r0.xyzw, l(10.000000, 10.000000, 10.000000, 10.000000)
    r0.xyzw = (saturate((r0.xyzw)*(float4(10.000000,10.000000,10.000000,10.000000)))).xyzw;
    // 36: add r0.yzw, -r1.xxyz, r0.yyzw
    r0.yzw = ((-(r1.xxyz))+(r0.yyzw)).yzw;
    // 37: mul r0.yzw, r0.yyzw, cb0[13].wwww
    r0.yzw = ((r0.yyzw)*(source[13].wwww)).yzw;
    // 38: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 39: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 40: mul r0.yzw, r0.yyzw, cb0[14].xxxx
    r0.yzw = ((r0.yyzw)*(source[14].xxxx)).yzw;
    // 41: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 42: mul r1.xyz, cb0[1].xyzx, cb0[4].xyzx
    r1.xyz = ((source[1].xyzx)*(source[4].xyzx)).xyz;
    // 43: mad r2.xy, v4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), cb0[5].xyxx
    r2.xy = ((v4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(source[5].xyxx)).xy;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t3.yzwx, s3, l(0.000000)
    r1.w = (ALTVNativeSample3((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 45: mul r2.x, |r1.w|, |r1.w|
    r2.x = ((abs(r1.wwww))*(abs(r1.wwww))).x;
    // 46: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 47: mul r1.xyz, r1.xyzx, r2.xxxx
    r1.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 48: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 49: mad r0.yzw, r0.yyzw, r1.xxyz, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r0.yyzw)*(r1.xxyz)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 50: mad r1.xy, v4.xyxx, cb0[15].yzyy, cb0[16].xzxx
    r1.xy = ((v4.xyxx)*(source[15].yzyy)+(source[16].xzxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.x = (ALTVNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 52: mad r1.xy, r1.xxxx, cb0[16].wwww, v4.xyxx
    r1.xy = ((r1.xxxx)*(source[16].wwww)+(v4.xyxx)).xy;
    // 53: mad r1.xy, r1.xyxx, cb0[17].xyxx, cb0[17].zwzz
    r1.xy = ((r1.xyxx)*(source[17].xyxx)+(source[17].zwzz)).xy;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyz = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 55: mul r1.xyz, r1.xyzx, cb0[18].xxxx
    r1.xyz = ((r1.xyzx)*(source[18].xxxx)).xyz;
    // 56: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 57: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 58: mul r1.xyz, r1.xyzx, cb0[18].yyyy
    r1.xyz = ((r1.xyzx)*(source[18].yyyy)).xyz;
    // 59: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 60: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 62: mad r1.xyz, cb0[18].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[18].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 63: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 64: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 65: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 66: mul r1.x, v4.y, cb0[18].w
    r1.x = ((v4.yyyy)*(source[18].wwww)).x;
    // 67: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 68: add r1.y, -r1.x, l(1.000000)
    r1.y = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 69: mul r1.z, cb0[3].x, cb0[19].x
    r1.z = ((source[3].xxxx)*(source[19].xxxx)).z;
    // 70: mul r1.z, r1.z, cb0[19].y
    r1.z = ((r1.zzzz)*(source[19].yyyy)).z;
    // 71: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 72: round_ni r1.xy, r1.xyxx
    r1.xy = (floor(r1.xyxx)).xy;
    // 73: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 74: mul_sat r0.x, r0.x, r1.x
    r0.x = (saturate((r0.xxxx)*(r1.xxxx))).x;
    // 75: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 76: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 77: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 78: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_swp_expbox_02: efe18343f7f03c4c82f0b616871e2570; selected map 562a55d9ecadb0e251f9ee546b72cf699eeeb43d1b42031f9845ada120b5581e.
float4 ALTVNative98(ALTV_NATIVE_INPUT input)
{
    float4 source[26]; [unroll] for (uint i=0u; i<26u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[20u];
    source[3] = g_ALTVSourceMaterialParameters[18u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[6].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[6].w = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[7u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[8].z = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[9].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[9].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].xxxx)).x;
    source[9].z = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[9].w = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[10].x = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[10].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[10].w = ((g_ALTVSourceMaterialParameters[8u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[11].x = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[17u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[11].w = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[12].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].zzzz)).x;
    source[12].w = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[13].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[13].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[13].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[14].x = ((g_ALTVSourceMaterialParameters[9u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[14].y = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[14].z = (g_ALTVSourceMaterialParameters[17u].yyyy).x;
    source[14].w = (g_ALTVSourceMaterialParameters[16u].zzzz).x;
    source[15].x = (g_ALTVSourceMaterialParameters[16u].wwww).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[15].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[15].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[16].y = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[16].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[16].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[17].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].wwww)).x;
    source[17].y = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[17].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[17].w = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[18].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[18].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[18].z = ((g_ALTVSourceMaterialParameters[7u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[18].w = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[19].x = (g_ALTVSourceMaterialParameters[15u].wwww).x;
    source[19].y = (g_ALTVSourceMaterialParameters[17u].zzzz).x;
    source[19].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[19].w = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[20].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[20].y = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[20].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].yyyy)).x;
    source[20].w = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[21].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[21].y = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[21].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[21].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[22].x = ((g_ALTVSourceMaterialParameters[8u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[22].y = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[22].z = (g_ALTVSourceMaterialParameters[16u].xxxx).x;
    source[22].w = (g_ALTVSourceMaterialParameters[17u].wwww).x;
    source[23].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[23].y = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[23].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[23].w = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[24].x = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[24].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[24].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[24].w = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[25].x = (g_ALTVSourceMaterialParameters[16u].yyyy).x;
    source[25].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[25].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
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
    // 1: mad r0.x, cb0[4].y, cb0[11].w, cb0[12].x
    r0.x = ((source[4].yyyy)*(source[11].wwww)+(source[12].xxxx)).x;
    // 2: mad r0.x, v4.y, cb0[11].z, r0.x
    r0.x = ((v4.yyyy)*(source[11].zzzz)+(r0.xxxx)).x;
    // 3: add r0.x, r0.x, cb0[12].z
    r0.x = ((r0.xxxx)+(source[12].zzzz)).x;
    // 4: mad r0.y, cb0[4].y, cb0[13].y, cb0[13].z
    r0.y = ((source[4].yyyy)*(source[13].yyyy)+(source[13].zzzz)).y;
    // 5: mad r0.y, cb0[13].w, v4.x, r0.y
    r0.y = ((source[13].wwww)*(v4.xxxx)+(r0.yyyy)).y;
    // 6: add r0.y, r0.y, cb0[14].x
    r0.y = ((r0.yyyy)+(source[14].xxxx)).y;
    // 7: mad r1.x, cb0[12].w, r0.x, r0.y
    r1.x = ((source[12].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 8: mad r1.y, r0.y, cb0[14].y, r0.x
    r1.y = ((r0.yyyy)*(source[14].yyyy)+(r0.xxxx)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mul r0.xy, r0.xyxx, cb0[14].zzzz
    r0.xy = ((r0.xyxx)*(source[14].zzzz)).xy;
    // 11: mad r0.z, cb0[4].y, cb0[8].z, cb0[8].w
    r0.z = ((source[4].yyyy)*(source[8].zzzz)+(source[8].wwww)).z;
    // 12: mad r0.z, v4.y, cb0[8].y, r0.z
    r0.z = ((v4.yyyy)*(source[8].yyyy)+(r0.zzzz)).z;
    // 13: add r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)+(source[9].yyyy)).z;
    // 14: mad r0.w, cb0[4].y, cb0[10].x, cb0[10].y
    r0.w = ((source[4].yyyy)*(source[10].xxxx)+(source[10].yyyy)).w;
    // 15: mad r0.w, cb0[10].z, v4.x, r0.w
    r0.w = ((source[10].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 16: add r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)+(source[10].wwww)).w;
    // 17: mad r1.x, cb0[9].z, r0.z, r0.w
    r1.x = ((source[9].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 18: mad r1.y, r0.w, cb0[11].x, r0.z
    r1.y = ((r0.wwww)*(source[11].xxxx)+(r0.zzzz)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 20: mad r0.xy, cb0[11].yyyy, r0.zwzz, r0.xyxx
    r0.xy = ((source[11].yyyy)*(r0.zwzz)+(r0.xyxx)).xy;
    // 21: mul r0.xy, r0.xyxx, cb0[4].zzzz
    r0.xy = ((r0.xyxx)*(source[4].zzzz)).xy;
    // 22: mad r0.z, cb0[4].x, cb0[16].y, cb0[16].z
    r0.z = ((source[4].xxxx)*(source[16].yyyy)+(source[16].zzzz)).z;
    // 23: mad r0.z, v4.y, cb0[16].x, r0.z
    r0.z = ((v4.yyyy)*(source[16].xxxx)+(r0.zzzz)).z;
    // 24: add r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)+(source[17].xxxx)).z;
    // 25: mad r0.w, cb0[4].x, cb0[17].w, cb0[18].x
    r0.w = ((source[4].xxxx)*(source[17].wwww)+(source[18].xxxx)).w;
    // 26: mad r0.w, cb0[18].y, v4.x, r0.w
    r0.w = ((source[18].yyyy)*(v4.xxxx)+(r0.wwww)).w;
    // 27: add r0.w, r0.w, cb0[18].z
    r0.w = ((r0.wwww)+(source[18].zzzz)).w;
    // 28: mad r1.x, cb0[17].y, r0.z, r0.w
    r1.x = ((source[17].yyyy)*(r0.zzzz)+(r0.wwww)).x;
    // 29: mad r1.y, r0.w, cb0[18].w, r0.z
    r1.y = ((r0.wwww)*(source[18].wwww)+(r0.zzzz)).y;
    // 30: mad r0.zw, cb0[19].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[19].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (ALTVNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mul r0.z, r0.z, cb0[19].y
    r0.z = ((r0.zzzz)*(source[19].yyyy)).z;
    // 33: mad r0.w, cb0[4].x, cb0[19].w, cb0[20].x
    r0.w = ((source[4].xxxx)*(source[19].wwww)+(source[20].xxxx)).w;
    // 34: mad r0.w, v4.y, cb0[19].z, r0.w
    r0.w = ((v4.yyyy)*(source[19].zzzz)+(r0.wwww)).w;
    // 35: add r0.w, r0.w, cb0[20].z
    r0.w = ((r0.wwww)+(source[20].zzzz)).w;
    // 36: mad r1.x, cb0[4].x, cb0[21].y, cb0[21].z
    r1.x = ((source[4].xxxx)*(source[21].yyyy)+(source[21].zzzz)).x;
    // 37: mad r1.x, cb0[21].w, v4.x, r1.x
    r1.x = ((source[21].wwww)*(v4.xxxx)+(r1.xxxx)).x;
    // 38: add r1.x, r1.x, cb0[22].x
    r1.x = ((r1.xxxx)+(source[22].xxxx)).x;
    // 39: mad r2.x, cb0[20].w, r0.w, r1.x
    r2.x = ((source[20].wwww)*(r0.wwww)+(r1.xxxx)).x;
    // 40: mad r2.y, r1.x, cb0[22].y, r0.w
    r2.y = ((r1.xxxx)*(source[22].yyyy)+(r0.wwww)).y;
    // 41: mad r1.xy, cb0[22].zzzz, r0.xyxx, r2.xyxx
    r1.xy = ((source[22].zzzz)*(r0.xyxx)+(r2.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s4, l(0.000000)
    r0.w = (ALTVNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: mul r0.w, r0.w, cb0[22].w
    r0.w = ((r0.wwww)*(source[22].wwww)).w;
    // 44: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 45: mad r0.w, cb0[4].x, cb0[23].y, cb0[23].z
    r0.w = ((source[4].xxxx)*(source[23].yyyy)+(source[23].zzzz)).w;
    // 46: mad r0.w, v4.y, cb0[23].x, r0.w
    r0.w = ((v4.yyyy)*(source[23].xxxx)+(r0.wwww)).w;
    // 47: mad r1.x, cb0[4].x, cb0[24].x, cb0[24].y
    r1.x = ((source[4].xxxx)*(source[24].xxxx)+(source[24].yyyy)).x;
    // 48: mad r1.x, cb0[24].z, v4.x, r1.x
    r1.x = ((source[24].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 49: mad r2.x, cb0[23].w, r0.w, r1.x
    r2.x = ((source[23].wwww)*(r0.wwww)+(r1.xxxx)).x;
    // 50: mad r2.y, r1.x, cb0[24].w, r0.w
    r2.y = ((r1.xxxx)*(source[24].wwww)+(r0.wwww)).y;
    // 51: mad r1.xy, cb0[25].xxxx, r0.xyxx, r2.xyxx
    r1.xy = ((source[25].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s5, l(0.000000)
    r0.w = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 53: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 54: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 55: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 56: mul r0.w, cb0[4].w, cb0[25].y
    r0.w = ((source[4].wwww)*(source[25].yyyy)).w;
    // 57: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 58: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 59: mul_sat r0.z, r0.z, cb0[25].z
    r0.z = (saturate((r0.zzzz)*(source[25].zzzz))).z;
    // 60: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 61: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 62: mad r0.w, cb0[4].x, cb0[5].y, cb0[5].z
    r0.w = ((source[4].xxxx)*(source[5].yyyy)+(source[5].zzzz)).w;
    // 63: mad r0.w, v4.y, cb0[5].x, r0.w
    r0.w = ((v4.yyyy)*(source[5].xxxx)+(r0.wwww)).w;
    // 64: add r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)+(source[6].yyyy)).w;
    // 65: mad r1.x, cb0[4].x, cb0[7].x, cb0[7].y
    r1.x = ((source[4].xxxx)*(source[7].xxxx)+(source[7].yyyy)).x;
    // 66: mad r1.x, cb0[7].z, v4.x, r1.x
    r1.x = ((source[7].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 67: add r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)+(source[7].wwww)).x;
    // 68: mad r2.x, cb0[6].z, r0.w, r1.x
    r2.x = ((source[6].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 69: mad r2.y, r1.x, cb0[8].x, r0.w
    r2.y = ((r1.xxxx)*(source[8].xxxx)+(r0.wwww)).y;
    // 70: mad r0.xy, cb0[14].wwww, r0.xyxx, r2.xyxx
    r0.xy = ((source[14].wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t5.xywz, s2, l(0.000000)
    r0.xyw = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 72: dp3 r1.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 73: add r1.xyz, -r0.xywx, r1.xxxx
    r1.xyz = ((-(r0.xywx))+(r1.xxxx)).xyz;
    // 74: mad r0.xyw, cb0[15].xxxx, r1.xyxz, r0.xyxw
    r0.xyw = ((source[15].xxxx)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 75: max r0.xyw, |r0.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r0.xyw = (max(abs(r0.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 76: log r0.xyw, r0.xyxw
    r0.xyw = (log2(r0.xyxw)).xyw;
    // 77: mul r0.xyw, r0.xyxw, cb0[15].yyyy
    r0.xyw = ((r0.xyxw)*(source[15].yyyy)).xyw;
    // 78: exp r0.xyw, r0.xyxw
    r0.xyw = (exp2(r0.xyxw)).xyw;
    // 79: mad r0.xyw, cb0[15].zzzz, r0.xyxw, cb0[15].wwww
    r0.xyw = ((source[15].zzzz)*(r0.xyxw)+(source[15].wwww)).xyw;
    // 80: mul r0.xyw, r0.xyxw, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)).xyw;
    // 81: mad r0.xyw, cb0[3].xyxz, r0.xyxw, cb0[2].xyxz
    r0.xyw = ((source[3].xyxz)*(r0.xyxw)+(source[2].xyxz)).xyw;
    // 82: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 83: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 84: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// bfx_i_pa_glow_01_ad: ff1194d8453ded4fba5fe87ac55b4348; selected map 76880f6eef92d922eb8d9e84ec7ec5d9972d9282f897cd088bc864f08680555f.
float4 ALTVNative99(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[3].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
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
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: max r0.y, r0.x, l(0.000000)
    r0.y = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 6: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 9: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 10: mul r0.z, r0.z, cb0[2].x
    r0.z = ((r0.zzzz)*(source[2].xxxx)).z;
    // 11: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 12: mul r0.z, r0.z, cb0[2].y
    r0.z = ((r0.zzzz)*(source[2].yyyy)).z;
    // 13: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)*(source[2].zzzz)).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 19: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 20: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 21: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 22: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 23: mul r0.y, r0.y, cb0[3].x
    r0.y = ((r0.yyyy)*(source[3].xxxx)).y;
    // 24: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 25: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 26: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 27: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 28: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 29: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 30: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 31: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_d_pa_shine_02_09_dt_tr: 4e9ca1966af20d498126981d3da74a9a; selected map b06bd4b25cabf6d82a355cadd05f33f5e324df063e0216d758b1a3241af22d02.
float4 ALTVNative100(ALTV_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].wwww)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[6].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[6].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx))).x;
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
    // 22: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 23: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 24: source device depth mapped to centimetre view depth; reconstruction at 26.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 26-29: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 30: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 31: add r0.w, -cb0[6].z, l(1.000000)
    r0.w = ((-(source[6].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 33: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 34: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 35: log r0.w, r0.y
    r0.w = (log2(r0.yyyy)).w;
    // 36: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 37: mul r0.w, r0.w, cb0[5].w
    r0.w = ((r0.wwww)*(source[5].wwww)).w;
    // 38: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 39: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 40: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 41: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 42: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 43: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 44: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 45: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_d_pa_shine_01_09_ad: eed5a0bfbb11fb4cbe047cc2cefd45f5; selected map 813c7a9653d5d31479688e52db86c7fc08a10cb679d4957d37544db222fd06a2.
float4 ALTVNative101(ALTV_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[5u];
    source[2].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[2].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[2].w = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[3].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[4].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    r0.xy = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[4].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[4].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.zw, r0.xxxy, cb0[3].xxxy
    r0.zw = ((r0.xxxy)*(source[3].xxxy)).zw;
    // 8: mul r0.xy, r0.xyxx, cb0[5].yzyy
    r0.xy = ((r0.xyxx)*(source[5].yzyy)).xy;
    // 9: mad r0.xy, cb0[2].wwww, cb0[5].xwxx, r0.xyxx
    r0.xy = ((source[2].wwww)*(source[5].xwxx)+(r0.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.x, cb0[2].w, cb0[2].z, r0.z
    r1.x = ((source[2].wwww)*(source[2].zzzz)+(r0.zzzz)).x;
    // 12: mad r1.y, cb0[2].w, cb0[4].w, r0.w
    r1.y = ((source[2].wwww)*(source[4].wwww)+(r0.wwww)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t1.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
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

// fx_d_pa_master_01_031_ad: 0a52266d329e4b488d7647a96dd0e351; selected map 34a4d37ffb3ea7a7eca0f5094aa65241b7ca21e4e7c4d7e646e22d21966c9230.
float4 ALTVNative102(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[3u];
    source[2].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[2].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[3].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[4].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
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
    // 6: mul r0.y, r0.y, cb0[4].x
    r0.y = ((r0.yyyy)*(source[4].xxxx)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul_sat r0.y, r0.y, cb0[4].y
    r0.y = (saturate((r0.yyyy)*(source[4].yyyy))).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: mul r0.y, v2.x, cb0[2].w
    r0.y = ((v2.xxxx)*(source[2].wwww)).y;
    // 11: mul r0.z, cb0[2].x, cb0[2].y
    r0.z = ((source[2].xxxx)*(source[2].yyyy)).z;
    // 12: mad r1.x, r0.z, cb0[2].z, r0.y
    r1.x = ((r0.zzzz)*(source[2].zzzz)+(r0.yyyy)).x;
    // 13: mul r0.y, v2.y, cb0[3].x
    r0.y = ((v2.yyyy)*(source[3].xxxx)).y;
    // 14: mad r1.y, r0.z, cb0[3].y, r0.y
    r1.y = ((r0.zzzz)*(source[3].yyyy)+(r0.yyyy)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 16: add r0.z, -v4.x, l(1.000000)
    r0.z = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 17: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 18: mul_sat r0.y, r0.y, cb0[3].z
    r0.y = (saturate((r0.yyyy)*(source[3].zzzz))).y;
    // 19: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 20: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 21: mul r0.z, r0.z, cb0[3].w
    r0.z = ((r0.zzzz)*(source[3].wwww)).z;
    // 22: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 23: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 24: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 25: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 26: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 27: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 28: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 29: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 30: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_whkf_rgbsplit_01_2_ad: d880c361f16370468cfdac8d4688d393; selected map e84de71c52a818919646772cd3b0a0181fe0758eaae2cdb6c89bc69d21826863.
float4 ALTVNative103(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[0u].zzzz,g_ALTVSourceMaterialParameters[0u].wwww,1u);
    source[3].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 3: mul r0.xyz, r0.xxxx, l(0.100000, 0.500000, 0.100000, 0.000000)
    r0.xyz = ((r0.xxxx)*(float4(0.100000,0.500000,0.100000,0.000000))).xyz;
    // 4: mul r1.xz, v4.xxxx, l(0.100000, 0.000000, -0.100000, 0.000000)
    r1.xz = ((v4.xxxx)*(float4(0.100000,0.000000,-0.100000,0.000000))).xz;
    // 5: mov r1.yw, l(0,0,0,0)
    r1.yw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).yw;
    // 6: mad r1.xyzw, v2.xyxy, cb0[2].xyxy, r1.xyzw
    r1.xyzw = ((v2.xyxy)*(source[2].xyxy)+(r1.xyzw)).xyzw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s0, l(0.000000)
    r0.w = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t0.xyzw, s0, l(0.000000)
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
    // 17: mul r0.w, v3.w, cb0[0].x
    r0.w = ((v3.wwww)*(source[0].xxxx)).w;
    // 18: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 19: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_c_pa_lensflare_01_03_ad: 55883598a583ae4389d79bb0fdf1d392; selected map ae9ed651c317374306f77e3e999680cc10ae6be4f2b89b76263a55721100b6de.
float4 ALTVNative104(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[2].z = ((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))).x;
    source[2].w = (((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[3].x = (sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].w = (((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].x = (sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[4].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[4].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    r0.yzw = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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

// fx_c_pa_lensflare_01_09_ad: 55883598a583ae4389d79bb0fdf1d392; selected map ae9ed651c317374306f77e3e999680cc10ae6be4f2b89b76263a55721100b6de.
float4 ALTVNative105(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[2].z = ((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))).x;
    source[2].w = (((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))).x;
    source[3].x = (sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0)))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[3].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[3].w = (((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].x = (sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[4].y = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[4].z = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    r0.yzw = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
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

// fx_m_pa_dl_ss_wave_03_tr: e6ff424f78d699489467b24352fb6885; selected map ecc3ed5848e4544adddebbfacf3e7b57426a624c9db3c437a63989e9b6e3fc5c.
float4 ALTVNative106(ALTV_NATIVE_INPUT input)
{
    float4 source[24]; [unroll] for (uint i=0u; i<24u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[19u];
    source[2].x = (g_ALTVSourceMaterialParameters[18u].zzzz).x;
    source[2].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[2].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[3].x = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[3].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].zzzz)).x;
    source[3].w = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[4].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[5].x = ((g_ALTVSourceMaterialParameters[7u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[5].w = (g_ALTVSourceMaterialParameters[15u].wwww).x;
    source[6].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[6].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].wwww)).x;
    source[6].w = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[7].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[7u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[8].z = (g_ALTVSourceMaterialParameters[18u].xxxx).x;
    source[8].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[16u].xxxx).x;
    source[9].y = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[9].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[10u].xxxx)).x;
    source[10].x = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[11].x = ((g_ALTVSourceMaterialParameters[7u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[11].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[18u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[17u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[13].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].yyyy)).x;
    source[13].z = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[13].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[14].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[14].w = ((g_ALTVSourceMaterialParameters[8u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[15].x = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[15].y = (g_ALTVSourceMaterialParameters[16u].yyyy).x;
    source[15].z = (g_ALTVSourceMaterialParameters[17u].zzzz).x;
    source[15].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[16].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[16].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[16].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].wwww)).x;
    source[17].x = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[17].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[17].z = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[17].w = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[18].x = ((g_ALTVSourceMaterialParameters[8u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[18].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[18].z = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[18].w = (g_ALTVSourceMaterialParameters[16u].zzzz).x;
    source[19].x = (g_ALTVSourceMaterialParameters[17u].wwww).x;
    source[19].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[19].z = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[19].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[20].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[20].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].yyyy)).x;
    source[20].z = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[20].w = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[21].x = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[21].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[21].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[21].w = ((g_ALTVSourceMaterialParameters[9u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[22].x = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[22].y = (g_ALTVSourceMaterialParameters[16u].wwww).x;
    source[22].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[22].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[23].x = (g_ALTVSourceMaterialParameters[17u].yyyy).x;
    source[23].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[23].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[23].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 36: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t1.zwxy, s3, cb0[2].x
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
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.xy, r2.xyxx, t0.xyzw, s2, cb0[2].x
    r1.xy = (ALTVNativeSample1((r2.xyxx).xy, (source[2].xxxx).x, true).xyzw).xy;
    // 47: mad r0.zw, cb0[8].zzzz, r1.xxxy, r0.zzzw
    r0.zw = ((source[8].zzzz)*(r1.xxxy)+(r0.zzzw)).zw;
    // 48: mul r0.zw, r0.zzzw, v4.zzzz
    r0.zw = ((r0.zzzw)*(v4.zzzz)).zw;
    // 49: mad r1.x, v4.x, cb0[14].x, cb0[14].y
    r1.x = ((v4.xxxx)*(source[14].xxxx)+(source[14].yyyy)).x;
    // 50: mad r1.x, cb0[14].z, r0.y, r1.x
    r1.x = ((source[14].zzzz)*(r0.yyyy)+(r1.xxxx)).x;
    // 51: add r1.x, r1.x, cb0[14].w
    r1.x = ((r1.xxxx)+(source[14].wwww)).x;
    // 52: mad r1.y, v4.x, cb0[12].z, cb0[12].w
    r1.y = ((v4.xxxx)*(source[12].zzzz)+(source[12].wwww)).y;
    // 53: mad r1.y, r0.x, cb0[12].y, r1.y
    r1.y = ((r0.xxxx)*(source[12].yyyy)+(r1.yyyy)).y;
    // 54: add r1.y, r1.y, cb0[13].y
    r1.y = ((r1.yyyy)+(source[13].yyyy)).y;
    // 55: mad r2.x, cb0[13].z, r1.y, r1.x
    r2.x = ((source[13].zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 56: mad r2.y, r1.x, cb0[15].x, r1.y
    r2.y = ((r1.xxxx)*(source[15].xxxx)+(r1.yyyy)).y;
    // 57: mad r1.xy, cb0[15].yyyy, r0.zwzz, r2.xyxx
    r1.xy = ((source[15].yyyy)*(r0.zwzz)+(r2.xyxx)).xy;
    // 58: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s4, cb0[2].x
    r1.xyz = (ALTVNativeSample3((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 59: mul r1.xyzw, r1.xxyz, cb0[15].zzzz
    r1.xyzw = ((r1.xxyz)*(source[15].zzzz)).xyzw;
    // 60: mad r2.x, v4.x, cb0[17].z, cb0[17].w
    r2.x = ((v4.xxxx)*(source[17].zzzz)+(source[17].wwww)).x;
    // 61: add r2.x, r2.x, cb0[18].x
    r2.x = ((r2.xxxx)+(source[18].xxxx)).x;
    // 62: mad r2.x, cb0[18].y, r0.y, r2.x
    r2.x = ((source[18].yyyy)*(r0.yyyy)+(r2.xxxx)).x;
    // 63: mad r2.y, v4.x, cb0[16].x, cb0[16].y
    r2.y = ((v4.xxxx)*(source[16].xxxx)+(source[16].yyyy)).y;
    // 64: add r2.y, r2.y, cb0[16].w
    r2.y = ((r2.yyyy)+(source[16].wwww)).y;
    // 65: mad r2.y, r0.x, cb0[15].w, r2.y
    r2.y = ((r0.xxxx)*(source[15].wwww)+(r2.yyyy)).y;
    // 66: mad r3.x, cb0[17].x, r2.y, r2.x
    r3.x = ((source[17].xxxx)*(r2.yyyy)+(r2.xxxx)).x;
    // 67: mad r3.y, r2.x, cb0[18].z, r2.y
    r3.y = ((r2.xxxx)*(source[18].zzzz)+(r2.yyyy)).y;
    // 68: mad r2.xy, cb0[18].wwww, r0.zwzz, r3.xyxx
    r2.xy = ((source[18].wwww)*(r0.zwzz)+(r3.xyxx)).xy;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s5, cb0[2].x
    r2.xyz = (ALTVNativeSample4((r2.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 70: mul r2.xyzw, r2.xxyz, cb0[19].xxxx
    r2.xyzw = ((r2.xxyz)*(source[19].xxxx)).xyzw;
    // 71: mul r1.xyzw, r1.xyzw, r2.xyzw
    r1.xyzw = ((r1.xyzw)*(r2.xyzw)).xyzw;
    // 72: mad r2.x, v4.x, cb0[21].x, cb0[21].y
    r2.x = ((v4.xxxx)*(source[21].xxxx)+(source[21].yyyy)).x;
    // 73: mad r2.x, cb0[21].z, r0.y, r2.x
    r2.x = ((source[21].zzzz)*(r0.yyyy)+(r2.xxxx)).x;
    // 74: add r2.x, r2.x, cb0[21].w
    r2.x = ((r2.xxxx)+(source[21].wwww)).x;
    // 75: mad r2.y, v4.x, cb0[19].z, cb0[19].w
    r2.y = ((v4.xxxx)*(source[19].zzzz)+(source[19].wwww)).y;
    // 76: mad r2.y, r0.x, cb0[19].y, r2.y
    r2.y = ((r0.xxxx)*(source[19].yyyy)+(r2.yyyy)).y;
    // 77: add r2.y, r2.y, cb0[20].y
    r2.y = ((r2.yyyy)+(source[20].yyyy)).y;
    // 78: mad r3.x, cb0[20].z, r2.y, r2.x
    r3.x = ((source[20].zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 79: mad r3.y, r2.x, cb0[22].x, r2.y
    r3.y = ((r2.xxxx)*(source[22].xxxx)+(r2.yyyy)).y;
    // 80: mad r2.xy, cb0[22].yyyy, r0.zwzz, r3.xyxx
    r2.xy = ((source[22].yyyy)*(r0.zwzz)+(r3.xyxx)).xy;
    // 81: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s6, cb0[2].x
    r2.xyz = (ALTVNativeSample5((r2.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 82: mul r1.xyzw, r1.xyzw, r2.xxyz
    r1.xyzw = ((r1.xyzw)*(r2.xxyz)).xyzw;
    // 83: max r1.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 84: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 85: mul r2.x, v4.w, cb0[22].z
    r2.x = ((v4.wwww)*(source[22].zzzz)).x;
    // 86: mul r1.xyzw, r1.xyzw, r2.xxxx
    r1.xyzw = ((r1.xyzw)*(r2.xxxx)).xyzw;
    // 87: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 88: mul_sat r1.xyzw, r1.xyzw, cb0[22].wwww
    r1.xyzw = (saturate((r1.xyzw)*(source[22].wwww))).xyzw;
    // 89: mad r2.x, v4.x, cb0[4].y, cb0[4].z
    r2.x = ((v4.xxxx)*(source[4].yyyy)+(source[4].zzzz)).x;
    // 90: mad r0.y, cb0[4].w, r0.y, r2.x
    r0.y = ((source[4].wwww)*(r0.yyyy)+(r2.xxxx)).y;
    // 91: add r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)+(source[5].xxxx)).y;
    // 92: mad r2.x, v4.x, cb0[2].z, cb0[2].w
    r2.x = ((v4.xxxx)*(source[2].zzzz)+(source[2].wwww)).x;
    // 93: mad r0.x, r0.x, cb0[2].y, r2.x
    r0.x = ((r0.xxxx)*(source[2].yyyy)+(r2.xxxx)).x;
    // 94: add r0.x, r0.x, cb0[3].z
    r0.x = ((r0.xxxx)+(source[3].zzzz)).x;
    // 95: mad r2.x, cb0[3].w, r0.x, r0.y
    r2.x = ((source[3].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 96: mad r2.y, r0.y, cb0[5].y, r0.x
    r2.y = ((r0.yyyy)*(source[5].yyyy)+(r0.xxxx)).y;
    // 97: mad r0.xy, cb0[12].xxxx, r0.zwzz, r2.xyxx
    r0.xy = ((source[12].xxxx)*(r0.zwzz)+(r2.xyxx)).xy;
    // 98: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s1, cb0[2].x
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 99: mul r2.xyz, r1.yzwy, r0.xyzx
    r2.xyz = ((r1.yzwy)*(r0.xyzx)).xyz;
    // 100: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: mad r0.xyz, -r0.xyzx, r1.yzwy, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(r1.yzwy)+(r0.wwww)).xyz;
    // 102: mad r0.xyz, cb0[23].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[23].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 103: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 104: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 105: mul r0.xyz, r0.xyzx, cb0[23].yyyy
    r0.xyz = ((r0.xyzx)*(source[23].yyyy)).xyz;
    // 106: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 107: mul r0.xyz, r0.xyzx, cb0[23].zzzz
    r0.xyz = ((r0.xyzx)*(source[23].zzzz)).xyz;
    // 108: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 109: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 110: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 111: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 112: source device depth mapped to centimetre view depth; reconstruction at 114.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 114-117: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 118: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 119: add r0.y, -cb0[23].w, l(1.000000)
    r0.y = ((-(source[23].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 121: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 122: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 123: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 124: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    return output;
}

// fx_d_pa_flare_03_ad: d6041fd1cc1ab44bae5b05c296b2d90f; selected map e8e8f2d00fec8e1235140f0675cd799a78a85cba5b1966b75552dfd207c86a98.
float4 ALTVNative107(ALTV_NATIVE_INPUT input)
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
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: add r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)+(r0.xyxx)).xy;
    // 3: mad r0.z, -|r0.x|, |r0.y|, l(1.000000)
    r0.z = ((-(abs(r0.xxxx)))*(abs(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 4: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 5: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: log r0.y, |r0.z|
    r0.y = (log2(abs(r0.zzzz))).y;
    // 9: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 10: mul r0.y, r0.y, l(800.000000)
    r0.y = ((r0.yyyy)*(float4(800.000000,800.000000,800.000000,800.000000))).y;
    // 11: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 12: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 13: mul r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 14: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 15: mul r0.w, r0.w, l(0.150000)
    r0.w = ((r0.wwww)*(float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 16: lt r1.x, r0.x, l(0.000001)
    r1.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 17: mad r0.x, r0.y, l(2.000000), r0.x
    r0.x = ((r0.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(r0.xxxx)).x;
    // 18: mad r1.yzw, r0.xxxx, v3.xxyz, cb0[1].xxyz
    r1.yzw = ((r0.xxxx)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 19: mul r1.yzw, r1.yyzw, v5.wwww
    r1.yzw = ((r1.yyzw)*(v5.wwww)).yzw;
    // 20: movc r0.xz, r1.xxxx, l(0,0,0,0), r0.wwzw
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwzw)).xz;
    // 21: mad r0.x, r0.z, r0.y, r0.x
    r0.x = ((r0.zzzz)*(r0.yyyy)+(r0.xxxx)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: mul o0.xyz, r0.xxxx, r1.yzwy
    output.xyz = ((r0.xxxx)*(r1.yzwy)).xyz;
    // 25: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// bfx_c_pa_lightflare_01_ddt_4_ad: 62d22e79ebe9a24b98406751d77a5ac9; selected map 059656cc63615f570a09b85d568328904dd0a9354e3275e24447a0a5b6966d25.
float4 ALTVNative108(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[0u].wwww,g_ALTVSourceMaterialParameters[1u].xxxx,1u);
    source[3].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[4].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))).x;
    source[4].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(6.28318548, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[4].z = (((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))).x;
    source[4].w = (sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0)))).x;
    source[5].x = ((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))).x;
    source[5].y = (((float4(1.0, 0.0, 0.0, 0.0)+sin(((g_ALTVSourceMaterialTime.xxxx*float4(4.0, 0.0, 0.0, 0.0))*float4(4.58626699, 0.0, 0.0, 0.0))))*float4(0.5, 0.0, 0.0, 0.0))).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    float4 r0=0.f, r1=0.f;
    // 1: add r0.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul_sat r0.xy, r0.xyxx, l(3.000000, 3.000000, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xyxx)*(float4(3.000000,3.000000,0.000000,0.000000)))).xy;
    // 3: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 4: mul_sat r0.yz, v2.xxyx, l(0.000000, 3.000000, 3.000000, 0.000000)
    r0.yz = (saturate((v2.xxyx)*(float4(0.000000,3.000000,3.000000,0.000000)))).yz;
    // 5: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, l(1.700000)
    r0.x = ((r0.xxxx)*(float4(1.700000,1.700000,1.700000,1.700000))).x;
    // 8: mul r1.x, v2.x, cb0[2].x
    r1.x = ((v2.xxxx)*(source[2].xxxx)).x;
    // 9: mad r1.z, v2.y, cb0[2].y, cb0[3].z
    r1.z = ((v2.yyyy)*(source[2].yyyy)+(source[3].zzzz)).z;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xzxx, t0.wxyz, s0, l(0.000000)
    r0.yzw = (ALTVNativeSample0((r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 11: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 13: mad r0.yzw, cb0[3].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[3].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 14: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 15: mad r0.w, cb0[4].y, l(0.300000), l(0.700000)
    r0.w = ((source[4].yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 16: mad r1.x, cb0[5].y, l(0.300000), l(0.700000)
    r1.x = ((source[5].yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 17: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 18: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 19: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 20: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 21: mul r0.w, v3.w, cb0[5].z
    r0.w = ((v3.wwww)*(source[5].zzzz)).w;
    // 22: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 23: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 24: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_pa_bloodcliff_glow_depthfade_01_02_ad: f1ee928cd5135a43b5f7ecda979adc4e; selected map 78660aa0098523388cfb3e8846030823e481eab40c423d1a96f4a05c73bfd2e3.
float4 ALTVNative109(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx))).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[3].x = ((g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[3].y = ((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[3].z = (max((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[3].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-(g_ALTVSourceMaterialParameters[0u].yyyy*float4(1.0, 0.0, 0.0, 0.0))),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[4].y = ((g_ALTVSourceMaterialParameters[0u].zzzz*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[1u].xxxx)).x;
    source[5].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[1u].xxxx))).x;
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

// fx_o_me_watertrail_01_47_tr: 0c986668f7b6b8438ad4e59533ac0d95; selected map 04f23ee5f526fb7f1b8f245b449c842f3c96ad9cc0aef03ea1bde8686efad069.
float4 ALTVNative110(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = input.dynamicParameter;
    source[4] = ALTVNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[3u].zzzz,g_ALTVSourceMaterialParameters[3u].wwww,1u);
    source[7].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[11].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[14].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    r1.yz = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 19: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 20: add r1.xy, cb0[3].xzxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[3].xzxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mad r2.x, r1.x, cb0[7].x, r0.z
    r2.x = ((r1.xxxx)*(source[7].xxxx)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[12].y, r0.w
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r2.xyxx, t2.xwyz, s2, l(0.000000)
    r1.xzw = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.x = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

// fx_c_pa_aura_02_tr: 845a7f642142e64c9a42769724b7f8e9; selected map 0db7bf127e3cc8866e3815e0cae2c9a6add9fbe4c31bc3594001134936310ccb.
float4 ALTVNative111(ALTV_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[0u];
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.zwzz, t0.wxyz, s1, l(0.000000)
    r0.x = (ALTVNativeSample1((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).x;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t0.xwyz, s1, l(0.000000)
    r0.y = (ALTVNativeSample1((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).y;
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
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 9: mul r1.xyz, v3.xyzx, l(0.359551, 0.330854, 0.274713, 0.000000)
    r1.xyz = ((v3.xyzx)*(float4(0.359551,0.330854,0.274713,0.000000))).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 11: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_pa_linearwave_01_56_tr: d47fac399d98f94c8e77decddcc2a4a5; selected map 9ab70c8da5249d46d85d05135326c5e62ae05afb873b8e0889d17666a28d7343.
float4 ALTVNative112(ALTV_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[13u];
    source[2] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ALTVSourceMaterialParameters[12u];
    source[7].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[8].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].xxxx)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[8].w = ((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[12].w = (floor(g_ALTVSourceMaterialParameters[8u].wwww)).x;
    source[13].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[13].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[13].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[13].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].wwww)).x;
    source[14].x = (cos((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[15].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[15].y = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[15].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[15].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].yyyy)).x;
    source[16].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[16].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[16].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[17].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[17].y = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[17].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[17].w = (floor(g_ALTVSourceMaterialParameters[7u].zzzz)).x;
    source[18].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[18].z = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[18].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[19].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[19].y = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[19].z = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[19].w = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[20].x = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[20].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[20].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
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
    // 1: mad r0.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
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
    // 22: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 23: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 24: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 25: mov r0.y, -r0.x
    r0.y = (-(r0.xxxx)).y;
    // 26: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 27: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 28: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 29: mul r0.z, r0.z, l(0.159155)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))).z;
    // 30: frc r0.x, r0.z
    r0.x = (frac(r0.zzzz)).x;
    // 31: add r0.xy, r0.xyxx, l(-0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,0.500000,0.000000,0.000000))).xy;
    // 32: mad r0.zw, cb0[9].wwww, r0.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((source[9].wwww)*(r0.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 33: mad r0.xy, cb0[14].yyyy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[14].yyyy)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 34: mad r0.xy, r0.xyxx, cb0[14].zwzz, cb0[15].xwxx
    r0.xy = ((r0.xyxx)*(source[14].zwzz)+(source[15].xwxx)).xy;
    // 35: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 36: mul r0.w, r0.w, cb0[10].y
    r0.w = ((r0.wwww)*(source[10].yyyy)).w;
    // 37: mad r1.x, r0.z, cb0[10].x, cb0[10].z
    r1.x = ((r0.zzzz)*(source[10].xxxx)+(source[10].zzzz)).x;
    // 38: mad r1.y, cb0[10].w, v4.x, r0.w
    r1.y = ((source[10].wwww)*(v4.xxxx)+(r0.wwww)).y;
    // 39: add r0.zw, r1.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 40: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 41: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 42: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 43: mad r1.xy, v2.xyxx, cb0[11].xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(source[11].xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.xy = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 45: mul r1.z, v4.w, cb0[11].z
    r1.z = ((v4.wwww)*(source[11].zzzz)).z;
    // 46: mad r0.zw, r1.zzzz, r1.xxxy, r0.zzzw
    r0.zw = ((r1.zzzz)*(r1.xxxy)+(r0.zzzw)).zw;
    // 47: mul r1.z, v2.x, cb0[11].w
    r1.z = ((v2.xxxx)*(source[11].wwww)).z;
    // 48: mul r1.w, v2.y, cb0[12].x
    r1.w = ((v2.yyyy)*(source[12].xxxx)).w;
    // 49: add r1.xy, r1.zwzz, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s3, l(0.000000)
    r1.xy = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 51: mad r0.zw, cb0[12].yyyy, r1.xxxy, r0.zzzw
    r0.zw = ((source[12].yyyy)*(r1.xxxy)+(r0.zzzw)).zw;
    // 52: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s1, cb0[8].y
    r0.z = (ALTVNativeSample1((r0.zwzz).xy, (source[8].yyyy).x, true).yzxw).z;
    // 53: mad r0.z, cb0[12].w, -r0.z, r0.z
    r0.z = ((source[12].wwww)*(-(r0.zzzz))+(r0.zzzz)).z;
    // 54: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 55: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 56: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 57: mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // 58: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 59: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 60: dp2 r1.x, cb0[4].xyxx, r0.xyxx
    r1.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 61: dp2 r1.y, cb0[5].xyxx, r0.xyxx
    r1.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 62: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 63: mad r1.xy, v2.xyxx, cb0[16].xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(source[16].xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t3.xyzw, s5, l(0.000000)
    r1.xy = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 65: mul r0.w, v4.w, cb0[16].z
    r0.w = ((v4.wwww)*(source[16].zzzz)).w;
    // 66: mad r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((r0.wwww)*(r1.xyxx)+(r0.xyxx)).xy;
    // 67: mul r1.z, v2.x, cb0[16].w
    r1.z = ((v2.xxxx)*(source[16].wwww)).z;
    // 68: mul r1.w, v2.y, cb0[17].x
    r1.w = ((v2.yyyy)*(source[17].xxxx)).w;
    // 69: add r1.xy, r1.zwzz, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t4.xyzw, s6, l(0.000000)
    r1.xy = (ALTVNativeSample6((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 71: mad r0.xy, cb0[17].yyyy, r1.xyxx, r0.xyxx
    r0.xy = ((source[17].yyyy)*(r1.xyxx)+(r0.xyxx)).xy;
    // 72: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s4, cb0[13].w
    r0.x = (ALTVNativeSample4((r0.xyxx).xy, (source[13].wwww).x, true).xyzw).x;
    // 73: mad r0.x, cb0[17].w, -r0.x, r0.x
    r0.x = ((source[17].wwww)*(-(r0.xxxx))+(r0.xxxx)).x;
    // 74: mul r0.x, r0.x, cb0[18].x
    r0.x = ((r0.xxxx)*(source[18].xxxx)).x;
    // 75: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 76: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 77: mul r0.x, r0.x, cb0[18].y
    r0.x = ((r0.xxxx)*(source[18].yyyy)).x;
    // 78: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 79: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 80: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 81: mul r0.x, r0.x, cb0[18].z
    r0.x = ((r0.xxxx)*(source[18].zzzz)).x;
    // 82: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 83: mul r0.y, r0.y, cb0[19].y
    r0.y = ((r0.yyyy)*(source[19].yyyy)).y;
    // 84: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 85: mul r0.y, r0.y, cb0[19].z
    r0.y = ((r0.yyyy)*(source[19].zzzz)).y;
    // 86: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 87: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 88: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 89: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 90: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 91: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 92: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 93: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 94: mad r1.x, cb0[19].w, l(10.000000), l(10.000000)
    r1.x = ((source[19].wwww)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 95: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 96: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 97: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 98: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.z, r0.z, cb0[20].x
    r0.z = ((r0.zzzz)*(source[20].xxxx)).z;
    // 100: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 101: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 102: mul r0.z, r0.z, cb0[20].y
    r0.z = ((r0.zzzz)*(source[20].yyyy)).z;
    // 103: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 104: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 105: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 106: mad r0.yz, v2.xxyx, cb0[7].yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((v2.xxyx)*(source[7].yyzy)+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 107: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t6.wxyz, s0, cb0[7].x
    r0.yzw = (ALTVNativeSample0((r0.yzyy).xy, (source[7].xxxx).x, true).wxyz).yzw;
    // 108: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 110: mad r0.yzw, cb0[7].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 111: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 112: mul r0.xyz, r0.xyzx, cb0[18].wwww
    r0.xyz = ((r0.xyzx)*(source[18].wwww)).xyz;
    // 113: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 114: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 115: mul r0.xyz, r0.xyzx, cb0[19].xxxx
    r0.xyz = ((r0.xyzx)*(source[19].xxxx)).xyz;
    // 116: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 117: mul_sat r0.xyz, r0.xyzx, cb0[6].xyzx
    r0.xyz = (saturate((r0.xyzx)*(source[6].xyzx))).xyz;
    // 118: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 119: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_d_pa_atta_05_07_ad: f0aa4e3e7314dd4e9c13f71b608b7852; selected map 05b29fa49858053a667bb6066e164e45c67777aed8d16de4034a1f011b111555.
float4 ALTVNative113(ALTV_NATIVE_INPUT input)
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
    float4 v4 = float4(0.f,0.f,0.f,0.f); // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ALTVNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

// fx_o_me_makeflow_02_22_tr: 9765660da7e1414994a02fe197e8e364; selected map 918ae65d939d6b75a6cc152d5ac743e52b01221f2cff31ca56faeeb01242cf25.
float4 ALTVNative114(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[7u];
    source[3] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[3u].wwww,g_ALTVSourceMaterialParameters[4u].xxxx,1u);
    source[4] = ALTVNativeAppend((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[3u].yyyy),(g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[3u].zzzz),1u);
    source[5] = input.dynamicParameter;
    source[6] = ALTVNativeAppend((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[0u].yyyy),(g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[0u].zzzz),1u);
    source[7] = ALTVNativeAppend((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].yyyy),(g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].zzzz),1u);
    source[8] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[5u].zzzz,g_ALTVSourceMaterialParameters[5u].wwww,1u);
    source[9] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[10] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[11].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[11].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[12].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[0u].zzzz)).x;
    source[12].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[12].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[14].x = (cos((g_ALTVSourceMaterialParameters[5u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[15].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[15].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
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
    // 1: mov r0.y, cb0[11].z
    r0.y = (source[11].zzzz).y;
    // 2: mov r0.x, l(0)
    r0.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 3: mad r0.zw, v4.xxxy, cb0[3].xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[3].xxxy)+(source[4].xxxy)).zw;
    // 4: add r1.xyzw, r0.zwzw, r0.yxxy
    r1.xyzw = ((r0.zwzw)+(r0.yxxy)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.zwzz, t0.xyzw, s0, l(0.000000)
    r0.x = (ALTVNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (ALTVNativeSample0((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r1.xyz = (ALTVNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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
    r0.x = (ALTVNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

// fx_o_pa_circledisort_01_01_ad: 36a0459be7616a4bacc60514830f1037; selected map 2449918b9aefcd38dfba19a96c9200cd10fb6ac36a0aaaf38df7a2ecaaa60233.
float4 ALTVNative115(ALTV_NATIVE_INPUT input)
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

// fx_f_pa_wind_05_tr: bd8398a6efa91243bb7de7f4bed27970; selected map b2ae4f2aac6beb7fbe8393c856ec9810b7bff830a2d04ee479916d7f6c017df9.
float4 ALTVNative116(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[2].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].w = ((g_ALTVSourceMaterialParameters[0u].zzzz*float4(3.5, 0.0, 0.0, 0.0))).x;
    source[3].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mul r0.xyzw, v2.xyxy, l(0.500000, 0.500000, 0.700000, 0.700000)
    r0.xyzw = ((v2.xyxy)*(float4(0.500000,0.500000,0.700000,0.700000))).xyzw;
    // 2: mad r0.xy, v4.wwww, l(0.100000, -0.100000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((v4.wwww)*(float4(0.100000,-0.100000,0.000000,0.000000))+(r0.xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 4: mad r0.yz, r0.xxxx, l(0.000000, 0.400000, 0.400000, 0.000000), r0.zzwz
    r0.yz = ((r0.xxxx)*(float4(0.000000,0.400000,0.400000,0.000000))+(r0.zzwz)).yz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.yzyy, t1.yzwx, s4, l(0.000000)
    r0.w = (ALTVNativeSample4((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 7: add r0.z, r0.w, r0.w
    r0.z = ((r0.wwww)+(r0.wwww)).z;
    // 8: mul r0.w, r0.x, l(0.200000)
    r0.w = ((r0.xxxx)*(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 9: add r1.xy, r0.xxxx, v2.xyxx
    r1.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t5.xyzw, s2, l(0.000000)
    r0.x = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 11: mad r1.xy, v2.xyxx, l(0.950000, 0.950000, 0.000000, 0.000000), r0.wwww
    r1.xy = ((v2.xyxx)*(float4(0.950000,0.950000,0.000000,0.000000))+(r0.wwww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s5, l(0.000000)
    r0.w = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r1.z = (ALTVNativeSample3((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.x = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

// fx_c_pa_dist_05_ad: 0fc154ee4cdbcc408e07e6b3989115cd; selected map 51630230004172d119766ea88ac7bbd7786f285ef0fc919f13cb27502021606a.
float4 ALTVNative117(ALTV_NATIVE_INPUT input)
{
    float4 source[2]; [unroll] for (uint i=0u; i<2u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
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
    float4 r0=0.f;
    // 1: mul r0.xyz, v5.wwww, cb0[1].xyzx
    r0.xyz = ((v5.wwww)*(source[1].xyzx)).xyz;
    // 2: mul o0.xyz, r0.xyzx, cb0[0].xxxx
    output.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 3: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_d_me_master_01_018_ad: c9cf79250d787c4b8faa16cdb042f143; selected map e8bae060e5bc8394b3e0755ed4d89824b43afc42dcdbf4d111327e2960a7236f.
float4 ALTVNative118(ALTV_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[6u];
    source[3] = input.dynamicParameter;
    source[4] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[3u].yyyy,g_ALTVSourceMaterialParameters[3u].zzzz,1u);
    source[5].x = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[5].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[7].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].wwww)).x;
    source[7].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[9].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].wwww)).x;
    source[9].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
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
    r0.yz = (ALTVNativeSample0((r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
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
    r0.x = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
    // 20: mul r1.xy, r0.yzyy, cb0[8].yzyy
    r1.xy = ((r0.yzyy)*(source[8].yzyy)).xy;
    // 21: mad r1.xy, r0.wwww, cb0[8].xwxx, r1.xyxx
    r1.xy = ((r0.wwww)*(source[8].xwxx)+(r1.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.zxyw, s2, l(0.000000)
    r1.x = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).x;
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
    r0.y = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
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

// fx_x_me_watertrail_29_02_dt_tr: 4a404b08868bf045b58d8e3d466fbdb3; selected map 5f099821f79dc2732db77dc65699037a88e185b3127dd4b13c6315ec54d70491.
float4 ALTVNative119(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = ALTVNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[4u].yyyy,g_ALTVSourceMaterialParameters[4u].zzzz,1u);
    source[7].x = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[10].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[11].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[12].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[13].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[13].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[14].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[15].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
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
    // 11: mul_sat r0.x, r0.x, cb0[15].w
    r0.x = (saturate((r0.xxxx)*(source[15].wwww))).x;
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
    // 17: mul r0.z, r0.z, cb0[15].y
    r0.z = ((r0.zzzz)*(source[15].yyyy)).z;
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
    r2.xy = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 38: mad r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((r1.zzzz)*(r2.xyxx)+(r1.xyxx)).xy;
    // 39: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 40: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 41: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyz = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xy = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 44: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: mul r3.xyzw, r1.xxyz, cb0[1].wxyz
    r3.xyzw = ((r1.xxyz)*(source[1].wxyz)).xyzw;
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
    r0.z = (ALTVNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 55: add r0.z, -r1.w, r0.z
    r0.z = ((-(r1.wwww))+(r0.zzzz)).z;
    // 56: mul_sat r0.z, r0.z, cb0[15].x
    r0.z = (saturate((r0.zzzz)*(source[15].xxxx))).z;
    // 57: mul r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)*(r3.xxxx)).z;
    // 58: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 59: mul_sat r0.y, r0.y, cb0[15].z
    r0.y = (saturate((r0.yyyy)*(source[15].zzzz))).y;
    // 60: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 61: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 62: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 63: mad r0.xyz, -cb0[1].xyzx, r1.xyzx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 64: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 65: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 71: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_o_pa_ri_04_ad_2s: 096d7ee0efa1eb4bb2e91b406fe61913; selected map d3ae7817e320de17bcfcd6b6dde23265c1c47d93ae32f74e18619b11001ac907.
float4 ALTVNative120(ALTV_NATIVE_INPUT input)
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
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

// fx_r_me_watertrail_02_01_tr: 0c986668f7b6b8438ad4e59533ac0d95; selected map 04f23ee5f526fb7f1b8f245b449c842f3c96ad9cc0aef03ea1bde8686efad069.
float4 ALTVNative121(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = input.dynamicParameter;
    source[4] = ALTVNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[3u].zzzz,g_ALTVSourceMaterialParameters[3u].wwww,1u);
    source[7].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[11].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[14].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    r1.yz = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 19: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 20: add r1.xy, cb0[3].xzxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[3].xzxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mad r2.x, r1.x, cb0[7].x, r0.z
    r2.x = ((r1.xxxx)*(source[7].xxxx)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[12].y, r0.w
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r2.xyxx, t2.xwyz, s2, l(0.000000)
    r1.xzw = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.x = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

// fx_m_me_watertrail_01_46_tr: 0c986668f7b6b8438ad4e59533ac0d95; selected map 04f23ee5f526fb7f1b8f245b449c842f3c96ad9cc0aef03ea1bde8686efad069.
float4 ALTVNative122(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = input.dynamicParameter;
    source[4] = ALTVNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[3u].zzzz,g_ALTVSourceMaterialParameters[3u].wwww,1u);
    source[7].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[10].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[11].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[12].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[14].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    r1.yz = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 19: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 20: add r1.xy, cb0[3].xzxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((source[3].xzxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mad r2.x, r1.x, cb0[7].x, r0.z
    r2.x = ((r1.xxxx)*(source[7].xxxx)+(r0.zzzz)).x;
    // 22: mad r2.y, r1.x, cb0[12].y, r0.w
    r2.y = ((r1.xxxx)*(source[12].yyyy)+(r0.wwww)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r2.xyxx, t2.xwyz, s2, l(0.000000)
    r1.xzw = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xwyz).xzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r0.x = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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

// fx_o_pa_circledisort_01_ad: 36a0459be7616a4bacc60514830f1037; selected map 2449918b9aefcd38dfba19a96c9200cd10fb6ac36a0aaaf38df7a2ecaaa60233.
float4 ALTVNative123(ALTV_NATIVE_INPUT input)
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

// fx_a_me_panning_01_ts_ad: 905554360efecf4e81927d2bf03deaa7; selected map 3745764fc4800dd265e1171186d40052b1d9f01814668929cf1c9e28149ae648.
float4 ALTVNative124(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[1u];
    source[3] = input.dynamicParameter;
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
    // 10: mul_sat r0.x, r0.x, l(0.034483)
    r0.x = (saturate((r0.xxxx)*(float4(0.034483,0.034483,0.034483,0.034483)))).x;
    // 11: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 12: add r1.y, r0.y, cb0[3].x
    r1.y = ((r0.yyyy)+(source[3].xxxx)).y;
    // 13: mul r1.x, v4.x, cb0[3].y
    r1.x = ((v4.xxxx)*(source[3].yyyy)).x;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 15: mul r0.y, r0.y, cb0[1].w
    r0.y = ((r0.yyyy)*(source[1].wwww)).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v4.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (ALTVNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 17: mul_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)*(r0.yyyy))).y;
    // 18: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 19: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 20: add r0.yzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((source[1].xxyz)+(source[2].xxyz)).yzw;
    // 21: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 22: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 23: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_cudi_odtrail_03_tr: 1a9d41a1ae76e04ebc77966256f38db0; selected map 9684bde04e23f82b8f8dbb3bc2747a41daa636f07440b8c7173b3abbb2529e27.
float4 ALTVNative125(ALTV_NATIVE_INPUT input)
{
    float4 source[26]; [unroll] for (uint i=0u; i<26u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[20u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[4].y = (g_ALTVSourceMaterialParameters[15u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[4].w = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].yyyy)).x;
    source[5].z = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[5].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[6].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[6].w = ((g_ALTVSourceMaterialParameters[8u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[7].x = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialParameters[16u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[8].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].wwww)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[9].y = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].w = ((g_ALTVSourceMaterialParameters[9u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].x = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[10].y = (g_ALTVSourceMaterialParameters[17u].wwww).x;
    source[10].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[10].w = (g_ALTVSourceMaterialParameters[16u].yyyy).x;
    source[11].x = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[11].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[10u].yyyy)).x;
    source[11].w = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[12].x = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[12].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[12].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[13].x = ((g_ALTVSourceMaterialParameters[10u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[13].y = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[13].z = (g_ALTVSourceMaterialParameters[18u].xxxx).x;
    source[13].w = (g_ALTVSourceMaterialParameters[17u].yyyy).x;
    source[14].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[14].y = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[15].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[15].y = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[15].z = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[15].w = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[16].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[16].z = ((g_ALTVSourceMaterialParameters[8u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[16].w = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[17].x = (g_ALTVSourceMaterialParameters[16u].zzzz).x;
    source[17].y = (g_ALTVSourceMaterialParameters[18u].yyyy).x;
    source[17].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[17].w = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[18].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[18].y = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[18].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].xxxx)).x;
    source[18].w = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[19].x = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[19].y = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[19].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[19].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[20].x = ((g_ALTVSourceMaterialParameters[8u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[20].y = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[20].z = (g_ALTVSourceMaterialParameters[16u].wwww).x;
    source[20].w = (g_ALTVSourceMaterialParameters[18u].zzzz).x;
    source[21].x = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[21].y = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[21].z = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[21].w = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[22].x = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[22].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[22].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[22].w = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[23].x = (g_ALTVSourceMaterialParameters[17u].xxxx).x;
    source[23].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[23].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[23].w = (g_ALTVSourceMaterialParameters[17u].zzzz).x;
    source[24].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[24].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[24].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[24].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[25].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[25].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
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
    // 1: mad r0.x, cb0[3].y, cb0[10].w, cb0[11].x
    r0.x = ((source[3].yyyy)*(source[10].wwww)+(source[11].xxxx)).x;
    // 2: mad r0.x, v4.y, cb0[10].z, r0.x
    r0.x = ((v4.yyyy)*(source[10].zzzz)+(r0.xxxx)).x;
    // 3: add r0.x, r0.x, cb0[11].z
    r0.x = ((r0.xxxx)+(source[11].zzzz)).x;
    // 4: mad r0.y, cb0[3].y, cb0[12].y, cb0[12].z
    r0.y = ((source[3].yyyy)*(source[12].yyyy)+(source[12].zzzz)).y;
    // 5: mad r0.y, cb0[12].w, v4.x, r0.y
    r0.y = ((source[12].wwww)*(v4.xxxx)+(r0.yyyy)).y;
    // 6: add r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)+(source[13].xxxx)).y;
    // 7: mad r1.x, cb0[11].w, r0.x, r0.y
    r1.x = ((source[11].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 8: mad r1.y, r0.y, cb0[13].y, r0.x
    r1.y = ((r0.yyyy)*(source[13].yyyy)+(r0.xxxx)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mul r0.xy, r0.xyxx, cb0[13].zzzz
    r0.xy = ((r0.xyxx)*(source[13].zzzz)).xy;
    // 11: mad r0.z, cb0[3].y, cb0[7].z, cb0[7].w
    r0.z = ((source[3].yyyy)*(source[7].zzzz)+(source[7].wwww)).z;
    // 12: mad r0.z, v4.y, cb0[7].y, r0.z
    r0.z = ((v4.yyyy)*(source[7].yyyy)+(r0.zzzz)).z;
    // 13: add r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)+(source[8].yyyy)).z;
    // 14: mad r0.w, cb0[3].y, cb0[9].x, cb0[9].y
    r0.w = ((source[3].yyyy)*(source[9].xxxx)+(source[9].yyyy)).w;
    // 15: mad r0.w, cb0[9].z, v4.x, r0.w
    r0.w = ((source[9].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 16: add r0.w, r0.w, cb0[9].w
    r0.w = ((r0.wwww)+(source[9].wwww)).w;
    // 17: mad r1.x, cb0[8].z, r0.z, r0.w
    r1.x = ((source[8].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 18: mad r1.y, r0.w, cb0[10].x, r0.z
    r1.y = ((r0.wwww)*(source[10].xxxx)+(r0.zzzz)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 20: mad r0.xy, cb0[10].yyyy, r0.zwzz, r0.xyxx
    r0.xy = ((source[10].yyyy)*(r0.zwzz)+(r0.xyxx)).xy;
    // 21: mul r0.xy, r0.xyxx, cb0[3].zzzz
    r0.xy = ((r0.xyxx)*(source[3].zzzz)).xy;
    // 22: mad r0.z, cb0[3].x, cb0[14].y, cb0[14].z
    r0.z = ((source[3].xxxx)*(source[14].yyyy)+(source[14].zzzz)).z;
    // 23: mad r0.z, v4.y, cb0[14].x, r0.z
    r0.z = ((v4.yyyy)*(source[14].xxxx)+(r0.zzzz)).z;
    // 24: add r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)+(source[15].xxxx)).z;
    // 25: mad r0.w, cb0[3].x, cb0[15].w, cb0[16].x
    r0.w = ((source[3].xxxx)*(source[15].wwww)+(source[16].xxxx)).w;
    // 26: mad r0.w, cb0[16].y, v4.x, r0.w
    r0.w = ((source[16].yyyy)*(v4.xxxx)+(r0.wwww)).w;
    // 27: add r0.w, r0.w, cb0[16].z
    r0.w = ((r0.wwww)+(source[16].zzzz)).w;
    // 28: mad r1.x, cb0[15].y, r0.z, r0.w
    r1.x = ((source[15].yyyy)*(r0.zzzz)+(r0.wwww)).x;
    // 29: mad r1.y, r0.w, cb0[16].w, r0.z
    r1.y = ((r0.wwww)*(source[16].wwww)+(r0.zzzz)).y;
    // 30: mad r0.zw, cb0[17].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[17].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t2.xyzw, s4, l(0.000000)
    r1.xyz = (ALTVNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 32: mul r1.xyzw, r1.xxyz, cb0[17].yyyy
    r1.xyzw = ((r1.xxyz)*(source[17].yyyy)).xyzw;
    // 33: mad r0.z, cb0[3].x, cb0[17].w, cb0[18].x
    r0.z = ((source[3].xxxx)*(source[17].wwww)+(source[18].xxxx)).z;
    // 34: mad r0.z, v4.y, cb0[17].z, r0.z
    r0.z = ((v4.yyyy)*(source[17].zzzz)+(r0.zzzz)).z;
    // 35: add r0.z, r0.z, cb0[18].z
    r0.z = ((r0.zzzz)+(source[18].zzzz)).z;
    // 36: mad r0.w, cb0[3].x, cb0[19].y, cb0[19].z
    r0.w = ((source[3].xxxx)*(source[19].yyyy)+(source[19].zzzz)).w;
    // 37: mad r0.w, cb0[19].w, v4.x, r0.w
    r0.w = ((source[19].wwww)*(v4.xxxx)+(r0.wwww)).w;
    // 38: add r0.w, r0.w, cb0[20].x
    r0.w = ((r0.wwww)+(source[20].xxxx)).w;
    // 39: mad r2.x, cb0[18].w, r0.z, r0.w
    r2.x = ((source[18].wwww)*(r0.zzzz)+(r0.wwww)).x;
    // 40: mad r2.y, r0.w, cb0[20].y, r0.z
    r2.y = ((r0.wwww)*(source[20].yyyy)+(r0.zzzz)).y;
    // 41: mad r0.zw, cb0[20].zzzz, r0.xxxy, r2.xxxy
    r0.zw = ((source[20].zzzz)*(r0.xxxy)+(r2.xxxy)).zw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s5, l(0.000000)
    r2.xyz = (ALTVNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: mul r2.xyzw, r2.xxyz, cb0[20].wwww
    r2.xyzw = ((r2.xxyz)*(source[20].wwww)).xyzw;
    // 44: mul r1.xyzw, r1.xyzw, r2.xyzw
    r1.xyzw = ((r1.xyzw)*(r2.xyzw)).xyzw;
    // 45: mad r0.z, cb0[3].x, cb0[21].y, cb0[21].z
    r0.z = ((source[3].xxxx)*(source[21].yyyy)+(source[21].zzzz)).z;
    // 46: mad r0.z, v4.y, cb0[21].x, r0.z
    r0.z = ((v4.yyyy)*(source[21].xxxx)+(r0.zzzz)).z;
    // 47: mad r0.w, cb0[3].x, cb0[22].x, cb0[22].y
    r0.w = ((source[3].xxxx)*(source[22].xxxx)+(source[22].yyyy)).w;
    // 48: mad r0.w, cb0[22].z, v4.x, r0.w
    r0.w = ((source[22].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 49: mad r2.x, cb0[21].w, r0.z, r0.w
    r2.x = ((source[21].wwww)*(r0.zzzz)+(r0.wwww)).x;
    // 50: mad r2.y, r0.w, cb0[22].w, r0.z
    r2.y = ((r0.wwww)*(source[22].wwww)+(r0.zzzz)).y;
    // 51: mad r0.zw, cb0[23].xxxx, r0.xxxy, r2.xxxy
    r0.zw = ((source[23].xxxx)*(r0.xxxy)+(r2.xxxy)).zw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t4.xyzw, s6, l(0.000000)
    r2.xyz = (ALTVNativeSample5((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 53: mul r1.xyzw, r1.xyzw, r2.xxyz
    r1.xyzw = ((r1.xyzw)*(r2.xxyz)).xyzw;
    // 54: max r1.xyzw, |r1.xyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 55: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 56: mul r0.z, cb0[3].w, cb0[23].y
    r0.z = ((source[3].wwww)*(source[23].yyyy)).z;
    // 57: mul r1.xyzw, r1.xyzw, r0.zzzz
    r1.xyzw = ((r1.xyzw)*(r0.zzzz)).xyzw;
    // 58: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 59: mul_sat r1.xyzw, r1.xyzw, cb0[23].zzzz
    r1.xyzw = (saturate((r1.xyzw)*(source[23].zzzz))).xyzw;
    // 60: mad r0.z, cb0[3].x, cb0[4].y, cb0[4].z
    r0.z = ((source[3].xxxx)*(source[4].yyyy)+(source[4].zzzz)).z;
    // 61: mad r0.z, v4.y, cb0[4].x, r0.z
    r0.z = ((v4.yyyy)*(source[4].xxxx)+(r0.zzzz)).z;
    // 62: add r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)+(source[5].yyyy)).z;
    // 63: mad r0.w, cb0[3].x, cb0[6].x, cb0[6].y
    r0.w = ((source[3].xxxx)*(source[6].xxxx)+(source[6].yyyy)).w;
    // 64: mad r0.w, cb0[6].z, v4.x, r0.w
    r0.w = ((source[6].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 65: add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // 66: mad r2.x, cb0[5].z, r0.z, r0.w
    r2.x = ((source[5].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 67: mad r2.y, r0.w, cb0[7].x, r0.z
    r2.y = ((r0.wwww)*(source[7].xxxx)+(r0.zzzz)).y;
    // 68: mad r0.xy, cb0[13].wwww, r0.xyxx, r2.xyxx
    r0.xy = ((source[13].wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s3, l(0.000000)
    r0.xyz = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 70: mul r2.xyz, r1.yzwy, r0.xyzx
    r2.xyz = ((r1.yzwy)*(r0.xyzx)).xyz;
    // 71: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 72: mad r0.xyz, -r0.xyzx, r1.yzwy, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(r1.yzwy)+(r0.wwww)).xyz;
    // 73: mad r0.xyz, cb0[23].wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((source[23].wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 74: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 75: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 76: mul r0.xyz, r0.xyzx, cb0[24].xxxx
    r0.xyz = ((r0.xyzx)*(source[24].xxxx)).xyz;
    // 77: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 78: mad r0.xyz, cb0[24].yyyy, r0.xyzx, cb0[24].zzzz
    r0.xyz = ((source[24].yyyy)*(r0.xyzx)+(source[24].zzzz)).xyz;
    // 79: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 80: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 81: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 82: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 83: source device depth mapped to centimetre view depth; reconstruction at 85.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 85-88: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 89: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 90: add r0.y, -cb0[24].w, l(1.000000)
    r0.y = ((-(source[24].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 91: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 92: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 93: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 94: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 95: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 96: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 97: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 98: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 99: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 100: mul r0.z, r0.z, cb0[25].x
    r0.z = ((r0.zzzz)*(source[25].xxxx)).z;
    // 101: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 102: mul_sat r0.z, r0.z, cb0[25].y
    r0.z = (saturate((r0.zzzz)*(source[25].yyyy))).z;
    // 103: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 104: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 105: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// fx_n_pa_linearwave_01_55_tr: ecd406dc7852eb449d9323ed7f4b240d; selected map 0faf6856e8f56474c5cb36396952e2d6b6545e0458c166b7ec9059a48832ed20.
float4 ALTVNative126(ALTV_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[13u];
    source[2] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ALTVSourceMaterialParameters[12u];
    source[7].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[8].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].xxxx)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[8].w = ((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos((g_ALTVSourceMaterialParameters[4u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[12].w = (floor(g_ALTVSourceMaterialParameters[8u].wwww)).x;
    source[13].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[13].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[13].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[13].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].wwww)).x;
    source[14].x = (cos((g_ALTVSourceMaterialParameters[6u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[14].y = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[15].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[15].y = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[15].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[15].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[16].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[16].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[16].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[17].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[17].y = (floor(g_ALTVSourceMaterialParameters[7u].zzzz)).x;
    source[17].z = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[17].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[18].x = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[18].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[18].w = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[19].x = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[19].y = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[19].z = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[19].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 1: mad r0.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
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
    // 22: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 23: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 24: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 25: mov r0.y, -r0.x
    r0.y = (-(r0.xxxx)).y;
    // 26: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 27: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 28: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 29: mul r0.z, r0.z, l(0.159155)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))).z;
    // 30: frc r0.x, r0.z
    r0.x = (frac(r0.zzzz)).x;
    // 31: add r0.xy, r0.xyxx, l(-0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,0.500000,0.000000,0.000000))).xy;
    // 32: mad r0.zw, cb0[9].wwww, r0.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((source[9].wwww)*(r0.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 33: mad r0.xy, cb0[14].yyyy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[14].yyyy)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 34: mul r0.w, r0.w, cb0[10].y
    r0.w = ((r0.wwww)*(source[10].yyyy)).w;
    // 35: mad r1.x, r0.z, cb0[10].x, cb0[10].z
    r1.x = ((r0.zzzz)*(source[10].xxxx)+(source[10].zzzz)).x;
    // 36: mad r1.y, cb0[10].w, v4.x, r0.w
    r1.y = ((source[10].wwww)*(v4.xxxx)+(r0.wwww)).y;
    // 37: add r0.zw, r1.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 38: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 39: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 40: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 41: mad r1.xy, v2.xyxx, cb0[11].xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(source[11].xyxx)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.xy = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 43: mul r1.z, v4.w, cb0[11].z
    r1.z = ((v4.wwww)*(source[11].zzzz)).z;
    // 44: mad r0.zw, r1.zzzz, r1.xxxy, r0.zzzw
    r0.zw = ((r1.zzzz)*(r1.xxxy)+(r0.zzzw)).zw;
    // 45: mul r1.z, v2.x, cb0[11].w
    r1.z = ((v2.xxxx)*(source[11].wwww)).z;
    // 46: mul r1.w, v2.y, cb0[12].x
    r1.w = ((v2.yyyy)*(source[12].xxxx)).w;
    // 47: add r1.xy, r1.zwzz, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s3, l(0.000000)
    r1.xy = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 49: mad r0.zw, cb0[12].yyyy, r1.xxxy, r0.zzzw
    r0.zw = ((source[12].yyyy)*(r1.xxxy)+(r0.zzzw)).zw;
    // 50: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s1, cb0[8].y
    r0.z = (ALTVNativeSample1((r0.zwzz).xy, (source[8].yyyy).x, true).yzxw).z;
    // 51: mad r0.z, cb0[12].w, -r0.z, r0.z
    r0.z = ((source[12].wwww)*(-(r0.zzzz))+(r0.zzzz)).z;
    // 52: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 53: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 54: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 55: mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // 56: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 57: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 58: mul r0.y, r0.y, cb0[14].w
    r0.y = ((r0.yyyy)*(source[14].wwww)).y;
    // 59: mad r1.x, r0.x, cb0[14].z, cb0[15].x
    r1.x = ((r0.xxxx)*(source[14].zzzz)+(source[15].xxxx)).x;
    // 60: mad r1.y, cb0[15].y, v4.y, r0.y
    r1.y = ((source[15].yyyy)*(v4.yyyy)+(r0.yyyy)).y;
    // 61: add r0.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 62: dp2 r1.x, cb0[4].xyxx, r0.xyxx
    r1.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 63: dp2 r1.y, cb0[5].xyxx, r0.xyxx
    r1.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 64: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: mad r1.xy, v2.xyxx, cb0[15].zwzz, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(source[15].zwzz)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t3.xyzw, s5, l(0.000000)
    r1.xy = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 67: mad r0.xy, cb0[16].xxxx, r1.xyxx, r0.xyxx
    r0.xy = ((source[16].xxxx)*(r1.xyxx)+(r0.xyxx)).xy;
    // 68: mad r1.xy, v2.xyxx, cb0[16].yzyy, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(source[16].yzyy)+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t4.xyzw, s6, l(0.000000)
    r1.xy = (ALTVNativeSample6((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 70: mad r0.xy, cb0[16].wwww, r1.xyxx, r0.xyxx
    r0.xy = ((source[16].wwww)*(r1.xyxx)+(r0.xyxx)).xy;
    // 71: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s4, cb0[13].w
    r0.x = (ALTVNativeSample4((r0.xyxx).xy, (source[13].wwww).x, true).xyzw).x;
    // 72: mad r0.x, cb0[17].y, -r0.x, r0.x
    r0.x = ((source[17].yyyy)*(-(r0.xxxx))+(r0.xxxx)).x;
    // 73: mul r0.x, r0.x, cb0[17].z
    r0.x = ((r0.xxxx)*(source[17].zzzz)).x;
    // 74: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 75: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 76: mul r0.x, r0.x, cb0[17].w
    r0.x = ((r0.xxxx)*(source[17].wwww)).x;
    // 77: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 78: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 79: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 80: mul r0.x, r0.x, cb0[18].x
    r0.x = ((r0.xxxx)*(source[18].xxxx)).x;
    // 81: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 82: mul r0.y, r0.y, cb0[18].w
    r0.y = ((r0.yyyy)*(source[18].wwww)).y;
    // 83: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 84: mul r0.y, r0.y, cb0[19].x
    r0.y = ((r0.yyyy)*(source[19].xxxx)).y;
    // 85: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 86: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 87: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 88: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 89: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 90: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 91: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 92: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 93: mad r1.x, cb0[19].y, l(10.000000), l(10.000000)
    r1.x = ((source[19].yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 94: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 95: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 96: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 97: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r0.z, r0.z, cb0[19].z
    r0.z = ((r0.zzzz)*(source[19].zzzz)).z;
    // 99: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 100: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 101: mul r0.z, r0.z, cb0[19].w
    r0.z = ((r0.zzzz)*(source[19].wwww)).z;
    // 102: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 103: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 104: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 105: mad r0.yz, v2.xxyx, cb0[7].yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((v2.xxyx)*(source[7].yyzy)+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 106: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t6.wxyz, s0, cb0[7].x
    r0.yzw = (ALTVNativeSample0((r0.yzyy).xy, (source[7].xxxx).x, true).wxyz).yzw;
    // 107: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 108: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 109: mad r0.yzw, cb0[7].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 110: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 111: mul r0.xyz, r0.xyzx, cb0[18].yyyy
    r0.xyz = ((r0.xyzx)*(source[18].yyyy)).xyz;
    // 112: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 113: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 114: mul r0.xyz, r0.xyzx, cb0[18].zzzz
    r0.xyz = ((r0.xyzx)*(source[18].zzzz)).xyz;
    // 115: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 116: mul_sat r0.xyz, r0.xyzx, cb0[6].xyzx
    r0.xyz = (saturate((r0.xyzx)*(source[6].xyzx))).xyz;
    // 117: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 118: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_pa_shorkwave_01_8_tr: 5c707b8a1ddaf5478de6e37c4b55bf26; selected map f1f290972b5b31eec74b516c45ee383023e1c16a6224be179cff954bdcae7562.
float4 ALTVNative127(ALTV_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[3u];
    source[2] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4].x = (cos((g_ALTVSourceMaterialParameters[1u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[4].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[5].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
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
    // 28: mul r1.x, r0.y, cb0[6].x
    r1.x = ((r0.yyyy)*(source[6].xxxx)).x;
    // 29: mul r2.xy, r0.yyyy, cb0[4].ywyy
    r2.xy = ((r0.yyyy)*(source[4].ywyy)).xy;
    // 30: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 31: mul r0.y, r0.y, l(0.800000)
    r0.y = ((r0.yyyy)*(float4(0.800000,0.800000,0.800000,0.800000))).y;
    // 32: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 33: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 35: mad r0.x, -r0.x, l(2.222222), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.222222,2.222222,2.222222,2.222222))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mul r0.x, r0.x, l(0.333333)
    r0.x = ((r0.xxxx)*(float4(0.333333,0.333333,0.333333,0.333333))).x;
    // 37: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 38: mul r0.x, r0.x, l(4.000000)
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 39: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 41: mul r2.z, r0.y, cb0[5].x
    r2.z = ((r0.yyyy)*(source[5].xxxx)).z;
    // 42: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, -0.030000, -0.100000), r2.yyyz
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,-0.030000,-0.100000))+(r2.yyyz)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (ALTVNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 44: mad r3.xyzw, r0.zwzw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.500000, -1.500000)
    r3.xyzw = ((r0.zwzw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.500000,-1.500000))).xyzw;
    // 45: mul r1.y, r0.y, cb0[6].y
    r1.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 46: mul r2.w, r0.y, cb0[4].z
    r2.w = ((r0.yyyy)*(source[4].zzzz)).w;
    // 47: mad r0.yz, r3.xxyx, l(0.000000, 0.080000, 0.080000, 0.000000), r1.xxyx
    r0.yz = ((r3.xxyx)*(float4(0.000000,0.080000,0.080000,0.000000))+(r1.xxyx)).yz;
    // 48: mad r0.yz, v4.zzzz, l(0.000000, -0.050000, -0.100000, 0.000000), r0.yyzy
    r0.yz = ((v4.zzzz)*(float4(0.000000,-0.050000,-0.100000,0.000000))+(r0.yyzy)).yz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s2, l(0.000000)
    r0.y = (ALTVNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 50: add r0.zw, v4.yyyx, l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((v4.yyyx)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 51: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 52: add_sat r0.y, r0.y, r0.y
    r0.y = (saturate((r0.yyyy)+(r0.yyyy))).y;
    // 53: dp2 r1.x, l(0.000796, -1.000000, 0.000000, 0.000000), r3.zwzz
    r1.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).x;
    // 54: dp2 r1.y, l(1.000000, 0.000796, 0.000000, 0.000000), r3.zwzz
    r1.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r3.zwzz).xy).xxxx).y;
    // 55: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: mad r1.xy, r0.zzzz, r1.xyxx, r2.xwxx
    r1.xy = ((r0.zzzz)*(r1.xyxx)+(r2.xwxx)).xy;
    // 57: mad r1.z, v4.w, l(0.200000), r1.y
    r1.z = ((v4.wwww)*(float4(0.200000,0.200000,0.200000,0.200000))+(r1.yyyy)).z;
    // 58: add r0.zw, r1.xxxz, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r1.xxxz)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 59: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 60: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 61: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ALTVNativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 63: add r0.z, r1.x, r1.x
    r0.z = ((r1.xxxx)+(r1.xxxx)).z;
    // 64: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 65: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 66: mul_sat r0.x, r0.y, r0.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx))).x;
    // 67: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 68: max r0.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 69: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 70: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 71: mul r0.xyz, r0.xyzx, cb0[5].yyyy
    r0.xyz = ((r0.xyzx)*(source[5].yyyy)).xyz;
    // 72: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 73: mad r0.xyz, cb0[5].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[5].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 74: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 75: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}
