SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase800(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[19]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].z=(g_SourceCharacterTime.xxxx).x;
    source[25].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[25].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[26].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[26].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[26].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[26].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[27].x=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[27].y=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[27].z=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 2: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 3: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 4: mad r0.x, cb0[24].y, cb0[24].z, r0.x
    r0.x = ((source[24].yyyy)*(source[24].zzzz)+(r0.xxxx)).x;
    // 5: mul r0.y, r0.x, l(3.524534)
    r0.y = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).y;
    // 6: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 7: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 8: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 9: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 10: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 11: mad r0.x, r0.x, l(0.500000), cb0[24].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[24].xxxx)).x;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v4.xyxx, t6.wxyz, s5, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 13: mul r1.xyz, cb0[11].xyzx, cb0[23].wwww
    r1.xyz = ((source[11].xyzx)*(source[23].wwww)).xyz;
    // 14: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 15: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 16: mul r1.xyz, v8.yyyy, cb1[1].xywx
    r1.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 17: mad r1.xyz, cb1[0].xywx, v8.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v8.xxxx)+(r1.xyzx)).xyz;
    // 18: mad r1.xyz, cb1[2].xywx, v8.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v8.zzzz)+(r1.xyzx)).xyz;
    // 19: mad r1.xyz, cb1[3].xywx, v8.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v8.wwww)+(r1.xyzx)).xyz;
    // 20: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 21: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 22: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 23: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 24: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 25: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 26: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 27: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 28: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 29: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 30: rcp r1.x, |r0.w|
    r1.x = (1.0/(abs(r0.wwww))).x;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 34: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 35: mul r1.z, r1.z, cb0[21].x
    r1.z = ((r1.zzzz)*(source[21].xxxx)).z;
    // 36: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 37: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 39: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 40: mul r1.z, r1.z, cb0[21].y
    r1.z = ((r1.zzzz)*(source[21].yyyy)).z;
    // 41: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r0.w, |r0.w|, r1.x
    r0.w = ((abs(r0.wwww))+(r1.xxxx)).w;
    // 44: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 46: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 47: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 48: mul r3.xy, r1.xzxx, cb0[20].xxxx
    r3.xy = ((r1.xzxx)*(source[20].xxxx)).xy;
    // 49: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 51: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 52: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 53: add r1.xzw, -r3.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r3.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 54: mad r1.xzw, cb0[20].wwww, r1.xxzw, r3.xxyz
    r1.xzw = ((source[20].wwww)*(r1.xxzw)+(r3.xxyz)).xzw;
    // 55: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 56: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 57: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 58: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 59: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 60: mul r4.xyz, r2.wwww, v0.xyzx
    r4.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 61: dp3 r5.x, r4.xyzx, r1.xzwx
    r5.x = (dot((r4.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 62: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 63: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 64: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 65: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 66: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 67: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 68: dp3 r5.y, r7.xyzx, r1.xzwx
    r5.y = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 69: dp3 r5.z, r6.xyzx, r1.xzwx
    r5.z = (dot((r6.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 70: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 71: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 72: mul r8.xyz, r1.xxxx, v6.xyzx
    r8.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // 73: mad r1.xzw, v6.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v6.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 74: dp3 r9.y, r7.xyzx, r8.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 75: dp3 r9.x, r4.xyzx, r8.xyzx
    r9.x = (dot((r4.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 76: dp3 r9.z, r6.xyzx, r8.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 77: dp3 r2.w, r5.xyzx, r9.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 78: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 79: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 80: mov r5.w, -r5.x
    r5.w = (-(r5.xxxx)).w;
    // 81: dp2 r2.w, r5.ywyy, r5.ywyy
    r2.w = (dot((r5.ywyy).xy,(r5.ywyy).xy).xxxx).w;
    // 82: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 83: div r5.xy, r5.ywyy, r2.wwww
    r5.xy = ((r5.ywyy)/(r2.wwww)).xy;
    // 84: mad r2.w, -r5.z, l(0.250000), l(0.250000)
    r2.w = ((-(r5.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 85: add r3.w, r5.z, l(1.000000)
    r3.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 87: mad r5.xy, r2.wwww, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s3, r0.w
    r5.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r5.xyxx).xy, (r0.wwww).x)).xyzw).xyz;
    // 89: log r9.xyz, r5.xyzx
    r9.xyz = (log2(r5.xyzx)).xyz;
    // 90: rcp r0.w, cb0[21].z
    r0.w = (1.0/(source[21].zzzz)).w;
    // 91: mul r10.xyz, r9.xyzx, r0.wwww
    r10.xyz = ((r9.xyzx)*(r0.wwww)).xyz;
    // 92: mul r9.xyz, r9.xyzx, cb0[21].zzzz
    r9.xyz = ((r9.xyzx)*(source[21].zzzz)).xyz;
    // 93: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 94: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 95: mul r10.xyz, r0.wwww, r10.xyzx
    r10.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 96: mad r9.xyz, r9.xyzx, cb0[21].zzzz, r10.xyzx
    r9.xyz = ((r9.xyzx)*(source[21].zzzz)+(r10.xyzx)).xyz;
    // 97: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 98: mul r5.xyz, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 99: add r0.w, cb0[21].z, l(1.000000)
    r0.w = ((source[21].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 101: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: add r5.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r5.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 103: mad r5.xyz, r3.wwww, r5.xyzx, cb0[9].xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(source[9].xyzx)).xyz;
    // 104: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 105: mul r5.xyz, r5.xyzx, cb0[21].wwww
    r5.xyz = ((r5.xyzx)*(source[21].wwww)).xyz;
    // 106: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 107: add r9.xyz, -r2.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 108: mad r2.xyz, cb0[20].yyyy, r9.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 109: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: add r9.xyz, -r2.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 111: mad r2.xyz, cb0[20].zzzz, r9.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 112: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: add r9.xyz, -r2.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 114: mul r9.xyz, r9.xyzx, cb0[22].xxxx
    r9.xyz = ((r9.xyzx)*(source[22].xxxx)).xyz;
    // 115: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 116: add r0.w, r10.y, r10.x
    r0.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 117: add r0.w, r10.z, r0.w
    r0.w = ((r10.zzzz)+(r0.wwww)).w;
    // 118: add_sat r0.w, r10.w, r0.w
    r0.w = (saturate((r10.wwww)+(r0.wwww))).w;
    // 119: mad r2.xyz, r0.wwww, r9.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 120: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 121: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 122: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 123: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 124: dp3 r0.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 126: mul r0.w, r0.w, cb0[23].x
    r0.w = ((r0.wwww)*(source[23].xxxx)).w;
    // 127: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 128: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: mad r2.w, -r0.w, r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 131: div r2.w, cb0[23].y, r2.w
    r2.w = ((source[23].yyyy)/(r2.wwww)).w;
    // 132: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 133: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 134: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 135: dp3 r3.w, r3.xyzx, r8.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 136: mul_sat r4.w, r3.w, cb0[22].y
    r4.w = (saturate((r3.wwww)*(source[22].yyyy))).w;
    // 137: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul_sat r5.w, r8.z, cb0[22].y
    r5.w = (saturate((r8.zzzz)*(source[22].yyyy))).w;
    // 140: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add_sat r5.w, r5.w, -cb0[22].z
    r5.w = (saturate((r5.wwww)+(-(source[22].zzzz)))).w;
    // 142: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 143: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 144: mul r6.w, r6.w, cb0[22].w
    r6.w = ((r6.wwww)*(source[22].wwww)).w;
    // 145: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 146: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 147: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 148: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 149: mul r9.xyz, r5.xyzx, r2.wwww
    r9.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 150: mul r10.xyz, cb0[6].xyzx, cb0[6].wwww
    r10.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 151: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 152: mad r11.xyz, -cb0[6].wwww, cb0[6].xyzx, r2.wwww
    r11.xyz = ((-(source[6].wwww))*(source[6].xyzx)+(r2.wwww)).xyz;
    // 153: mad r10.xyz, cb0[20].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 154: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 156: mad r10.xyz, cb0[20].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 157: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 158: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 160: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 161: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 162: dp3 r2.w, r12.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r13.xyz, -r12.yzwy, r2.wwww
    r13.xyz = ((-(r12.yzwy))+(r2.wwww)).xyz;
    // 164: mad r12.yzw, cb0[20].yyyy, r13.xxyz, r12.yyzw
    r12.yzw = ((source[20].yyyy)*(r13.xxyz)+(r12.yyzw)).yzw;
    // 165: mov_sat r12.x, r12.x
    r12.x = (saturate(r12.xxxx)).x;
    // 166: mul_sat r2.w, r12.x, cb0[27].w
    r2.w = (saturate((r12.xxxx)*(source[27].wwww))).w;
    // 167: mul o0.w, r2.w, cb0[3].x
    output.targets[0].w = ((r2.wwww)*(source[3].xxxx)).w;
    // 168: dp3 r2.w, r12.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 169: add r13.xyz, -r12.yzwy, r2.wwww
    r13.xyz = ((-(r12.yzwy))+(r2.wwww)).xyz;
    // 170: mad r12.xyz, cb0[20].zzzz, r13.xyzx, r12.yzwy
    r12.xyz = ((source[20].zzzz)*(r13.xyzx)+(r12.yzwy)).xyz;
    // 171: mul r13.xyz, r10.xyzx, r12.xyzx
    r13.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 172: mad r10.xyz, r10.xyzx, r12.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = ((r10.xyzx)*(r12.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 173: mul r5.xyz, r5.xyzx, r13.xyzx
    r5.xyz = ((r5.xyzx)*(r13.xyzx)).xyz;
    // 174: mad r2.xyz, r2.xyzx, r9.xyzx, -r5.xyzx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 175: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: mul r2.w, r2.w, cb0[23].z
    r2.w = ((r2.wwww)*(source[23].zzzz)).w;
    // 177: mad r2.xyz, r2.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 178: frc r2.w, cb0[5].x
    r2.w = (frac(source[5].xxxx)).w;
    // 179: add r5.x, -r2.w, l(1.000000)
    r5.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: mad r0.xyz, r5.xxxx, r2.xyzx, r0.xyzx
    r0.xyz = ((r5.xxxx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 181: dp3 r5.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 182: add r5.xyz, -r0.xyzx, r5.xxxx
    r5.xyz = ((-(r0.xyzx))+(r5.xxxx)).xyz;
    // 183: mad r0.xyz, cb0[20].yyyy, r5.xyzx, r0.xyzx
    r0.xyz = ((source[20].yyyy)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 184: dp3 r5.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 185: add r5.xyz, -r0.xyzx, r5.xxxx
    r5.xyz = ((-(r0.xyzx))+(r5.xxxx)).xyz;
    // 186: mad r0.xyz, cb0[20].zzzz, r5.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 187: dp3 r5.x, r10.xyzx, r10.xyzx
    r5.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 188: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 189: div r5.xyz, r10.xyzx, r5.xxxx
    r5.xyz = ((r10.xyzx)/(r5.xxxx)).xyz;
    // 190: dp3 r5.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 191: add r9.xyz, -r5.xyzx, r5.wwww
    r9.xyz = ((-(r5.xyzx))+(r5.wwww)).xyz;
    // 192: add r5.xyz, r5.xyzx, -r9.xyzx
    r5.xyz = ((r5.xyzx)+(-(r9.xyzx))).xyz;
    // 193: mul r9.xyz, cb0[14].xyzx, cb0[25].xxxx
    r9.xyz = ((source[14].xyzx)*(source[25].xxxx)).xyz;
    // 194: mul r9.xyz, r9.xyzx, cb0[26].zzzz
    r9.xyz = ((r9.xyzx)*(source[26].zzzz)).xyz;
    // 195: mul r9.xyz, r4.wwww, r9.xyzx
    r9.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 196: mad r10.xyz, r4.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r10.xyz = ((r4.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 197: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 198: mad r4.w, cb0[12].w, r4.w, l(1.000000)
    r4.w = ((source[12].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 199: mad r10.xyz, cb0[13].wwww, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((source[13].wwww)*(r10.xyzx)+(source[13].xyzx)).xyz;
    // 200: mad r5.xyz, r5.xyzx, r9.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 201: mad r5.xyz, r4.wwww, cb0[12].xyzx, r5.xyzx
    r5.xyz = ((r4.wwww)*(source[12].xyzx)+(r5.xyzx)).xyz;
    // 202: mad r0.xyz, r0.xyzx, r11.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r5.xyzx)).xyz;
    // 203: add r4.w, -|r8.z|, l(1.000000)
    r4.w = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 205: log r4.w, |r3.w|
    r4.w = (log2(abs(r3.wwww))).w;
    // 206: lt r3.w, |r3.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 207: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 208: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 209: mul r5.xyz, r4.wwww, cb0[15].xyzx
    r5.xyz = ((r4.wwww)*(source[15].xyzx)).xyz;
    // 210: movc r5.xyz, r3.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 211: add r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)+(r5.xyzx)).xyz;
    // 212: add r0.xyz, r0.xyzx, cb0[4].xyzx
    r0.xyz = ((r0.xyzx)+(source[4].xyzx)).xyz;
    // 213: dp3 r3.w, r1.xzwx, r1.xzwx
    r3.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 214: sqrt r4.w, r3.w
    r4.w = (sqrt(r3.wwww)).w;
    // 215: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 216: dp3 r1.x, r1.xzwx, r8.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 217: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 218: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 219: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 220: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 221: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 222: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 223: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 224: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 225: div_sat r1.x, r1.x, r3.w
    r1.x = (saturate((r1.xxxx)/(r3.wwww))).x;
    // 226: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 227: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 228: mad r1.xyz, r1.xxxx, r2.xyzx, -r13.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 229: mad r1.xyz, r0.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 230: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 231: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 232: mad r1.xyz, cb0[20].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 233: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 234: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 235: mad r1.xyz, cb0[20].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 236: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 237: add r0.w, -cb0[5].w, l(1.000000)
    r0.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 238: mul r0.w, r0.w, cb0[24].z
    r0.w = ((r0.wwww)*(source[24].zzzz)).w;
    // 239: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 240: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 241: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 242: mul r1.w, cb0[5].z, l(1.500000)
    r1.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 243: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 244: mad r0.w, r0.w, l(0.500000), cb0[5].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 245: add r1.w, -r2.w, cb0[5].x
    r1.w = ((-(r2.wwww))+(source[5].xxxx)).w;
    // 246: mul r5.z, r1.w, l(0.125000)
    r5.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 247: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 248: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 249: mul r5.y, cb0[5].y, cb0[16].y
    r5.y = ((source[5].yyyy)*(source[16].yyyy)).y;
    // 250: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 251: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 252: add r2.xy, r2.xyxx, r5.xyxx
    r2.xy = ((r2.xyxx)+(r5.xyxx)).xy;
    // 253: add r2.xy, r2.xyxx, r5.zwzz
    r2.xy = ((r2.xyxx)+(r5.zwzz)).xy;
    // 254: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 255: mul r2.xyz, r0.wwww, r5.xyzx
    r2.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 256: mul r0.w, r2.w, r5.w
    r0.w = ((r2.wwww)*(r5.wwww)).w;
    // 257: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 258: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 259: add r2.xyzw, v8.yzxy, cb0[0].yzxy
    r2.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 260: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 261: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 262: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 263: mad r2.xy, cb0[17].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 264: mul r0.w, cb0[17].y, cb0[24].z
    r0.w = ((source[17].yyyy)*(source[24].zzzz)).w;
    // 265: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 266: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 267: mul r5.y, r0.w, l(0.020000)
    r5.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 268: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 269: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 270: mul r1.w, cb0[17].x, l(0.001000)
    r1.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 271: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 272: mad r2.xy, r1.wwww, r2.xyxx, r5.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r5.xyxx)).xy;
    // 273: dp2 r1.w, cb0[18].xyxx, r2.xyxx
    r1.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 274: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 275: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 276: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 277: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 278: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 279: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 280: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 281: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 282: mad r5.xyz, cb0[17].zzzz, r2.xyzx, -r1.xyzx
    r5.xyz = ((source[17].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 283: mul r2.xyz, r2.xyzx, cb0[17].zzzz
    r2.xyz = ((r2.xyzx)*(source[17].zzzz)).xyz;
    // 284: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 285: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 286: mad r1.xyz, r0.wwww, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 287: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 288: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 289: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 290: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 291: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 292: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 293: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 294: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 295: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 296: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 297: mul r3.yzw, r3.yyyy, cb0[29].xxyz
    r3.yzw = ((r3.yyyy)*(source[29].xxyz)).yzw;
    // 298: mad r3.xyz, r3.xxxx, cb0[28].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[28].xyzx)+(r3.yzwy)).xyz;
    // 299: mul r3.xyz, r3.xyzx, cb0[30].wwww
    r3.xyz = ((r3.xyzx)*(source[30].wwww)).xyz;
    // 300: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 301: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 302: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 303: mad r0.xyz, r1.xyzx, cb0[30].xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(source[30].xyzx)+(r0.xyzx)).xyz;
    // 304: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 305: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 306: dp3 r0.x, r4.xyzx, r2.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 307: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 308: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 309: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 310: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 311: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 312: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 313: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 314: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 315: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 316: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 317: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 318: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 319: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 320: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 321: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 322: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 323: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 324: ret
    return output;
}

// Original uniform Periodic uses signed fractional, including negative scroll rates.
float4 SourceCharacterPeriodic801(float4 value) { return sign(value) * frac(abs(value)); }
// source.character.sk-sdm-car-tr.v1 / source program 0f31ad3a7d3ec14caf7b444327672a4f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase801(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[2]=SourceCharacterAppend(SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(-0.5,0,0,0))*float4(1,0,0,0))),SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(-0.5,0,0,0))*float4(0,0,0,0))),1u);
    source[3]=SourceCharacterAppend(SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(0,0,0,0))*float4(0,0,0,0))),SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(0,0,0,0))*float4(1,0,0,0))),1u);
    source[5]=SourceCharacterAppend(SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(-1.20000005,0,0,0))*float4(1,0,0,0))),SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(-1.20000005,0,0,0))*float4(0,0,0,0))),1u);
    source[6]=SourceCharacterAppend(SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(0.300000012,0,0,0))*float4(0,0,0,0))),SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(0.300000012,0,0,0))*float4(1,0,0,0))),1u);
    source[9].x=(SourceCharacterPeriodic801(((g_SourceCharacterTime.xxxx*float4(0.300000012,0,0,0))*float4(0,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: mad r0.x, v4.x, l(1.000000), cb0[2].x
    r0.x = ((v4.xxxx)*(float4(1.000000,1.000000,1.000000,1.000000))+(source[2].xxxx)).x;
    // 2: mad r0.y, v4.y, l(2.000000), cb0[3].y
    r0.y = ((v4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[3].yyyy)).y;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 4: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: mad r0.xy, r0.xyxx, l(0.012000, 0.012000, 0.000000, 0.000000), v4.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.012000,0.012000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t7.xyzw, s7, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 7: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t3.zwxy, s4, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 9: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mad r0.zw, r2.xxxy, l(0.000000, 0.000000, 0.100000, 0.100000), r0.xxxy
    r0.zw = ((r2.xxxy)*(float4(0.000000,0.000000,0.100000,0.100000))+(r0.xxxy)).zw;
    // 11: mad r3.x, r0.z, l(0.500000), cb0[2].x
    r3.x = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].xxxx)).x;
    // 12: mad r3.y, r0.w, l(2.000000), cb0[3].y
    r3.y = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[3].yyyy)).y;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 15: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 16: mad r1.xyz, r1.xyzx, l(20.000000, 20.000000, 20.000000, 0.000000), r3.xyzx
    r1.xyz = ((r1.xyzx)*(float4(20.000000,20.000000,20.000000,0.000000))+(r3.xyzx)).xyz;
    // 17: mul r3.xyz, cb0[8].xyzx, cb0[8].wwww
    r3.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 18: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 19: mul r3.xyz, cb0[7].xyzx, cb0[7].wwww
    r3.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 20: mul r3.xyz, r3.xyzx, cb0[9].yyyy
    r3.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 21: mad r4.x, v4.x, l(2.000000), cb0[5].x
    r4.x = ((v4.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(source[5].xxxx)).x;
    // 22: mad r4.y, v4.y, l(3.400000), cb0[6].y
    r4.y = ((v4.yyyy)*(float4(3.400000,3.400000,3.400000,3.400000))+(source[6].yyyy)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r4.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 24: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.040000, 0.040000), v4.xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.040000,0.040000))+(v4.xxxy)).zw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.zwzz, t1.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 29: mad r4.xyz, r4.xyzx, l(0.200000, 0.200000, 0.200000, 0.000000), r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.200000,0.200000,0.200000,0.000000))+(r5.xyzx)).xyz;
    // 30: mad r1.xyz, r4.xyzx, r3.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 31: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 32: mad r0.xyz, r0.xyzx, r3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 33: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 34: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 35: add r0.x, r4.y, r4.x
    r0.x = ((r4.yyyy)+(r4.xxxx)).x;
    // 36: add r0.x, r4.z, r0.x
    r0.x = ((r4.zzzz)+(r0.xxxx)).x;
    // 37: dp2 r0.y, r2.xyxx, r2.xyxx
    r0.y = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 38: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 40: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 41: add r2.z, r0.y, l(0.000010)
    r2.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 43: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 44: mul r0.yzw, r0.yyyy, v6.xxyz
    r0.yzw = ((r0.yyyy)*(v6.xxyz)).yzw;
    // 45: dp3 r0.y, r2.xyzx, r0.yzwy
    r0.y = (dot((r2.xyzx).xyz,(r0.yzwy).xyz).xxxx).y;
    // 46: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 47: mul r0.y, |r0.y|, |r0.y|
    r0.y = ((abs(r0.yyyy))*(abs(r0.yyyy))).y;
    // 48: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 49: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 50: mad r0.x, r0.x, l(0.333330), r0.y
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))+(r0.yyyy)).x;
    // 51: mul_sat r0.x, r0.x, cb0[9].w
    r0.x = (saturate((r0.xxxx)*(source[9].wwww))).x;
    // 52: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 53: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 54: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 55: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 56: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 57: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 58: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 59: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 60: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 61: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 62: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 63: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 64: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 65: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 66: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 67: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 68: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 69: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 70: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 71: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 72: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 73: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 74: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 75: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 76: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 77: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 78: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 79: ret
    return output;
}

// source.character.equipment-native-901.v1 / source program bfc004c118ef0c459844dfe0ffccb7d3
