SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase17(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].w=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 4: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: mul r1.xyz, v7.yyyy, cb1[1].xywx
    r1.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 8: mad r1.xyz, cb1[0].xywx, v7.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v7.xxxx)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[2].xywx, v7.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v7.zzzz)+(r1.xyzx)).xyz;
    // 10: mad r1.xyz, cb1[3].xywx, v7.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v7.wwww)+(r1.xyzx)).xyz;
    // 11: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 14: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 15: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 16: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 17: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 18: max r0.x, r0.x, r1.x
    r0.x = (max(r0.xxxx,r1.xxxx)).x;
    // 19: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 20: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 21: rcp r1.x, |r0.x|
    r1.x = (1.0/(abs(r0.xxxx))).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: lt r1.z, |r1.y|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 25: log r1.y, |r1.y|
    r1.y = (log2(abs(r1.yyyy))).y;
    // 26: add r1.w, cb0[20].w, -cb0[21].x
    r1.w = ((source[20].wwww)+(-(source[21].xxxx))).w;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 28: mad r1.w, r3.x, r1.w, cb0[21].x
    r1.w = ((r3.xxxx)*(r1.wwww)+(source[21].xxxx)).w;
    // 29: add r2.w, -r1.w, cb0[21].y
    r2.w = ((-(r1.wwww))+(source[21].yyyy)).w;
    // 30: mad r1.w, r3.y, r2.w, r1.w
    r1.w = ((r3.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 31: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 32: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 33: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 35: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 36: add r1.w, -cb0[20].y, cb0[20].x
    r1.w = ((-(source[20].yyyy))+(source[20].xxxx)).w;
    // 37: mad r1.w, r3.x, r1.w, cb0[20].y
    r1.w = ((r3.xxxx)*(r1.wwww)+(source[20].yyyy)).w;
    // 38: add r2.w, -r1.w, cb0[20].z
    r2.w = ((-(r1.wwww))+(source[20].zzzz)).w;
    // 39: mad r1.w, r3.y, r2.w, r1.w
    r1.w = ((r3.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 40: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 41: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 44: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 46: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 47: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 48: mul r4.xy, r1.xzxx, cb0[19].xxxx
    r4.xy = ((r1.xzxx)*(source[19].xxxx)).xy;
    // 49: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 51: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 52: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 53: add r1.xzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 54: mad r1.xzw, cb0[19].wwww, r1.xxzw, r4.xxyz
    r1.xzw = ((source[19].wwww)*(r1.xxzw)+(r4.xxyz)).xzw;
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
    // 60: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 61: dp3 r6.x, r5.xyzx, r1.xzwx
    r6.x = (dot((r5.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 62: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 63: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 64: mul r7.xyz, r2.wwww, v1.xyzx
    r7.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 65: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 66: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 67: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 68: dp3 r6.y, r8.xyzx, r1.xzwx
    r6.y = (dot((r8.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 69: dp3 r6.z, r7.xyzx, r1.xzwx
    r6.z = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 70: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 71: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 72: mul r9.xyz, r1.xxxx, v5.xyzx
    r9.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 73: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 74: dp3 r10.y, r8.xyzx, r9.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 75: dp3 r10.x, r5.xyzx, r9.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 76: dp3 r10.z, r7.xyzx, r9.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 77: dp3 r2.w, r6.xyzx, r10.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 78: mul r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 79: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 80: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 81: dp2 r2.w, r6.ywyy, r6.ywyy
    r2.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 82: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 83: div r6.xy, r6.ywyy, r2.wwww
    r6.xy = ((r6.ywyy)/(r2.wwww)).xy;
    // 84: mad r2.w, -r6.z, l(0.250000), l(0.250000)
    r2.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 85: add r4.w, r6.z, l(1.000000)
    r4.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 87: mad r6.xy, r2.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, r0.x
    r6.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 89: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 90: rcp r0.x, cb0[21].z
    r0.x = (1.0/(source[21].zzzz)).x;
    // 91: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 92: mul r10.xyz, r10.xyzx, cb0[21].zzzz
    r10.xyz = ((r10.xyzx)*(source[21].zzzz)).xyz;
    // 93: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 94: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 95: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 96: mad r10.xyz, r10.xyzx, cb0[21].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[21].zzzz)+(r11.xyzx)).xyz;
    // 97: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 98: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 99: add r0.x, cb0[21].z, l(1.000000)
    r0.x = ((source[21].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 101: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 102: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 103: mad r6.xyz, r4.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 104: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 105: mul r6.xyz, r6.xyzx, cb0[21].wwww
    r6.xyz = ((r6.xyzx)*(source[21].wwww)).xyz;
    // 106: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 107: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 108: div r4.xyz, r4.xyzx, r0.xxxx
    r4.xyz = ((r4.xyzx)/(r0.xxxx)).xyz;
    // 109: dp3 r0.x, r4.xyzx, r9.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 110: mul_sat r2.w, r0.x, cb0[22].y
    r2.w = (saturate((r0.xxxx)*(source[22].yyyy))).w;
    // 111: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mul_sat r4.w, r9.z, cb0[22].y
    r4.w = (saturate((r9.zzzz)*(source[22].yyyy))).w;
    // 114: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: add_sat r4.w, r4.w, -cb0[22].z
    r4.w = (saturate((r4.wwww)+(-(source[22].zzzz)))).w;
    // 116: log r5.w, r4.w
    r5.w = (log2(r4.wwww)).w;
    // 117: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 118: mul r5.w, r5.w, cb0[22].w
    r5.w = ((r5.wwww)*(source[22].wwww)).w;
    // 119: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 120: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 121: movc r2.w, r4.w, l(0), r2.w
    r2.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 122: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: add r10.xyz, -r2.xyzx, r4.wwww
    r10.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 124: mad r2.xyz, cb0[19].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 125: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r10.xyz, -r2.xyzx, r4.wwww
    r10.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 127: mad r2.xyz, cb0[19].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 128: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: add r10.xyz, -r2.xyzx, r4.wwww
    r10.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 130: mul r10.xyz, r10.xyzx, cb0[22].xxxx
    r10.xyz = ((r10.xyzx)*(source[22].xxxx)).xyz;
    // 131: add r4.w, r3.y, r3.x
    r4.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 132: add r4.w, r3.z, r4.w
    r4.w = ((r3.zzzz)+(r4.wwww)).w;
    // 133: add_sat r3.w, r3.w, r4.w
    r3.w = (saturate((r3.wwww)+(r4.wwww))).w;
    // 134: mad r2.xyz, r3.wwww, r10.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 135: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 136: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 137: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 138: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 139: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 141: mul r3.w, r3.w, cb0[23].x
    r3.w = ((r3.wwww)*(source[23].xxxx)).w;
    // 142: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 143: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: div r4.w, cb0[23].y, r4.w
    r4.w = ((source[23].yyyy)/(r4.wwww)).w;
    // 147: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 148: mul r10.xyz, r6.xyzx, r4.wwww
    r10.xyz = ((r6.xyzx)*(r4.wwww)).xyz;
    // 149: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 151: mad r0.yzw, cb0[19].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 152: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 154: mad r0.yzw, cb0[19].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 155: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 156: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 157: mad r11.xyz, r3.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 158: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 159: mad r11.xyz, r3.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 160: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 161: mad r11.xyz, r3.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 162: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 163: add r12.xyz, -r11.xyzx, r3.xxxx
    r12.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 164: mad r11.xyz, cb0[19].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 165: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 166: add r12.xyz, -r11.xyzx, r3.xxxx
    r12.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 167: mad r11.xyz, cb0[19].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 168: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mad r13.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 171: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 172: mul r0.yzw, r0.yyzw, r11.xxyz
    r0.yzw = ((r0.yyzw)*(r11.xxyz)).yzw;
    // 173: mul r6.xyz, r6.xyzx, r0.yzwy
    r6.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 174: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 175: add r3.x, -r3.w, l(1.000000)
    r3.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 176: mul r3.x, r3.x, cb0[23].z
    r3.x = ((r3.xxxx)*(source[23].zzzz)).x;
    // 177: mad r2.xyz, r3.xxxx, r2.xyzx, r6.xyzx
    r2.xyz = ((r3.xxxx)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 178: dp3 r3.x, r1.xzwx, r1.xzwx
    r3.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 179: sqrt r3.y, r3.x
    r3.y = (sqrt(r3.xxxx)).y;
    // 180: div r1.xzw, r1.xxzw, r3.yyyy
    r1.xzw = ((r1.xxzw)/(r3.yyyy)).xzw;
    // 181: dp3 r1.x, r1.xzwx, r9.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 182: add r1.z, -|r9.z|, l(1.000000)
    r1.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 183: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 184: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 185: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 186: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 187: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 188: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 189: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 190: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 191: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 192: div_sat r1.x, r1.x, r3.x
    r1.x = (saturate((r1.xxxx)/(r3.xxxx))).x;
    // 193: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 194: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 195: mad r1.xyz, r1.xxxx, r2.xyzx, -r0.yzwy
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r0.yzwy))).xyz;
    // 196: mad r0.yzw, r3.wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 197: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 199: mad r0.yzw, cb0[19].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[19].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 200: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 201: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 202: mad r0.yzw, cb0[19].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[19].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 203: mul r0.yzw, r12.xxyz, r0.yyzw
    r0.yzw = ((r12.xxyz)*(r0.yyzw)).yzw;
    // 204: add r1.x, -cb0[3].w, l(1.000000)
    r1.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r1.x, r1.x, cb0[23].w
    r1.x = ((r1.xxxx)*(source[23].wwww)).x;
    // 206: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 207: sincos r1.x, null, r1.x
    r1.x = (sin(r1.xxxx)).x;
    // 208: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 209: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 210: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 211: mad r1.x, r1.x, l(0.500000), cb0[3].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 212: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 213: mul r3.x, r1.y, l(0.125000)
    r3.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 214: mul r6.y, cb0[3].y, cb0[15].y
    r6.y = ((source[3].yyyy)*(source[15].yyyy)).y;
    // 215: mov r3.y, v4.y
    r3.y = (v4.yyyy).y;
    // 216: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 217: add r1.yz, r3.xxyx, r6.xxyx
    r1.yz = ((r3.xxyx)+(r6.xxyx)).yz;
    // 218: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 219: add r3.x, -r1.w, cb0[3].x
    r3.x = ((-(r1.wwww))+(source[3].xxxx)).x;
    // 220: mul r6.z, r3.x, l(0.125000)
    r6.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 221: add r1.yz, r1.yyzy, r6.zzwz
    r1.yz = ((r1.yyzy)+(r6.zzwz)).yz;
    // 222: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.yzyy, t5.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 223: mul r1.xyz, r1.xxxx, r6.xyzx
    r1.xyz = ((r1.xxxx)*(r6.xyzx)).xyz;
    // 224: mul r3.x, r1.w, r6.w
    r3.x = ((r1.wwww)*(r6.wwww)).x;
    // 225: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.yzwy
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.yzwy))).xyz;
    // 227: mad r0.yzw, r3.xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 228: add r6.xyzw, v7.yzxy, cb0[0].yzxy
    r6.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 229: add r6.xyzw, r6.xyzw, -cb0[1].yzxy
    r6.xyzw = ((r6.xyzw)+(-(source[1].yzxy))).xyzw;
    // 230: add r1.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 231: add r1.xy, -r6.zwzz, r1.xyxx
    r1.xy = ((-(r6.zwzz))+(r1.xyxx)).xy;
    // 232: mad r1.xy, cb0[16].wwww, r1.xyxx, r6.zwzz
    r1.xy = ((source[16].wwww)*(r1.xyxx)+(r6.zwzz)).xy;
    // 233: mul r1.z, cb0[16].y, cb0[23].w
    r1.z = ((source[16].yyyy)*(source[23].wwww)).z;
    // 234: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 235: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 236: mul r3.y, r1.z, l(0.020000)
    r3.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 237: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 238: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 239: mul r3.w, cb0[16].x, l(0.001000)
    r3.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 240: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 241: mad r1.xy, r3.wwww, r1.xyxx, r3.xyxx
    r1.xy = ((r3.wwww)*(r1.xyxx)+(r3.xyxx)).xy;
    // 242: dp2 r3.x, cb0[17].xyxx, r1.xyxx
    r3.x = (dot((source[17].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 243: dp2 r1.y, cb0[18].xyxx, r1.xyxx
    r1.y = (dot((source[18].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 244: frc r3.x, r3.x
    r3.x = (frac(r3.xxxx)).x;
    // 245: mul r1.x, r3.x, l(0.125000)
    r1.x = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 246: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 247: mad r3.xyw, r6.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r0.yzyw
    r3.xyw = ((r6.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r0.yzyw))).xyw;
    // 248: mul r1.x, r6.w, l(0.900000)
    r1.x = ((r6.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 249: mad r3.xyw, r1.xxxx, r3.xyxw, r0.yzyw
    r3.xyw = ((r1.xxxx)*(r3.xyxw)+(r0.yzyw)).xyw;
    // 250: mul_sat r1.xyz, r1.zzzz, r3.xywx
    r1.xyz = (saturate((r1.zzzz)*(r3.xywx))).xyz;
    // 251: mad r3.xyw, cb0[16].zzzz, r1.xyxz, -r0.yzyw
    r3.xyw = ((source[16].zzzz)*(r1.xyxz)+(-(r0.yzyw))).xyw;
    // 252: mul r1.xyz, r1.xyzx, cb0[16].zzzz
    r1.xyz = ((r1.xyzx)*(source[16].zzzz)).xyz;
    // 253: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 254: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 255: mad r0.yzw, r1.xxxx, r3.xxyw, r0.yyzw
    r0.yzw = ((r1.xxxx)*(r3.xxyw)+(r0.yyzw)).yzw;
    // 256: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 257: add r1.x, -r3.z, r2.w
    r1.x = ((-(r3.zzzz))+(r2.wwww)).x;
    // 258: mad r3.xyw, r2.wwww, cb0[13].xyxz, -cb0[13].xyxz
    r3.xyw = ((r2.wwww)*(source[13].xyxz)+(-(source[13].xyxz))).xyw;
    // 259: mad r3.xyw, cb0[13].wwww, r3.xyxw, cb0[13].xyxz
    r3.xyw = ((source[13].wwww)*(r3.xyxw)+(source[13].xyxz)).xyw;
    // 260: mad r1.x, cb0[12].w, r1.x, r3.z
    r1.x = ((source[12].wwww)*(r1.xxxx)+(r3.zzzz)).x;
    // 261: mad r1.xyz, r1.xxxx, cb0[12].xyzx, r3.xywx
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r3.xywx)).xyz;
    // 262: mul r3.xyz, r2.xyzx, r1.wwww
    r3.xyz = ((r2.xyzx)*(r1.wwww)).xyz;
    // 263: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 264: mad r2.xyz, -r1.wwww, r2.xyzx, r2.wwww
    r2.xyz = ((-(r1.wwww))*(r2.xyzx)+(r2.wwww)).xyz;
    // 265: mad r2.xyz, cb0[19].yyyy, r2.xyzx, r3.xyzx
    r2.xyz = ((source[19].yyyy)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 266: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 267: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 268: mad r2.xyz, cb0[19].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 269: mad r1.xyz, r2.xyzx, r12.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r12.xyzx)+(r1.xyzx)).xyz;
    // 270: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 271: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 272: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 273: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 274: mul r2.xyz, r1.wwww, cb0[14].xyzx
    r2.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 275: movc r2.xyz, r0.xxxx, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 276: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 277: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 278: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 279: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 280: mul r2.xyz, r0.xxxx, r4.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 281: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 282: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 283: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 284: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 285: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 286: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 287: mul r3.yzw, r3.yyyy, cb0[25].xxyz
    r3.yzw = ((r3.yyyy)*(source[25].xxyz)).yzw;
    // 288: mad r3.xyz, r3.xxxx, cb0[24].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[24].xyzx)+(r3.yzwy)).xyz;
    // 289: mul r3.xyz, r3.xyzx, cb0[26].wwww
    r3.xyz = ((r3.xyzx)*(source[26].wwww)).xyz;
    // 290: mad r1.xyz, r3.xyzx, r0.yzwy, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 291: mul r3.xyz, r0.yzwy, r3.xyzx
    r3.xyz = ((r0.yzwy)*(r3.xyzx)).xyz;
    // 292: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 293: mad o0.xyz, r0.yzwy, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 294: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 295: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 296: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 297: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 298: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 299: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 300: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 301: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 302: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 303: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 304: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 305: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 306: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 307: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 308: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 309: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 310: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 311: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 312: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 313: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 314: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase18(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    // Existing source draw uses full material coverage and identity colour scale.
    source[0].x = 1.f; source[1].w = 1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: add r0.x, v4.w, l(0.500000)
    r0.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 2: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 3: mad r0.z, cb0[16].y, l(-3.500000), l(5.000000)
    r0.z = ((source[16].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 4: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 5: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 6: add r1.x, -r0.w, v4.z
    r1.x = ((-(r0.wwww))+(v4.zzzz)).x;
    // 7: mad r0.w, cb0[17].y, r1.x, r0.w
    r0.w = ((source[17].yyyy)*(r1.xxxx)+(r0.wwww)).w;
    // 8: mul r1.x, r0.w, cb0[17].z
    r1.x = ((r0.wwww)*(source[17].zzzz)).x;
    // 9: mad r0.w, r1.x, l(0.750000), r0.w
    r0.w = ((r1.xxxx)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 10: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 11: mad_sat r0.w, cb0[18].x, r0.w, r0.w
    r0.w = (saturate((source[18].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 13: add r2.x, -r1.y, l(1.000000)
    r2.x = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 15: mad r3.xy, r2.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r2.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r2.y, r3.x, r1.x, l(0.200000)
    r2.y = ((r3.xxxx)*(r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 17: add r2.x, -r2.y, r2.x
    r2.x = ((-(r2.yyyy))+(r2.xxxx)).x;
    // 18: mad r2.z, cb0[19].x, r2.x, r2.y
    r2.z = ((source[19].xxxx)*(r2.xxxx)+(r2.yyyy)).z;
    // 19: mad r2.x, cb0[18].z, r2.x, r2.y
    r2.x = ((source[18].zzzz)*(r2.xxxx)+(r2.yyyy)).x;
    // 20: add r2.xy, -r0.wwww, r2.xzxx
    r2.xy = ((-(r0.wwww))+(r2.xzxx)).xy;
    // 21: mul r2.z, cb0[17].w, l(0.700000)
    r2.z = ((source[17].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).z;
    // 22: mad r2.y, r2.z, r2.y, r0.w
    r2.y = ((r2.zzzz)*(r2.yyyy)+(r0.wwww)).y;
    // 23: mad r0.w, r2.z, r2.x, r0.w
    r0.w = ((r2.zzzz)*(r2.xxxx)+(r0.wwww)).w;
    // 24: div r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)/(source[18].yyyy)).w;
    // 25: add r0.yw, -r0.xxxw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = ((-(r0.xxxw))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 26: mul r0.w, r0.w, r0.z
    r0.w = ((r0.wwww)*(r0.zzzz)).w;
    // 27: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 28: mul_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)*(r0.wwww))).x;
    // 29: div r0.w, r2.y, cb0[18].w
    r0.w = ((r2.yyyy)/(source[18].wwww)).w;
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
    // 35: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 36: mad r0.x, cb0[19].y, r0.x, r1.y
    r0.x = ((source[19].yyyy)*(r0.xxxx)+(r1.yyyy)).x;
    // 37: add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // 38: mul r0.yzw, r0.yyzw, cb0[16].xxxx
    r0.yzw = ((r0.yyzw)*(source[16].xxxx)).yzw;
    // 39: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r0.xyz, cb0[19].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 43: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 45: mad r0.xyz, cb0[19].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[19].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
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
    // 55: mul r1.xyz, r1.zzzz, r2.xyzx
    r1.xyz = ((r1.zzzz)*(r2.xyzx)).xyz;
    // 56: mul o0.w, r1.w, cb0[1].w
    output.targets[0].w = ((r1.wwww)*(source[1].wwww)).w;
    // 57: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 58: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 59: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 60: mov_sat r0.w, r2.z
    r0.w = (saturate(r2.zzzz)).w;
    // 61: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 62: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 63: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 64: mul r1.xyz, r1.xyzx, cb0[20].xxxx
    r1.xyz = ((r1.xyzx)*(source[20].xxxx)).xyz;
    // 65: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 66: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 68: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 69: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 70: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 71: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 72: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 73: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 74: add r1.w, -|r0.w|, l(1.000000)
    r1.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: add r2.x, -|r2.z|, l(1.000000)
    r2.x = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 76: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 77: mad r2.xyw, r1.wwww, cb0[9].xyxz, -cb0[9].xyxz
    r2.xyw = ((r1.wwww)*(source[9].xyxz)+(-(source[9].xyxz))).xyw;
    // 78: mad r2.xyw, cb0[9].wwww, r2.xyxw, cb0[9].xyxz
    r2.xyw = ((source[9].wwww)*(r2.xyxw)+(source[9].xyxz)).xyw;
    // 79: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: mul_sat r0.w, r0.w, cb0[21].z
    r0.w = (saturate((r0.wwww)*(source[21].zzzz))).w;
    // 81: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 82: mad r2.xyw, r1.wwww, cb0[8].xyxz, r2.xyxw
    r2.xyw = ((r1.wwww)*(source[8].xyxz)+(r2.xyxw)).xyw;
    // 83: mul_sat r1.w, r2.z, cb0[21].z
    r1.w = (saturate((r2.zzzz)*(source[21].zzzz))).w;
    // 84: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: add_sat r1.w, r1.w, -cb0[21].w
    r1.w = (saturate((r1.wwww)+(-(source[21].wwww)))).w;
    // 86: log r3.w, r1.w
    r3.w = (log2(r1.wwww)).w;
    // 87: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 88: mul r3.w, r3.w, cb0[22].x
    r3.w = ((r3.wwww)*(source[22].xxxx)).w;
    // 89: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 90: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 91: mul r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)*(source[10].wwww)).w;
    // 92: mul r4.xyz, r0.wwww, cb0[10].xyzx
    r4.xyz = ((r0.wwww)*(source[10].xyzx)).xyz;
    // 93: movc r4.xyz, r1.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 94: add r2.xyw, r2.xyxw, r4.xyxz
    r2.xyw = ((r2.xyxw)+(r4.xyxz)).xyw;
    // 95: add r4.xyz, v8.xyzx, cb0[0].yzwy
    r4.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 96: add r5.xyz, -r4.xyzx, cb0[0].yzwy
    r5.xyz = ((-(r4.xyzx))+(source[0].yzwy)).xyz;
    // 97: add r4.xyzw, r4.yzxy, -cb0[1].yzxy
    r4.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 98: mul r6.xyz, r2.zzzz, r5.xyzx
    r6.xyz = ((r2.zzzz)*(r5.xyzx)).xyz;
    // 99: mad r5.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r5.xyzx)).xyz;
    // 100: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 101: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 102: div r0.w, r5.z, r0.w
    r0.w = ((r5.zzzz)/(r0.wwww)).w;
    // 103: add r0.w, r0.w, cb0[7].z
    r0.w = ((r0.wwww)+(source[7].zzzz)).w;
    // 104: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 105: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 106: mul r5.xyz, r1.wwww, v1.xyzx
    r5.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 107: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 108: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 109: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 110: add r0.w, r0.w, l(-0.500000)
    r0.w = ((r0.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 111: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 112: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: log r1.w, r0.w
    r1.w = (log2(r0.wwww)).w;
    // 114: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 115: mad r2.z, cb0[20].w, l(4.500000), l(0.500000)
    r2.z = ((source[20].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 116: mul r2.z, r2.z, cb0[21].x
    r2.z = ((r2.zzzz)*(source[21].xxxx)).z;
    // 117: mul r2.z, r2.z, l(0.050000)
    r2.z = ((r2.zzzz)*(float4(0.050000,0.050000,0.050000,0.050000))).z;
    // 118: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 119: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 120: mul r1.w, r1.w, cb0[21].y
    r1.w = ((r1.wwww)*(source[21].yyyy)).w;
    // 121: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 122: mad r1.xyz, r0.wwww, r1.xyzx, r2.xywx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xywx)).xyz;
    // 123: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 124: add r2.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 125: add r2.xy, -r4.zwzz, r2.xyxx
    r2.xy = ((-(r4.zwzz))+(r2.xyxx)).xy;
    // 126: mad r2.xy, cb0[13].wwww, r2.xyxx, r4.zwzz
    r2.xy = ((source[13].wwww)*(r2.xyxx)+(r4.zwzz)).xy;
    // 127: mul r0.w, cb0[13].y, cb0[22].z
    r0.w = ((source[13].yyyy)*(source[22].zzzz)).w;
    // 128: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 129: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 130: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 131: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 133: mul r1.w, cb0[13].x, l(0.001000)
    r1.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 134: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 135: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 136: dp2 r4.y, cb0[15].xyxx, r2.xyxx
    r4.y = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 137: dp2 r1.w, cb0[14].xyxx, r2.xyxx
    r1.w = (dot((source[14].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 138: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 139: mul r4.x, r1.w, l(0.125000)
    r4.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 141: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 142: add r2.w, -cb0[11].w, l(1.000000)
    r2.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mul r2.w, r2.w, cb0[22].z
    r2.w = ((r2.wwww)*(source[22].zzzz)).w;
    // 144: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 145: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 146: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r3.w, cb0[11].z, l(1.500000)
    r3.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 148: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 149: mad r2.w, r2.w, l(0.500000), cb0[11].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 150: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 151: mul r4.x, r3.w, l(0.125000)
    r4.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 152: mul r6.y, cb0[11].y, cb0[12].y
    r6.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 153: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 154: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 155: add r4.xy, r4.xyxx, r6.xyxx
    r4.xy = ((r4.xyxx)+(r6.xyxx)).xy;
    // 156: frc r3.w, cb0[11].x
    r3.w = (frac(source[11].xxxx)).w;
    // 157: add r4.z, -r3.w, cb0[11].x
    r4.z = ((-(r3.wwww))+(source[11].xxxx)).z;
    // 158: mul r6.z, r4.z, l(0.125000)
    r6.z = ((r4.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 159: add r4.xy, r4.xyxx, r6.zwzz
    r4.xy = ((r4.xyxx)+(r6.zwzz)).xy;
    // 160: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 161: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 162: mul r2.w, r3.w, r4.w
    r2.w = ((r3.wwww)*(r4.wwww)).w;
    // 163: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 164: mad r0.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 165: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 166: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 167: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 168: mad r4.xyz, cb0[13].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[13].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 169: mul r2.xyz, r2.xyzx, cb0[13].zzzz
    r2.xyz = ((r2.xyzx)*(source[13].zzzz)).xyz;
    // 170: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 171: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 172: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 173: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 174: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 175: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 176: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 177: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 178: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 179: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 180: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 181: mul r2.xyz, r2.xyzx, cb0[0].xxxx
    r2.xyz = ((r2.xyzx)*(source[0].xxxx)).xyz;
    // 182: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 183: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 184: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 185: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 186: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 187: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 188: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 189: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 190: mad r1.xyz, r0.xyzx, cb0[25].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[25].xyzx)+(r1.xyzx)).xyz;
    // 191: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 192: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 193: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 194: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 195: mul r0.xyz, r0.xxxx, v0.xyzx
    r0.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 196: mul r1.xyz, r0.yzxy, r5.zxyz
    r1.xyz = ((r0.yzxy)*(r5.zxyz)).xyz;
    // 197: mad r1.xyz, r5.yzxy, r0.zxyz, -r1.xyzx
    r1.xyz = ((r5.yzxy)*(r0.zxyz)+(-(r1.xyzx))).xyz;
    // 198: dp3 r3.z, r5.xyzx, r2.xyzx
    r3.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 199: dp3 r3.x, r0.xyzx, r2.xyzx
    r3.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 200: mul r0.xyz, r1.xyzx, v1.wwww
    r0.xyz = ((r1.xyzx)*(v1.wwww)).xyz;
    // 201: dp3 r3.y, r0.xyzx, r2.xyzx
    r3.y = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 202: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 203: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 204: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 205: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 206: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 207: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 208: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 209: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 210: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 211: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 212: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 213: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 214: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 215: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 216: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 217: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase19(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].z=(g_SourceCharacterTime.xxxx).x;
    source[21].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22].x=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[22].y=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[22].z=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Existing source draw uses full material coverage and identity colour scale.
    source[0].x = 1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add_sat r0.w, r0.w, -cb0[23].w
    r0.w = (saturate((r0.wwww)+(-(source[23].wwww)))).w;
    // 3: add r0.w, r0.w, l(-0.001000)
    r0.w = ((r0.wwww)+(float4(-0.001000,-0.001000,-0.001000,-0.001000))).w;
    // 4: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 5: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 8: add r1.x, -r0.w, v4.z
    r1.x = ((-(r0.wwww))+(v4.zzzz)).x;
    // 9: mad r0.w, cb0[17].y, r1.x, r0.w
    r0.w = ((source[17].yyyy)*(r1.xxxx)+(r0.wwww)).w;
    // 10: mul r1.x, r0.w, cb0[17].z
    r1.x = ((r0.wwww)*(source[17].zzzz)).x;
    // 11: mad r0.w, r1.x, l(0.750000), r0.w
    r0.w = ((r1.xxxx)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 12: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 13: mad_sat r0.w, cb0[18].x, r0.w, r0.w
    r0.w = (saturate((source[18].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 14: add r1.x, -r0.y, l(1.000000)
    r1.x = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r1.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 16: mad r2.xy, r1.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r1.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 17: mad r1.y, r2.x, r0.x, l(0.200000)
    r1.y = ((r2.xxxx)*(r0.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 18: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 19: mad r1.z, cb0[19].x, r1.x, r1.y
    r1.z = ((source[19].xxxx)*(r1.xxxx)+(r1.yyyy)).z;
    // 20: mad r1.x, cb0[18].z, r1.x, r1.y
    r1.x = ((source[18].zzzz)*(r1.xxxx)+(r1.yyyy)).x;
    // 21: add r1.xy, -r0.wwww, r1.xzxx
    r1.xy = ((-(r0.wwww))+(r1.xzxx)).xy;
    // 22: mul r1.z, cb0[17].w, l(0.700000)
    r1.z = ((source[17].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).z;
    // 23: mad r1.y, r1.z, r1.y, r0.w
    r1.y = ((r1.zzzz)*(r1.yyyy)+(r0.wwww)).y;
    // 24: mad r0.w, r1.z, r1.x, r0.w
    r0.w = ((r1.zzzz)*(r1.xxxx)+(r0.wwww)).w;
    // 25: div r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)/(source[18].yyyy)).w;
    // 26: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: div r1.x, r1.y, cb0[18].w
    r1.x = ((r1.yyyy)/(source[18].wwww)).x;
    // 28: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: mad r1.y, cb0[16].y, l(-3.500000), l(5.000000)
    r1.y = ((source[16].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 30: mul r1.y, r1.y, cb0[17].x
    r1.y = ((r1.yyyy)*(source[17].xxxx)).y;
    // 31: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 32: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 33: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 34: mul r1.x, r1.x, l(4.000000)
    r1.x = ((r1.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 35: add r1.y, v4.w, l(0.500000)
    r1.y = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 36: round_ni r1.y, r1.y
    r1.y = (floor(r1.yyyy)).y;
    // 37: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: mul_sat r0.w, r0.w, r1.y
    r0.w = (saturate((r0.wwww)*(r1.yyyy))).w;
    // 39: mul_sat r1.x, r1.z, r1.x
    r1.x = (saturate((r1.zzzz)*(r1.xxxx))).x;
    // 40: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 41: add r0.w, -r0.y, r0.w
    r0.w = ((-(r0.yyyy))+(r0.wwww)).w;
    // 42: mad r0.y, cb0[19].y, r0.w, r0.y
    r0.y = ((source[19].yyyy)*(r0.wwww)+(r0.yyyy)).y;
    // 43: add r1.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r1.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 44: mul r1.xyz, r1.xyzx, cb0[16].xxxx
    r1.xyz = ((r1.xyzx)*(source[16].xxxx)).xyz;
    // 45: mad r1.xyz, r0.yyyy, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 46: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 47: add r3.xyz, -r1.xyzx, r0.yyyy
    r3.xyz = ((-(r1.xyzx))+(r0.yyyy)).xyz;
    // 48: mad r1.xyz, cb0[19].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 49: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 50: add r3.xyz, -r1.xyzx, r0.yyyy
    r3.xyz = ((-(r1.xyzx))+(r0.yyyy)).xyz;
    // 51: mad r1.xyz, cb0[19].wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((source[19].wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 52: mad r3.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 55: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 56: mad r1.xyz, r1.xyzx, r3.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 57: mul r0.xyw, r0.xxxx, r4.xyxz
    r0.xyw = ((r0.xxxx)*(r4.xyxz)).xyw;
    // 58: add r1.w, -cb0[9].w, l(1.000000)
    r1.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul r1.w, r1.w, cb0[21].z
    r1.w = ((r1.wwww)*(source[21].zzzz)).w;
    // 60: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 61: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 62: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 63: mul r2.w, cb0[9].z, l(1.500000)
    r2.w = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 64: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 65: mad r1.w, r1.w, l(0.500000), cb0[9].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 66: mul r3.y, cb0[9].y, cb0[10].y
    r3.y = ((source[9].yyyy)*(source[10].yyyy)).y;
    // 67: mul r4.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r4.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 68: frc r2.w, r4.x
    r2.w = (frac(r4.xxxx)).w;
    // 69: mul r4.y, r2.w, l(0.125000)
    r4.y = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 70: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 71: add r3.xy, r3.xyxx, r4.yzyy
    r3.xy = ((r3.xyxx)+(r4.yzyy)).xy;
    // 72: frc r2.w, cb0[9].x
    r2.w = (frac(source[9].xxxx)).w;
    // 73: add r4.x, -r2.w, cb0[9].x
    r4.x = ((-(r2.wwww))+(source[9].xxxx)).x;
    // 74: mul r3.z, r4.x, l(0.125000)
    r3.z = ((r4.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 75: add r3.xy, r3.xyxx, r3.zwzz
    r3.xy = ((r3.xyxx)+(r3.zwzz)).xy;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 77: mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 78: mul r1.w, r2.w, r3.w
    r1.w = ((r2.wwww)*(r3.wwww)).w;
    // 79: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xywx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xywx))).xyz;
    // 80: mad r0.xyw, r1.wwww, r3.xyxz, r0.xyxw
    r0.xyw = ((r1.wwww)*(r3.xyxz)+(r0.xyxw)).xyw;
    // 81: add r3.xyz, v7.xyzx, cb0[0].yzwy
    r3.xyz = ((v7.xyzx)+(source[0].yzwy)).xyz;
    // 82: add r4.xyzw, r3.yzxy, -cb0[1].yzxy
    r4.xyzw = ((r3.yzxy)+(-(source[1].yzxy))).xyzw;
    // 83: add r3.xyz, -r3.xyzx, cb0[0].yzwy
    r3.xyz = ((-(r3.xyzx))+(source[0].yzwy)).xyz;
    // 84: add r4.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r4.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 85: add r4.xy, -r4.zwzz, r4.xyxx
    r4.xy = ((-(r4.zwzz))+(r4.xyxx)).xy;
    // 86: mad r4.xy, cb0[11].wwww, r4.xyxx, r4.zwzz
    r4.xy = ((source[11].wwww)*(r4.xyxx)+(r4.zwzz)).xy;
    // 87: mul r1.w, cb0[11].y, cb0[21].z
    r1.w = ((source[11].yyyy)*(source[21].zzzz)).w;
    // 88: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 89: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 90: mul r5.y, r1.w, l(0.020000)
    r5.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 91: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 93: mul r2.w, cb0[11].x, l(0.001000)
    r2.w = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 94: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 95: mad r4.xy, r2.wwww, r4.xyxx, r5.xyxx
    r4.xy = ((r2.wwww)*(r4.xyxx)+(r5.xyxx)).xy;
    // 96: dp2 r2.w, cb0[12].xyxx, r4.xyxx
    r2.w = (dot((source[12].xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 97: dp2 r4.y, cb0[13].xyxx, r4.xyxx
    r4.y = (dot((source[13].xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // 98: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 99: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xywx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xywx))).xyz;
    // 102: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 103: mad r4.xyz, r2.wwww, r4.xyzx, r0.xywx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xywx)).xyz;
    // 104: mul_sat r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = (saturate((r1.wwww)*(r4.xyzx))).xyz;
    // 105: mad r5.xyz, cb0[11].zzzz, r4.xyzx, -r0.xywx
    r5.xyz = ((source[11].zzzz)*(r4.xyzx)+(-(r0.xywx))).xyz;
    // 106: mul r4.xyz, r4.xyzx, cb0[11].zzzz
    r4.xyz = ((r4.xyzx)*(source[11].zzzz)).xyz;
    // 107: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 108: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 109: mad r0.xyw, r1.wwww, r5.xyxz, r0.xyxw
    r0.xyw = ((r1.wwww)*(r5.xyxz)+(r0.xyxw)).xyw;
    // 110: mul r0.xyw, r0.xyxw, cb0[22].wwww
    r0.xyw = ((r0.xyxw)*(source[22].wwww)).xyw;
    // 111: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 112: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 114: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 115: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 116: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 117: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 118: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 119: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 120: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 121: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 122: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 123: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mad r4.xyw, r2.wwww, cb0[8].xyxz, r0.xyxw
    r4.xyw = ((r2.wwww)*(source[8].xyxz)+(r0.xyxw)).xyw;
    // 125: mad r0.xyw, r0.xyxw, cb2[3].wwww, cb2[3].xyxz
    r0.xyw = ((r0.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 126: add r2.w, -|r1.w|, l(1.000000)
    r2.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: mul_sat r1.w, r1.w, cb0[23].x
    r1.w = (saturate((r1.wwww)*(source[23].xxxx))).w;
    // 128: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: add r3.w, -|r4.z|, l(1.000000)
    r3.w = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 131: mad r5.xyz, r2.wwww, cb0[14].xyzx, -cb0[14].xyzx
    r5.xyz = ((r2.wwww)*(source[14].xyzx)+(-(source[14].xyzx))).xyz;
    // 132: mad r5.xyz, cb0[14].wwww, r5.xyzx, cb0[14].xyzx
    r5.xyz = ((source[14].wwww)*(r5.xyzx)+(source[14].xyzx)).xyz;
    // 133: add r4.xyw, r4.xyxw, r5.xyxz
    r4.xyw = ((r4.xyxw)+(r5.xyxz)).xyw;
    // 134: mul_sat r2.w, r4.z, cb0[23].x
    r2.w = (saturate((r4.zzzz)*(source[23].xxxx))).w;
    // 135: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: add_sat r2.w, r2.w, -cb0[23].y
    r2.w = (saturate((r2.wwww)+(-(source[23].yyyy)))).w;
    // 137: log r3.w, r2.w
    r3.w = (log2(r2.wwww)).w;
    // 138: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: mul r3.w, r3.w, cb0[23].z
    r3.w = ((r3.wwww)*(source[23].zzzz)).w;
    // 140: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 141: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 142: mul r1.w, r1.w, cb0[15].w
    r1.w = ((r1.wwww)*(source[15].wwww)).w;
    // 143: mul r5.xyz, r1.wwww, cb0[15].xyzx
    r5.xyz = ((r1.wwww)*(source[15].xyzx)).xyz;
    // 144: movc r5.xyz, r2.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 145: add r4.xyw, r4.xyxw, r5.xyxz
    r4.xyw = ((r4.xyxw)+(r5.xyxz)).xyw;
    // 146: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 147: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 148: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 149: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 150: mov_sat r0.z, r4.z
    r0.z = (saturate(r4.zzzz)).z;
    // 151: mul r5.xyz, r3.xyzx, r4.zzzz
    r5.xyz = ((r3.xyzx)*(r4.zzzz)).xyz;
    // 152: mad r3.xyz, r5.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r3.xyzx)).xyz;
    // 153: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 154: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 155: mul r1.xyz, r1.xyzx, r0.zzzz
    r1.xyz = ((r1.xyzx)*(r0.zzzz)).xyz;
    // 156: mul r1.xyz, r1.xyzx, cb0[20].xxxx
    r1.xyz = ((r1.xyzx)*(source[20].xxxx)).xyz;
    // 157: dp3 r0.z, r3.xyzx, r3.xyzx
    r0.z = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 158: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 159: div r0.z, r3.z, r0.z
    r0.z = ((r3.zzzz)/(r0.zzzz)).z;
    // 160: add r0.z, r0.z, cb0[7].z
    r0.z = ((r0.zzzz)+(source[7].zzzz)).z;
    // 161: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 162: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 163: mul r3.xyz, r1.wwww, v1.xyzx
    r3.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 164: dp3 r1.w, r3.xyzx, r2.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 165: add r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)+(r1.wwww)).z;
    // 166: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 167: add r0.z, r0.z, l(-0.500000)
    r0.z = ((r0.zzzz)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).z;
    // 168: add r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)+(r0.zzzz)).z;
    // 169: add r0.z, -|r0.z|, l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 170: log r1.w, r0.z
    r1.w = (log2(r0.zzzz)).w;
    // 171: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 172: mad r2.w, cb0[20].w, l(4.500000), l(0.500000)
    r2.w = ((source[20].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 173: mul r2.w, r2.w, cb0[21].x
    r2.w = ((r2.wwww)*(source[21].xxxx)).w;
    // 174: mul r2.w, r2.w, l(0.050000)
    r2.w = ((r2.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 175: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 176: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 177: mul r1.w, r1.w, cb0[21].y
    r1.w = ((r1.wwww)*(source[21].yyyy)).w;
    // 178: movc r0.z, r0.z, l(0), r1.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 179: mad r1.xyz, r0.zzzz, r1.xyzx, r4.xywx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r4.xywx)).xyz;
    // 180: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 181: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 182: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 183: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 184: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 185: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 186: mul r4.xyz, r0.zzzz, v6.xyzx
    r4.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 187: dp3 r0.z, r4.xyzx, r2.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 188: mad r4.xy, r0.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 189: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 190: mul r4.yzw, r4.yyyy, cb0[25].xxyz
    r4.yzw = ((r4.yyyy)*(source[25].xxyz)).yzw;
    // 191: mad r4.xyz, r4.xxxx, cb0[24].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[24].xyzx)+(r4.yzwy)).xyz;
    // 192: mul r4.xyz, r4.xyzx, cb0[26].wwww
    r4.xyz = ((r4.xyzx)*(source[26].wwww)).xyz;
    // 193: mad r1.xyz, r4.xyzx, r0.xywx, r1.xyzx
    r1.xyz = ((r4.xyzx)*(r0.xywx)+(r1.xyzx)).xyz;
    // 194: mul r4.xyz, r0.xywx, r4.xyzx
    r4.xyz = ((r0.xywx)*(r4.xyzx)).xyz;
    // 195: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mad o0.xyz, r0.xywx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xywx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 197: mov o3.xyz, r0.xywx
    output.targets[3].xyz = (r0.xywx).xyz;
    // 198: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 199: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 200: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 201: mul r0.xyz, r0.xxxx, v0.xyzx
    r0.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 202: mul r1.xyz, r0.yzxy, r3.zxyz
    r1.xyz = ((r0.yzxy)*(r3.zxyz)).xyz;
    // 203: mad r1.xyz, r3.yzxy, r0.zxyz, -r1.xyzx
    r1.xyz = ((r3.yzxy)*(r0.zxyz)+(-(r1.xyzx))).xyz;
    // 204: mul r1.xyz, r1.xyzx, v1.wwww
    r1.xyz = ((r1.xyzx)*(v1.wwww)).xyz;
    // 205: movc r0.w, v8.x, l(1.000000), l(-1.000000)
    r0.w = ((asuint(v8.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 206: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 207: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 208: dp3 r1.y, r1.xyzx, r2.xyzx
    r1.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 209: dp3 r1.z, r3.xyzx, r2.xyzx
    r1.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 210: dp3 r1.x, r0.xyzx, r2.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 211: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 212: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 213: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 214: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 215: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 216: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 217: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 218: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 219: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 220: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 221: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 222: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 223: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 224: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 225: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 226: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase20(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].y=(g_SourceCharacterTime.xxxx).x;
    source[19].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[19].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Existing source draw uses full material coverage and identity colour scale.
    source[0].x = 1.f; source[1].w = 1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 5: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 6: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 7: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 9: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 10: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 11: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 12: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 13: mul r1.xyz, r0.wwww, v6.zxyz
    r1.xyz = ((r0.wwww)*(v6.zxyz)).xyz;
    // 14: dp3 r0.w, r0.zxyz, r1.xyzx
    r0.w = (dot((r0.zxyz).xyz,(r1.xyzx).xyz).xxxx).w;
    // 15: mul_sat r1.y, r0.w, cb0[18].y
    r1.y = (saturate((r0.wwww)*(source[18].yyyy))).y;
    // 16: mul_sat r1.z, r1.x, cb0[18].y
    r1.z = (saturate((r1.xxxx)*(source[18].yyyy))).z;
    // 17: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 18: add_sat r1.z, r1.z, -cb0[18].z
    r1.z = (saturate((r1.zzzz)+(-(source[18].zzzz)))).z;
    // 19: log r1.w, r1.z
    r1.w = (log2(r1.zzzz)).w;
    // 20: lt r1.z, r1.z, l(0.000001)
    r1.z = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 21: mul r1.w, r1.w, cb0[18].w
    r1.w = ((r1.wwww)*(source[18].wwww)).w;
    // 22: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 23: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 24: mul r1.y, r1.y, cb0[10].w
    r1.y = ((r1.yyyy)*(source[10].wwww)).y;
    // 25: mul r2.xyz, r1.yyyy, cb0[10].xyzx
    r2.xyz = ((r1.yyyy)*(source[10].xyzx)).xyz;
    // 26: movc r1.yzw, r1.zzzz, l(0,0,0,0), r2.xxyz
    r1.yzw = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).yzw;
    // 27: add r2.x, -|r1.x|, l(1.000000)
    r2.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: add r2.y, -|r0.w|, l(1.000000)
    r2.y = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 29: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mul r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)*(r2.xxxx)).x;
    // 31: mad r2.xyz, r2.xxxx, cb0[9].xyzx, -cb0[9].xyzx
    r2.xyz = ((r2.xxxx)*(source[9].xyzx)+(-(source[9].xyzx))).xyz;
    // 32: mad r2.xyz, cb0[9].wwww, r2.xyzx, cb0[9].xyzx
    r2.xyz = ((source[9].wwww)*(r2.xyzx)+(source[9].xyzx)).xyz;
    // 33: mad r2.xyz, r0.wwww, cb0[8].xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(source[8].xyzx)+(r2.xyzx)).xyz;
    // 34: add r1.yzw, r1.yyzw, r2.xxyz
    r1.yzw = ((r1.yyzw)+(r2.xxyz)).yzw;
    // 35: add r2.xyz, v8.xyzx, cb0[0].yzwy
    r2.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 36: add r3.xyz, -r2.xyzx, cb0[0].yzwy
    r3.xyz = ((-(r2.xyzx))+(source[0].yzwy)).xyz;
    // 37: add r2.xyzw, r2.yzxy, -cb0[1].yzxy
    r2.xyzw = ((r2.yzxy)+(-(source[1].yzxy))).xyzw;
    // 38: mul r4.xyz, r1.xxxx, r3.xyzx
    r4.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 39: mov_sat r1.x, r1.x
    r1.x = (saturate(r1.xxxx)).x;
    // 40: mul r0.w, r1.x, r1.x
    r0.w = ((r1.xxxx)*(r1.xxxx)).w;
    // 41: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 42: mad r3.xyz, r4.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r3.xyzx)).xyz;
    // 43: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 44: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 45: div r1.x, r3.z, r1.x
    r1.x = ((r3.zzzz)/(r1.xxxx)).x;
    // 46: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 47: dp3 r3.x, v1.xyzx, v1.xyzx
    r3.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 48: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 49: mul r3.xyz, r3.xxxx, v1.xyzx
    r3.xyz = ((r3.xxxx)*(v1.xyzx)).xyz;
    // 50: dp3 r3.w, r3.xyzx, r0.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 51: add r1.x, r1.x, r3.w
    r1.x = ((r1.xxxx)+(r3.wwww)).x;
    // 52: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 53: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 54: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 55: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 56: log r3.w, r1.x
    r3.w = (log2(r1.xxxx)).w;
    // 57: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 58: mad r4.x, cb0[17].z, l(4.500000), l(0.500000)
    r4.x = ((source[17].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 59: mul r4.x, r4.x, cb0[17].w
    r4.x = ((r4.xxxx)*(source[17].wwww)).x;
    // 60: mul r4.x, r4.x, l(0.050000)
    r4.x = ((r4.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))).x;
    // 61: mul r3.w, r3.w, r4.x
    r3.w = ((r3.wwww)*(r4.xxxx)).w;
    // 62: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 63: mul r3.w, r3.w, cb0[18].x
    r3.w = ((r3.wwww)*(source[18].xxxx)).w;
    // 64: movc r1.x, r1.x, l(0), r3.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).x;
    // 65: add r4.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r4.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 66: mul r4.xyz, r4.xyzx, cb0[16].xxxx
    r4.xyz = ((r4.xyzx)*(source[16].xxxx)).xyz;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 68: mad r4.xyz, r5.yyyy, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r5.yyyy)*(r4.xyzx)+(source[3].xyzx)).xyz;
    // 69: dp3 r3.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: add r6.xyz, -r4.xyzx, r3.wwww
    r6.xyz = ((-(r4.xyzx))+(r3.wwww)).xyz;
    // 71: mad r4.xyz, cb0[16].yyyy, r6.xyzx, r4.xyzx
    r4.xyz = ((source[16].yyyy)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 72: dp3 r3.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r6.xyz, -r4.xyzx, r3.wwww
    r6.xyz = ((-(r4.xyzx))+(r3.wwww)).xyz;
    // 74: mad r4.xyz, cb0[16].zzzz, r6.xyzx, r4.xyzx
    r4.xyz = ((source[16].zzzz)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 75: mad r6.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mad r7.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 78: mad r7.xyz, r4.xyzx, r6.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r7.xyz = ((r4.xyzx)*(r6.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 79: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 80: mul r4.xyz, r4.xyzx, r5.xxxx
    r4.xyz = ((r4.xyzx)*(r5.xxxx)).xyz;
    // 81: dp3 r3.w, r7.xyzx, r7.xyzx
    r3.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 82: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 83: div r6.xyz, r7.xyzx, r3.wwww
    r6.xyz = ((r7.xyzx)/(r3.wwww)).xyz;
    // 84: mul r5.xyz, r5.zzzz, r6.xyzx
    r5.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 85: mul o0.w, r5.w, cb0[1].w
    output.targets[0].w = ((r5.wwww)*(source[1].wwww)).w;
    // 86: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 87: mul r5.xyz, r5.xyzx, cb0[16].wwww
    r5.xyz = ((r5.xyzx)*(source[16].wwww)).xyz;
    // 88: mad r1.xyz, r1.xxxx, r5.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(r1.yzwy)).xyz;
    // 89: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 90: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 91: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 92: mad r2.xy, cb0[13].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[13].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 93: mul r0.w, cb0[13].y, cb0[19].y
    r0.w = ((source[13].yyyy)*(source[19].yyyy)).w;
    // 94: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 95: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 96: mul r5.y, r0.w, l(0.020000)
    r5.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 97: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 99: mul r1.w, cb0[13].x, l(0.001000)
    r1.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 100: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 101: mad r2.xy, r1.wwww, r2.xyxx, r5.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r5.xyxx)).xy;
    // 102: dp2 r5.y, cb0[15].xyxx, r2.xyxx
    r5.y = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 103: dp2 r1.w, cb0[14].xyxx, r2.xyxx
    r1.w = (dot((source[14].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 104: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 105: mul r5.x, r1.w, l(0.125000)
    r5.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 107: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 108: add r2.w, -cb0[11].w, l(1.000000)
    r2.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r2.w, r2.w, cb0[19].y
    r2.w = ((r2.wwww)*(source[19].yyyy)).w;
    // 110: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 111: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 112: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mul r3.w, cb0[11].z, l(1.500000)
    r3.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 114: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 115: mad r2.w, r2.w, l(0.500000), cb0[11].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 116: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 117: mul r5.x, r3.w, l(0.125000)
    r5.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 118: mul r6.y, cb0[11].y, cb0[12].y
    r6.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 119: mov r5.y, v4.y
    r5.y = (v4.yyyy).y;
    // 120: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 121: add r5.xy, r5.xyxx, r6.xyxx
    r5.xy = ((r5.xyxx)+(r6.xyxx)).xy;
    // 122: frc r3.w, cb0[11].x
    r3.w = (frac(source[11].xxxx)).w;
    // 123: add r4.w, -r3.w, cb0[11].x
    r4.w = ((-(r3.wwww))+(source[11].xxxx)).w;
    // 124: mul r6.z, r4.w, l(0.125000)
    r6.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 125: add r5.xy, r5.xyxx, r6.zwzz
    r5.xy = ((r5.xyxx)+(r6.zwzz)).xy;
    // 126: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 127: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 128: mul r2.w, r3.w, r5.w
    r2.w = ((r3.wwww)*(r5.wwww)).w;
    // 129: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 130: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 131: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r4.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r4.xyzx))).xyz;
    // 132: mad r2.xyz, r1.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 133: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 134: mad r5.xyz, cb0[13].zzzz, r2.xyzx, -r4.xyzx
    r5.xyz = ((source[13].zzzz)*(r2.xyzx)+(-(r4.xyzx))).xyz;
    // 135: mul r2.xyz, r2.xyzx, cb0[13].zzzz
    r2.xyz = ((r2.xyzx)*(source[13].zzzz)).xyz;
    // 136: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 138: mad r2.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r2.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 139: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 140: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 141: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 142: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 143: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 144: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 145: mul r4.xyz, r0.wwww, v7.xyzx
    r4.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 146: dp3 r0.w, r4.xyzx, r0.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 147: mul r0.xyz, r0.xyzx, cb0[0].xxxx
    r0.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 148: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 149: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 150: mul r4.yzw, r4.yyyy, cb0[21].xxyz
    r4.yzw = ((r4.yyyy)*(source[21].xxyz)).yzw;
    // 151: mad r4.xyz, r4.xxxx, cb0[20].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[20].xyzx)+(r4.yzwy)).xyz;
    // 152: mul r4.xyz, r4.xyzx, cb0[22].wwww
    r4.xyz = ((r4.xyzx)*(source[22].wwww)).xyz;
    // 153: mad r1.xyz, r4.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 154: mul r4.xyz, r2.xyzx, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 155: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 156: mad r1.xyz, r2.xyzx, cb0[22].xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[22].xyzx)+(r1.xyzx)).xyz;
    // 157: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 158: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 159: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 160: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 161: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 162: mul r2.xyz, r1.yzxy, r3.zxyz
    r2.xyz = ((r1.yzxy)*(r3.zxyz)).xyz;
    // 163: mad r2.xyz, r3.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r3.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 164: dp3 r3.z, r3.xyzx, r0.xyzx
    r3.z = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 165: dp3 r3.x, r1.xyzx, r0.xyzx
    r3.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 166: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 167: dp3 r3.y, r1.xyzx, r0.xyzx
    r3.y = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 168: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 169: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 170: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 171: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 172: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 173: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 174: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 175: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 176: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 177: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 178: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 179: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 180: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 181: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 182: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 183: ret
    return output;
}


SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase21(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].z=(g_SourceCharacterTime.xxxx).x;
    source[24].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[25].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[25].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[25].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[25].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s1, l(0x00000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(0xbeaaa64c)
    r0.x = ((r0.xxxx)+(float4(asfloat(0xbeaaa64cu),asfloat(0xbeaaa64cu),asfloat(0xbeaaa64cu),asfloat(0xbeaaa64cu)))).x;
    // 4: lt r0.x, r0.x, l(0x00000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 8: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 9: add r1.x, -r0.x, l(0x447a0000)
    r1.x = ((-(r0.xxxx))+(float4(asfloat(0x447a0000u),asfloat(0x447a0000u),asfloat(0x447a0000u),asfloat(0x447a0000u)))).x;
    // 10: mad r0.x, cb0[23].w, r1.x, r0.x
    r0.x = ((source[23].wwww)*(r1.xxxx)+(r0.xxxx)).x;
    // 11: mul r0.x, r0.x, l(0x3c23d70a)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3c23d70au),asfloat(0x3c23d70au),asfloat(0x3c23d70au),asfloat(0x3c23d70au)))).x;
    // 12: mad r0.x, cb0[23].y, cb0[23].z, r0.x
    r0.x = ((source[23].yyyy)*(source[23].zzzz)+(r0.xxxx)).x;
    // 13: mul r1.x, r0.x, l(0x406191f5)
    r1.x = ((r0.xxxx)*(float4(asfloat(0x406191f5u),asfloat(0x406191f5u),asfloat(0x406191f5u),asfloat(0x406191f5u)))).x;
    // 14: sincos null, r1.x, r1.x
    r1.x = (cos(r1.xxxx)).x;
    // 15: add r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 16: mul r0.x, r0.x, l(0x3faa1c41)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3faa1c41u),asfloat(0x3faa1c41u),asfloat(0x3faa1c41u),asfloat(0x3faa1c41u)))).x;
    // 17: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 18: add r0.x, r0.x, l(0x3f800000)
    r0.x = ((r0.xxxx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 19: mad r0.x, r0.x, l(0x3f000000), cb0[23].x
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))+(source[23].xxxx)).x;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t6.xyzw, s5, l(0x00000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyz;
    // 21: mul r2.xyz, cb0[10].xyzx, cb0[22].wwww
    r2.xyz = ((source[10].xyzx)*(source[22].wwww)).xyz;
    // 22: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 24: mul r2.xyz, v7.yyyy, cb1[1].xywx
    r2.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 25: mad r2.xyz, cb1[0].xywx, v7.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v7.xxxx)+(r2.xyzx)).xyz;
    // 26: mad r2.xyz, cb1[2].xywx, v7.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v7.zzzz)+(r2.xyzx)).xyz;
    // 27: mad r2.xyz, cb1[3].xywx, v7.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v7.wwww)+(r2.xyzx)).xyz;
    // 28: div r2.xy, r2.xyxx, r2.zzzz
    r2.xy = ((r2.xyxx)/(r2.zzzz)).xy;
    // 29: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: mul r2.xy, r2.xyxx, l(0x442f0000,0x442f0000,0x00000000,0x00000000)
    r2.xy = ((r2.xyxx)*(float4(asfloat(0x442f0000u),asfloat(0x442f0000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 31: deriv_rtx_coarse r2.zw, r2.xxxy
    r2.zw = (ddx_coarse(r2.xxxy)).zw;
    // 32: deriv_rty_coarse r2.xy, r2.xyxx
    r2.xy = (ddy_coarse(r2.xyxx)).xy;
    // 33: dp2 r0.x, r2.xyxx, r2.xyxx
    r0.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 34: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 35: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 36: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 37: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 38: rcp r1.w, |r0.x|
    r1.w = (1.0/(abs(r0.xxxx))).w;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0x00000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 40: add r2.w, -r2.w, l(0x3f800000)
    r2.w = ((-(r2.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 41: log r3.x, |r2.w|
    r3.x = (log2(abs(r2.wwww))).x;
    // 42: lt r2.w, |r2.w|, l(0x358637bd)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).w;
    // 43: mul r3.x, r3.x, cb0[20].x
    r3.x = ((r3.xxxx)*(source[20].xxxx)).x;
    // 44: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 45: min r3.x, r3.x, l(0x3f800000)
    r3.x = (min(r3.xxxx,float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 46: movc r2.w, r2.w, l(0x00000000), r3.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r3.xxxx)).w;
    // 47: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 48: mul r3.x, r3.x, cb0[20].y
    r3.x = ((r3.xxxx)*(source[20].yyyy)).x;
    // 49: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: add r0.x, |r0.x|, r1.w
    r0.x = ((abs(r0.xxxx))+(r1.wwww)).x;
    // 52: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0x00000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xy;
    // 54: mad r3.xy, r3.xyxx, l(0x40000000,0x40000000,0x00000000,0x00000000), l(0xbf800000,0xbf800000,0x00000000,0x00000000)
    r3.xy = ((r3.xyxx)*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x00000000u),asfloat(0x00000000u)))+(float4(asfloat(0xbf800000u),asfloat(0xbf800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 55: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 56: mul r3.xy, r3.xyxx, cb0[19].xxxx
    r3.xy = ((r3.xyxx)*(source[19].xxxx)).xy;
    // 57: add r1.w, -r1.w, l(0x3f800000)
    r1.w = ((-(r1.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 58: max r1.w, r1.w, l(0x00000000)
    r1.w = (max(r1.wwww,float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).w;
    // 59: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 60: add r3.z, r1.w, l(0x3727c5ac)
    r3.z = ((r1.wwww)+(float4(asfloat(0x3727c5acu),asfloat(0x3727c5acu),asfloat(0x3727c5acu),asfloat(0x3727c5acu)))).z;
    // 61: add r4.xyz, -r3.xyzx, l(0x00000000,0x00000000,0x3f800000,0x00000000)
    r4.xyz = ((-(r3.xyzx))+(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 62: mad r4.xyz, cb0[19].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[19].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 63: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 64: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 65: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 66: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 67: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 68: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 69: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 70: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 71: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 72: mul r7.xyz, r1.wwww, v1.xyzx
    r7.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 73: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 74: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 75: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 76: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 77: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 78: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 79: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 80: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 81: mad r9.xyz, v5.xyzx, r1.wwww, l(0x00000000,0x00000000,0x3f800000,0x00000000)
    r9.xyz = ((v5.xyzx)*(r1.wwww)+(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 82: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 83: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 84: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 85: dp3 r1.w, r6.xyzx, r10.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 86: mul r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 87: mad r6.xyz, r6.xyzx, l(0x40000000,0x40000000,0x40000000,0x00000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x00000000u)))+(-(r10.xyzx))).xyz;
    // 88: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 89: dp2 r1.w, r6.ywyy, r6.ywyy
    r1.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 90: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 91: div r6.xy, r6.ywyy, r1.wwww
    r6.xy = ((r6.ywyy)/(r1.wwww)).xy;
    // 92: mad r1.w, -r6.z, l(0x3e800000), l(0x3e800000)
    r1.w = ((-(r6.zzzz))*(float4(asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u)))+(float4(asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u)))).w;
    // 93: add r3.w, r6.z, l(0x3f800000)
    r3.w = ((r6.zzzz)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 94: mul r3.w, r3.w, l(0x3f000000)
    r3.w = ((r3.wwww)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).w;
    // 95: mad r6.xy, r1.wwww, r6.xyxx, l(0x3f000000,0x3f000000,0x00000000,0x00000000)
    r6.xy = ((r1.wwww)*(r6.xyxx)+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t3.xyzw, s3, r0.x
    r6.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 97: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 98: rcp r0.x, cb0[20].z
    r0.x = (1.0/(source[20].zzzz)).x;
    // 99: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 100: mul r10.xyz, r10.xyzx, cb0[20].zzzz
    r10.xyz = ((r10.xyzx)*(source[20].zzzz)).xyz;
    // 101: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 102: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 103: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 104: mad r10.xyz, r10.xyzx, cb0[20].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[20].zzzz)+(r11.xyzx)).xyz;
    // 105: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 106: mul r6.xyz, r6.xyzx, l(0x3eaaaa9f,0x3eaaaa9f,0x3eaaaa9f,0x00000000)
    r6.xyz = ((r6.xyzx)*(float4(asfloat(0x3eaaaa9fu),asfloat(0x3eaaaa9fu),asfloat(0x3eaaaa9fu),asfloat(0x00000000u)))).xyz;
    // 107: add r0.x, cb0[20].z, l(0x3f800000)
    r0.x = ((source[20].zzzz)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 108: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 109: dp3 r0.x, r6.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 110: add r6.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r6.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 111: mad r6.xyz, r3.wwww, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[8].xyzx)).xyz;
    // 112: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 113: mul r6.xyz, r6.xyzx, cb0[20].wwww
    r6.xyz = ((r6.xyzx)*(source[20].wwww)).xyz;
    // 114: dp3 r0.x, r2.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 115: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 116: mad r2.xyz, cb0[19].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 117: dp3 r0.x, r2.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 118: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 119: mad r2.xyz, cb0[19].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 120: dp3 r0.x, r2.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 121: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 122: mul r10.xyz, r10.xyzx, cb0[21].xxxx
    r10.xyz = ((r10.xyzx)*(source[21].xxxx)).xyz;
    // 123: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t4.xyzw, s4, l(0x00000000)
    r11.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 124: add r0.x, r11.y, r11.x
    r0.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 125: add r0.x, r11.z, r0.x
    r0.x = ((r11.zzzz)+(r0.xxxx)).x;
    // 126: add_sat r0.x, r11.w, r0.x
    r0.x = (saturate((r11.wwww)+(r0.xxxx))).x;
    // 127: mad r2.xyz, r0.xxxx, r10.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 128: max r10.xyz, |r2.xyzx|, l(0x358637bd,0x358637bd,0x358637bd,0x00000000)
    r10.xyz = (max(abs(r2.xyzx),float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x00000000u)))).xyz;
    // 129: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 130: mul r10.xyz, r10.xyzx, l(0x3ee8ba1f,0x3ee8ba1f,0x3ee8ba1f,0x00000000)
    r10.xyz = ((r10.xyzx)*(float4(asfloat(0x3ee8ba1fu),asfloat(0x3ee8ba1fu),asfloat(0x3ee8ba1fu),asfloat(0x00000000u)))).xyz;
    // 131: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 132: dp3 r0.x, r10.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 133: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 134: mul r0.x, r0.x, cb0[22].x
    r0.x = ((r0.xxxx)*(source[22].xxxx)).x;
    // 135: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 136: min r0.x, r0.x, l(0x3f800000)
    r0.x = (min(r0.xxxx,float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 137: mad r1.w, -r0.x, r0.x, l(0x3f800000)
    r1.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 138: max r1.w, r1.w, l(0x3a83126f)
    r1.w = (max(r1.wwww,float4(asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu)))).w;
    // 139: div r1.w, cb0[22].y, r1.w
    r1.w = ((source[22].yyyy)/(r1.wwww)).w;
    // 140: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 141: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 142: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 143: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 144: mul_sat r4.w, r3.w, cb0[21].y
    r4.w = (saturate((r3.wwww)*(source[21].yyyy))).w;
    // 145: add r3.w, -|r3.w|, l(0x3f800000)
    r3.w = ((-(abs(r3.wwww)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 146: add r4.w, -r4.w, l(0x3f800000)
    r4.w = ((-(r4.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 147: mul_sat r5.w, r4.z, cb0[21].y
    r5.w = (saturate((r4.zzzz)*(source[21].yyyy))).w;
    // 148: add r5.w, -r5.w, l(0x3f800000)
    r5.w = ((-(r5.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 149: add_sat r5.w, r5.w, -cb0[21].z
    r5.w = (saturate((r5.wwww)+(-(source[21].zzzz)))).w;
    // 150: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 151: lt r5.w, r5.w, l(0x358637bd)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).w;
    // 152: mul r6.w, r6.w, cb0[21].w
    r6.w = ((r6.wwww)*(source[21].wwww)).w;
    // 153: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 154: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 155: movc r4.w, r5.w, l(0x00000000), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r4.wwww)).w;
    // 156: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 157: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 158: dp3 r1.w, r0.yzwy, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 159: add r11.xyz, -r0.yzwy, r1.wwww
    r11.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 160: mad r0.yzw, cb0[19].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 161: dp3 r1.w, r0.yzwy, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 162: add r11.xyz, -r0.yzwy, r1.wwww
    r11.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 163: mad r0.yzw, cb0[19].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 164: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 165: dp3 r1.w, r11.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 166: mad r12.xyz, -cb0[5].wwww, cb0[5].xyzx, r1.wwww
    r12.xyz = ((-(source[5].wwww))*(source[5].xyzx)+(r1.wwww)).xyz;
    // 167: mad r11.xyz, cb0[19].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 168: dp3 r1.w, r11.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 169: add r12.xyz, -r11.xyzx, r1.wwww
    r12.xyz = ((-(r11.xyzx))+(r1.wwww)).xyz;
    // 170: mad r11.xyz, cb0[19].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 171: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(0x3f800000,0x3f800000,0x3f800000,0x00000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 172: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, l(0x3f800000,0x3f800000,0x3f800000,0x00000000)
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 173: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 174: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 175: mul r13.xyz, r0.yzwy, r11.xyzx
    r13.xyz = ((r0.yzwy)*(r11.xyzx)).xyz;
    // 176: mad r0.yzw, r11.xxyz, r0.yyzw, l(0x00000000,0x3c23d70a,0x3c23d70a,0x3c23d70a)
    r0.yzw = ((r11.xxyz)*(r0.yyzw)+(float4(asfloat(0x00000000u),asfloat(0x3c23d70au),asfloat(0x3c23d70au),asfloat(0x3c23d70au)))).yzw;
    // 177: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 178: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 179: add r1.w, -r0.x, l(0x3f800000)
    r1.w = ((-(r0.xxxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 180: mul r1.w, r1.w, cb0[22].z
    r1.w = ((r1.wwww)*(source[22].zzzz)).w;
    // 181: mad r2.xyz, r1.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 182: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 183: add r5.w, -r1.w, l(0x3f800000)
    r5.w = ((-(r1.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 184: mad r1.xyz, r5.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 185: dp3 r5.w, r1.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 186: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 187: mad r1.xyz, cb0[19].yyyy, r6.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 188: dp3 r5.w, r1.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 189: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 190: mad r1.xyz, cb0[19].zzzz, r6.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 191: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 192: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 193: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 194: dp3 r5.w, r0.yzwy, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 195: add r6.xyz, -r0.yzwy, r5.wwww
    r6.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 196: add r0.yzw, r0.yyzw, -r6.xxyz
    r0.yzw = ((r0.yyzw)+(-(r6.xxyz))).yzw;
    // 197: mul r6.xyz, cb0[13].xyzx, cb0[24].yyyy
    r6.xyz = ((source[13].xyzx)*(source[24].yyyy)).xyz;
    // 198: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 199: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 200: mad r10.xyz, r4.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r10.xyz = ((r4.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 201: add r4.w, r4.w, l(0xbf800000)
    r4.w = ((r4.wwww)+(float4(asfloat(0xbf800000u),asfloat(0xbf800000u),asfloat(0xbf800000u),asfloat(0xbf800000u)))).w;
    // 202: mad r4.w, cb0[11].w, r4.w, l(0x3f800000)
    r4.w = ((source[11].wwww)*(r4.wwww)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 203: mad r10.xyz, cb0[12].wwww, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((source[12].wwww)*(r10.xyzx)+(source[12].xyzx)).xyz;
    // 204: mad r0.yzw, r0.yyzw, r6.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r6.xxyz)+(r10.xxyz)).yzw;
    // 205: mad r0.yzw, r4.wwww, cb0[11].xxyz, r0.yyzw
    r0.yzw = ((r4.wwww)*(source[11].xxyz)+(r0.yyzw)).yzw;
    // 206: mad r0.yzw, r1.xxyz, r12.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 207: add r1.x, -|r4.z|, l(0x3f800000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 208: mul r1.x, r3.w, r1.x
    r1.x = ((r3.wwww)*(r1.xxxx)).x;
    // 209: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 210: lt r1.x, |r1.x|, l(0x358637bd)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).x;
    // 211: mul r1.y, r1.y, l(0x3fc00000)
    r1.y = ((r1.yyyy)*(float4(asfloat(0x3fc00000u),asfloat(0x3fc00000u),asfloat(0x3fc00000u),asfloat(0x3fc00000u)))).y;
    // 212: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 213: mul r6.xyz, r1.yyyy, cb0[14].xyzx
    r6.xyz = ((r1.yyyy)*(source[14].xyzx)).xyz;
    // 214: movc r1.xyz, r1.xxxx, l(0x00000000,0x00000000,0x00000000,0x00000000), r6.xyzx
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r6.xyzx)).xyz;
    // 215: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 216: add r0.yzw, r0.yyzw, cb0[3].xxyz
    r0.yzw = ((r0.yyzw)+(source[3].xxyz)).yzw;
    // 217: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 218: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 219: div r6.xyz, r9.xyzx, r1.yyyy
    r6.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 220: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 221: add r1.y, -r1.y, l(0x3f800000)
    r1.y = ((-(r1.yyyy))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
    // 222: mul r1.z, |r1.y|, |r1.y|
    r1.z = ((abs(r1.yyyy))*(abs(r1.yyyy))).z;
    // 223: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 224: mul r1.z, r1.z, |r1.y|
    r1.z = ((r1.zzzz)*(abs(r1.yyyy))).z;
    // 225: lt r1.y, |r1.y|, l(0x358637bd)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).y;
    // 226: movc r1.y, r1.y, l(0x00000000), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r1.zzzz)).y;
    // 227: add r1.z, r1.y, l(0xbce38eb0)
    r1.z = ((r1.yyyy)+(float4(asfloat(0xbce38eb0u),asfloat(0xbce38eb0u),asfloat(0xbce38eb0u),asfloat(0xbce38eb0u)))).z;
    // 228: mad r1.y, r1.y, r1.z, l(0x3ce38eb0)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(asfloat(0x3ce38eb0u),asfloat(0x3ce38eb0u),asfloat(0x3ce38eb0u),asfloat(0x3ce38eb0u)))).y;
    // 229: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 230: add r1.x, -r1.x, l(0x3f800000)
    r1.x = ((-(r1.xxxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 231: mul r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)*(r2.wwww)).x;
    // 232: mad r1.xyz, r1.xxxx, r2.xyzx, -r13.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 233: mad r1.xyz, r0.xxxx, r1.xyzx, r13.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 234: dp3 r0.x, r1.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 235: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 236: mad r1.xyz, cb0[19].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 237: dp3 r0.x, r1.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 238: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 239: mad r1.xyz, cb0[19].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 240: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 241: add r0.x, -cb0[4].w, l(0x3f800000)
    r0.x = ((-(source[4].wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 242: mul r0.x, r0.x, cb0[23].z
    r0.x = ((r0.xxxx)*(source[23].zzzz)).x;
    // 243: mul r0.x, r0.x, l(0x40c90fdb)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu)))).x;
    // 244: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 245: add r0.x, r0.x, l(0x3f800000)
    r0.x = ((r0.xxxx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 246: mul r2.x, cb0[4].z, l(0x3fc00000)
    r2.x = ((source[4].zzzz)*(float4(asfloat(0x3fc00000u),asfloat(0x3fc00000u),asfloat(0x3fc00000u),asfloat(0x3fc00000u)))).x;
    // 247: mul r0.x, r0.x, r2.x
    r0.x = ((r0.xxxx)*(r2.xxxx)).x;
    // 248: mad r0.x, r0.x, l(0x3f000000), cb0[4].z
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))+(source[4].zzzz)).x;
    // 249: add r2.x, -r1.w, cb0[4].x
    r2.x = ((-(r1.wwww))+(source[4].xxxx)).x;
    // 250: mul r2.z, r2.x, l(0x3e000000)
    r2.z = ((r2.xxxx)*(float4(asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u)))).z;
    // 251: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 252: mul r4.x, r3.w, l(0x3e000000)
    r4.x = ((r3.wwww)*(float4(asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u)))).x;
    // 253: mul r2.y, cb0[4].y, cb0[15].y
    r2.y = ((source[4].yyyy)*(source[15].yyyy)).y;
    // 254: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 255: mov r2.xw, l(0x00000000,0x00000000,0x00000000,0x00000000)
    r2.xw = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).xw;
    // 256: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 257: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 258: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0x00000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 259: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 260: mul r0.x, r1.w, r2.w
    r0.x = ((r1.wwww)*(r2.wwww)).x;
    // 261: mad r2.xyz, r2.xyzx, l(0x40000000,0x40000000,0x40000000,0x00000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x00000000u)))+(-(r1.xyzx))).xyz;
    // 262: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 263: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 264: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 265: add r2.xy, -r2.xyxx, l(0x3f800000,0x3f800000,0x00000000,0x00000000)
    r2.xy = ((-(r2.xyxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 266: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 267: mad r2.xy, cb0[16].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[16].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 268: mul r0.x, cb0[16].y, cb0[23].z
    r0.x = ((source[16].yyyy)*(source[23].zzzz)).x;
    // 269: mul r0.x, r0.x, l(0x3f20d97c)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3f20d97cu),asfloat(0x3f20d97cu),asfloat(0x3f20d97cu),asfloat(0x3f20d97cu)))).x;
    // 270: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 271: mul r4.y, r0.x, l(0x3ca3d70b)
    r4.y = ((r0.xxxx)*(float4(asfloat(0x3ca3d70bu),asfloat(0x3ca3d70bu),asfloat(0x3ca3d70bu),asfloat(0x3ca3d70bu)))).y;
    // 272: add r0.x, r0.x, l(0x3f800000)
    r0.x = ((r0.xxxx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 273: mul r0.x, r0.x, l(0x3f000000)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
    // 274: mul r1.w, cb0[16].x, l(0x3a83126f)
    r1.w = ((source[16].xxxx)*(float4(asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu)))).w;
    // 275: mov r4.x, l(0x00000000)
    r4.x = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x;
    // 276: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 277: dp2 r1.w, cb0[17].xyxx, r2.xyxx
    r1.w = (dot((source[17].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 278: dp2 r2.y, cb0[18].xyxx, r2.xyxx
    r2.y = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 279: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 280: mul r2.x, r1.w, l(0x3e000000)
    r2.x = ((r1.wwww)*(float4(asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u)))).x;
    // 281: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0x00000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 282: mad r2.xyz, r2.xyzx, l(0x40600000,0x40600000,0x40600000,0x00000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(asfloat(0x40600000u),asfloat(0x40600000u),asfloat(0x40600000u),asfloat(0x00000000u)))+(-(r1.xyzx))).xyz;
    // 283: mul r1.w, r2.w, l(0x3f666666)
    r1.w = ((r2.wwww)*(float4(asfloat(0x3f666666u),asfloat(0x3f666666u),asfloat(0x3f666666u),asfloat(0x3f666666u)))).w;
    // 284: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 285: mul_sat r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (saturate((r0.xxxx)*(r2.xyzx))).xyz;
    // 286: mad r4.xyz, cb0[16].zzzz, r2.xyzx, -r1.xyzx
    r4.xyz = ((source[16].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 287: mul r2.xyz, r2.xyzx, cb0[16].zzzz
    r2.xyz = ((r2.xyzx)*(source[16].zzzz)).xyz;
    // 288: dp3 r0.x, r2.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).x;
    // 289: mul r0.x, r0.x, l(0x40400000)
    r0.x = ((r0.xxxx)*(float4(asfloat(0x40400000u),asfloat(0x40400000u),asfloat(0x40400000u),asfloat(0x40400000u)))).x;
    // 290: mad r1.xyz, r0.xxxx, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 291: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 292: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 293: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 294: mul r2.xyz, r0.xxxx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 295: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 296: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 297: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 298: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 299: mad r3.xy, r0.xxxx, l(0x3f000000,0xbf000000,0x00000000,0x00000000), l(0x3f000000,0x3f000000,0x00000000,0x00000000)
    r3.xy = ((r0.xxxx)*(float4(asfloat(0x3f000000u),asfloat(0xbf000000u),asfloat(0x00000000u),asfloat(0x00000000u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 300: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 301: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 302: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 303: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 304: mad r0.xyz, r3.xyzx, r1.xyzx, r0.yzwy
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 305: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 306: dp3 o4.y, r3.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).y;
    // 307: mad o0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 308: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 309: mov o0.w, l(0x00000000)
    output.targets[0].w = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).w;
    // 310: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 311: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 312: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 313: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 314: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 315: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 316: ge r0.w, l(0x00000000), r0.z
    r0.w = (asfloat((uint4)((float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 317: dp3 r0.z, l(0x3f800000,0x3f800000,0x3f800000,0x00000000), |r0.xyzx|
    r0.z = (dot((float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u))).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 318: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 319: ge r1.xy, r0.xyxx, l(0x00000000,0x00000000,0x00000000,0x00000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))) * 0xffffffffu)).xy;
    // 320: movc r1.xy, r1.xyxx, l(0x3f800000,0x3f800000,0x00000000,0x00000000), l(0xbf800000,0xbf800000,0x00000000,0x00000000)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (float4(asfloat(0xbf800000u),asfloat(0xbf800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 321: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 322: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 323: mad o2.xy, r0.xyxx, l(0x3f000000,0x3f000000,0x00000000,0x00000000), l(0x3f000000,0x3f000000,0x00000000,0x00000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x00000000u),asfloat(0x00000000u)))+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 324: mov o2.zw, l(0x00000000,0x00000000,0x3f800000,0x00000000)
    output.targets[2].zw = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x3f800000u),asfloat(0x00000000u))).zw;
    // 325: mov o3.w, l(0x00000000)
    output.targets[3].w = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).w;
    // 326: mov o4.xzw, l(0x00000000,0x00000000,0x00000000,0x00000000)
    output.targets[4].xzw = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).xzw;
    // 327: mov o5.xyzw, l(0x00000000,0x00000000,0x00000000,0x00000000)
    output.targets[5].xyzw = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).xyzw;
    // 328: ret
    return output;
}


// source.character.monster-7493dcdfd412.v1 / source program cadfe3f99e0f6741839ddeb2a3adf933
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase22(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].w=(g_SourceCharacterTime.xxxx).x;
    source[17].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[17].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 4: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: mul r1.xyz, cb0[9].xyzx, cb0[17].wwww
    r1.xyz = ((source[9].xyzx)*(source[17].wwww)).xyz;
    // 8: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 9: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 10: mad r0.xyz, -r0.yzwy, r1.xyzx, r0.xxxx
    r0.xyz = ((-(r0.yzwy))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 11: mad r0.xyz, cb0[15].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[15].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 12: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[15].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[15].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 15: mad r1.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 16: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 17: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 18: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 19: add r0.w, -cb0[10].w, l(1.000000)
    r0.w = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 21: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 22: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 23: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r1.x, cb0[10].z, l(1.500000)
    r1.x = ((source[10].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 25: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 26: mad r0.w, r0.w, l(0.500000), cb0[10].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 27: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 28: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 29: mul r2.y, cb0[10].y, cb0[11].y
    r2.y = ((source[10].yyyy)*(source[11].yyyy)).y;
    // 30: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 31: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 32: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 33: frc r1.z, cb0[10].x
    r1.z = (frac(source[10].xxxx)).z;
    // 34: add r1.w, -r1.z, cb0[10].x
    r1.w = ((-(r1.zzzz))+(source[10].xxxx)).w;
    // 35: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 36: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul r1.xyw, r0.wwww, r2.xyxz
    r1.xyw = ((r0.wwww)*(r2.xyxz)).xyw;
    // 39: mul r0.w, r1.z, r2.w
    r0.w = ((r1.zzzz)*(r2.wwww)).w;
    // 40: mad r1.xyz, r1.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 41: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 42: add r1.xyzw, v7.yzxy, cb0[0].yzxy
    r1.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 43: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 44: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 45: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 46: mad r1.xy, cb0[12].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[12].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 47: mul r0.w, cb0[12].y, cb0[16].w
    r0.w = ((source[12].yyyy)*(source[16].wwww)).w;
    // 48: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 49: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 50: mul r2.y, r0.w, l(0.020000)
    r2.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 51: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 53: mul r1.z, cb0[12].x, l(0.001000)
    r1.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 54: mov r2.x, l(0)
    r2.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 55: mad r1.xy, r1.zzzz, r1.xyxx, r2.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r2.xyxx)).xy;
    // 56: dp2 r1.z, cb0[13].xyxx, r1.xyxx
    r1.z = (dot((source[13].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 57: dp2 r1.y, cb0[14].xyxx, r1.xyxx
    r1.y = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 58: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 59: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 61: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 62: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 63: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 64: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 65: mad r2.xyz, cb0[12].zzzz, r1.xyzx, -r0.xyzx
    r2.xyz = ((source[12].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 66: mul r1.xyz, r1.xyzx, cb0[12].zzzz
    r1.xyz = ((r1.xyzx)*(source[12].zzzz)).xyz;
    // 67: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 68: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 69: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 70: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 71: mul r1.xyz, cb0[8].xyzx, cb0[16].zzzz
    r1.xyz = ((source[8].xyzx)*(source[16].zzzz)).xyz;
    // 72: mul r1.xyz, r1.xyzx, cb0[17].yyyy
    r1.xyz = ((r1.xyzx)*(source[17].yyyy)).xyz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 74: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 75: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 76: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 78: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 79: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 80: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 81: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 82: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 83: mul r3.xyz, r0.wwww, r2.xyzx
    r3.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 84: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 85: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 86: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 87: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 88: dp3 r0.w, r2.xyzx, r4.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 89: add r1.w, -|r4.z|, l(1.000000)
    r1.w = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 92: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 93: mul r1.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 94: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 95: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 96: mul r1.xyz, r1.xyzx, cb0[17].zzzz
    r1.xyz = ((r1.xyzx)*(source[17].zzzz)).xyz;
    // 97: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 98: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 99: mad_sat r1.w, r0.w, cb0[15].z, -cb0[15].w
    r1.w = (saturate((r0.wwww)*(source[15].zzzz)+(-(source[15].wwww)))).w;
    // 100: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 101: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 102: mul r2.x, r2.x, cb0[16].x
    r2.x = ((r2.xxxx)*(source[16].xxxx)).x;
    // 103: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 104: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 105: mad r2.xyz, r1.wwww, cb0[7].xyzx, -cb0[7].xyzx
    r2.xyz = ((r1.wwww)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 106: mad r4.xyz, r1.wwww, cb0[6].xyzx, -cb0[6].xyzx
    r4.xyz = ((r1.wwww)*(source[6].xyzx)+(-(source[6].xyzx))).xyz;
    // 107: mad r4.xyz, cb0[6].wwww, r4.xyzx, cb0[6].xyzx
    r4.xyz = ((source[6].wwww)*(r4.xyzx)+(source[6].xyzx)).xyz;
    // 108: mad r2.xyz, cb0[7].wwww, r2.xyzx, cb0[7].xyzx
    r2.xyz = ((source[7].wwww)*(r2.xyzx)+(source[7].xyzx)).xyz;
    // 109: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 110: mad r1.xyz, cb0[16].yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((source[16].yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 111: add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // 112: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 113: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 114: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 115: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 116: mul r2.xyz, r1.wwww, cb0[3].xyzx
    r2.xyz = ((r1.wwww)*(source[3].xyzx)).xyz;
    // 117: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 118: mad r1.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xyzx)).xyz;
    // 119: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 120: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 121: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 122: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 123: dp3 r0.w, r2.xyzx, r3.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 124: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 125: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 126: mul r2.yzw, r2.yyyy, cb0[19].xxyz
    r2.yzw = ((r2.yyyy)*(source[19].xxyz)).yzw;
    // 127: mad r2.xyz, r2.xxxx, cb0[18].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[18].xyzx)+(r2.yzwy)).xyz;
    // 128: mul r2.xyz, r2.xyzx, cb0[20].wwww
    r2.xyz = ((r2.xyzx)*(source[20].wwww)).xyz;
    // 129: mad r1.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 130: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 131: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 132: mad o0.xyz, r0.xyzx, cb0[20].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)+(r1.xyzx)).xyz;
    // 133: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 134: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 135: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 136: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 137: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 138: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 139: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 140: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 141: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 142: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 143: dp3 r0.z, r0.xyzx, r3.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 144: dp3 r0.x, r1.xyzx, r3.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 145: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 146: dp3 r0.y, r1.xyzx, r3.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 147: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 148: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 149: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 150: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 151: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 152: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 153: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 154: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 155: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 156: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 157: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 158: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 159: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 160: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 161: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 162: ret
    return output;
}

// source.character.monster-d9d6c02905c3.v1 / source program b23ac74d1d62834dbffbd595ccc49376
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase23(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[21]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[27].z=(g_SourceCharacterTime.xxxx).x;
    source[28].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[29].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[29].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[29].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[29].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[22].xyzw
    r1.xyzw = ((r0.xyzw)*(source[22].xyzw)).xyzw;
    // 3: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 4: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 5: add r0.yz, r1.yywy, r1.xxzx
    r0.yz = ((r1.yywy)+(r1.xxzx)).yz;
    // 6: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 7: add r0.y, r0.y, l(-1.000000)
    r0.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 8: mad_sat r0.x, r0.x, r0.y, l(1.000000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 11: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 12: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 13: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 14: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 15: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 16: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 17: add r0.y, -r0.x, l(1000.000000)
    r0.y = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).y;
    // 18: mad r0.x, cb0[27].w, r0.y, r0.x
    r0.x = ((source[27].wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 19: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 20: mad r0.x, cb0[27].y, cb0[27].z, r0.x
    r0.x = ((source[27].yyyy)*(source[27].zzzz)+(r0.xxxx)).x;
    // 21: mul r0.y, r0.x, l(3.524534)
    r0.y = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).y;
    // 22: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 23: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 24: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 25: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 26: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: mad r0.x, r0.x, l(0.500000), cb0[27].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[27].xxxx)).x;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v4.xyxx, t7.wxyz, s5, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 29: mul r2.xyz, cb0[13].xyzx, cb0[26].wwww
    r2.xyz = ((source[13].xyzx)*(source[26].wwww)).xyz;
    // 30: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 31: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 32: mul r2.xyz, v7.yyyy, cb1[1].xywx
    r2.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 33: mad r2.xyz, cb1[0].xywx, v7.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v7.xxxx)+(r2.xyzx)).xyz;
    // 34: mad r2.xyz, cb1[2].xywx, v7.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v7.zzzz)+(r2.xyzx)).xyz;
    // 35: mad r2.xyz, cb1[3].xywx, v7.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v7.wwww)+(r2.xyzx)).xyz;
    // 36: div r2.xy, r2.xyxx, r2.zzzz
    r2.xy = ((r2.xyxx)/(r2.zzzz)).xy;
    // 37: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: mul r2.xy, r2.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 39: deriv_rtx_coarse r2.zw, r2.xxxy
    r2.zw = (ddx_coarse(r2.xxxy)).zw;
    // 40: deriv_rty_coarse r2.xy, r2.xyxx
    r2.xy = (ddy_coarse(r2.xyxx)).xy;
    // 41: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 42: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 43: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 44: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 45: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 46: rcp r1.w, |r0.w|
    r1.w = (1.0/(abs(r0.wwww))).w;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: log r3.x, |r2.w|
    r3.x = (log2(abs(r2.wwww))).x;
    // 50: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 51: mul r3.x, r3.x, cb0[24].x
    r3.x = ((r3.xxxx)*(source[24].xxxx)).x;
    // 52: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 53: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: movc r2.w, r2.w, l(0), r3.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 55: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 56: mul r3.x, r3.x, cb0[24].y
    r3.x = ((r3.xxxx)*(source[24].yyyy)).x;
    // 57: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 58: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 59: add r0.w, |r0.w|, r1.w
    r0.w = ((abs(r0.wwww))+(r1.wwww)).w;
    // 60: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 62: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 63: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 64: mul r3.xy, r3.xyxx, cb0[23].xxxx
    r3.xy = ((r3.xyxx)*(source[23].xxxx)).xy;
    // 65: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 67: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 68: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 69: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 70: mad r4.xyz, cb0[23].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[23].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 71: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 72: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 73: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 74: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 75: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 76: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 77: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 78: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 79: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 80: mul r7.xyz, r1.wwww, v1.xyzx
    r7.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 81: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 82: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 83: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 84: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 85: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 86: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 87: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 88: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 89: mad r9.xyz, v5.xyzx, r1.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r1.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 90: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 91: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 92: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 93: dp3 r1.w, r6.xyzx, r10.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 94: mul r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 95: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 96: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 97: dp2 r1.w, r6.ywyy, r6.ywyy
    r1.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 98: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 99: div r6.xy, r6.ywyy, r1.wwww
    r6.xy = ((r6.ywyy)/(r1.wwww)).xy;
    // 100: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 101: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 103: mad r6.xy, r1.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 104: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t5.xyzw, s4, r0.w
    r6.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.wwww).x)).xyzw).xyz;
    // 105: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 106: rcp r0.w, cb0[24].z
    r0.w = (1.0/(source[24].zzzz)).w;
    // 107: mul r11.xyz, r10.xyzx, r0.wwww
    r11.xyz = ((r10.xyzx)*(r0.wwww)).xyz;
    // 108: mul r10.xyz, r10.xyzx, cb0[24].zzzz
    r10.xyz = ((r10.xyzx)*(source[24].zzzz)).xyz;
    // 109: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 110: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 111: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 112: mad r10.xyz, r10.xyzx, cb0[24].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[24].zzzz)+(r11.xyzx)).xyz;
    // 113: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 114: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 115: add r0.w, cb0[24].z, l(1.000000)
    r0.w = ((source[24].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 117: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 118: add r6.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r6.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 119: mad r6.xyz, r3.wwww, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[11].xyzx)).xyz;
    // 120: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 121: mul r6.xyz, r6.xyzx, cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(source[24].wwww)).xyz;
    // 122: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 123: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 124: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 125: dp3 r0.w, r3.xyzx, r4.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 126: mul_sat r1.w, r0.w, cb0[25].y
    r1.w = (saturate((r0.wwww)*(source[25].yyyy))).w;
    // 127: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: mul_sat r3.w, r4.z, cb0[25].y
    r3.w = (saturate((r4.zzzz)*(source[25].yyyy))).w;
    // 130: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: add_sat r3.w, r3.w, -cb0[25].z
    r3.w = (saturate((r3.wwww)+(-(source[25].zzzz)))).w;
    // 132: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 133: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 134: mul r4.w, r4.w, cb0[25].w
    r4.w = ((r4.wwww)*(source[25].wwww)).w;
    // 135: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 136: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 137: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 138: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 139: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 140: mad r2.xyz, cb0[23].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[23].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 141: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 143: mad r2.xyz, cb0[23].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[23].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 144: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 146: mul r10.xyz, r10.xyzx, cb0[25].xxxx
    r10.xyz = ((r10.xyzx)*(source[25].xxxx)).xyz;
    // 147: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 148: add r3.w, r11.y, r11.x
    r3.w = ((r11.yyyy)+(r11.xxxx)).w;
    // 149: add r3.w, r11.z, r3.w
    r3.w = ((r11.zzzz)+(r3.wwww)).w;
    // 150: add_sat r3.w, r11.w, r3.w
    r3.w = (saturate((r11.wwww)+(r3.wwww))).w;
    // 151: mad r2.xyz, r3.wwww, r10.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 152: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 153: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 154: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 155: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 156: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 157: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 158: mul r3.w, r3.w, cb0[26].x
    r3.w = ((r3.wwww)*(source[26].xxxx)).w;
    // 159: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 160: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 163: div r4.w, cb0[26].y, r4.w
    r4.w = ((source[26].yyyy)/(r4.wwww)).w;
    // 164: mul r4.w, r1.w, r4.w
    r4.w = ((r1.wwww)*(r4.wwww)).w;
    // 165: mul r10.xyz, r6.xyzx, r4.wwww
    r10.xyz = ((r6.xyzx)*(r4.wwww)).xyz;
    // 166: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 167: add r12.xyz, -r1.xyzx, r4.wwww
    r12.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 168: mad r1.xyz, cb0[23].yyyy, r12.xyzx, r1.xyzx
    r1.xyz = ((source[23].yyyy)*(r12.xyzx)+(r1.xyzx)).xyz;
    // 169: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: add r12.xyz, -r1.xyzx, r4.wwww
    r12.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 171: mad r1.xyz, cb0[23].zzzz, r12.xyzx, r1.xyzx
    r1.xyz = ((source[23].zzzz)*(r12.xyzx)+(r1.xyzx)).xyz;
    // 172: mul r12.xyz, cb0[5].xyzx, cb0[5].wwww
    r12.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 173: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, -r12.xyzx
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r12.xyzx))).xyz;
    // 174: mad r12.xyz, r11.xxxx, r13.xyzx, r12.xyzx
    r12.xyz = ((r11.xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 175: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, -r12.xyzx
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r12.xyzx))).xyz;
    // 176: mad r11.xyw, r11.yyyy, r13.xyxz, r12.xyxz
    r11.xyw = ((r11.yyyy)*(r13.xyxz)+(r12.xyxz)).xyw;
    // 177: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, -r11.xywx
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r11.xywx))).xyz;
    // 178: mad r11.xyz, r11.zzzz, r12.xyzx, r11.xywx
    r11.xyz = ((r11.zzzz)*(r12.xyzx)+(r11.xywx)).xyz;
    // 179: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 180: add r12.xyz, -r11.xyzx, r4.wwww
    r12.xyz = ((-(r11.xyzx))+(r4.wwww)).xyz;
    // 181: mad r11.xyz, cb0[23].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[23].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 182: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 183: add r12.xyz, -r11.xyzx, r4.wwww
    r12.xyz = ((-(r11.xyzx))+(r4.wwww)).xyz;
    // 184: mad r11.xyz, cb0[23].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[23].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 185: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 186: mad r13.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 188: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 189: mul r13.xyz, r1.xyzx, r11.xyzx
    r13.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 190: mad r1.xyz, r11.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r11.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 191: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 192: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 193: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 194: mul r4.w, r4.w, cb0[26].z
    r4.w = ((r4.wwww)*(source[26].zzzz)).w;
    // 195: mad r2.xyz, r4.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r4.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 196: frc r4.w, cb0[4].x
    r4.w = (frac(source[4].xxxx)).w;
    // 197: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 198: mad r0.xyz, r5.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r5.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 199: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 200: add r6.xyz, -r0.xyzx, r5.wwww
    r6.xyz = ((-(r0.xyzx))+(r5.wwww)).xyz;
    // 201: mad r0.xyz, cb0[23].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[23].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 202: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 203: add r6.xyz, -r0.xyzx, r5.wwww
    r6.xyz = ((-(r0.xyzx))+(r5.wwww)).xyz;
    // 204: mad r0.xyz, cb0[23].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 205: dp3 r5.w, r1.xyzx, r1.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 206: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 207: div r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)/(r5.wwww)).xyz;
    // 208: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 209: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 210: add r1.xyz, r1.xyzx, -r6.xyzx
    r1.xyz = ((r1.xyzx)+(-(r6.xyzx))).xyz;
    // 211: mul r6.xyz, cb0[16].xyzx, cb0[28].yyyy
    r6.xyz = ((source[16].xyzx)*(source[28].yyyy)).xyz;
    // 212: mul r6.xyz, r6.xyzx, cb0[29].wwww
    r6.xyz = ((r6.xyzx)*(source[29].wwww)).xyz;
    // 213: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 214: mad r10.xyz, r1.wwww, cb0[15].xyzx, -cb0[15].xyzx
    r10.xyz = ((r1.wwww)*(source[15].xyzx)+(-(source[15].xyzx))).xyz;
    // 215: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 216: mad r1.w, cb0[14].w, r1.w, l(1.000000)
    r1.w = ((source[14].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 217: mad r10.xyz, cb0[15].wwww, r10.xyzx, cb0[15].xyzx
    r10.xyz = ((source[15].wwww)*(r10.xyzx)+(source[15].xyzx)).xyz;
    // 218: mad r1.xyz, r1.xyzx, r6.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(r10.xyzx)).xyz;
    // 219: mad r1.xyz, r1.wwww, cb0[14].xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(source[14].xyzx)+(r1.xyzx)).xyz;
    // 220: mad r0.xyz, r0.xyzx, r12.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)+(r1.xyzx)).xyz;
    // 221: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 222: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 223: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 224: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 225: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 226: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 227: mul r1.xyz, r1.xxxx, cb0[17].xyzx
    r1.xyz = ((r1.xxxx)*(source[17].xyzx)).xyz;
    // 228: movc r1.xyz, r0.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 229: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 230: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 231: dp3 r0.w, r9.xyzx, r9.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 232: sqrt r1.x, r0.w
    r1.x = (sqrt(r0.wwww)).x;
    // 233: div r1.xyz, r9.xyzx, r1.xxxx
    r1.xyz = ((r9.xyzx)/(r1.xxxx)).xyz;
    // 234: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 235: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 236: mul r1.y, |r1.x|, |r1.x|
    r1.y = ((abs(r1.xxxx))*(abs(r1.xxxx))).y;
    // 237: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 238: mul r1.y, r1.y, |r1.x|
    r1.y = ((r1.yyyy)*(abs(r1.xxxx))).y;
    // 239: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 240: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 241: add r1.y, r1.x, l(-0.027778)
    r1.y = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 242: mad r1.x, r1.x, r1.y, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 243: div_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)/(r0.wwww))).w;
    // 244: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 245: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 246: mad r1.xyz, r0.wwww, r2.xyzx, -r13.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 247: mad r1.xyz, r3.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 248: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 249: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 250: mad r1.xyz, cb0[23].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[23].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 251: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 252: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 253: mad r1.xyz, cb0[23].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[23].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 254: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 255: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 256: mul r0.w, r0.w, cb0[27].z
    r0.w = ((r0.wwww)*(source[27].zzzz)).w;
    // 257: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 258: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 259: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 260: mul r1.w, cb0[4].z, l(1.500000)
    r1.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 261: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 262: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 263: add r1.w, -r4.w, cb0[4].x
    r1.w = ((-(r4.wwww))+(source[4].xxxx)).w;
    // 264: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 265: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 266: mul r4.x, r1.w, l(0.125000)
    r4.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 267: mul r2.y, cb0[4].y, cb0[18].y
    r2.y = ((source[4].yyyy)*(source[18].yyyy)).y;
    // 268: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 269: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 270: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 271: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 272: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 273: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 274: mul r0.w, r4.w, r2.w
    r0.w = ((r4.wwww)*(r2.wwww)).w;
    // 275: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 276: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 277: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 278: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 279: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 280: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 281: mad r2.xy, cb0[19].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[19].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 282: mul r0.w, cb0[19].y, cb0[27].z
    r0.w = ((source[19].yyyy)*(source[27].zzzz)).w;
    // 283: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 284: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 285: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 286: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 287: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 288: mul r1.w, cb0[19].x, l(0.001000)
    r1.w = ((source[19].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 289: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 290: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 291: dp2 r1.w, cb0[20].xyxx, r2.xyxx
    r1.w = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 292: dp2 r2.y, cb0[21].xyxx, r2.xyxx
    r2.y = (dot((source[21].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 293: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 294: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 295: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 296: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 297: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 298: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 299: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 300: mad r4.xyz, cb0[19].zzzz, r2.xyzx, -r1.xyzx
    r4.xyz = ((source[19].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 301: mul r2.xyz, r2.xyzx, cb0[19].zzzz
    r2.xyz = ((r2.xyzx)*(source[19].zzzz)).xyz;
    // 302: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 303: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 304: mad r1.xyz, r0.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 305: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 306: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 307: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 308: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 309: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 310: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 311: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 312: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 313: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 314: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 315: mul r3.yzw, r3.yyyy, cb0[31].xxyz
    r3.yzw = ((r3.yyyy)*(source[31].xxyz)).yzw;
    // 316: mad r3.xyz, r3.xxxx, cb0[30].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[30].xyzx)+(r3.yzwy)).xyz;
    // 317: mul r3.xyz, r3.xyzx, cb0[32].wwww
    r3.xyz = ((r3.xyzx)*(source[32].wwww)).xyz;
    // 318: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 319: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 320: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 321: mad o0.xyz, r1.xyzx, cb0[32].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[32].xyzx)+(r0.xyzx)).xyz;
    // 322: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 323: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 324: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 325: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 326: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 327: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 328: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 329: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 330: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 331: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 332: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 333: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 334: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 335: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 336: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 337: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 338: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 339: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 340: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 341: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 342: ret
    return output;
}

// source.character.monster-d621a47e69ad.v1 / source program 0d0f537eebee6d4a854349bb2fc36d79
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase24(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[19]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].y=(g_SourceCharacterTime.xxxx).x;
    source[24].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[25].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[25].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[25].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[25].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 4: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: mul r1.xyz, v7.yyyy, cb1[1].xywx
    r1.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 8: mad r1.xyz, cb1[0].xywx, v7.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v7.xxxx)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[2].xywx, v7.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v7.zzzz)+(r1.xyzx)).xyz;
    // 10: mad r1.xyz, cb1[3].xywx, v7.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v7.wwww)+(r1.xyzx)).xyz;
    // 11: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 14: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 15: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 16: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 17: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 18: max r0.x, r0.x, r1.x
    r0.x = (max(r0.xxxx,r1.xxxx)).x;
    // 19: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 20: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 21: rcp r1.x, |r0.x|
    r1.x = (1.0/(abs(r0.xxxx))).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 25: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r1.z, r1.z, cb0[21].x
    r1.z = ((r1.zzzz)*(source[21].xxxx)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 30: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 31: mul r1.z, r1.z, cb0[21].y
    r1.z = ((r1.zzzz)*(source[21].yyyy)).z;
    // 32: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 33: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 34: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 35: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 37: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 38: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 39: mul r3.xy, r1.xzxx, cb0[20].xxxx
    r3.xy = ((r1.xzxx)*(source[20].xxxx)).xy;
    // 40: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: add r1.xzw, -r3.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r3.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 45: mad r1.xzw, cb0[20].wwww, r1.xxzw, r3.xxyz
    r1.xzw = ((source[20].wwww)*(r1.xxzw)+(r3.xxyz)).xzw;
    // 46: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 47: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 48: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 49: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 50: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 51: mul r4.xyz, r2.wwww, v0.xyzx
    r4.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 52: dp3 r5.x, r4.xyzx, r1.xzwx
    r5.x = (dot((r4.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 53: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 54: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 55: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 56: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 57: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 58: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 59: dp3 r5.y, r7.xyzx, r1.xzwx
    r5.y = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 60: dp3 r5.z, r6.xyzx, r1.xzwx
    r5.z = (dot((r6.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 61: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 62: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 63: mul r8.xyz, r1.xxxx, v5.xyzx
    r8.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 64: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 65: dp3 r9.y, r7.xyzx, r8.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 66: dp3 r9.x, r4.xyzx, r8.xyzx
    r9.x = (dot((r4.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 67: dp3 r9.z, r6.xyzx, r8.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 68: dp3 r2.w, r5.xyzx, r9.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 69: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 70: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 71: mov r5.w, -r5.x
    r5.w = (-(r5.xxxx)).w;
    // 72: dp2 r2.w, r5.ywyy, r5.ywyy
    r2.w = (dot((r5.ywyy).xy,(r5.ywyy).xy).xxxx).w;
    // 73: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 74: div r5.xy, r5.ywyy, r2.wwww
    r5.xy = ((r5.ywyy)/(r2.wwww)).xy;
    // 75: mad r2.w, -r5.z, l(0.250000), l(0.250000)
    r2.w = ((-(r5.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 76: add r3.w, r5.z, l(1.000000)
    r3.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 78: mad r5.xy, r2.wwww, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 79: sample_l_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t4.xyzw, s4, r0.x
    r5.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r5.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 80: log r9.xyz, r5.xyzx
    r9.xyz = (log2(r5.xyzx)).xyz;
    // 81: rcp r0.x, cb0[21].z
    r0.x = (1.0/(source[21].zzzz)).x;
    // 82: mul r10.xyz, r9.xyzx, r0.xxxx
    r10.xyz = ((r9.xyzx)*(r0.xxxx)).xyz;
    // 83: mul r9.xyz, r9.xyzx, cb0[21].zzzz
    r9.xyz = ((r9.xyzx)*(source[21].zzzz)).xyz;
    // 84: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 85: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 86: mul r10.xyz, r0.xxxx, r10.xyzx
    r10.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 87: mad r9.xyz, r9.xyzx, cb0[21].zzzz, r10.xyzx
    r9.xyz = ((r9.xyzx)*(source[21].zzzz)+(r10.xyzx)).xyz;
    // 88: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 89: mul r5.xyz, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 90: add r0.x, cb0[21].z, l(1.000000)
    r0.x = ((source[21].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 92: dp3 r0.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r5.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r5.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 94: mad r5.xyz, r3.wwww, r5.xyzx, cb0[10].xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(source[10].xyzx)).xyz;
    // 95: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 96: mul r5.xyz, r5.xyzx, cb0[21].wwww
    r5.xyz = ((r5.xyzx)*(source[21].wwww)).xyz;
    // 97: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 98: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 99: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 100: dp3 r0.x, r3.xyzx, r8.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 101: mul_sat r2.w, r0.x, cb0[22].y
    r2.w = (saturate((r0.xxxx)*(source[22].yyyy))).w;
    // 102: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 104: mul_sat r3.w, r8.z, cb0[22].y
    r3.w = (saturate((r8.zzzz)*(source[22].yyyy))).w;
    // 105: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: add_sat r3.w, r3.w, -cb0[22].z
    r3.w = (saturate((r3.wwww)+(-(source[22].zzzz)))).w;
    // 107: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 108: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 109: mul r4.w, r4.w, cb0[22].w
    r4.w = ((r4.wwww)*(source[22].wwww)).w;
    // 110: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 111: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 112: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 113: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r9.xyz, -r2.xyzx, r3.wwww
    r9.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 115: mad r2.xyz, cb0[20].yyyy, r9.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 116: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 117: add r9.xyz, -r2.xyzx, r3.wwww
    r9.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 118: mad r2.xyz, cb0[20].zzzz, r9.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 119: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 120: add r9.xyz, -r2.xyzx, r3.wwww
    r9.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 121: mul r9.xyz, r9.xyzx, cb0[22].xxxx
    r9.xyz = ((r9.xyzx)*(source[22].xxxx)).xyz;
    // 122: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 123: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 124: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 125: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 126: mad r2.xyz, r3.wwww, r9.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 127: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 128: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 129: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 130: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 131: dp3 r3.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 133: mul r3.w, r3.w, cb0[23].x
    r3.w = ((r3.wwww)*(source[23].xxxx)).w;
    // 134: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 135: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 138: div r4.w, cb0[23].y, r4.w
    r4.w = ((source[23].yyyy)/(r4.wwww)).w;
    // 139: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 140: mul r9.xyz, r5.xyzx, r4.wwww
    r9.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 141: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 143: mad r0.yzw, cb0[20].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[20].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 144: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 146: mad r0.yzw, cb0[20].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[20].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 147: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 148: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 149: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 150: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 151: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 152: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r10.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r10.xywx))).xyz;
    // 153: mad r10.xyz, r10.zzzz, r11.xyzx, r10.xywx
    r10.xyz = ((r10.zzzz)*(r11.xyzx)+(r10.xywx)).xyz;
    // 154: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 156: mad r10.xyz, cb0[20].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 157: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 158: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 159: mad r10.xyz, cb0[20].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 160: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 162: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 163: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 164: mul r12.xyz, r0.yzwy, r10.xyzx
    r12.xyz = ((r0.yzwy)*(r10.xyzx)).xyz;
    // 165: mad r0.yzw, r10.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r10.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 166: mul r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)*(r12.xyzx)).xyz;
    // 167: mad r2.xyz, r2.xyzx, r9.xyzx, -r5.xyzx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 168: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mul r4.w, r4.w, cb0[23].z
    r4.w = ((r4.wwww)*(source[23].zzzz)).w;
    // 170: mad r2.xyz, r4.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r4.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 171: frc r4.w, cb0[3].x
    r4.w = (frac(source[3].xxxx)).w;
    // 172: add r5.x, -r4.w, l(1.000000)
    r5.x = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 173: mul r5.yzw, r2.xxyz, r5.xxxx
    r5.yzw = ((r2.xxyz)*(r5.xxxx)).yzw;
    // 174: dp3 r6.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: mad r9.xyz, -r5.xxxx, r2.xyzx, r6.wwww
    r9.xyz = ((-(r5.xxxx))*(r2.xyzx)+(r6.wwww)).xyz;
    // 176: mad r5.xyz, cb0[20].yyyy, r9.xyzx, r5.yzwy
    r5.xyz = ((source[20].yyyy)*(r9.xyzx)+(r5.yzwy)).xyz;
    // 177: dp3 r5.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r9.xyz, -r5.xyzx, r5.wwww
    r9.xyz = ((-(r5.xyzx))+(r5.wwww)).xyz;
    // 179: mad r5.xyz, cb0[20].zzzz, r9.xyzx, r5.xyzx
    r5.xyz = ((source[20].zzzz)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 180: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 181: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 182: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 183: dp3 r5.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 184: add r9.xyz, -r0.yzwy, r5.wwww
    r9.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 185: add r0.yzw, r0.yyzw, -r9.xxyz
    r0.yzw = ((r0.yyzw)+(-(r9.xxyz))).yzw;
    // 186: mul r9.xyz, cb0[14].xyzx, cb0[24].xxxx
    r9.xyz = ((source[14].xyzx)*(source[24].xxxx)).xyz;
    // 187: mul r9.xyz, r9.xyzx, cb0[25].wwww
    r9.xyz = ((r9.xyzx)*(source[25].wwww)).xyz;
    // 188: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 189: mad r10.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r10.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 190: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 191: mad r2.w, cb0[12].w, r2.w, l(1.000000)
    r2.w = ((source[12].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 192: mad r10.xyz, cb0[13].wwww, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((source[13].wwww)*(r10.xyzx)+(source[13].xyzx)).xyz;
    // 193: mad r0.yzw, r0.yyzw, r9.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r9.xxyz)+(r10.xxyz)).yzw;
    // 194: mad r0.yzw, r2.wwww, cb0[12].xxyz, r0.yyzw
    r0.yzw = ((r2.wwww)*(source[12].xxyz)+(r0.yyzw)).yzw;
    // 195: mad r0.yzw, r5.xxyz, r11.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 196: add r2.w, -|r8.z|, l(1.000000)
    r2.w = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 197: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 198: log r2.w, |r0.x|
    r2.w = (log2(abs(r0.xxxx))).w;
    // 199: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 200: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 201: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 202: mul r5.xyz, r2.wwww, cb0[15].xyzx
    r5.xyz = ((r2.wwww)*(source[15].xyzx)).xyz;
    // 203: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 204: add r0.xyz, r0.yzwy, r5.xyzx
    r0.xyz = ((r0.yzwy)+(r5.xyzx)).xyz;
    // 205: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 206: dp3 r0.w, r1.xzwx, r1.xzwx
    r0.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 207: sqrt r2.w, r0.w
    r2.w = (sqrt(r0.wwww)).w;
    // 208: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 209: dp3 r1.x, r1.xzwx, r8.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 210: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 211: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 212: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 213: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 214: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 215: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 216: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 217: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 218: div_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)/(r0.wwww))).w;
    // 219: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 221: mad r1.xyz, r0.wwww, r2.xyzx, -r12.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(-(r12.xyzx))).xyz;
    // 222: mad r1.xyz, r3.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 223: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 224: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 225: mad r1.xyz, cb0[20].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 226: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 227: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 228: mad r1.xyz, cb0[20].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 229: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 230: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 231: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 232: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 233: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 234: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 235: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 236: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 237: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 238: add r1.w, -r4.w, cb0[3].x
    r1.w = ((-(r4.wwww))+(source[3].xxxx)).w;
    // 239: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 240: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 241: mul r5.x, r1.w, l(0.125000)
    r5.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 242: mul r2.y, cb0[3].y, cb0[16].y
    r2.y = ((source[3].yyyy)*(source[16].yyyy)).y;
    // 243: mov r5.y, v4.y
    r5.y = (v4.yyyy).y;
    // 244: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 245: add r2.xy, r2.xyxx, r5.xyxx
    r2.xy = ((r2.xyxx)+(r5.xyxx)).xy;
    // 246: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 247: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 248: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 249: mul r0.w, r4.w, r2.w
    r0.w = ((r4.wwww)*(r2.wwww)).w;
    // 250: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 251: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 252: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 253: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 254: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 255: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 256: mad r2.xy, cb0[17].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 257: mul r0.w, cb0[17].y, cb0[24].y
    r0.w = ((source[17].yyyy)*(source[24].yyyy)).w;
    // 258: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 259: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 260: mul r5.y, r0.w, l(0.020000)
    r5.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 261: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 262: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 263: mul r1.w, cb0[17].x, l(0.001000)
    r1.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 264: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 265: mad r2.xy, r1.wwww, r2.xyxx, r5.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r5.xyxx)).xy;
    // 266: dp2 r1.w, cb0[18].xyxx, r2.xyxx
    r1.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 267: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 268: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 269: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 270: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 271: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 272: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 273: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 274: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 275: mad r5.xyz, cb0[17].zzzz, r2.xyzx, -r1.xyzx
    r5.xyz = ((source[17].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 276: mul r2.xyz, r2.xyzx, cb0[17].zzzz
    r2.xyz = ((r2.xyzx)*(source[17].zzzz)).xyz;
    // 277: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 278: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 279: mad r1.xyz, r0.wwww, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 280: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 281: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 282: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 283: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 284: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 285: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 286: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 287: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 288: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 289: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 290: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 291: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 292: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 293: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 294: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 295: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 296: mad o0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 297: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 298: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 299: dp3 r0.x, r4.xyzx, r2.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 300: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 301: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 302: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 303: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 304: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 305: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 306: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 307: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 308: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 309: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 310: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 311: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 312: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 313: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 314: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 315: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 316: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 317: ret
    return output;
}

// source.character.monster-be5bc0ded311.v1 / source program ab0d30ff2ccc914b84c29564ae10ccd8
