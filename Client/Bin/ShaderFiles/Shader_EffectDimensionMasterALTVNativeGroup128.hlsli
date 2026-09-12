#ifndef ALTV_NATIVE_CAPTURE_ONLY
// Single source owner for ALTVNative profiles 128..191.
// fx_m_me_boxshockwaveline_01_tr: 498f181c974da1419002ef9160f610d9; selected map 7b1b05b99d3ce26b110c9d56f80c9577f723b1684600e8a69dad89fd1c640c6c.
float4 ALTVNative128(ALTV_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[18u];
    source[3] = g_ALTVSourceMaterialParameters[16u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[5].y = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].zzzz)).x;
    source[6].z = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[6].w = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[7u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[8].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[8].z = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[9].x = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[9].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].w = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[11].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[12].z = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[13].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[14].y = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[14].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[14].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[15].x = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[15].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[15].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[16].y = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[16].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[17].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[17].z = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[17].w = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[18].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[18].z = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[18].w = (g_ALTVSourceMaterialParameters[15u].wwww).x;
    source[19].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[19].y = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[19].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[19].w = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[20].x = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[20].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[20].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[20].w = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[21].x = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[21].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[21].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
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
    // 1: mad r0.x, cb0[4].y, cb0[10].w, cb0[11].x
    r0.x = ((source[4].yyyy)*(source[10].wwww)+(source[11].xxxx)).x;
    // 2: mad r0.x, v4.y, cb0[10].z, r0.x
    r0.x = ((v4.yyyy)*(source[10].zzzz)+(r0.xxxx)).x;
    // 3: mad r0.y, cb0[4].y, cb0[11].z, cb0[11].w
    r0.y = ((source[4].yyyy)*(source[11].zzzz)+(source[11].wwww)).y;
    // 4: mad r0.y, cb0[12].x, v4.x, r0.y
    r0.y = ((source[12].xxxx)*(v4.xxxx)+(r0.yyyy)).y;
    // 5: mad r1.x, cb0[11].y, r0.x, r0.y
    r1.x = ((source[11].yyyy)*(r0.xxxx)+(r0.yyyy)).x;
    // 6: mad r1.y, r0.y, cb0[12].y, r0.x
    r1.y = ((r0.yyyy)*(source[12].yyyy)+(r0.xxxx)).y;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 8: mul r0.xy, r0.xyxx, cb0[12].zzzz
    r0.xy = ((r0.xyxx)*(source[12].zzzz)).xy;
    // 9: mad r0.z, cb0[4].y, cb0[8].z, cb0[8].w
    r0.z = ((source[4].yyyy)*(source[8].zzzz)+(source[8].wwww)).z;
    // 10: mad r0.z, v4.y, cb0[8].y, r0.z
    r0.z = ((v4.yyyy)*(source[8].yyyy)+(r0.zzzz)).z;
    // 11: mad r0.w, cb0[4].y, cb0[9].y, cb0[9].z
    r0.w = ((source[4].yyyy)*(source[9].yyyy)+(source[9].zzzz)).w;
    // 12: mad r0.w, cb0[9].w, v4.x, r0.w
    r0.w = ((source[9].wwww)*(v4.xxxx)+(r0.wwww)).w;
    // 13: mad r1.x, cb0[9].x, r0.z, r0.w
    r1.x = ((source[9].xxxx)*(r0.zzzz)+(r0.wwww)).x;
    // 14: mad r1.y, r0.w, cb0[10].x, r0.z
    r1.y = ((r0.wwww)*(source[10].xxxx)+(r0.zzzz)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 16: mad r0.xy, cb0[10].yyyy, r0.zwzz, r0.xyxx
    r0.xy = ((source[10].yyyy)*(r0.zwzz)+(r0.xyxx)).xy;
    // 17: mul r0.xy, r0.xyxx, cb0[4].zzzz
    r0.xy = ((r0.xyxx)*(source[4].zzzz)).xy;
    // 18: mad r0.z, cb0[4].x, cb0[16].w, cb0[17].x
    r0.z = ((source[4].xxxx)*(source[16].wwww)+(source[17].xxxx)).z;
    // 19: mad r0.z, v4.y, cb0[16].z, r0.z
    r0.z = ((v4.yyyy)*(source[16].zzzz)+(r0.zzzz)).z;
    // 20: mad r0.w, cb0[4].x, cb0[17].z, cb0[17].w
    r0.w = ((source[4].xxxx)*(source[17].zzzz)+(source[17].wwww)).w;
    // 21: mad r0.w, cb0[18].x, v4.x, r0.w
    r0.w = ((source[18].xxxx)*(v4.xxxx)+(r0.wwww)).w;
    // 22: mad r1.x, cb0[17].y, r0.z, r0.w
    r1.x = ((source[17].yyyy)*(r0.zzzz)+(r0.wwww)).x;
    // 23: mad r1.y, r0.w, cb0[18].y, r0.z
    r1.y = ((r0.wwww)*(source[18].yyyy)+(r0.zzzz)).y;
    // 24: mad r0.zw, cb0[18].zzzz, r0.xxxy, r1.xxxy
    r0.zw = ((source[18].zzzz)*(r0.xxxy)+(r1.xxxy)).zw;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s4, l(0.000000)
    r0.z = (ALTVNativeSample4((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 26: mul r0.z, r0.z, cb0[18].w
    r0.z = ((r0.zzzz)*(source[18].wwww)).z;
    // 27: mad r0.w, cb0[4].x, cb0[14].y, cb0[14].z
    r0.w = ((source[4].xxxx)*(source[14].yyyy)+(source[14].zzzz)).w;
    // 28: mad r0.w, v4.y, cb0[14].x, r0.w
    r0.w = ((v4.yyyy)*(source[14].xxxx)+(r0.wwww)).w;
    // 29: mad r1.x, cb0[4].x, cb0[15].x, cb0[15].y
    r1.x = ((source[4].xxxx)*(source[15].xxxx)+(source[15].yyyy)).x;
    // 30: mad r1.x, cb0[15].z, v4.x, r1.x
    r1.x = ((source[15].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 31: mad r2.x, cb0[14].w, r0.w, r1.x
    r2.x = ((source[14].wwww)*(r0.wwww)+(r1.xxxx)).x;
    // 32: mad r2.y, r1.x, cb0[15].w, r0.w
    r2.y = ((r1.xxxx)*(source[15].wwww)+(r0.wwww)).y;
    // 33: mad r1.xy, cb0[16].xxxx, r0.xyxx, r2.xyxx
    r1.xy = ((source[16].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 35: mad r0.z, cb0[16].y, r0.w, r0.z
    r0.z = ((source[16].yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 36: mad r0.w, cb0[4].x, cb0[19].y, cb0[19].z
    r0.w = ((source[4].xxxx)*(source[19].yyyy)+(source[19].zzzz)).w;
    // 37: mad r0.w, v4.y, cb0[19].x, r0.w
    r0.w = ((v4.yyyy)*(source[19].xxxx)+(r0.wwww)).w;
    // 38: mad r1.x, cb0[4].x, cb0[20].x, cb0[20].y
    r1.x = ((source[4].xxxx)*(source[20].xxxx)+(source[20].yyyy)).x;
    // 39: mad r1.x, cb0[20].z, v4.x, r1.x
    r1.x = ((source[20].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 40: mad r2.x, cb0[19].w, r0.w, r1.x
    r2.x = ((source[19].wwww)*(r0.wwww)+(r1.xxxx)).x;
    // 41: mad r2.y, r1.x, cb0[20].w, r0.w
    r2.y = ((r1.xxxx)*(source[20].wwww)+(r0.wwww)).y;
    // 42: mad r1.xy, cb0[21].xxxx, r0.xyxx, r2.xyxx
    r1.xy = ((source[21].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s5, l(0.000000)
    r0.w = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 44: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 45: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 46: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 47: mul r0.w, cb0[4].w, cb0[21].y
    r0.w = ((source[4].wwww)*(source[21].yyyy)).w;
    // 48: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 49: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 50: mul_sat r0.z, r0.z, cb0[21].z
    r0.z = (saturate((r0.zzzz)*(source[21].zzzz))).z;
    // 51: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 52: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 53: mad r0.w, cb0[4].x, cb0[5].y, cb0[5].z
    r0.w = ((source[4].xxxx)*(source[5].yyyy)+(source[5].zzzz)).w;
    // 54: mad r0.w, v4.y, cb0[5].x, r0.w
    r0.w = ((v4.yyyy)*(source[5].xxxx)+(r0.wwww)).w;
    // 55: add r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)+(source[6].yyyy)).w;
    // 56: mad r1.x, cb0[4].x, cb0[7].x, cb0[7].y
    r1.x = ((source[4].xxxx)*(source[7].xxxx)+(source[7].yyyy)).x;
    // 57: mad r1.x, cb0[7].z, v4.x, r1.x
    r1.x = ((source[7].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 58: add r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)+(source[7].wwww)).x;
    // 59: mad r2.x, cb0[6].z, r0.w, r1.x
    r2.x = ((source[6].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 60: mad r2.y, r1.x, cb0[8].x, r0.w
    r2.y = ((r1.xxxx)*(source[8].xxxx)+(r0.wwww)).y;
    // 61: mad r0.xy, cb0[12].wwww, r0.xyxx, r2.xyxx
    r0.xy = ((source[12].wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t5.xywz, s2, l(0.000000)
    r0.xyw = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 63: dp3 r1.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 64: add r1.xyz, -r0.xywx, r1.xxxx
    r1.xyz = ((-(r0.xywx))+(r1.xxxx)).xyz;
    // 65: mad r0.xyw, cb0[13].xxxx, r1.xyxz, r0.xyxw
    r0.xyw = ((source[13].xxxx)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 66: max r0.xyw, |r0.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r0.xyw = (max(abs(r0.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 67: log r0.xyw, r0.xyxw
    r0.xyw = (log2(r0.xyxw)).xyw;
    // 68: mul r0.xyw, r0.xyxw, cb0[13].yyyy
    r0.xyw = ((r0.xyxw)*(source[13].yyyy)).xyw;
    // 69: exp r0.xyw, r0.xyxw
    r0.xyw = (exp2(r0.xyxw)).xyw;
    // 70: mad r0.xyw, cb0[13].zzzz, r0.xyxw, cb0[13].wwww
    r0.xyw = ((source[13].zzzz)*(r0.xyxw)+(source[13].wwww)).xyw;
    // 71: mul r0.xyw, r0.xyxw, cb0[1].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)).xyw;
    // 72: mad r0.xyw, cb0[3].xyxz, r0.xyxw, cb0[2].xyxz
    r0.xyw = ((source[3].xyxz)*(r0.xyxw)+(source[2].xyxz)).xyw;
    // 73: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 74: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 75: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_d_pa_circ_01_2_tr: 653a5bb9cb1a754193f542b0fda91232; selected map b9a00a284058a01b21ea0d6fd6eec705cebc4a6f9f17beb3a1ba17da3210361f.
float4 ALTVNative129(ALTV_NATIVE_INPUT input)
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

// fx_d_pa_ringmaster_01_204_ts_dt_ad: 14767d574ea1a24a978b41f3ef3ab4bb; selected map 76128b62e60f0afce452a5f3a5fb24fc28464314aec0fb525ab66906bf5caab8.
float4 ALTVNative130(ALTV_NATIVE_INPUT input)
{
    float4 source[11]; [unroll] for (uint i=0u; i<11u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[4u];
    source[2] = g_ALTVSourceMaterialParameters[3u];
    source[3] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[5].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].wwww)).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].wwww))).x;
    source[5].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[6].x = ((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[1u].xxxx)).x;
    source[6].y = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[1u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[6].z = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[1u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[6].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[7].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].yyyy)).x;
    source[7].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].yyyy))).x;
    source[7].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[7].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].zzzz)).x;
    source[8].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[8].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[9].x = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].wwww)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 10: add r0.y, -cb0[10].x, l(1.000000)
    r0.y = ((-(source[10].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 14: dp2 r0.w, r0.yzyy, r0.yzyy
    r0.w = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: mad r1.x, -r0.w, cb0[5].z, l(1.000000)
    r1.x = ((-(r0.wwww))*(source[5].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mad r0.w, -r0.w, cb0[7].y, l(1.000000)
    r0.w = ((-(r0.wwww))*(source[7].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mul_sat r0.w, r0.w, cb0[8].y
    r0.w = (saturate((r0.wwww)*(source[8].yyyy))).w;
    // 19: mul_sat r1.x, r1.x, cb0[6].z
    r1.x = (saturate((r1.xxxx)*(source[6].zzzz))).x;
    // 20: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 21: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 22: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 23: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 24: mul r1.x, r1.x, cb0[8].z
    r1.x = ((r1.xxxx)*(source[8].zzzz)).x;
    // 25: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 26: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 27: dp2 r1.x, cb0[3].xyxx, r0.yzyy
    r1.x = (dot((source[3].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 28: dp2 r1.y, cb0[4].xyxx, r0.yzyy
    r1.y = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 29: mad r0.yz, cb0[9].yyyy, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((source[9].yyyy)*(r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 31: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 32: mul_sat r0.y, r0.y, cb0[9].z
    r0.y = (saturate((r0.yyyy)*(source[9].zzzz))).y;
    // 33: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 34: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 35: mul r0.z, r0.z, cb0[9].w
    r0.z = ((r0.zzzz)*(source[9].wwww)).z;
    // 36: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 37: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 38: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 39: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 40: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 41: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 42: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 43: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 44: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 45: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// bfx_c_pa_lightflare_01_ddt_4_ad: 62d22e79ebe9a24b98406751d77a5ac9; selected map 059656cc63615f570a09b85d568328904dd0a9354e3275e24447a0a5b6966d25.
float4 ALTVNative131(ALTV_NATIVE_INPUT input)
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

// fx_j_pa_highpixelring_01_05_ad: c0343bbdf1fcc04c8584c9955e2f1cd3; selected map 6531357a44b97566139e37daafa06fdad07397f79c40be7e4efdfe202a77429b.
float4 ALTVNative132(ALTV_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[5u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[4u].xxxx,g_ALTVSourceMaterialParameters[4u].yyyy,1u);
    source[3] = ALTVNativeAppend(ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ALTVSourceMaterialParameters[6u];
    source[6] = ALTVNativeAppend(ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))),1u);
    source[7].x = ((g_ALTVSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0))).x;
    source[7].y = (ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0199999996, 0.0, 0.0, 0.0)))).x;
    source[7].z = (ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[8].w = ((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0))).x;
    source[9].x = (ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.100000001, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[9].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[10].x = ((g_ALTVSourceMaterialParameters[1u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[10].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[1u].yyyy)).x;
    source[10].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[12].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
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
    // 32: mad r0.xy, r0.xyxx, cb0[9].yzyy, cb0[10].xzxx
    r0.xy = ((r0.xyxx)*(source[9].yzyy)+(source[10].xzxx)).xy;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r0.x = (ALTVNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 34: mad r0.xy, r0.xxxx, cb0[10].wwww, v2.xyxx
    r0.xy = ((r0.xxxx)*(source[10].wwww)+(v2.xyxx)).xy;
    // 35: mad r0.xy, r0.xyxx, cb0[11].xyxx, cb0[11].zwzz
    r0.xy = ((r0.xyxx)*(source[11].xyxx)+(source[11].zwzz)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r0.x = (ALTVNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 37: mul r0.x, r0.x, cb0[12].x
    r0.x = ((r0.xxxx)*(source[12].xxxx)).x;
    // 38: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 39: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 40: mul r0.y, r0.y, cb0[12].y
    r0.y = ((r0.yyyy)*(source[12].yyyy)).y;
    // 41: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 42: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 43: add r0.yz, v2.xxyx, cb0[3].xxyx
    r0.yz = ((v2.xxyx)+(source[3].xxyx)).yz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t0.zxyw, s0, l(0.000000)
    r0.yz = (ALTVNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 45: mad r0.yz, r0.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 46: mul r0.yz, r0.yyzy, l(0.000000, 0.050000, 0.050000, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.050000,0.050000,0.000000))).yz;
    // 47: mad r0.yz, cb0[2].xxyx, v2.xxyx, r0.yyzy
    r0.yz = ((source[2].xxyx)*(v2.xxyx)+(r0.yyzy)).yz;
    // 48: add r0.yz, r0.yyzy, cb0[4].xxyx
    r0.yz = ((r0.yyzy)+(source[4].xxyx)).yz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 50: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 51: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 52: mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // 53: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 54: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 55: add r0.z, v4.y, cb0[8].x
    r0.z = ((v4.yyyy)+(source[8].xxxx)).z;
    // 56: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 57: mul_sat r0.yz, r0.yyyy, l(0.000000, 10.000000, 8.000000, 0.000000)
    r0.yz = (saturate((r0.yyyy)*(float4(0.000000,10.000000,8.000000,0.000000)))).yz;
    // 58: add r0.z, -r0.z, r0.y
    r0.z = ((-(r0.zzzz))+(r0.yyyy)).z;
    // 59: mul r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)*(source[8].yyyy)).z;
    // 60: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 61: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 62: mul r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)*(source[8].zzzz)).w;
    // 63: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 64: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 65: mul r1.xyz, v3.xyzx, cb0[5].xyzx
    r1.xyz = ((v3.xyzx)*(source[5].xyzx)).xyz;
    // 66: mad r2.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), cb0[6].xyxx
    r2.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(source[6].xyxx)).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 68: mul r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))*(abs(r0.wwww))).w;
    // 69: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 70: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 71: movc r1.xyz, r0.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 72: mad r1.xyz, r0.zzzz, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 73: add r0.xzw, r0.xxxx, r1.xxyz
    r0.xzw = ((r0.xxxx)+(r1.xxyz)).xzw;
    // 74: mad r0.xzw, r0.xxzw, v3.xxyz, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)*(v3.xxyz)+(source[1].xxyz)).xzw;
    // 75: mul r0.xzw, r0.xxzw, v5.wwww
    r0.xzw = ((r0.xxzw)*(v5.wwww)).xzw;
    // 76: add r1.x, v4.x, cb0[12].z
    r1.x = ((v4.xxxx)+(source[12].zzzz)).x;
    // 77: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 78: mad r1.x, r1.x, l(10.000000), l(10.000000)
    r1.x = ((r1.xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 79: add r1.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 80: mul r1.yz, r1.yyzy, r1.yyzy
    r1.yz = ((r1.yyzy)*(r1.yyzy)).yz;
    // 81: add r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)+(r1.yyyy)).y;
    // 82: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 83: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 84: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)*(r1.xxxx)).x;
    // 86: mul r1.z, r1.z, l(10.000000)
    r1.z = ((r1.zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // 87: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 88: mul r1.z, r1.z, l(5.000000)
    r1.z = ((r1.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 89: round_ni r1.z, r1.z
    r1.z = (floor(r1.zzzz)).z;
    // 90: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 91: movc r1.z, r1.y, l(0), r1.z
    r1.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 92: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 93: mul r1.x, r1.x, l(5.000000)
    r1.x = ((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 94: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 95: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 96: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 97: add r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)+(r1.zzzz)).x;
    // 98: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 99: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 100: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 101: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 102: mul o0.xyz, r0.yyyy, r0.xzwx
    output.xyz = ((r0.yyyy)*(r0.xzwx)).xyz;
    // 103: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_a_pa_gl_01_9_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ALTVNative133(ALTV_NATIVE_INPUT input)
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

// fx_a_pa_gl_01_5_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ALTVNative134(ALTV_NATIVE_INPUT input)
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

// fx_a_pa_gl_01_6_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ALTVNative135(ALTV_NATIVE_INPUT input)
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

// fx_b_pa_gl_01_4_ad: 8b4801a9801c444caa83855e137d58f2; selected map 87fda8ea57a1afa755cb1c98d9c5b13a9bb3ea2705562d75a2868daf1f283b11.
float4 ALTVNative136(ALTV_NATIVE_INPUT input)
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

// fx_d_pa_master_01_112_ts_dt5_ad: c2e4b3708023dc4fa7b963af459ecb19; selected map 10f6e9f516e2e21cc594c1ffb2bb4833c2af725a701ee2915ea125e9e532d351.
float4 ALTVNative137(ALTV_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[5u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[1u].wwww,g_ALTVSourceMaterialParameters[2u].xxxx,1u);
    source[3] = g_ALTVSourceMaterialParameters[4u];
    source[4].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[5].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[6].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].yyyy)).x;
    source[6].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[7].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 1: add r0.x, v4.y, l(-1.000000)
    r0.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mad r0.y, cb0[4].y, cb0[5].w, cb0[6].x
    r0.y = ((source[4].yyyy)*(source[5].wwww)+(source[6].xxxx)).y;
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
    // 10: mad r0.x, r0.y, cb0[2].x, r0.x
    r0.x = ((r0.yyyy)*(source[2].xxxx)+(r0.xxxx)).x;
    // 11: mul r0.z, r0.w, cb0[2].y
    r0.z = ((r0.wwww)*(source[2].yyyy)).z;
    // 12: add r0.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 14: mul r0.w, v2.x, cb0[4].w
    r0.w = ((v2.xxxx)*(source[4].wwww)).w;
    // 15: mul r1.x, cb0[4].x, cb0[4].y
    r1.x = ((source[4].xxxx)*(source[4].yyyy)).x;
    // 16: mad r2.x, r1.x, cb0[4].z, r0.w
    r2.x = ((r1.xxxx)*(source[4].zzzz)+(r0.wwww)).x;
    // 17: mul r0.w, v2.y, cb0[5].x
    r0.w = ((v2.yyyy)*(source[5].xxxx)).w;
    // 18: mad r2.y, r1.x, cb0[5].y, r0.w
    r2.y = ((r1.xxxx)*(source[5].yyyy)+(r0.wwww)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 20: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 21: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 22: mad r0.xyz, -r1.xyzx, r0.xyzx, r0.wwww
    r0.xyz = ((-(r1.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 23: mad r0.xyz, cb0[6].wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((source[6].wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 24: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 25: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 26: mul r0.xyz, r0.xyzx, cb0[7].xxxx
    r0.xyz = ((r0.xyzx)*(source[7].xxxx)).xyz;
    // 27: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 28: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 29: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 30: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 32: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 33: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 34: source device depth mapped to centimetre view depth; reconstruction at 36.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 36-39: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 40: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 41: add r1.x, -cb0[7].w, l(1.000000)
    r1.x = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 42: max r1.x, -r1.x, l(0.001000)
    r1.x = (max(-(r1.xxxx),float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 43: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 44: add r1.x, -v4.x, l(1.000000)
    r1.x = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: mul_sat r1.x, r1.x, cb0[7].y
    r1.x = (saturate((r1.xxxx)*(source[7].yyyy))).x;
    // 47: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 48: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 49: mul r1.y, r1.y, cb0[7].z
    r1.y = ((r1.yyyy)*(source[7].zzzz)).y;
    // 50: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 51: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 52: mul_sat r0.w, r0.w, v3.w
    r0.w = (saturate((r0.wwww)*(v3.wwww))).w;
    // 53: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 54: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 55: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 56: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_k_pa_magiccircle_01_01_tr: a9c1396bf55a9f48b8a345a3b817a99a; selected map b0c0df1af7258b7a1abed92f77751c3f2f5c4aadb0808807f8f0a0fb0ee922eb.
float4 ALTVNative138(ALTV_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[10u];
    source[2] = ALTVNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[8u].zzzz,g_ALTVSourceMaterialParameters[8u].wwww,1u);
    source[5] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[4u].wwww,g_ALTVSourceMaterialParameters[5u].xxxx,1u);
    source[6] = ALTVNativeAppend(cos(((g_ALTVSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ALTVNativeAppend(sin(((g_ALTVSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ALTVSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[9] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[10] = g_ALTVSourceMaterialParameters[9u];
    source[11].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[11].y = (cos(((g_ALTVSourceMaterialParameters[5u].wwww*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[11].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialTime.xxxx).x;
    source[12].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[13].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].wwww)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[13].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[13].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[14].y = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[14].w = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[15].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[15].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[16].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[16].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[16].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[16].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[17].x = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[17].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[17].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[17].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[18].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[18].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[18].z = ((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].w = (sin((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[19].y = (cos((g_ALTVSourceMaterialParameters[1u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[19].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[19].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[20].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[20].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[20].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[20].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[21].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[21].y = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[21].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[16].xyxx
    r0.xy = ((v2.xyxx)*(source[16].xyxx)).xy;
    // 2: mad r1.x, cb0[11].w, cb0[15].w, r0.x
    r1.x = ((source[11].wwww)*(source[15].wwww)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[11].w, cb0[16].z, r0.y
    r1.y = ((source[11].wwww)*(source[16].zzzz)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r0.x = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 5: mul r0.y, cb0[11].w, cb0[13].z
    r0.y = ((source[11].wwww)*(source[13].zzzz)).y;
    // 6: mad r1.x, cb0[13].w, v2.x, r0.y
    r1.x = ((source[13].wwww)*(v2.xxxx)+(r0.yyyy)).x;
    // 7: mul r0.y, v2.y, cb0[14].x
    r0.y = ((v2.yyyy)*(source[14].xxxx)).y;
    // 8: mad r1.y, cb0[11].w, cb0[14].y, r0.y
    r1.y = ((source[11].wwww)*(source[14].yyyy)+(r0.yyyy)).y;
    // 9: add r0.yz, r1.xxyx, cb0[4].xxyx
    r0.yz = ((r1.xxyx)+(source[4].xxyx)).yz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t0.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 11: add r0.z, v4.z, cb0[15].x
    r0.z = ((v4.zzzz)+(source[15].xxxx)).z;
    // 12: mad r0.yz, r0.yyyy, r0.zzzz, cb0[5].xxyx
    r0.yz = ((r0.yyyy)*(r0.zzzz)+(source[5].xxyx)).yz;
    // 13: add r1.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 14: dp2 r2.x, cb0[2].xyxx, r1.xyxx
    r2.x = (dot((source[2].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 15: dp2 r2.y, cb0[3].xyxx, r1.xyxx
    r2.y = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 16: mad r1.zw, cb0[12].zzzw, v4.xxxx, r2.xxxy
    r1.zw = ((source[12].zzzw)*(v4.xxxx)+(r2.xxxy)).zw;
    // 17: add r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r1.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 18: mul r1.zw, r1.zzzw, cb0[12].xxxy
    r1.zw = ((r1.zzzw)*(source[12].xxxy)).zw;
    // 19: mad r2.x, cb0[11].w, cb0[11].z, r1.z
    r2.x = ((source[11].wwww)*(source[11].zzzz)+(r1.zzzz)).x;
    // 20: mad r2.y, cb0[11].w, cb0[13].y, r1.w
    r2.y = ((source[11].wwww)*(source[13].yyyy)+(r1.wwww)).y;
    // 21: add r0.yz, r0.yyzy, r2.xxyx
    r0.yz = ((r0.yyzy)+(r2.xxyx)).yz;
    // 22: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 23: dp2 r2.x, cb0[6].xyxx, r0.yzyy
    r2.x = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 24: dp2 r2.y, cb0[7].xyxx, r0.yzyy
    r2.y = (dot((source[7].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 25: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 26: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(-1.000000)
    r0.y = (ALTVNativeSample0((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yxzw).y;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 29: mul r0.z, r0.z, cb0[16].w
    r0.z = ((r0.zzzz)*(source[16].wwww)).z;
    // 30: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 31: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 32: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 33: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 34: mad r0.x, r0.x, cb0[17].y, r0.z
    r0.x = ((r0.xxxx)*(source[17].yyyy)+(r0.zzzz)).x;
    // 35: dp2 r2.x, cb0[8].xyxx, r1.xyxx
    r2.x = (dot((source[8].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 36: dp2 r2.y, cb0[9].xyxx, r1.xyxx
    r2.y = (dot((source[9].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 37: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 38: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 39: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 40: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 41: mul r0.z, r0.z, cb0[20].w
    r0.z = ((r0.zzzz)*(source[20].wwww)).z;
    // 42: max r0.z, r0.z, cb0[21].y
    r0.z = (max(r0.zzzz,source[21].yyyy)).z;
    // 43: min r0.z, r0.z, cb0[21].x
    r0.z = (min(r0.zzzz,source[21].xxxx)).z;
    // 44: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 45: mul r0.w, r1.x, cb0[17].w
    r0.w = ((r1.xxxx)*(source[17].wwww)).w;
    // 46: mul r1.x, r1.y, cb0[18].x
    r1.x = ((r1.yyyy)*(source[18].xxxx)).x;
    // 47: mad r1.x, cb0[11].w, cb0[19].z, r1.x
    r1.x = ((source[11].wwww)*(source[19].zzzz)+(r1.xxxx)).x;
    // 48: add r1.y, r1.x, cb0[20].x
    r1.y = ((r1.xxxx)+(source[20].xxxx)).y;
    // 49: mad r0.w, cb0[11].w, cb0[17].z, r0.w
    r0.w = ((source[11].wwww)*(source[17].zzzz)+(r0.wwww)).w;
    // 50: add r1.x, r0.w, cb0[19].w
    r1.x = ((r0.wwww)+(source[19].wwww)).x;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 52: add r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 53: add r1.x, v4.y, l(-1.000000)
    r1.x = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 54: add_sat r0.w, r0.w, -r1.x
    r0.w = (saturate((r0.wwww)+(-(r1.xxxx)))).w;
    // 55: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 56: mul r1.x, r1.x, cb0[20].z
    r1.x = ((r1.xxxx)*(source[20].zzzz)).x;
    // 57: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 58: mul_sat r1.x, r1.x, cb0[20].y
    r1.x = (saturate((r1.xxxx)*(source[20].yyyy))).x;
    // 59: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 60: mul r0.w, r0.w, cb0[20].y
    r0.w = ((r0.wwww)*(source[20].yyyy)).w;
    // 61: movc r1.x, r1.y, l(-0.000000), -r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r1.xxxx))).x;
    // 62: mov_sat r1.y, r0.w
    r1.y = (saturate(r0.wwww)).y;
    // 63: mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // 64: mul r0.y, r0.y, cb0[21].z
    r0.y = ((r0.yyyy)*(source[21].zzzz)).y;
    // 65: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 66: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 67: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 68: add r0.y, r1.x, r1.y
    r0.y = ((r1.xxxx)+(r1.yyyy)).y;
    // 69: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 70: mad r0.xyz, r0.yyyy, cb0[10].xyzx, r0.xxxx
    r0.xyz = ((r0.yyyy)*(source[10].xyzx)+(r0.xxxx)).xyz;
    // 71: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_k_pa_magiccircle_01_02_ad: db552fb0d1a3de4db8dd2fada414db6e; selected map 59864f7bafcd8798512469458498a478048e1cb362d8a91192eef277800107e7.
float4 ALTVNative139(ALTV_NATIVE_INPUT input)
{
    float4 source[19]; [unroll] for (uint i=0u; i<19u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[11u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[8u].wwww,g_ALTVSourceMaterialParameters[9u].xxxx,1u);
    source[3] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[5u].xxxx,g_ALTVSourceMaterialParameters[5u].yyyy,1u);
    source[4] = ALTVNativeAppend(cos(((g_ALTVSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin(((g_ALTVSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ALTVSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[7] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[8] = g_ALTVSourceMaterialParameters[10u];
    source[9].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].y = (cos(((g_ALTVSourceMaterialParameters[6u].xxxx*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialTime.xxxx).x;
    source[10].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[11].x = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[13].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[13].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[14].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[14].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[14].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[14].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[15].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[15].w = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[16].x = (cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[16].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[16].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[16].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[17].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[17].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[17].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[17].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[18].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[18].y = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[18].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
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
    // 31: lt r1.w, r0.z, l(0.000001)
    r1.w = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 32: movc r1.y, r1.w, l(0), r0.w
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 33: mul r2.xy, r1.xyxx, cb0[10].xyxx
    r2.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 34: mad r3.x, cb0[9].w, cb0[9].z, r2.x
    r3.x = ((source[9].wwww)*(source[9].zzzz)+(r2.xxxx)).x;
    // 35: mad r3.y, cb0[9].w, cb0[10].z, r2.y
    r3.y = ((source[9].wwww)*(source[10].zzzz)+(r2.yyyy)).y;
    // 36: mul r2.x, v4.x, cb0[10].w
    r2.x = ((v4.xxxx)*(source[10].wwww)).x;
    // 37: mul r2.y, v4.x, cb0[11].x
    r2.y = ((v4.xxxx)*(source[11].xxxx)).y;
    // 38: mad r2.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r2.xyxx)).xy;
    // 39: rsq r0.w, r0.z
    r0.w = (rsqrt(r0.zzzz)).w;
    // 40: mad r0.z, -r0.z, l(2.000000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 41: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 42: mul r0.z, r0.z, cb0[17].w
    r0.z = ((r0.zzzz)*(source[17].wwww)).z;
    // 43: max r0.z, r0.z, cb0[18].y
    r0.z = (max(r0.zzzz,source[18].yyyy)).z;
    // 44: min r0.z, r0.z, cb0[18].x
    r0.z = (min(r0.zzzz,source[18].xxxx)).z;
    // 45: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 46: movc r1.z, r1.w, l(0), r0.w
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 47: mul r1.xy, r1.xzxx, cb0[11].zwzz
    r1.xy = ((r1.xzxx)*(source[11].zwzz)).xy;
    // 48: mad r3.x, cb0[9].w, cb0[11].y, r1.x
    r3.x = ((source[9].wwww)*(source[11].yyyy)+(r1.xxxx)).x;
    // 49: mad r3.y, cb0[9].w, cb0[12].x, r1.y
    r3.y = ((source[9].wwww)*(source[12].xxxx)+(r1.yyyy)).y;
    // 50: add r1.xy, r3.xyxx, cb0[2].xyxx
    r1.xy = ((r3.xyxx)+(source[2].xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 52: add r1.x, v4.z, cb0[12].w
    r1.x = ((v4.zzzz)+(source[12].wwww)).x;
    // 53: mad r1.xy, r0.wwww, r1.xxxx, cb0[3].xyxx
    r1.xy = ((r0.wwww)*(r1.xxxx)+(source[3].xyxx)).xy;
    // 54: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 55: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 56: dp2 r2.x, cb0[4].xyxx, r1.xyxx
    r2.x = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 57: dp2 r2.y, cb0[5].xyxx, r1.xyxx
    r2.y = (dot((source[5].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 58: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 59: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (ALTVNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 60: mul r1.x, v2.x, cb0[13].w
    r1.x = ((v2.xxxx)*(source[13].wwww)).x;
    // 61: mad r1.x, cb0[9].w, cb0[13].z, r1.x
    r1.x = ((source[9].wwww)*(source[13].zzzz)+(r1.xxxx)).x;
    // 62: mul r1.z, v2.y, cb0[14].x
    r1.z = ((v2.yyyy)*(source[14].xxxx)).z;
    // 63: mad r1.y, cb0[9].w, cb0[14].y, r1.z
    r1.y = ((source[9].wwww)*(source[14].yyyy)+(r1.zzzz)).y;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.x = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 65: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 66: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 67: mul r1.y, r1.y, cb0[14].z
    r1.y = ((r1.yyyy)*(source[14].zzzz)).y;
    // 68: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 69: mul r1.y, r1.y, cb0[14].w
    r1.y = ((r1.yyyy)*(source[14].wwww)).y;
    // 70: lt r1.z, |r1.x|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 71: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 72: mad r1.x, r1.x, cb0[15].x, r1.y
    r1.x = ((r1.xxxx)*(source[15].xxxx)+(r1.yyyy)).x;
    // 73: dp2 r2.x, cb0[6].xyxx, r0.xyxx
    r2.x = (dot((source[6].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 74: dp2 r2.y, cb0[7].xyxx, r0.xyxx
    r2.y = (dot((source[7].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 75: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 76: mul r0.xy, r0.xyxx, cb0[15].zwzz
    r0.xy = ((r0.xyxx)*(source[15].zwzz)).xy;
    // 77: mad r2.x, cb0[9].w, cb0[15].y, r0.x
    r2.x = ((source[9].wwww)*(source[15].yyyy)+(r0.xxxx)).x;
    // 78: mad r2.y, cb0[9].w, cb0[16].y, r0.y
    r2.y = ((source[9].wwww)*(source[16].yyyy)+(r0.yyyy)).y;
    // 79: add r0.xy, r2.xyxx, cb0[16].zwzz
    r0.xy = ((r2.xyxx)+(source[16].zwzz)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (ALTVNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 81: add r0.x, r0.x, l(0.100000)
    r0.x = ((r0.xxxx)+(float4(0.100000,0.100000,0.100000,0.100000))).x;
    // 82: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 83: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 84: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 85: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 86: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 87: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 88: add_sat r0.x, -r0.y, r0.x
    r0.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 89: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 90: mul r0.y, r0.y, cb0[17].z
    r0.y = ((r0.yyyy)*(source[17].zzzz)).y;
    // 91: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 92: mul_sat r0.y, r0.y, cb0[17].y
    r0.y = (saturate((r0.yyyy)*(source[17].yyyy))).y;
    // 93: lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 94: mul r0.x, r0.x, cb0[17].y
    r0.x = ((r0.xxxx)*(source[17].yyyy)).x;
    // 95: movc r0.y, r1.y, l(-0.000000), -r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(-0.000000,-0.000000,-0.000000,-0.000000)) : (-(r0.yyyy))).y;
    // 96: mov_sat r1.y, r0.x
    r1.y = (saturate(r0.xxxx)).y;
    // 97: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 98: mul r0.x, r0.x, cb0[18].z
    r0.x = ((r0.xxxx)*(source[18].zzzz)).x;
    // 99: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 100: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 101: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 102: add r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)+(r1.yyyy)).y;
    // 103: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 104: mad r0.yzw, r0.yyyy, cb0[8].xxyz, r1.xxxx
    r0.yzw = ((r0.yyyy)*(source[8].xxyz)+(r1.xxxx)).yzw;
    // 105: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 106: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 107: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 108: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_me_swp_aura_01_tr: 0496e83dafb2b1478b28a6eedfafc546; selected map b3c4b82ca02b45ed0e815396912ecdf1d4109e3760e07e0756ce5b65399e481e.
float4 ALTVNative140(ALTV_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[17u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[4].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[4].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[5].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].xxxx)).x;
    source[5].z = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[6].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[6].w = ((g_ALTVSourceMaterialParameters[6u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[7].x = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[8].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[9].x = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[9].w = ((g_ALTVSourceMaterialParameters[8u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].x = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[10].y = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[10].w = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[11].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[12].x = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[12].z = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[12].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].yyyy)).x;
    source[13].x = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[13].z = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[13].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[14].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[14].y = ((g_ALTVSourceMaterialParameters[7u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[14].z = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[15].x = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[15].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[16].x = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[16].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].wwww)).x;
    source[16].z = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[17].x = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[17].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[17].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[17].w = ((g_ALTVSourceMaterialParameters[7u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[18].x = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[18].y = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[18].z = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[18].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[19].x = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[19].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[19].z = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[19].w = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[20].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[20].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[20].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[20].w = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[21].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[21].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[21].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[21].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
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
    // 35: mad r0.w, cb0[3].x, cb0[19].x, cb0[19].y
    r0.w = ((source[3].xxxx)*(source[19].xxxx)+(source[19].yyyy)).w;
    // 36: mad r0.w, v4.y, cb0[18].w, r0.w
    r0.w = ((v4.yyyy)*(source[18].wwww)+(r0.wwww)).w;
    // 37: mad r1.x, cb0[3].x, cb0[19].w, cb0[20].x
    r1.x = ((source[3].xxxx)*(source[19].wwww)+(source[20].xxxx)).x;
    // 38: mad r1.x, cb0[20].y, v4.x, r1.x
    r1.x = ((source[20].yyyy)*(v4.xxxx)+(r1.xxxx)).x;
    // 39: mad r2.x, cb0[19].z, r0.w, r1.x
    r2.x = ((source[19].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 40: mad r2.y, r1.x, cb0[20].z, r0.w
    r2.y = ((r1.xxxx)*(source[20].zzzz)+(r0.wwww)).y;
    // 41: mad r1.xy, cb0[20].wwww, r0.xyxx, r2.xyxx
    r1.xy = ((source[20].wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s4, l(0.000000)
    r0.w = (ALTVNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 44: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 45: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 46: mul r0.w, cb0[3].w, cb0[21].x
    r0.w = ((source[3].wwww)*(source[21].xxxx)).w;
    // 47: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 48: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 49: mul_sat r0.z, r0.z, cb0[21].y
    r0.z = (saturate((r0.zzzz)*(source[21].yyyy))).z;
    // 50: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 51: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 52: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 53: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 54: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 55: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 56: mul r1.x, r1.x, cb0[21].z
    r1.x = ((r1.xxxx)*(source[21].zzzz)).x;
    // 57: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 58: mul_sat r1.x, r1.x, cb0[21].w
    r1.x = (saturate((r1.xxxx)*(source[21].wwww))).x;
    // 59: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 60: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 61: movc o0.w, r0.w, l(0), r0.z
    output.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).w;
    // 62: mad r0.z, cb0[3].x, cb0[4].y, cb0[4].z
    r0.z = ((source[3].xxxx)*(source[4].yyyy)+(source[4].zzzz)).z;
    // 63: mad r0.z, v4.y, cb0[4].x, r0.z
    r0.z = ((v4.yyyy)*(source[4].xxxx)+(r0.zzzz)).z;
    // 64: add r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)+(source[5].yyyy)).z;
    // 65: mad r0.w, cb0[3].x, cb0[6].x, cb0[6].y
    r0.w = ((source[3].xxxx)*(source[6].xxxx)+(source[6].yyyy)).w;
    // 66: mad r0.w, cb0[6].z, v4.x, r0.w
    r0.w = ((source[6].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 67: add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // 68: mad r1.x, cb0[5].z, r0.z, r0.w
    r1.x = ((source[5].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 69: mad r1.y, r0.w, cb0[7].x, r0.z
    r1.y = ((r0.wwww)*(source[7].xxxx)+(r0.zzzz)).y;
    // 70: mad r0.xy, cb0[10].zzzz, r0.xyxx, r1.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(r1.xyxx)).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 72: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 74: mad r0.xyz, cb0[10].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[10].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 75: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 76: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 77: mul r0.xyz, r0.xyzx, cb0[11].xxxx
    r0.xyz = ((r0.xyzx)*(source[11].xxxx)).xyz;
    // 78: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 79: mad r0.xyz, cb0[11].yyyy, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((source[11].yyyy)*(r0.xyzx)+(source[11].zzzz)).xyz;
    // 80: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_me_ringrainbow_03_2_ts_tr: f922cf5112e35a4596b5f59fac591eac; selected map a3caae346cc3791259131ae7a5f56eb6706eec765453b041c0def754633db0d4.
float4 ALTVNative141(ALTV_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = g_ALTVSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[5].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].yyyy)).x;
    source[7].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
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
    r0.zw = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.xzyw, s2, l(0.000000)
    r1.y = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 22: mul r1.z, r0.w, cb0[10].x
    r1.z = ((r0.wwww)*(source[10].xxxx)).z;
    // 23: mad r2.w, r1.x, cb0[10].y, r1.z
    r2.w = ((r1.xxxx)*(source[10].yyyy)+(r1.zzzz)).w;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t2.xyzw, s3, l(0.000000)
    r1.z = (ALTVNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
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
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

// fx_y_me_ringrainbow_03_1_ts_tr: f922cf5112e35a4596b5f59fac591eac; selected map a3caae346cc3791259131ae7a5f56eb6706eec765453b041c0def754633db0d4.
float4 ALTVNative142(ALTV_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = g_ALTVSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[5].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].yyyy)).x;
    source[7].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
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
    r0.zw = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.xzyw, s2, l(0.000000)
    r1.y = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 22: mul r1.z, r0.w, cb0[10].x
    r1.z = ((r0.wwww)*(source[10].xxxx)).z;
    // 23: mad r2.w, r1.x, cb0[10].y, r1.z
    r2.w = ((r1.xxxx)*(source[10].yyyy)+(r1.zzzz)).w;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t2.xyzw, s3, l(0.000000)
    r1.z = (ALTVNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
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
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

// fx_m_me_ringrainbow_03_1_ts_tr: f922cf5112e35a4596b5f59fac591eac; selected map a3caae346cc3791259131ae7a5f56eb6706eec765453b041c0def754633db0d4.
float4 ALTVNative143(ALTV_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = g_ALTVSourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[5].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].yyyy)).x;
    source[7].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[9].w = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
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
    r0.zw = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.xzyw, s2, l(0.000000)
    r1.y = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xzyw).y;
    // 22: mul r1.z, r0.w, cb0[10].x
    r1.z = ((r0.wwww)*(source[10].xxxx)).z;
    // 23: mad r2.w, r1.x, cb0[10].y, r1.z
    r2.w = ((r1.xxxx)*(source[10].yyyy)+(r1.zzzz)).w;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t2.xyzw, s3, l(0.000000)
    r1.z = (ALTVNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
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
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

// fx_l_me_shine_02_2_ad: 220a27d436d0c149ae50a935ac278157; selected map 0b5b336819872cbee7f1bd00cd0d4982adbc8147ff99bf8739fa91f66a2b9a4f.
float4 ALTVNative144(ALTV_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[6u];
    source[3] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5] = g_ALTVSourceMaterialParameters[5u];
    source[6].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[6].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[7].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[9].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[10].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[10].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[11].x = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[11].y = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx))).x;
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
    r1.xyz = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 58: mad r2.x, cb0[6].y, cb0[7].w, r0.x
    r2.x = ((source[6].yyyy)*(source[7].wwww)+(r0.xxxx)).x;
    // 59: mad r2.y, cb0[6].y, cb0[8].z, r0.y
    r2.y = ((source[6].yyyy)*(source[8].zzzz)+(r0.yyyy)).y;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r2.xyxx, t2.xywz, s2, l(0.000000)
    r0.xyw = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
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

// fx_j_me_localcrack_01_07_tr: 8b228c7b319b544781cca408746673a2; selected map cb7fea6335f99773642abba576c9c60a2c351f25a720c8f239556eb64c8886fd.
float4 ALTVNative145(ALTV_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[7u];
    source[3] = ALTVNativeAppend(ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(0.300000012, 0.0, 0.0, 0.0))),1u);
    source[4] = g_ALTVSourceMaterialParameters[6u];
    source[5] = input.dynamicParameter;
    source[6] = g_ALTVSourceMaterialParameters[3u];
    source[7] = g_ALTVSourceMaterialParameters[5u];
    source[8].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[8].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[9].x = (ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[9].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[10].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[10].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 3: mul r0.zw, v4.xxxy, cb0[8].xxxy
    r0.zw = ((v4.xxxy)*(source[8].xxxy)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = (ALTVNativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
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
    r2.xyw = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
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
    r3.xyz = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
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

// fx_m_me_swp_master_01_008_ts_dt_ds_tr: 77b11abeed5c9c45b366e622f553fa69; selected map 68dbd33e02fef3d0fbd9d9280013ef39b4cb162cbf5095bdb5475f7bf7ecf26e.
float4 ALTVNative146(ALTV_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[9u];
    source[3] = g_ALTVSourceMaterialParameters[7u];
    source[4] = input.dynamicParameter;
    source[5] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[3u].wwww,g_ALTVSourceMaterialParameters[4u].xxxx,1u);
    source[6].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[6].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[6].w = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[7].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[8].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[1u].zzzz)).x;
    source[8].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[9].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[10].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[10].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[11].x = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[4u].yyyy)).x;
    source[11].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[12].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
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
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[12].y, l(1.000000)
    r0.y = ((-(source[12].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 16: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 17: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[12].z
    r0.z = ((r0.zzzz)*(source[12].zzzz)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 21: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 22: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 24: mad r0.z, cb0[6].y, cb0[10].w, cb0[11].x
    r0.z = ((source[6].yyyy)*(source[10].wwww)+(source[11].xxxx)).z;
    // 25: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 26: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 27: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 28: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 29: mul r0.zw, v4.xxxy, cb0[7].zzzw
    r0.zw = ((v4.xxxy)*(source[7].zzzw)).zw;
    // 30: mul r1.x, cb0[6].x, cb0[6].y
    r1.x = ((source[6].xxxx)*(source[6].yyyy)).x;
    // 31: mad r2.x, r1.x, cb0[7].y, r0.z
    r2.x = ((r1.xxxx)*(source[7].yyyy)+(r0.zzzz)).x;
    // 32: mad r2.y, r1.x, cb0[8].x, r0.w
    r2.y = ((r1.xxxx)*(source[8].xxxx)+(r0.wwww)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s1, l(0.000000)
    r0.zw = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 34: mad r0.zw, cb0[8].zzzz, r0.zzzw, v4.xxxy
    r0.zw = ((source[8].zzzz)*(r0.zzzw)+(v4.xxxy)).zw;
    // 35: add r1.yz, r0.zzwz, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.zzwz)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 36: dp2 r1.w, r3.yxyy, r1.yzyy
    r1.w = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).w;
    // 37: dp2 r1.y, r3.zyzz, r1.yzyy
    r1.y = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).y;
    // 38: mul r2.z, r1.y, cb0[5].y
    r2.z = ((r1.yyyy)*(source[5].yyyy)).z;
    // 39: mad r2.x, r1.w, cb0[5].x, r0.y
    r2.x = ((r1.wwww)*(source[5].xxxx)+(r0.yyyy)).x;
    // 40: add r1.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.yzyy, t2.yxzw, s4, l(0.000000)
    r0.y = (ALTVNativeSample3((r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 42: mul r1.y, r0.z, cb0[9].w
    r1.y = ((r0.zzzz)*(source[9].wwww)).y;
    // 43: mad r2.x, r1.x, cb0[9].z, r1.y
    r2.x = ((r1.xxxx)*(source[9].zzzz)+(r1.yyyy)).x;
    // 44: mul r1.y, r0.w, cb0[10].x
    r1.y = ((r0.wwww)*(source[10].xxxx)).y;
    // 45: mad r2.y, r1.x, cb0[10].y, r1.y
    r2.y = ((r1.xxxx)*(source[10].yyyy)+(r1.yyyy)).y;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s3, l(0.000000)
    r1.y = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 47: add r1.z, -cb0[4].x, l(1.000000)
    r1.z = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 48: mad r0.y, r1.y, r0.y, -r1.z
    r0.y = ((r1.yyyy)*(r0.yyyy)+(-(r1.zzzz))).y;
    // 49: mul_sat r0.y, r0.y, cb0[11].w
    r0.y = (saturate((r0.yyyy)*(source[11].wwww))).y;
    // 50: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 51: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 52: mul r1.y, r1.y, cb0[12].x
    r1.y = ((r1.yyyy)*(source[12].xxxx)).y;
    // 53: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 54: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 55: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 56: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 57: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 58: mul r0.x, r0.z, cb0[6].w
    r0.x = ((r0.zzzz)*(source[6].wwww)).x;
    // 59: mad r0.x, r1.x, cb0[6].z, r0.x
    r0.x = ((r1.xxxx)*(source[6].zzzz)+(r0.xxxx)).x;
    // 60: mul r0.z, r1.x, cb0[8].w
    r0.z = ((r1.xxxx)*(source[8].wwww)).z;
    // 61: mad r0.y, cb0[7].x, r0.w, r0.z
    r0.y = ((source[7].xxxx)*(r0.wwww)+(r0.zzzz)).y;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 63: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 64: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 65: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
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
    // 72: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 73: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_pa_swp_horizenshine_01_ad: 990edec17089b24da08ac0fc2ec137b5; selected map d4db6c10c1a1baf57d0217874c550c60ae36bc76f5e8e4fa0b4720bdbf7ce772.
float4 ALTVNative147(ALTV_NATIVE_INPUT input)
{
    float4 source[21]; [unroll] for (uint i=0u; i<21u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[16u];
    source[2].x = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[2].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[2].z = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[2].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[3].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].xxxx)).x;
    source[3].w = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[4].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[4].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].x = ((g_ALTVSourceMaterialParameters[6u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[6].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].yyyy)).x;
    source[6].w = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[7].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[6u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[8].z = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[9].x = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[9].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[9].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[9].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[10].x = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[10].z = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[10].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].wwww)).x;
    source[11].x = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[12].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].y = ((g_ALTVSourceMaterialParameters[6u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[12].z = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[13].z = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[13].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[14].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].yyyy)).x;
    source[14].z = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[14].w = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[15].x = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[15].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[15].z = ((g_ALTVSourceMaterialParameters[7u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[15].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[16].y = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[16].z = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[17].x = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[17].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[17].z = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[17].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].wwww)).x;
    source[18].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[18].y = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[18].z = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[18].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[19].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[19].y = ((g_ALTVSourceMaterialParameters[7u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[19].z = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[19].w = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[20].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[20].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
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
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s1, cb0[2].x
    r0.zw = (ALTVNativeSample1((r2.xyxx).xy, (source[2].xxxx).x, true).zwxy).zw;
    // 45: mul r0.zw, r0.zzzw, cb0[8].zzzz
    r0.zw = ((r0.zzzw)*(source[8].zzzz)).zw;
    // 46: mul r0.zw, r0.zzzw, v4.zzzz
    r0.zw = ((r0.zzzw)*(v4.zzzz)).zw;
    // 47: mad r1.xy, cb0[12].wwww, r0.zwzz, r1.xyxx
    r1.xy = ((source[12].wwww)*(r0.zwzz)+(r1.xyxx)).xy;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, cb0[2].x
    r1.x = (ALTVNativeSample2((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).x;
    // 49: mul r1.x, r1.x, cb0[13].x
    r1.x = ((r1.xxxx)*(source[13].xxxx)).x;
    // 50: mad r1.y, v4.x, cb0[13].z, cb0[13].w
    r1.y = ((v4.xxxx)*(source[13].zzzz)+(source[13].wwww)).y;
    // 51: add r1.y, r1.y, cb0[14].y
    r1.y = ((r1.yyyy)+(source[14].yyyy)).y;
    // 52: mad r1.y, v2.y, cb0[13].y, r1.y
    r1.y = ((v2.yyyy)*(source[13].yyyy)+(r1.yyyy)).y;
    // 53: mad r1.z, v4.x, cb0[15].x, cb0[15].y
    r1.z = ((v4.xxxx)*(source[15].xxxx)+(source[15].yyyy)).z;
    // 54: add r1.z, r1.z, cb0[15].z
    r1.z = ((r1.zzzz)+(source[15].zzzz)).z;
    // 55: mad r1.z, cb0[15].w, v2.x, r1.z
    r1.z = ((source[15].wwww)*(v2.xxxx)+(r1.zzzz)).z;
    // 56: mad r2.x, cb0[14].z, r1.y, r1.z
    r2.x = ((source[14].zzzz)*(r1.yyyy)+(r1.zzzz)).x;
    // 57: mad r2.y, r1.z, cb0[16].x, r1.y
    r2.y = ((r1.zzzz)*(source[16].xxxx)+(r1.yyyy)).y;
    // 58: mad r1.yz, cb0[16].yyyy, r0.zzwz, r2.xxyx
    r1.yz = ((source[16].yyyy)*(r0.zzwz)+(r2.xxyx)).yz;
    // 59: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t2.yxzw, s3, cb0[2].x
    r1.y = (ALTVNativeSample3((r1.yzyy).xy, (source[2].xxxx).x, true).yxzw).y;
    // 60: mul r1.y, r1.y, cb0[16].z
    r1.y = ((r1.yyyy)*(source[16].zzzz)).y;
    // 61: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 62: mad r1.y, v4.x, cb0[17].x, cb0[17].y
    r1.y = ((v4.xxxx)*(source[17].xxxx)+(source[17].yyyy)).y;
    // 63: mad r1.y, v2.y, cb0[16].w, r1.y
    r1.y = ((v2.yyyy)*(source[16].wwww)+(r1.yyyy)).y;
    // 64: add r1.y, r1.y, cb0[17].w
    r1.y = ((r1.yyyy)+(source[17].wwww)).y;
    // 65: mad r1.z, v4.x, cb0[18].z, cb0[18].w
    r1.z = ((v4.xxxx)*(source[18].zzzz)+(source[18].wwww)).z;
    // 66: mad r1.z, cb0[19].x, v2.x, r1.z
    r1.z = ((source[19].xxxx)*(v2.xxxx)+(r1.zzzz)).z;
    // 67: add r1.z, r1.z, cb0[19].y
    r1.z = ((r1.zzzz)+(source[19].yyyy)).z;
    // 68: mad r2.x, cb0[18].x, r1.y, r1.z
    r2.x = ((source[18].xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // 69: mad r2.y, r1.z, cb0[19].z, r1.y
    r2.y = ((r1.zzzz)*(source[19].zzzz)+(r1.yyyy)).y;
    // 70: mad r1.yz, cb0[19].wwww, r0.zzwz, r2.xxyx
    r1.yz = ((source[19].wwww)*(r0.zzwz)+(r2.xxyx)).yz;
    // 71: sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t3.yxzw, s4, cb0[2].x
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
    // 79: mul_sat r1.x, r1.x, v3.w
    r1.x = (saturate((r1.xxxx)*(v3.wwww))).x;
    // 80: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 81: mad r1.x, v4.x, cb0[4].y, cb0[4].z
    r1.x = ((v4.xxxx)*(source[4].yyyy)+(source[4].zzzz)).x;
    // 82: mad r0.y, cb0[4].w, r0.y, r1.x
    r0.y = ((source[4].wwww)*(r0.yyyy)+(r1.xxxx)).y;
    // 83: add r0.y, r0.y, cb0[5].x
    r0.y = ((r0.yyyy)+(source[5].xxxx)).y;
    // 84: mad r1.x, v4.x, cb0[2].z, cb0[2].w
    r1.x = ((v4.xxxx)*(source[2].zzzz)+(source[2].wwww)).x;
    // 85: mad r0.x, r0.x, cb0[2].y, r1.x
    r0.x = ((r0.xxxx)*(source[2].yyyy)+(r1.xxxx)).x;
    // 86: add r0.x, r0.x, cb0[3].z
    r0.x = ((r0.xxxx)+(source[3].zzzz)).x;
    // 87: mad r1.x, cb0[3].w, r0.x, r0.y
    r1.x = ((source[3].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 88: mad r1.y, r0.y, cb0[5].y, r0.x
    r1.y = ((r0.yyyy)*(source[5].yyyy)+(r0.xxxx)).y;
    // 89: mad r0.xy, cb0[8].wwww, r0.zwzz, r1.xyxx
    r0.xy = ((source[8].wwww)*(r0.zwzz)+(r1.xyxx)).xy;
    // 90: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s0, cb0[2].x
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 91: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 93: mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 94: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 95: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 96: mul r0.xyz, r0.xyzx, cb0[9].yyyy
    r0.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 97: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 98: mul r0.xyz, r0.xyzx, cb0[9].zzzz
    r0.xyz = ((r0.xyzx)*(source[9].zzzz)).xyz;
    // 99: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 100: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_rgbsplit_01_2_ad: d880c361f16370468cfdac8d4688d393; selected map e84de71c52a818919646772cd3b0a0181fe0758eaae2cdb6c89bc69d21826863.
float4 ALTVNative148(ALTV_NATIVE_INPUT input)
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

// fx_m_pa_swp_ht_01_2_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ALTVNative149(ALTV_NATIVE_INPUT input)
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

// fx_d_pa_ring_12_ad_inst35: 2ff88b3f0d9a8c43a28b46e6f7a596b5; selected map d997538f1226f51ee7f6c69629306ef01597ba8e4708973b644d0a8f6facf118.
float4 ALTVNative150(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[2].y = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].wwww)).x;
    source[2].z = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].wwww))).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].z = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[3].w = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx))).x;
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
    // 22: div r0.yz, v7.xxyx, v7.wwww
    r0.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // 23: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // Native 24: source device depth mapped to centimetre view depth; reconstruction at 26.
    r0.y = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.yzyy).xy, 0.f).y * 100000.f;
    // Native 26-29: reconstructed view depth is supplied by the runtime adapter.
    r0.y = r0.y;
    // 30: add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // 31: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: mul r0.z, r0.z, l(100.000000)
    r0.z = ((r0.zzzz)*(float4(100.000000,100.000000,100.000000,100.000000))).z;
    // 33: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 34: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 35: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 36: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 37: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 38: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 39: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 40: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 41: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_d_me_capturergbsplit_01_01_tr: 3bfd10a04e6ff94e8cad688fec0df23a; selected map 5a1d840814a787b1856ffa420c672207581a828aba9ba64b38393a928ed68dc8.
float4 ALTVNative151(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[1u];
    source[3].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[4].x = ((g_ALTVSourceMaterialParameters[0u].yyyy*float4(-1.0, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 2: mad r0.zw, r0.xxxy, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.xxxy)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 3: mov r1.yz, r0.wwzw
    r1.yz = (r0.wwzw).yz;
    // 4: add r1.xw, r0.zzzw, -cb0[3].xxxx
    r1.xw = ((r0.zzzw)+(-(source[3].xxxx))).xw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r1.zwzz, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 7: add r0.xy, r0.zwzz, cb0[3].xxxx
    r0.xy = ((r0.zwzz)+(source[3].xxxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r0.xwxx, t0.yzxw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzxw).z;
    // 9: add r2.x, r2.x, r2.z
    r2.x = ((r2.xxxx)+(r2.zzzz)).x;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r0.zyzz, t0.yzxw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzxw).z;
    // 11: add r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)+(r2.xxxx)).x;
    // 12: add r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r0.xyxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 14: mov r1.yz, r0.yyxy
    r1.yz = (r0.yyxy).yz;
    // 15: add r0.x, r2.y, r2.x
    r0.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xwxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 17: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).x;
    // 20: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 21: add r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)+(r0.xxxx)).x;
    // 22: mul r1.x, r0.x, cb0[1].x
    r1.x = ((r0.xxxx)*(source[1].xxxx)).x;
    // 23: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 24: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 25: mul r0.x, r0.x, v5.z
    r0.x = ((r0.xxxx)*(v5.zzzz)).x;
    // 26: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 28: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
    // 30: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 31: mul_sat r0.y, r0.y, cb0[3].z
    r0.y = (saturate((r0.yyyy)*(source[3].zzzz))).y;
    // 32: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 33: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 35: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 36: mad r2.zw, r0.yyyy, cb0[3].wwww, r0.zzzw
    r2.zw = ((r0.yyyy)*(source[3].wwww)+(r0.zzzw)).zw;
    // 37: mad r0.zw, r0.yyyy, cb0[4].xxxx, r0.zzzw
    r0.zw = ((r0.yyyy)*(source[4].xxxx)+(r0.zzzw)).zw;
    // 38: add r2.xy, r2.zwzz, cb0[3].xxxx
    r2.xy = ((r2.zwzz)+(source[3].xxxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xwxx, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 40: mov r3.yz, r2.wwzw
    r3.yz = (r2.wwzw).yz;
    // 41: add r3.xw, r2.zzzw, -cb0[3].xxxx
    r3.xw = ((r2.zzzw)+(-(source[3].xxxx))).xw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.zyzz, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).z;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r4.x, r3.zwzz, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 45: add r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)+(r2.wwww)).w;
    // 46: add r1.w, r2.z, r1.w
    r1.w = ((r2.zzzz)+(r1.wwww)).w;
    // 47: add r1.w, r4.x, r1.w
    r1.w = ((r4.xxxx)+(r1.wwww)).w;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.xyxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).z;
    // 49: mov r3.yz, r2.yyxy
    r3.yz = (r2.yyxy).yz;
    // 50: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r3.xwxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 52: add r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)+(r2.xxxx)).w;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r3.xyxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r3.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).y;
    // 55: add r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)+(r2.xxxx)).w;
    // 56: add r1.w, r2.y, r1.w
    r1.w = ((r2.yyyy)+(r1.wwww)).w;
    // 57: mul r1.y, r1.w, cb0[1].y
    r1.y = ((r1.wwww)*(source[1].yyyy)).y;
    // 58: add r0.xy, r0.zwzz, cb0[3].xxxx
    r0.xy = ((r0.zwzz)+(source[3].xxxx)).xy;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r0.xwxx, t0.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 60: mov r2.yz, r0.wwzw
    r2.yz = (r0.wwzw).yz;
    // 61: add r2.xw, r0.zzzw, -cb0[3].xxxx
    r2.xw = ((r0.zzzw)+(-(source[3].xxxx))).xw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zyzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).z;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r2.zwzz, t0.zxyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).zxyw).x;
    // 65: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 66: add r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)+(r0.wwww)).z;
    // 67: add r0.z, r3.x, r0.z
    r0.z = ((r3.xxxx)+(r0.zzzz)).z;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.xyxx, t0.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 69: mov r2.yz, r0.yyxy
    r2.yz = (r0.yyxy).yz;
    // 70: add r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)+(r0.zzzz)).x;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xwxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 72: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).z;
    // 75: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 76: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 77: mul r1.z, r0.x, cb0[1].z
    r1.z = ((r0.xxxx)*(source[1].zzzz)).z;
    // 78: mad r0.xyz, r1.xyzx, l(0.125000, 0.125000, 0.125000, 0.000000), cb0[2].xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.125000,0.125000,0.125000,0.000000))+(source[2].xyzx)).xyz;
    // 79: mad o0.xyz, r0.xyzx, v4.wwww, v4.xyzx
    output.xyz = ((r0.xyzx)*(v4.wwww)+(v4.xyzx)).xyz;
    return output;
}

// fx_c_pa_lensflare_01_11_dt5_ad: 55883598a583ae4389d79bb0fdf1d392; selected map ae9ed651c317374306f77e3e999680cc10ae6be4f2b89b76263a55721100b6de.
float4 ALTVNative152(ALTV_NATIVE_INPUT input)
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

// fx_o_pa_spritewave_01_29_tr: 896de46482584f40beae5d174ab6a67c; selected map 656abb016151e52a25ea0326da9bfc1b2fa6381531ff0286f6b5b2a2364259da.
float4 ALTVNative153(ALTV_NATIVE_INPUT input)
{
    float4 source[23]; [unroll] for (uint i=0u; i<23u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[12u];
    source[2] = ALTVNativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[4u].zzzz,g_ALTVSourceMaterialParameters[4u].wwww,1u);
    source[5] = ALTVNativeAppend(cos(((g_ALTVSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[6] = ALTVNativeAppend(sin(((g_ALTVSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),cos(((g_ALTVSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[7] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[8] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[9] = g_ALTVSourceMaterialParameters[11u];
    source[10].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((g_ALTVSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].y = (cos(((g_ALTVSourceMaterialParameters[5u].zzzz*float4(6.28000021, 0.0, 0.0, 0.0))*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[10].w = (g_ALTVSourceMaterialTime.xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[11].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[11].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].x = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].zzzz)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[12].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[13].x = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[13].y = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[13].z = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[14].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[14].y = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[14].z = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[15].y = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[15].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[15].w = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[16].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[16].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[16].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[17].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[17].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[17].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[17].w = ((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[18].x = (sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[18].z = (cos((g_ALTVSourceMaterialParameters[2u].xxxx*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[19].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[19].y = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[19].z = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[19].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[20].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[20].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[20].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[20].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[21].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[21].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[21].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[21].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[22].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[22].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
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
    // 1: mul r0.xy, v2.xyxx, cb0[19].xyxx
    r0.xy = ((v2.xyxx)*(source[19].xyxx)).xy;
    // 2: mad r1.x, cb0[10].w, cb0[18].w, r0.x
    r1.x = ((source[10].wwww)*(source[18].wwww)+(r0.xxxx)).x;
    // 3: mad r1.y, cb0[10].w, cb0[19].z, r0.y
    r1.y = ((source[10].wwww)*(source[19].zzzz)+(r0.yyyy)).y;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r0.x = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.x = (ALTVNativeSample4((r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
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
    r0.w = (ALTVNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.w = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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
    r0.z = (ALTVNativeSample0((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
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

// fx_a_pa_gl_01_6_ad: c058e92770ba0b488812c453984a6c6c; selected map 3bfdf33f4814032d8490d912e6851095862f6b1a880a141b9d66c4d2c795070e.
float4 ALTVNative154(ALTV_NATIVE_INPUT input)
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

// fx_c_pa_zoomblur_01_tr: 3fc4c0de7f119c49b1e0478e97872fc9; selected map d759fa1ad738c7c8c48ad5775499deb1dafee1b7c91903e48fb751adeb6a9a0d.
float4 ALTVNative155(ALTV_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[1u];
    source[2].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[2].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
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
    // 10: add r2.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r2.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 11: mul r1.xy, r2.xyxx, r2.xyxx
    r1.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 12: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 16: mul r0.w, r0.w, cb0[2].x
    r0.w = ((r0.wwww)*(source[2].xxxx)).w;
    // 17: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 18: mul_sat r0.w, r0.w, cb0[2].y
    r0.w = (saturate((r0.wwww)*(source[2].yyyy))).w;
    // 19: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 20: mul r1.x, r0.w, v3.w
    r1.x = ((r0.wwww)*(v3.wwww)).x;
    // 21: mul o0.w, r1.x, cb0[0].x
    output.w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 22: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 23: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 24: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r1.xyxx, t0.xyzw, s0 (project resolved HDR SceneColor snapshot adapter)
    r3.xyz = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r1.xyxx).xy).xyzw).xyz;
    // 25: mul r1.w, v4.x, l(-0.010000)
    r1.w = ((v4.xxxx)*(float4(-0.010000,-0.010000,-0.010000,-0.010000))).w;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r1.xyxx, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 27: dp2 r2.x, r2.zwzz, r2.zwzz
    r2.x = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).x;
    // 28: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 29: mul r5.xy, r2.xxxx, r2.zwzz
    r5.xy = ((r2.xxxx)*(r2.zwzz)).xy;
    // 30: mad r2.xy, -r2.zwzz, r2.xxxx, r2.zwzz
    r2.xy = ((-(r2.zwzz))*(r2.xxxx)+(r2.zwzz)).xy;
    // 31: mad r2.xy, r2.xyxx, l(0.900000, 0.900000, 0.000000, 0.000000), r5.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.900000,0.900000,0.000000,0.000000))+(r5.xyxx)).xy;
    // 32: mul r2.xy, r1.wwww, r2.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)).xy;
    // 33: mad r2.xy, r2.xyxx, l(2.000000, -2.000000, 0.000000, 0.000000), l(-1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,-2.000000,0.000000,0.000000))+(float4(-1.000000,1.000000,0.000000,0.000000))).xy;
    // 34: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 35: mov r5.xyz, r4.xyzx
    r5.xyz = (r4.xyzx).xyz;
    // 36: mov r2.zw, r1.xxxy
    r2.zw = (r1.xxxy).zw;
    // 37: mov r1.w, l(0)
    r1.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 38: loop
    [loop] while (true) {
    // 39: ge r3.w, r1.w, l(3.000000)
    r3.w = (asfloat((uint4)((r1.wwww)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 40: breakc_nz r3.w
    if ((asuint(r3.wwww)).x != 0u) break;
    // 41: add r2.zw, r2.xxxy, r2.zzzw
    r2.zw = ((r2.xxxy)+(r2.zzzw)).zw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r6.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 43: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 44: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: endloop
    }
    // 46: mad r1.xyw, r5.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000), -r3.xyxz
    r1.xyw = ((r5.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))+(-(r3.xyxz))).xyw;
    // 47: mul r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)*(r1.xyxw)).xyw;
    // 48: mad r1.xyw, cb0[2].zzzz, r1.xyxw, r3.xyxz
    r1.xyw = ((source[2].zzzz)*(r1.xyxw)+(r3.xyxz)).xyw;
    // 49: mad r1.xyw, v3.xyxz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((v3.xyxz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 50: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_po_rgbnoise_01_01_tr: 9f4cdbbbab89f745927fe4d13bb5e5a6; selected map 55cfde56e7c9a9e99fe1223b194f3b844d6179733dd9d8b1ed3a88c6ba451164.
float4 ALTVNative156(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[3u];
    source[2].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[2].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[3].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[3].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[4].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
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
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mov r1.z, l(1.000000)
    r1.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 4: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 5: mul r0.z, r1.y, cb0[3].x
    r0.z = ((r1.yyyy)*(source[3].xxxx)).z;
    // 6: mad r0.zw, r0.zzzz, r1.xxxz, r0.xxxy
    r0.zw = ((r0.zzzz)*(r1.xxxz)+(r0.xxxy)).zw;
    // 7: sample_indexable(texture2d)(float,float,float,float) r2.z, r0.zwzz, t1.xyzw, s1 (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r0.zwzz).xy).xyzw).z;
    // 8: mul r0.zw, r1.yyyy, cb0[2].zzzw
    r0.zw = ((r1.yyyy)*(source[2].zzzw)).zw;
    // 9: mad r1.xyzw, r0.zzww, r1.xzxz, r0.xyxy
    r1.xyzw = ((r0.zzww)*(r1.xzxz)+(r0.xyxy)).xyzw;
    // Native 10: source device depth mapped to centimetre view depth; reconstruction at 20.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // 12: sample_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t1.xyzw, s1 (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r1.xyxx).xy).xyzw).x;
    // 13: sample_indexable(texture2d)(float,float,float,float) r2.y, r1.zwzz, t1.xyzw, s1 (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r1.zwzz).xy).xyzw).y;
    // 14: mul r0.yzw, r2.xxyz, v3.xxyz
    r0.yzw = ((r2.xxyz)*(v3.xxyz)).yzw;
    // 15: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 16: mad r1.xyz, -r2.xyzx, v3.xyzx, r1.xxxx
    r1.xyz = ((-(r2.xyzx))*(v3.xyzx)+(r1.xxxx)).xyz;
    // 17: mad r0.yzw, cb0[3].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[3].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 18: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 19: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // Native 20-23: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 24: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 25: add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mul r0.y, r0.y, l(100.000000)
    r0.y = ((r0.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000))).y;
    // 27: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 28: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 29: add r0.yz, v2.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v2.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 30: mul r0.yz, r0.yyzy, r0.yyzy
    r0.yz = ((r0.yyzy)*(r0.yyzy)).yz;
    // 31: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 32: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 34: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 35: mul r0.w, v4.z, cb0[3].z
    r0.w = ((v4.zzzz)*(source[3].zzzz)).w;
    // 36: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 37: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 38: mul_sat r0.z, r0.z, cb0[3].w
    r0.z = (saturate((r0.zzzz)*(source[3].wwww))).z;
    // 39: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 40: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 41: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 42: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

// fx_m_me_ddk_capturergbsplit_01_01_tr: 3bfd10a04e6ff94e8cad688fec0df23a; selected map 5a1d840814a787b1856ffa420c672207581a828aba9ba64b38393a928ed68dc8.
float4 ALTVNative157(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[1u];
    source[3].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[3].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[4].x = ((g_ALTVSourceMaterialParameters[0u].yyyy*float4(-1.0, 0.0, 0.0, 0.0))).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,0.f,input.projectionW); // native texcoord5
    float4 v7 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 2: mad r0.zw, r0.xxxy, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.xxxy)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 3: mov r1.yz, r0.wwzw
    r1.yz = (r0.wwzw).yz;
    // 4: add r1.xw, r0.zzzw, -cb0[3].xxxx
    r1.xw = ((r0.zzzw)+(-(source[3].xxxx))).xw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r1.zwzz, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 7: add r0.xy, r0.zwzz, cb0[3].xxxx
    r0.xy = ((r0.zwzz)+(source[3].xxxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r0.xwxx, t0.yzxw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzxw).z;
    // 9: add r2.x, r2.x, r2.z
    r2.x = ((r2.xxxx)+(r2.zzzz)).x;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r0.zyzz, t0.yzxw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzxw).z;
    // 11: add r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)+(r2.xxxx)).x;
    // 12: add r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r0.xyxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 14: mov r1.yz, r0.yyxy
    r1.yz = (r0.yyxy).yz;
    // 15: add r0.x, r2.y, r2.x
    r0.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xwxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 17: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).x;
    // 20: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 21: add r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)+(r0.xxxx)).x;
    // 22: mul r1.x, r0.x, cb0[1].x
    r1.x = ((r0.xxxx)*(source[1].xxxx)).x;
    // 23: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 24: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 25: mul r0.x, r0.x, v5.z
    r0.x = ((r0.xxxx)*(v5.zzzz)).x;
    // 26: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 28: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 29: mul r0.y, r0.y, cb0[3].y
    r0.y = ((r0.yyyy)*(source[3].yyyy)).y;
    // 30: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 31: mul_sat r0.y, r0.y, cb0[3].z
    r0.y = (saturate((r0.yyyy)*(source[3].zzzz))).y;
    // 32: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 33: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 35: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 36: mad r2.zw, r0.yyyy, cb0[3].wwww, r0.zzzw
    r2.zw = ((r0.yyyy)*(source[3].wwww)+(r0.zzzw)).zw;
    // 37: mad r0.zw, r0.yyyy, cb0[4].xxxx, r0.zzzw
    r0.zw = ((r0.yyyy)*(source[4].xxxx)+(r0.zzzw)).zw;
    // 38: add r2.xy, r2.zwzz, cb0[3].xxxx
    r2.xy = ((r2.zwzz)+(source[3].xxxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.xwxx, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 40: mov r3.yz, r2.wwzw
    r3.yz = (r2.wwzw).yz;
    // 41: add r3.xw, r2.zzzw, -cb0[3].xxxx
    r3.xw = ((r2.zzzw)+(-(source[3].xxxx))).xw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.zyzz, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).z;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.xzwy, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzwy).w;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r4.x, r3.zwzz, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r4.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 45: add r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)+(r2.wwww)).w;
    // 46: add r1.w, r2.z, r1.w
    r1.w = ((r2.zzzz)+(r1.wwww)).w;
    // 47: add r1.w, r4.x, r1.w
    r1.w = ((r4.xxxx)+(r1.wwww)).w;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.xyxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).z;
    // 49: mov r3.yz, r2.yyxy
    r3.yz = (r2.yyxy).yz;
    // 50: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r3.xwxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 52: add r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)+(r2.xxxx)).w;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r3.xyxx, t0.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r3.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).y;
    // 55: add r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)+(r2.xxxx)).w;
    // 56: add r1.w, r2.y, r1.w
    r1.w = ((r2.yyyy)+(r1.wwww)).w;
    // 57: mul r1.y, r1.w, cb0[1].y
    r1.y = ((r1.wwww)*(source[1].yyyy)).y;
    // 58: add r0.xy, r0.zwzz, cb0[3].xxxx
    r0.xy = ((r0.zwzz)+(source[3].xxxx)).xy;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r0.xwxx, t0.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 60: mov r2.yz, r0.wwzw
    r2.yz = (r0.wwzw).yz;
    // 61: add r2.xw, r0.zzzw, -cb0[3].xxxx
    r2.xw = ((r0.zzzw)+(-(source[3].xxxx))).xw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zyzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).z;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t0.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r2.zwzz, t0.zxyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).zxyw).x;
    // 65: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 66: add r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)+(r0.wwww)).z;
    // 67: add r0.z, r3.x, r0.z
    r0.z = ((r3.xxxx)+(r0.zzzz)).z;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r0.xyxx, t0.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 69: mov r2.yz, r0.yyxy
    r2.yz = (r0.yyxy).yz;
    // 70: add r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)+(r0.zzzz)).x;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xwxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 72: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t0.xzyw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xzyw).y;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r2.zwzz, t0.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r0.z = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).z;
    // 75: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 76: add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // 77: mul r1.z, r0.x, cb0[1].z
    r1.z = ((r0.xxxx)*(source[1].zzzz)).z;
    // 78: mad r0.xyz, r1.xyzx, l(0.125000, 0.125000, 0.125000, 0.000000), cb0[2].xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.125000,0.125000,0.125000,0.000000))+(source[2].xyzx)).xyz;
    // 79: mad o0.xyz, r0.xyzx, v4.wwww, v4.xyzx
    output.xyz = ((r0.xyzx)*(v4.wwww)+(v4.xyzx)).xyz;
    return output;
}

// fx_m_swp_cam_line_02_tr: cb00f25c542bff4e80cadb10ef536113; selected map 03b9b81443d762a972c3e6405ebd2127f5f0a2bf3d63c1b89c057bf31963bdc6.
float4 ALTVNative158(ALTV_NATIVE_INPUT input)
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

// fx_m_pp_crackbase_01_01: 7558d2a8551366429265d66471baed70; selected map b13d1829cd99b80ff1438aa7791ddbbed5158ad73699e1697878257f9e3973c0.
float4 ALTVNative159(ALTV_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[4u];
    source[2] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[1u].wwww,g_ALTVSourceMaterialParameters[2u].xxxx,1u);
    source[3] = g_ALTVSourceMaterialParameters[3u];
    source[4].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: max r1.x, |r0.z|, |r0.w|
    r1.x = (max(abs(r0.zzzz),abs(r0.wwww))).x;
    // 3: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 4: min r1.y, |r0.z|, |r0.w|
    r1.y = (min(abs(r0.zzzz),abs(r0.wwww))).y;
    // 5: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 6: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 7: mad r1.z, r1.y, l(0.020835), l(-0.085133)
    r1.z = ((r1.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 8: mad r1.z, r1.y, r1.z, l(0.180141)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 9: mad r1.z, r1.y, r1.z, l(-0.330299)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 10: mad r1.y, r1.y, r1.z, l(0.999866)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 11: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 12: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 13: lt r1.w, |r0.z|, |r0.w|
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.wwww))) * 0xffffffffu)).w;
    // 14: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 15: mad r1.x, r1.x, r1.y, r1.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // 16: lt r1.y, r0.z, -r0.z
    r1.y = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).y;
    // 17: and r1.y, r1.y, l(0xc0490fdb)
    r1.y = (asfloat(asuint(r1.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 18: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 19: min r1.y, r0.z, r0.w
    r1.y = (min(r0.zzzz,r0.wwww)).y;
    // 20: lt r1.y, r1.y, -r1.y
    r1.y = (asfloat((uint4)((r1.yyyy)<(-(r1.yyyy))) * 0xffffffffu)).y;
    // 21: max r1.z, r0.z, r0.w
    r1.z = (max(r0.zzzz,r0.wwww)).z;
    // 22: ge r1.z, r1.z, -r1.z
    r1.z = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).z;
    // 23: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 24: movc r1.x, r1.y, -r1.x, r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (-(r1.xxxx)) : (r1.xxxx)).x;
    // 25: mad r1.x, r1.x, l(0.318471), l(1.000000)
    r1.x = ((r1.xxxx)*(float4(0.318471,0.318471,0.318471,0.318471))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 26: mul r1.y, r1.x, cb0[2].x
    r1.y = ((r1.xxxx)*(source[2].xxxx)).y;
    // 27: mul r2.x, r1.y, l(0.500000)
    r2.x = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 28: dp2 r0.z, -r0.zwzz, -r0.zwzz
    r0.z = (dot((-(r0.zwzz)).xy,(-(r0.zwzz)).xy).xxxx).z;
    // 29: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 30: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 31: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 32: sqrt r1.z, r0.z
    r1.z = (sqrt(r0.zzzz)).z;
    // 33: mul r2.y, r1.z, cb0[2].y
    r2.y = ((r1.zzzz)*(source[2].yyyy)).y;
    // 34: mov r3.xz, l(0,0,0,0)
    r3.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // 35: mul r3.yw, v4.wwwz, l(0.000000, 0.050000, 0.000000, -0.500000)
    r3.yw = ((v4.wwwz)*(float4(0.000000,0.050000,0.000000,-0.500000))).yw;
    // 36: add r0.yz, r2.xxyx, r3.xxyx
    r0.yz = ((r2.xxyx)+(r3.xxyx)).yz;
    // 37: sample_l_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t0.zxyw, s0, l(-1.000000)
    r0.yz = (ALTVNativeSample0((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).zxyw).yz;
    // 38: mad r0.yz, r0.yyzy, l(0.000000, 0.600000, 0.600000, 0.000000), r1.xxzx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.600000,0.600000,0.000000))+(r1.xxzx)).yz;
    // 39: add r0.yz, r0.yyzy, r3.zzwz
    r0.yz = ((r0.yyzy)+(r3.zzwz)).yz;
    // 40: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t2.xyzw, s2, l(-1.000000)
    r1.xyz = (ALTVNativeSample2((r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s1, l(0.000000)
    r0.yzw = (ALTVNativeSample1((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 42: max r2.xyzw, |r0.yyzw|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r2.xyzw = (max(abs(r0.yyzw),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 43: log r2.xyzw, r2.xyzw
    r2.xyzw = (log2(r2.xyzw)).xyzw;
    // 44: mul r2.xyzw, r2.xyzw, cb0[4].zzzz
    r2.xyzw = ((r2.xyzw)*(source[4].zzzz)).xyzw;
    // 45: exp r2.xyzw, r2.xyzw
    r2.xyzw = (exp2(r2.xyzw)).xyzw;
    // 46: mul r2.xyzw, r2.xyzw, cb0[4].wwww
    r2.xyzw = ((r2.xyzw)*(source[4].wwww)).xyzw;
    // 47: max r1.xyzw, |r1.xxyz|, l(0.000001, 0.000001, 0.000001, 0.000001)
    r1.xyzw = (max(abs(r1.xxyz),float4(0.000001,0.000001,0.000001,0.000001))).xyzw;
    // 48: log r1.xyzw, r1.xyzw
    r1.xyzw = (log2(r1.xyzw)).xyzw;
    // 49: mul r1.xyzw, r1.xyzw, cb0[5].xxxx
    r1.xyzw = ((r1.xyzw)*(source[5].xxxx)).xyzw;
    // 50: exp r1.xyzw, r1.xyzw
    r1.xyzw = (exp2(r1.xyzw)).xyzw;
    // 51: mul r1.xyzw, r1.xyzw, cb0[5].yyyy
    r1.xyzw = ((r1.xyzw)*(source[5].yyyy)).xyzw;
    // 52: mul r1.xyzw, r1.xyzw, cb0[3].xxyz
    r1.xyzw = ((r1.xyzw)*(source[3].xxyz)).xyzw;
    // 53: mul r3.xyzw, r1.xyzw, r2.xyzw
    r3.xyzw = ((r1.xyzw)*(r2.xyzw)).xyzw;
    // 54: dp3 r0.y, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 55: mad r1.xyzw, -r2.yyzw, r1.yyzw, r0.yyyy
    r1.xyzw = ((-(r2.yyzw))*(r1.yyzw)+(r0.yyyy)).xyzw;
    // 56: add r0.y, v4.y, cb0[5].z
    r0.y = ((v4.yyyy)+(source[5].zzzz)).y;
    // 57: mad r1.xyzw, r0.yyyy, r1.xyzw, r3.xyzw
    r1.xyzw = ((r0.yyyy)*(r1.xyzw)+(r3.xyzw)).xyzw;
    // 58: mov_sat r1.x, r1.x
    r1.x = (saturate(r1.xxxx)).x;
    // 59: mad r0.yzw, r1.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r1.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 60: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 61: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 62: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 63: mul r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)*(source[5].wwww)).y;
    // 64: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 65: mul_sat r0.y, r0.y, cb0[6].x
    r0.y = (saturate((r0.yyyy)*(source[6].xxxx))).y;
    // 66: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 67: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 68: mul r0.y, r1.x, r0.y
    r0.y = ((r1.xxxx)*(r0.yyyy)).y;
    // 69: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 70: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}

// fx_n_pa_desaturation_03_tr: ac1471f2597e1e4a9009858ac936a5f8; selected map 559b1ef1a1cb014446c3e4f43c3691c7c064cee6466972372903003c55ad24a6.
float4 ALTVNative160(ALTV_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[4u];
    source[2].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[2].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[2].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[3].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[3].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[3].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
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
    // 2: max r1.x, |r0.w|, |r0.z|
    r1.x = (max(abs(r0.wwww),abs(r0.zzzz))).x;
    // 3: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 4: min r1.y, |r0.w|, |r0.z|
    r1.y = (min(abs(r0.wwww),abs(r0.zzzz))).y;
    // 5: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 6: mul r1.y, r1.x, r1.x
    r1.y = ((r1.xxxx)*(r1.xxxx)).y;
    // 7: mad r1.z, r1.y, l(0.020835), l(-0.085133)
    r1.z = ((r1.yyyy)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 8: mad r1.z, r1.y, r1.z, l(0.180141)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 9: mad r1.z, r1.y, r1.z, l(-0.330299)
    r1.z = ((r1.yyyy)*(r1.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 10: mad r1.y, r1.y, r1.z, l(0.999866)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).y;
    // 11: mul r1.z, r1.y, r1.x
    r1.z = ((r1.yyyy)*(r1.xxxx)).z;
    // 12: mad r1.z, r1.z, l(-2.000000), l(1.570796)
    r1.z = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 13: lt r1.w, |r0.w|, |r0.z|
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(abs(r0.zzzz))) * 0xffffffffu)).w;
    // 14: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 15: mad r1.x, r1.x, r1.y, r1.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // 16: lt r1.y, r0.w, -r0.w
    r1.y = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).y;
    // 17: and r1.y, r1.y, l(0xc0490fdb)
    r1.y = (asfloat(asuint(r1.yyyy) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).y;
    // 18: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 19: min r1.y, r0.w, r0.z
    r1.y = (min(r0.wwww,r0.zzzz)).y;
    // 20: lt r1.y, r1.y, -r1.y
    r1.y = (asfloat((uint4)((r1.yyyy)<(-(r1.yyyy))) * 0xffffffffu)).y;
    // 21: max r1.z, r0.w, r0.z
    r1.z = (max(r0.wwww,r0.zzzz)).z;
    // 22: ge r1.z, r1.z, -r1.z
    r1.z = (asfloat((uint4)((r1.zzzz)>=(-(r1.zzzz))) * 0xffffffffu)).z;
    // 23: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 24: movc r1.x, r1.y, -r1.x, r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (-(r1.xxxx)) : (r1.xxxx)).x;
    // 25: mad r1.x, r1.x, l(0.159155), l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: mul r1.z, cb0[2].y, cb0[3].z
    r1.z = ((source[2].yyyy)*(source[3].zzzz)).z;
    // 27: mad r2.x, cb0[3].w, r1.x, r1.z
    r2.x = ((source[3].wwww)*(r1.xxxx)+(r1.zzzz)).x;
    // 28: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 29: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 30: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 31: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 32: sqrt r0.y, r0.z
    r0.y = (sqrt(r0.zzzz)).y;
    // 33: add r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)+(r0.yyyy)).y;
    // 34: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 36: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 37: mul r0.w, r0.z, cb0[4].y
    r0.w = ((r0.zzzz)*(source[4].yyyy)).w;
    // 38: mul r0.z, r0.z, cb0[3].x
    r0.z = ((r0.zzzz)*(source[3].xxxx)).z;
    // 39: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 40: movc r0.z, r0.y, l(0), r0.z
    r0.z = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 41: add r0.z, r0.z, v4.z
    r0.z = ((r0.zzzz)+(v4.zzzz)).z;
    // 42: add r1.y, r0.z, l(-1.000000)
    r1.y = ((r0.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 43: mul r1.xy, r1.xyxx, cb0[2].zwzz
    r1.xy = ((r1.xyxx)*(source[2].zwzz)).xy;
    // 44: exp r0.z, r0.w
    r0.z = (exp2(r0.wwww)).z;
    // 45: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 46: mul r0.y, r0.y, cb0[4].x
    r0.y = ((r0.yyyy)*(source[4].xxxx)).y;
    // 47: mad r2.y, cb0[2].y, cb0[4].z, r0.y
    r2.y = ((source[2].yyyy)*(source[4].zzzz)+(r0.yyyy)).y;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r2.xyxx, t1.yxzw, s2, l(0.000000)
    r0.y = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).yxzw).y;
    // 49: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 50: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 51: add r1.zw, r0.zzzw, r0.yyyy
    r1.zw = ((r0.zzzw)+(r0.yyyy)).zw;
    // 52: add r0.yz, r0.zzwz, -r1.zzwz
    r0.yz = ((r0.zzwz)+(-(r1.zzwz))).yz;
    // 53: mad r0.yz, v4.yyyy, r0.yyzy, r1.zzwz
    r0.yz = ((v4.yyyy)*(r0.yyzy)+(r1.zzwz)).yz;
    // 54: sample_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s0 (project resolved HDR SceneColor snapshot adapter)
    r0.yzw = (g_EffectSceneColorTexture.Sample(LinearClampUVSampler, (r0.yzyy).xy).wxyz).yzw;
    // 55: dp3 r0.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 56: add r0.z, -r0.y, r0.z
    r0.z = ((-(r0.yyyy))+(r0.zzzz)).z;
    // 57: mad r0.y, cb0[4].w, r0.z, r0.y
    r0.y = ((source[4].wwww)*(r0.zzzz)+(r0.yyyy)).y;
    // 58: ge r0.y, r0.y, cb0[5].x
    r0.y = (asfloat((uint4)((r0.yyyy)>=(source[5].xxxx)) * 0xffffffffu)).y;
    // 59: mul_sat r2.xyz, v3.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000)
    r2.xyz = (saturate((v3.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000)))).xyz;
    // 60: movc r0.yzw, r0.yyyy, l(0,0,0,0), r2.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 61: sqrt r2.xyz, r0.yzwy
    r2.xyz = (sqrt(r0.yzwy)).xyz;
    // 62: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 63: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 64: mad r0.yzw, -r2.xxyz, r2.xxyz, r0.yyzw
    r0.yzw = ((-(r2.xxyz))*(r2.xxyz)+(r0.yyzw)).yzw;
    // 65: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 66: mad r0.yzw, v4.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((v4.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 67: mad r2.x, cb0[2].y, cb0[2].x, r1.x
    r2.x = ((source[2].yyyy)*(source[2].xxxx)+(r1.xxxx)).x;
    // 68: mad r2.y, cb0[2].y, cb0[3].y, r1.y
    r2.y = ((source[2].yyyy)*(source[3].yyyy)+(r1.yyyy)).y;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xyz = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, true).xyzw).xyz;
    // 70: mov_sat r1.xyz, r1.xyzx
    r1.xyz = (saturate(r1.xyzx)).xyz;
    // 71: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 72: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 73: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 74: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 75: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 76: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 77: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 78: mul_sat r0.y, r0.y, cb0[5].z
    r0.y = (saturate((r0.yyyy)*(source[5].zzzz))).y;
    // 79: mul r0.y, r0.y, cb0[5].w
    r0.y = ((r0.yyyy)*(source[5].wwww)).y;
    // 80: mul r0.y, r0.y, v3.w
    r0.y = ((r0.yyyy)*(v3.wwww)).y;
    // 81: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 82: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}

// fx_r_pa_flashbang_03_02_tr: 58e1a4d7a3da5d40a9872a0c55cc295f; selected map 64ee4fe278d46b60b34e916f70633058783cf3d035e347377bff587d04bf7e41.
float4 ALTVNative161(ALTV_NATIVE_INPUT input)
{
    float4 source[7]; [unroll] for (uint i=0u; i<7u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[3u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[4].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[5].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[6].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
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
    r0.z = (ALTVNativeSample0((r1.xyxx).xy, (source[4].xxxx).x, true).yzxw).z;
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
    r0.w = (ALTVNativeSample1((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
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

// fx_e_me_ri_04_1_ts_tr: 5beae44d424f1541be35fe21f3b44a4a; selected map 24d97e9b6c25b59ca60d2c7a9b16376c0c58c630e2d1ffc7c3f083032479680a.
float4 ALTVNative162(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[4].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[4].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
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
    // 1: mad r0.x, cb0[4].x, cb0[3].x, l(-1.000000)
    r0.x = ((source[4].xxxx)*(source[3].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, cb0[3].x, cb0[4].x
    r0.y = ((source[3].xxxx)*(source[4].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v4.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v4.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 7: mul r1.x, r1.x, cb0[4].w
    r1.x = ((r1.xxxx)*(source[4].wwww)).x;
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
    // 23: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 24: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_k_pa_ring_12_ad2355: 107e821614ee944aa4b7e35e6f57cded; selected map 33b95adfbb6e274fffd0a5208ae5c12ab5da2fc5d2a1420d1fc6ceb5a288e459.
float4 ALTVNative163(ALTV_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[6u];
    source[2] = ALTVNativeAppend(ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[3u].zzzz*g_ALTVSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[3u].zzzz*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[3] = g_ALTVSourceMaterialParameters[5u];
    source[4] = ALTVNativeAppend(ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))),ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[5].w = ((g_ALTVSourceMaterialParameters[3u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[6].x = (((g_ALTVSourceMaterialParameters[3u].zzzz*g_ALTVSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[6].y = (((g_ALTVSourceMaterialParameters[3u].zzzz*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[6].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[6].w = (ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[3u].zzzz*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[7].x = (ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[3u].zzzz*g_ALTVSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[0u].yyyy)).x;
    source[9].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[9].w = ((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].x = (((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[10].y = (((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0))).x;
    source[10].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[10].w = (ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(0.0, 0.0, 0.0, 0.0)))).x;
    source[11].x = (ALTVNativePeriodic(((g_ALTVSourceMaterialParameters[1u].wwww*g_ALTVSourceMaterialTime.xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[11].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[12].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[12].y = ((float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[12].z = ((float4(0.00999999978, 0.0, 0.0, 0.0)*(float4(100.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].xxxx))).x;
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
    r0.z = (ALTVNativeSample1((r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
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
    r2.xyz = (ALTVNativeSample0((r0.ywyy).xy, (source[5].xxxx).x, true).xyzw).xyz;
    // 60: mad r0.yw, r0.zzzz, cb0[8].wwww, r1.xxxy
    r0.yw = ((r0.zzzz)*(source[8].wwww)+(r1.xxxy)).yw;
    // 61: mad r1.xy, r0.zzzz, cb0[8].wwww, v2.xyxx
    r1.xy = ((r0.zzzz)*(source[8].wwww)+(v2.xyxx)).xy;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t2.yzxw, s4, l(0.000000)
    r0.z = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 63: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t1.yxzw, s3, cb0[5].x
    r0.y = (ALTVNativeSample2((r0.ywyy).xy, (source[5].xxxx).x, true).yxzw).y;
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

// fx_g_pa_ring_01_ad: d2f98fa3ff9867468d6951becfbcf7e7; selected map 5651218fc865f2d1162a0637698d7183bb98441305f2a03f0e35e99d2ff77bfb.
float4 ALTVNative164(ALTV_NATIVE_INPUT input)
{
    float4 source[14]; [unroll] for (uint i=0u; i<14u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[9u];
    source[2] = g_ALTVSourceMaterialParameters[8u];
    source[3] = ALTVNativeAppend(g_ALTVSourceMaterialParameters[4u].wwww,g_ALTVSourceMaterialParameters[5u].xxxx,1u);
    source[4].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[4].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[4].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[5].x = ((g_ALTVSourceMaterialParameters[4u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[5].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[6].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[6].w = ((g_ALTVSourceMaterialParameters[0u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[7].x = (((g_ALTVSourceMaterialParameters[0u].xxxx*g_ALTVSourceMaterialTime.xxxx)*g_ALTVSourceMaterialParameters[5u].zzzz)).x;
    source[7].y = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[8].x = ((g_ALTVSourceMaterialParameters[6u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].y = (((g_ALTVSourceMaterialParameters[6u].zzzz*g_ALTVSourceMaterialTime.xxxx)*g_ALTVSourceMaterialParameters[5u].zzzz)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[9].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[10].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[10].y = ((g_ALTVSourceMaterialParameters[2u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[10].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[11].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[11].z = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[11].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[12].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].z = ((float4(0.0, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[3u].xxxx)).x;
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
    // 32: mul r2.x, r1.x, cb0[9].z
    r2.x = ((r1.xxxx)*(source[9].zzzz)).x;
    // 33: mul r3.x, r1.x, cb0[7].y
    r3.x = ((r1.xxxx)*(source[7].yyyy)).x;
    // 34: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 35: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 36: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 37: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 38: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 39: mul r0.y, r0.y, cb0[5].z
    r0.y = ((r0.yyyy)*(source[5].zzzz)).y;
    // 40: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 41: add r0.y, v4.x, cb0[5].w
    r0.y = ((v4.xxxx)+(source[5].wwww)).y;
    // 42: mad r1.y, r0.y, l(-0.400000), r0.x
    r1.y = ((r0.yyyy)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.xxxx)).y;
    // 43: mad r3.y, r1.y, cb0[7].z, cb0[8].y
    r3.y = ((r1.yyyy)*(source[7].zzzz)+(source[8].yyyy)).y;
    // 44: sample_l_indexable(texture2d)(float,float,float,float) r0.xy, r3.xyxx, t0.xyzw, s1, l(-1.000000)
    r0.xy = (ALTVNativeSample1((r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xy;
    // 45: max r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (max(abs(r0.xyxx),float4(0.000001,0.000001,0.000000,0.000000))).xy;
    // 46: log r0.xy, r0.xyxx
    r0.xy = (log2(r0.xyxx)).xy;
    // 47: mul r0.xy, r0.xyxx, cb0[8].zzzz
    r0.xy = ((r0.xyxx)*(source[8].zzzz)).xy;
    // 48: exp r0.xy, r0.xyxx
    r0.xy = (exp2(r0.xyxx)).xy;
    // 49: mad r2.y, r1.y, cb0[9].w, cb0[10].y
    r2.y = ((r1.yyyy)*(source[9].wwww)+(source[10].yyyy)).y;
    // 50: mad r0.zw, cb0[8].wwww, r0.xxxy, r2.xxxy
    r0.zw = ((source[8].wwww)*(r0.xxxy)+(r2.xxxy)).zw;
    // 51: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(-1.000000)
    r2.xyz = (ALTVNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 52: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 53: add r3.xyz, -r2.xyzx, r0.zzzz
    r3.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 54: mad r2.xyz, r3.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r2.xyzx)).xyz;
    // 55: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 56: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 57: mul r2.xyz, r2.xyzx, cb0[10].zzzz
    r2.xyz = ((r2.xyzx)*(source[10].zzzz)).xyz;
    // 58: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 59: mul r3.xyz, r2.xyzx, cb0[10].wwww
    r3.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // 60: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 61: mad r2.xyz, -cb0[10].wwww, r2.xyzx, r0.zzzz
    r2.xyz = ((-(source[10].wwww))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 62: mad r2.xyz, cb0[11].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[11].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 63: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 64: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 1.000000, 0.800000), cb0[3].xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,1.000000,0.800000))+(source[3].xxxy)).zw;
    // 65: mul r3.x, r1.x, cb0[6].x
    r3.x = ((r1.xxxx)*(source[6].xxxx)).x;
    // 66: mad r3.y, r1.y, cb0[6].y, cb0[7].x
    r3.y = ((r1.yyyy)*(source[6].yyyy)+(source[7].xxxx)).y;
    // 67: mad r1.xy, cb0[8].wwww, r0.xyxx, r3.xyxx
    r1.xy = ((source[8].wwww)*(r0.xyxx)+(r3.xyxx)).xy;
    // 68: mad r0.xy, cb0[8].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[8].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s3, l(-1.000000)
    r0.x = (ALTVNativeSample3((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 70: mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 71: mul r1.xy, r1.xyxx, l(1.500000, 0.250000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(1.500000,0.250000,0.000000,0.000000))).xy;
    // 72: sample_l_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s0, l(-1.000000)
    r0.w = (ALTVNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzwx).w;
    // 73: mul r1.xyz, r0.xyzx, cb0[11].wwww
    r1.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 74: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 75: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 76: mul r1.xyz, r1.xyzx, cb0[12].xxxx
    r1.xyz = ((r1.xyzx)*(source[12].xxxx)).xyz;
    // 77: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 78: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 79: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 80: mul r1.w, r1.w, cb0[9].x
    r1.w = ((r1.wwww)*(source[9].xxxx)).w;
    // 81: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 82: mul r1.w, r1.w, cb0[9].y
    r1.w = ((r1.wwww)*(source[9].yyyy)).w;
    // 83: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 84: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 85: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 86: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 87: mad r1.xy, v2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 88: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 89: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 91: mul r1.x, r1.x, cb0[12].z
    r1.x = ((r1.xxxx)*(source[12].zzzz)).x;
    // 92: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 93: mul r1.x, r1.x, cb0[12].w
    r1.x = ((r1.xxxx)*(source[12].wwww)).x;
    // 94: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 95: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 97: log r1.y, r0.w
    r1.y = (log2(r0.wwww)).y;
    // 98: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 99: mul r1.y, r1.y, cb0[12].y
    r1.y = ((r1.yyyy)*(source[12].yyyy)).y;
    // 100: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 101: movc r0.w, r0.w, l(0), r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).w;
    // 102: mul_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)*(r0.wwww))).w;
    // 103: mul r0.w, r0.w, cb0[13].x
    r0.w = ((r0.wwww)*(source[13].xxxx)).w;
    // 104: mul r0.w, r0.w, v3.w
    r0.w = ((r0.wwww)*(v3.wwww)).w;
    // 105: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 106: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 107: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_o_pa_sy_08_2_ts_tr: 201c5765d89297459daec45853e82d62; selected map 5a23fdbd3f9a93a4ff0b80d0575a9d7d44caea27aa93dca573be998c8a730aca.
float4 ALTVNative165(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2] = g_ALTVSourceMaterialParameters[1u];
    source[3].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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

// fx_d_pa_atta_05_23_tr: 9b7d0436b976c84d8292abbeeb27f843; selected map 293d2a3997f15bd7c6b29400e6515afc4b53673272c9dd05a77b8590bafc05d7.
float4 ALTVNative166(ALTV_NATIVE_INPUT input)
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyz = (ALTVNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 2: mul r0.w, r0.x, v3.w
    r0.w = ((r0.xxxx)*(v3.wwww)).w;
    // 3: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 4: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 5: mov_sat r0.w, r0.w
    r0.w = (saturate(r0.wwww)).w;
    // 6: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    return output;
}

// fx_j_rgbsplit_01_01_ad: d880c361f16370468cfdac8d4688d393; selected map e84de71c52a818919646772cd3b0a0181fe0758eaae2cdb6c89bc69d21826863.
float4 ALTVNative168(ALTV_NATIVE_INPUT input)
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

// fx_m_pa_shine_01_3_ad: f308e6c8bf787a4fbb242a07ff277211; selected map 7300c3c9b40788b80b30a5d639bda8170624b700959cd1ce48b334633e0f6586.
float4 ALTVNative169(ALTV_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[5u];
    source[2] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[4].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[4].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[2u].zzzz)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[4].w = (clamp(g_ALTVSourceMaterialParameters[4u].yyyy,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0))).x;
    source[5].x = ((float4(1.0, 0.0, 0.0, 0.0)-clamp(g_ALTVSourceMaterialParameters[4u].yyyy,float4(0.0, 0.0, 0.0, 0.0),float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[6].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[6].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[6].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[7].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[9].x = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[9].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
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
    // 2: dp4 r0.x, cb0[2].xyxy, r0.xyzw
    r0.x = (dot((source[2].xyxy).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 3: dp2 r0.y, cb0[3].xyxx, r0.zwzz
    r0.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: add r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 5: add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 6: mad r0.w, r0.z, cb0[4].w, cb0[5].x
    r0.w = ((r0.zzzz)*(source[4].wwww)+(source[5].xxxx)).w;
    // 7: mul_sat r0.z, r0.z, cb0[9].x
    r0.z = (saturate((r0.zzzz)*(source[9].xxxx))).z;
    // 8: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 9: div r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)/(r0.wwww)).x;
    // 10: mad_sat r0.x, r0.x, l(0.500000), l(0.500000)
    r0.x = (saturate((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000)))).x;
    // 11: add r0.w, -r0.x, l(1.000000)
    r0.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: mul r0.w, r0.x, r0.w
    r0.w = ((r0.xxxx)*(r0.wwww)).w;
    // 13: mul r1.x, r0.w, l(4.000000)
    r1.x = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 14: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 15: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 16: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 17: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 18: mul r1.x, r1.x, cb0[5].z
    r1.x = ((r1.xxxx)*(source[5].zzzz)).x;
    // 19: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 20: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 21: mul r1.x, r1.x, cb0[5].w
    r1.x = ((r1.xxxx)*(source[5].wwww)).x;
    // 22: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 23: mul r1.x, r1.x, cb0[6].x
    r1.x = ((r1.xxxx)*(source[6].xxxx)).x;
    // 24: lt r1.y, |r0.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 26: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 27: mul r1.x, r0.x, cb0[6].w
    r1.x = ((r0.xxxx)*(source[6].wwww)).x;
    // 28: mad r1.x, cb0[6].z, cb0[6].y, r1.x
    r1.x = ((source[6].zzzz)*(source[6].yyyy)+(r1.xxxx)).x;
    // 29: mul r1.zw, cb0[6].zzzz, cb0[7].yyyz
    r1.zw = ((source[6].zzzz)*(source[7].yyyz)).zw;
    // 30: mad r1.y, cb0[7].x, r0.y, r1.z
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r1.zzzz)).y;
    // 31: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 32: mad r2.y, cb0[6].z, cb0[8].y, r0.y
    r2.y = ((source[6].zzzz)*(source[8].yyyy)+(r0.yyyy)).y;
    // 33: mad r2.x, cb0[7].w, r0.x, r1.w
    r2.x = ((source[7].wwww)*(r0.xxxx)+(r1.wwww)).x;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t0.yxzw, s0, l(0.000000)
    r0.y = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 36: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 37: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 38: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 39: mul r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)*(source[8].zzzz)).x;
    // 40: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 41: mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // 42: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 43: mul_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)*(r0.wwww))).x;
    // 44: log r0.y, r0.z
    r0.y = (log2(r0.zzzz)).y;
    // 45: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 46: mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // 47: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 48: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 49: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 51: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 52: add r0.yzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((v3.xxyz)+(source[1].xxyz)).yzw;
    // 53: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 54: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 55: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_whkf_theworld_glitterwave_01: 2cc2a10636a51f4cb79676b8843c4a38; selected map d07ad4067fe7bd1c82b5f16720a90ee6e3cfedaca2900e55bd3fc89eeee5a5bc.
float4 ALTVNative170(ALTV_NATIVE_INPUT input)
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
    source[12].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[12].z = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[12].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[13].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].xxxx)).x;
    source[13].z = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[13].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[14].x = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[14].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[14].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[14].w = ((g_ALTVSourceMaterialParameters[7u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[15].x = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[15].y = (g_ALTVSourceMaterialParameters[16u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[17u].yyyy).x;
    source[15].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[16].x = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[16].y = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[16].z = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[16].w = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[17].x = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[17].y = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[17].z = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[17].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[18].x = ((g_ALTVSourceMaterialParameters[8u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[18].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[18].z = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[18].w = (g_ALTVSourceMaterialParameters[16u].yyyy).x;
    source[19].x = (g_ALTVSourceMaterialParameters[17u].zzzz).x;
    source[19].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[19].z = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[19].w = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[20].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[20].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].xxxx)).x;
    source[20].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[20].w = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[21].x = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[21].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[21].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[21].w = ((g_ALTVSourceMaterialParameters[8u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[22].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[22].y = (g_ALTVSourceMaterialParameters[16u].zzzz).x;
    source[22].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[22].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[23].x = (g_ALTVSourceMaterialParameters[17u].xxxx).x;
    source[23].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[23].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
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
    // 49: mad r1.x, v4.x, cb0[17].z, cb0[17].w
    r1.x = ((v4.xxxx)*(source[17].zzzz)+(source[17].wwww)).x;
    // 50: add r1.x, r1.x, cb0[18].x
    r1.x = ((r1.xxxx)+(source[18].xxxx)).x;
    // 51: mad r1.x, cb0[18].y, r0.y, r1.x
    r1.x = ((source[18].yyyy)*(r0.yyyy)+(r1.xxxx)).x;
    // 52: mad r1.y, v4.x, cb0[16].x, cb0[16].y
    r1.y = ((v4.xxxx)*(source[16].xxxx)+(source[16].yyyy)).y;
    // 53: add r1.y, r1.y, cb0[16].w
    r1.y = ((r1.yyyy)+(source[16].wwww)).y;
    // 54: mad r1.y, r0.x, cb0[15].w, r1.y
    r1.y = ((r0.xxxx)*(source[15].wwww)+(r1.yyyy)).y;
    // 55: mad r2.x, cb0[17].x, r1.y, r1.x
    r2.x = ((source[17].xxxx)*(r1.yyyy)+(r1.xxxx)).x;
    // 56: mad r2.y, r1.x, cb0[18].z, r1.y
    r2.y = ((r1.xxxx)*(source[18].zzzz)+(r1.yyyy)).y;
    // 57: mad r1.xy, cb0[18].wwww, r0.zwzz, r2.xyxx
    r1.xy = ((source[18].wwww)*(r0.zwzz)+(r2.xyxx)).xy;
    // 58: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s4, cb0[2].x
    r1.xyz = (ALTVNativeSample4((r1.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 59: mad r1.w, v4.x, cb0[14].x, cb0[14].y
    r1.w = ((v4.xxxx)*(source[14].xxxx)+(source[14].yyyy)).w;
    // 60: mad r1.w, cb0[14].z, r0.y, r1.w
    r1.w = ((source[14].zzzz)*(r0.yyyy)+(r1.wwww)).w;
    // 61: add r1.w, r1.w, cb0[14].w
    r1.w = ((r1.wwww)+(source[14].wwww)).w;
    // 62: mad r2.x, v4.x, cb0[12].z, cb0[12].w
    r2.x = ((v4.xxxx)*(source[12].zzzz)+(source[12].wwww)).x;
    // 63: mad r2.x, r0.x, cb0[12].y, r2.x
    r2.x = ((r0.xxxx)*(source[12].yyyy)+(r2.xxxx)).x;
    // 64: add r2.x, r2.x, cb0[13].y
    r2.x = ((r2.xxxx)+(source[13].yyyy)).x;
    // 65: mad r3.x, cb0[13].z, r2.x, r1.w
    r3.x = ((source[13].zzzz)*(r2.xxxx)+(r1.wwww)).x;
    // 66: mad r3.y, r1.w, cb0[15].x, r2.x
    r3.y = ((r1.wwww)*(source[15].xxxx)+(r2.xxxx)).y;
    // 67: mad r2.xy, cb0[15].yyyy, r0.zwzz, r3.xyxx
    r2.xy = ((source[15].yyyy)*(r0.zwzz)+(r3.xyxx)).xy;
    // 68: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s3, cb0[2].x
    r2.xyz = (ALTVNativeSample3((r2.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 69: mul r2.xyzw, r2.xxyz, cb0[15].zzzz
    r2.xyzw = ((r2.xxyz)*(source[15].zzzz)).xyzw;
    // 70: mad r1.xyzw, cb0[19].xxxx, r1.xxyz, r2.xyzw
    r1.xyzw = ((source[19].xxxx)*(r1.xxyz)+(r2.xyzw)).xyzw;
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
    // 81: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s5, cb0[2].x
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
    // 98: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s0, cb0[2].x
    r0.xyz = (ALTVNativeSample0((r0.xyxx).xy, (source[2].xxxx).x, true).xyzw).xyz;
    // 99: mul r2.xyz, r1.yzwy, r0.xyzx
    r2.xyz = ((r1.yzwy)*(r0.xyzx)).xyz;
    // 100: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: mad r0.xyz, -r0.xyzx, r1.yzwy, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(r1.yzwy)+(r0.wwww)).xyz;
    // 102: mul_sat r0.w, r1.x, v3.w
    r0.w = (saturate((r1.xxxx)*(v3.wwww))).w;
    // 103: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 104: mad r0.xyz, cb0[23].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[23].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 105: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 106: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 107: mul r0.xyz, r0.xyzx, cb0[23].yyyy
    r0.xyz = ((r0.xyzx)*(source[23].yyyy)).xyz;
    // 108: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 109: mul r0.xyz, r0.xyzx, cb0[23].zzzz
    r0.xyz = ((r0.xyzx)*(source[23].zzzz)).xyz;
    // 110: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 111: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_me_softsplitline_01_01_ad: 35589f6cd8949843afcec787e7b40da4; selected map e046a07808c3f2de8beb19eae3baa92061827b50e3b4d69add8502b8d9993308.
float4 ALTVNative171(ALTV_NATIVE_INPUT input)
{
    float4 source[5]; [unroll] for (uint i=0u; i<5u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[2u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[4].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 10: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
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
    // 23: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: min r2.x, r0.w, l(1.000000)
    r2.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: mul r2.y, r2.x, v4.y
    r2.y = ((r2.xxxx)*(v4.yyyy)).y;
    // 26: mul r2.y, r2.y, cb0[1].w
    r2.y = ((r2.yyyy)*(source[1].wwww)).y;
    // 27: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 28: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: lt r2.y, |r0.w|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 30: mul r2.z, |r0.w|, |r0.w|
    r2.z = ((abs(r0.wwww))*(abs(r0.wwww))).z;
    // 31: mul r0.w, |r0.w|, r2.z
    r0.w = ((abs(r0.wwww))*(r2.zzzz)).w;
    // 32: mul_sat r0.w, r0.w, cb0[4].x
    r0.w = (saturate((r0.wwww)*(source[4].xxxx))).w;
    // 33: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: movc r0.w, r2.y, l(1.000000), r0.w
    r0.w = ((asuint(r2.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.wwww)).w;
    // 35: lt r2.y, r0.w, l(0.000001)
    r2.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 36: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 37: mul r0.w, r0.w, cb0[4].y
    r0.w = ((r0.wwww)*(source[4].yyyy)).w;
    // 38: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 39: movc r0.w, r2.y, l(0), r0.w
    r0.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 40: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 41: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 42: add r0.w, v4.x, l(-0.500000)
    r0.w = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 43: add r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))+(abs(r0.wwww))).w;
    // 44: lt r0.w, |r0.w|, l(0.000000)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 45: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 46: mul r1.w, r1.w, l(10.000000)
    r1.w = ((r1.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: mul r1.w, r1.w, l(0.035000)
    r1.w = ((r1.wwww)*(float4(0.035000,0.035000,0.035000,0.035000))).w;
    // 49: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: mad r0.w, cb0[3].x, l(0.010000), r0.w
    r0.w = ((source[3].xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r0.wwww)).w;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, r1.xyxx, t1.wxyz, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.yzw = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).wxyz).yzw;
    // 52: add r3.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 53: mul r3.xy, r3.xyxx, l(0.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(0.000000,1.000000,0.000000,0.000000))).xy;
    // 54: mul r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 55: mov r3.zw, r1.yyyx
    r3.zw = (r1.yyyx).zw;
    // 56: mov r4.x, r2.y
    r4.x = (r2.yyyy).x;
    // 57: mov r4.y, l(0)
    r4.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 58: loop
    [loop] while (true) {
    // 59: ge r1.w, r4.y, l(3.000000)
    r1.w = (asfloat((uint4)((r4.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 60: breakc_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) break;
    // 61: mad r3.zw, -r3.yyyx, r0.wwww, r3.zzzw
    r3.zw = ((-(r3.yyyx))*(r0.wwww)+(r3.zzzw)).zw;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.wzww, t1.yzwx, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.wzww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 63: add r4.x, r1.w, r4.x
    r4.x = ((r1.wwww)+(r4.xxxx)).x;
    // 64: add r4.y, r4.y, l(1.000000)
    r4.y = ((r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: endloop
    }
    // 66: mov r1.w, r1.x
    r1.w = (r1.xxxx).w;
    // 67: mov r1.y, r3.z
    r1.y = (r3.zzzz).y;
    // 68: mov r5.x, r2.z
    r5.x = (r2.zzzz).x;
    // 69: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 70: loop
    [loop] while (true) {
    // 71: ge r2.y, r5.y, l(3.000000)
    r2.y = (asfloat((uint4)((r5.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).y;
    // 72: breakc_nz r2.y
    if ((asuint(r2.yyyy)).x != 0u) break;
    // 73: mad r1.yw, -r3.yyyx, r0.wwww, r1.yyyw
    r1.yw = ((-(r3.yyyx))*(r0.wwww)+(r1.yyyw)).yw;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r1.wyww, t1.xyzw, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.y = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r1.wyww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).y;
    // 75: add r5.x, r2.y, r5.x
    r5.x = ((r2.yyyy)+(r5.xxxx)).x;
    // 76: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 77: endloop
    }
    // 78: mov r4.y, r5.x
    r4.y = (r5.xxxx).y;
    // 79: mov r5.xy, r1.xyxx
    r5.xy = (r1.xyxx).xy;
    // 80: mov r6.x, r2.w
    r6.x = (r2.wwww).x;
    // 81: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 82: loop
    [loop] while (true) {
    // 83: ge r1.w, r6.y, l(3.000000)
    r1.w = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 84: breakc_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) break;
    // 85: mad r5.xy, -r3.xyxx, r0.wwww, r5.xyxx
    r5.xy = ((-(r3.xyxx))*(r0.wwww)+(r5.xyxx)).xy;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r5.xyxx, t1.xywz, s1, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r1.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 87: add r6.x, r1.w, r6.x
    r6.x = ((r1.wwww)+(r6.xxxx)).x;
    // 88: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: endloop
    }
    // 90: mov r4.z, r6.x
    r4.z = (r6.xxxx).z;
    // 91: mul r1.xyw, r4.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000)
    r1.xyw = ((r4.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))).xyw;
    // 92: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: mad r2.yzw, -r4.xxyz, l(0.000000, 0.250000, 0.250000, 0.250000), r0.wwww
    r2.yzw = ((-(r4.xxyz))*(float4(0.000000,0.250000,0.250000,0.250000))+(r0.wwww)).yzw;
    // 94: mad r1.xyw, cb0[3].yyyy, r2.yzyw, r1.xyxw
    r1.xyw = ((source[3].yyyy)*(r2.yzyw)+(r1.xyxw)).xyw;
    // 95: lt r0.w, r2.x, l(0.000001)
    r0.w = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 96: mul r2.y, r2.x, r2.x
    r2.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 97: mul r2.y, r2.y, r2.x
    r2.y = ((r2.yyyy)*(r2.xxxx)).y;
    // 98: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: lt r2.z, r2.x, l(0.000001)
    r2.z = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 100: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 101: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 102: or r0.w, r0.w, r2.z
    r0.w = (asfloat(asuint(r0.wwww) | asuint(r2.zzzz))).w;
    // 103: movc r0.w, r0.w, l(0), r2.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 104: add r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)+(r1.xyxw)).xyw;
    // 105: mul_sat r1.xyw, r1.xyxw, cb0[1].xyxz
    r1.xyw = (saturate((r1.xyxw)*(source[1].xyxz))).xyw;
    // 106: add r1.xyw, r1.xyxw, cb0[2].xyxz
    r1.xyw = ((r1.xyxw)+(source[2].xyxz)).xyw;
    // 107: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_pa_swp_gra_01_tr: 7e1e5a147983d54693c042eafe7094e4; selected map fd27f1140d5016268e79e725521e4f4f01197849fbb63ee5b62be3e1e5a6cc14.
float4 ALTVNative172(ALTV_NATIVE_INPUT input)
{
    float4 source[15]; [unroll] for (uint i=0u; i<15u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[12u];
    source[2].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[2].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[2].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[2].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[3].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[5u].wwww)).x;
    source[3].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[3].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[4].x = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[4].y = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[4].w = ((g_ALTVSourceMaterialParameters[5u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[5].z = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[6].y = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[6].w = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[8].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[8].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[9].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[9].w = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[10].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[10].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[10].z = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[10].w = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[12].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[12].y = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[12].w = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[13].y = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[13].z = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[13].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[14].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 8: mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // 9: mul r0.xy, r0.xyxx, v4.zzzz
    r0.xy = ((r0.xyxx)*(v4.zzzz)).xy;
    // 10: mad r0.z, v4.x, cb0[9].x, cb0[9].y
    r0.z = ((v4.xxxx)*(source[9].xxxx)+(source[9].yyyy)).z;
    // 11: mad r0.z, v2.y, cb0[8].w, r0.z
    r0.z = ((v2.yyyy)*(source[8].wwww)+(r0.zzzz)).z;
    // 12: mad r0.w, v4.x, cb0[9].w, cb0[10].x
    r0.w = ((v4.xxxx)*(source[9].wwww)+(source[10].xxxx)).w;
    // 13: mad r0.w, cb0[10].y, v2.x, r0.w
    r0.w = ((source[10].yyyy)*(v2.xxxx)+(r0.wwww)).w;
    // 14: mad r1.x, cb0[9].z, r0.z, r0.w
    r1.x = ((source[9].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 15: mad r1.y, r0.w, cb0[10].z, r0.z
    r1.y = ((r0.wwww)*(source[10].zzzz)+(r0.zzzz)).y;
    // 16: mad r0.zw, cb0[10].wwww, r0.xxxy, r1.xxxy
    r0.zw = ((source[10].wwww)*(r0.xxxy)+(r1.xxxy)).zw;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s3, l(0.000000)
    r0.z = (ALTVNativeSample2((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 18: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 19: mad r0.w, v4.x, cb0[11].z, cb0[11].w
    r0.w = ((v4.xxxx)*(source[11].zzzz)+(source[11].wwww)).w;
    // 20: mad r0.w, v2.y, cb0[11].y, r0.w
    r0.w = ((v2.yyyy)*(source[11].yyyy)+(r0.wwww)).w;
    // 21: mad r1.x, v4.x, cb0[12].y, cb0[12].z
    r1.x = ((v4.xxxx)*(source[12].yyyy)+(source[12].zzzz)).x;
    // 22: mad r1.x, cb0[12].w, v2.x, r1.x
    r1.x = ((source[12].wwww)*(v2.xxxx)+(r1.xxxx)).x;
    // 23: mad r2.x, cb0[12].x, r0.w, r1.x
    r2.x = ((source[12].xxxx)*(r0.wwww)+(r1.xxxx)).x;
    // 24: mad r2.y, r1.x, cb0[13].x, r0.w
    r2.y = ((r1.xxxx)*(source[13].xxxx)+(r0.wwww)).y;
    // 25: mad r1.xy, cb0[13].yyyy, r0.xyxx, r2.xyxx
    r1.xy = ((source[13].yyyy)*(r0.xyxx)+(r2.xyxx)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s4, l(0.000000)
    r0.w = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 27: mul r0.w, r0.w, cb0[13].z
    r0.w = ((r0.wwww)*(source[13].zzzz)).w;
    // 28: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 29: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 30: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 31: mul r0.w, v4.w, cb0[13].w
    r0.w = ((v4.wwww)*(source[13].wwww)).w;
    // 32: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: mul_sat r0.z, r0.z, cb0[14].x
    r0.z = (saturate((r0.zzzz)*(source[14].xxxx))).z;
    // 35: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 36: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 37: source device depth mapped to centimetre view depth; reconstruction at 39.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 39-42: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 43: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 44: add r1.x, -cb0[14].y, l(1.000000)
    r1.x = ((-(source[14].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 46: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 47: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 48: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 49: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 50: mad r0.z, v4.x, cb0[2].y, cb0[2].z
    r0.z = ((v4.xxxx)*(source[2].yyyy)+(source[2].zzzz)).z;
    // 51: mad r0.z, v2.y, cb0[2].x, r0.z
    r0.z = ((v2.yyyy)*(source[2].xxxx)+(r0.zzzz)).z;
    // 52: add r0.z, r0.z, cb0[3].y
    r0.z = ((r0.zzzz)+(source[3].yyyy)).z;
    // 53: mad r0.w, v4.x, cb0[4].x, cb0[4].y
    r0.w = ((v4.xxxx)*(source[4].xxxx)+(source[4].yyyy)).w;
    // 54: mad r0.w, cb0[4].z, v2.x, r0.w
    r0.w = ((source[4].zzzz)*(v2.xxxx)+(r0.wwww)).w;
    // 55: add r0.w, r0.w, cb0[4].w
    r0.w = ((r0.wwww)+(source[4].wwww)).w;
    // 56: mad r1.x, cb0[3].z, r0.z, r0.w
    r1.x = ((source[3].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 57: mad r1.y, r0.w, cb0[5].x, r0.z
    r1.y = ((r0.wwww)*(source[5].xxxx)+(r0.zzzz)).y;
    // 58: mad r0.xy, cb0[7].zzzz, r0.xyxx, r1.xyxx
    r0.xy = ((source[7].zzzz)*(r0.xyxx)+(r1.xyxx)).xy;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xyz = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 60: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 62: mad r0.xyz, cb0[7].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[7].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 63: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 64: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 65: mul r0.xyz, r0.xyzx, cb0[8].xxxx
    r0.xyz = ((r0.xyzx)*(source[8].xxxx)).xyz;
    // 66: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 67: mad r0.xyz, cb0[8].yyyy, r0.xyzx, cb0[8].zzzz
    r0.xyz = ((source[8].yyyy)*(r0.xyzx)+(source[8].zzzz)).xyz;
    // 68: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 69: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_s_pa_ringmaster_01_221_ad: 077a427ac67c02408d75c570c7b866a1; selected map c9d800a9638511bc72a1645b184e4f39f765cb2eafdfc9ade81d075be8ec1cde.
float4 ALTVNative173(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[8u];
    source[2] = g_ALTVSourceMaterialParameters[7u];
    source[3] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[9].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[1u].wwww)).x;
    source[10].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[1u].wwww))).x;
    source[10].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[10].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[2u].xxxx)).x;
    source[10].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[11].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[11].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[11].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx))).x;
    source[12].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[12].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[12].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[13].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)).x;
    source[13].w = (((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[14].x = (sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[14].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[14].z = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[14].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[15].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
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
    // 22: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 23: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 24: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 25: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: mul r0.z, v4.y, cb0[7].z
    r0.z = ((v4.yyyy)*(source[7].zzzz)).z;
    // 27: mul r0.w, v2.x, cb0[6].w
    r0.w = ((v2.xxxx)*(source[6].wwww)).w;
    // 28: mul r1.z, cb0[5].x, cb0[5].y
    r1.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 29: mad r2.x, r1.z, cb0[6].z, r0.w
    r2.x = ((r1.zzzz)*(source[6].zzzz)+(r0.wwww)).x;
    // 30: mul r2.zw, r1.zzzz, cb0[7].yyyw
    r2.zw = ((r1.zzzz)*(source[7].yyyw)).zw;
    // 31: mad r2.y, cb0[7].x, v2.y, r2.z
    r2.y = ((source[7].xxxx)*(v2.yyyy)+(r2.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 33: mul r0.w, v4.z, cb0[6].y
    r0.w = ((v4.zzzz)*(source[6].yyyy)).w;
    // 34: dp2 r1.w, r0.xyxx, r0.xyxx
    r1.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 35: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 36: add r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)+(r1.wwww)).z;
    // 37: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 38: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 39: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 40: lt r2.z, r1.w, l(0.000000)
    r2.z = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 41: movc r1.y, r2.z, l(0), r0.w
    r1.y = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 42: mad r0.zw, r0.zzzz, r2.xxxy, r1.xxxy
    r0.zw = ((r0.zzzz)*(r2.xxxy)+(r1.xxxy)).zw;
    // 43: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 44: mad r1.x, r1.z, cb0[5].z, r1.x
    r1.x = ((r1.zzzz)*(source[5].zzzz)+(r1.xxxx)).x;
    // 45: mad r1.y, cb0[6].x, r0.w, r2.w
    r1.y = ((source[6].xxxx)*(r0.wwww)+(r2.wwww)).y;
    // 46: mul r0.zw, r0.zzzw, cb0[8].yyyz
    r0.zw = ((r0.zzzw)*(source[8].yyyz)).zw;
    // 47: mad r0.zw, r1.zzzz, cb0[8].xxxw, r0.zzzw
    r0.zw = ((r1.zzzz)*(source[8].xxxw)+(r0.zzzw)).zw;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s2, l(-1.000000)
    r2.xyz = (ALTVNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s0, l(-1.000000)
    r1.xyz = (ALTVNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 50: mul r3.xyz, r2.xyzx, r1.xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 51: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 52: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 53: mad r1.xyz, cb0[9].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[9].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 54: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 56: mul r1.xyz, r1.xyzx, cb0[9].yyyy
    r1.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 57: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 58: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 59: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 60: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 61: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 62: dp2 r2.x, cb0[3].xyxx, r0.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 63: dp2 r2.y, cb0[4].xyxx, r0.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 64: mad r0.xy, cb0[14].wwww, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[14].wwww)*(r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s3, l(0.000000)
    r0.x = (ALTVNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 66: mad r0.y, -r1.w, cb0[10].x, l(1.000000)
    r0.y = ((-(r1.wwww))*(source[10].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: mad r0.z, -r1.w, cb0[11].w, l(1.000000)
    r0.z = ((-(r1.wwww))*(source[11].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 68: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 69: mul_sat r0.y, r0.y, cb0[11].x
    r0.y = (saturate((r0.yyyy)*(source[11].xxxx))).y;
    // 70: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 71: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 72: lt r0.z, r0.y, l(0.000001)
    r0.z = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 73: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 74: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 75: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 76: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 77: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 78: mul_sat r0.x, r0.x, cb0[15].x
    r0.x = (saturate((r0.xxxx)*(source[15].xxxx))).x;
    // 79: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 81: mul r0.x, r0.x, cb0[15].y
    r0.x = ((r0.xxxx)*(source[15].yyyy)).x;
    // 82: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 83: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 84: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 85: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 86: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_d_pa_ringmaster_01_227_ad: 077a427ac67c02408d75c570c7b866a1; selected map c9d800a9638511bc72a1645b184e4f39f765cb2eafdfc9ade81d075be8ec1cde.
float4 ALTVNative174(ALTV_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[8u];
    source[2] = g_ALTVSourceMaterialParameters[7u];
    source[3] = ALTVNativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[4] = ALTVNativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[5].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[5].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[5].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[6].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[6].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[7].x = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[8].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[9].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[9].w = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[1u].wwww)).x;
    source[10].x = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[1u].wwww))).x;
    source[10].y = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[10].z = ((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[2u].xxxx)).x;
    source[10].w = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[11].x = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[2u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[11].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[11].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[11].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_ALTVSourceMaterialParameters[0u].xxxx))).x;
    source[12].x = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[12].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].yyyy)).x;
    source[12].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[12].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_ALTVSourceMaterialParameters[0u].yyyy),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[13].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)).x;
    source[13].w = (((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))).x;
    source[14].x = (sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[14].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[14].z = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_ALTVSourceMaterialParameters[5u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[14].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[15].y = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
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
    // 22: ge r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).x;
    // 23: and r0.w, r0.w, r1.x
    r0.w = (asfloat(asuint(r0.wwww) & asuint(r1.xxxx))).w;
    // 24: movc r0.z, r0.w, -r0.z, r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).z;
    // 25: mad r1.x, r0.z, l(0.159155), l(0.500000)
    r1.x = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 26: mul r0.z, v4.y, cb0[7].z
    r0.z = ((v4.yyyy)*(source[7].zzzz)).z;
    // 27: mul r0.w, v2.x, cb0[6].w
    r0.w = ((v2.xxxx)*(source[6].wwww)).w;
    // 28: mul r1.z, cb0[5].x, cb0[5].y
    r1.z = ((source[5].xxxx)*(source[5].yyyy)).z;
    // 29: mad r2.x, r1.z, cb0[6].z, r0.w
    r2.x = ((r1.zzzz)*(source[6].zzzz)+(r0.wwww)).x;
    // 30: mul r2.zw, r1.zzzz, cb0[7].yyyw
    r2.zw = ((r1.zzzz)*(source[7].yyyw)).zw;
    // 31: mad r2.y, cb0[7].x, v2.y, r2.z
    r2.y = ((source[7].xxxx)*(v2.yyyy)+(r2.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = (ALTVNativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 33: mul r0.w, v4.z, cb0[6].y
    r0.w = ((v4.zzzz)*(source[6].yyyy)).w;
    // 34: dp2 r1.w, r0.xyxx, r0.xyxx
    r1.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 35: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 36: add r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)+(r1.wwww)).z;
    // 37: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 38: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 39: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 40: lt r2.z, r1.w, l(0.000000)
    r2.z = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 41: movc r1.y, r2.z, l(0), r0.w
    r1.y = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 42: mad r0.zw, r0.zzzz, r2.xxxy, r1.xxxy
    r0.zw = ((r0.zzzz)*(r2.xxxy)+(r1.xxxy)).zw;
    // 43: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 44: mad r1.x, r1.z, cb0[5].z, r1.x
    r1.x = ((r1.zzzz)*(source[5].zzzz)+(r1.xxxx)).x;
    // 45: mad r1.y, cb0[6].x, r0.w, r2.w
    r1.y = ((source[6].xxxx)*(r0.wwww)+(r2.wwww)).y;
    // 46: mul r0.zw, r0.zzzw, cb0[8].yyyz
    r0.zw = ((r0.zzzw)*(source[8].yyyz)).zw;
    // 47: mad r0.zw, r1.zzzz, cb0[8].xxxw, r0.zzzw
    r0.zw = ((r1.zzzz)*(source[8].xxxw)+(r0.zzzw)).zw;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t3.xyzw, s2, l(-1.000000)
    r2.xyz = (ALTVNativeSample2((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 49: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s0, l(-1.000000)
    r1.xyz = (ALTVNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).xyz;
    // 50: mul r3.xyz, r2.xyzx, r1.xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 51: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 52: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 53: mad r1.xyz, cb0[9].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[9].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 54: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 56: mul r1.xyz, r1.xyzx, cb0[9].yyyy
    r1.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 57: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 58: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 59: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 60: mad r1.xyz, r1.xyzx, v3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 61: mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 62: dp2 r2.x, cb0[3].xyxx, r0.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 63: dp2 r2.y, cb0[4].xyxx, r0.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 64: mad r0.xy, cb0[14].wwww, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[14].wwww)*(r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s3, l(0.000000)
    r0.x = (ALTVNativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 66: mad r0.y, -r1.w, cb0[10].x, l(1.000000)
    r0.y = ((-(r1.wwww))*(source[10].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: mad r0.z, -r1.w, cb0[11].w, l(1.000000)
    r0.z = ((-(r1.wwww))*(source[11].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 68: mul_sat r0.z, r0.z, cb0[12].w
    r0.z = (saturate((r0.zzzz)*(source[12].wwww))).z;
    // 69: mul_sat r0.y, r0.y, cb0[11].x
    r0.y = (saturate((r0.yyyy)*(source[11].xxxx))).y;
    // 70: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 71: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 72: lt r0.z, r0.y, l(0.000001)
    r0.z = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 73: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 74: mul r0.y, r0.y, cb0[13].x
    r0.y = ((r0.yyyy)*(source[13].xxxx)).y;
    // 75: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 76: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 77: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 78: mul_sat r0.x, r0.x, cb0[15].x
    r0.x = (saturate((r0.xxxx)*(source[15].xxxx))).x;
    // 79: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 81: mul r0.x, r0.x, cb0[15].y
    r0.x = ((r0.xxxx)*(source[15].yyyy)).x;
    // 82: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 83: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 84: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 85: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 86: mul o0.xyz, r0.xxxx, r1.xyzx
    output.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_pa_highpixelring_01_05_ad: d9f3da526f61294bbc774b9fcc81634f; selected map b55a9a321fafcb0509636bbdd7eeb87bdf6366483544abaa428daba4e9ecd346.
float4 ALTVNative175(ALTV_NATIVE_INPUT input)
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

// fx_m_swp_line_01-2_tr: 315898462552c0409154253e11fcfcd2; selected map 0658c2ffeb58a3cfd6fb179b8852f23eaa413c8d768f2c14ede0177d6538f19c.
float4 ALTVNative176(ALTV_NATIVE_INPUT input)
{
    float4 source[23]; [unroll] for (uint i=0u; i<23u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[18u];
    source[2].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[2].y = (g_ALTVSourceMaterialParameters[15u].xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[2].w = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[3].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[3].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].zzzz)).x;
    source[3].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[3].w = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[4].x = (g_ALTVSourceMaterialParameters[13u].zzzz).x;
    source[4].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[4].w = ((g_ALTVSourceMaterialParameters[7u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[5].x = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[5].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[5].z = (g_ALTVSourceMaterialParameters[15u].yyyy).x;
    source[5].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[6].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[6].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].xxxx)).x;
    source[6].z = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[7].x = (g_ALTVSourceMaterialParameters[13u].wwww).x;
    source[7].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[7].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].w = ((g_ALTVSourceMaterialParameters[8u].wwww*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].x = (g_ALTVSourceMaterialParameters[12u].yyyy).x;
    source[8].y = (g_ALTVSourceMaterialParameters[17u].xxxx).x;
    source[8].z = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[8].w = (g_ALTVSourceMaterialParameters[15u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[9].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[9].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[9u].zzzz)).x;
    source[9].w = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[10].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[14u].xxxx).x;
    source[10].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[10].w = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[11].x = ((g_ALTVSourceMaterialParameters[9u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[11].y = (g_ALTVSourceMaterialParameters[12u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[17u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[16u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[16u].wwww).x;
    source[12].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[12].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[13].y = (g_ALTVSourceMaterialParameters[14u].yyyy).x;
    source[13].z = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[13].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[14].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[7u].wwww)).x;
    source[14].y = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[14].w = (g_ALTVSourceMaterialParameters[12u].wwww).x;
    source[15].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[15].z = ((g_ALTVSourceMaterialParameters[7u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[15].w = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[15u].wwww).x;
    source[16].y = (g_ALTVSourceMaterialParameters[17u].zzzz).x;
    source[16].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[14u].zzzz).x;
    source[17].x = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[17].y = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[17].z = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[8u].yyyy)).x;
    source[17].w = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[18].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[18].y = (g_ALTVSourceMaterialParameters[13u].xxxx).x;
    source[18].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[18].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[19].x = ((g_ALTVSourceMaterialParameters[8u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[19].y = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[19].z = (g_ALTVSourceMaterialParameters[16u].xxxx).x;
    source[19].w = (g_ALTVSourceMaterialParameters[17u].wwww).x;
    source[20].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[20].y = (g_ALTVSourceMaterialParameters[14u].wwww).x;
    source[20].z = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[20].w = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[21].x = (g_ALTVSourceMaterialParameters[13u].yyyy).x;
    source[21].y = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[21].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[21].w = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[22].x = (g_ALTVSourceMaterialParameters[16u].yyyy).x;
    source[22].y = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[22].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
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
    // 1: mad r0.x, v4.y, cb0[8].w, cb0[9].x
    r0.x = ((v4.yyyy)*(source[8].wwww)+(source[9].xxxx)).x;
    // 2: mad r0.x, v2.y, cb0[8].z, r0.x
    r0.x = ((v2.yyyy)*(source[8].zzzz)+(r0.xxxx)).x;
    // 3: add r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)+(source[9].zzzz)).x;
    // 4: mad r0.y, v4.y, cb0[10].y, cb0[10].z
    r0.y = ((v4.yyyy)*(source[10].yyyy)+(source[10].zzzz)).y;
    // 5: mad r0.y, cb0[10].w, v2.x, r0.y
    r0.y = ((source[10].wwww)*(v2.xxxx)+(r0.yyyy)).y;
    // 6: add r0.y, r0.y, cb0[11].x
    r0.y = ((r0.yyyy)+(source[11].xxxx)).y;
    // 7: mad r1.x, cb0[9].w, r0.x, r0.y
    r1.x = ((source[9].wwww)*(r0.xxxx)+(r0.yyyy)).x;
    // 8: mad r1.y, r0.y, cb0[11].y, r0.x
    r1.y = ((r0.yyyy)*(source[11].yyyy)+(r0.xxxx)).y;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 10: mul r0.xy, r0.xyxx, cb0[11].zzzz
    r0.xy = ((r0.xyxx)*(source[11].zzzz)).xy;
    // 11: mad r0.z, v4.y, cb0[5].z, cb0[5].w
    r0.z = ((v4.yyyy)*(source[5].zzzz)+(source[5].wwww)).z;
    // 12: mad r0.z, v2.y, cb0[5].y, r0.z
    r0.z = ((v2.yyyy)*(source[5].yyyy)+(r0.zzzz)).z;
    // 13: add r0.z, r0.z, cb0[6].y
    r0.z = ((r0.zzzz)+(source[6].yyyy)).z;
    // 14: mad r0.w, v4.y, cb0[7].x, cb0[7].y
    r0.w = ((v4.yyyy)*(source[7].xxxx)+(source[7].yyyy)).w;
    // 15: mad r0.w, cb0[7].z, v2.x, r0.w
    r0.w = ((source[7].zzzz)*(v2.xxxx)+(r0.wwww)).w;
    // 16: add r0.w, r0.w, cb0[7].w
    r0.w = ((r0.wwww)+(source[7].wwww)).w;
    // 17: mad r1.x, cb0[6].z, r0.z, r0.w
    r1.x = ((source[6].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 18: mad r1.y, r0.w, cb0[8].x, r0.z
    r1.y = ((r0.wwww)*(source[8].xxxx)+(r0.zzzz)).y;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 20: mad r0.xy, cb0[8].yyyy, r0.zwzz, r0.xyxx
    r0.xy = ((source[8].yyyy)*(r0.zwzz)+(r0.xyxx)).xy;
    // 21: mul r0.xy, r0.xyxx, v4.zzzz
    r0.xy = ((r0.xyxx)*(v4.zzzz)).xy;
    // 22: mad r0.z, v4.x, cb0[13].y, cb0[13].z
    r0.z = ((v4.xxxx)*(source[13].yyyy)+(source[13].zzzz)).z;
    // 23: mad r0.z, v2.y, cb0[13].x, r0.z
    r0.z = ((v2.yyyy)*(source[13].xxxx)+(r0.zzzz)).z;
    // 24: add r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)+(source[14].xxxx)).z;
    // 25: mad r0.w, v4.x, cb0[14].w, cb0[15].x
    r0.w = ((v4.xxxx)*(source[14].wwww)+(source[15].xxxx)).w;
    // 26: mad r0.w, cb0[15].y, v2.x, r0.w
    r0.w = ((source[15].yyyy)*(v2.xxxx)+(r0.wwww)).w;
    // 27: add r0.w, r0.w, cb0[15].z
    r0.w = ((r0.wwww)+(source[15].zzzz)).w;
    // 28: mad r1.x, cb0[14].y, r0.z, r0.w
    r1.x = ((source[14].yyyy)*(r0.zzzz)+(r0.wwww)).x;
    // 29: mad r1.y, r0.w, cb0[15].w, r0.z
    r1.y = ((r0.wwww)*(source[15].wwww)+(r0.zzzz)).y;
    // 30: mad r0.zw, cb0[16].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[16].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (ALTVNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 32: mul r0.z, r0.z, cb0[16].y
    r0.z = ((r0.zzzz)*(source[16].yyyy)).z;
    // 33: mad r0.w, v4.x, cb0[16].w, cb0[17].x
    r0.w = ((v4.xxxx)*(source[16].wwww)+(source[17].xxxx)).w;
    // 34: mad r0.w, v2.y, cb0[16].z, r0.w
    r0.w = ((v2.yyyy)*(source[16].zzzz)+(r0.wwww)).w;
    // 35: add r0.w, r0.w, cb0[17].z
    r0.w = ((r0.wwww)+(source[17].zzzz)).w;
    // 36: mad r1.x, v4.x, cb0[18].y, cb0[18].z
    r1.x = ((v4.xxxx)*(source[18].yyyy)+(source[18].zzzz)).x;
    // 37: mad r1.x, cb0[18].w, v2.x, r1.x
    r1.x = ((source[18].wwww)*(v2.xxxx)+(r1.xxxx)).x;
    // 38: add r1.x, r1.x, cb0[19].x
    r1.x = ((r1.xxxx)+(source[19].xxxx)).x;
    // 39: mad r2.x, cb0[17].w, r0.w, r1.x
    r2.x = ((source[17].wwww)*(r0.wwww)+(r1.xxxx)).x;
    // 40: mad r2.y, r1.x, cb0[19].y, r0.w
    r2.y = ((r1.xxxx)*(source[19].yyyy)+(r0.wwww)).y;
    // 41: mad r1.xy, cb0[19].zzzz, r0.xyxx, r2.xyxx
    r1.xy = ((source[19].zzzz)*(r0.xyxx)+(r2.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s4, l(0.000000)
    r0.w = (ALTVNativeSample4((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 43: mul r0.w, r0.w, cb0[19].w
    r0.w = ((r0.wwww)*(source[19].wwww)).w;
    // 44: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 45: mad r0.w, v4.x, cb0[20].y, cb0[20].z
    r0.w = ((v4.xxxx)*(source[20].yyyy)+(source[20].zzzz)).w;
    // 46: mad r0.w, v2.y, cb0[20].x, r0.w
    r0.w = ((v2.yyyy)*(source[20].xxxx)+(r0.wwww)).w;
    // 47: mad r1.x, v4.x, cb0[21].x, cb0[21].y
    r1.x = ((v4.xxxx)*(source[21].xxxx)+(source[21].yyyy)).x;
    // 48: mad r1.x, cb0[21].z, v2.x, r1.x
    r1.x = ((source[21].zzzz)*(v2.xxxx)+(r1.xxxx)).x;
    // 49: mad r2.x, cb0[20].w, r0.w, r1.x
    r2.x = ((source[20].wwww)*(r0.wwww)+(r1.xxxx)).x;
    // 50: mad r2.y, r1.x, cb0[21].w, r0.w
    r2.y = ((r1.xxxx)*(source[21].wwww)+(r0.wwww)).y;
    // 51: mad r1.xy, cb0[22].xxxx, r0.xyxx, r2.xyxx
    r1.xy = ((source[22].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t4.yzwx, s5, l(0.000000)
    r0.w = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 53: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 54: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 55: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 56: mul r0.w, v4.w, cb0[22].y
    r0.w = ((v4.wwww)*(source[22].yyyy)).w;
    // 57: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 58: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 59: mul_sat r0.z, r0.z, cb0[22].z
    r0.z = (saturate((r0.zzzz)*(source[22].zzzz))).z;
    // 60: mul_sat r0.z, r0.z, v3.w
    r0.z = (saturate((r0.zzzz)*(v3.wwww))).z;
    // 61: mul o0.w, r0.z, cb0[0].x
    output.w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 62: mad r0.z, v4.x, cb0[2].y, cb0[2].z
    r0.z = ((v4.xxxx)*(source[2].yyyy)+(source[2].zzzz)).z;
    // 63: mad r0.z, v2.y, cb0[2].x, r0.z
    r0.z = ((v2.yyyy)*(source[2].xxxx)+(r0.zzzz)).z;
    // 64: add r0.z, r0.z, cb0[3].y
    r0.z = ((r0.zzzz)+(source[3].yyyy)).z;
    // 65: mad r0.w, v4.x, cb0[4].x, cb0[4].y
    r0.w = ((v4.xxxx)*(source[4].xxxx)+(source[4].yyyy)).w;
    // 66: mad r0.w, cb0[4].z, v2.x, r0.w
    r0.w = ((source[4].zzzz)*(v2.xxxx)+(r0.wwww)).w;
    // 67: add r0.w, r0.w, cb0[4].w
    r0.w = ((r0.wwww)+(source[4].wwww)).w;
    // 68: mad r1.x, cb0[3].z, r0.z, r0.w
    r1.x = ((source[3].zzzz)*(r0.zzzz)+(r0.wwww)).x;
    // 69: mad r1.y, r0.w, cb0[5].x, r0.z
    r1.y = ((r0.wwww)*(source[5].xxxx)+(r0.zzzz)).y;
    // 70: mad r0.xy, cb0[11].wwww, r0.xyxx, r1.xyxx
    r0.xy = ((source[11].wwww)*(r0.xyxx)+(r1.xyxx)).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s2, l(0.000000)
    r0.xyz = (ALTVNativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 72: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 74: mad r0.xyz, cb0[12].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[12].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 75: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 76: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 77: mul r0.xyz, r0.xyzx, cb0[12].yyyy
    r0.xyz = ((r0.xyzx)*(source[12].yyyy)).xyz;
    // 78: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 79: mad r0.xyz, cb0[12].zzzz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(source[12].wwww)).xyz;
    // 80: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_o_me_splitline_02_tr: b5aa37584aae4f4daa270f6938d91812; selected map b32027d5745f5cc77d4653ac99a0b8b93bab7c02ccc0a1d27c82284459df9c4e.
float4 ALTVNative177(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[0u];
    source[3] = ALTVNativeAppend(ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0500000007, 0.0, 0.0, 0.0))),ALTVNativePeriodic((g_ALTVSourceMaterialTime.xxxx*float4(0.0, 0.0, 0.0, 0.0))),1u);
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
    // 10: mad r1.xy, v4.xyxx, l(0.000000, 0.700000, 0.000000, 0.000000), cb0[3].xyxx
    r1.xy = ((v4.xyxx)*(float4(0.000000,0.700000,0.000000,0.000000))+(source[3].xyxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s1, l(0.000000)
    r0.w = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 12: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 14: mul r0.w, r0.w, v4.y
    r0.w = ((r0.wwww)*(v4.yyyy)).w;
    // 15: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 16: mul o0.w, r0.w, cb0[0].x
    output.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 17: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 18: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: add r0.w, v4.x, l(-0.500000)
    r0.w = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 20: add r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))+(abs(r0.wwww))).w;
    // 21: lt r0.w, |r0.w|, l(0.000000)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 22: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 23: mul r1.w, r1.w, l(10.000000)
    r1.w = ((r1.wwww)*(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 24: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 25: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 26: mul r1.w, r0.w, l(0.350000)
    r1.w = ((r0.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))).w;
    // 27: mad r2.xy, r0.wwww, l(0.017500, 0.017500, 0.000000, 0.000000), r1.xyxx
    r2.xy = ((r0.wwww)*(float4(0.017500,0.017500,0.000000,0.000000))+(r1.xyxx)).xy;
    // 28: mad r2.z, r0.w, l(0.035000), l(0.010000)
    r2.z = ((r0.wwww)*(float4(0.035000,0.035000,0.035000,0.035000))+(float4(0.010000,0.010000,0.010000,0.010000))).z;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t1.xyzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.xyz = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xyzw).xyz;
    // 30: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 31: mul r1.xy, r1.xyxx, l(0.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.000000,1.000000,0.000000,0.000000))).xy;
    // 32: mul r2.z, r2.z, l(0.100000)
    r2.z = ((r2.zzzz)*(float4(0.100000,0.100000,0.100000,0.100000))).z;
    // 33: mov r4.xy, r2.yxyy
    r4.xy = (r2.yxyy).xy;
    // 34: mov r5.x, r3.x
    r5.x = (r3.xxxx).x;
    // 35: mov r5.y, l(0)
    r5.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 36: loop
    [loop] while (true) {
    // 37: ge r2.w, r5.y, l(3.000000)
    r2.w = (asfloat((uint4)((r5.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 38: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 39: mad r4.xy, -r1.yxyy, r2.zzzz, r4.xyxx
    r4.xy = ((-(r1.yxyy))*(r2.zzzz)+(r4.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r4.yxyy, t1.yzwx, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r4.yxyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yzwx).w;
    // 41: add r5.x, r2.w, r5.x
    r5.x = ((r2.wwww)+(r5.xxxx)).x;
    // 42: add r5.y, r5.y, l(1.000000)
    r5.y = ((r5.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 43: endloop
    }
    // 44: mov r2.w, r2.x
    r2.w = (r2.xxxx).w;
    // 45: mov r2.y, r4.x
    r2.y = (r4.xxxx).y;
    // 46: mov r6.x, r3.y
    r6.x = (r3.yyyy).x;
    // 47: mov r6.y, l(0)
    r6.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 48: loop
    [loop] while (true) {
    // 49: ge r3.x, r6.y, l(3.000000)
    r3.x = (asfloat((uint4)((r6.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).x;
    // 50: breakc_nz r3.x
    if ((asuint(r3.xxxx)).x != 0u) break;
    // 51: mad r2.yw, -r1.yyyx, r2.zzzz, r2.yyyw
    r2.yw = ((-(r1.yyyx))*(r2.zzzz)+(r2.yyyw)).yw;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r2.wyww, t1.yxzw, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r3.x = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r2.wyww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).yxzw).x;
    // 53: add r6.x, r3.x, r6.x
    r6.x = ((r3.xxxx)+(r6.xxxx)).x;
    // 54: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 55: endloop
    }
    // 56: mov r5.y, r6.x
    r5.y = (r6.xxxx).y;
    // 57: mov r3.xy, r2.xyxx
    r3.xy = (r2.xyxx).xy;
    // 58: mov r4.x, r3.z
    r4.x = (r3.zzzz).x;
    // 59: mov r4.y, l(0)
    r4.y = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).y;
    // 60: loop
    [loop] while (true) {
    // 61: ge r2.w, r4.y, l(3.000000)
    r2.w = (asfloat((uint4)((r4.yyyy)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 62: breakc_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) break;
    // 63: mad r3.xy, -r1.xyxx, r2.zzzz, r3.xyxx
    r3.xy = ((-(r1.xyxx))*(r2.zzzz)+(r3.xyxx)).xy;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t1.xywz, s0, l(0.000000) (project resolved HDR SceneColor snapshot adapter)
    r2.w = (g_EffectSceneColorTexture.SampleBias(LinearClampUVSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x).xywz).w;
    // 65: add r4.x, r2.w, r4.x
    r4.x = ((r2.wwww)+(r4.xxxx)).x;
    // 66: add r4.y, r4.y, l(1.000000)
    r4.y = ((r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: endloop
    }
    // 68: mov r5.z, r4.x
    r5.z = (r4.xxxx).z;
    // 69: mul r2.xyz, r5.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r2.xyz = ((r5.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 70: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 71: add r1.y, -v4.y, l(1.000000)
    r1.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 72: lt r2.x, |r1.y|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 73: log r1.y, |r1.y|
    r1.y = (log2(abs(r1.yyyy))).y;
    // 74: mul r1.y, r1.y, l(15.000000)
    r1.y = ((r1.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 75: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 76: movc r1.y, r2.x, l(0), r1.y
    r1.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 77: lt r2.x, |v4.y|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(v4.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 78: log r2.y, |v4.y|
    r2.y = (log2(abs(v4.yyyy))).y;
    // 79: mul r2.y, r2.y, l(15.000000)
    r2.y = ((r2.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 80: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 81: movc r2.y, r2.x, l(0), r2.y
    r2.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 82: add r1.y, r1.y, r2.y
    r1.y = ((r1.yyyy)+(r2.yyyy)).y;
    // 83: add r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)+(r1.yyyy)).w;
    // 84: lt r1.y, r0.w, l(0.000001)
    r1.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 86: movc r0.w, r1.y, l(0), r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 87: mul r1.y, |v4.y|, |v4.y|
    r1.y = ((abs(v4.yyyy))*(abs(v4.yyyy))).y;
    // 88: mul r1.y, r1.y, |v4.y|
    r1.y = ((r1.yyyy)*(abs(v4.yyyy))).y;
    // 89: movc r1.y, r2.x, l(0), r1.y
    r1.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 90: mad r0.w, r1.y, r1.w, r0.w
    r0.w = ((r1.yyyy)*(r1.wwww)+(r0.wwww)).w;
    // 91: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 92: mad r1.xyw, cb0[1].xyxz, r0.wwww, cb0[2].xyxz
    r1.xyw = ((source[1].xyxz)*(r0.wwww)+(source[2].xyxz)).xyw;
    // 93: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_m_me_swp_boxlinelight_colortex_01_01_ts: d16014b14d2fd446a90efaeceed55c9a; selected map a8d9c40af058c7a14d9a0d6d89dd0114dcdab8bd29569494c621aa4babbd3c80.
float4 ALTVNative179(ALTV_NATIVE_INPUT input)
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

// fx_m_me_swp_boxlinelight_colortex_02_01_ts: d16014b14d2fd446a90efaeceed55c9a; selected map a8d9c40af058c7a14d9a0d6d89dd0114dcdab8bd29569494c621aa4babbd3c80.
float4 ALTVNative180(ALTV_NATIVE_INPUT input)
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

// fx_m_swp_boxedge_01_ad: abf0b682be3dc74497b60c7c4b61bd20; selected map 2c0f25c6558cad33d5fc7046e05eb0d0c57ab13c3f9d1f5fb4ba41127693cf29.
float4 ALTVNative182(ALTV_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_ALTVSourceMaterialParameters[14u];
    source[3] = input.dynamicParameter;
    source[4].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[4].y = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[4].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[4].w = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[5].x = (g_ALTVSourceMaterialTime.xxxx).x;
    source[5].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].xxxx)).x;
    source[5].z = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[5].w = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[6].x = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[6].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[6].w = ((g_ALTVSourceMaterialParameters[5u].xxxx*g_ALTVSourceMaterialTime.xxxx)).x;
    source[7].x = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[7].w = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[8].y = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[6u].yyyy)).x;
    source[8].z = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[8].w = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[9].x = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[9].w = ((g_ALTVSourceMaterialParameters[5u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[10].x = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[10].y = (g_ALTVSourceMaterialParameters[11u].zzzz).x;
    source[10].z = (g_ALTVSourceMaterialParameters[11u].xxxx).x;
    source[10].w = (g_ALTVSourceMaterialParameters[11u].yyyy).x;
    source[11].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[11].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[11].z = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[12].z = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[12].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[13].w = (g_ALTVSourceMaterialParameters[11u].wwww).x;
    source[14].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[14].y = (g_ALTVSourceMaterialParameters[9u].wwww).x;
    source[14].z = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[14].w = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[15].x = ((g_ALTVSourceMaterialTime.xxxx*g_ALTVSourceMaterialParameters[5u].wwww)).x;
    source[15].y = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[15].z = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[15].w = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[16].x = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[16].y = ((g_ALTVSourceMaterialParameters[5u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[16].z = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[16].w = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[17].x = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[17].y = (g_ALTVSourceMaterialParameters[12u].xxxx).x;
    source[17].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[17].w = (g_ALTVSourceMaterialParameters[3u].wwww).x;
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
    // 4: mad r0.y, cb0[3].y, cb0[8].w, cb0[9].x
    r0.y = ((source[3].yyyy)*(source[8].wwww)+(source[9].xxxx)).y;
    // 5: mad r0.y, cb0[9].y, v4.x, r0.y
    r0.y = ((source[9].yyyy)*(v4.xxxx)+(r0.yyyy)).y;
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
    // 12: mad r0.z, cb0[3].x, cb0[14].y, cb0[14].z
    r0.z = ((source[3].xxxx)*(source[14].yyyy)+(source[14].zzzz)).z;
    // 13: add r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)+(source[15].xxxx)).z;
    // 14: mad r0.z, v4.y, cb0[14].x, r0.z
    r0.z = ((v4.yyyy)*(source[14].xxxx)+(r0.zzzz)).z;
    // 15: mad r0.w, cb0[3].x, cb0[15].w, cb0[16].x
    r0.w = ((source[3].xxxx)*(source[15].wwww)+(source[16].xxxx)).w;
    // 16: add r0.w, r0.w, cb0[16].y
    r0.w = ((r0.wwww)+(source[16].yyyy)).w;
    // 17: mad r0.w, cb0[16].z, v4.x, r0.w
    r0.w = ((source[16].zzzz)*(v4.xxxx)+(r0.wwww)).w;
    // 18: mad r1.x, cb0[15].y, r0.z, r0.w
    r1.x = ((source[15].yyyy)*(r0.zzzz)+(r0.wwww)).x;
    // 19: mad r1.y, r0.w, cb0[16].w, r0.z
    r1.y = ((r0.wwww)*(source[16].wwww)+(r0.zzzz)).y;
    // 20: mad r0.zw, cb0[17].xxxx, r0.xxxy, r1.xxxy
    r0.zw = ((source[17].xxxx)*(r0.xxxy)+(r1.xxxy)).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s3, l(0.000000)
    r0.z = (ALTVNativeSample3((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 22: mul r0.z, r0.z, cb0[17].y
    r0.z = ((r0.zzzz)*(source[17].yyyy)).z;
    // 23: mad r0.w, cb0[3].x, cb0[11].w, cb0[12].x
    r0.w = ((source[3].xxxx)*(source[11].wwww)+(source[12].xxxx)).w;
    // 24: mad r0.w, v4.y, cb0[11].z, r0.w
    r0.w = ((v4.yyyy)*(source[11].zzzz)+(r0.wwww)).w;
    // 25: mad r1.x, cb0[3].x, cb0[12].z, cb0[12].w
    r1.x = ((source[3].xxxx)*(source[12].zzzz)+(source[12].wwww)).x;
    // 26: mad r1.x, cb0[13].x, v4.x, r1.x
    r1.x = ((source[13].xxxx)*(v4.xxxx)+(r1.xxxx)).x;
    // 27: mad r2.x, cb0[12].y, r0.w, r1.x
    r2.x = ((source[12].yyyy)*(r0.wwww)+(r1.xxxx)).x;
    // 28: mad r2.y, r1.x, cb0[13].y, r0.w
    r2.y = ((r1.xxxx)*(source[13].yyyy)+(r0.wwww)).y;
    // 29: mad r1.xy, cb0[13].zzzz, r0.xyxx, r2.xyxx
    r1.xy = ((source[13].zzzz)*(r0.xyxx)+(r2.xyxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s2, l(0.000000)
    r0.w = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 31: mul r0.w, r0.w, cb0[13].w
    r0.w = ((r0.wwww)*(source[13].wwww)).w;
    // 32: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 33: max r0.z, |r0.z|, l(0.000001)
    r0.z = (max(abs(r0.zzzz),float4(0.000001,0.000001,0.000001,0.000001))).z;
    // 34: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 35: mul r0.w, cb0[3].w, cb0[17].z
    r0.w = ((source[3].wwww)*(source[17].zzzz)).w;
    // 36: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 37: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 38: mul_sat r0.z, r0.z, cb0[17].w
    r0.z = (saturate((r0.zzzz)*(source[17].wwww))).z;
    // 39: mul_sat r0.z, r0.z, cb0[1].w
    r0.z = (saturate((r0.zzzz)*(source[1].wwww))).z;
    // 40: mul r0.z, r0.z, cb0[0].x
    r0.z = ((r0.zzzz)*(source[0].xxxx)).z;
    // 41: mad r0.w, cb0[3].x, cb0[4].y, cb0[4].z
    r0.w = ((source[3].xxxx)*(source[4].yyyy)+(source[4].zzzz)).w;
    // 42: mad r0.w, v4.y, cb0[4].x, r0.w
    r0.w = ((v4.yyyy)*(source[4].xxxx)+(r0.wwww)).w;
    // 43: add r0.w, r0.w, cb0[5].y
    r0.w = ((r0.wwww)+(source[5].yyyy)).w;
    // 44: mad r1.x, cb0[3].x, cb0[6].x, cb0[6].y
    r1.x = ((source[3].xxxx)*(source[6].xxxx)+(source[6].yyyy)).x;
    // 45: mad r1.x, cb0[6].z, v4.x, r1.x
    r1.x = ((source[6].zzzz)*(v4.xxxx)+(r1.xxxx)).x;
    // 46: add r1.x, r1.x, cb0[6].w
    r1.x = ((r1.xxxx)+(source[6].wwww)).x;
    // 47: mad r2.x, cb0[5].z, r0.w, r1.x
    r2.x = ((source[5].zzzz)*(r0.wwww)+(r1.xxxx)).x;
    // 48: mad r2.y, r1.x, cb0[7].x, r0.w
    r2.y = ((r1.xxxx)*(source[7].xxxx)+(r0.wwww)).y;
    // 49: mad r0.xy, cb0[10].zzzz, r0.xyxx, r2.xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(r2.xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t3.xywz, s1, l(0.000000)
    r0.xyw = (ALTVNativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xywz).xyw;
    // 51: dp3 r1.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 52: add r1.xyz, -r0.xywx, r1.xxxx
    r1.xyz = ((-(r0.xywx))+(r1.xxxx)).xyz;
    // 53: mad r0.xyw, cb0[10].wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((source[10].wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 54: max r0.xyw, |r0.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r0.xyw = (max(abs(r0.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 55: log r0.xyw, r0.xyxw
    r0.xyw = (log2(r0.xyxw)).xyw;
    // 56: mul r0.xyw, r0.xyxw, cb0[11].xxxx
    r0.xyw = ((r0.xyxw)*(source[11].xxxx)).xyw;
    // 57: exp r0.xyw, r0.xyxw
    r0.xyw = (exp2(r0.xyxw)).xyw;
    // 58: mul r0.xyw, r0.xyxw, cb0[11].yyyy
    r0.xyw = ((r0.xyxw)*(source[11].yyyy)).xyw;
    // 59: mad r0.xyw, r0.xyxw, cb0[1].xyxz, cb0[2].xyxz
    r0.xyw = ((r0.xyxw)*(source[1].xyxz)+(source[2].xyxz)).xyw;
    // 60: mul r0.xyw, r0.xyxw, v5.wwww
    r0.xyw = ((r0.xyxw)*(v5.wwww)).xyw;
    // 61: mul o0.xyz, r0.zzzz, r0.xywx
    output.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 62: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_m_swp_boxcrack_basemaster_01-2_ad: 1fe38ae0c402e44bb0a9abf699e1096d; selected map 0961a5d1989f4fb20981d70a1a6d4783c919f5808ceeab25c0242a2c7666efd0.
float4 ALTVNative183(ALTV_NATIVE_INPUT input)
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

// fx_m_me_boxlinelight_02_01_ts_ad: d16014b14d2fd446a90efaeceed55c9a; selected map a8d9c40af058c7a14d9a0d6d89dd0114dcdab8bd29569494c621aa4babbd3c80.
float4 ALTVNative184(ALTV_NATIVE_INPUT input)
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

// fx_a_pa_db_01_1_ad: 1f6b66cb17f56f4d8ec7cb004d70bd5b; selected map 43e8b055b0a689ebacafd88e6eeacb8475f50eccafe6091ef4aba92e7bdf29c2.
float4 ALTVNative185(ALTV_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[2u];
    source[2] = g_ALTVSourceMaterialParameters[1u];
    source[3].x = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (ALTVNativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (ALTVNativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
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

// fx_z_pa_grad_01_1_tr: c4e3bde2576c2b46b3dd6ac9b693f4ed; selected map 154931681653007185176db8267f10aa3331472347eae2285d7d0b44dd35daf8.
float4 ALTVNative186(ALTV_NATIVE_INPUT input)
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
    // 1: mad r0.x, cb0[2].x, v4.z, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.z, cb0[2].x
    r0.y = ((v4.zzzz)*(source[2].xxxx)).y;
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

// fx_m_pa_swp_camglass_01_tr: d702b8ba23179f41abc4e2a152785415; selected map 0ba5180f3362847b42597c7b2510acbff9224499ba2b73010de3f1f009510742.
float4 ALTVNative187(ALTV_NATIVE_INPUT input)
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
    // 60: mad r1.y, v4.x, cb0[16].w, cb0[17].x
    r1.y = ((v4.xxxx)*(source[16].wwww)+(source[17].xxxx)).y;
    // 61: add r1.y, r1.y, cb0[17].z
    r1.y = ((r1.yyyy)+(source[17].zzzz)).y;
    // 62: mad r1.y, v2.y, cb0[16].z, r1.y
    r1.y = ((v2.yyyy)*(source[16].zzzz)+(r1.yyyy)).y;
    // 63: mad r1.z, v4.x, cb0[18].y, cb0[18].z
    r1.z = ((v4.xxxx)*(source[18].yyyy)+(source[18].zzzz)).z;
    // 64: add r1.z, r1.z, cb0[18].w
    r1.z = ((r1.zzzz)+(source[18].wwww)).z;
    // 65: mad r1.z, cb0[19].x, v2.x, r1.z
    r1.z = ((source[19].xxxx)*(v2.xxxx)+(r1.zzzz)).z;
    // 66: mad r2.x, cb0[17].w, r1.y, r1.z
    r2.x = ((source[17].wwww)*(r1.yyyy)+(r1.zzzz)).x;
    // 67: mad r2.y, r1.z, cb0[19].y, r1.y
    r2.y = ((r1.zzzz)*(source[19].yyyy)+(r1.yyyy)).y;
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

// fx_j_pa_slice_01_03_tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 ALTVNative188(ALTV_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[3u];
    source[2] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[6].zzwz, cb0[7].xxyx
    r0.yz = ((r0.yyzy)*(source[6].zzwz)+(source[7].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v4.z, cb0[7].z
    r0.z = ((v4.zzzz)*(source[7].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v2.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v2.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[4].xyxx, r0.yzyy
    r0.w = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[5].xyxx, r0.yzyy
    r0.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[8].x, l(10.000000), l(10.000000)
    r0.w = ((source[8].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 25: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 30: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 37: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 38: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 40: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 41: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 44: mad r0.xz, v2.xxyx, l(-1.000000, 0.000000, 1.000000, 0.000000), l(1.000000, 0.000000, 0.000000, 0.000000)
    r0.xz = ((v2.xxyx)*(float4(-1.000000,0.000000,1.000000,0.000000))+(float4(1.000000,0.000000,0.000000,0.000000))).xz;
    // 45: mad r0.xz, r0.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 46: dp2 r0.x, r0.xzxx, r0.xzxx
    r0.x = (dot((r0.xzxx).xy,(r0.xzxx).xy).xxxx).x;
    // 47: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 48: add r0.x, r0.x, -v4.x
    r0.x = ((r0.xxxx)+(-(v4.xxxx))).x;
    // 49: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 50: div r0.x, l(0.100000), |r0.x|
    r0.x = ((float4(0.100000,0.100000,0.100000,0.100000))/(abs(r0.xxxx))).x;
    // 51: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_pa_linearwave_03_05_tr: 2ff7fcbd8473174495860fbbfd468350; selected map 65e100ae3110fca8065b53b45f332bb745a722031e412433b290c7dd63bd2651.
float4 ALTVNative189(ALTV_NATIVE_INPUT input)
{
    float4 source[22]; [unroll] for (uint i=0u; i<22u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[12u];
    source[2] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = g_ALTVSourceMaterialParameters[11u];
    source[7].x = (g_ALTVSourceMaterialParameters[9u].yyyy).x;
    source[7].y = (g_ALTVSourceMaterialParameters[9u].zzzz).x;
    source[7].z = (g_ALTVSourceMaterialTime.xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[8u].yyyy).x;
    source[8].x = ((g_ALTVSourceMaterialParameters[8u].yyyy*g_ALTVSourceMaterialTime.xxxx)).x;
    source[8].y = (((g_ALTVSourceMaterialParameters[8u].yyyy*g_ALTVSourceMaterialTime.xxxx)+float4(1.0, 0.0, 0.0, 0.0))).x;
    source[8].z = (g_ALTVSourceMaterialParameters[8u].zzzz).x;
    source[8].w = ((g_ALTVSourceMaterialParameters[8u].zzzz*g_ALTVSourceMaterialTime.xxxx)).x;
    source[9].x = (((g_ALTVSourceMaterialParameters[8u].zzzz*g_ALTVSourceMaterialTime.xxxx)+float4(1.0, 0.0, 0.0, 0.0))).x;
    source[9].y = (g_ALTVSourceMaterialParameters[8u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[9].w = ((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[10].x = (sin((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[10].z = (cos((g_ALTVSourceMaterialParameters[2u].wwww*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[10].w = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[11].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[11].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[11].z = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[11].w = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[12].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[12].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[12].z = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[12].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[13].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[13].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[13].w = (floor(g_ALTVSourceMaterialParameters[3u].wwww)).x;
    source[14].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[14].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[14].z = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[14].w = ((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[15].x = (sin((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[15].z = (cos((g_ALTVSourceMaterialParameters[6u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[15].w = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[16].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[16].y = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
    source[16].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[16].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[17].x = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[17].y = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[17].z = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[17].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[18].x = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[18].y = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[18].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[18].w = (floor(g_ALTVSourceMaterialParameters[0u].xxxx)).x;
    source[19].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[19].y = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[19].z = (g_ALTVSourceMaterialParameters[9u].xxxx).x;
    source[19].w = (g_ALTVSourceMaterialParameters[8u].wwww).x;
    source[20].x = (g_ALTVSourceMaterialParameters[10u].zzzz).x;
    source[20].y = (g_ALTVSourceMaterialParameters[10u].wwww).x;
    source[20].z = (g_ALTVSourceMaterialParameters[10u].yyyy).x;
    source[20].w = (g_ALTVSourceMaterialParameters[10u].xxxx).x;
    source[21].x = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[21].y = (g_ALTVSourceMaterialParameters[7u].wwww).x;
    source[21].z = (g_ALTVSourceMaterialParameters[9u].wwww).x;
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
    // 32: mad r0.zw, cb0[10].wwww, r0.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((source[10].wwww)*(r0.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 33: mad r0.xy, cb0[15].wwww, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((source[15].wwww)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 34: add r1.x, v4.x, cb0[11].w
    r1.x = ((v4.xxxx)+(source[11].wwww)).x;
    // 35: mad r1.y, r0.w, cb0[11].y, r1.x
    r1.y = ((r0.wwww)*(source[11].yyyy)+(r1.xxxx)).y;
    // 36: mad r1.x, r0.z, cb0[11].x, cb0[11].z
    r1.x = ((r0.zzzz)*(source[11].xxxx)+(source[11].zzzz)).x;
    // 37: add r0.zw, r1.xxxy, l(0.000000, 0.000000, -0.500000, -1.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,-0.500000,-1.500000))).zw;
    // 38: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 39: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 40: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 41: mul r1.x, v4.w, cb0[12].z
    r1.x = ((v4.wwww)*(source[12].zzzz)).x;
    // 42: mul r2.xyz, v2.xyxx, cb0[12].xywx
    r2.xyz = ((v2.xyxx)*(source[12].xywx)).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r2.xyxx, t0.zxyw, s3, l(0.000000)
    r1.yz = (ALTVNativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zxyw).yz;
    // 44: mad r0.zw, r1.xxxx, r1.yyyz, r0.zzzw
    r0.zw = ((r1.xxxx)*(r1.yyyz)+(r0.zzzw)).zw;
    // 45: mul r2.w, v2.y, cb0[13].x
    r2.w = ((v2.yyyy)*(source[13].xxxx)).w;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.zwzz, t1.xyzw, s4, l(0.000000)
    r1.xy = (ALTVNativeSample3((r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 47: mad r0.zw, cb0[13].yyyy, r1.xxxy, r0.zzzw
    r0.zw = ((source[13].yyyy)*(r1.xxxy)+(r0.zzzw)).zw;
    // 48: sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s2, l(-1.000000)
    r0.z = (ALTVNativeSample1((r0.zwzz).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).yzxw).z;
    // 49: mad r0.z, cb0[13].w, -r0.z, r0.z
    r0.z = ((source[13].wwww)*(-(r0.zzzz))+(r0.zzzz)).z;
    // 50: mul r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)*(source[14].xxxx)).z;
    // 51: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 52: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 53: mul r0.w, r0.w, cb0[14].y
    r0.w = ((r0.wwww)*(source[14].yyyy)).w;
    // 54: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 55: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 56: add r0.w, v4.y, cb0[16].w
    r0.w = ((v4.yyyy)+(source[16].wwww)).w;
    // 57: mad r1.y, r0.y, cb0[16].y, r0.w
    r1.y = ((r0.yyyy)*(source[16].yyyy)+(r0.wwww)).y;
    // 58: mad r1.x, r0.x, cb0[16].x, cb0[16].z
    r1.x = ((r0.xxxx)*(source[16].xxxx)+(source[16].zzzz)).x;
    // 59: add r0.xy, r1.xyxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 60: dp2 r1.x, cb0[4].xyxx, r0.xyxx
    r1.x = (dot((source[4].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 61: dp2 r1.y, cb0[5].xyxx, r0.xyxx
    r1.y = (dot((source[5].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 62: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 63: mul r1.xyz, v2.xyxx, cb0[17].xywx
    r1.xyz = ((v2.xyxx)*(source[17].xywx)).xyz;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t3.xyzw, s6, l(0.000000)
    r1.xy = (ALTVNativeSample5((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 65: mad r0.xy, cb0[17].zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((source[17].zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 66: mul r1.w, v2.y, cb0[18].x
    r1.w = ((v2.yyyy)*(source[18].xxxx)).w;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.zwzz, t4.xyzw, s7, l(0.000000)
    r1.xy = (ALTVNativeSample6((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 68: mad r0.xy, cb0[18].yyyy, r1.xyxx, r0.xyxx
    r0.xy = ((source[18].yyyy)*(r1.xyxx)+(r0.xyxx)).xy;
    // 69: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s5, l(-1.000000)
    r0.x = (ALTVNativeSample4((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 70: mad r0.x, cb0[18].w, -r0.x, r0.x
    r0.x = ((source[18].wwww)*(-(r0.xxxx))+(r0.xxxx)).x;
    // 71: mul r0.x, r0.x, cb0[19].x
    r0.x = ((r0.xxxx)*(source[19].xxxx)).x;
    // 72: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 73: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 74: mul r0.x, r0.x, cb0[19].y
    r0.x = ((r0.xxxx)*(source[19].yyyy)).x;
    // 75: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 76: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 77: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 78: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 79: mul r0.y, r0.y, cb0[20].x
    r0.y = ((r0.yyyy)*(source[20].xxxx)).y;
    // 80: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 81: mul r0.y, r0.y, cb0[20].y
    r0.y = ((r0.yyyy)*(source[20].yyyy)).y;
    // 82: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 83: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 84: add r0.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 85: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 86: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 87: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 88: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 89: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 90: mad r1.x, cb0[20].z, l(10.000000), l(10.000000)
    r1.x = ((source[20].zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).x;
    // 91: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 92: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 93: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 94: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mul r0.z, r0.z, cb0[20].w
    r0.z = ((r0.zzzz)*(source[20].wwww)).z;
    // 96: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 97: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 98: mul r0.z, r0.z, cb0[21].x
    r0.z = ((r0.zzzz)*(source[21].xxxx)).z;
    // 99: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 100: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 101: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 102: source device depth mapped to centimetre view depth; reconstruction at 104.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 104-107: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 108: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 109: add r0.w, -cb0[21].y, l(1.000000)
    r0.w = ((-(source[21].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 111: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 112: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 113: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 114: mul r0.z, v3.w, v3.w
    r0.z = ((v3.wwww)*(v3.wwww)).z;
    // 115: mul_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)*(r0.yyyy))).y;
    // 116: mul o0.w, r0.y, cb0[0].x
    output.w = ((r0.yyyy)*(source[0].xxxx)).w;
    // 117: mad r1.x, v2.x, cb0[7].x, cb0[8].y
    r1.x = ((v2.xxxx)*(source[7].xxxx)+(source[8].yyyy)).x;
    // 118: mad r1.y, v2.y, cb0[7].y, cb0[9].x
    r1.y = ((v2.yyyy)*(source[7].yyyy)+(source[9].xxxx)).y;
    // 119: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t7.wxyz, s1, l(-1.000000)
    r0.yzw = (ALTVNativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 120: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 122: mad r0.yzw, cb0[9].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[9].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 123: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 124: mul r0.xyz, r0.xyzx, cb0[19].zzzz
    r0.xyz = ((r0.xyzx)*(source[19].zzzz)).xyz;
    // 125: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 126: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 127: mul r0.xyz, r0.xyzx, cb0[19].wwww
    r0.xyz = ((r0.xyzx)*(source[19].wwww)).xyz;
    // 128: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 129: mul_sat r0.xyz, r0.xyzx, cb0[6].xyzx
    r0.xyz = (saturate((r0.xyzx)*(source[6].xyzx))).xyz;
    // 130: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 131: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

// fx_j_pa_rotturbulance_01_08_ad: 4fe721e700949f4891b94dffc5f19da5; selected map d00414bb63739d4321c2bed6aa422075f5e8e97c589072244f2e0c129ea78639.
float4 ALTVNative190(ALTV_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[8u];
    source[2].x = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[2].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[2].z = (g_ALTVSourceMaterialParameters[5u].xxxx).x;
    source[2].w = (g_ALTVSourceMaterialParameters[5u].yyyy).x;
    source[3].x = (g_ALTVSourceMaterialParameters[4u].zzzz).x;
    source[3].y = (g_ALTVSourceMaterialParameters[4u].wwww).x;
    source[3].z = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[3].w = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[4].x = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[4].y = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[4].z = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[4].w = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[5].x = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[5].y = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[5].z = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[5].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[6].x = (g_ALTVSourceMaterialParameters[7u].xxxx).x;
    source[6].y = (g_ALTVSourceMaterialParameters[6u].wwww).x;
    source[6].z = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[7].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[7].y = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[7].w = (g_ALTVSourceMaterialParameters[6u].yyyy).x;
    source[8].x = (g_ALTVSourceMaterialParameters[6u].zzzz).x;
    source[8].y = (g_ALTVSourceMaterialParameters[5u].wwww).x;
    source[8].z = (g_ALTVSourceMaterialParameters[6u].xxxx).x;
    source[8].w = (g_ALTVSourceMaterialParameters[5u].zzzz).x;
    source[9].x = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[9].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[9].z = (g_ALTVSourceMaterialParameters[7u].zzzz).x;
    source[9].w = (g_ALTVSourceMaterialParameters[7u].yyyy).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: mul r0.x, v2.x, cb0[7].w
    r0.x = ((v2.xxxx)*(source[7].wwww)).x;
    // 2: mul r0.y, v2.y, cb0[8].x
    r0.y = ((v2.yyyy)*(source[8].xxxx)).y;
    // 3: add r0.xy, r0.xyxx, cb0[8].yzyy
    r0.xy = ((r0.xyxx)+(source[8].yzyy)).xy;
    // 4: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.zw, v4.xxxx, l(0.000000, 0.000000, 0.250000, -0.100000)
    r0.zw = ((v4.xxxx)*(float4(0.000000,0.000000,0.250000,-0.100000))).zw;
    // 6: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 7: sincos r3.x, r4.x, r0.z
    r3.x = (sin(r0.zzzz)).x; r4.x = (cos(r0.zzzz)).x;
    // 8: mov r5.x, -r1.x
    r5.x = (-(r1.xxxx)).x;
    // 9: mov r5.y, r2.x
    r5.y = (r2.xxxx).y;
    // 10: mov r5.z, r1.x
    r5.z = (r1.xxxx).z;
    // 11: dp2 r1.y, r5.zyzz, r0.xyxx
    r1.y = (dot((r5.zyzz).xy,(r0.xyxx).xy).xxxx).y;
    // 12: dp2 r1.x, r5.yxyy, r0.xyxx
    r1.x = (dot((r5.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 13: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t6.xyzw, s5, l(0.000000)
    r0.x = (ALTVNativeSample4((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 15: mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // 16: mul r0.y, v2.y, cb0[6].w
    r0.y = ((v2.yyyy)*(source[6].wwww)).y;
    // 17: mad r1.y, cb0[7].y, v4.y, r0.y
    r1.y = ((source[7].yyyy)*(v4.yyyy)+(r0.yyyy)).y;
    // 18: mad r1.x, v2.x, cb0[6].z, cb0[7].x
    r1.x = ((v2.xxxx)*(source[6].zzzz)+(source[7].xxxx)).x;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r1.xyxx, t5.yxzw, s4, l(0.000000)
    r0.y = (ALTVNativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 20: mad r0.x, r0.y, cb0[7].z, r0.x
    r0.x = ((r0.yyyy)*(source[7].zzzz)+(r0.xxxx)).x;
    // 21: add r0.xy, r0.xxxx, v2.xyxx
    r0.xy = ((r0.xxxx)+(v2.xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t7.xyzw, s6, l(0.000000)
    r0.x = (ALTVNativeSample5((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 23: mul r0.x, r0.x, cb0[9].x
    r0.x = ((r0.xxxx)*(source[9].xxxx)).x;
    // 24: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 25: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 26: mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // 27: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 28: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 29: mov r1.x, -r3.x
    r1.x = (-(r3.xxxx)).x;
    // 30: mad r0.yz, v2.xxyx, cb0[5].xxyx, cb0[5].zzwz
    r0.yz = ((v2.xxyx)*(source[5].xxyx)+(source[5].zzwz)).yz;
    // 31: add r0.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 32: mov r1.y, r4.x
    r1.y = (r4.xxxx).y;
    // 33: mov r1.z, r3.x
    r1.z = (r3.xxxx).z;
    // 34: dp2 r2.y, r1.zyzz, r0.yzyy
    r2.y = (dot((r1.zyzz).xy,(r0.yzyy).xy).xxxx).y;
    // 35: dp2 r2.x, r1.yxyy, r0.yzyy
    r2.x = (dot((r1.yxyy).xy,(r0.yzyy).xy).xxxx).x;
    // 36: add r0.yz, r2.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t4.yxzw, s3, l(0.000000)
    r0.y = (ALTVNativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 38: mul r0.y, r0.y, cb0[6].x
    r0.y = ((r0.yyyy)*(source[6].xxxx)).y;
    // 39: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 40: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 41: mul r0.y, r0.y, cb0[6].y
    r0.y = ((r0.yyyy)*(source[6].yyyy)).y;
    // 42: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 43: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 44: mul r0.z, r0.x, r0.y
    r0.z = ((r0.xxxx)*(r0.yyyy)).z;
    // 45: mul r1.xy, v2.xyxx, cb0[2].zwzz
    r1.xy = ((v2.xyxx)*(source[2].zwzz)).xy;
    // 46: mad r2.x, cb0[2].y, cb0[2].x, r1.x
    r2.x = ((source[2].yyyy)*(source[2].xxxx)+(r1.xxxx)).x;
    // 47: mad r2.y, cb0[2].y, cb0[3].x, r1.y
    r2.y = ((source[2].yyyy)*(source[3].xxxx)+(r1.yyyy)).y;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t2.yzwx, s1, l(0.000000)
    r0.w = (ALTVNativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 49: mad r1.xy, r0.wwww, cb0[3].yyyy, v2.xyxx
    r1.xy = ((r0.wwww)*(source[3].yyyy)+(v2.xyxx)).xy;
    // 50: mad r1.xy, r1.xyxx, cb0[3].zwzz, cb0[4].xyxx
    r1.xy = ((r1.xyxx)*(source[3].zwzz)+(source[4].xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t3.yzwx, s2, l(0.000000)
    r0.w = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 52: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 53: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 54: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 56: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 57: mul r1.y, v4.z, cb0[4].w
    r1.y = ((v4.zzzz)*(source[4].wwww)).y;
    // 58: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 59: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 60: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 61: mul r1.x, r0.z, r0.w
    r1.x = ((r0.zzzz)*(r0.wwww)).x;
    // 62: dp3 r1.y, r1.xxxx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r1.xxxx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 63: mad r0.z, -r0.w, r0.z, r1.y
    r0.z = ((-(r0.wwww))*(r0.zzzz)+(r1.yyyy)).z;
    // 64: mad r0.z, cb0[9].z, r0.z, r1.x
    r0.z = ((source[9].zzzz)*(r0.zzzz)+(r1.xxxx)).z;
    // 65: mad r0.x, r0.y, r0.x, r0.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(r0.zzzz)).x;
    // 66: mad r0.xyz, v3.xyzx, r0.xxxx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)*(r0.xxxx)+(source[1].xyzx)).xyz;
    // 67: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 68: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 69: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 70: source device depth mapped to centimetre view depth; reconstruction at 72.
    r0.w = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r1.xyxx).xy, 0.f).y * 100000.f;
    // Native 72-75: reconstructed view depth is supplied by the runtime adapter.
    r0.w = r0.w;
    // 76: add r0.w, r0.w, -v7.w
    r0.w = ((r0.wwww)+(-(v7.wwww))).w;
    // 77: add r1.x, -cb0[9].w, l(1.000000)
    r1.x = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: mul r1.x, r1.x, l(100.000000)
    r1.x = ((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000))).x;
    // 79: max r1.x, r1.x, l(0.001000)
    r1.x = (max(r1.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 80: div_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)/(r1.xxxx))).w;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v2.xyxx, t0.xyzw, s7, l(0.000000)
    r1.x = (ALTVNativeSample6((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 82: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 83: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 84: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 85: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 86: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 87: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

// fx_k_pa_slice_01_01_tr: 12959b8a47f91c4dab8a19b9de0871ed; selected map 2695da93e1927eeaf690fdee512c9461bd4fe3fbbb9bcadcc0d1d610f14a68fa.
float4 ALTVNative191(ALTV_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_ALTVSourceMaterialParameters[3u];
    source[2] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[3] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[4] = ALTVNativeAppend(cos((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = ALTVNativeAppend(sin((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_ALTVSourceMaterialParameters[2u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[6].y = (cos((g_ALTVSourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[6].z = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[6].w = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[7].x = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[7].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[7].w = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[8].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
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
    // 1: add r0.xyzw, v2.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((v2.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 2: dp2 r1.x, cb0[2].xyxx, r0.zwzz
    r1.x = (dot((source[2].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 3: dp2 r1.y, cb0[3].xyxx, r0.zwzz
    r1.y = (dot((source[3].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 4: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 5: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 6: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: add r0.yz, r1.xxyx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r1.xxyx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 8: mad r0.yz, r0.yyzy, cb0[6].zzwz, cb0[7].xxyx
    r0.yz = ((r0.yyzy)*(source[6].zzwz)+(source[7].xxyx)).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s1, l(0.000000)
    r0.y = (ALTVNativeSample0((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 10: mul r0.z, v4.z, cb0[7].z
    r0.z = ((v4.zzzz)*(source[7].zzzz)).z;
    // 11: mad r1.xy, r0.yyyy, r0.zzzz, v2.xyxx
    r1.xy = ((r0.yyyy)*(r0.zzzz)+(v2.xyxx)).xy;
    // 12: add r1.zw, -r1.xxxx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = ((-(r1.xxxx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 13: add r0.yz, r1.wwyw, r1.zzyz
    r0.yz = ((r1.wwyw)+(r1.zzyz)).yz;
    // 14: add r0.yz, r0.yyzy, l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 15: dp2 r0.w, cb0[4].xyxx, r0.yzyy
    r0.w = (dot((source[4].xyxx).xy,(r0.yzyy).xy).xxxx).w;
    // 16: dp2 r0.y, cb0[5].xyxx, r0.yzyy
    r0.y = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 17: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 18: div r0.y, l(0.010000), r0.y
    r0.y = ((float4(0.010000,0.010000,0.010000,0.010000))/(r0.yyyy)).y;
    // 19: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 20: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 21: mad r0.w, cb0[8].x, l(10.000000), l(10.000000)
    r0.w = ((source[8].xxxx)*(float4(10.000000,10.000000,10.000000,10.000000))+(float4(10.000000,10.000000,10.000000,10.000000))).w;
    // 22: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 23: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 24: mul r0.z, r0.z, v4.y
    r0.z = ((r0.zzzz)*(v4.yyyy)).z;
    // 25: mul r0.z, r0.z, v3.w
    r0.z = ((r0.zzzz)*(v3.wwww)).z;
    // 26: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 27: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 28: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 29: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 30: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 31: source device depth mapped to centimetre view depth; reconstruction at 33.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 33-36: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 37: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 38: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r0.w, r0.w, l(100.000000)
    r0.w = ((r0.wwww)*(float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 40: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 41: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 42: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 43: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 44: mad r0.xz, v2.xxyx, l(-1.000000, 0.000000, 1.000000, 0.000000), l(1.000000, 0.000000, 0.000000, 0.000000)
    r0.xz = ((v2.xxyx)*(float4(-1.000000,0.000000,1.000000,0.000000))+(float4(1.000000,0.000000,0.000000,0.000000))).xz;
    // 45: mad r0.xz, r0.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r0.xz = ((r0.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 46: dp2 r0.x, r0.xzxx, r0.xzxx
    r0.x = (dot((r0.xzxx).xy,(r0.xzxx).xy).xxxx).x;
    // 47: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 48: add r0.x, r0.x, -v4.x
    r0.x = ((r0.xxxx)+(-(v4.xxxx))).x;
    // 49: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 50: div r0.x, l(0.100000), |r0.x|
    r0.x = ((float4(0.100000,0.100000,0.100000,0.100000))/(abs(r0.xxxx))).x;
    // 51: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

#endif // ALTV_NATIVE_CAPTURE_ONLY
// fx_m_me_swp_box_01: 7a34bdb1e8c49f48b1f747c41f5c1880; selected map 71e3af30a40e72420b44e66fc636115067f70cbdbc126b91596f6526ad129a02.
float4 ALTVNative178(ALTV_NATIVE_INPUT input)
{
    float4 source[18]; [unroll] for (uint i=0u; i<18u; ++i) source[i]=0.f;
    source[2]=1.f; // Project capture color scale; native capture-view constants were not exported.
    source[3]=float4(1.f,0.f,0.f,0.f);
    source[4]=float4(0.f,1.f,0.f,0.f);
    float4 output=0.f;
    source[6] = g_ALTVSourceMaterialParameters[10u];
    source[7] = g_ALTVSourceMaterialParameters[5u];
    source[8] = g_ALTVSourceMaterialParameters[6u];
    source[9] = g_ALTVSourceMaterialParameters[7u];
    source[10] = input.dynamicParameter;
    source[11] = g_ALTVSourceMaterialParameters[9u];
    source[12].x = (g_ALTVSourceMaterialParameters[2u].yyyy).x;
    source[12].y = (g_ALTVSourceMaterialTime.xxxx).x;
    source[12].z = (g_ALTVSourceMaterialParameters[2u].wwww).x;
    source[12].w = ((g_ALTVSourceMaterialTime.xxxx*float4(-0.100000001, 0.0, 0.0, 0.0))).x;
    source[13].x = (g_ALTVSourceMaterialParameters[3u].xxxx).x;
    source[13].y = (g_ALTVSourceMaterialParameters[2u].zzzz).x;
    source[13].z = ((g_ALTVSourceMaterialTime.xxxx*float4(0.200000003, 0.0, 0.0, 0.0))).x;
    source[13].w = (g_ALTVSourceMaterialParameters[3u].zzzz).x;
    source[14].x = (g_ALTVSourceMaterialParameters[3u].yyyy).x;
    source[14].y = (g_ALTVSourceMaterialParameters[1u].xxxx).x;
    source[14].z = (g_ALTVSourceMaterialParameters[4u].xxxx).x;
    source[14].w = (g_ALTVSourceMaterialParameters[4u].yyyy).x;
    source[15].x = (g_ALTVSourceMaterialParameters[2u].xxxx).x;
    source[15].y = (g_ALTVSourceMaterialParameters[0u].xxxx).x;
    source[15].z = (g_ALTVSourceMaterialParameters[0u].zzzz).x;
    source[15].w = (g_ALTVSourceMaterialParameters[0u].yyyy).x;
    source[16].x = (g_ALTVSourceMaterialParameters[0u].wwww).x;
    source[16].y = (g_ALTVSourceMaterialParameters[1u].yyyy).x;
    source[16].z = (g_ALTVSourceMaterialParameters[1u].zzzz).x;
    source[16].w = (g_ALTVSourceMaterialParameters[1u].wwww).x;
    source[17].x = (g_ALTVSourceMaterialParameters[3u].wwww).x;
    float4 passValues[4]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f)};
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(input.tangentView,1.f); // native texcoord6
    float4 v6 = float4(input.uv.x*360.f,(1.f-input.uv.y)*360.f,0.f,1.f); // native texcoord5
    float4 v7 = asfloat(uint4(0xffffffffu,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.x, cb0[17].x, l(-0.333300)
    r0.x = ((source[17].xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 2: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 3: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) clip(-1.f);
    // 5: mul r0.xy, v4.xyxx, l(0.800000, 0.800000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)*(float4(0.800000,0.800000,0.000000,0.000000))).xy;
    // 6: mul r1.z, r0.y, cb0[12].z
    r1.z = ((r0.yyyy)*(source[12].zzzz)).z;
    // 7: mad r1.x, cb0[12].z, r0.x, cb0[12].w
    r1.x = ((source[12].zzzz)*(r0.xxxx)+(source[12].wwww)).x;
    // 8: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 9: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 10: mul r2.xyz, r0.zzzz, v5.xyzx
    r2.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 11: mad r0.z, cb0[13].x, l(0.050000), l(-0.025000)
    r0.z = ((source[13].xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).z;
    // 12: mad r1.xy, r0.zzzz, r2.xyxx, r1.xzxx
    r1.xy = ((r0.zzzz)*(r2.xyxx)+(r1.xzxx)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = (ALTVNativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 14: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 16: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: add r0.w, r0.w, l(0.000010)
    r0.w = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 20: mad r0.y, cb0[13].y, r0.y, cb0[13].z
    r0.y = ((source[13].yyyy)*(r0.yyyy)+(source[13].zzzz)).y;
    // 21: mul r3.y, r0.x, cb0[13].y
    r3.y = ((r0.xxxx)*(source[13].yyyy)).y;
    // 22: add r3.x, r0.y, l(0.500000)
    r3.x = ((r0.yyyy)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 23: mad r0.xy, r0.zzzz, r2.xyxx, r3.xyxx
    r0.xy = ((r0.zzzz)*(r2.xyxx)+(r3.xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = (ALTVNativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 25: mad r4.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: dp2 r0.x, r4.xyxx, r4.xyxx
    r0.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 27: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 29: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 30: add r4.z, r0.w, r0.x
    r4.z = ((r0.wwww)+(r0.xxxx)).z;
    // 31: mov r1.z, l(0.000010)
    r1.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // 32: add r0.xyw, r1.xyxz, r4.xyxz
    r0.xyw = ((r1.xyxz)+(r4.xyxz)).xyw;
    // 33: mad r1.xy, r4.xyxx, l(0.100000, 0.100000, 0.000000, 0.000000), r3.xyxx
    r1.xy = ((r4.xyxx)*(float4(0.100000,0.100000,0.000000,0.000000))+(r3.xyxx)).xy;
    // 34: mad r1.xy, r0.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((r0.zzzz)*(r2.xyxx)+(r1.xyxx)).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = (ALTVNativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 36: dp3 r0.z, r0.xywx, r0.xywx
    r0.z = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).z;
    // 37: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 38: div r0.xyz, r0.xywx, r0.zzzz
    r0.xyz = ((r0.xywx)/(r0.zzzz)).xyz;
    // 39: dp3_sat r0.x, r2.xyzx, r0.xyzx
    r0.x = (saturate(dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx)).x;
    // 40: add r0.x, r0.x, l(0.200000)
    r0.x = ((r0.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 41: mul_sat r0.x, r0.x, r2.z
    r0.x = (saturate((r0.xxxx)*(r2.zzzz))).x;
    // 42: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 44: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 45: mul r0.y, r0.y, cb0[13].w
    r0.y = ((r0.yyyy)*(source[13].wwww)).y;
    // 46: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 47: mul r0.y, r0.y, cb0[14].x
    r0.y = ((r0.yyyy)*(source[14].xxxx)).y;
    // 48: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 49: mul r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)*(r0.xxxx)).xyz;
    // 50: mad r1.xy, v4.xyxx, l(0.800000, 0.800000, 0.000000, 0.000000), l(-0.300000, -0.300000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)*(float4(0.800000,0.800000,0.000000,0.000000))+(float4(-0.300000,-0.300000,0.000000,0.000000))).xy;
    // 51: dp2 r0.w, l(0.598472, -0.801144, 0.000000, 0.000000), r1.xyxx
    r0.w = (dot((float4(0.598472,-0.801144,0.000000,0.000000)).xy,(r1.xyxx).xy).xxxx).w;
    // 52: add r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 53: mul r1.x, |r0.w|, |r0.w|
    r1.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // 54: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 56: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 57: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 58: add r1.xyz, v6.xyzx, cb0[0].xyzx
    r1.xyz = ((v6.xyzx)+(source[0].xyzx)).xyz;
    // 59: add r1.xyz, r1.xyzx, -cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(-(source[1].xyzx))).xyz;
    // 60: mul r1.yw, r1.yyyy, cb0[4].xxxy
    r1.yw = ((r1.yyyy)*(source[4].xxxy)).yw;
    // 61: mad r1.xy, cb0[3].xyxx, r1.xxxx, r1.ywyy
    r1.xy = ((source[3].xyxx)*(r1.xxxx)+(r1.ywyy)).xy;
    // 62: mad r1.xy, cb0[5].xyxx, r1.zzzz, r1.xyxx
    r1.xy = ((source[5].xyxx)*(r1.zzzz)+(r1.xyxx)).xy;
    // 63: mad r0.w, r1.x, l(0.002778), cb0[14].z
    r0.w = ((r1.xxxx)*(float4(0.002778,0.002778,0.002778,0.002778))+(source[14].zzzz)).w;
    // 64: mad r1.x, -r1.y, l(0.002778), cb0[14].w
    r1.x = ((-(r1.yyyy))*(float4(0.002778,0.002778,0.002778,0.002778))+(source[14].wwww)).x;
    // 65: mul r1.y, cb0[10].x, cb0[15].x
    r1.y = ((source[10].xxxx)*(source[15].xxxx)).y;
    // 66: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 67: mad r1.x, r1.x, r1.y, l(1.000000)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 68: mad r1.y, r1.x, cb0[15].w, cb0[16].x
    r1.y = ((r1.xxxx)*(source[15].wwww)+(source[16].xxxx)).y;
    // 69: mad r1.x, r0.w, cb0[15].y, cb0[15].z
    r1.x = ((r0.wwww)*(source[15].yyyy)+(source[15].zzzz)).x;
    // 70: mad r0.w, cb0[16].y, l(0.050000), l(-0.025000)
    r0.w = ((source[16].yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).w;
    // 71: mad r1.xy, r0.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((r0.wwww)*(r2.xyxx)+(r1.xyxx)).xy;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r2.y = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).y;
    // 73: add r0.w, cb0[10].z, cb0[14].y
    r0.w = ((source[10].zzzz)+(source[14].yyyy)).w;
    // 74: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 75: add r1.zw, r1.xxxy, r0.wwww
    r1.zw = ((r1.xxxy)+(r0.wwww)).zw;
    // 76: add r1.xy, -r0.wwww, r1.xyxx
    r1.xy = ((-(r0.wwww))+(r1.xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r2.z = (ALTVNativeSample2((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).z;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r1.zwzz, t2.xyzw, s2, l(0.000000)
    r2.x = (ALTVNativeSample2((r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 79: mul r1.xyz, r2.xyzx, cb0[16].zzzz
    r1.xyz = ((r2.xyzx)*(source[16].zzzz)).xyz;
    // 80: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 81: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 82: mul r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = ((r1.xyzx)*(source[16].wwww)).xyz;
    // 83: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 84: mul r1.xyz, r1.xyzx, cb0[11].xyzx
    r1.xyz = ((r1.xyzx)*(source[11].xyzx)).xyz;
    // 85: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 86: mad r0.xyz, r0.xyzx, cb0[9].xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(source[9].xyzx)+(r1.xyzx)).xyz;
    // 87: add r0.w, -v4.y, l(1.000000)
    r0.w = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 89: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 90: mul r1.x, r1.x, l(15.000000)
    r1.x = ((r1.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 91: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 92: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 93: log r1.x, |v4.y|
    r1.x = (log2(abs(v4.yyyy))).x;
    // 94: mul r1.x, r1.x, l(15.000000)
    r1.x = ((r1.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 95: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 96: lt r1.y, |v4.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(v4.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 97: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 98: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 99: add r1.x, v4.x, l(-0.500000)
    r1.x = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 100: add r1.y, |r1.x|, |r1.x|
    r1.y = ((abs(r1.xxxx))+(abs(r1.xxxx))).y;
    // 101: lt r1.x, |r1.x|, l(0.000000)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 102: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 103: mul r1.y, r1.y, l(10.000000)
    r1.y = ((r1.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 104: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 105: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 106: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 107: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 108: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 109: mul r1.x, r1.x, cb0[12].x
    r1.x = ((r1.xxxx)*(source[12].xxxx)).x;
    // 110: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 111: mul r1.xyz, r1.xxxx, cb0[7].xyzx
    r1.xyz = ((r1.xxxx)*(source[7].xyzx)).xyz;
    // 112: mul r1.xyz, r1.xyzx, cb0[8].xyzx
    r1.xyz = ((r1.xyzx)*(source[8].xyzx)).xyz;
    // 113: movc r1.xyz, r0.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 114: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 115: add o0.xyz, r0.xyzx, cb0[6].xyzx
    output.xyz = ((r0.xyzx)+(source[6].xyzx)).xyz;
    // 116: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    output.a=saturate(input.color.a);
    return output;
}
