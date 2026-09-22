SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase176(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[21]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[30].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s7, l(0.000000)
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
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 15: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 16: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 17: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 18: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 19: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 20: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 21: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 22: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 23: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 24: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 25: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 26: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 27: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 28: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 29: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 30: add r0.z, -cb0[24].y, cb0[24].x
    r0.z = ((-(source[24].yyyy))+(source[24].xxxx)).z;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mad r0.z, r2.x, r0.z, cb0[24].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[24].yyyy)).z;
    // 33: add r0.w, -r0.z, cb0[24].z
    r0.w = ((-(r0.zzzz))+(source[24].zzzz)).w;
    // 34: mad r0.z, r2.y, r0.w, r0.z
    r0.z = ((r2.yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 35: add r0.w, -r0.z, cb0[24].w
    r0.w = ((-(r0.zzzz))+(source[24].wwww)).w;
    // 36: mad r0.z, r2.z, r0.w, r0.z
    r0.z = ((r2.zzzz)*(r0.wwww)+(r0.zzzz)).z;
    // 37: add r0.w, -r0.z, cb0[25].x
    r0.w = ((-(r0.zzzz))+(source[25].xxxx)).w;
    // 38: mad r0.z, r2.w, r0.w, r0.z
    r0.z = ((r2.wwww)*(r0.wwww)+(r0.zzzz)).z;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: add r0.w, -r3.w, l(1.000000)
    r0.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 42: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 43: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 44: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 45: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 47: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 48: add r1.w, -r0.w, cb0[27].x
    r1.w = ((-(r0.wwww))+(source[27].xxxx)).w;
    // 49: mad r0.w, r2.w, r1.w, r0.w
    r0.w = ((r2.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 50: add r1.w, -cb0[26].y, cb0[26].x
    r1.w = ((-(source[26].yyyy))+(source[26].xxxx)).w;
    // 51: mad r1.w, r2.x, r1.w, cb0[26].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[26].yyyy)).w;
    // 52: add r3.w, -r1.w, cb0[26].z
    r3.w = ((-(r1.wwww))+(source[26].zzzz)).w;
    // 53: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 54: add r3.w, -r1.w, cb0[26].w
    r3.w = ((-(r1.wwww))+(source[26].wwww)).w;
    // 55: mad r1.w, r2.z, r3.w, r1.w
    r1.w = ((r2.zzzz)*(r3.wwww)+(r1.wwww)).w;
    // 56: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 57: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 58: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 59: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 60: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 62: mad r4.xyzw, r0.ywyw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r0.ywyw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 63: dp2 r0.y, r4.zwzz, r4.zwzz
    r0.y = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).y;
    // 64: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 66: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 67: add r5.z, r0.y, l(0.000010)
    r5.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 68: mul r5.xy, r4.xyxx, cb0[23].xxxx
    r5.xy = ((r4.xyxx)*(source[23].xxxx)).xy;
    // 69: mad r4.xy, cb0[23].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[23].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 70: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 71: mad r4.xyz, r2.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 72: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: mad r5.xyz, cb0[25].wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((source[25].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 74: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 75: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 76: div r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)/(r0.yyyy)).xyz;
    // 77: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 78: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 79: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 80: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 81: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 82: mul r7.xyz, r0.yyyy, v0.xyzx
    r7.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 83: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 84: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 85: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 86: dp3 r9.y, r8.xyzx, r5.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 87: dp3 r9.x, r7.xyzx, r5.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 88: dp3 r9.z, r6.xyzx, r5.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 89: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 90: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 91: mul r5.xyz, r0.yyyy, v5.xyzx
    r5.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 92: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 93: dp3 r11.y, r8.xyzx, r5.xyzx
    r11.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 94: dp3 r11.x, r7.xyzx, r5.xyzx
    r11.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 95: dp3 r11.z, r6.xyzx, r5.xyzx
    r11.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 96: dp3 r0.y, r9.xyzx, r11.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 97: mul r9.xyz, r9.xyzx, r0.yyyy
    r9.xyz = ((r9.xyzx)*(r0.yyyy)).xyz;
    // 98: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 99: mov r9.w, -r9.x
    r9.w = (-(r9.xxxx)).w;
    // 100: dp2 r0.y, r9.ywyy, r9.ywyy
    r0.y = (dot((r9.ywyy).xy,(r9.ywyy).xy).xxxx).y;
    // 101: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 102: div r0.yw, r9.yyyw, r0.yyyy
    r0.yw = ((r9.yyyw)/(r0.yyyy)).yw;
    // 103: mad r1.w, -r9.z, l(0.250000), l(0.250000)
    r1.w = ((-(r9.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 104: add r3.w, r9.z, l(1.000000)
    r3.w = ((r9.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 106: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 107: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 108: log r9.xyz, r0.xywx
    r9.xyz = (log2(r0.xywx)).xyz;
    // 109: rcp r1.w, cb0[27].y
    r1.w = (1.0/(source[27].yyyy)).w;
    // 110: mul r11.xyz, r9.xyzx, r1.wwww
    r11.xyz = ((r9.xyzx)*(r1.wwww)).xyz;
    // 111: mul r9.xyz, r9.xyzx, cb0[27].yyyy
    r9.xyz = ((r9.xyzx)*(source[27].yyyy)).xyz;
    // 112: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 113: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 114: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 115: mad r9.xyz, r9.xyzx, cb0[27].yyyy, r11.xyzx
    r9.xyz = ((r9.xyzx)*(source[27].yyyy)+(r11.xyzx)).xyz;
    // 116: add r0.xyw, r0.xyxw, r9.xyxz
    r0.xyw = ((r0.xyxw)+(r9.xyxz)).xyw;
    // 117: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 118: add r1.w, cb0[27].y, l(1.000000)
    r1.w = ((source[27].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 120: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r9.xyz, -cb0[12].xyzx, cb0[13].xyzx
    r9.xyz = ((-(source[12].xyzx))+(source[13].xyzx)).xyz;
    // 122: mad r9.xyz, r3.wwww, r9.xyzx, cb0[12].xyzx
    r9.xyz = ((r3.wwww)*(r9.xyzx)+(source[12].xyzx)).xyz;
    // 123: mul r0.xyw, r0.xxxx, r9.xyxz
    r0.xyw = ((r0.xxxx)*(r9.xyxz)).xyw;
    // 124: mul r0.xyw, r0.xyxw, cb0[27].zzzz
    r0.xyw = ((r0.xyxw)*(source[27].zzzz)).xyw;
    // 125: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r9.xyz, -r3.xyzx, r1.wwww
    r9.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 127: mad r3.yzw, cb0[25].yyyy, r9.xxyz, r3.xxyz
    r3.yzw = ((source[25].yyyy)*(r9.xxyz)+(r3.xxyz)).yzw;
    // 128: dp3 r1.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: add r9.xyz, -r3.yzwy, r1.wwww
    r9.xyz = ((-(r3.yzwy))+(r1.wwww)).xyz;
    // 130: mad r3.yzw, cb0[25].zzzz, r9.xxyz, r3.yyzw
    r3.yzw = ((source[25].zzzz)*(r9.xxyz)+(r3.yyzw)).yzw;
    // 131: dp3 r1.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: add r9.xyz, -r3.yzwy, r1.wwww
    r9.xyz = ((-(r3.yzwy))+(r1.wwww)).xyz;
    // 133: mul r9.xyz, r9.xyzx, cb0[27].wwww
    r9.xyz = ((r9.xyzx)*(source[27].wwww)).xyz;
    // 134: add r1.w, r2.y, r2.x
    r1.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 135: add r1.w, r2.z, r1.w
    r1.w = ((r2.zzzz)+(r1.wwww)).w;
    // 136: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 137: mad r3.yzw, r1.wwww, r9.xxyz, r3.yyzw
    r3.yzw = ((r1.wwww)*(r9.xxyz)+(r3.yyzw)).yzw;
    // 138: add r9.xyz, -r3.yzwy, r3.xxxx
    r9.xyz = ((-(r3.yzwy))+(r3.xxxx)).xyz;
    // 139: mad r3.xyz, r2.wwww, r9.xyzx, r3.yzwy
    r3.xyz = ((r2.wwww)*(r9.xyzx)+(r3.yzwy)).xyz;
    // 140: max r9.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 141: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 142: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 143: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 144: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 146: add r3.w, cb0[28].w, -cb0[29].x
    r3.w = ((source[28].wwww)+(-(source[29].xxxx))).w;
    // 147: mad r3.w, r2.w, r3.w, cb0[29].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[29].xxxx)).w;
    // 148: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 149: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 150: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mad r3.w, -r1.w, r1.w, l(1.000000)
    r3.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: max r3.w, r3.w, l(0.001000)
    r3.w = (max(r3.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 153: div r3.w, cb0[29].y, r3.w
    r3.w = ((source[29].yyyy)/(r3.wwww)).w;
    // 154: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 155: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 156: div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // 157: dp3 r4.w, r4.xyzx, r5.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 158: mul_sat r5.w, r4.w, cb0[28].x
    r5.w = (saturate((r4.wwww)*(source[28].xxxx))).w;
    // 159: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul_sat r6.w, r5.z, cb0[28].x
    r6.w = (saturate((r5.zzzz)*(source[28].xxxx))).w;
    // 162: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: add_sat r6.w, r6.w, -cb0[28].y
    r6.w = (saturate((r6.wwww)+(-(source[28].yyyy)))).w;
    // 164: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 165: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 166: mul r7.w, r7.w, cb0[28].z
    r7.w = ((r7.wwww)*(source[28].zzzz)).w;
    // 167: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 168: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 169: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 170: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 171: mul r9.xyz, r0.xywx, r3.wwww
    r9.xyz = ((r0.xywx)*(r3.wwww)).xyz;
    // 172: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 173: add r11.xyz, -r1.xyzx, r3.wwww
    r11.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 174: mad r1.xyz, cb0[25].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[25].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 175: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: add r11.xyz, -r1.xyzx, r3.wwww
    r11.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 177: mad r1.xyz, cb0[25].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[25].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 178: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 179: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 180: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 181: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 182: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 183: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, -r11.xyzx
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r11.xyzx))).xyz;
    // 184: mad r2.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r2.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 185: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 187: mad r2.xyz, cb0[25].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[25].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 188: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 190: mad r2.xyz, cb0[25].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[25].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 191: mad r11.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mad r12.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 194: mul r12.xyz, r2.xyzx, r11.xyzx
    r12.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 195: mad r2.xyz, -r2.xyzx, r11.xyzx, cb0[11].xyzx
    r2.xyz = ((-(r2.xyzx))*(r11.xyzx)+(source[11].xyzx)).xyz;
    // 196: mad r2.xyz, r2.wwww, r2.xyzx, r12.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 197: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 198: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 199: mad r2.xyz, r3.xyzx, r9.xyzx, -r0.xywx
    r2.xyz = ((r3.xyzx)*(r9.xyzx)+(-(r0.xywx))).xyz;
    // 200: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 201: mul r2.w, r2.w, cb0[29].z
    r2.w = ((r2.wwww)*(source[29].zzzz)).w;
    // 202: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 203: dp3 r2.x, r10.xyzx, r10.xyzx
    r2.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 204: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 205: div r2.yzw, r10.xxyz, r2.yyyy
    r2.yzw = ((r10.xxyz)/(r2.yyyy)).yzw;
    // 206: dp3 r2.y, r2.yzwy, r5.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // 207: add r2.z, -|r5.z|, l(1.000000)
    r2.z = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 208: mul r2.z, r4.w, r2.z
    r2.z = ((r4.wwww)*(r2.zzzz)).z;
    // 209: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 210: mul r2.w, |r2.y|, |r2.y|
    r2.w = ((abs(r2.yyyy))*(abs(r2.yyyy))).w;
    // 211: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 212: mul r2.w, r2.w, |r2.y|
    r2.w = ((r2.wwww)*(abs(r2.yyyy))).w;
    // 213: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 214: movc r2.y, r2.y, l(0), r2.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 215: add r2.w, r2.y, l(-0.027778)
    r2.w = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 216: mad r2.y, r2.y, r2.w, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 217: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 218: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 219: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 220: mad r2.xyw, r0.zzzz, r0.xyxw, -r1.xyxz
    r2.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r1.xyxz))).xyw;
    // 221: mad r1.xyz, r1.wwww, r2.xywx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 222: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 223: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 224: mad r1.xyz, cb0[25].yyyy, r2.xywx, r1.xyzx
    r1.xyz = ((source[25].yyyy)*(r2.xywx)+(r1.xyzx)).xyz;
    // 225: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 226: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 227: mad r1.xyz, cb0[25].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[25].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 228: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 229: add r0.z, -cb0[4].w, l(1.000000)
    r0.z = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 230: mul r0.z, r0.z, cb0[30].z
    r0.z = ((r0.zzzz)*(source[30].zzzz)).z;
    // 231: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 232: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 233: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 234: mul r1.w, cb0[4].z, l(1.500000)
    r1.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 235: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 236: mad r0.z, r0.z, l(0.500000), cb0[4].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).z;
    // 237: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 238: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 239: mul r3.y, cb0[4].y, cb0[18].y
    r3.y = ((source[4].yyyy)*(source[18].yyyy)).y;
    // 240: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 241: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 242: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 243: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 244: add r2.w, -r1.w, cb0[4].x
    r2.w = ((-(r1.wwww))+(source[4].xxxx)).w;
    // 245: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 246: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 247: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 248: mul r2.xyw, r0.zzzz, r3.xyxz
    r2.xyw = ((r0.zzzz)*(r3.xyxz)).xyw;
    // 249: mul r0.z, r1.w, r3.w
    r0.z = ((r1.wwww)*(r3.wwww)).z;
    // 250: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mad r2.xyw, r2.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxz
    r2.xyw = ((r2.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxz))).xyw;
    // 252: mad r1.xyz, r0.zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 253: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 254: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 255: add r2.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 256: add r2.xy, -r3.zwzz, r2.xyxx
    r2.xy = ((-(r3.zwzz))+(r2.xyxx)).xy;
    // 257: mad r2.xy, cb0[19].wwww, r2.xyxx, r3.zwzz
    r2.xy = ((source[19].wwww)*(r2.xyxx)+(r3.zwzz)).xy;
    // 258: mul r0.z, cb0[19].y, cb0[30].z
    r0.z = ((source[19].yyyy)*(source[30].zzzz)).z;
    // 259: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 260: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 261: mul r3.y, r0.z, l(0.020000)
    r3.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 262: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 263: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 264: mul r2.w, cb0[19].x, l(0.001000)
    r2.w = ((source[19].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 265: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 266: mad r2.xy, r2.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 267: dp2 r2.w, cb0[20].xyxx, r2.xyxx
    r2.w = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 268: dp2 r2.y, cb0[21].xyxx, r2.xyxx
    r2.y = (dot((source[21].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 269: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 270: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 271: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 272: mad r2.xyw, r3.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r3.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 273: mul r3.x, r3.w, l(0.900000)
    r3.x = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 274: mad r2.xyw, r3.xxxx, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.xxxx)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 275: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 276: mad r3.xyz, cb0[19].zzzz, r2.xywx, -r1.xyzx
    r3.xyz = ((source[19].zzzz)*(r2.xywx)+(-(r1.xyzx))).xyz;
    // 277: mul r2.xyw, r2.xyxw, cb0[19].zzzz
    r2.xyw = ((r2.xyxw)*(source[19].zzzz)).xyw;
    // 278: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 279: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 280: mad r1.xyz, r0.zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 281: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 282: mad r2.xyw, r5.wwww, cb0[16].xyxz, -cb0[16].xyxz
    r2.xyw = ((r5.wwww)*(source[16].xyxz)+(-(source[16].xyxz))).xyw;
    // 283: mul r0.z, r5.w, cb0[15].w
    r0.z = ((r5.wwww)*(source[15].wwww)).z;
    // 284: mad r2.xyw, cb0[16].wwww, r2.xyxw, cb0[16].xyxz
    r2.xyw = ((source[16].wwww)*(r2.xyxw)+(source[16].xyxz)).xyw;
    // 285: mad r2.xyw, r0.zzzz, cb0[15].xyxz, r2.xyxw
    r2.xyw = ((r0.zzzz)*(source[15].xyxz)+(r2.xyxw)).xyw;
    // 286: add r0.z, cb0[2].y, cb0[2].x
    r0.z = ((source[2].yyyy)+(source[2].xxxx)).z;
    // 287: add r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)+(source[2].zzzz)).z;
    // 288: add r3.x, -r0.z, l(1000.000000)
    r3.x = ((-(r0.zzzz))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 289: mad r0.z, cb0[30].w, r3.x, r0.z
    r0.z = ((source[30].wwww)*(r3.xxxx)+(r0.zzzz)).z;
    // 290: mul r0.z, r0.z, l(0.010000)
    r0.z = ((r0.zzzz)*(float4(0.010000,0.010000,0.010000,0.010000))).z;
    // 291: mad r0.z, cb0[30].y, cb0[30].z, r0.z
    r0.z = ((source[30].yyyy)*(source[30].zzzz)+(r0.zzzz)).z;
    // 292: mul r3.x, r0.z, l(3.524534)
    r3.x = ((r0.zzzz)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 293: sincos null, r3.x, r3.x
    r3.x = (cos(r3.xxxx)).x;
    // 294: add r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)+(r3.xxxx)).z;
    // 295: mul r0.z, r0.z, l(1.328987)
    r0.z = ((r0.zzzz)*(float4(1.328987,1.328987,1.328987,1.328987))).z;
    // 296: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 297: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 298: mad r0.z, r0.z, l(0.500000), cb0[30].x
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[30].xxxx)).z;
    // 299: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t7.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 300: mul r5.xyz, cb0[14].xyzx, cb0[29].wwww
    r5.xyz = ((source[14].xyzx)*(source[29].wwww)).xyz;
    // 301: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 302: mul r3.xyz, r0.zzzz, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)).xyz;
    // 303: mad r0.xyz, r1.wwww, r0.xywx, r3.xyzx
    r0.xyz = ((r1.wwww)*(r0.xywx)+(r3.xyzx)).xyz;
    // 304: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 305: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 306: mad r0.xyz, cb0[25].yyyy, r3.xyzx, r0.xyzx
    r0.xyz = ((source[25].yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 307: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 308: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 309: mad r0.xyz, cb0[25].zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((source[25].zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 310: mad r0.xyz, r0.xyzx, r11.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.xywx)).xyz;
    // 311: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 312: lt r1.w, |r2.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 313: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 314: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 315: mul r2.xyz, r0.wwww, cb0[17].xyzx
    r2.xyz = ((r0.wwww)*(source[17].xyzx)).xyz;
    // 316: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 317: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 318: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 319: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 320: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 321: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 322: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 323: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 324: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 325: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 326: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 327: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 328: mul r3.yzw, r3.yyyy, cb0[32].xxyz
    r3.yzw = ((r3.yyyy)*(source[32].xxyz)).yzw;
    // 329: mad r3.xyz, r3.xxxx, cb0[31].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[31].xyzx)+(r3.yzwy)).xyz;
    // 330: mul r3.xyz, r3.xyzx, cb0[33].wwww
    r3.xyz = ((r3.xyzx)*(source[33].wwww)).xyz;
    // 331: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 332: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 333: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 334: mad o0.xyz, r1.xyzx, cb0[33].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[33].xyzx)+(r0.xyzx)).xyz;
    // 335: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 336: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 337: dp3 r0.x, r7.xyzx, r2.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 338: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 339: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 340: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 341: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 342: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 343: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 344: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 345: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 346: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 347: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 348: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 349: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 350: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 351: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 352: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 353: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 354: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 355: ret
    return output;
}

// source.character.equipment-native-177.v1 / source program 403c8811cabeb74d97b34548290a6553
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase177(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[25]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[27]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[28]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[36].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.wxyz, s4, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 22: add r1.y, -cb0[30].y, cb0[30].x
    r1.y = ((-(source[30].yyyy))+(source[30].xxxx)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: mad r1.y, r2.x, r1.y, cb0[30].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[30].yyyy)).y;
    // 25: add r1.z, -r1.y, cb0[30].z
    r1.z = ((-(r1.yyyy))+(source[30].zzzz)).z;
    // 26: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 27: add r1.z, -r1.y, cb0[30].w
    r1.z = ((-(r1.yyyy))+(source[30].wwww)).z;
    // 28: mad r1.y, r2.z, r1.z, r1.y
    r1.y = ((r2.zzzz)*(r1.zzzz)+(r1.yyyy)).y;
    // 29: add r1.z, -r1.y, cb0[31].x
    r1.z = ((-(r1.yyyy))+(source[31].xxxx)).z;
    // 30: mad r1.y, r2.w, r1.z, r1.y
    r1.y = ((r2.wwww)*(r1.zzzz)+(r1.yyyy)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: add r1.z, -r3.w, l(1.000000)
    r1.z = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 34: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 35: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 36: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 37: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 39: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 40: add r1.w, -r1.z, cb0[33].x
    r1.w = ((-(r1.zzzz))+(source[33].xxxx)).w;
    // 41: mad r1.z, r2.w, r1.w, r1.z
    r1.z = ((r2.wwww)*(r1.wwww)+(r1.zzzz)).z;
    // 42: add r1.w, -cb0[32].y, cb0[32].x
    r1.w = ((-(source[32].yyyy))+(source[32].xxxx)).w;
    // 43: mad r1.w, r2.x, r1.w, cb0[32].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[32].yyyy)).w;
    // 44: add r3.w, -r1.w, cb0[32].z
    r3.w = ((-(r1.wwww))+(source[32].zzzz)).w;
    // 45: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 46: add r3.w, -r1.w, cb0[32].w
    r3.w = ((-(r1.wwww))+(source[32].wwww)).w;
    // 47: mad r1.w, r2.z, r3.w, r1.w
    r1.w = ((r2.zzzz)*(r3.wwww)+(r1.wwww)).w;
    // 48: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 49: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 50: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 51: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 52: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 54: mad r4.xyzw, r1.xzxz, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r1.xzxz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 55: dp2 r1.x, r4.zwzz, r4.zwzz
    r1.x = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).x;
    // 56: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 58: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 59: add r5.z, r1.x, l(0.000010)
    r5.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 60: mul r5.xy, r4.xyxx, cb0[29].xxxx
    r5.xy = ((r4.xyxx)*(source[29].xxxx)).xy;
    // 61: mad r4.xy, cb0[29].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[29].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 62: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 63: mad r1.xzw, r2.wwww, r4.xxyz, r5.xxyz
    r1.xzw = ((r2.wwww)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 64: add r4.xyz, -r1.xzwx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xzwx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 65: mad r4.xyz, cb0[31].wwww, r4.xyzx, r1.xzwx
    r4.xyz = ((source[31].wwww)*(r4.xyzx)+(r1.xzwx)).xyz;
    // 66: dp3 r3.w, r4.xyzx, r4.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 67: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 68: div r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)/(r3.wwww)).xyz;
    // 69: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 70: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 71: mul r5.xyz, r3.wwww, v1.xyzx
    r5.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 72: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 73: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 74: mul r6.xyz, r3.wwww, v0.xyzx
    r6.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 75: mul r7.xyz, r5.zxyz, r6.yzxy
    r7.xyz = ((r5.zxyz)*(r6.yzxy)).xyz;
    // 76: mad r7.xyz, r5.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r5.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 77: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 78: dp3 r8.y, r7.xyzx, r4.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 79: dp3 r8.x, r6.xyzx, r4.xyzx
    r8.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 80: dp3 r8.z, r5.xyzx, r4.xyzx
    r8.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 81: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 82: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 83: mul r4.xyz, r3.wwww, v5.xyzx
    r4.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 84: mad r9.xyz, v5.xyzx, r3.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r3.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 85: dp3 r10.y, r7.xyzx, r4.xyzx
    r10.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 86: dp3 r10.x, r6.xyzx, r4.xyzx
    r10.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 87: dp3 r10.z, r5.xyzx, r4.xyzx
    r10.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 88: dp3 r3.w, r8.xyzx, r10.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 89: mul r8.xyz, r8.xyzx, r3.wwww
    r8.xyz = ((r8.xyzx)*(r3.wwww)).xyz;
    // 90: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 91: mov r8.w, -r8.x
    r8.w = (-(r8.xxxx)).w;
    // 92: dp2 r3.w, r8.ywyy, r8.ywyy
    r3.w = (dot((r8.ywyy).xy,(r8.ywyy).xy).xxxx).w;
    // 93: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 94: div r8.xy, r8.ywyy, r3.wwww
    r8.xy = ((r8.ywyy)/(r3.wwww)).xy;
    // 95: mad r3.w, -r8.z, l(0.250000), l(0.250000)
    r3.w = ((-(r8.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 96: add r4.w, r8.z, l(1.000000)
    r4.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 98: mad r8.xy, r3.wwww, r8.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r3.wwww)*(r8.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 99: sample_l_indexable(texture2d)(float,float,float,float) r8.xyz, r8.xyxx, t5.xyzw, s5, r0.x
    r8.xyz = ((g_SourceCharacterTexture5.SampleLevel(SourceCharacterLookupSampler, (r8.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 100: log r10.xyz, r8.xyzx
    r10.xyz = (log2(r8.xyzx)).xyz;
    // 101: rcp r0.x, cb0[33].y
    r0.x = (1.0/(source[33].yyyy)).x;
    // 102: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 103: mul r10.xyz, r10.xyzx, cb0[33].yyyy
    r10.xyz = ((r10.xyzx)*(source[33].yyyy)).xyz;
    // 104: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 105: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 106: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 107: mad r10.xyz, r10.xyzx, cb0[33].yyyy, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[33].yyyy)+(r11.xyzx)).xyz;
    // 108: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 109: mul r8.xyz, r8.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 110: add r0.x, cb0[33].y, l(1.000000)
    r0.x = ((source[33].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 112: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: add r8.xyz, -cb0[19].xyzx, cb0[20].xyzx
    r8.xyz = ((-(source[19].xyzx))+(source[20].xyzx)).xyz;
    // 114: mad r8.xyz, r4.wwww, r8.xyzx, cb0[19].xyzx
    r8.xyz = ((r4.wwww)*(r8.xyzx)+(source[19].xyzx)).xyz;
    // 115: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 116: mul r8.xyz, r8.xyzx, cb0[33].zzzz
    r8.xyz = ((r8.xyzx)*(source[33].zzzz)).xyz;
    // 117: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 118: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 119: mad r3.yzw, cb0[31].yyyy, r10.xxyz, r3.xxyz
    r3.yzw = ((source[31].yyyy)*(r10.xxyz)+(r3.xxyz)).yzw;
    // 120: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r10.xyz, -r3.yzwy, r0.xxxx
    r10.xyz = ((-(r3.yzwy))+(r0.xxxx)).xyz;
    // 122: mad r3.yzw, cb0[31].zzzz, r10.xxyz, r3.yyzw
    r3.yzw = ((source[31].zzzz)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 123: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 124: add r10.xyz, -r3.yzwy, r0.xxxx
    r10.xyz = ((-(r3.yzwy))+(r0.xxxx)).xyz;
    // 125: mul r10.xyz, r10.xyzx, cb0[33].wwww
    r10.xyz = ((r10.xyzx)*(source[33].wwww)).xyz;
    // 126: add r0.x, r2.y, r2.x
    r0.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 127: add r0.x, r2.z, r0.x
    r0.x = ((r2.zzzz)+(r0.xxxx)).x;
    // 128: add_sat r0.x, r2.w, r0.x
    r0.x = (saturate((r2.wwww)+(r0.xxxx))).x;
    // 129: mad r3.yzw, r0.xxxx, r10.xxyz, r3.yyzw
    r3.yzw = ((r0.xxxx)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 130: add r10.xyz, -r3.yzwy, r3.xxxx
    r10.xyz = ((-(r3.yzwy))+(r3.xxxx)).xyz;
    // 131: mad r3.xyz, r2.wwww, r10.xyzx, r3.yzwy
    r3.xyz = ((r2.wwww)*(r10.xyzx)+(r3.yzwy)).xyz;
    // 132: max r10.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 133: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 134: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 135: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 136: dp3 r0.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 137: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 138: add r3.w, cb0[34].w, -cb0[35].x
    r3.w = ((source[34].wwww)+(-(source[35].xxxx))).w;
    // 139: mad r3.w, r2.w, r3.w, cb0[35].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[35].xxxx)).w;
    // 140: mul r0.x, r0.x, r3.w
    r0.x = ((r0.xxxx)*(r3.wwww)).x;
    // 141: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 142: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: mad r3.w, -r0.x, r0.x, l(1.000000)
    r3.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: max r3.w, r3.w, l(0.001000)
    r3.w = (max(r3.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 145: div r3.w, cb0[35].y, r3.w
    r3.w = ((source[35].yyyy)/(r3.wwww)).w;
    // 146: dp3 r4.w, r1.xzwx, r1.xzwx
    r4.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 147: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 148: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 149: dp3 r4.w, r1.xzwx, r4.xyzx
    r4.w = (dot((r1.xzwx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 150: mul_sat r5.w, r4.w, cb0[34].x
    r5.w = (saturate((r4.wwww)*(source[34].xxxx))).w;
    // 151: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mul_sat r6.w, r4.z, cb0[34].x
    r6.w = (saturate((r4.zzzz)*(source[34].xxxx))).w;
    // 154: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: add_sat r6.w, r6.w, -cb0[34].y
    r6.w = (saturate((r6.wwww)+(-(source[34].yyyy)))).w;
    // 156: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 157: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 158: mul r7.w, r7.w, cb0[34].z
    r7.w = ((r7.wwww)*(source[34].zzzz)).w;
    // 159: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 160: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 161: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 162: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 163: mul r10.xyz, r8.xyzx, r3.wwww
    r10.xyz = ((r8.xyzx)*(r3.wwww)).xyz;
    // 164: dp3 r3.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 165: add r11.xyz, -r0.yzwy, r3.wwww
    r11.xyz = ((-(r0.yzwy))+(r3.wwww)).xyz;
    // 166: mad r0.yzw, cb0[31].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[31].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 167: dp3 r3.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 168: add r11.xyz, -r0.yzwy, r3.wwww
    r11.xyz = ((-(r0.yzwy))+(r3.wwww)).xyz;
    // 169: mad r0.yzw, cb0[31].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[31].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 170: add r11.xy, v4.zwzz, l(-0.050000, -0.050000, 0.000000, 0.000000)
    r11.xy = ((v4.zwzz)+(float4(-0.050000,-0.050000,0.000000,0.000000))).xy;
    // 171: mul_sat r11.xy, r11.xyxx, l(256.000000, 256.000000, 0.000000, 0.000000)
    r11.xy = (saturate((r11.xyxx)*(float4(256.000000,256.000000,0.000000,0.000000)))).xy;
    // 172: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 173: mad r3.w, -r11.x, r11.y, l(1.000000)
    r3.w = ((-(r11.xxxx))*(r11.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: mul r11.xy, v4.wzww, l(4.000000, 4.000000, 0.000000, 0.000000)
    r11.xy = ((v4.wzww)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 175: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r11.xyxx, t4.xyzw, s3, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r11.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 176: dp4 r6.w, r11.xyzw, cb0[12].xyzw
    r6.w = (dot((r11.xyzw).xyzw,(source[12].xyzw).xyzw).xxxx).w;
    // 177: mul r6.w, r3.w, r6.w
    r6.w = ((r3.wwww)*(r6.wwww)).w;
    // 178: mul r6.w, r6.w, cb0[8].y
    r6.w = ((r6.wwww)*(source[8].yyyy)).w;
    // 179: mul r12.xyz, cb0[10].xyzx, cb0[10].wwww
    r12.xyz = ((source[10].xyzx)*(source[10].wwww)).xyz;
    // 180: mad r13.xyz, cb0[11].wwww, cb0[11].xyzx, -r12.xyzx
    r13.xyz = ((source[11].wwww)*(source[11].xyzx)+(-(r12.xyzx))).xyz;
    // 181: mad r12.xyz, r6.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r6.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 182: dp4 r6.w, r11.xyzw, cb0[9].xyzw
    r6.w = (dot((r11.xyzw).xyzw,(source[9].xyzw).xyzw).xxxx).w;
    // 183: dp4 r7.w, r11.xyzw, cb0[15].xyzw
    r7.w = (dot((r11.xyzw).xyzw,(source[15].xyzw).xyzw).xxxx).w;
    // 184: mul r7.w, r3.w, r7.w
    r7.w = ((r3.wwww)*(r7.wwww)).w;
    // 185: mul r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)*(r6.wwww)).w;
    // 186: mul r3.w, r3.w, cb0[8].x
    r3.w = ((r3.wwww)*(source[8].xxxx)).w;
    // 187: mul r6.w, r7.w, cb0[8].z
    r6.w = ((r7.wwww)*(source[8].zzzz)).w;
    // 188: mul r11.xyz, cb0[6].xyzx, cb0[6].wwww
    r11.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 189: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 190: mad r11.xyz, r3.wwww, r13.xyzx, r11.xyzx
    r11.xyz = ((r3.wwww)*(r13.xyzx)+(r11.xyzx)).xyz;
    // 191: mad r11.xyz, -cb0[5].wwww, cb0[5].xyzx, r11.xyzx
    r11.xyz = ((-(source[5].wwww))*(source[5].xyzx)+(r11.xyzx)).xyz;
    // 192: mul r13.xyz, cb0[5].xyzx, cb0[5].wwww
    r13.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 193: mad r11.xyz, r2.xxxx, r11.xyzx, r13.xyzx
    r11.xyz = ((r2.xxxx)*(r11.xyzx)+(r13.xyzx)).xyz;
    // 194: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 195: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 196: mul r12.xyz, cb0[13].xyzx, cb0[13].wwww
    r12.xyz = ((source[13].xyzx)*(source[13].wwww)).xyz;
    // 197: mad r13.xyz, cb0[14].wwww, cb0[14].xyzx, -r12.xyzx
    r13.xyz = ((source[14].wwww)*(source[14].xyzx)+(-(r12.xyzx))).xyz;
    // 198: mad r12.xyz, r6.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r6.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 199: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 200: mad r2.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r2.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 201: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 202: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 203: mad r2.xyz, cb0[31].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[31].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 204: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 205: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 206: mad r2.xyz, cb0[31].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[31].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 207: mad r11.xyz, cb0[16].wwww, cb0[16].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[16].wwww)*(source[16].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 208: mad r12.xyz, cb0[17].wwww, cb0[17].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[17].wwww)*(source[17].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 209: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 210: mul r12.xyz, r2.xyzx, r11.xyzx
    r12.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 211: mad r2.xyz, -r2.xyzx, r11.xyzx, cb0[18].xyzx
    r2.xyz = ((-(r2.xyzx))*(r11.xyzx)+(source[18].xyzx)).xyz;
    // 212: mad r2.xyz, r2.wwww, r2.xyzx, r12.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 213: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 214: mul r2.xyz, r8.xyzx, r0.yzwy
    r2.xyz = ((r8.xyzx)*(r0.yzwy)).xyz;
    // 215: mad r3.xyz, r3.xyzx, r10.xyzx, -r2.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r2.xyzx))).xyz;
    // 216: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 217: mul r2.w, r2.w, cb0[35].z
    r2.w = ((r2.wwww)*(source[35].zzzz)).w;
    // 218: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 219: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 220: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 221: div r3.xyz, r9.xyzx, r3.xxxx
    r3.xyz = ((r9.xyzx)/(r3.xxxx)).xyz;
    // 222: dp3 r3.x, r3.xyzx, r4.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 223: add r3.y, -|r4.z|, l(1.000000)
    r3.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 224: mul r3.y, r4.w, r3.y
    r3.y = ((r4.wwww)*(r3.yyyy)).y;
    // 225: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 226: mul r3.z, |r3.x|, |r3.x|
    r3.z = ((abs(r3.xxxx))*(abs(r3.xxxx))).z;
    // 227: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 228: mul r3.z, r3.z, |r3.x|
    r3.z = ((r3.zzzz)*(abs(r3.xxxx))).z;
    // 229: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 230: movc r3.x, r3.x, l(0), r3.z
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).x;
    // 231: add r3.z, r3.x, l(-0.027778)
    r3.z = ((r3.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 232: mad r3.x, r3.x, r3.z, l(0.027778)
    r3.x = ((r3.xxxx)*(r3.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 233: div_sat r2.w, r3.x, r2.w
    r2.w = (saturate((r3.xxxx)/(r2.wwww))).w;
    // 234: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 235: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 236: mad r3.xzw, r1.yyyy, r2.xxyz, -r0.yyzw
    r3.xzw = ((r1.yyyy)*(r2.xxyz)+(-(r0.yyzw))).xzw;
    // 237: mad r0.xyz, r0.xxxx, r3.xzwx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xzwx)+(r0.yzwy)).xyz;
    // 238: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 239: add r3.xzw, -r0.xxyz, r0.wwww
    r3.xzw = ((-(r0.xxyz))+(r0.wwww)).xzw;
    // 240: mad r0.xyz, cb0[31].yyyy, r3.xzwx, r0.xyzx
    r0.xyz = ((source[31].yyyy)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 241: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 242: add r3.xzw, -r0.xxyz, r0.wwww
    r3.xzw = ((-(r0.xxyz))+(r0.wwww)).xzw;
    // 243: mad r0.xyz, cb0[31].zzzz, r3.xzwx, r0.xyzx
    r0.xyz = ((source[31].zzzz)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 244: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 245: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: mul r0.w, r0.w, cb0[36].z
    r0.w = ((r0.wwww)*(source[36].zzzz)).w;
    // 247: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 248: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 249: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 250: mul r1.y, cb0[4].z, l(1.500000)
    r1.y = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 251: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 252: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 253: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 254: mul r4.x, r1.y, l(0.125000)
    r4.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 255: mul r8.y, cb0[4].y, cb0[25].y
    r8.y = ((source[4].yyyy)*(source[25].yyyy)).y;
    // 256: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 257: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 258: add r3.xz, r4.xxyx, r8.xxyx
    r3.xz = ((r4.xxyx)+(r8.xxyx)).xz;
    // 259: frc r1.y, cb0[4].x
    r1.y = (frac(source[4].xxxx)).y;
    // 260: add r2.w, -r1.y, cb0[4].x
    r2.w = ((-(r1.yyyy))+(source[4].xxxx)).w;
    // 261: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 262: add r3.xz, r3.xxzx, r8.zzwz
    r3.xz = ((r3.xxzx)+(r8.zzwz)).xz;
    // 263: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xzxx, t6.xyzw, s7, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r3.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 264: mul r3.xzw, r0.wwww, r4.xxyz
    r3.xzw = ((r0.wwww)*(r4.xxyz)).xzw;
    // 265: mul r0.w, r1.y, r4.w
    r0.w = ((r1.yyyy)*(r4.wwww)).w;
    // 266: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 267: mad r3.xzw, r3.xxzw, l(2.000000, 0.000000, 2.000000, 2.000000), -r0.xxyz
    r3.xzw = ((r3.xxzw)*(float4(2.000000,0.000000,2.000000,2.000000))+(-(r0.xxyz))).xzw;
    // 268: mad r0.xyz, r0.wwww, r3.xzwx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 269: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 270: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 271: add r3.xz, -r4.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r3.xz = ((-(r4.xxyx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 272: add r3.xz, -r4.zzwz, r3.xxzx
    r3.xz = ((-(r4.zzwz))+(r3.xxzx)).xz;
    // 273: mad r3.xz, cb0[26].wwww, r3.xxzx, r4.zzwz
    r3.xz = ((source[26].wwww)*(r3.xxzx)+(r4.zzwz)).xz;
    // 274: mul r0.w, cb0[26].y, cb0[36].z
    r0.w = ((source[26].yyyy)*(source[36].zzzz)).w;
    // 275: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 276: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 277: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 278: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 279: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 280: mul r2.w, cb0[26].x, l(0.001000)
    r2.w = ((source[26].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 281: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 282: mad r3.xz, r2.wwww, r3.xxzx, r4.xxyx
    r3.xz = ((r2.wwww)*(r3.xxzx)+(r4.xxyx)).xz;
    // 283: dp2 r2.w, cb0[27].xyxx, r3.xzxx
    r2.w = (dot((source[27].xyxx).xy,(r3.xzxx).xy).xxxx).w;
    // 284: dp2 r4.y, cb0[28].xyxx, r3.xzxx
    r4.y = (dot((source[28].xyxx).xy,(r3.xzxx).xy).xxxx).y;
    // 285: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 286: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mad r3.xzw, r4.xxyz, l(3.500000, 0.000000, 3.500000, 3.500000), -r0.xxyz
    r3.xzw = ((r4.xxyz)*(float4(3.500000,0.000000,3.500000,3.500000))+(-(r0.xxyz))).xzw;
    // 289: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 290: mad r3.xzw, r2.wwww, r3.xxzw, r0.xxyz
    r3.xzw = ((r2.wwww)*(r3.xxzw)+(r0.xxyz)).xzw;
    // 291: mul_sat r3.xzw, r0.wwww, r3.xxzw
    r3.xzw = (saturate((r0.wwww)*(r3.xxzw))).xzw;
    // 292: mad r4.xyz, cb0[26].zzzz, r3.xzwx, -r0.xyzx
    r4.xyz = ((source[26].zzzz)*(r3.xzwx)+(-(r0.xyzx))).xyz;
    // 293: mul r3.xzw, r3.xxzw, cb0[26].zzzz
    r3.xzw = ((r3.xxzw)*(source[26].zzzz)).xzw;
    // 294: dp3 r0.w, r3.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 295: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 296: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 297: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 298: mad r3.xzw, r5.wwww, cb0[23].xxyz, -cb0[23].xxyz
    r3.xzw = ((r5.wwww)*(source[23].xxyz)+(-(source[23].xxyz))).xzw;
    // 299: mul r0.w, r5.w, cb0[22].w
    r0.w = ((r5.wwww)*(source[22].wwww)).w;
    // 300: mad r3.xzw, cb0[23].wwww, r3.xxzw, cb0[23].xxyz
    r3.xzw = ((source[23].wwww)*(r3.xxzw)+(source[23].xxyz)).xzw;
    // 301: mad r3.xzw, r0.wwww, cb0[22].xxyz, r3.xxzw
    r3.xzw = ((r0.wwww)*(source[22].xxyz)+(r3.xxzw)).xzw;
    // 302: add r0.w, cb0[2].y, cb0[2].x
    r0.w = ((source[2].yyyy)+(source[2].xxxx)).w;
    // 303: add r0.w, r0.w, cb0[2].z
    r0.w = ((r0.wwww)+(source[2].zzzz)).w;
    // 304: add r2.w, -r0.w, l(1000.000000)
    r2.w = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 305: mad r0.w, cb0[36].w, r2.w, r0.w
    r0.w = ((source[36].wwww)*(r2.wwww)+(r0.wwww)).w;
    // 306: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 307: mad r0.w, cb0[36].y, cb0[36].z, r0.w
    r0.w = ((source[36].yyyy)*(source[36].zzzz)+(r0.wwww)).w;
    // 308: mul r2.w, r0.w, l(3.524534)
    r2.w = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 309: sincos null, r2.w, r2.w
    r2.w = (cos(r2.wwww)).w;
    // 310: add r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)+(r2.wwww)).w;
    // 311: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 312: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 313: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 314: mad r0.w, r0.w, l(0.500000), cb0[36].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[36].xxxx)).w;
    // 315: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t7.xyzw, s6, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 316: mul r8.xyz, cb0[21].xyzx, cb0[35].wwww
    r8.xyz = ((source[21].xyzx)*(source[35].wwww)).xyz;
    // 317: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 318: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 319: mad r2.xyz, r1.yyyy, r2.xyzx, r4.xyzx
    r2.xyz = ((r1.yyyy)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 320: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 321: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 322: mad r2.xyz, cb0[31].yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((source[31].yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 323: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 324: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 325: mad r2.xyz, cb0[31].zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((source[31].zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 326: mad r2.xyz, r2.xyzx, r11.xyzx, r3.xzwx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)+(r3.xzwx)).xyz;
    // 327: log r0.w, |r3.y|
    r0.w = (log2(abs(r3.yyyy))).w;
    // 328: lt r1.y, |r3.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 329: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 330: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 331: mul r3.xyz, r0.wwww, cb0[24].xyzx
    r3.xyz = ((r0.wwww)*(source[24].xyzx)).xyz;
    // 332: movc r3.xyz, r1.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 333: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 334: add r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 335: dp3 r0.w, r1.xzwx, r1.xzwx
    r0.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 336: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 337: mul r1.xyz, r0.wwww, r1.xzwx
    r1.xyz = ((r0.wwww)*(r1.xzwx)).xyz;
    // 338: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 339: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 340: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 341: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 342: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 343: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 344: mul r3.yzw, r3.yyyy, cb0[38].xxyz
    r3.yzw = ((r3.yyyy)*(source[38].xxyz)).yzw;
    // 345: mad r3.xyz, r3.xxxx, cb0[37].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[37].xyzx)+(r3.yzwy)).xyz;
    // 346: mul r3.xyz, r3.xyzx, cb0[39].wwww
    r3.xyz = ((r3.xyzx)*(source[39].wwww)).xyz;
    // 347: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 348: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 349: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 350: mad o0.xyz, r0.xyzx, cb0[39].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[39].xyzx)+(r2.xyzx)).xyz;
    // 351: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 352: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 353: dp3 r0.x, r6.xyzx, r1.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 354: dp3 r0.z, r5.xyzx, r1.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 355: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 356: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 357: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 358: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 359: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 360: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 361: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 362: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 363: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 364: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 365: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 366: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 367: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 368: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 369: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 370: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 371: ret
    return output;
}

// source.character.equipment-native-178.v1 / source program 2f921a0dc9c17145b7105439f7ae3e70
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase178(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[23]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[25]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[26]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[34].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[27].xyzw
    r1.xyzw = ((r0.xyzw)*(source[27].xyzw)).xyzw;
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
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 15: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 16: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 17: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 18: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 19: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 20: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 21: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 22: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 23: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 24: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 25: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 26: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 27: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 28: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 29: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 30: add r0.z, -cb0[29].y, cb0[29].x
    r0.z = ((-(source[29].yyyy))+(source[29].xxxx)).z;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mad r0.z, r2.x, r0.z, cb0[29].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[29].yyyy)).z;
    // 33: add r0.w, -r0.z, cb0[29].z
    r0.w = ((-(r0.zzzz))+(source[29].zzzz)).w;
    // 34: mad r0.z, r2.y, r0.w, r0.z
    r0.z = ((r2.yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 35: add r0.w, -r0.z, cb0[29].w
    r0.w = ((-(r0.zzzz))+(source[29].wwww)).w;
    // 36: mad r0.z, r2.z, r0.w, r0.z
    r0.z = ((r2.zzzz)*(r0.wwww)+(r0.zzzz)).z;
    // 37: add r0.w, -r0.z, cb0[30].x
    r0.w = ((-(r0.zzzz))+(source[30].xxxx)).w;
    // 38: mad r0.z, r2.w, r0.w, r0.z
    r0.z = ((r2.wwww)*(r0.wwww)+(r0.zzzz)).z;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: add r0.w, -r3.w, l(1.000000)
    r0.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 42: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 43: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 44: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 45: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 47: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 48: add r1.w, -r0.w, cb0[32].x
    r1.w = ((-(r0.wwww))+(source[32].xxxx)).w;
    // 49: mad r0.w, r2.w, r1.w, r0.w
    r0.w = ((r2.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 50: add r1.w, -cb0[31].y, cb0[31].x
    r1.w = ((-(source[31].yyyy))+(source[31].xxxx)).w;
    // 51: mad r1.w, r2.x, r1.w, cb0[31].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[31].yyyy)).w;
    // 52: add r3.w, -r1.w, cb0[31].z
    r3.w = ((-(r1.wwww))+(source[31].zzzz)).w;
    // 53: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 54: add r3.w, -r1.w, cb0[31].w
    r3.w = ((-(r1.wwww))+(source[31].wwww)).w;
    // 55: mad r1.w, r2.z, r3.w, r1.w
    r1.w = ((r2.zzzz)*(r3.wwww)+(r1.wwww)).w;
    // 56: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 57: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 58: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 59: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 60: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 62: mad r4.xyzw, r0.ywyw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r0.ywyw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 63: dp2 r0.y, r4.zwzz, r4.zwzz
    r0.y = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).y;
    // 64: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 66: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 67: add r5.z, r0.y, l(0.000010)
    r5.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 68: mul r5.xy, r4.xyxx, cb0[28].xxxx
    r5.xy = ((r4.xyxx)*(source[28].xxxx)).xy;
    // 69: mad r4.xy, cb0[28].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[28].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 70: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 71: mad r4.xyz, r2.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 72: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: mad r5.xyz, cb0[30].wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((source[30].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 74: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 75: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 76: div r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)/(r0.yyyy)).xyz;
    // 77: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 78: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 79: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 80: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 81: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 82: mul r7.xyz, r0.yyyy, v0.xyzx
    r7.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 83: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 84: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 85: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 86: dp3 r9.y, r8.xyzx, r5.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 87: dp3 r9.x, r7.xyzx, r5.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 88: dp3 r9.z, r6.xyzx, r5.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 89: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 90: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 91: mul r5.xyz, r0.yyyy, v5.xyzx
    r5.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 92: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 93: dp3 r11.y, r8.xyzx, r5.xyzx
    r11.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 94: dp3 r11.x, r7.xyzx, r5.xyzx
    r11.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 95: dp3 r11.z, r6.xyzx, r5.xyzx
    r11.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 96: dp3 r0.y, r9.xyzx, r11.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 97: mul r9.xyz, r9.xyzx, r0.yyyy
    r9.xyz = ((r9.xyzx)*(r0.yyyy)).xyz;
    // 98: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 99: mov r9.w, -r9.x
    r9.w = (-(r9.xxxx)).w;
    // 100: dp2 r0.y, r9.ywyy, r9.ywyy
    r0.y = (dot((r9.ywyy).xy,(r9.ywyy).xy).xxxx).y;
    // 101: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 102: div r0.yw, r9.yyyw, r0.yyyy
    r0.yw = ((r9.yyyw)/(r0.yyyy)).yw;
    // 103: mad r1.w, -r9.z, l(0.250000), l(0.250000)
    r1.w = ((-(r9.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 104: add r3.w, r9.z, l(1.000000)
    r3.w = ((r9.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 106: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 107: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t6.xywz, s5, r0.x
    r0.xyw = ((g_SourceCharacterTexture5.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 108: log r9.xyz, r0.xywx
    r9.xyz = (log2(r0.xywx)).xyz;
    // 109: rcp r1.w, cb0[32].y
    r1.w = (1.0/(source[32].yyyy)).w;
    // 110: mul r11.xyz, r9.xyzx, r1.wwww
    r11.xyz = ((r9.xyzx)*(r1.wwww)).xyz;
    // 111: mul r9.xyz, r9.xyzx, cb0[32].yyyy
    r9.xyz = ((r9.xyzx)*(source[32].yyyy)).xyz;
    // 112: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 113: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 114: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 115: mad r9.xyz, r9.xyzx, cb0[32].yyyy, r11.xyzx
    r9.xyz = ((r9.xyzx)*(source[32].yyyy)+(r11.xyzx)).xyz;
    // 116: add r0.xyw, r0.xyxw, r9.xyxz
    r0.xyw = ((r0.xyxw)+(r9.xyxz)).xyw;
    // 117: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 118: add r1.w, cb0[32].y, l(1.000000)
    r1.w = ((source[32].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 120: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r9.xyz, -cb0[18].xyzx, cb0[19].xyzx
    r9.xyz = ((-(source[18].xyzx))+(source[19].xyzx)).xyz;
    // 122: mad r9.xyz, r3.wwww, r9.xyzx, cb0[18].xyzx
    r9.xyz = ((r3.wwww)*(r9.xyzx)+(source[18].xyzx)).xyz;
    // 123: mul r0.xyw, r0.xxxx, r9.xyxz
    r0.xyw = ((r0.xxxx)*(r9.xyxz)).xyw;
    // 124: mul r0.xyw, r0.xyxw, cb0[32].zzzz
    r0.xyw = ((r0.xyxw)*(source[32].zzzz)).xyw;
    // 125: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r9.xyz, -r3.xyzx, r1.wwww
    r9.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 127: mad r3.yzw, cb0[30].yyyy, r9.xxyz, r3.xxyz
    r3.yzw = ((source[30].yyyy)*(r9.xxyz)+(r3.xxyz)).yzw;
    // 128: dp3 r1.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: add r9.xyz, -r3.yzwy, r1.wwww
    r9.xyz = ((-(r3.yzwy))+(r1.wwww)).xyz;
    // 130: mad r3.yzw, cb0[30].zzzz, r9.xxyz, r3.yyzw
    r3.yzw = ((source[30].zzzz)*(r9.xxyz)+(r3.yyzw)).yzw;
    // 131: dp3 r1.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: add r9.xyz, -r3.yzwy, r1.wwww
    r9.xyz = ((-(r3.yzwy))+(r1.wwww)).xyz;
    // 133: mul r9.xyz, r9.xyzx, cb0[32].wwww
    r9.xyz = ((r9.xyzx)*(source[32].wwww)).xyz;
    // 134: add r1.w, r2.y, r2.x
    r1.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 135: add r1.w, r2.z, r1.w
    r1.w = ((r2.zzzz)+(r1.wwww)).w;
    // 136: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 137: mad r3.yzw, r1.wwww, r9.xxyz, r3.yyzw
    r3.yzw = ((r1.wwww)*(r9.xxyz)+(r3.yyzw)).yzw;
    // 138: add r9.xyz, -r3.yzwy, r3.xxxx
    r9.xyz = ((-(r3.yzwy))+(r3.xxxx)).xyz;
    // 139: mad r3.xyz, r2.wwww, r9.xyzx, r3.yzwy
    r3.xyz = ((r2.wwww)*(r9.xyzx)+(r3.yzwy)).xyz;
    // 140: max r9.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 141: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 142: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 143: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 144: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 146: add r3.w, cb0[33].w, -cb0[34].x
    r3.w = ((source[33].wwww)+(-(source[34].xxxx))).w;
    // 147: mad r3.w, r2.w, r3.w, cb0[34].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[34].xxxx)).w;
    // 148: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 149: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 150: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mad r3.w, -r1.w, r1.w, l(1.000000)
    r3.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: max r3.w, r3.w, l(0.001000)
    r3.w = (max(r3.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 153: div r3.w, cb0[34].y, r3.w
    r3.w = ((source[34].yyyy)/(r3.wwww)).w;
    // 154: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 155: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 156: div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // 157: dp3 r4.w, r4.xyzx, r5.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 158: mul_sat r5.w, r4.w, cb0[33].x
    r5.w = (saturate((r4.wwww)*(source[33].xxxx))).w;
    // 159: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul_sat r6.w, r5.z, cb0[33].x
    r6.w = (saturate((r5.zzzz)*(source[33].xxxx))).w;
    // 162: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: add_sat r6.w, r6.w, -cb0[33].y
    r6.w = (saturate((r6.wwww)+(-(source[33].yyyy)))).w;
    // 164: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 165: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 166: mul r7.w, r7.w, cb0[33].z
    r7.w = ((r7.wwww)*(source[33].zzzz)).w;
    // 167: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 168: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 169: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 170: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 171: mul r9.xyz, r0.xywx, r3.wwww
    r9.xyz = ((r0.xywx)*(r3.wwww)).xyz;
    // 172: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 173: add r11.xyz, -r1.xyzx, r3.wwww
    r11.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 174: mad r1.xyz, cb0[30].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[30].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 175: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: add r11.xyz, -r1.xyzx, r3.wwww
    r11.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 177: mad r1.xyz, cb0[30].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[30].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 178: add r11.xy, v4.zwzz, l(-0.050000, -0.050000, 0.000000, 0.000000)
    r11.xy = ((v4.zwzz)+(float4(-0.050000,-0.050000,0.000000,0.000000))).xy;
    // 179: mul_sat r11.xy, r11.xyxx, l(256.000000, 256.000000, 0.000000, 0.000000)
    r11.xy = (saturate((r11.xyxx)*(float4(256.000000,256.000000,0.000000,0.000000)))).xy;
    // 180: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 181: mad r3.w, -r11.x, r11.y, l(1.000000)
    r3.w = ((-(r11.xxxx))*(r11.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 182: mul r11.xy, v4.wzww, l(4.000000, 4.000000, 0.000000, 0.000000)
    r11.xy = ((v4.wzww)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 183: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r11.xyxx, t5.xyzw, s3, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r11.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 184: dp4 r6.w, r11.xyzw, cb0[11].xyzw
    r6.w = (dot((r11.xyzw).xyzw,(source[11].xyzw).xyzw).xxxx).w;
    // 185: mul r6.w, r3.w, r6.w
    r6.w = ((r3.wwww)*(r6.wwww)).w;
    // 186: mul r6.w, r6.w, cb0[7].y
    r6.w = ((r6.wwww)*(source[7].yyyy)).w;
    // 187: mul r12.xyz, cb0[9].xyzx, cb0[9].wwww
    r12.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 188: mad r13.xyz, cb0[10].wwww, cb0[10].xyzx, -r12.xyzx
    r13.xyz = ((source[10].wwww)*(source[10].xyzx)+(-(r12.xyzx))).xyz;
    // 189: mad r12.xyz, r6.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r6.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 190: dp4 r6.w, r11.xyzw, cb0[8].xyzw
    r6.w = (dot((r11.xyzw).xyzw,(source[8].xyzw).xyzw).xxxx).w;
    // 191: dp4 r7.w, r11.xyzw, cb0[14].xyzw
    r7.w = (dot((r11.xyzw).xyzw,(source[14].xyzw).xyzw).xxxx).w;
    // 192: mul r7.w, r3.w, r7.w
    r7.w = ((r3.wwww)*(r7.wwww)).w;
    // 193: mul r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)*(r6.wwww)).w;
    // 194: mul r3.w, r3.w, cb0[7].x
    r3.w = ((r3.wwww)*(source[7].xxxx)).w;
    // 195: mul r6.w, r7.w, cb0[7].z
    r6.w = ((r7.wwww)*(source[7].zzzz)).w;
    // 196: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 197: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 198: mad r11.xyz, r3.wwww, r13.xyzx, r11.xyzx
    r11.xyz = ((r3.wwww)*(r13.xyzx)+(r11.xyzx)).xyz;
    // 199: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r11.xyzx
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r11.xyzx)).xyz;
    // 200: mul r13.xyz, cb0[4].xyzx, cb0[4].wwww
    r13.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 201: mad r11.xyz, r2.xxxx, r11.xyzx, r13.xyzx
    r11.xyz = ((r2.xxxx)*(r11.xyzx)+(r13.xyzx)).xyz;
    // 202: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 203: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 204: mul r12.xyz, cb0[12].xyzx, cb0[12].wwww
    r12.xyz = ((source[12].xyzx)*(source[12].wwww)).xyz;
    // 205: mad r13.xyz, cb0[13].wwww, cb0[13].xyzx, -r12.xyzx
    r13.xyz = ((source[13].wwww)*(source[13].xyzx)+(-(r12.xyzx))).xyz;
    // 206: mad r12.xyz, r6.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r6.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 207: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 208: mad r2.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r2.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 209: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 210: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 211: mad r2.xyz, cb0[30].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[30].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 212: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 213: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 214: mad r2.xyz, cb0[30].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[30].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 215: mad r11.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 216: mad r12.xyz, cb0[16].wwww, cb0[16].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[16].wwww)*(source[16].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 217: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 218: mul r12.xyz, r2.xyzx, r11.xyzx
    r12.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 219: mad r2.xyz, -r2.xyzx, r11.xyzx, cb0[17].xyzx
    r2.xyz = ((-(r2.xyzx))*(r11.xyzx)+(source[17].xyzx)).xyz;
    // 220: mad r2.xyz, r2.wwww, r2.xyzx, r12.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 221: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 222: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 223: mad r2.xyz, r3.xyzx, r9.xyzx, -r0.xywx
    r2.xyz = ((r3.xyzx)*(r9.xyzx)+(-(r0.xywx))).xyz;
    // 224: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 225: mul r2.w, r2.w, cb0[34].z
    r2.w = ((r2.wwww)*(source[34].zzzz)).w;
    // 226: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 227: dp3 r2.x, r10.xyzx, r10.xyzx
    r2.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 228: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 229: div r2.yzw, r10.xxyz, r2.yyyy
    r2.yzw = ((r10.xxyz)/(r2.yyyy)).yzw;
    // 230: dp3 r2.y, r2.yzwy, r5.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // 231: add r2.z, -|r5.z|, l(1.000000)
    r2.z = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 232: mul r2.z, r4.w, r2.z
    r2.z = ((r4.wwww)*(r2.zzzz)).z;
    // 233: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 234: mul r2.w, |r2.y|, |r2.y|
    r2.w = ((abs(r2.yyyy))*(abs(r2.yyyy))).w;
    // 235: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 236: mul r2.w, r2.w, |r2.y|
    r2.w = ((r2.wwww)*(abs(r2.yyyy))).w;
    // 237: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 238: movc r2.y, r2.y, l(0), r2.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 239: add r2.w, r2.y, l(-0.027778)
    r2.w = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 240: mad r2.y, r2.y, r2.w, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 241: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 242: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 243: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 244: mad r2.xyw, r0.zzzz, r0.xyxw, -r1.xyxz
    r2.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r1.xyxz))).xyw;
    // 245: mad r1.xyz, r1.wwww, r2.xywx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 246: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 247: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 248: mad r1.xyz, cb0[30].yyyy, r2.xywx, r1.xyzx
    r1.xyz = ((source[30].yyyy)*(r2.xywx)+(r1.xyzx)).xyz;
    // 249: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 250: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 251: mad r1.xyz, cb0[30].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[30].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 252: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 253: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 254: mul r0.z, r0.z, cb0[34].w
    r0.z = ((r0.zzzz)*(source[34].wwww)).z;
    // 255: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 256: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 257: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 258: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 259: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 260: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 261: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 262: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 263: mul r3.y, cb0[3].y, cb0[23].y
    r3.y = ((source[3].yyyy)*(source[23].yyyy)).y;
    // 264: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 265: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 266: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 267: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 268: add r2.w, -r1.w, cb0[3].x
    r2.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 269: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 270: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 271: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t7.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 272: mul r2.xyw, r0.zzzz, r3.xyxz
    r2.xyw = ((r0.zzzz)*(r3.xyxz)).xyw;
    // 273: mul r0.z, r1.w, r3.w
    r0.z = ((r1.wwww)*(r3.wwww)).z;
    // 274: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 275: mad r2.xyw, r2.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxz
    r2.xyw = ((r2.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxz))).xyw;
    // 276: mad r1.xyz, r0.zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 277: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 278: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 279: add r2.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 280: add r2.xy, -r3.zwzz, r2.xyxx
    r2.xy = ((-(r3.zwzz))+(r2.xyxx)).xy;
    // 281: mad r2.xy, cb0[24].wwww, r2.xyxx, r3.zwzz
    r2.xy = ((source[24].wwww)*(r2.xyxx)+(r3.zwzz)).xy;
    // 282: mul r0.z, cb0[24].y, cb0[34].w
    r0.z = ((source[24].yyyy)*(source[34].wwww)).z;
    // 283: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 284: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 285: mul r3.y, r0.z, l(0.020000)
    r3.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 286: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 287: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 288: mul r2.w, cb0[24].x, l(0.001000)
    r2.w = ((source[24].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 289: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 290: mad r2.xy, r2.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 291: dp2 r2.w, cb0[25].xyxx, r2.xyxx
    r2.w = (dot((source[25].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 292: dp2 r2.y, cb0[26].xyxx, r2.xyxx
    r2.y = (dot((source[26].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 293: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 294: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 295: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t7.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 296: mad r2.xyw, r3.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r3.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 297: mul r3.x, r3.w, l(0.900000)
    r3.x = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 298: mad r2.xyw, r3.xxxx, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.xxxx)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 299: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 300: mad r3.xyz, cb0[24].zzzz, r2.xywx, -r1.xyzx
    r3.xyz = ((source[24].zzzz)*(r2.xywx)+(-(r1.xyzx))).xyz;
    // 301: mul r2.xyw, r2.xyxw, cb0[24].zzzz
    r2.xyw = ((r2.xyxw)*(source[24].zzzz)).xyw;
    // 302: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 303: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 304: mad r1.xyz, r0.zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 305: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 306: mul r2.xyw, r0.xyxw, r1.wwww
    r2.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 307: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 308: mad r0.xyz, -r1.wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(r1.wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 309: mad r0.xyz, cb0[30].yyyy, r0.xyzx, r2.xywx
    r0.xyz = ((source[30].yyyy)*(r0.xyzx)+(r2.xywx)).xyz;
    // 310: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 311: add r2.xyw, -r0.xyxz, r0.wwww
    r2.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 312: mad r0.xyz, cb0[30].zzzz, r2.xywx, r0.xyzx
    r0.xyz = ((source[30].zzzz)*(r2.xywx)+(r0.xyzx)).xyz;
    // 313: mad r2.xyw, r5.wwww, cb0[21].xyxz, -cb0[21].xyxz
    r2.xyw = ((r5.wwww)*(source[21].xyxz)+(-(source[21].xyxz))).xyw;
    // 314: mul r0.w, r5.w, cb0[20].w
    r0.w = ((r5.wwww)*(source[20].wwww)).w;
    // 315: mad r2.xyw, cb0[21].wwww, r2.xyxw, cb0[21].xyxz
    r2.xyw = ((source[21].wwww)*(r2.xyxw)+(source[21].xyxz)).xyw;
    // 316: mad r2.xyw, r0.wwww, cb0[20].xyxz, r2.xyxw
    r2.xyw = ((r0.wwww)*(source[20].xyxz)+(r2.xyxw)).xyw;
    // 317: mad r0.xyz, r0.xyzx, r11.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.xywx)).xyz;
    // 318: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 319: lt r1.w, |r2.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 320: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 321: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 322: mul r2.xyz, r0.wwww, cb0[22].xyzx
    r2.xyz = ((r0.wwww)*(source[22].xyzx)).xyz;
    // 323: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 324: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 325: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 326: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 327: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 328: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 329: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 330: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 331: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 332: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 333: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 334: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 335: mul r3.yzw, r3.yyyy, cb0[36].xxyz
    r3.yzw = ((r3.yyyy)*(source[36].xxyz)).yzw;
    // 336: mad r3.xyz, r3.xxxx, cb0[35].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[35].xyzx)+(r3.yzwy)).xyz;
    // 337: mul r3.xyz, r3.xyzx, cb0[37].wwww
    r3.xyz = ((r3.xyzx)*(source[37].wwww)).xyz;
    // 338: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 339: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 340: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 341: mad o0.xyz, r1.xyzx, cb0[37].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[37].xyzx)+(r0.xyzx)).xyz;
    // 342: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 343: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 344: dp3 r0.x, r7.xyzx, r2.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 345: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 346: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 347: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 348: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 349: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 350: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 351: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 352: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 353: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 354: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 355: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 356: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 357: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 358: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 359: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 360: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 361: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 362: ret
    return output;
}

// source.character.equipment-native-179.v1 / source program d9fbf5826d3928459780e13498f0e372
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase179(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add_sat r0.w, r0.w, -cb0[24].x
    r0.w = (saturate((r0.wwww)+(-(source[24].xxxx)))).w;
    // 3: add r0.w, r0.w, l(-0.001000)
    r0.w = ((r0.wwww)+(float4(-0.001000,-0.001000,-0.001000,-0.001000))).w;
    // 4: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 5: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: add r0.w, v4.w, l(0.500000)
    r0.w = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 8: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 9: add r1.x, -r0.w, l(1.000000)
    r1.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 10: mad r1.y, cb0[17].y, l(-3.500000), l(5.000000)
    r1.y = ((source[17].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 11: mul r1.y, r1.y, cb0[18].x
    r1.y = ((r1.yyyy)*(source[18].xxxx)).y;
    // 12: add r1.z, -v4.z, l(1.000000)
    r1.z = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 13: add r1.w, -r1.z, v4.z
    r1.w = ((-(r1.zzzz))+(v4.zzzz)).w;
    // 14: mad r1.z, cb0[18].y, r1.w, r1.z
    r1.z = ((source[18].yyyy)*(r1.wwww)+(r1.zzzz)).z;
    // 15: mul r1.w, r1.z, cb0[18].z
    r1.w = ((r1.zzzz)*(source[18].zzzz)).w;
    // 16: mad r1.z, r1.w, l(0.750000), r1.z
    r1.z = ((r1.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r1.zzzz)).z;
    // 17: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 18: mad_sat r1.z, cb0[19].x, r1.z, r1.z
    r1.z = (saturate((source[19].xxxx)*(r1.zzzz)+(r1.zzzz))).z;
    // 19: mul r1.w, cb0[18].w, l(0.700000)
    r1.w = ((source[18].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 20: add r2.x, -r0.y, l(1.000000)
    r2.x = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yzw;
    // 22: mad r3.xy, r2.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r2.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: mad r2.y, r3.x, r0.x, l(0.200000)
    r2.y = ((r3.xxxx)*(r0.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 24: add r2.x, -r2.y, r2.x
    r2.x = ((-(r2.yyyy))+(r2.xxxx)).x;
    // 25: mad r2.z, cb0[20].x, r2.x, r2.y
    r2.z = ((source[20].xxxx)*(r2.xxxx)+(r2.yyyy)).z;
    // 26: mad r2.x, cb0[19].z, r2.x, r2.y
    r2.x = ((source[19].zzzz)*(r2.xxxx)+(r2.yyyy)).x;
    // 27: add r2.x, -r1.z, r2.x
    r2.x = ((-(r1.zzzz))+(r2.xxxx)).x;
    // 28: mad r2.x, r1.w, r2.x, r1.z
    r2.x = ((r1.wwww)*(r2.xxxx)+(r1.zzzz)).x;
    // 29: div r2.x, r2.x, cb0[19].y
    r2.x = ((r2.xxxx)/(source[19].yyyy)).x;
    // 30: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mul r2.x, r1.y, r2.x
    r2.x = ((r1.yyyy)*(r2.xxxx)).x;
    // 32: mul r2.x, r2.x, l(4.000000)
    r2.x = ((r2.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 33: mul_sat r0.w, r0.w, r2.x
    r0.w = (saturate((r0.wwww)*(r2.xxxx))).w;
    // 34: add r2.x, -r1.z, r2.z
    r2.x = ((-(r1.zzzz))+(r2.zzzz)).x;
    // 35: mad r1.z, r1.w, r2.x, r1.z
    r1.z = ((r1.wwww)*(r2.xxxx)+(r1.zzzz)).z;
    // 36: div r1.z, r1.z, cb0[19].w
    r1.z = ((r1.zzzz)/(source[19].wwww)).z;
    // 37: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 39: mul r1.y, r1.y, l(4.000000)
    r1.y = ((r1.yyyy)*(float4(4.000000,4.000000,4.000000,4.000000))).y;
    // 40: mul_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)*(r1.yyyy))).x;
    // 41: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 42: add r0.w, -r0.y, r0.w
    r0.w = ((-(r0.yyyy))+(r0.wwww)).w;
    // 43: mad r0.w, cb0[20].y, r0.w, r0.y
    r0.w = ((source[20].yyyy)*(r0.wwww)+(r0.yyyy)).w;
    // 44: add r1.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r1.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 45: mul r1.xyz, r1.xyzx, cb0[17].xxxx
    r1.xyz = ((r1.xyzx)*(source[17].xxxx)).xyz;
    // 46: mad r1.xyz, r0.wwww, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 47: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 49: mad r1.xyz, cb0[20].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 50: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 52: mad r1.xyz, cb0[20].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 53: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mad r4.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 56: mul r4.xyz, r1.xyzx, r2.xyzx
    r4.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 57: mad r1.xyz, r1.xyzx, r2.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 58: mad r2.xyz, cb0[9].xyzx, r0.xyzx, -r0.xyzx
    r2.xyz = ((source[9].xyzx)*(r0.xyzx)+(-(r0.xyzx))).xyz;
    // 59: mad r2.xyz, r2.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 60: mad r2.xyz, -r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r4.xyzx)+(r2.xyzx)).xyz;
    // 61: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 62: mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 63: add r0.w, -cb0[10].w, l(1.000000)
    r0.w = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: mul r0.w, r0.w, cb0[22].w
    r0.w = ((r0.wwww)*(source[22].wwww)).w;
    // 65: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 66: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 67: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 68: mul r1.w, cb0[10].z, l(1.500000)
    r1.w = ((source[10].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 69: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 70: mad r0.w, r0.w, l(0.500000), cb0[10].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 71: mul r4.y, cb0[10].y, cb0[11].y
    r4.y = ((source[10].yyyy)*(source[11].yyyy)).y;
    // 72: mul r5.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r5.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 73: frc r1.w, r5.x
    r1.w = (frac(r5.xxxx)).w;
    // 74: mul r5.y, r1.w, l(0.125000)
    r5.y = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 75: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 76: add r4.xy, r4.xyxx, r5.yzyy
    r4.xy = ((r4.xyxx)+(r5.yzyy)).xy;
    // 77: frc r1.w, cb0[10].x
    r1.w = (frac(source[10].xxxx)).w;
    // 78: add r3.w, -r1.w, cb0[10].x
    r3.w = ((-(r1.wwww))+(source[10].xxxx)).w;
    // 79: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 80: add r4.xy, r4.xyxx, r4.zwzz
    r4.xy = ((r4.xyxx)+(r4.zwzz)).xy;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 82: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 83: mul r0.w, r1.w, r4.w
    r0.w = ((r1.wwww)*(r4.wwww)).w;
    // 84: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 85: mad r2.xyz, r0.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 86: add r4.xyz, v7.xyzx, cb0[0].yzwy
    r4.xyz = ((v7.xyzx)+(source[0].yzwy)).xyz;
    // 87: add r5.xyzw, r4.yzxy, -cb0[1].yzxy
    r5.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 88: add r4.xyz, -r4.xyzx, cb0[0].yzwy
    r4.xyz = ((-(r4.xyzx))+(source[0].yzwy)).xyz;
    // 89: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 90: add r5.xy, -r5.zwzz, r5.xyxx
    r5.xy = ((-(r5.zwzz))+(r5.xyxx)).xy;
    // 91: mad r5.xy, cb0[12].wwww, r5.xyxx, r5.zwzz
    r5.xy = ((source[12].wwww)*(r5.xyxx)+(r5.zwzz)).xy;
    // 92: mul r0.w, cb0[12].y, cb0[22].w
    r0.w = ((source[12].yyyy)*(source[22].wwww)).w;
    // 93: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 94: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 95: mul r6.y, r0.w, l(0.020000)
    r6.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 96: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 98: mul r1.w, cb0[12].x, l(0.001000)
    r1.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 99: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 100: mad r5.xy, r1.wwww, r5.xyxx, r6.xyxx
    r5.xy = ((r1.wwww)*(r5.xyxx)+(r6.xyxx)).xy;
    // 101: dp2 r1.w, cb0[13].xyxx, r5.xyxx
    r1.w = (dot((source[13].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 102: dp2 r5.y, cb0[14].xyxx, r5.xyxx
    r5.y = (dot((source[14].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 103: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 104: mul r5.x, r1.w, l(0.125000)
    r5.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 106: mad r5.xyz, r5.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r2.xyzx
    r5.xyz = ((r5.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r2.xyzx))).xyz;
    // 107: mul r1.w, r5.w, l(0.900000)
    r1.w = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 108: mad r5.xyz, r1.wwww, r5.xyzx, r2.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 109: mul_sat r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = (saturate((r0.wwww)*(r5.xyzx))).xyz;
    // 110: mad r6.xyz, cb0[12].zzzz, r5.xyzx, -r2.xyzx
    r6.xyz = ((source[12].zzzz)*(r5.xyzx)+(-(r2.xyzx))).xyz;
    // 111: mul r5.xyz, r5.xyzx, cb0[12].zzzz
    r5.xyz = ((r5.xyzx)*(source[12].zzzz)).xyz;
    // 112: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 114: mad r2.xyz, r0.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 115: mul r2.xyz, r2.xyzx, cb0[23].xxxx
    r2.xyz = ((r2.xyzx)*(source[23].xxxx)).xyz;
    // 116: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 117: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 119: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 120: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 121: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 122: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 123: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 124: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 125: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 126: mul r5.xyz, r0.wwww, v5.xyzx
    r5.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 127: dp3 r0.w, r3.xyzx, r5.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 128: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: mad r5.xyw, r1.wwww, cb0[8].xyxz, r2.xyxz
    r5.xyw = ((r1.wwww)*(source[8].xyxz)+(r2.xyxz)).xyw;
    // 130: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 131: add r1.w, -|r5.z|, l(1.000000)
    r1.w = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: add r3.w, -|r0.w|, l(1.000000)
    r3.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mul_sat r0.w, r0.w, cb0[23].y
    r0.w = (saturate((r0.wwww)*(source[23].yyyy))).w;
    // 134: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 136: mad r6.xyz, r1.wwww, cb0[15].xyzx, -cb0[15].xyzx
    r6.xyz = ((r1.wwww)*(source[15].xyzx)+(-(source[15].xyzx))).xyz;
    // 137: mad r6.xyz, cb0[15].wwww, r6.xyzx, cb0[15].xyzx
    r6.xyz = ((source[15].wwww)*(r6.xyzx)+(source[15].xyzx)).xyz;
    // 138: add r5.xyw, r5.xyxw, r6.xyxz
    r5.xyw = ((r5.xyxw)+(r6.xyxz)).xyw;
    // 139: mul_sat r1.w, r5.z, cb0[23].y
    r1.w = (saturate((r5.zzzz)*(source[23].yyyy))).w;
    // 140: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add_sat r1.w, r1.w, -cb0[23].z
    r1.w = (saturate((r1.wwww)+(-(source[23].zzzz)))).w;
    // 142: log r3.w, r1.w
    r3.w = (log2(r1.wwww)).w;
    // 143: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 144: mul r3.w, r3.w, cb0[23].w
    r3.w = ((r3.wwww)*(source[23].wwww)).w;
    // 145: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 146: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 147: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 148: mul r6.xyz, r0.wwww, cb0[16].xyzx
    r6.xyz = ((r0.wwww)*(source[16].xyzx)).xyz;
    // 149: movc r6.xyz, r1.wwww, l(0,0,0,0), r6.xyzx
    r6.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xyzx)).xyz;
    // 150: add r5.xyw, r5.xyxw, r6.xyxz
    r5.xyw = ((r5.xyxw)+(r6.xyxz)).xyw;
    // 151: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 152: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 153: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 154: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 155: mov_sat r0.w, r5.z
    r0.w = (saturate(r5.zzzz)).w;
    // 156: mul r6.xyz, r4.xyzx, r5.zzzz
    r6.xyz = ((r4.xyzx)*(r5.zzzz)).xyz;
    // 157: mad r4.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r4.xyzx)).xyz;
    // 158: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 159: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 160: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 161: mul r1.xyz, r1.xyzx, cb0[21].xxxx
    r1.xyz = ((r1.xyzx)*(source[21].xxxx)).xyz;
    // 162: mad r0.xyz, cb0[21].yyyy, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[21].yyyy)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 163: mad r0.xyz, r2.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 164: add r0.w, -cb0[22].x, l(0.200000)
    r0.w = ((-(source[22].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 165: mad r0.w, r2.w, r0.w, cb0[22].x
    r0.w = ((r2.wwww)*(r0.wwww)+(source[22].xxxx)).w;
    // 166: mad r0.w, r0.w, l(4.500000), l(0.500000)
    r0.w = ((r0.wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 167: mul r0.w, r0.w, cb0[22].y
    r0.w = ((r0.wwww)*(source[22].yyyy)).w;
    // 168: mul r0.w, r0.w, l(0.050000)
    r0.w = ((r0.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 169: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 170: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 171: div r1.x, r4.z, r1.x
    r1.x = ((r4.zzzz)/(r1.xxxx)).x;
    // 172: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 173: dp3 r1.y, v1.xyzx, v1.xyzx
    r1.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 174: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 175: mul r1.yzw, r1.yyyy, v1.xxyz
    r1.yzw = ((r1.yyyy)*(v1.xxyz)).yzw;
    // 176: dp3 r2.w, r1.yzwy, r3.xyzx
    r2.w = (dot((r1.yzwy).xyz,(r3.xyzx).xyz).xxxx).w;
    // 177: add r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)+(r2.wwww)).x;
    // 178: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 179: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 180: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 181: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 182: log r2.w, r1.x
    r2.w = (log2(r1.xxxx)).w;
    // 183: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 184: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 185: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 186: mul r0.w, r0.w, cb0[22].z
    r0.w = ((r0.wwww)*(source[22].zzzz)).w;
    // 187: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 188: mad r0.xyz, r0.wwww, r0.xyzx, r5.xywx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xywx)).xyz;
    // 189: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 190: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 191: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 192: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 193: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 194: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 195: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 196: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 197: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 198: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 199: mul r4.yzw, r4.yyyy, cb0[26].xxyz
    r4.yzw = ((r4.yyyy)*(source[26].xxyz)).yzw;
    // 200: mad r4.xyz, r4.xxxx, cb0[25].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[25].xyzx)+(r4.yzwy)).xyz;
    // 201: mul r4.xyz, r4.xyzx, cb0[27].wwww
    r4.xyz = ((r4.xyzx)*(source[27].wwww)).xyz;
    // 202: mad r0.xyz, r4.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 203: mul r4.xyz, r2.xyzx, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 204: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 205: mad o0.xyz, r2.xyzx, cb0[27].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[27].xyzx)+(r0.xyzx)).xyz;
    // 206: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 207: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 208: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 209: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 210: mul r0.xyz, r0.xxxx, v0.xyzx
    r0.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 211: mul r2.xyz, r0.yzxy, r1.wyzw
    r2.xyz = ((r0.yzxy)*(r1.wyzw)).xyz;
    // 212: mad r2.xyz, r1.zwyz, r0.zxyz, -r2.xyzx
    r2.xyz = ((r1.zwyz)*(r0.zxyz)+(-(r2.xyzx))).xyz;
    // 213: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 214: movc r0.w, v8.x, l(1.000000), l(-1.000000)
    r0.w = ((asuint(v8.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 215: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 216: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 217: dp3 r2.y, r2.xyzx, r3.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 218: dp3 r2.z, r1.yzwy, r3.xyzx
    r2.z = (dot((r1.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 219: dp3 r2.x, r0.xyzx, r3.xyzx
    r2.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 220: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 221: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 222: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 223: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 224: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 225: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 226: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 227: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 228: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 229: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 230: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 231: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 232: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 233: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 234: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 235: ret
    return output;
}

// source.character.equipment-native-180.v1 / source program 88ff2901ff0d8140a4a1db1cc43090de
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase180(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].x=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[25]=g_SourceCharacterEnvironmentColor; source[26]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
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
    // 7: add r0.x, -cb0[8].w, l(1.000000)
    r0.x = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul r0.x, r0.x, cb0[20].x
    r0.x = ((r0.xxxx)*(source[20].xxxx)).x;
    // 9: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 10: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 11: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r1.x, cb0[8].z, l(1.500000)
    r1.x = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 13: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 15: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 16: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[8].y, cb0[15].y
    r2.y = ((source[8].yyyy)*(source[15].yyyy)).y;
    // 18: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 21: frc r1.z, cb0[8].x
    r1.z = (frac(source[8].xxxx)).z;
    // 22: add r1.w, -r1.z, cb0[8].x
    r1.w = ((-(r1.zzzz))+(source[8].xxxx)).w;
    // 23: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 24: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r1.xyw, r0.xxxx, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r2.xyxz)).xyw;
    // 27: mul r0.x, r1.z, r2.w
    r0.x = ((r1.zzzz)*(r2.wwww)).x;
    // 28: dp3 r1.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 29: add r2.xyz, -r0.yzwy, r1.zzzz
    r2.xyz = ((-(r0.yzwy))+(r1.zzzz)).xyz;
    // 30: mad r2.xyz, cb0[18].yyyy, r2.xyzx, r0.yzwy
    r2.xyz = ((source[18].yyyy)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[18].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[18].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.z, r6.y, cb0[17].y
    r1.z = ((r6.yyyy)*(source[17].yyyy)).z;
    // 44: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 45: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: movc r1.z, r5.y, l(0), r1.z
    r1.z = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 47: mad r3.xyz, r1.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.zzzz, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 56: round_ni r5.yw, v7.xxxy
    r5.yw = (floor(v7.xxxy)).yw;
    // 57: dp2 r2.w, r5.ywyy, l(12.989800, 78.233002, 0.000000, 0.000000)
    r2.w = (dot((r5.ywyy).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).w;
    // 58: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 59: mul r2.w, r2.w, l(43758.546875)
    r2.w = ((r2.wwww)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).w;
    // 60: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 61: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 63: mad r2.w, r2.w, l(0.010000), r7.x
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(r7.xxxx)).w;
    // 64: lt r3.w, cb0[17].w, r2.w
    r3.w = (asfloat((uint4)((source[17].wwww)<(r2.wwww)) * 0xffffffffu)).w;
    // 65: lt r2.w, r2.w, cb0[17].z
    r2.w = (asfloat((uint4)((r2.wwww)<(source[17].zzzz)) * 0xffffffffu)).w;
    // 66: movc r2.w, r2.w, l(-1.000000), l(-0.000000)
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).w;
    // 67: and r4.w, r3.w, l(0x3f800000)
    r4.w = (asfloat(asuint(r3.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 68: movc r3.w, r3.w, l(0), l(1.000000)
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: add r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)+(r3.wwww)).w;
    // 70: mad r3.xyz, r4.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r4.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 71: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 72: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 73: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 74: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 75: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 76: mad r4.xyz, r1.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 77: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 78: mad r3.xyz, r2.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 79: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 80: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 81: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 82: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 83: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 84: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 85: mad r4.xyz, r1.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 86: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 87: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 88: mul r4.xyz, cb0[7].xyzx, cb0[7].wwww
    r4.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 89: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 90: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 91: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 92: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 93: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 94: mad r4.xyz, r1.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 95: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 96: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 97: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r4.xyz, -r3.xyzx, r3.wwww
    r4.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 99: mad r4.xyz, cb0[18].yyyy, r4.xyzx, r3.xyzx
    r4.xyz = ((source[18].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 100: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 101: dp3 r3.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 102: add r3.xyz, -r4.xyzx, r3.xxxx
    r3.xyz = ((-(r4.xyzx))+(r3.xxxx)).xyz;
    // 103: mad r3.xyz, cb0[18].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[18].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 104: mad r4.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 105: mad r8.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 106: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 107: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 108: mul r8.xyz, r2.xyzx, r3.xyzx
    r8.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 109: dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: mad r2.xyz, -r3.xyzx, r2.xyzx, r3.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r3.wwww)).xyz;
    // 111: mad r2.xyz, cb0[18].yyyy, r2.xyzx, r8.xyzx
    r2.xyz = ((source[18].yyyy)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 112: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: add r3.xyz, -r2.xyzx, r3.xxxx
    r3.xyz = ((-(r2.xyzx))+(r3.xxxx)).xyz;
    // 114: mad r2.xyz, cb0[18].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[18].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 115: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 116: mad r1.xyw, r1.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r2.xyxz
    r1.xyw = ((r1.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r2.xyxz))).xyw;
    // 117: mad r1.xyw, r0.xxxx, r1.xyxw, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 118: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 119: add r0.x, r1.w, r0.x
    r0.x = ((r1.wwww)+(r0.xxxx)).x;
    // 120: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 121: max r0.x, r0.x, cb0[20].z
    r0.x = (max(r0.xxxx,source[20].zzzz)).x;
    // 122: min r0.x, r0.x, cb0[20].y
    r0.x = (min(r0.xxxx,source[20].yyyy)).x;
    // 123: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 124: mad r0.x, r1.z, r2.x, r0.x
    r0.x = ((r1.zzzz)*(r2.xxxx)+(r0.xxxx)).x;
    // 125: mul_sat r3.w, r1.z, cb2[3].w
    r3.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 126: add r1.z, r0.x, l(-1.000000)
    r1.z = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 127: mad r1.z, cb0[21].x, r1.z, l(1.000000)
    r1.z = ((source[21].xxxx)*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 128: mul r2.xyz, r1.xywx, r1.zzzz
    r2.xyz = ((r1.xywx)*(r1.zzzz)).xyz;
    // 129: mul r3.x, r6.x, cb0[19].z
    r3.x = ((r6.xxxx)*(source[19].zzzz)).x;
    // 130: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 131: movc r3.x, r5.x, l(0), r3.x
    r3.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 132: add_sat r3.x, r3.x, cb0[19].w
    r3.x = (saturate((r3.xxxx)+(source[19].wwww))).x;
    // 133: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 134: mul r4.xyz, r3.yyyy, cb0[14].xyzx
    r4.xyz = ((r3.yyyy)*(source[14].xyzx)).xyz;
    // 135: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 136: mad r1.xyz, r1.zzzz, r1.xywx, -r2.xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(-(r2.xyzx))).xyz;
    // 137: mad r1.xyz, r3.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 138: mul_sat r0.x, r0.x, r3.x
    r0.x = (saturate((r0.xxxx)*(r3.xxxx))).x;
    // 139: add r2.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 140: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 141: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 142: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 143: add r2.y, -cb0[22].y, cb0[22].x
    r2.y = ((-(source[22].yyyy))+(source[22].xxxx)).y;
    // 144: mad r2.y, r7.x, r2.y, cb0[22].y
    r2.y = ((r7.xxxx)*(r2.yyyy)+(source[22].yyyy)).y;
    // 145: add r2.z, -r2.y, cb0[22].w
    r2.z = ((-(r2.yyyy))+(source[22].wwww)).z;
    // 146: mad r2.y, r7.y, r2.z, r2.y
    r2.y = ((r7.yyyy)*(r2.zzzz)+(r2.yyyy)).y;
    // 147: add r2.z, -r2.y, cb0[23].y
    r2.z = ((-(r2.yyyy))+(source[23].yyyy)).z;
    // 148: mad r2.y, r7.z, r2.z, r2.y
    r2.y = ((r7.zzzz)*(r2.zzzz)+(r2.yyyy)).y;
    // 149: add r2.z, -r2.y, cb0[23].w
    r2.z = ((-(r2.yyyy))+(source[23].wwww)).z;
    // 150: mad r2.y, r2.w, r2.z, r2.y
    r2.y = ((r2.wwww)*(r2.zzzz)+(r2.yyyy)).y;
    // 151: mul r2.y, r6.z, r2.y
    r2.y = ((r6.zzzz)*(r2.yyyy)).y;
    // 152: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 153: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 154: movc r2.y, r5.z, l(0), r2.y
    r2.y = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 155: max r2.y, r2.y, cb0[0].x
    r2.y = (max(r2.yyyy,source[0].xxxx)).y;
    // 156: min r3.z, r2.y, l(1.000000)
    r3.z = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 157: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 158: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 159: dp2 r2.w, r2.yzyy, r2.yzyy
    r2.w = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).w;
    // 160: mul r4.xy, r2.yzyy, cb0[17].xxxx
    r4.xy = ((r2.yzyy)*(source[17].xxxx)).xy;
    // 161: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 162: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 163: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 164: add r4.z, r2.y, l(0.000010)
    r4.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 165: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 166: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 167: div r2.yzw, r4.xxyz, r2.yyyy
    r2.yzw = ((r4.xxyz)/(r2.yyyy)).yzw;
    // 168: dp3 r3.x, r2.yzwy, r2.yzwy
    r3.x = (dot((r2.yzwy).xyz,(r2.yzwy).xyz).xxxx).x;
    // 169: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 170: mul r4.xyz, r2.yzwy, r3.xxxx
    r4.xyz = ((r2.yzwy)*(r3.xxxx)).xyz;
    // 171: dp3 r3.x, v5.xyzx, v5.xyzx
    r3.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 172: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 173: mul r5.xyz, r3.xxxx, v5.xyzx
    r5.xyz = ((r3.xxxx)*(v5.xyzx)).xyz;
    // 174: dp3 r3.x, r4.xyzx, r5.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 175: deriv_rtx_coarse r6.x, r3.x
    r6.x = (ddx_coarse(r3.xxxx)).x;
    // 176: deriv_rty_coarse r6.y, r3.x
    r6.y = (ddy_coarse(r3.xxxx)).y;
    // 177: dp2 r3.y, r6.xyxx, r6.xyxx
    r3.y = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 178: sqrt r3.y, r3.y
    r3.y = (sqrt(r3.yyyy)).y;
    // 179: mad r3.y, r3.y, l(0.300000), r3.z
    r3.y = ((r3.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz)).y;
    // 180: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 181: min r6.y, r3.y, l(1.000000)
    r6.y = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 182: mul r3.y, r6.y, l(5.000000)
    r3.y = ((r6.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 183: dp3 r3.z, v1.xyzx, v1.xyzx
    r3.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 184: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 185: mul r7.xyz, r3.zzzz, v1.xyzx
    r7.xyz = ((r3.zzzz)*(v1.xyzx)).xyz;
    // 186: dp3 r3.z, v0.xyzx, v0.xyzx
    r3.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 187: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 188: mul r8.xyz, r3.zzzz, v0.xyzx
    r8.xyz = ((r3.zzzz)*(v0.xyzx)).xyz;
    // 189: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 190: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 191: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 192: mul r10.xyz, r3.xxxx, r4.xyzx
    r10.xyz = ((r3.xxxx)*(r4.xyzx)).xyz;
    // 193: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 194: dp3 r11.y, r9.xyzx, r10.xyzx
    r11.y = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 195: dp3 r9.y, r9.xyzx, r4.xyzx
    r9.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 196: dp3 r11.x, r8.xyzx, r10.xyzx
    r11.x = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 197: dp3 r9.x, r8.xyzx, r4.xyzx
    r9.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 198: dp2 r8.z, r11.xyxx, cb0[26].xyxx
    r8.z = (dot((r11.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 199: mul r6.zw, cb0[26].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r6.zw = ((source[26].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 200: dp2 r8.x, r11.xyxx, r6.zwzz
    r8.x = (dot((r11.xyxx).xy,(r6.zwzz).xy).xxxx).x;
    // 201: dp2 r11.x, r9.xyxx, r6.zwzz
    r11.x = (dot((r9.xyxx).xy,(r6.zwzz).xy).xxxx).x;
    // 202: dp3 r8.y, r7.xyzx, r10.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 203: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 204: sample_l_indexable(texturecube)(float,float,float,float) r8.xyzw, r8.xyzx, t6.xyzw, s5, r3.y
    r8.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r8.xyzx).xyz, (r3.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 205: mul r7.xyz, r8.xyzx, r8.wwww
    r7.xyz = ((r8.xyzx)*(r8.wwww)).xyz;
    // 206: mul r7.xyz, r7.xyzx, cb0[25].xyzx
    r7.xyz = ((r7.xyzx)*(source[25].xyzx)).xyz;
    // 207: mul r7.xyz, r7.xyzx, cb0[26].zzzz
    r7.xyz = ((r7.xyzx)*(source[26].zzzz)).xyz;
    // 208: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[25].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[25].wwww)).xyz;
    // 209: dp3 r3.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 210: add r7.xyz, -r3.yyyy, r7.xyzx
    r7.xyz = ((-(r3.yyyy))+(r7.xyzx)).xyz;
    // 211: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.yyyy
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.yyyy)).xyz;
    // 212: dp3 r3.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 213: mad r3.z, r6.y, l(2.000000), l(2.000000)
    r3.z = ((r6.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).z;
    // 214: div r3.y, r3.y, r3.z
    r3.y = ((r3.yyyy)/(r3.zzzz)).y;
    // 215: mad r3.y, r2.x, l(5.000000), r3.y
    r3.y = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.yyyy)).y;
    // 216: add_sat r3.y, r3.w, r3.y
    r3.y = (saturate((r3.wwww)+(r3.yyyy))).y;
    // 217: mad r4.w, r3.y, l(-2.000000), l(3.000000)
    r4.w = ((r3.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 218: mul r3.y, r3.y, r3.y
    r3.y = ((r3.yyyy)*(r3.yyyy)).y;
    // 219: mul r3.y, r3.y, r4.w
    r3.y = ((r3.yyyy)*(r4.wwww)).y;
    // 220: log r3.y, r3.y
    r3.y = (log2(r3.yyyy)).y;
    // 221: mul r3.y, r3.y, l(1.500000)
    r3.y = ((r3.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 222: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 223: mul r7.xyz, r3.yyyy, r7.xyzx
    r7.xyz = ((r3.yyyy)*(r7.xyzx)).xyz;
    // 224: mov_sat r1.w, cb0[21].y
    r1.w = (saturate(source[21].yyyy)).w;
    // 225: mad r8.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r8.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 226: mul r3.y, r1.w, l(0.080000)
    r3.y = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 227: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 228: mad r8.xyz, r3.wwww, r8.xyzx, r3.yyyy
    r8.xyz = ((r3.wwww)*(r8.xyzx)+(r3.yyyy)).xyz;
    // 229: mul_sat r1.w, r8.y, l(50.000000)
    r1.w = (saturate((r8.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 230: add r3.y, -r6.y, l(1.000000)
    r3.y = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 231: max r12.xyz, r8.xyzx, r3.yyyy
    r12.xyz = (max(r8.xyzx,r3.yyyy)).xyz;
    // 232: add r12.xyz, -r8.xyzx, r12.xyzx
    r12.xyz = ((-(r8.xyzx))+(r12.xyzx)).xyz;
    // 233: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 234: add r1.w, r3.x, l(1.000000)
    r1.w = ((r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 235: mov_sat r3.x, r3.x
    r3.x = (saturate(r3.xxxx)).x;
    // 236: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 237: mul r3.x, r3.x, cb0[1].y
    r3.x = ((r3.xxxx)*(source[1].yyyy)).x;
    // 238: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 239: mad_sat r3.x, r3.x, cb0[1].w, cb0[1].z
    r3.x = (saturate((r3.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 240: mul r3.x, r3.x, cb0[24].x
    r3.x = ((r3.xxxx)*(source[24].xxxx)).x;
    // 241: add r3.y, r10.z, l(1.000000)
    r3.y = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 242: min r3.y, r3.y, l(1.000000)
    r3.y = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 243: add_sat r6.x, r1.w, -r3.y
    r6.x = (saturate((r1.wwww)+(-(r3.yyyy)))).x;
    // 244: sample_indexable(texture2d)(float,float,float,float) r6.zw, r6.xyxx, t5.zwxy, s6
    r6.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 245: mul r1.w, r6.y, r6.y
    r1.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 246: add r3.y, r0.x, r6.x
    r3.y = ((r0.xxxx)+(r6.xxxx)).y;
    // 247: log r3.y, r3.y
    r3.y = (log2(r3.yyyy)).y;
    // 248: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 249: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 250: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 251: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 252: mul r13.xyz, r6.wwww, r8.xyzx
    r13.xyz = ((r6.wwww)*(r8.xyzx)).xyz;
    // 253: mad r6.xyz, r12.xyzx, r6.zzzz, r13.xyzx
    r6.xyz = ((r12.xyzx)*(r6.zzzz)+(r13.xyzx)).xyz;
    // 254: div r3.y, l(1.000000, 1.000000, 1.000000, 1.000000), r6.w
    r3.y = r6.w != 0.f ? 1.f / r6.w : 0.f;
    // 255: add r3.y, r3.y, l(-1.000000)
    r3.y = ((r3.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 256: mad r12.xyz, r8.xyzx, r3.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r8.xyzx)*(r3.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 257: dp3 r3.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 258: mad r8.xyz, r3.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r8.xyz = ((r3.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 259: mul r13.xyz, r6.xyzx, r12.xyzx
    r13.xyz = ((r6.xyzx)*(r12.xyzx)).xyz;
    // 260: mad r6.xyz, -r6.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r6.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 261: mul r12.xyz, r7.xyzx, r13.xyzx
    r12.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 262: mad r14.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r14.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 263: mad r15.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r15.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 264: mad r16.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r16.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 265: mad r15.xyz, r0.xxxx, r15.xyzx, r16.xyzx
    r15.xyz = ((r0.xxxx)*(r15.xyzx)+(r16.xyzx)).xyz;
    // 266: mad r14.xyz, r15.xyzx, r0.xxxx, r14.xyzx
    r14.xyz = ((r15.xyzx)*(r0.xxxx)+(r14.xyzx)).xyz;
    // 267: mul r14.xyz, r0.xxxx, r14.xyzx
    r14.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 268: max r14.xyz, r0.xxxx, r14.xyzx
    r14.xyz = (max(r0.xxxx,r14.xyzx)).xyz;
    // 269: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 270: dp2 r11.z, r9.xyxx, cb0[26].xyxx
    r11.z = (dot((r9.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 271: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 272: dp4 r15.x, cb0[27].xyzw, r11.xyzw
    r15.x = (dot((source[27].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 273: dp4 r15.y, cb0[28].xyzw, r11.xyzw
    r15.y = (dot((source[28].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 274: dp4 r15.z, cb0[29].xyzw, r11.xyzw
    r15.z = (dot((source[29].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 275: mul r16.xyzw, r11.yzzx, r11.xyzz
    r16.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 276: dp4 r17.x, cb0[30].xyzw, r16.xyzw
    r17.x = (dot((source[30].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 277: dp4 r17.y, cb0[31].xyzw, r16.xyzw
    r17.y = (dot((source[31].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 278: dp4 r17.z, cb0[32].xyzw, r16.xyzw
    r17.z = (dot((source[32].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 279: add r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)+(r17.xyzx)).xyz;
    // 280: mul r0.x, r11.y, r11.y
    r0.x = ((r11.yyyy)*(r11.yyyy)).x;
    // 281: mov r9.z, r11.y
    r9.z = (r11.yyyy).z;
    // 282: mad r0.x, r11.x, r11.x, -r0.x
    r0.x = ((r11.xxxx)*(r11.xxxx)+(-(r0.xxxx))).x;
    // 283: mad r11.xyz, cb0[33].xyzx, r0.xxxx, r15.xyzx
    r11.xyz = ((source[33].xyzx)*(r0.xxxx)+(r15.xyzx)).xyz;
    // 284: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 285: mul r11.xyz, r11.xyzx, cb0[25].xyzx
    r11.xyz = ((r11.xyzx)*(source[25].xyzx)).xyz;
    // 286: mul r11.xyz, r11.xyzx, cb0[26].zzzz
    r11.xyz = ((r11.xyzx)*(source[26].zzzz)).xyz;
    // 287: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[25].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[25].wwww)).xyz;
    // 288: dp3 r0.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 289: add r11.xyz, -r0.xxxx, r11.xyzx
    r11.xyz = ((-(r0.xxxx))+(r11.xyzx)).xyz;
    // 290: mad r11.xyz, r11.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r0.xxxx
    r11.xyz = ((r11.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r0.xxxx)).xyz;
    // 291: dp3 r0.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 292: div r0.x, r0.x, r3.z
    r0.x = ((r0.xxxx)/(r3.zzzz)).x;
    // 293: mad r0.x, r2.x, l(5.000000), r0.x
    r0.x = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.xxxx)).x;
    // 294: add_sat r0.x, r3.w, r0.x
    r0.x = (saturate((r3.wwww)+(r0.xxxx))).x;
    // 295: mad r2.x, r0.x, l(-2.000000), l(3.000000)
    r2.x = ((r0.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 296: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 297: mul r0.x, r0.x, r2.x
    r0.x = ((r0.xxxx)*(r2.xxxx)).x;
    // 298: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 299: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 300: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 301: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 302: mul r15.xyz, r1.xyzx, r6.xyzx
    r15.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 303: mul r6.xyz, r6.xyzx, r11.xyzx
    r6.xyz = ((r6.xyzx)*(r11.xyzx)).xyz;
    // 304: add r0.x, -r3.w, l(1.000000)
    r0.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 305: mul r15.xyz, r0.xxxx, r15.xyzx
    r15.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 306: mul r11.xyz, r11.xyzx, r15.xyzx
    r11.xyz = ((r11.xyzx)*(r15.xyzx)).xyz;
    // 307: mul r11.xyz, r14.xyzx, r11.xyzx
    r11.xyz = ((r14.xyzx)*(r11.xyzx)).xyz;
    // 308: mad r2.x, r1.w, r8.x, r8.y
    r2.x = ((r1.wwww)*(r8.xxxx)+(r8.yyyy)).x;
    // 309: mad r2.x, r2.x, r1.w, r8.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r8.zzzz)).x;
    // 310: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 311: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 312: mad r8.xyz, r12.xyzx, r1.wwww, r11.xyzx
    r8.xyz = ((r12.xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 313: dp3 r2.x, v6.xyzx, v6.xyzx
    r2.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 314: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 315: mul r11.xyz, r2.xxxx, v6.xyzx
    r11.xyz = ((r2.xxxx)*(v6.xyzx)).xyz;
    // 316: dp3 r2.x, r11.xyzx, r4.xyzx
    r2.x = (dot((r11.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 317: dp3 r3.y, -r11.xyzx, r4.xyzx
    r3.y = (dot((-(r11.xyzx)).xyz,(r4.xyzx).xyz).xxxx).y;
    // 318: dp3 r3.z, r11.xyzx, r10.xyzx
    r3.z = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 319: mad r4.xy, r3.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r3.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 320: mad r3.yz, r3.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r3.yz = ((r3.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 321: mul r3.yz, r3.yyzy, r3.yyzy
    r3.yz = ((r3.yyzy)*(r3.yyzy)).yz;
    // 322: mad r4.zw, r2.xxxx, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r4.zw = ((r2.xxxx)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 323: mul r4.xyzw, r4.xyzw, r4.xyzw
    r4.xyzw = ((r4.xyzw)*(r4.xyzw)).xyzw;
    // 324: mul r10.xyz, r4.wwww, cb0[36].xyzx
    r10.xyz = ((r4.wwww)*(source[36].xyzx)).xyz;
    // 325: mad r10.xyz, r4.zzzz, cb0[35].xyzx, r10.xyzx
    r10.xyz = ((r4.zzzz)*(source[35].xyzx)+(r10.xyzx)).xyz;
    // 326: mul r10.xyz, r10.xyzx, cb0[37].wwww
    r10.xyz = ((r10.xyzx)*(source[37].wwww)).xyz;
    // 327: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 328: mul r10.xyz, r14.xyzx, r10.xyzx
    r10.xyz = ((r14.xyzx)*(r10.xyzx)).xyz;
    // 329: mul r10.xyz, r10.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 330: mul r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 331: mad r6.xyz, -r6.xyzx, r3.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r3.wwww)+(r6.xyzx)).xyz;
    // 332: mad r6.xyz, r8.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r6.xyzx
    r6.xyz = ((r8.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r6.xyzx)).xyz;
    // 333: mul r4.yzw, r4.yyyy, cb0[36].xxyz
    r4.yzw = ((r4.yyyy)*(source[36].xxyz)).yzw;
    // 334: mad r4.xyz, cb0[35].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[35].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 335: mul r4.xyz, r4.xyzx, cb0[37].wwww
    r4.xyz = ((r4.xyzx)*(source[37].wwww)).xyz;
    // 336: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 337: mul r4.xyz, r7.xyzx, r4.xyzx
    r4.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 338: mul r4.xyz, r4.xyzx, r13.xyzx
    r4.xyz = ((r4.xyzx)*(r13.xyzx)).xyz;
    // 339: mad r6.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r6.xyzx
    r6.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r6.xyzx)).xyz;
    // 340: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 341: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 342: dp3 r1.w, r2.yzwy, r5.xyzx
    r1.w = (dot((r2.yzwy).xyz,(r5.xyzx).xyz).xxxx).w;
    // 343: mul_sat r2.x, r1.w, cb0[18].w
    r2.x = (saturate((r1.wwww)*(source[18].wwww))).x;
    // 344: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 345: mul_sat r2.y, r5.z, cb0[18].w
    r2.y = (saturate((r5.zzzz)*(source[18].wwww))).y;
    // 346: add r2.z, -|r5.z|, l(1.000000)
    r2.z = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 347: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 348: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 349: add_sat r2.y, r2.y, -cb0[19].x
    r2.y = (saturate((r2.yyyy)+(-(source[19].xxxx)))).y;
    // 350: log r2.z, r2.y
    r2.z = (log2(r2.yyyy)).z;
    // 351: lt r2.y, r2.y, l(0.000001)
    r2.y = (asfloat((uint4)((r2.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 352: mul r2.z, r2.z, cb0[19].y
    r2.z = ((r2.zzzz)*(source[19].yyyy)).z;
    // 353: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 354: mul r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)*(r2.xxxx)).x;
    // 355: movc r2.x, r2.y, l(0), r2.x
    r2.x = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 356: add r2.y, -r7.w, r2.x
    r2.y = ((-(r7.wwww))+(r2.xxxx)).y;
    // 357: mad r2.xzw, r2.xxxx, cb0[12].xxyz, -cb0[12].xxyz
    r2.xzw = ((r2.xxxx)*(source[12].xxyz)+(-(source[12].xxyz))).xzw;
    // 358: mad r2.xzw, cb0[12].wwww, r2.xxzw, cb0[12].xxyz
    r2.xzw = ((source[12].wwww)*(r2.xxzw)+(source[12].xxyz)).xzw;
    // 359: mad r2.y, cb0[11].w, r2.y, r7.w
    r2.y = ((source[11].wwww)*(r2.yyyy)+(r7.wwww)).y;
    // 360: mad r2.xyz, r2.yyyy, cb0[11].xyzx, r2.xzwx
    r2.xyz = ((r2.yyyy)*(source[11].xyzx)+(r2.xzwx)).xyz;
    // 361: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 362: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 363: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 364: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 365: mul r4.xyz, r2.wwww, cb0[13].xyzx
    r4.xyz = ((r2.wwww)*(source[13].xyzx)).xyz;
    // 366: movc r4.xyz, r1.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 367: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 368: mad r0.yzw, cb0[18].xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((source[18].xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 369: add r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)+(source[2].xxyz)).yzw;
    // 370: mul r2.xyz, r3.zzzz, cb0[36].xyzx
    r2.xyz = ((r3.zzzz)*(source[36].xyzx)).xyz;
    // 371: mad r2.xyz, r3.yyyy, cb0[35].xyzx, r2.xyzx
    r2.xyz = ((r3.yyyy)*(source[35].xyzx)+(r2.xyzx)).xyz;
    // 372: mul r2.xyz, r2.xyzx, cb0[37].wwww
    r2.xyz = ((r2.xyzx)*(source[37].wwww)).xyz;
    // 373: mul_sat r3.yzw, cb0[16].xxyz, cb0[16].wwww
    r3.yzw = (saturate((source[16].xxyz)*(source[16].wwww))).yzw;
    // 374: mul r4.xyz, r3.yzwy, r3.xxxx
    r4.xyz = ((r3.yzwy)*(r3.xxxx)).xyz;
    // 375: mul r3.xyz, r3.yzwy, cb0[24].xxxx
    r3.xyz = ((r3.yzwy)*(source[24].xxxx)).xyz;
    // 376: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 377: mul r3.xyz, r0.xxxx, r4.xyzx
    r3.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 378: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 379: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 380: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.yzwy
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.yzwy)).xyz;
    // 381: add r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)+(r0.xyzx)).xyz;
    // 382: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 383: mad o0.xyz, r1.xyzx, cb0[37].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[37].xyzx)+(r0.xyzx)).xyz;
    // 384: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 385: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 386: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 387: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 388: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 389: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 390: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 391: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 392: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 393: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 394: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 395: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 396: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 397: ftou r0.x, cb0[34].z
    r0.x = (asfloat((uint4)(source[34].zzzz))).x;
    // 398: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 399: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 400: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 401: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 402: ret
    return output;
}

// source.character.equipment-native-181.v1 / source program 2f820cd1d12f1b47864c45bdabb83922
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase181(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[22].x=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[29]=g_SourceCharacterEnvironmentColor; source[30]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0, r22=0.0, r23=0.0, r24=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[18].xyzw
    r1.xyzw = ((r0.xyzw)*(source[18].xyzw)).xyzw;
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
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 15: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 16: mul r0.x, r0.x, l(0.125000)
    r0.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[9].y, cb0[17].y
    r2.y = ((source[9].yyyy)*(source[17].yyyy)).y;
    // 18: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 21: frc r0.z, cb0[9].x
    r0.z = (frac(source[9].xxxx)).z;
    // 22: add r0.w, -r0.z, cb0[9].x
    r0.w = ((-(r0.zzzz))+(source[9].xxxx)).w;
    // 23: mul r2.z, r0.w, l(0.125000)
    r2.z = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 24: add r0.xy, r0.xyxx, r2.zwzz
    r0.xy = ((r0.xyxx)+(r2.zwzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r0.x, r0.z, r2.w
    r0.x = ((r0.zzzz)*(r2.wwww)).x;
    // 27: add r0.y, -cb0[9].w, l(1.000000)
    r0.y = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r0.y, r0.y, cb0[22].x
    r0.y = ((r0.yyyy)*(source[22].xxxx)).y;
    // 29: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 30: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 31: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: mul r0.z, cb0[9].z, l(1.500000)
    r0.z = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 33: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 34: mad r0.y, r0.y, l(0.500000), cb0[9].z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).y;
    // 35: mul r0.yzw, r2.xxyz, r0.yyyy
    r0.yzw = ((r2.xxyz)*(r0.yyyy)).yzw;
    // 36: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 38: mad r2.xyz, cb0[22].zzzz, r2.xyzx, r1.xyzx
    r2.xyz = ((source[22].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 39: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 41: mad r2.xyz, cb0[22].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[22].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 42: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 43: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 44: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 45: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 47: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 49: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 50: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 51: mul r1.w, r6.y, cb0[20].y
    r1.w = ((r6.yyyy)*(source[20].yyyy)).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 55: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 56: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 57: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 58: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 59: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 61: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 62: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 63: add r7.xyz, -r3.xyzx, r4.xyzx
    r7.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 64: round_ni r5.yw, v7.xxxy
    r5.yw = (floor(v7.xxxy)).yw;
    // 65: dp2 r2.w, r5.ywyy, l(12.989800, 78.233002, 0.000000, 0.000000)
    r2.w = (dot((r5.ywyy).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).w;
    // 66: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 67: mul r2.w, r2.w, l(43758.546875)
    r2.w = ((r2.wwww)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).w;
    // 68: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 69: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 71: mad r2.w, r2.w, l(0.010000), r8.x
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(r8.xxxx)).w;
    // 72: lt r3.w, cb0[20].w, r2.w
    r3.w = (asfloat((uint4)((source[20].wwww)<(r2.wwww)) * 0xffffffffu)).w;
    // 73: lt r2.w, r2.w, cb0[20].z
    r2.w = (asfloat((uint4)((r2.wwww)<(source[20].zzzz)) * 0xffffffffu)).w;
    // 74: movc r2.w, r2.w, l(-1.000000), l(-0.000000)
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).w;
    // 75: and r4.w, r3.w, l(0x3f800000)
    r4.w = (asfloat(asuint(r3.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 76: movc r3.w, r3.w, l(0), l(1.000000)
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: add r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)+(r3.wwww)).w;
    // 78: mad r3.xyz, r4.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r4.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 79: add r3.w, r8.y, r4.w
    r3.w = ((r8.yyyy)+(r4.wwww)).w;
    // 80: add r3.w, r8.z, r3.w
    r3.w = ((r8.zzzz)+(r3.wwww)).w;
    // 81: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 82: mul r7.xyz, cb0[6].xyzx, cb0[6].wwww
    r7.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 83: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 84: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 85: max r9.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 86: add r9.xyz, -r7.xyzx, r9.xyzx
    r9.xyz = ((-(r7.xyzx))+(r9.xyzx)).xyz;
    // 87: mad r7.xyz, r1.wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 88: add r9.xyz, -r3.xyzx, r7.xyzx
    r9.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 89: mad r3.xyz, r2.wwww, r9.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 90: mul r9.xyz, cb0[7].xyzx, cb0[7].wwww
    r9.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 91: max r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 92: max r9.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 93: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 94: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 95: add r10.xyz, -r9.xyzx, r10.xyzx
    r10.xyz = ((-(r9.xyzx))+(r10.xyzx)).xyz;
    // 96: mad r9.xyz, r1.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r1.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 97: add r10.xyz, -r3.xyzx, r9.xyzx
    r10.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 98: mad r3.xyz, r8.yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((r8.yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 99: mul r10.xyz, cb0[8].xyzx, cb0[8].wwww
    r10.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 100: max r11.xyz, r10.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r11.xyz = (max(r10.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 101: max r10.xyz, r10.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 102: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 103: min r11.xyz, r11.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r11.xyz = (min(r11.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 104: add r11.xyz, -r10.xyzx, r11.xyzx
    r11.xyz = ((-(r10.xyzx))+(r11.xyzx)).xyz;
    // 105: mad r10.xyz, r1.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r1.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 106: add r11.xyz, -r3.xyzx, r10.xyzx
    r11.xyz = ((-(r3.xyzx))+(r10.xyzx)).xyz;
    // 107: mad r3.xyz, r8.zzzz, r11.xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 108: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r11.xyz, -r3.xyzx, r4.wwww
    r11.xyz = ((-(r3.xyzx))+(r4.wwww)).xyz;
    // 110: mad r11.xyz, cb0[22].zzzz, r11.xyzx, r3.xyzx
    r11.xyz = ((source[22].zzzz)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 111: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 112: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: add r3.xyz, -r11.xyzx, r3.xxxx
    r3.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 114: mad r3.xyz, cb0[22].wwww, r3.xyzx, r11.xyzx
    r3.xyz = ((source[22].wwww)*(r3.xyzx)+(r11.xyzx)).xyz;
    // 115: mad r11.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mad r12.xyz, cb0[12].wwww, cb0[12].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[12].wwww)*(source[12].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 117: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 118: mul r3.xyz, r3.xyzx, r11.xyzx
    r3.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 119: mul r12.xyz, r2.xyzx, r3.xyzx
    r12.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 120: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 121: mad r2.xyz, -r3.xyzx, r2.xyzx, r4.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r4.wwww)).xyz;
    // 122: mad r2.xyz, cb0[22].zzzz, r2.xyzx, r12.xyzx
    r2.xyz = ((source[22].zzzz)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 123: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 124: add r3.xyz, -r2.xyzx, r3.xxxx
    r3.xyz = ((-(r2.xyzx))+(r3.xxxx)).xyz;
    // 125: mad r2.xyz, cb0[22].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[22].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 126: mul r2.xyz, r11.xyzx, r2.xyzx
    r2.xyz = ((r11.xyzx)*(r2.xyzx)).xyz;
    // 127: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r2.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r2.xxyz))).yzw;
    // 128: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 129: add r0.w, r0.y, r0.x
    r0.w = ((r0.yyyy)+(r0.xxxx)).w;
    // 130: add r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)+(r0.wwww)).w;
    // 131: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 132: max r0.w, r0.w, cb0[24].z
    r0.w = (max(r0.wwww,source[24].zzzz)).w;
    // 133: min r0.w, r0.w, cb0[24].y
    r0.w = (min(r0.wwww,source[24].yyyy)).w;
    // 134: add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: mad r0.w, r1.w, r2.x, r0.w
    r0.w = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).w;
    // 136: mul_sat r12.w, r1.w, cb2[3].w
    r12.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 137: add r1.w, r0.w, l(-1.000000)
    r1.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 138: mad r1.w, cb0[25].x, r1.w, l(1.000000)
    r1.w = ((source[25].xxxx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r2.xyz, r0.xyzx, r1.wwww
    r2.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 140: mul r3.x, r6.x, cb0[23].w
    r3.x = ((r6.xxxx)*(source[23].wwww)).x;
    // 141: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 142: movc r3.x, r5.x, l(0), r3.x
    r3.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 143: add_sat r3.x, r3.x, cb0[24].x
    r3.x = (saturate((r3.xxxx)+(source[24].xxxx))).x;
    // 144: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 145: mul r5.xyw, r3.yyyy, cb0[16].xyxz
    r5.xyw = ((r3.yyyy)*(source[16].xyxz)).xyw;
    // 146: mul r2.xyz, r2.xyzx, r5.xywx
    r2.xyz = ((r2.xyzx)*(r5.xywx)).xyz;
    // 147: mad r0.xyz, r1.wwww, r0.xyzx, -r2.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 148: mad r0.xyz, r3.xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 149: mul_sat r0.w, r0.w, r3.x
    r0.w = (saturate((r0.wwww)*(r3.xxxx))).w;
    // 150: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 151: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 152: mad_sat r13.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r13.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 153: dp3 r0.x, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 154: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 155: mad r0.yz, r0.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 156: dp2 r1.w, r0.yzyy, r0.yzyy
    r1.w = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).w;
    // 157: mul r2.xy, r0.yzyy, cb0[20].xxxx
    r2.xy = ((r0.yzyy)*(source[20].xxxx)).xy;
    // 158: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 159: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 160: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 161: add r2.z, r0.y, l(0.000010)
    r2.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 162: dp3 r0.y, r2.xyzx, r2.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 163: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 164: div r2.xyz, r2.xyzx, r0.yyyy
    r2.xyz = ((r2.xyzx)/(r0.yyyy)).xyz;
    // 165: dp3 r0.y, r2.xyzx, r2.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 166: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 167: mul r3.xyz, r0.yyyy, r2.xyzx
    r3.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 168: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 169: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 170: mul r5.xyw, r0.yyyy, v1.xyxz
    r5.xyw = ((r0.yyyy)*(v1.xyxz)).xyw;
    // 171: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 172: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 173: mul r6.xyw, r0.yyyy, v0.xyxz
    r6.xyw = ((r0.yyyy)*(v0.xyxz)).xyw;
    // 174: mul r14.xyz, r5.wxyw, r6.ywxy
    r14.xyz = ((r5.wxyw)*(r6.ywxy)).xyz;
    // 175: mad r14.xyz, r5.ywxy, r6.wxyw, -r14.xyzx
    r14.xyz = ((r5.ywxy)*(r6.wxyw)+(-(r14.xyzx))).xyz;
    // 176: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 177: dp3 r15.y, r14.xyzx, r3.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 178: dp3 r15.x, r6.xywx, r3.xyzx
    r15.x = (dot((r6.xywx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 179: dp2 r16.z, r15.xyxx, cb0[30].xyxx
    r16.z = (dot((r15.xyxx).xy,(source[30].xyxx).xy).xxxx).z;
    // 180: mul r0.yz, cb0[30].yyxy, l(0.000000, 1.000000, -1.000000, 0.000000)
    r0.yz = ((source[30].yyxy)*(float4(0.000000,1.000000,-1.000000,0.000000))).yz;
    // 181: dp2 r16.x, r15.xyxx, r0.yzyy
    r16.x = (dot((r15.xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 182: dp3 r16.y, r5.xywx, r3.xyzx
    r16.y = (dot((r5.xywx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 183: mov r16.w, l(1.000000)
    r16.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 184: dp4 r17.x, cb0[31].xyzw, r16.xyzw
    r17.x = (dot((source[31].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 185: dp4 r17.y, cb0[32].xyzw, r16.xyzw
    r17.y = (dot((source[32].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 186: dp4 r17.z, cb0[33].xyzw, r16.xyzw
    r17.z = (dot((source[33].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 187: mul r18.xyzw, r16.yzzx, r16.xyzz
    r18.xyzw = ((r16.yzzx)*(r16.xyzz)).xyzw;
    // 188: dp4 r19.x, cb0[34].xyzw, r18.xyzw
    r19.x = (dot((source[34].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).x;
    // 189: dp4 r19.y, cb0[35].xyzw, r18.xyzw
    r19.y = (dot((source[35].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).y;
    // 190: dp4 r19.z, cb0[36].xyzw, r18.xyzw
    r19.z = (dot((source[36].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).z;
    // 191: add r17.xyz, r17.xyzx, r19.xyzx
    r17.xyz = ((r17.xyzx)+(r19.xyzx)).xyz;
    // 192: mul r1.w, r16.y, r16.y
    r1.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 193: mov r15.z, r16.y
    r15.z = (r16.yyyy).z;
    // 194: mad r1.w, r16.x, r16.x, -r1.w
    r1.w = ((r16.xxxx)*(r16.xxxx)+(-(r1.wwww))).w;
    // 195: mad r16.xyz, cb0[37].xyzx, r1.wwww, r17.xyzx
    r16.xyz = ((source[37].xyzx)*(r1.wwww)+(r17.xyzx)).xyz;
    // 196: max r16.xyz, r16.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r16.xyz = (max(r16.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 197: mul r16.xyz, r16.xyzx, cb0[29].xyzx
    r16.xyz = ((r16.xyzx)*(source[29].xyzx)).xyz;
    // 198: mul r16.xyz, r16.xyzx, cb0[30].zzzz
    r16.xyz = ((r16.xyzx)*(source[30].zzzz)).xyz;
    // 199: mad r16.xyz, r16.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[29].wwww
    r16.xyz = ((r16.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[29].wwww)).xyz;
    // 200: dp3 r1.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 201: add r16.xyz, -r1.wwww, r16.xyzx
    r16.xyz = ((-(r1.wwww))+(r16.xyzx)).xyz;
    // 202: mad r16.xyz, r16.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r16.xyz = ((r16.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 203: dp3 r1.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 204: add r4.w, -cb0[26].y, cb0[26].x
    r4.w = ((-(source[26].yyyy))+(source[26].xxxx)).w;
    // 205: mad r4.w, r8.x, r4.w, cb0[26].y
    r4.w = ((r8.xxxx)*(r4.wwww)+(source[26].yyyy)).w;
    // 206: add r7.w, -r4.w, cb0[26].w
    r7.w = ((-(r4.wwww))+(source[26].wwww)).w;
    // 207: mad r4.w, r8.y, r7.w, r4.w
    r4.w = ((r8.yyyy)*(r7.wwww)+(r4.wwww)).w;
    // 208: add r7.w, -r4.w, cb0[27].y
    r7.w = ((-(r4.wwww))+(source[27].yyyy)).w;
    // 209: mad r4.w, r8.z, r7.w, r4.w
    r4.w = ((r8.zzzz)*(r7.wwww)+(r4.wwww)).w;
    // 210: add r7.w, -r4.w, cb0[27].w
    r7.w = ((-(r4.wwww))+(source[27].wwww)).w;
    // 211: mad r4.w, r2.w, r7.w, r4.w
    r4.w = ((r2.wwww)*(r7.wwww)+(r4.wwww)).w;
    // 212: mul r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)*(r4.wwww)).w;
    // 213: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 214: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 215: movc r4.w, r5.z, l(0), r4.w
    r4.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 216: max r4.w, r4.w, cb0[1].x
    r4.w = (max(r4.wwww,source[1].xxxx)).w;
    // 217: min r12.z, r4.w, l(1.000000)
    r12.z = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 218: dp3 r4.w, v5.xyzx, v5.xyzx
    r4.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 219: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 220: mul r17.xyz, r4.wwww, v5.xyzx
    r17.xyz = ((r4.wwww)*(v5.xyzx)).xyz;
    // 221: dp3 r4.w, r3.xyzx, r17.xyzx
    r4.w = (dot((r3.xyzx).xyz,(r17.xyzx).xyz).xxxx).w;
    // 222: deriv_rtx_coarse r12.x, r4.w
    r12.x = (ddx_coarse(r4.wwww)).x;
    // 223: deriv_rty_coarse r12.y, r4.w
    r12.y = (ddy_coarse(r4.wwww)).y;
    // 224: dp2 r5.z, r12.xyxx, r12.xyxx
    r5.z = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).z;
    // 225: sqrt r5.z, r5.z
    r5.z = (sqrt(r5.zzzz)).z;
    // 226: mad r5.z, r5.z, l(0.300000), r12.z
    r5.z = ((r5.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r12.zzzz)).z;
    // 227: mov o2.zw, r12.zzzw
    output.targets[2].zw = (r12.zzzw).zw;
    // 228: min r12.y, r5.z, l(1.000000)
    r12.y = (min(r5.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 229: mad r5.z, r12.y, l(2.000000), l(2.000000)
    r5.z = ((r12.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).z;
    // 230: div r1.w, r1.w, r5.z
    r1.w = ((r1.wwww)/(r5.zzzz)).w;
    // 231: mad r1.w, r0.x, l(5.000000), r1.w
    r1.w = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 232: add_sat r1.w, r12.w, r1.w
    r1.w = (saturate((r12.wwww)+(r1.wwww))).w;
    // 233: mad r6.z, r1.w, l(-2.000000), l(3.000000)
    r6.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 234: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 235: mul r1.w, r1.w, r6.z
    r1.w = ((r1.wwww)*(r6.zzzz)).w;
    // 236: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 237: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 238: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 239: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 240: mov_sat r13.w, cb0[25].y
    r13.w = (saturate(source[25].yyyy)).w;
    // 241: mad r18.xyz, -r13.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r13.xyzx
    r18.xyz = ((-(r13.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r13.xyzx)).xyz;
    // 242: mul r1.w, r13.w, l(0.080000)
    r1.w = ((r13.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 243: mov o3.xyzw, r13.xyzw
    output.targets[3].xyzw = (r13.xyzw).xyzw;
    // 244: mad r18.xyz, r12.wwww, r18.xyzx, r1.wwww
    r18.xyz = ((r12.wwww)*(r18.xyzx)+(r1.wwww)).xyz;
    // 245: mul_sat r1.w, r18.y, l(50.000000)
    r1.w = (saturate((r18.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 246: add r6.z, -r12.y, l(1.000000)
    r6.z = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 247: max r19.xyz, r18.xyzx, r6.zzzz
    r19.xyz = (max(r18.xyzx,r6.zzzz)).xyz;
    // 248: add r19.xyz, -r18.xyzx, r19.xyzx
    r19.xyz = ((-(r18.xyzx))+(r19.xyzx)).xyz;
    // 249: mul r19.xyz, r1.wwww, r19.xyzx
    r19.xyz = ((r1.wwww)*(r19.xyzx)).xyz;
    // 250: mul r20.xyz, r3.xyzx, r4.wwww
    r20.xyz = ((r3.xyzx)*(r4.wwww)).xyz;
    // 251: mad r20.xyz, r20.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r17.xyzx
    r20.xyz = ((r20.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r17.xyzx))).xyz;
    // 252: add r1.w, r20.z, l(1.000000)
    r1.w = ((r20.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 253: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 254: add r6.z, r4.w, l(1.000000)
    r6.z = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 255: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 256: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 257: mul r4.w, r4.w, cb0[2].y
    r4.w = ((r4.wwww)*(source[2].yyyy)).w;
    // 258: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 259: mad_sat r4.w, r4.w, cb0[2].w, cb0[2].z
    r4.w = (saturate((r4.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 260: mul r4.w, r4.w, cb0[28].x
    r4.w = ((r4.wwww)*(source[28].xxxx)).w;
    // 261: add_sat r12.x, -r1.w, r6.z
    r12.x = (saturate((-(r1.wwww))+(r6.zzzz))).x;
    // 262: sample_indexable(texture2d)(float,float,float,float) r21.xy, r12.xyxx, t7.xyzw, s8
    r21.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 263: add r1.w, r0.w, r12.x
    r1.w = ((r0.wwww)+(r12.xxxx)).w;
    // 264: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 265: mul r22.xyz, r18.xyzx, r21.yyyy
    r22.xyz = ((r18.xyzx)*(r21.yyyy)).xyz;
    // 266: mad r19.xyz, r19.xyzx, r21.xxxx, r22.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xxxx)+(r22.xyzx)).xyz;
    // 267: div r6.z, l(1.000000, 1.000000, 1.000000, 1.000000), r21.y
    r6.z = r21.y != 0.f ? 1.f / r21.y : 0.f;
    // 268: add r6.z, r6.z, l(-1.000000)
    r6.z = ((r6.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 269: mad r21.xyz, r18.xyzx, r6.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r21.xyz = ((r18.xyzx)*(r6.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 270: dp3 r6.z, r18.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.z = (dot((r18.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 271: mad r18.xyz, r6.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r18.xyz = ((r6.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 272: mad r22.xyz, -r19.xyzx, r21.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r22.xyz = ((-(r19.xyzx))*(r21.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 273: mul r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xyzx)).xyz;
    // 274: mul r21.xyz, r13.xyzx, r22.xyzx
    r21.xyz = ((r13.xyzx)*(r22.xyzx)).xyz;
    // 275: mul r22.xyz, r16.xyzx, r22.xyzx
    r22.xyz = ((r16.xyzx)*(r22.xyzx)).xyz;
    // 276: add r6.z, -r12.w, l(1.000000)
    r6.z = ((-(r12.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 277: mul r21.xyz, r6.zzzz, r21.xyzx
    r21.xyz = ((r6.zzzz)*(r21.xyzx)).xyz;
    // 278: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 279: mad r21.xyz, r13.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r21.xyz = ((r13.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 280: mad r23.xyz, r13.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r23.xyz = ((r13.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 281: mad r24.xyz, r13.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r24.xyz = ((r13.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 282: mad r23.xyz, r0.wwww, r23.xyzx, r24.xyzx
    r23.xyz = ((r0.wwww)*(r23.xyzx)+(r24.xyzx)).xyz;
    // 283: mad r21.xyz, r23.xyzx, r0.wwww, r21.xyzx
    r21.xyz = ((r23.xyzx)*(r0.wwww)+(r21.xyzx)).xyz;
    // 284: mul r21.xyz, r0.wwww, r21.xyzx
    r21.xyz = ((r0.wwww)*(r21.xyzx)).xyz;
    // 285: max r21.xyz, r0.wwww, r21.xyzx
    r21.xyz = (max(r0.wwww,r21.xyzx)).xyz;
    // 286: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 287: dp3 r14.y, r14.xyzx, r20.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 288: dp3 r14.x, r6.xywx, r20.xyzx
    r14.x = (dot((r6.xywx).xyz,(r20.xyzx).xyz).xxxx).x;
    // 289: dp2 r23.x, r14.xyxx, r0.yzyy
    r23.x = (dot((r14.xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 290: dp2 r23.z, r14.xyxx, cb0[30].xyxx
    r23.z = (dot((r14.xyxx).xy,(source[30].xyxx).xy).xxxx).z;
    // 291: mul r0.y, r12.y, l(5.000000)
    r0.y = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 292: mul r0.z, r12.y, r12.y
    r0.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 293: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 294: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 295: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 296: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 297: add_sat r0.z, r0.z, l(-1.000000)
    r0.z = (saturate((r0.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).z;
    // 298: dp3 r23.y, r5.xywx, r20.xyzx
    r23.y = (dot((r5.xywx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 299: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r23.xyzx, t8.xyzw, s7, r0.y
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r23.xyzx).xyz, (r0.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 300: mul r5.xyw, r14.xyxz, r14.wwww
    r5.xyw = ((r14.xyxz)*(r14.wwww)).xyw;
    // 301: mul r5.xyw, r5.xyxw, cb0[29].xyxz
    r5.xyw = ((r5.xyxw)*(source[29].xyxz)).xyw;
    // 302: mul r5.xyw, r5.xyxw, cb0[30].zzzz
    r5.xyw = ((r5.xyxw)*(source[30].zzzz)).xyw;
    // 303: mad r5.xyw, r5.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[29].wwww
    r5.xyw = ((r5.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[29].wwww)).xyw;
    // 304: dp3 r0.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 305: add r5.xyw, -r0.yyyy, r5.xyxw
    r5.xyw = ((-(r0.yyyy))+(r5.xyxw)).xyw;
    // 306: mad r5.xyw, r5.xyxw, l(0.800000, 0.800000, 0.000000, 0.800000), r0.yyyy
    r5.xyw = ((r5.xyxw)*(float4(0.800000,0.800000,0.000000,0.800000))+(r0.yyyy)).xyw;
    // 307: dp3 r0.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 308: div r0.y, r0.y, r5.z
    r0.y = ((r0.yyyy)/(r5.zzzz)).y;
    // 309: mad r0.x, r0.x, l(5.000000), r0.y
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.yyyy)).x;
    // 310: add_sat r0.x, r12.w, r0.x
    r0.x = (saturate((r12.wwww)+(r0.xxxx))).x;
    // 311: mad r0.y, r0.x, l(-2.000000), l(3.000000)
    r0.y = ((r0.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 312: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 313: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 314: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 315: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 316: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 317: mul r0.xyw, r0.xxxx, r5.xyxw
    r0.xyw = ((r0.xxxx)*(r5.xyxw)).xyw;
    // 318: mul r5.xyz, r0.xywx, r19.xyzx
    r5.xyz = ((r0.xywx)*(r19.xyzx)).xyz;
    // 319: mad r1.w, r0.z, r18.x, r18.y
    r1.w = ((r0.zzzz)*(r18.xxxx)+(r18.yyyy)).w;
    // 320: mad r1.w, r1.w, r0.z, r18.z
    r1.w = ((r1.wwww)*(r0.zzzz)+(r18.zzzz)).w;
    // 321: mul r1.w, r0.z, r1.w
    r1.w = ((r0.zzzz)*(r1.wwww)).w;
    // 322: max r0.z, r0.z, r1.w
    r0.z = (max(r0.zzzz,r1.wwww)).z;
    // 323: mad r5.xyz, r5.xyzx, r0.zzzz, r16.xyzx
    r5.xyz = ((r5.xyzx)*(r0.zzzz)+(r16.xyzx)).xyz;
    // 324: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 325: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 326: mul r6.xyw, r1.wwww, v6.xyxz
    r6.xyw = ((r1.wwww)*(v6.xyxz)).xyw;
    // 327: dp3 r1.w, r6.xywx, r3.xyzx
    r1.w = (dot((r6.xywx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 328: dp3 r3.x, -r6.xywx, r3.xyzx
    r3.x = (dot((-(r6.xywx)).xyz,(r3.xyzx).xyz).xxxx).x;
    // 329: dp3 r3.y, r6.xywx, r20.xyzx
    r3.y = (dot((r6.xywx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 330: mad r3.yz, r3.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r3.yz = ((r3.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 331: mul r3.yz, r3.yyzy, r3.yyzy
    r3.yz = ((r3.yyzy)*(r3.yyzy)).yz;
    // 332: mad r6.xy, r3.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 333: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 334: mad r12.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 335: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 336: mul r14.xyz, r12.yyyy, cb0[40].xyzx
    r14.xyz = ((r12.yyyy)*(source[40].xyzx)).xyz;
    // 337: mad r12.xyz, r12.xxxx, cb0[39].xyzx, r14.xyzx
    r12.xyz = ((r12.xxxx)*(source[39].xyzx)+(r14.xyzx)).xyz;
    // 338: mul r12.xyz, r12.xyzx, cb0[41].wwww
    r12.xyz = ((r12.xyzx)*(source[41].wwww)).xyz;
    // 339: mul r12.xyz, r13.xyzx, r12.xyzx
    r12.xyz = ((r13.xyzx)*(r12.xyzx)).xyz;
    // 340: mul r12.xyz, r21.xyzx, r12.xyzx
    r12.xyz = ((r21.xyzx)*(r12.xyzx)).xyz;
    // 341: mul r12.xyz, r12.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r12.xyz = ((r12.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 342: mul r12.xyz, r22.xyzx, r12.xyzx
    r12.xyz = ((r22.xyzx)*(r12.xyzx)).xyz;
    // 343: mad r12.xyz, -r12.xyzx, r12.wwww, r12.xyzx
    r12.xyz = ((-(r12.xyzx))*(r12.wwww)+(r12.xyzx)).xyz;
    // 344: mad r5.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r12.xyzx
    r5.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r12.xyzx)).xyz;
    // 345: mul r12.xyz, r3.zzzz, cb0[40].xyzx
    r12.xyz = ((r3.zzzz)*(source[40].xyzx)).xyz;
    // 346: mad r3.xyz, cb0[39].xyzx, r3.yyyy, r12.xyzx
    r3.xyz = ((source[39].xyzx)*(r3.yyyy)+(r12.xyzx)).xyz;
    // 347: mul r3.xyz, r3.xyzx, cb0[41].wwww
    r3.xyz = ((r3.xyzx)*(source[41].wwww)).xyz;
    // 348: mul r3.xyz, r0.zzzz, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)).xyz;
    // 349: mul r0.xyz, r0.xywx, r3.xyzx
    r0.xyz = ((r0.xywx)*(r3.xyzx)).xyz;
    // 350: mul r0.xyz, r0.xyzx, r19.xyzx
    r0.xyz = ((r0.xyzx)*(r19.xyzx)).xyz;
    // 351: mad r3.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r5.xyzx
    r3.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r5.xyzx)).xyz;
    // 352: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 353: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 354: mad r0.xyz, -r8.xxxx, r4.xyzx, r7.xyzx
    r0.xyz = ((-(r8.xxxx))*(r4.xyzx)+(r7.xyzx)).xyz;
    // 355: mul r4.xyz, r4.xyzx, r8.xxxx
    r4.xyz = ((r4.xyzx)*(r8.xxxx)).xyz;
    // 356: mad r0.xyz, r2.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 357: add r4.xyz, -r0.xyzx, r9.xyzx
    r4.xyz = ((-(r0.xyzx))+(r9.xyzx)).xyz;
    // 358: mad r0.xyz, r8.yyyy, r4.xyzx, r0.xyzx
    r0.xyz = ((r8.yyyy)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 359: add r4.xyz, -r0.xyzx, r10.xyzx
    r4.xyz = ((-(r0.xyzx))+(r10.xyzx)).xyz;
    // 360: mad r0.xyz, r8.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r8.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 361: add r4.xyz, -r0.xyzx, r7.xyzx
    r4.xyz = ((-(r0.xyzx))+(r7.xyzx)).xyz;
    // 362: mad r0.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 363: add r0.xyz, r0.xyzx, -cb0[10].xyzx
    r0.xyz = ((r0.xyzx)+(-(source[10].xyzx))).xyz;
    // 364: mad r0.xyz, r3.wwww, r0.xyzx, cb0[10].xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(source[10].xyzx)).xyz;
    // 365: mul r0.xyz, r0.xyzx, cb0[21].yyyy
    r0.xyz = ((r0.xyzx)*(source[21].yyyy)).xyz;
    // 366: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 367: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 368: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 369: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 370: add r1.w, -r0.w, l(1000.000000)
    r1.w = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 371: mad r0.w, cb0[22].y, r1.w, r0.w
    r0.w = ((source[22].yyyy)*(r1.wwww)+(r0.wwww)).w;
    // 372: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 373: mad r0.w, cb0[21].w, cb0[22].x, r0.w
    r0.w = ((source[21].wwww)*(source[22].xxxx)+(r0.wwww)).w;
    // 374: mul r1.w, r0.w, l(3.524534)
    r1.w = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 375: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 376: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 377: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 378: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 379: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 380: mad r0.w, r0.w, l(0.500000), cb0[21].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[21].zzzz)).w;
    // 381: mul r4.xyz, r0.xyzx, r0.wwww
    r4.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 382: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 383: mad r0.xyz, -r0.wwww, r0.xyzx, r1.wwww
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r1.wwww)).xyz;
    // 384: mad r0.xyz, cb0[22].zzzz, r0.xyzx, r4.xyzx
    r0.xyz = ((source[22].zzzz)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 385: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 386: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 387: mad r0.xyz, cb0[22].wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((source[22].wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 388: dp3 r0.w, r2.xyzx, r17.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r17.xyzx).xyz).xxxx).w;
    // 389: mul_sat r1.w, r0.w, cb0[23].x
    r1.w = (saturate((r0.wwww)*(source[23].xxxx))).w;
    // 390: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 391: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 392: mul_sat r2.x, r17.z, cb0[23].x
    r2.x = (saturate((r17.zzzz)*(source[23].xxxx))).x;
    // 393: add r2.y, -|r17.z|, l(1.000000)
    r2.y = ((-(abs(r17.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 394: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 395: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 396: add_sat r2.x, r2.x, -cb0[23].y
    r2.x = (saturate((r2.xxxx)+(-(source[23].yyyy)))).x;
    // 397: log r2.y, r2.x
    r2.y = (log2(r2.xxxx)).y;
    // 398: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 399: mul r2.y, r2.y, cb0[23].z
    r2.y = ((r2.yyyy)*(source[23].zzzz)).y;
    // 400: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 401: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 402: movc r1.w, r2.x, l(0), r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 403: add r2.x, -r8.w, r1.w
    r2.x = ((-(r8.wwww))+(r1.wwww)).x;
    // 404: mad r2.yzw, r1.wwww, cb0[14].xxyz, -cb0[14].xxyz
    r2.yzw = ((r1.wwww)*(source[14].xxyz)+(-(source[14].xxyz))).yzw;
    // 405: mad r2.yzw, cb0[14].wwww, r2.yyzw, cb0[14].xxyz
    r2.yzw = ((source[14].wwww)*(r2.yyzw)+(source[14].xxyz)).yzw;
    // 406: mad r1.w, cb0[13].w, r2.x, r8.w
    r1.w = ((source[13].wwww)*(r2.xxxx)+(r8.wwww)).w;
    // 407: mad r2.xyz, r1.wwww, cb0[13].xyzx, r2.yzwy
    r2.xyz = ((r1.wwww)*(source[13].xyzx)+(r2.yzwy)).xyz;
    // 408: mad r0.xyz, r0.xyzx, r11.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 409: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 410: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 411: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 412: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 413: mul r2.xyz, r1.wwww, cb0[15].xyzx
    r2.xyz = ((r1.wwww)*(source[15].xyzx)).xyz;
    // 414: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 415: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 416: mad r0.xyz, cb0[21].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[21].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 417: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 418: mul r1.xyz, r6.yyyy, cb0[40].xyzx
    r1.xyz = ((r6.yyyy)*(source[40].xyzx)).xyz;
    // 419: mad r1.xyz, r6.xxxx, cb0[39].xyzx, r1.xyzx
    r1.xyz = ((r6.xxxx)*(source[39].xyzx)+(r1.xyzx)).xyz;
    // 420: mul r1.xyz, r1.xyzx, cb0[41].wwww
    r1.xyz = ((r1.xyzx)*(source[41].wwww)).xyz;
    // 421: mul_sat r2.xyz, cb0[19].xyzx, cb0[19].wwww
    r2.xyz = (saturate((source[19].xyzx)*(source[19].wwww))).xyz;
    // 422: mul r4.xyz, r2.xyzx, r4.wwww
    r4.xyz = ((r2.xyzx)*(r4.wwww)).xyz;
    // 423: mul r2.xyz, r2.xyzx, cb0[28].xxxx
    r2.xyz = ((r2.xyzx)*(source[28].xxxx)).xyz;
    // 424: dp3_sat o5.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 425: mul r2.xyz, r6.zzzz, r4.xyzx
    r2.xyz = ((r6.zzzz)*(r4.xyzx)).xyz;
    // 426: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 427: mul r1.xyz, r13.xyzx, r1.xyzx
    r1.xyz = ((r13.xyzx)*(r1.xyzx)).xyz;
    // 428: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 429: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 430: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 431: mad o0.xyz, r13.xyzx, cb0[41].xyzx, r0.xyzx
    output.targets[0].xyz = ((r13.xyzx)*(source[41].xyzx)+(r0.xyzx)).xyz;
    // 432: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 433: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 434: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 435: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 436: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 437: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 438: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 439: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 440: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 441: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 442: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 443: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 444: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 445: ftou r0.x, cb0[38].z
    r0.x = (asfloat((uint4)(source[38].zzzz))).x;
    // 446: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 447: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 448: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 449: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 450: ret
    return output;
}

// source.character.equipment-native-182.v1 / source program 597084aa3299934b97fadd46624cad6e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase182(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].y=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].x=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].yzwy
    r1.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 5: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r3.yzw, r3.xxyz, r0.wwww
    r3.yzw = ((r3.xxyz)/(r0.wwww)).yzw;
    // 18: dp3 r0.w, r3.yzwy, r3.yzwy
    r0.w = (dot((r3.yzwy).xyz,(r3.yzwy).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r4.xyz, r0.wwww, r3.yzwy
    r4.xyz = ((r0.wwww)*(r3.yzwy)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r0.w, r5.w, cb0[1].x
    r0.w = ((r5.wwww)*(source[1].xxxx)).w;
    // 23: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 24: mul r6.xyz, r6.xyzx, cb0[14].xxxx
    r6.xyz = ((r6.xyzx)*(source[14].xxxx)).xyz;
    // 25: mad r1.w, cb0[14].y, l(-3.500000), l(5.000000)
    r1.w = ((source[14].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 26: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 27: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: add r4.w, -r2.w, v4.z
    r4.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 29: mad r2.w, cb0[15].y, r4.w, r2.w
    r2.w = ((source[15].yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 30: mul r4.w, r2.w, cb0[15].z
    r4.w = ((r2.wwww)*(source[15].zzzz)).w;
    // 31: mad r2.w, r4.w, l(0.750000), r2.w
    r2.w = ((r4.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 32: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 33: mad_sat r2.w, cb0[16].x, r2.w, r2.w
    r2.w = (saturate((source[16].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 34: mad r3.x, r3.x, r5.x, l(0.200000)
    r3.x = ((r3.xxxx)*(r5.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 35: add r4.w, -r5.y, l(1.000000)
    r4.w = ((-(r5.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: add r4.w, -r3.x, r4.w
    r4.w = ((-(r3.xxxx))+(r4.wwww)).w;
    // 37: mad r6.w, cb0[16].z, r4.w, r3.x
    r6.w = ((source[16].zzzz)*(r4.wwww)+(r3.xxxx)).w;
    // 38: mul r7.x, cb0[15].w, l(0.700000)
    r7.x = ((source[15].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 39: add r6.w, -r2.w, r6.w
    r6.w = ((-(r2.wwww))+(r6.wwww)).w;
    // 40: mad r6.w, r7.x, r6.w, r2.w
    r6.w = ((r7.xxxx)*(r6.wwww)+(r2.wwww)).w;
    // 41: div r6.w, r6.w, cb0[16].y
    r6.w = ((r6.wwww)/(source[16].yyyy)).w;
    // 42: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r6.w, r1.w, r6.w
    r6.w = ((r1.wwww)*(r6.wwww)).w;
    // 44: mul r6.w, r6.w, l(4.000000)
    r6.w = ((r6.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 45: add r7.y, v4.w, l(0.500000)
    r7.y = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 46: round_ni r7.y, r7.y
    r7.y = (floor(r7.yyyy)).y;
    // 47: mul_sat r6.w, r6.w, r7.y
    r6.w = (saturate((r6.wwww)*(r7.yyyy))).w;
    // 48: mad r3.x, cb0[17].x, r4.w, r3.x
    r3.x = ((source[17].xxxx)*(r4.wwww)+(r3.xxxx)).x;
    // 49: add r3.x, -r2.w, r3.x
    r3.x = ((-(r2.wwww))+(r3.xxxx)).x;
    // 50: mad r2.w, r7.x, r3.x, r2.w
    r2.w = ((r7.xxxx)*(r3.xxxx)+(r2.wwww)).w;
    // 51: div r2.w, r2.w, cb0[16].w
    r2.w = ((r2.wwww)/(source[16].wwww)).w;
    // 52: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 54: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 55: add r2.w, -r7.y, l(1.000000)
    r2.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 57: add r1.w, r1.w, r6.w
    r1.w = ((r1.wwww)+(r6.wwww)).w;
    // 58: add r1.w, -r5.y, r1.w
    r1.w = ((-(r5.yyyy))+(r1.wwww)).w;
    // 59: mad r1.w, cb0[17].y, r1.w, r5.y
    r1.w = ((source[17].yyyy)*(r1.wwww)+(r5.yyyy)).w;
    // 60: mad r6.xyz, r1.wwww, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 61: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 63: mad r6.xyz, cb0[17].zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((source[17].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 64: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 66: mad r6.xyz, cb0[17].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[17].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 67: mad r7.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 71: mul r8.xyz, r5.xxxx, r8.xyzx
    r8.xyz = ((r5.xxxx)*(r8.xyzx)).xyz;
    // 72: mul r1.w, cb0[11].z, l(1.500000)
    r1.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 73: add r2.w, -cb0[11].w, l(1.000000)
    r2.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: mul r2.w, r2.w, cb0[20].y
    r2.w = ((r2.wwww)*(source[20].yyyy)).w;
    // 75: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 76: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 77: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 78: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 79: mad r1.w, r1.w, l(0.500000), cb0[11].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 80: frc r2.w, cb0[11].x
    r2.w = (frac(source[11].xxxx)).w;
    // 81: add r3.x, -r2.w, cb0[11].x
    r3.x = ((-(r2.wwww))+(source[11].xxxx)).x;
    // 82: mul r9.z, r3.x, l(0.125000)
    r9.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 83: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 84: mul r9.y, cb0[11].y, cb0[12].y
    r9.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 85: mul r10.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r10.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 86: frc r3.x, r10.x
    r3.x = (frac(r10.xxxx)).x;
    // 87: mul r10.y, r3.x, l(0.125000)
    r10.y = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 88: add r5.xy, r9.xyxx, r10.yzyy
    r5.xy = ((r9.xyxx)+(r10.yzyy)).xy;
    // 89: add r5.xy, r5.xyxx, r9.zwzz
    r5.xy = ((r5.xyxx)+(r9.zwzz)).xy;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 91: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 92: mul r1.w, r2.w, r9.w
    r1.w = ((r2.wwww)*(r9.wwww)).w;
    // 93: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 94: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 95: mad r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = ((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 96: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: dp3 r2.x, r3.yzwy, r2.xyzx
    r2.x = (dot((r3.yzwy).xyz,(r2.xyzx).xyz).xxxx).x;
    // 98: add r2.y, -|r2.x|, l(1.000000)
    r2.y = ((-(abs(r2.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 99: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 100: mul r1.w, r1.w, cb0[13].x
    r1.w = ((r1.wwww)*(source[13].xxxx)).w;
    // 101: lt r2.y, |r1.w|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 102: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 103: mul r1.w, r1.w, cb0[13].y
    r1.w = ((r1.wwww)*(source[13].yyyy)).w;
    // 104: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 105: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: movc r1.w, r2.y, l(1.000000), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r1.wwww)).w;
    // 107: mul r1.w, r1.w, cb0[13].z
    r1.w = ((r1.wwww)*(source[13].zzzz)).w;
    // 108: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 109: lt r2.y, |r1.w|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 110: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 111: mul r1.w, r1.w, cb0[13].w
    r1.w = ((r1.wwww)*(source[13].wwww)).w;
    // 112: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 113: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 114: mad r6.xyz, r6.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r6.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 115: dp3 r2.y, r6.xyzx, r6.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 116: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 117: div r6.xyz, r6.xyzx, r2.yyyy
    r6.xyz = ((r6.xyzx)/(r2.yyyy)).xyz;
    // 118: mul r6.xyz, r5.zzzz, r6.xyzx
    r6.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 119: mov_sat r2.y, r2.z
    r2.y = (saturate(r2.zzzz)).y;
    // 120: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 121: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 122: mul r6.xyz, r6.xyzx, r2.yyyy
    r6.xyz = ((r6.xyzx)*(r2.yyyy)).xyz;
    // 123: mul r6.xyz, r6.xyzx, cb0[18].xxxx
    r6.xyz = ((r6.xyzx)*(source[18].xxxx)).xyz;
    // 124: add r1.xyz, -r1.xyzx, cb0[0].yzwy
    r1.xyz = ((-(r1.xyzx))+(source[0].yzwy)).xyz;
    // 125: mul r7.xyz, r2.zzzz, r1.xyzx
    r7.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 126: mad r1.xyz, r7.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r7.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 127: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 128: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 129: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 130: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 131: dp3 r1.y, r0.xyzx, r3.yzwy
    r1.y = (dot((r0.xyzx).xyz,(r3.yzwy).xyz).xxxx).y;
    // 132: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 133: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 134: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 135: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 136: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 137: mad r1.y, cb0[18].w, l(4.500000), l(0.500000)
    r1.y = ((source[18].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 138: mul r1.y, r1.y, cb0[19].x
    r1.y = ((r1.yyyy)*(source[19].xxxx)).y;
    // 139: mul r1.y, r1.y, l(0.050000)
    r1.y = ((r1.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 140: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 141: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 142: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 143: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 144: mul r1.x, r1.x, cb0[19].y
    r1.x = ((r1.xxxx)*(source[19].yyyy)).x;
    // 145: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 146: add r1.y, -r2.x, l(1.000000)
    r1.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 147: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 148: add r1.z, -r2.x, l(1.000000)
    r1.z = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 149: mul_sat r2.x, r2.z, cb0[19].z
    r2.x = (saturate((r2.zzzz)*(source[19].zzzz))).x;
    // 150: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 151: mad_sat r2.x, r2.x, cb0[19].z, -cb0[19].w
    r2.x = (saturate((r2.xxxx)*(source[19].zzzz)+(-(source[19].wwww)))).x;
    // 152: lt r2.y, r2.x, l(0.000001)
    r2.y = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 153: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 154: mul r2.x, r2.x, cb0[20].x
    r2.x = ((r2.xxxx)*(source[20].xxxx)).x;
    // 155: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 156: mul r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)*(r2.xxxx)).z;
    // 157: movc r1.z, r2.y, l(0), r1.z
    r1.z = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 158: mad r2.xyz, r1.zzzz, cb0[9].xyzx, -cb0[9].xyzx
    r2.xyz = ((r1.zzzz)*(source[9].xyzx)+(-(source[9].xyzx))).xyz;
    // 159: mad r2.xyz, cb0[9].wwww, r2.xyzx, cb0[9].xyzx
    r2.xyz = ((source[9].wwww)*(r2.xyzx)+(source[9].xyzx)).xyz;
    // 160: mad r2.xyz, r1.yyyy, cb0[8].xyzx, r2.xyzx
    r2.xyz = ((r1.yyyy)*(source[8].xyzx)+(r2.xyzx)).xyz;
    // 161: mul r1.y, r1.z, cb0[10].w
    r1.y = ((r1.zzzz)*(source[10].wwww)).y;
    // 162: mad r2.xyz, r1.yyyy, cb0[10].xyzx, r2.xyzx
    r2.xyz = ((r1.yyyy)*(source[10].xyzx)+(r2.xyzx)).xyz;
    // 163: mad r1.xyz, r1.xxxx, r6.xyzx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 164: mad r1.xyz, r1.wwww, r5.zzzz, r1.xyzx
    r1.xyz = ((r1.wwww)*(r5.zzzz)+(r1.xyzx)).xyz;
    // 165: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 166: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 167: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 168: mul r2.xyz, r1.wwww, v7.xyzx
    r2.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 169: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 170: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 171: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 172: mul r2.yzw, r2.yyyy, cb0[22].xxyz
    r2.yzw = ((r2.yyyy)*(source[22].xxyz)).yzw;
    // 173: mad r2.xyz, r2.xxxx, cb0[21].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[21].xyzx)+(r2.yzwy)).xyz;
    // 174: mul r2.xyz, r2.xyzx, cb0[23].wwww
    r2.xyz = ((r2.xyzx)*(source[23].wwww)).xyz;
    // 175: mul r3.xyz, r8.xyzx, r2.xyzx
    r3.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 176: mad r1.xyz, r2.xyzx, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 177: mad r1.xyz, r8.xyzx, cb0[23].xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 178: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 179: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 180: eq r2.x, cb0[24].x, l(0.000000)
    r2.x = (asfloat((uint4)((source[24].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 181: not r2.y, r2.x
    r2.y = (asfloat(~asuint(r2.xxxx))).y;
    // 182: lt r2.z, r0.w, r1.w
    r2.z = (asfloat((uint4)((r0.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 183: and r2.y, r2.z, r2.y
    r2.y = (asfloat(asuint(r2.zzzz) & asuint(r2.yyyy))).y;
    // 184: discard_nz r2.y
    if ((asuint(r2.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 185: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 186: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 187: mul r2.yzw, r2.yyyy, v0.xxyz
    r2.yzw = ((r2.yyyy)*(v0.xxyz)).yzw;
    // 188: mul r5.xyz, r0.zxyz, r2.zwyz
    r5.xyz = ((r0.zxyz)*(r2.zwyz)).xyz;
    // 189: mad r5.xyz, r0.yzxy, r2.wyzw, -r5.xyzx
    r5.xyz = ((r0.yzxy)*(r2.wyzw)+(-(r5.xyzx))).xyz;
    // 190: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 191: movc r3.w, v9.x, l(1.000000), l(-1.000000)
    r3.w = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 192: mul r3.w, r3.w, cb0[0].x
    r3.w = ((r3.wwww)*(source[0].xxxx)).w;
    // 193: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 194: ge r1.w, r0.w, r1.w
    r1.w = (asfloat((uint4)((r0.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 195: mad r3.w, r5.w, cb0[1].x, l(-0.900000)
    r3.w = ((r5.wwww)*(source[1].xxxx)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 196: mul_sat r3.w, r3.w, l(9.999998)
    r3.w = (saturate((r3.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 197: mad r4.w, r3.w, l(-2.000000), l(3.000000)
    r4.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 198: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 199: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 200: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 201: movc r1.w, r1.w, r3.w, r0.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r3.wwww) : (r0.wwww)).w;
    // 202: movc o0.w, r2.x, r1.w, r0.w
    output.targets[0].w = ((asuint(r2.xxxx) != 0u) ? (r1.wwww) : (r0.wwww)).w;
    // 203: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 204: dp3 r1.x, r2.yzwy, r4.xyzx
    r1.x = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).x;
    // 205: dp3 r1.y, r5.xyzx, r4.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 206: dp3 r1.z, r0.xyzx, r4.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 207: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 208: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 209: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 210: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 211: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 212: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 213: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 214: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 215: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 216: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 217: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 218: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 219: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 220: mov o3.xyz, r8.xyzx
    output.targets[3].xyz = (r8.xyzx).xyz;
    // 221: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 222: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 223: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 224: ret
    return output;
}

// source.character.equipment-native-183.v1 / source program f64d5c3026cea04daa80e12a7841688d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase183(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[23]=g_SourceCharacterEnvironmentColor; source[24]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
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
    // 7: add r0.x, -cb0[7].w, l(1.000000)
    r0.x = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul r0.x, r0.x, cb0[18].z
    r0.x = ((r0.xxxx)*(source[18].zzzz)).x;
    // 9: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 10: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 11: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r1.x, cb0[7].z, l(1.500000)
    r1.x = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 13: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 15: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 16: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[7].y, cb0[14].y
    r2.y = ((source[7].yyyy)*(source[14].yyyy)).y;
    // 18: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 21: frc r1.z, cb0[7].x
    r1.z = (frac(source[7].xxxx)).z;
    // 22: add r1.w, -r1.z, cb0[7].x
    r1.w = ((-(r1.zzzz))+(source[7].xxxx)).w;
    // 23: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 24: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r1.xyw, r0.xxxx, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r2.xyxz)).xyw;
    // 27: mul r0.x, r1.z, r2.w
    r0.x = ((r1.zzzz)*(r2.wwww)).x;
    // 28: dp3 r1.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 29: add r2.xyz, -r0.yzwy, r1.zzzz
    r2.xyz = ((-(r0.yzwy))+(r1.zzzz)).xyz;
    // 30: mad r2.xyz, cb0[16].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[16].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[17].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[17].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.z, r6.y, cb0[16].y
    r1.z = ((r6.yyyy)*(source[16].yyyy)).z;
    // 44: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 45: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: movc r1.z, r5.y, l(0), r1.z
    r1.z = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 47: mad r3.xyz, r1.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.zzzz, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 57: mad r3.xyz, r7.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r7.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 58: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 59: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 60: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 61: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 62: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 63: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 64: mad r4.xyz, r1.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 65: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 66: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 67: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 68: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 69: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 70: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 71: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 72: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 73: mad r4.xyz, r1.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 74: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 75: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 76: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 78: mad r4.xyz, cb0[16].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[16].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 79: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 80: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: add r3.xyz, -r4.xyzx, r2.wwww
    r3.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 82: mad r3.xyz, cb0[17].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[17].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 83: mad r4.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mad r8.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 86: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 87: mul r8.xyz, r2.xyzx, r3.xyzx
    r8.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 88: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: mad r2.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 90: mad r2.xyz, cb0[16].wwww, r2.xyzx, r8.xyzx
    r2.xyz = ((source[16].wwww)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 91: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 93: mad r2.xyz, cb0[17].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[17].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 94: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 95: mad r1.xyw, r1.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r2.xyxz
    r1.xyw = ((r1.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r2.xyxz))).xyw;
    // 96: mad r1.xyw, r0.xxxx, r1.xyxw, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 97: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 98: add r0.x, r1.w, r0.x
    r0.x = ((r1.wwww)+(r0.xxxx)).x;
    // 99: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 100: max r0.x, r0.x, cb0[19].x
    r0.x = (max(r0.xxxx,source[19].xxxx)).x;
    // 101: min r0.x, r0.x, cb0[18].w
    r0.x = (min(r0.xxxx,source[18].wwww)).x;
    // 102: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: mad r0.x, r1.z, r2.x, r0.x
    r0.x = ((r1.zzzz)*(r2.xxxx)+(r0.xxxx)).x;
    // 104: mul_sat r2.w, r1.z, cb2[3].w
    r2.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 105: add r1.z, r0.x, l(-1.000000)
    r1.z = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 106: mad r1.z, cb0[19].z, r1.z, l(1.000000)
    r1.z = ((source[19].zzzz)*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 107: mul r3.xyz, r1.xywx, r1.zzzz
    r3.xyz = ((r1.xywx)*(r1.zzzz)).xyz;
    // 108: mul r2.x, r6.x, cb0[18].x
    r2.x = ((r6.xxxx)*(source[18].xxxx)).x;
    // 109: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 110: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 111: add_sat r2.x, r2.x, cb0[18].y
    r2.x = (saturate((r2.xxxx)+(source[18].yyyy))).x;
    // 112: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 113: mul r4.xyz, r2.yyyy, cb0[13].xyzx
    r4.xyz = ((r2.yyyy)*(source[13].xyzx)).xyz;
    // 114: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 115: mad r1.xyz, r1.zzzz, r1.xywx, -r3.xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(-(r3.xyzx))).xyz;
    // 116: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 117: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 118: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 119: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 120: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 121: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 122: mad r4.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 123: mad r5.xyw, r1.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r5.xyw = ((r1.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 124: mad r4.xyz, r0.xxxx, r4.xyzx, r5.xywx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)+(r5.xywx)).xyz;
    // 125: mad r3.xyz, r4.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 126: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 127: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 128: mov_sat r1.w, cb0[19].w
    r1.w = (saturate(source[19].wwww)).w;
    // 129: mad r4.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r4.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 130: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 131: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 132: mad r4.xyz, r2.wwww, r4.xyzx, r2.xxxx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xxxx)).xyz;
    // 133: mul_sat r1.w, r4.y, l(50.000000)
    r1.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 134: add r2.x, -cb0[20].w, cb0[20].z
    r2.x = ((-(source[20].wwww))+(source[20].zzzz)).x;
    // 135: mad r2.x, r7.x, r2.x, cb0[20].w
    r2.x = ((r7.xxxx)*(r2.xxxx)+(source[20].wwww)).x;
    // 136: add r2.y, -r2.x, cb0[21].y
    r2.y = ((-(r2.xxxx))+(source[21].yyyy)).y;
    // 137: mad r2.x, r7.y, r2.y, r2.x
    r2.x = ((r7.yyyy)*(r2.yyyy)+(r2.xxxx)).x;
    // 138: add r2.y, -r2.x, cb0[21].w
    r2.y = ((-(r2.xxxx))+(source[21].wwww)).y;
    // 139: mad r2.x, r7.z, r2.y, r2.x
    r2.x = ((r7.zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 140: mul r2.x, r6.z, r2.x
    r2.x = ((r6.zzzz)*(r2.xxxx)).x;
    // 141: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 142: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: movc r2.x, r5.z, l(0), r2.x
    r2.x = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 144: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 145: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 147: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 148: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 149: mul r5.xy, r2.xyxx, cb0[16].xxxx
    r5.xy = ((r2.xyxx)*(source[16].xxxx)).xy;
    // 150: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 151: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 152: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 153: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 154: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 155: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 156: div r5.xyz, r5.xyzx, r2.xxxx
    r5.xyz = ((r5.xyzx)/(r2.xxxx)).xyz;
    // 157: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 158: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 159: mul r6.xyz, r2.xxxx, r5.xyzx
    r6.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 160: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 161: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 162: mul r7.xyz, r2.xxxx, v5.xyzx
    r7.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 163: dp3 r2.x, r6.xyzx, r7.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 164: deriv_rtx_coarse r8.x, r2.x
    r8.x = (ddx_coarse(r2.xxxx)).x;
    // 165: deriv_rty_coarse r8.y, r2.x
    r8.y = (ddy_coarse(r2.xxxx)).y;
    // 166: dp2 r2.y, r8.xyxx, r8.xyxx
    r2.y = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).y;
    // 167: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 168: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 169: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 170: min r8.y, r2.y, l(1.000000)
    r8.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: add r2.y, -r8.y, l(1.000000)
    r2.y = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 172: max r9.xyz, r4.xyzx, r2.yyyy
    r9.xyz = (max(r4.xyzx,r2.yyyy)).xyz;
    // 173: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 174: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 175: mul r10.xyz, r2.xxxx, r6.xyzx
    r10.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 176: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 177: add r1.w, r10.z, l(1.000000)
    r1.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 180: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 181: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 182: mul r2.x, r2.x, cb0[1].y
    r2.x = ((r2.xxxx)*(source[1].yyyy)).x;
    // 183: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 184: mad_sat r2.x, r2.x, cb0[1].w, cb0[1].z
    r2.x = (saturate((r2.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 185: mul r2.x, r2.x, cb0[22].x
    r2.x = ((r2.xxxx)*(source[22].xxxx)).x;
    // 186: add_sat r8.x, -r1.w, r2.y
    r8.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 187: sample_indexable(texture2d)(float,float,float,float) r2.yz, r8.xyxx, t5.zxyw, s6
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 188: add r1.w, r0.x, r8.x
    r1.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 189: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 190: mul r8.xzw, r2.zzzz, r4.xxyz
    r8.xzw = ((r2.zzzz)*(r4.xxyz)).xzw;
    // 191: mad r8.xzw, r9.xxyz, r2.yyyy, r8.xxzw
    r8.xzw = ((r9.xxyz)*(r2.yyyy)+(r8.xxzw)).xzw;
    // 192: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 193: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 194: mad r9.xyz, r4.xyzx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((r4.xyzx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 195: dp3 r2.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mad r4.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 197: mad r11.xyz, -r8.xzwx, r9.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r8.xzwx))*(r9.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mul r8.xzw, r8.xxzw, r9.xxyz
    r8.xzw = ((r8.xxzw)*(r9.xxyz)).xzw;
    // 199: mul r9.xyz, r1.xyzx, r11.xyzx
    r9.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 200: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 201: mul r9.xyz, r2.yyyy, r9.xyzx
    r9.xyz = ((r2.yyyy)*(r9.xyzx)).xyz;
    // 202: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 203: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 204: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 205: mul r12.xyz, r3.wwww, v1.xyzx
    r12.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 206: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 207: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 208: mul r13.xyz, r3.wwww, v0.xyzx
    r13.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 209: mul r14.xyz, r12.zxyz, r13.yzxy
    r14.xyz = ((r12.zxyz)*(r13.yzxy)).xyz;
    // 210: mad r14.xyz, r12.yzxy, r13.zxyz, -r14.xyzx
    r14.xyz = ((r12.yzxy)*(r13.zxyz)+(-(r14.xyzx))).xyz;
    // 211: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 212: dp3 r15.y, r14.xyzx, r6.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 213: dp3 r14.y, r14.xyzx, r10.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 214: dp3 r15.x, r13.xyzx, r6.xyzx
    r15.x = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 215: dp3 r14.x, r13.xyzx, r10.xyzx
    r14.x = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 216: dp2 r13.z, r15.xyxx, cb0[24].xyxx
    r13.z = (dot((r15.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 217: mul r14.zw, cb0[24].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r14.zw = ((source[24].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 218: dp2 r13.x, r15.xyxx, r14.zwzz
    r13.x = (dot((r15.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 219: dp2 r16.x, r14.xyxx, r14.zwzz
    r16.x = (dot((r14.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 220: dp2 r16.z, r14.xyxx, cb0[24].xyxx
    r16.z = (dot((r14.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 221: dp3 r13.y, r12.xyzx, r6.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 222: dp3 r16.y, r12.xyzx, r10.xyzx
    r16.y = (dot((r12.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 223: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 224: dp4 r12.x, cb0[25].xyzw, r13.xyzw
    r12.x = (dot((source[25].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 225: dp4 r12.y, cb0[26].xyzw, r13.xyzw
    r12.y = (dot((source[26].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 226: dp4 r12.z, cb0[27].xyzw, r13.xyzw
    r12.z = (dot((source[27].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 227: mul r14.xyzw, r13.yzzx, r13.xyzz
    r14.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 228: dp4 r17.x, cb0[28].xyzw, r14.xyzw
    r17.x = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 229: dp4 r17.y, cb0[29].xyzw, r14.xyzw
    r17.y = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 230: dp4 r17.z, cb0[30].xyzw, r14.xyzw
    r17.z = (dot((source[30].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 231: add r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)+(r17.xyzx)).xyz;
    // 232: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 233: mov r15.z, r13.y
    r15.z = (r13.yyyy).z;
    // 234: mad r3.w, r13.x, r13.x, -r3.w
    r3.w = ((r13.xxxx)*(r13.xxxx)+(-(r3.wwww))).w;
    // 235: mad r12.xyz, cb0[31].xyzx, r3.wwww, r12.xyzx
    r12.xyz = ((source[31].xyzx)*(r3.wwww)+(r12.xyzx)).xyz;
    // 236: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 237: mul r12.xyz, r12.xyzx, cb0[23].xyzx
    r12.xyz = ((r12.xyzx)*(source[23].xyzx)).xyz;
    // 238: mul r12.xyz, r12.xyzx, cb0[24].zzzz
    r12.xyz = ((r12.xyzx)*(source[24].zzzz)).xyz;
    // 239: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[23].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[23].wwww)).xyz;
    // 240: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 241: add r12.xyz, -r3.wwww, r12.xyzx
    r12.xyz = ((-(r3.wwww))+(r12.xyzx)).xyz;
    // 242: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 243: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 244: mad r4.w, r8.y, l(2.000000), l(2.000000)
    r4.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 245: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 246: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 247: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 248: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 249: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 250: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 251: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 252: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 253: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 254: mul r12.xyz, r3.wwww, r12.xyzx
    r12.xyz = ((r3.wwww)*(r12.xyzx)).xyz;
    // 255: mul r9.xyz, r9.xyzx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r12.xyzx)).xyz;
    // 256: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 257: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 258: mul r3.w, r8.y, l(5.000000)
    r3.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 259: mul r5.w, r8.y, r8.y
    r5.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 260: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 261: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 262: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 263: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 264: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 265: sample_l_indexable(texturecube)(float,float,float,float) r12.xyzw, r16.xyzx, t6.xyzw, s5, r3.w
    r12.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r16.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 266: mul r12.xyz, r12.xyzx, r12.wwww
    r12.xyz = ((r12.xyzx)*(r12.wwww)).xyz;
    // 267: mul r12.xyz, r12.xyzx, cb0[23].xyzx
    r12.xyz = ((r12.xyzx)*(source[23].xyzx)).xyz;
    // 268: mul r12.xyz, r12.xyzx, cb0[24].zzzz
    r12.xyz = ((r12.xyzx)*(source[24].zzzz)).xyz;
    // 269: mad r12.xyz, r12.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[23].wwww
    r12.xyz = ((r12.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[23].wwww)).xyz;
    // 270: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 271: add r12.xyz, -r1.wwww, r12.xyzx
    r12.xyz = ((-(r1.wwww))+(r12.xyzx)).xyz;
    // 272: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 273: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 275: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 276: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 277: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 278: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 279: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 280: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 281: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 282: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 283: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 284: mul r13.xyz, r8.xzwx, r12.xyzx
    r13.xyz = ((r8.xzwx)*(r12.xyzx)).xyz;
    // 285: mad r1.w, r0.x, r4.x, r4.y
    r1.w = ((r0.xxxx)*(r4.xxxx)+(r4.yyyy)).w;
    // 286: mad r1.w, r1.w, r0.x, r4.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r4.zzzz)).w;
    // 287: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 288: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 289: mad r4.xyz, r13.xyzx, r0.xxxx, r9.xyzx
    r4.xyz = ((r13.xyzx)*(r0.xxxx)+(r9.xyzx)).xyz;
    // 290: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 291: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 292: mul r9.xyz, r1.wwww, v6.xyzx
    r9.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 293: dp3 r1.w, r9.xyzx, r6.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 294: dp3 r2.z, -r9.xyzx, r6.xyzx
    r2.z = (dot((-(r9.xyzx)).xyz,(r6.xyzx).xyz).xxxx).z;
    // 295: dp3 r3.w, r9.xyzx, r10.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 296: mad r6.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 297: mad r6.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 298: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 299: mad r9.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 300: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 301: mul r9.yzw, r9.yyyy, cb0[34].xxyz
    r9.yzw = ((r9.yyyy)*(source[34].xxyz)).yzw;
    // 302: mad r9.xyz, r9.xxxx, cb0[33].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[33].xyzx)+(r9.yzwy)).xyz;
    // 303: mul r9.xyz, r9.xyzx, cb0[35].wwww
    r9.xyz = ((r9.xyzx)*(source[35].wwww)).xyz;
    // 304: mul r9.xyz, r1.xyzx, r9.xyzx
    r9.xyz = ((r1.xyzx)*(r9.xyzx)).xyz;
    // 305: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 306: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 307: mul r3.xyz, r11.xyzx, r3.xyzx
    r3.xyz = ((r11.xyzx)*(r3.xyzx)).xyz;
    // 308: mad r3.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 309: mad r3.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 310: mul r4.xyz, r6.yyyy, cb0[34].xyzx
    r4.xyz = ((r6.yyyy)*(source[34].xyzx)).xyz;
    // 311: mad r4.xyz, cb0[33].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[33].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 312: mul r4.xyz, r4.xyzx, cb0[35].wwww
    r4.xyz = ((r4.xyzx)*(source[35].wwww)).xyz;
    // 313: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 314: mul r4.xyz, r12.xyzx, r4.xyzx
    r4.xyz = ((r12.xyzx)*(r4.xyzx)).xyz;
    // 315: mul r4.xyz, r4.xyzx, r8.xzwx
    r4.xyz = ((r4.xyzx)*(r8.xzwx)).xyz;
    // 316: mad r3.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 317: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 318: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 319: dp3 r0.x, r5.xyzx, r7.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 320: mul_sat r1.w, r0.x, cb0[17].y
    r1.w = (saturate((r0.xxxx)*(source[17].yyyy))).w;
    // 321: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 322: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 323: mul_sat r2.z, r7.z, cb0[17].y
    r2.z = (saturate((r7.zzzz)*(source[17].yyyy))).z;
    // 324: add r2.w, -|r7.z|, l(1.000000)
    r2.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 325: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 326: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 327: add_sat r2.z, r2.z, -cb0[17].z
    r2.z = (saturate((r2.zzzz)+(-(source[17].zzzz)))).z;
    // 328: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 329: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 330: mul r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)*(source[17].wwww)).w;
    // 331: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 332: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 333: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 334: mad r4.xyz, r1.wwww, cb0[11].xyzx, -cb0[11].xyzx
    r4.xyz = ((r1.wwww)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 335: mul r1.w, r1.w, cb0[10].w
    r1.w = ((r1.wwww)*(source[10].wwww)).w;
    // 336: mad r4.xyz, cb0[11].wwww, r4.xyzx, cb0[11].xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(source[11].xyzx)).xyz;
    // 337: mad r4.xyz, r1.wwww, cb0[10].xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(source[10].xyzx)+(r4.xyzx)).xyz;
    // 338: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 339: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 340: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 341: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 342: mul r5.xyz, r1.wwww, cb0[12].xyzx
    r5.xyz = ((r1.wwww)*(source[12].xyzx)).xyz;
    // 343: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 344: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 345: mad r0.xyz, cb0[16].zzzz, r0.yzwy, r4.xyzx
    r0.xyz = ((source[16].zzzz)*(r0.yzwy)+(r4.xyzx)).xyz;
    // 346: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 347: mul r4.xyz, r6.wwww, cb0[34].xyzx
    r4.xyz = ((r6.wwww)*(source[34].xyzx)).xyz;
    // 348: mad r4.xyz, r6.zzzz, cb0[33].xyzx, r4.xyzx
    r4.xyz = ((r6.zzzz)*(source[33].xyzx)+(r4.xyzx)).xyz;
    // 349: mul r4.xyz, r4.xyzx, cb0[35].wwww
    r4.xyz = ((r4.xyzx)*(source[35].wwww)).xyz;
    // 350: mul_sat r5.xyz, cb0[15].xyzx, cb0[15].wwww
    r5.xyz = (saturate((source[15].xyzx)*(source[15].wwww))).xyz;
    // 351: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 352: mul r5.xyz, r5.xyzx, cb0[22].xxxx
    r5.xyz = ((r5.xyzx)*(source[22].xxxx)).xyz;
    // 353: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 354: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 355: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 356: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 357: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 358: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 359: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 360: mad o0.xyz, r1.xyzx, cb0[35].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[35].xyzx)+(r0.xyzx)).xyz;
    // 361: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 362: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 363: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 364: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 365: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 366: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 367: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 368: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 369: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 370: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 371: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 372: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 373: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 374: ftou r0.x, cb0[32].z
    r0.x = (asfloat((uint4)(source[32].zzzz))).x;
    // 375: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 376: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 377: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 378: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 379: ret
    return output;
}

// source.character.equipment-native-184.v1 / source program 1b436c9728fe7c43880ac0f415a894ff
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase184(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[19]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[23].x=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[30]=g_SourceCharacterEnvironmentColor; source[31]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0, r22=0.0, r23=0.0, r24=0.0;
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
    // 7: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 8: mul r1.x, r0.x, l(0.125000)
    r1.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 9: mul r2.y, cb0[9].y, cb0[19].y
    r2.y = ((source[9].yyyy)*(source[19].yyyy)).y;
    // 10: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 11: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 12: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 13: frc r0.x, cb0[9].x
    r0.x = (frac(source[9].xxxx)).x;
    // 14: add r1.z, -r0.x, cb0[9].x
    r1.z = ((-(r0.xxxx))+(source[9].xxxx)).z;
    // 15: mul r2.z, r1.z, l(0.125000)
    r2.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 16: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t4.xyzw, s6, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 19: add r1.w, -cb0[9].w, l(1.000000)
    r1.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mul r1.w, r1.w, cb0[23].x
    r1.w = ((r1.wwww)*(source[23].xxxx)).w;
    // 21: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 22: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 23: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r2.x, cb0[9].z, l(1.500000)
    r2.x = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mad r1.w, r1.w, l(0.500000), cb0[9].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 27: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 28: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 30: mad r2.xyz, cb0[24].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[24].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 33: mad r2.xyz, cb0[25].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.w, r6.y, cb0[21].y
    r1.w = ((r6.yyyy)*(source[21].yyyy)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: add r7.xyz, -r3.xyzx, r4.xyzx
    r7.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 56: round_ni r5.yw, v7.xxxy
    r5.yw = (floor(v7.xxxy)).yw;
    // 57: dp2 r2.w, r5.ywyy, l(12.989800, 78.233002, 0.000000, 0.000000)
    r2.w = (dot((r5.ywyy).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).w;
    // 58: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 59: mul r2.w, r2.w, l(43758.546875)
    r2.w = ((r2.wwww)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).w;
    // 60: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 61: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 63: mad r2.w, r2.w, l(0.010000), r8.x
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(r8.xxxx)).w;
    // 64: lt r3.w, cb0[21].w, r2.w
    r3.w = (asfloat((uint4)((source[21].wwww)<(r2.wwww)) * 0xffffffffu)).w;
    // 65: lt r2.w, r2.w, cb0[21].z
    r2.w = (asfloat((uint4)((r2.wwww)<(source[21].zzzz)) * 0xffffffffu)).w;
    // 66: movc r2.w, r2.w, l(-1.000000), l(-0.000000)
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).w;
    // 67: and r4.w, r3.w, l(0x3f800000)
    r4.w = (asfloat(asuint(r3.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 68: movc r3.w, r3.w, l(0), l(1.000000)
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: add r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)+(r3.wwww)).w;
    // 70: mad r3.xyz, r4.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r4.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 71: add r3.w, r8.y, r4.w
    r3.w = ((r8.yyyy)+(r4.wwww)).w;
    // 72: add r3.w, r8.z, r3.w
    r3.w = ((r8.zzzz)+(r3.wwww)).w;
    // 73: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 74: mul r7.xyz, cb0[6].xyzx, cb0[6].wwww
    r7.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 75: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 76: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: max r9.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 78: add r9.xyz, -r7.xyzx, r9.xyzx
    r9.xyz = ((-(r7.xyzx))+(r9.xyzx)).xyz;
    // 79: mad r7.xyz, r1.wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 80: add r9.xyz, -r3.xyzx, r7.xyzx
    r9.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 81: mad r3.xyz, r2.wwww, r9.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r9.xyz, cb0[7].xyzx, cb0[7].wwww
    r9.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 83: max r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 84: max r9.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 85: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 86: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 87: add r10.xyz, -r9.xyzx, r10.xyzx
    r10.xyz = ((-(r9.xyzx))+(r10.xyzx)).xyz;
    // 88: mad r9.xyz, r1.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r1.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 89: add r10.xyz, -r3.xyzx, r9.xyzx
    r10.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 90: mad r3.xyz, r8.yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((r8.yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 91: mul r10.xyz, cb0[8].xyzx, cb0[8].wwww
    r10.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 92: max r11.xyz, r10.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r11.xyz = (max(r10.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 93: max r10.xyz, r10.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 94: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 95: min r11.xyz, r11.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r11.xyz = (min(r11.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 96: add r11.xyz, -r10.xyzx, r11.xyzx
    r11.xyz = ((-(r10.xyzx))+(r11.xyzx)).xyz;
    // 97: mad r10.xyz, r1.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r1.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 98: add r11.xyz, -r3.xyzx, r10.xyzx
    r11.xyz = ((-(r3.xyzx))+(r10.xyzx)).xyz;
    // 99: mad r3.xyz, r8.zzzz, r11.xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 100: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: add r11.xyz, -r3.xyzx, r4.wwww
    r11.xyz = ((-(r3.xyzx))+(r4.wwww)).xyz;
    // 102: mad r11.xyz, cb0[24].wwww, r11.xyzx, r3.xyzx
    r11.xyz = ((source[24].wwww)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 103: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 104: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 105: add r3.xyz, -r11.xyzx, r3.xxxx
    r3.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 106: mad r3.xyz, cb0[25].xxxx, r3.xyzx, r11.xyzx
    r3.xyz = ((source[25].xxxx)*(r3.xyzx)+(r11.xyzx)).xyz;
    // 107: mad r11.xyz, cb0[13].wwww, cb0[13].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[13].wwww)*(source[13].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 108: mad r12.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 109: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 110: mul r3.xyz, r3.xyzx, r11.xyzx
    r3.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 111: mul r12.xyz, r2.xyzx, r3.xyzx
    r12.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 112: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: mad r2.xyz, -r3.xyzx, r2.xyzx, r4.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r4.wwww)).xyz;
    // 114: mad r2.xyz, cb0[24].wwww, r2.xyzx, r12.xyzx
    r2.xyz = ((source[24].wwww)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 115: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: add r3.xyz, -r2.xyzx, r3.xxxx
    r3.xyz = ((-(r2.xyzx))+(r3.xxxx)).xyz;
    // 117: mad r2.xyz, cb0[25].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 118: mul r2.xyz, r11.xyzx, r2.xyzx
    r2.xyz = ((r11.xyzx)*(r2.xyzx)).xyz;
    // 119: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 120: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 121: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 122: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 123: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 124: max r0.x, r0.x, cb0[26].x
    r0.x = (max(r0.xxxx,source[26].xxxx)).x;
    // 125: min r0.x, r0.x, cb0[25].w
    r0.x = (min(r0.xxxx,source[25].wwww)).x;
    // 126: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 127: mad r0.x, r1.w, r2.x, r0.x
    r0.x = ((r1.wwww)*(r2.xxxx)+(r0.xxxx)).x;
    // 128: mul_sat r12.w, r1.w, cb2[3].w
    r12.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 129: add r1.w, r0.x, l(-1.000000)
    r1.w = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 130: mad r1.w, cb0[26].z, r1.w, l(1.000000)
    r1.w = ((source[26].zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r2.xyz, r1.xyzx, r1.wwww
    r2.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 132: mul r3.x, r6.x, cb0[25].y
    r3.x = ((r6.xxxx)*(source[25].yyyy)).x;
    // 133: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 134: movc r3.x, r5.x, l(0), r3.x
    r3.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 135: add_sat r3.x, r3.x, cb0[25].z
    r3.x = (saturate((r3.xxxx)+(source[25].zzzz))).x;
    // 136: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: mul r5.xyw, r3.yyyy, cb0[18].xyxz
    r5.xyw = ((r3.yyyy)*(source[18].xyxz)).xyw;
    // 138: mul r2.xyz, r2.xyzx, r5.xywx
    r2.xyz = ((r2.xyzx)*(r5.xywx)).xyz;
    // 139: mad r1.xyz, r1.wwww, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 140: mad r1.xyz, r3.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 141: mul_sat r0.x, r0.x, r3.x
    r0.x = (saturate((r0.xxxx)*(r3.xxxx))).x;
    // 142: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 143: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 144: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 145: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 147: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 148: dp2 r3.x, r2.yzyy, r2.yzyy
    r3.x = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).x;
    // 149: mul r13.xy, r2.yzyy, cb0[21].xxxx
    r13.xy = ((r2.yzyy)*(source[21].xxxx)).xy;
    // 150: add r2.y, -r3.x, l(1.000000)
    r2.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 151: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 152: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 153: add r13.z, r2.y, l(0.000010)
    r13.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 154: dp3 r2.y, r13.xyzx, r13.xyzx
    r2.y = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 155: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 156: div r3.xyz, r13.xyzx, r2.yyyy
    r3.xyz = ((r13.xyzx)/(r2.yyyy)).xyz;
    // 157: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 158: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 159: mul r5.xyw, r2.yyyy, r3.xyxz
    r5.xyw = ((r2.yyyy)*(r3.xyxz)).xyw;
    // 160: dp3 r2.y, v1.xyzx, v1.xyzx
    r2.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 161: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 162: mul r6.xyw, r2.yyyy, v1.xyxz
    r6.xyw = ((r2.yyyy)*(v1.xyxz)).xyw;
    // 163: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 164: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 165: mul r13.xyz, r2.yyyy, v0.xyzx
    r13.xyz = ((r2.yyyy)*(v0.xyzx)).xyz;
    // 166: mul r14.xyz, r6.wxyw, r13.yzxy
    r14.xyz = ((r6.wxyw)*(r13.yzxy)).xyz;
    // 167: mad r14.xyz, r6.ywxy, r13.zxyz, -r14.xyzx
    r14.xyz = ((r6.ywxy)*(r13.zxyz)+(-(r14.xyzx))).xyz;
    // 168: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 169: dp3 r15.y, r14.xyzx, r5.xywx
    r15.y = (dot((r14.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 170: dp3 r15.x, r13.xyzx, r5.xywx
    r15.x = (dot((r13.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 171: dp2 r16.z, r15.xyxx, cb0[31].xyxx
    r16.z = (dot((r15.xyxx).xy,(source[31].xyxx).xy).xxxx).z;
    // 172: mul r2.yz, cb0[31].yyxy, l(0.000000, 1.000000, -1.000000, 0.000000)
    r2.yz = ((source[31].yyxy)*(float4(0.000000,1.000000,-1.000000,0.000000))).yz;
    // 173: dp2 r16.x, r15.xyxx, r2.yzyy
    r16.x = (dot((r15.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 174: dp3 r16.y, r6.xywx, r5.xywx
    r16.y = (dot((r6.xywx).xyz,(r5.xywx).xyz).xxxx).y;
    // 175: mov r16.w, l(1.000000)
    r16.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 176: dp4 r17.x, cb0[32].xyzw, r16.xyzw
    r17.x = (dot((source[32].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 177: dp4 r17.y, cb0[33].xyzw, r16.xyzw
    r17.y = (dot((source[33].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 178: dp4 r17.z, cb0[34].xyzw, r16.xyzw
    r17.z = (dot((source[34].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 179: mul r18.xyzw, r16.yzzx, r16.xyzz
    r18.xyzw = ((r16.yzzx)*(r16.xyzz)).xyzw;
    // 180: dp4 r19.x, cb0[35].xyzw, r18.xyzw
    r19.x = (dot((source[35].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).x;
    // 181: dp4 r19.y, cb0[36].xyzw, r18.xyzw
    r19.y = (dot((source[36].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).y;
    // 182: dp4 r19.z, cb0[37].xyzw, r18.xyzw
    r19.z = (dot((source[37].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).z;
    // 183: add r17.xyz, r17.xyzx, r19.xyzx
    r17.xyz = ((r17.xyzx)+(r19.xyzx)).xyz;
    // 184: mul r4.w, r16.y, r16.y
    r4.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 185: mov r15.z, r16.y
    r15.z = (r16.yyyy).z;
    // 186: mad r4.w, r16.x, r16.x, -r4.w
    r4.w = ((r16.xxxx)*(r16.xxxx)+(-(r4.wwww))).w;
    // 187: mad r16.xyz, cb0[38].xyzx, r4.wwww, r17.xyzx
    r16.xyz = ((source[38].xyzx)*(r4.wwww)+(r17.xyzx)).xyz;
    // 188: max r16.xyz, r16.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r16.xyz = (max(r16.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 189: mul r16.xyz, r16.xyzx, cb0[30].xyzx
    r16.xyz = ((r16.xyzx)*(source[30].xyzx)).xyz;
    // 190: mul r16.xyz, r16.xyzx, cb0[31].zzzz
    r16.xyz = ((r16.xyzx)*(source[31].zzzz)).xyz;
    // 191: mad r16.xyz, r16.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[30].wwww
    r16.xyz = ((r16.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[30].wwww)).xyz;
    // 192: dp3 r4.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 193: add r16.xyz, -r4.wwww, r16.xyzx
    r16.xyz = ((-(r4.wwww))+(r16.xyzx)).xyz;
    // 194: mad r16.xyz, r16.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r4.wwww
    r16.xyz = ((r16.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r4.wwww)).xyz;
    // 195: dp3 r4.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: add r7.w, -cb0[27].w, cb0[27].z
    r7.w = ((-(source[27].wwww))+(source[27].zzzz)).w;
    // 197: mad r7.w, r8.x, r7.w, cb0[27].w
    r7.w = ((r8.xxxx)*(r7.wwww)+(source[27].wwww)).w;
    // 198: add r9.w, -r7.w, cb0[28].y
    r9.w = ((-(r7.wwww))+(source[28].yyyy)).w;
    // 199: mad r7.w, r8.y, r9.w, r7.w
    r7.w = ((r8.yyyy)*(r9.wwww)+(r7.wwww)).w;
    // 200: add r9.w, -r7.w, cb0[28].w
    r9.w = ((-(r7.wwww))+(source[28].wwww)).w;
    // 201: mad r7.w, r8.z, r9.w, r7.w
    r7.w = ((r8.zzzz)*(r9.wwww)+(r7.wwww)).w;
    // 202: add r9.w, -r7.w, cb0[29].y
    r9.w = ((-(r7.wwww))+(source[29].yyyy)).w;
    // 203: mad r7.w, r2.w, r9.w, r7.w
    r7.w = ((r2.wwww)*(r9.wwww)+(r7.wwww)).w;
    // 204: mul r6.z, r6.z, r7.w
    r6.z = ((r6.zzzz)*(r7.wwww)).z;
    // 205: exp r6.z, r6.z
    r6.z = (exp2(r6.zzzz)).z;
    // 206: min r6.z, r6.z, l(1.000000)
    r6.z = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: movc r5.z, r5.z, l(0), r6.z
    r5.z = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.zzzz)).z;
    // 208: max r5.z, r5.z, cb0[1].x
    r5.z = (max(r5.zzzz,source[1].xxxx)).z;
    // 209: min r12.z, r5.z, l(1.000000)
    r12.z = (min(r5.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 210: dp3 r5.z, v5.xyzx, v5.xyzx
    r5.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 211: rsq r5.z, r5.z
    r5.z = (rsqrt(r5.zzzz)).z;
    // 212: mul r17.xyz, r5.zzzz, v5.xyzx
    r17.xyz = ((r5.zzzz)*(v5.xyzx)).xyz;
    // 213: dp3 r5.z, r5.xywx, r17.xyzx
    r5.z = (dot((r5.xywx).xyz,(r17.xyzx).xyz).xxxx).z;
    // 214: deriv_rtx_coarse r12.x, r5.z
    r12.x = (ddx_coarse(r5.zzzz)).x;
    // 215: deriv_rty_coarse r12.y, r5.z
    r12.y = (ddy_coarse(r5.zzzz)).y;
    // 216: dp2 r6.z, r12.xyxx, r12.xyxx
    r6.z = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).z;
    // 217: sqrt r6.z, r6.z
    r6.z = (sqrt(r6.zzzz)).z;
    // 218: mad r6.z, r6.z, l(0.300000), r12.z
    r6.z = ((r6.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r12.zzzz)).z;
    // 219: mov o2.zw, r12.zzzw
    output.targets[2].zw = (r12.zzzw).zw;
    // 220: min r12.y, r6.z, l(1.000000)
    r12.y = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 221: mad r6.z, r12.y, l(2.000000), l(2.000000)
    r6.z = ((r12.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).z;
    // 222: div r4.w, r4.w, r6.z
    r4.w = ((r4.wwww)/(r6.zzzz)).w;
    // 223: mad r4.w, r2.x, l(5.000000), r4.w
    r4.w = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r4.wwww)).w;
    // 224: add_sat r4.w, r12.w, r4.w
    r4.w = (saturate((r12.wwww)+(r4.wwww))).w;
    // 225: mad r7.w, r4.w, l(-2.000000), l(3.000000)
    r7.w = ((r4.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 226: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 227: mul r4.w, r4.w, r7.w
    r4.w = ((r4.wwww)*(r7.wwww)).w;
    // 228: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 229: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 230: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 231: mul r16.xyz, r4.wwww, r16.xyzx
    r16.xyz = ((r4.wwww)*(r16.xyzx)).xyz;
    // 232: mov_sat r1.w, cb0[26].w
    r1.w = (saturate(source[26].wwww)).w;
    // 233: mad r18.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r18.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 234: mul r4.w, r1.w, l(0.080000)
    r4.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 235: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 236: mad r18.xyz, r12.wwww, r18.xyzx, r4.wwww
    r18.xyz = ((r12.wwww)*(r18.xyzx)+(r4.wwww)).xyz;
    // 237: mul_sat r1.w, r18.y, l(50.000000)
    r1.w = (saturate((r18.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 238: add r4.w, -r12.y, l(1.000000)
    r4.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: max r19.xyz, r18.xyzx, r4.wwww
    r19.xyz = (max(r18.xyzx,r4.wwww)).xyz;
    // 240: add r19.xyz, -r18.xyzx, r19.xyzx
    r19.xyz = ((-(r18.xyzx))+(r19.xyzx)).xyz;
    // 241: mul r19.xyz, r1.wwww, r19.xyzx
    r19.xyz = ((r1.wwww)*(r19.xyzx)).xyz;
    // 242: mul r20.xyz, r5.zzzz, r5.xywx
    r20.xyz = ((r5.zzzz)*(r5.xywx)).xyz;
    // 243: mad r20.xyz, r20.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r17.xyzx
    r20.xyz = ((r20.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r17.xyzx))).xyz;
    // 244: add r1.w, r20.z, l(1.000000)
    r1.w = ((r20.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 245: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: add r4.w, r5.z, l(1.000000)
    r4.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 247: mov_sat r5.z, r5.z
    r5.z = (saturate(r5.zzzz)).z;
    // 248: log r5.z, r5.z
    r5.z = (log2(r5.zzzz)).z;
    // 249: mul r5.z, r5.z, cb0[2].y
    r5.z = ((r5.zzzz)*(source[2].yyyy)).z;
    // 250: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 251: mad_sat r5.z, r5.z, cb0[2].w, cb0[2].z
    r5.z = (saturate((r5.zzzz)*(source[2].wwww)+(source[2].zzzz))).z;
    // 252: mul r5.z, r5.z, cb0[29].z
    r5.z = ((r5.zzzz)*(source[29].zzzz)).z;
    // 253: add_sat r12.x, -r1.w, r4.w
    r12.x = (saturate((-(r1.wwww))+(r4.wwww))).x;
    // 254: sample_indexable(texture2d)(float,float,float,float) r21.xy, r12.xyxx, t7.xyzw, s8
    r21.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 255: add r1.w, r0.x, r12.x
    r1.w = ((r0.xxxx)+(r12.xxxx)).w;
    // 256: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 257: mul r22.xyz, r18.xyzx, r21.yyyy
    r22.xyz = ((r18.xyzx)*(r21.yyyy)).xyz;
    // 258: mad r19.xyz, r19.xyzx, r21.xxxx, r22.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xxxx)+(r22.xyzx)).xyz;
    // 259: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r21.y
    r4.w = r21.y != 0.f ? 1.f / r21.y : 0.f;
    // 260: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 261: mad r21.xyz, r18.xyzx, r4.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r21.xyz = ((r18.xyzx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 262: dp3 r4.w, r18.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r18.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: mad r18.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r18.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 264: mad r22.xyz, -r19.xyzx, r21.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r22.xyz = ((-(r19.xyzx))*(r21.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 265: mul r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xyzx)).xyz;
    // 266: mul r21.xyz, r1.xyzx, r22.xyzx
    r21.xyz = ((r1.xyzx)*(r22.xyzx)).xyz;
    // 267: mul r22.xyz, r16.xyzx, r22.xyzx
    r22.xyz = ((r16.xyzx)*(r22.xyzx)).xyz;
    // 268: add r4.w, -r12.w, l(1.000000)
    r4.w = ((-(r12.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 269: mul r21.xyz, r4.wwww, r21.xyzx
    r21.xyz = ((r4.wwww)*(r21.xyzx)).xyz;
    // 270: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 271: mad r21.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r21.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 272: mad r23.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r23.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 273: mad r24.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r24.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 274: mad r23.xyz, r0.xxxx, r23.xyzx, r24.xyzx
    r23.xyz = ((r0.xxxx)*(r23.xyzx)+(r24.xyzx)).xyz;
    // 275: mad r21.xyz, r23.xyzx, r0.xxxx, r21.xyzx
    r21.xyz = ((r23.xyzx)*(r0.xxxx)+(r21.xyzx)).xyz;
    // 276: mul r21.xyz, r0.xxxx, r21.xyzx
    r21.xyz = ((r0.xxxx)*(r21.xyzx)).xyz;
    // 277: max r21.xyz, r0.xxxx, r21.xyzx
    r21.xyz = (max(r0.xxxx,r21.xyzx)).xyz;
    // 278: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 279: dp3 r14.y, r14.xyzx, r20.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 280: dp3 r14.x, r13.xyzx, r20.xyzx
    r14.x = (dot((r13.xyzx).xyz,(r20.xyzx).xyz).xxxx).x;
    // 281: dp2 r13.x, r14.xyxx, r2.yzyy
    r13.x = (dot((r14.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 282: dp2 r13.z, r14.xyxx, cb0[31].xyxx
    r13.z = (dot((r14.xyxx).xy,(source[31].xyxx).xy).xxxx).z;
    // 283: mul r2.y, r12.y, l(5.000000)
    r2.y = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 284: mul r2.z, r12.y, r12.y
    r2.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 285: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 286: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 287: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 288: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 289: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 290: dp3 r13.y, r6.xywx, r20.xyzx
    r13.y = (dot((r6.xywx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 291: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r13.xyzx, t8.xyzw, s7, r2.y
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r13.xyzx).xyz, (r2.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 292: mul r6.xyw, r13.xyxz, r13.wwww
    r6.xyw = ((r13.xyxz)*(r13.wwww)).xyw;
    // 293: mul r6.xyw, r6.xyxw, cb0[30].xyxz
    r6.xyw = ((r6.xyxw)*(source[30].xyxz)).xyw;
    // 294: mul r6.xyw, r6.xyxw, cb0[31].zzzz
    r6.xyw = ((r6.xyxw)*(source[31].zzzz)).xyw;
    // 295: mad r6.xyw, r6.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[30].wwww
    r6.xyw = ((r6.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[30].wwww)).xyw;
    // 296: dp3 r1.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 297: add r6.xyw, -r1.wwww, r6.xyxw
    r6.xyw = ((-(r1.wwww))+(r6.xyxw)).xyw;
    // 298: mad r6.xyw, r6.xyxw, l(0.800000, 0.800000, 0.000000, 0.800000), r1.wwww
    r6.xyw = ((r6.xyxw)*(float4(0.800000,0.800000,0.000000,0.800000))+(r1.wwww)).xyw;
    // 299: dp3 r1.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 300: div r1.w, r1.w, r6.z
    r1.w = ((r1.wwww)/(r6.zzzz)).w;
    // 301: mad r1.w, r2.x, l(5.000000), r1.w
    r1.w = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 302: add_sat r1.w, r12.w, r1.w
    r1.w = (saturate((r12.wwww)+(r1.wwww))).w;
    // 303: mad r2.x, r1.w, l(-2.000000), l(3.000000)
    r2.x = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 304: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 305: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 306: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 307: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 308: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 309: mul r2.xyz, r1.wwww, r6.xywx
    r2.xyz = ((r1.wwww)*(r6.xywx)).xyz;
    // 310: mul r6.xyz, r2.xyzx, r19.xyzx
    r6.xyz = ((r2.xyzx)*(r19.xyzx)).xyz;
    // 311: mad r1.w, r0.x, r18.x, r18.y
    r1.w = ((r0.xxxx)*(r18.xxxx)+(r18.yyyy)).w;
    // 312: mad r1.w, r1.w, r0.x, r18.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r18.zzzz)).w;
    // 313: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 314: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 315: mad r6.xyz, r6.xyzx, r0.xxxx, r16.xyzx
    r6.xyz = ((r6.xyzx)*(r0.xxxx)+(r16.xyzx)).xyz;
    // 316: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 317: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 318: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 319: dp3 r1.w, r12.xyzx, r5.xywx
    r1.w = (dot((r12.xyzx).xyz,(r5.xywx).xyz).xxxx).w;
    // 320: dp3 r5.x, -r12.xyzx, r5.xywx
    r5.x = (dot((-(r12.xyzx)).xyz,(r5.xywx).xyz).xxxx).x;
    // 321: dp3 r5.y, r12.xyzx, r20.xyzx
    r5.y = (dot((r12.xyzx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 322: mad r5.yw, r5.yyyy, l(0.000000, 0.500000, 0.000000, -0.500000), l(0.000000, 0.500000, 0.000000, 0.500000)
    r5.yw = ((r5.yyyy)*(float4(0.000000,0.500000,0.000000,-0.500000))+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 323: mul r5.yw, r5.yyyw, r5.yyyw
    r5.yw = ((r5.yyyw)*(r5.yyyw)).yw;
    // 324: mad r12.xy, r5.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r5.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 325: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 326: mad r13.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r13.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 327: mul r13.xy, r13.xyxx, r13.xyxx
    r13.xy = ((r13.xyxx)*(r13.xyxx)).xy;
    // 328: mul r13.yzw, r13.yyyy, cb0[41].xxyz
    r13.yzw = ((r13.yyyy)*(source[41].xxyz)).yzw;
    // 329: mad r13.xyz, r13.xxxx, cb0[40].xyzx, r13.yzwy
    r13.xyz = ((r13.xxxx)*(source[40].xyzx)+(r13.yzwy)).xyz;
    // 330: mul r13.xyz, r13.xyzx, cb0[42].wwww
    r13.xyz = ((r13.xyzx)*(source[42].wwww)).xyz;
    // 331: mul r13.xyz, r1.xyzx, r13.xyzx
    r13.xyz = ((r1.xyzx)*(r13.xyzx)).xyz;
    // 332: mul r13.xyz, r21.xyzx, r13.xyzx
    r13.xyz = ((r21.xyzx)*(r13.xyzx)).xyz;
    // 333: mul r13.xyz, r13.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 334: mul r13.xyz, r22.xyzx, r13.xyzx
    r13.xyz = ((r22.xyzx)*(r13.xyzx)).xyz;
    // 335: mad r13.xyz, -r13.xyzx, r12.wwww, r13.xyzx
    r13.xyz = ((-(r13.xyzx))*(r12.wwww)+(r13.xyzx)).xyz;
    // 336: mad r6.xyz, r6.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r13.xyzx
    r6.xyz = ((r6.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r13.xyzx)).xyz;
    // 337: mul r13.xyz, r5.wwww, cb0[41].xyzx
    r13.xyz = ((r5.wwww)*(source[41].xyzx)).xyz;
    // 338: mad r5.xyw, cb0[40].xyxz, r5.yyyy, r13.xyxz
    r5.xyw = ((source[40].xyxz)*(r5.yyyy)+(r13.xyxz)).xyw;
    // 339: mul r5.xyw, r5.xyxw, cb0[42].wwww
    r5.xyw = ((r5.xyxw)*(source[42].wwww)).xyw;
    // 340: mul r5.xyw, r0.xxxx, r5.xyxw
    r5.xyw = ((r0.xxxx)*(r5.xyxw)).xyw;
    // 341: mul r2.xyz, r2.xyzx, r5.xywx
    r2.xyz = ((r2.xyzx)*(r5.xywx)).xyz;
    // 342: mul r2.xyz, r2.xyzx, r19.xyzx
    r2.xyz = ((r2.xyzx)*(r19.xyzx)).xyz;
    // 343: mad r5.xyw, r2.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r6.xyxz
    r5.xyw = ((r2.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r6.xyxz)).xyw;
    // 344: mul r2.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 345: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 346: mad r2.xyz, -r8.xxxx, r4.xyzx, r7.xyzx
    r2.xyz = ((-(r8.xxxx))*(r4.xyzx)+(r7.xyzx)).xyz;
    // 347: mul r4.xyz, r4.xyzx, r8.xxxx
    r4.xyz = ((r4.xyzx)*(r8.xxxx)).xyz;
    // 348: mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 349: add r4.xyz, -r2.xyzx, r9.xyzx
    r4.xyz = ((-(r2.xyzx))+(r9.xyzx)).xyz;
    // 350: mad r2.xyz, r8.yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((r8.yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 351: add r4.xyz, -r2.xyzx, r10.xyzx
    r4.xyz = ((-(r2.xyzx))+(r10.xyzx)).xyz;
    // 352: mad r2.xyz, r8.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r8.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 353: add r4.xyz, -r2.xyzx, r7.xyzx
    r4.xyz = ((-(r2.xyzx))+(r7.xyzx)).xyz;
    // 354: mad r2.xyz, r2.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 355: add r2.xyz, r2.xyzx, -cb0[10].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[10].xyzx))).xyz;
    // 356: mad r2.xyz, r3.wwww, r2.xyzx, cb0[10].xyzx
    r2.xyz = ((r3.wwww)*(r2.xyzx)+(source[10].xyzx)).xyz;
    // 357: mul r2.xyz, r2.xyzx, cb0[22].yyyy
    r2.xyz = ((r2.xyzx)*(source[22].yyyy)).xyz;
    // 358: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 359: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 360: add r0.x, cb0[0].y, cb0[0].x
    r0.x = ((source[0].yyyy)+(source[0].xxxx)).x;
    // 361: add r0.x, r0.x, cb0[0].z
    r0.x = ((r0.xxxx)+(source[0].zzzz)).x;
    // 362: add r1.w, -r0.x, l(1000.000000)
    r1.w = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 363: mad r0.x, cb0[23].y, r1.w, r0.x
    r0.x = ((source[23].yyyy)*(r1.wwww)+(r0.xxxx)).x;
    // 364: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 365: mad r0.x, cb0[22].w, cb0[23].x, r0.x
    r0.x = ((source[22].wwww)*(source[23].xxxx)+(r0.xxxx)).x;
    // 366: mul r1.w, r0.x, l(3.524534)
    r1.w = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 367: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 368: add r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)+(r1.wwww)).x;
    // 369: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 370: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 371: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 372: mad r0.x, r0.x, l(0.500000), cb0[22].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[22].zzzz)).x;
    // 373: dp3 r1.w, r3.xyzx, r17.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r17.xyzx).xyz).xxxx).w;
    // 374: mul_sat r2.w, r1.w, cb0[24].x
    r2.w = (saturate((r1.wwww)*(source[24].xxxx))).w;
    // 375: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 376: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 377: mul_sat r3.x, r17.z, cb0[24].x
    r3.x = (saturate((r17.zzzz)*(source[24].xxxx))).x;
    // 378: add r3.y, -|r17.z|, l(1.000000)
    r3.y = ((-(abs(r17.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 379: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 380: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 381: add_sat r3.x, r3.x, -cb0[24].y
    r3.x = (saturate((r3.xxxx)+(-(source[24].yyyy)))).x;
    // 382: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 383: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 384: mul r3.y, r3.y, cb0[24].z
    r3.y = ((r3.yyyy)*(source[24].zzzz)).y;
    // 385: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 386: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 387: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 388: mul r3.xy, v4.wzww, cb0[11].zzzz
    r3.xy = ((v4.wzww)*(source[11].zzzz)).xy;
    // 389: mul r3.zw, cb0[11].xxxy, cb0[23].xxxx
    r3.zw = ((source[11].xxxy)*(source[23].xxxx)).zw;
    // 390: mad r3.xy, r3.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r3.xyxx
    r3.xy = ((r3.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r3.xyxx)).xy;
    // 391: mad r3.zw, cb0[11].zzzz, v4.wwwz, r3.zzzw
    r3.zw = ((source[11].zzzz)*(v4.wwwz)+(r3.zzzw)).zw;
    // 392: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r3.xyxx, t6.xyzw, s5, l(0.000000)
    r3.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 393: mad r3.xy, r3.xxxx, cb0[23].zzzz, r3.zwzz
    r3.xy = ((r3.xxxx)*(source[23].zzzz)+(r3.zwzz)).xy;
    // 394: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t6.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 395: mul r3.xyz, r3.xyzx, r6.wwww
    r3.xyz = ((r3.xyzx)*(r6.wwww)).xyz;
    // 396: mul r4.xyz, cb0[12].xyzx, cb0[23].wwww
    r4.xyz = ((source[12].xyzx)*(source[23].wwww)).xyz;
    // 397: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 398: mad r4.xyz, r2.wwww, r3.xyzx, -r3.xyzx
    r4.xyz = ((r2.wwww)*(r3.xyzx)+(-(r3.xyzx))).xyz;
    // 399: mad r3.xyz, cb0[12].wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((source[12].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 400: mad r2.xyz, r0.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 401: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 402: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 403: mad r2.xyz, cb0[24].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[24].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 404: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 405: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 406: mad r2.xyz, cb0[25].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 407: add r0.x, -r8.w, r2.w
    r0.x = ((-(r8.wwww))+(r2.wwww)).x;
    // 408: mad r3.xyz, r2.wwww, cb0[16].xyzx, -cb0[16].xyzx
    r3.xyz = ((r2.wwww)*(source[16].xyzx)+(-(source[16].xyzx))).xyz;
    // 409: mad r3.xyz, cb0[16].wwww, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((source[16].wwww)*(r3.xyzx)+(source[16].xyzx)).xyz;
    // 410: mad r0.x, cb0[15].w, r0.x, r8.w
    r0.x = ((source[15].wwww)*(r0.xxxx)+(r8.wwww)).x;
    // 411: mad r3.xyz, r0.xxxx, cb0[15].xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(source[15].xyzx)+(r3.xyzx)).xyz;
    // 412: mad r2.xyz, r2.xyzx, r11.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 413: log r0.x, |r1.w|
    r0.x = (log2(abs(r1.wwww))).x;
    // 414: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 415: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 416: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 417: mul r3.xyz, r0.xxxx, cb0[17].xyzx
    r3.xyz = ((r0.xxxx)*(source[17].xyzx)).xyz;
    // 418: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 419: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 420: mad r0.xyz, cb0[22].xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((source[22].xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 421: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 422: mul r2.xyz, r12.yyyy, cb0[41].xyzx
    r2.xyz = ((r12.yyyy)*(source[41].xyzx)).xyz;
    // 423: mad r2.xyz, r12.xxxx, cb0[40].xyzx, r2.xyzx
    r2.xyz = ((r12.xxxx)*(source[40].xyzx)+(r2.xyzx)).xyz;
    // 424: mul r2.xyz, r2.xyzx, cb0[42].wwww
    r2.xyz = ((r2.xyzx)*(source[42].wwww)).xyz;
    // 425: mul_sat r3.xyz, cb0[20].xyzx, cb0[20].wwww
    r3.xyz = (saturate((source[20].xyzx)*(source[20].wwww))).xyz;
    // 426: mul r4.xyz, r3.xyzx, r5.zzzz
    r4.xyz = ((r3.xyzx)*(r5.zzzz)).xyz;
    // 427: mul r3.xyz, r3.xyzx, cb0[29].zzzz
    r3.xyz = ((r3.xyzx)*(source[29].zzzz)).xyz;
    // 428: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 429: mul r3.xyz, r4.wwww, r4.xyzx
    r3.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 430: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 431: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 432: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 433: add r0.xyz, r5.xywx, r0.xyzx
    r0.xyz = ((r5.xywx)+(r0.xyzx)).xyz;
    // 434: dp3 o4.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 435: mad o0.xyz, r1.xyzx, cb0[42].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[42].xyzx)+(r0.xyzx)).xyz;
    // 436: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 437: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 438: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 439: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 440: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 441: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 442: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 443: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 444: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 445: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 446: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 447: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 448: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 449: ftou r0.x, cb0[39].z
    r0.x = (asfloat((uint4)(source[39].zzzz))).x;
    // 450: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 451: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 452: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 453: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 454: ret
    return output;
}

// source.character.equipment-native-185.v1 / source program 4f1e52973a41f34eafd41ec567978b91
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase185(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[21].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[28]=g_SourceCharacterEnvironmentColor; source[29]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0, r22=0.0, r23=0.0, r24=0.0;
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
    // 7: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 8: mul r1.x, r0.x, l(0.125000)
    r1.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 9: mul r2.y, cb0[8].y, cb0[18].y
    r2.y = ((source[8].yyyy)*(source[18].yyyy)).y;
    // 10: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 11: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 12: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 13: frc r0.x, cb0[8].x
    r0.x = (frac(source[8].xxxx)).x;
    // 14: add r1.z, -r0.x, cb0[8].x
    r1.z = ((-(r0.xxxx))+(source[8].xxxx)).z;
    // 15: mul r2.z, r1.z, l(0.125000)
    r2.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 16: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t4.xyzw, s6, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 19: add r1.w, -cb0[8].w, l(1.000000)
    r1.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mul r1.w, r1.w, cb0[21].z
    r1.w = ((r1.wwww)*(source[21].zzzz)).w;
    // 21: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 22: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 23: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r2.x, cb0[8].z, l(1.500000)
    r2.x = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mad r1.w, r1.w, l(0.500000), cb0[8].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 27: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 28: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 30: mad r2.xyz, cb0[23].xxxx, r2.xyzx, r0.yzwy
    r2.xyz = ((source[23].xxxx)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 33: mad r2.xyz, cb0[23].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[23].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.w, r6.y, cb0[20].y
    r1.w = ((r6.yyyy)*(source[20].yyyy)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[4].xyzx, cb0[4].wwww
    r4.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: add r7.xyz, -r3.xyzx, r4.xyzx
    r7.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 56: round_ni r5.yw, v7.xxxy
    r5.yw = (floor(v7.xxxy)).yw;
    // 57: dp2 r2.w, r5.ywyy, l(12.989800, 78.233002, 0.000000, 0.000000)
    r2.w = (dot((r5.ywyy).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).w;
    // 58: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 59: mul r2.w, r2.w, l(43758.546875)
    r2.w = ((r2.wwww)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).w;
    // 60: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 61: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 63: mad r2.w, r2.w, l(0.010000), r8.x
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(r8.xxxx)).w;
    // 64: lt r3.w, cb0[20].w, r2.w
    r3.w = (asfloat((uint4)((source[20].wwww)<(r2.wwww)) * 0xffffffffu)).w;
    // 65: lt r2.w, r2.w, cb0[20].z
    r2.w = (asfloat((uint4)((r2.wwww)<(source[20].zzzz)) * 0xffffffffu)).w;
    // 66: movc r2.w, r2.w, l(-1.000000), l(-0.000000)
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).w;
    // 67: and r4.w, r3.w, l(0x3f800000)
    r4.w = (asfloat(asuint(r3.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 68: movc r3.w, r3.w, l(0), l(1.000000)
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: add r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)+(r3.wwww)).w;
    // 70: mad r3.xyz, r4.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r4.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 71: add r3.w, r8.y, r4.w
    r3.w = ((r8.yyyy)+(r4.wwww)).w;
    // 72: add r3.w, r8.z, r3.w
    r3.w = ((r8.zzzz)+(r3.wwww)).w;
    // 73: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 74: mul r7.xyz, cb0[5].xyzx, cb0[5].wwww
    r7.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 75: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 76: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: max r9.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 78: add r9.xyz, -r7.xyzx, r9.xyzx
    r9.xyz = ((-(r7.xyzx))+(r9.xyzx)).xyz;
    // 79: mad r7.xyz, r1.wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 80: add r9.xyz, -r3.xyzx, r7.xyzx
    r9.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 81: mad r3.xyz, r2.wwww, r9.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r9.xyz, cb0[6].xyzx, cb0[6].wwww
    r9.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 83: max r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 84: max r9.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 85: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 86: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 87: add r10.xyz, -r9.xyzx, r10.xyzx
    r10.xyz = ((-(r9.xyzx))+(r10.xyzx)).xyz;
    // 88: mad r9.xyz, r1.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r1.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 89: add r10.xyz, -r3.xyzx, r9.xyzx
    r10.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 90: mad r3.xyz, r8.yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((r8.yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 91: mul r10.xyz, cb0[7].xyzx, cb0[7].wwww
    r10.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 92: max r11.xyz, r10.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r11.xyz = (max(r10.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 93: max r10.xyz, r10.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 94: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 95: min r11.xyz, r11.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r11.xyz = (min(r11.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 96: add r11.xyz, -r10.xyzx, r11.xyzx
    r11.xyz = ((-(r10.xyzx))+(r11.xyzx)).xyz;
    // 97: mad r10.xyz, r1.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r1.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 98: add r11.xyz, -r3.xyzx, r10.xyzx
    r11.xyz = ((-(r3.xyzx))+(r10.xyzx)).xyz;
    // 99: mad r3.xyz, r8.zzzz, r11.xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 100: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: add r11.xyz, -r3.xyzx, r4.wwww
    r11.xyz = ((-(r3.xyzx))+(r4.wwww)).xyz;
    // 102: mad r11.xyz, cb0[23].xxxx, r11.xyzx, r3.xyzx
    r11.xyz = ((source[23].xxxx)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 103: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 104: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 105: add r3.xyz, -r11.xyzx, r3.xxxx
    r3.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 106: mad r3.xyz, cb0[23].yyyy, r3.xyzx, r11.xyzx
    r3.xyz = ((source[23].yyyy)*(r3.xyzx)+(r11.xyzx)).xyz;
    // 107: mad r11.xyz, cb0[12].wwww, cb0[12].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[12].wwww)*(source[12].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 108: mad r12.xyz, cb0[13].wwww, cb0[13].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[13].wwww)*(source[13].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 109: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 110: mul r3.xyz, r3.xyzx, r11.xyzx
    r3.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 111: mul r12.xyz, r2.xyzx, r3.xyzx
    r12.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 112: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: mad r2.xyz, -r3.xyzx, r2.xyzx, r4.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r4.wwww)).xyz;
    // 114: mad r2.xyz, cb0[23].xxxx, r2.xyzx, r12.xyzx
    r2.xyz = ((source[23].xxxx)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 115: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: add r3.xyz, -r2.xyzx, r3.xxxx
    r3.xyz = ((-(r2.xyzx))+(r3.xxxx)).xyz;
    // 117: mad r2.xyz, cb0[23].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[23].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 118: mul r2.xyz, r11.xyzx, r2.xyzx
    r2.xyz = ((r11.xyzx)*(r2.xyzx)).xyz;
    // 119: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 120: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 121: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 122: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 123: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 124: max r0.x, r0.x, cb0[24].y
    r0.x = (max(r0.xxxx,source[24].yyyy)).x;
    // 125: min r0.x, r0.x, cb0[24].x
    r0.x = (min(r0.xxxx,source[24].xxxx)).x;
    // 126: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 127: mad r0.x, r1.w, r2.x, r0.x
    r0.x = ((r1.wwww)*(r2.xxxx)+(r0.xxxx)).x;
    // 128: mul_sat r12.w, r1.w, cb2[3].w
    r12.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 129: add r1.w, r0.x, l(-1.000000)
    r1.w = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 130: mad r1.w, cb0[24].w, r1.w, l(1.000000)
    r1.w = ((source[24].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r2.xyz, r1.xyzx, r1.wwww
    r2.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 132: mul r3.x, r6.x, cb0[23].z
    r3.x = ((r6.xxxx)*(source[23].zzzz)).x;
    // 133: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 134: movc r3.x, r5.x, l(0), r3.x
    r3.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 135: add_sat r3.x, r3.x, cb0[23].w
    r3.x = (saturate((r3.xxxx)+(source[23].wwww))).x;
    // 136: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: mul r5.xyw, r3.yyyy, cb0[17].xyxz
    r5.xyw = ((r3.yyyy)*(source[17].xyxz)).xyw;
    // 138: mul r2.xyz, r2.xyzx, r5.xywx
    r2.xyz = ((r2.xyzx)*(r5.xywx)).xyz;
    // 139: mad r1.xyz, r1.wwww, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 140: mad r1.xyz, r3.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 141: mul_sat r0.x, r0.x, r3.x
    r0.x = (saturate((r0.xxxx)*(r3.xxxx))).x;
    // 142: add r2.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 143: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 144: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 145: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 147: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 148: dp2 r3.x, r2.yzyy, r2.yzyy
    r3.x = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).x;
    // 149: mul r13.xy, r2.yzyy, cb0[20].xxxx
    r13.xy = ((r2.yzyy)*(source[20].xxxx)).xy;
    // 150: add r2.y, -r3.x, l(1.000000)
    r2.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 151: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 152: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 153: add r13.z, r2.y, l(0.000010)
    r13.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 154: dp3 r2.y, r13.xyzx, r13.xyzx
    r2.y = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 155: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 156: div r3.xyz, r13.xyzx, r2.yyyy
    r3.xyz = ((r13.xyzx)/(r2.yyyy)).xyz;
    // 157: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 158: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 159: mul r5.xyw, r2.yyyy, r3.xyxz
    r5.xyw = ((r2.yyyy)*(r3.xyxz)).xyw;
    // 160: dp3 r2.y, v1.xyzx, v1.xyzx
    r2.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 161: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 162: mul r6.xyw, r2.yyyy, v1.xyxz
    r6.xyw = ((r2.yyyy)*(v1.xyxz)).xyw;
    // 163: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 164: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 165: mul r13.xyz, r2.yyyy, v0.xyzx
    r13.xyz = ((r2.yyyy)*(v0.xyzx)).xyz;
    // 166: mul r14.xyz, r6.wxyw, r13.yzxy
    r14.xyz = ((r6.wxyw)*(r13.yzxy)).xyz;
    // 167: mad r14.xyz, r6.ywxy, r13.zxyz, -r14.xyzx
    r14.xyz = ((r6.ywxy)*(r13.zxyz)+(-(r14.xyzx))).xyz;
    // 168: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 169: dp3 r15.y, r14.xyzx, r5.xywx
    r15.y = (dot((r14.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 170: dp3 r15.x, r13.xyzx, r5.xywx
    r15.x = (dot((r13.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 171: dp2 r16.z, r15.xyxx, cb0[29].xyxx
    r16.z = (dot((r15.xyxx).xy,(source[29].xyxx).xy).xxxx).z;
    // 172: mul r2.yz, cb0[29].yyxy, l(0.000000, 1.000000, -1.000000, 0.000000)
    r2.yz = ((source[29].yyxy)*(float4(0.000000,1.000000,-1.000000,0.000000))).yz;
    // 173: dp2 r16.x, r15.xyxx, r2.yzyy
    r16.x = (dot((r15.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 174: dp3 r16.y, r6.xywx, r5.xywx
    r16.y = (dot((r6.xywx).xyz,(r5.xywx).xyz).xxxx).y;
    // 175: mov r16.w, l(1.000000)
    r16.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 176: dp4 r17.x, cb0[30].xyzw, r16.xyzw
    r17.x = (dot((source[30].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 177: dp4 r17.y, cb0[31].xyzw, r16.xyzw
    r17.y = (dot((source[31].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 178: dp4 r17.z, cb0[32].xyzw, r16.xyzw
    r17.z = (dot((source[32].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 179: mul r18.xyzw, r16.yzzx, r16.xyzz
    r18.xyzw = ((r16.yzzx)*(r16.xyzz)).xyzw;
    // 180: dp4 r19.x, cb0[33].xyzw, r18.xyzw
    r19.x = (dot((source[33].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).x;
    // 181: dp4 r19.y, cb0[34].xyzw, r18.xyzw
    r19.y = (dot((source[34].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).y;
    // 182: dp4 r19.z, cb0[35].xyzw, r18.xyzw
    r19.z = (dot((source[35].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).z;
    // 183: add r17.xyz, r17.xyzx, r19.xyzx
    r17.xyz = ((r17.xyzx)+(r19.xyzx)).xyz;
    // 184: mul r4.w, r16.y, r16.y
    r4.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 185: mov r15.z, r16.y
    r15.z = (r16.yyyy).z;
    // 186: mad r4.w, r16.x, r16.x, -r4.w
    r4.w = ((r16.xxxx)*(r16.xxxx)+(-(r4.wwww))).w;
    // 187: mad r16.xyz, cb0[36].xyzx, r4.wwww, r17.xyzx
    r16.xyz = ((source[36].xyzx)*(r4.wwww)+(r17.xyzx)).xyz;
    // 188: max r16.xyz, r16.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r16.xyz = (max(r16.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 189: mul r16.xyz, r16.xyzx, cb0[28].xyzx
    r16.xyz = ((r16.xyzx)*(source[28].xyzx)).xyz;
    // 190: mul r16.xyz, r16.xyzx, cb0[29].zzzz
    r16.xyz = ((r16.xyzx)*(source[29].zzzz)).xyz;
    // 191: mad r16.xyz, r16.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[28].wwww
    r16.xyz = ((r16.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[28].wwww)).xyz;
    // 192: dp3 r4.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 193: add r16.xyz, -r4.wwww, r16.xyzx
    r16.xyz = ((-(r4.wwww))+(r16.xyzx)).xyz;
    // 194: mad r16.xyz, r16.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r4.wwww
    r16.xyz = ((r16.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r4.wwww)).xyz;
    // 195: dp3 r4.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: add r7.w, cb0[25].w, -cb0[26].x
    r7.w = ((source[25].wwww)+(-(source[26].xxxx))).w;
    // 197: mad r7.w, r8.x, r7.w, cb0[26].x
    r7.w = ((r8.xxxx)*(r7.wwww)+(source[26].xxxx)).w;
    // 198: add r9.w, -r7.w, cb0[26].z
    r9.w = ((-(r7.wwww))+(source[26].zzzz)).w;
    // 199: mad r7.w, r8.y, r9.w, r7.w
    r7.w = ((r8.yyyy)*(r9.wwww)+(r7.wwww)).w;
    // 200: add r9.w, -r7.w, cb0[27].x
    r9.w = ((-(r7.wwww))+(source[27].xxxx)).w;
    // 201: mad r7.w, r8.z, r9.w, r7.w
    r7.w = ((r8.zzzz)*(r9.wwww)+(r7.wwww)).w;
    // 202: add r9.w, -r7.w, cb0[27].z
    r9.w = ((-(r7.wwww))+(source[27].zzzz)).w;
    // 203: mad r7.w, r2.w, r9.w, r7.w
    r7.w = ((r2.wwww)*(r9.wwww)+(r7.wwww)).w;
    // 204: mul r6.z, r6.z, r7.w
    r6.z = ((r6.zzzz)*(r7.wwww)).z;
    // 205: exp r6.z, r6.z
    r6.z = (exp2(r6.zzzz)).z;
    // 206: min r6.z, r6.z, l(1.000000)
    r6.z = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: movc r5.z, r5.z, l(0), r6.z
    r5.z = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.zzzz)).z;
    // 208: max r5.z, r5.z, cb0[0].x
    r5.z = (max(r5.zzzz,source[0].xxxx)).z;
    // 209: min r12.z, r5.z, l(1.000000)
    r12.z = (min(r5.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 210: dp3 r5.z, v5.xyzx, v5.xyzx
    r5.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 211: rsq r5.z, r5.z
    r5.z = (rsqrt(r5.zzzz)).z;
    // 212: mul r17.xyz, r5.zzzz, v5.xyzx
    r17.xyz = ((r5.zzzz)*(v5.xyzx)).xyz;
    // 213: dp3 r5.z, r5.xywx, r17.xyzx
    r5.z = (dot((r5.xywx).xyz,(r17.xyzx).xyz).xxxx).z;
    // 214: deriv_rtx_coarse r12.x, r5.z
    r12.x = (ddx_coarse(r5.zzzz)).x;
    // 215: deriv_rty_coarse r12.y, r5.z
    r12.y = (ddy_coarse(r5.zzzz)).y;
    // 216: dp2 r6.z, r12.xyxx, r12.xyxx
    r6.z = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).z;
    // 217: sqrt r6.z, r6.z
    r6.z = (sqrt(r6.zzzz)).z;
    // 218: mad r6.z, r6.z, l(0.300000), r12.z
    r6.z = ((r6.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r12.zzzz)).z;
    // 219: mov o2.zw, r12.zzzw
    output.targets[2].zw = (r12.zzzw).zw;
    // 220: min r12.y, r6.z, l(1.000000)
    r12.y = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 221: mad r6.z, r12.y, l(2.000000), l(2.000000)
    r6.z = ((r12.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).z;
    // 222: div r4.w, r4.w, r6.z
    r4.w = ((r4.wwww)/(r6.zzzz)).w;
    // 223: mad r4.w, r2.x, l(5.000000), r4.w
    r4.w = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r4.wwww)).w;
    // 224: add_sat r4.w, r12.w, r4.w
    r4.w = (saturate((r12.wwww)+(r4.wwww))).w;
    // 225: mad r7.w, r4.w, l(-2.000000), l(3.000000)
    r7.w = ((r4.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 226: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 227: mul r4.w, r4.w, r7.w
    r4.w = ((r4.wwww)*(r7.wwww)).w;
    // 228: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 229: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 230: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 231: mul r16.xyz, r4.wwww, r16.xyzx
    r16.xyz = ((r4.wwww)*(r16.xyzx)).xyz;
    // 232: mov_sat r1.w, cb0[25].x
    r1.w = (saturate(source[25].xxxx)).w;
    // 233: mad r18.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r18.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 234: mul r4.w, r1.w, l(0.080000)
    r4.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 235: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 236: mad r18.xyz, r12.wwww, r18.xyzx, r4.wwww
    r18.xyz = ((r12.wwww)*(r18.xyzx)+(r4.wwww)).xyz;
    // 237: mul_sat r1.w, r18.y, l(50.000000)
    r1.w = (saturate((r18.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 238: add r4.w, -r12.y, l(1.000000)
    r4.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: max r19.xyz, r18.xyzx, r4.wwww
    r19.xyz = (max(r18.xyzx,r4.wwww)).xyz;
    // 240: add r19.xyz, -r18.xyzx, r19.xyzx
    r19.xyz = ((-(r18.xyzx))+(r19.xyzx)).xyz;
    // 241: mul r19.xyz, r1.wwww, r19.xyzx
    r19.xyz = ((r1.wwww)*(r19.xyzx)).xyz;
    // 242: mul r20.xyz, r5.zzzz, r5.xywx
    r20.xyz = ((r5.zzzz)*(r5.xywx)).xyz;
    // 243: mad r20.xyz, r20.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r17.xyzx
    r20.xyz = ((r20.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r17.xyzx))).xyz;
    // 244: add r1.w, r20.z, l(1.000000)
    r1.w = ((r20.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 245: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: add r4.w, r5.z, l(1.000000)
    r4.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 247: mov_sat r5.z, r5.z
    r5.z = (saturate(r5.zzzz)).z;
    // 248: log r5.z, r5.z
    r5.z = (log2(r5.zzzz)).z;
    // 249: mul r5.z, r5.z, cb0[1].y
    r5.z = ((r5.zzzz)*(source[1].yyyy)).z;
    // 250: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 251: mad_sat r5.z, r5.z, cb0[1].w, cb0[1].z
    r5.z = (saturate((r5.zzzz)*(source[1].wwww)+(source[1].zzzz))).z;
    // 252: mul r5.z, r5.z, cb0[27].w
    r5.z = ((r5.zzzz)*(source[27].wwww)).z;
    // 253: add_sat r12.x, -r1.w, r4.w
    r12.x = (saturate((-(r1.wwww))+(r4.wwww))).x;
    // 254: sample_indexable(texture2d)(float,float,float,float) r21.xy, r12.xyxx, t7.xyzw, s8
    r21.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 255: add r1.w, r0.x, r12.x
    r1.w = ((r0.xxxx)+(r12.xxxx)).w;
    // 256: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 257: mul r22.xyz, r18.xyzx, r21.yyyy
    r22.xyz = ((r18.xyzx)*(r21.yyyy)).xyz;
    // 258: mad r19.xyz, r19.xyzx, r21.xxxx, r22.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xxxx)+(r22.xyzx)).xyz;
    // 259: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r21.y
    r4.w = r21.y != 0.f ? 1.f / r21.y : 0.f;
    // 260: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 261: mad r21.xyz, r18.xyzx, r4.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r21.xyz = ((r18.xyzx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 262: dp3 r4.w, r18.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r18.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: mad r18.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r18.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 264: mad r22.xyz, -r19.xyzx, r21.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r22.xyz = ((-(r19.xyzx))*(r21.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 265: mul r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xyzx)).xyz;
    // 266: mul r21.xyz, r1.xyzx, r22.xyzx
    r21.xyz = ((r1.xyzx)*(r22.xyzx)).xyz;
    // 267: mul r22.xyz, r16.xyzx, r22.xyzx
    r22.xyz = ((r16.xyzx)*(r22.xyzx)).xyz;
    // 268: add r4.w, -r12.w, l(1.000000)
    r4.w = ((-(r12.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 269: mul r21.xyz, r4.wwww, r21.xyzx
    r21.xyz = ((r4.wwww)*(r21.xyzx)).xyz;
    // 270: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 271: mad r21.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r21.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 272: mad r23.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r23.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 273: mad r24.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r24.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 274: mad r23.xyz, r0.xxxx, r23.xyzx, r24.xyzx
    r23.xyz = ((r0.xxxx)*(r23.xyzx)+(r24.xyzx)).xyz;
    // 275: mad r21.xyz, r23.xyzx, r0.xxxx, r21.xyzx
    r21.xyz = ((r23.xyzx)*(r0.xxxx)+(r21.xyzx)).xyz;
    // 276: mul r21.xyz, r0.xxxx, r21.xyzx
    r21.xyz = ((r0.xxxx)*(r21.xyzx)).xyz;
    // 277: max r21.xyz, r0.xxxx, r21.xyzx
    r21.xyz = (max(r0.xxxx,r21.xyzx)).xyz;
    // 278: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 279: dp3 r14.y, r14.xyzx, r20.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 280: dp3 r14.x, r13.xyzx, r20.xyzx
    r14.x = (dot((r13.xyzx).xyz,(r20.xyzx).xyz).xxxx).x;
    // 281: dp2 r13.x, r14.xyxx, r2.yzyy
    r13.x = (dot((r14.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 282: dp2 r13.z, r14.xyxx, cb0[29].xyxx
    r13.z = (dot((r14.xyxx).xy,(source[29].xyxx).xy).xxxx).z;
    // 283: mul r2.y, r12.y, l(5.000000)
    r2.y = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 284: mul r2.z, r12.y, r12.y
    r2.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 285: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 286: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 287: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 288: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 289: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 290: dp3 r13.y, r6.xywx, r20.xyzx
    r13.y = (dot((r6.xywx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 291: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r13.xyzx, t8.xyzw, s7, r2.y
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r13.xyzx).xyz, (r2.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 292: mul r6.xyw, r13.xyxz, r13.wwww
    r6.xyw = ((r13.xyxz)*(r13.wwww)).xyw;
    // 293: mul r6.xyw, r6.xyxw, cb0[28].xyxz
    r6.xyw = ((r6.xyxw)*(source[28].xyxz)).xyw;
    // 294: mul r6.xyw, r6.xyxw, cb0[29].zzzz
    r6.xyw = ((r6.xyxw)*(source[29].zzzz)).xyw;
    // 295: mad r6.xyw, r6.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[28].wwww
    r6.xyw = ((r6.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[28].wwww)).xyw;
    // 296: dp3 r1.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 297: add r6.xyw, -r1.wwww, r6.xyxw
    r6.xyw = ((-(r1.wwww))+(r6.xyxw)).xyw;
    // 298: mad r6.xyw, r6.xyxw, l(0.800000, 0.800000, 0.000000, 0.800000), r1.wwww
    r6.xyw = ((r6.xyxw)*(float4(0.800000,0.800000,0.000000,0.800000))+(r1.wwww)).xyw;
    // 299: dp3 r1.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 300: div r1.w, r1.w, r6.z
    r1.w = ((r1.wwww)/(r6.zzzz)).w;
    // 301: mad r1.w, r2.x, l(5.000000), r1.w
    r1.w = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 302: add_sat r1.w, r12.w, r1.w
    r1.w = (saturate((r12.wwww)+(r1.wwww))).w;
    // 303: mad r2.x, r1.w, l(-2.000000), l(3.000000)
    r2.x = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 304: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 305: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 306: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 307: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 308: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 309: mul r2.xyz, r1.wwww, r6.xywx
    r2.xyz = ((r1.wwww)*(r6.xywx)).xyz;
    // 310: mul r6.xyz, r2.xyzx, r19.xyzx
    r6.xyz = ((r2.xyzx)*(r19.xyzx)).xyz;
    // 311: mad r1.w, r0.x, r18.x, r18.y
    r1.w = ((r0.xxxx)*(r18.xxxx)+(r18.yyyy)).w;
    // 312: mad r1.w, r1.w, r0.x, r18.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r18.zzzz)).w;
    // 313: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 314: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 315: mad r6.xyz, r6.xyzx, r0.xxxx, r16.xyzx
    r6.xyz = ((r6.xyzx)*(r0.xxxx)+(r16.xyzx)).xyz;
    // 316: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 317: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 318: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 319: dp3 r1.w, r12.xyzx, r5.xywx
    r1.w = (dot((r12.xyzx).xyz,(r5.xywx).xyz).xxxx).w;
    // 320: dp3 r5.x, -r12.xyzx, r5.xywx
    r5.x = (dot((-(r12.xyzx)).xyz,(r5.xywx).xyz).xxxx).x;
    // 321: dp3 r5.y, r12.xyzx, r20.xyzx
    r5.y = (dot((r12.xyzx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 322: mad r5.yw, r5.yyyy, l(0.000000, 0.500000, 0.000000, -0.500000), l(0.000000, 0.500000, 0.000000, 0.500000)
    r5.yw = ((r5.yyyy)*(float4(0.000000,0.500000,0.000000,-0.500000))+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 323: mul r5.yw, r5.yyyw, r5.yyyw
    r5.yw = ((r5.yyyw)*(r5.yyyw)).yw;
    // 324: mad r12.xy, r5.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r5.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 325: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 326: mad r13.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r13.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 327: mul r13.xy, r13.xyxx, r13.xyxx
    r13.xy = ((r13.xyxx)*(r13.xyxx)).xy;
    // 328: mul r13.yzw, r13.yyyy, cb0[39].xxyz
    r13.yzw = ((r13.yyyy)*(source[39].xxyz)).yzw;
    // 329: mad r13.xyz, r13.xxxx, cb0[38].xyzx, r13.yzwy
    r13.xyz = ((r13.xxxx)*(source[38].xyzx)+(r13.yzwy)).xyz;
    // 330: mul r13.xyz, r13.xyzx, cb0[40].wwww
    r13.xyz = ((r13.xyzx)*(source[40].wwww)).xyz;
    // 331: mul r13.xyz, r1.xyzx, r13.xyzx
    r13.xyz = ((r1.xyzx)*(r13.xyzx)).xyz;
    // 332: mul r13.xyz, r21.xyzx, r13.xyzx
    r13.xyz = ((r21.xyzx)*(r13.xyzx)).xyz;
    // 333: mul r13.xyz, r13.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 334: mul r13.xyz, r22.xyzx, r13.xyzx
    r13.xyz = ((r22.xyzx)*(r13.xyzx)).xyz;
    // 335: mad r13.xyz, -r13.xyzx, r12.wwww, r13.xyzx
    r13.xyz = ((-(r13.xyzx))*(r12.wwww)+(r13.xyzx)).xyz;
    // 336: mad r6.xyz, r6.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r13.xyzx
    r6.xyz = ((r6.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r13.xyzx)).xyz;
    // 337: mul r13.xyz, r5.wwww, cb0[39].xyzx
    r13.xyz = ((r5.wwww)*(source[39].xyzx)).xyz;
    // 338: mad r5.xyw, cb0[38].xyxz, r5.yyyy, r13.xyxz
    r5.xyw = ((source[38].xyxz)*(r5.yyyy)+(r13.xyxz)).xyw;
    // 339: mul r5.xyw, r5.xyxw, cb0[40].wwww
    r5.xyw = ((r5.xyxw)*(source[40].wwww)).xyw;
    // 340: mul r5.xyw, r0.xxxx, r5.xyxw
    r5.xyw = ((r0.xxxx)*(r5.xyxw)).xyw;
    // 341: mul r2.xyz, r2.xyzx, r5.xywx
    r2.xyz = ((r2.xyzx)*(r5.xywx)).xyz;
    // 342: mul r2.xyz, r2.xyzx, r19.xyzx
    r2.xyz = ((r2.xyzx)*(r19.xyzx)).xyz;
    // 343: mad r5.xyw, r2.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r6.xyxz
    r5.xyw = ((r2.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r6.xyxz)).xyw;
    // 344: mul r2.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 345: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 346: mad r2.xyz, -r8.xxxx, r4.xyzx, r7.xyzx
    r2.xyz = ((-(r8.xxxx))*(r4.xyzx)+(r7.xyzx)).xyz;
    // 347: mul r4.xyz, r4.xyzx, r8.xxxx
    r4.xyz = ((r4.xyzx)*(r8.xxxx)).xyz;
    // 348: mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 349: add r4.xyz, -r2.xyzx, r9.xyzx
    r4.xyz = ((-(r2.xyzx))+(r9.xyzx)).xyz;
    // 350: mad r2.xyz, r8.yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((r8.yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 351: add r4.xyz, -r2.xyzx, r10.xyzx
    r4.xyz = ((-(r2.xyzx))+(r10.xyzx)).xyz;
    // 352: mad r2.xyz, r8.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r8.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 353: add r4.xyz, -r2.xyzx, r7.xyzx
    r4.xyz = ((-(r2.xyzx))+(r7.xyzx)).xyz;
    // 354: mad r2.xyz, r2.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 355: add r4.xyz, r2.xyzx, -cb0[9].xyzx
    r4.xyz = ((r2.xyzx)+(-(source[9].xyzx))).xyz;
    // 356: add r2.xyz, r2.xyzx, -cb0[11].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[11].xyzx))).xyz;
    // 357: mad r2.xyz, r3.wwww, r2.xyzx, cb0[11].xyzx
    r2.xyz = ((r3.wwww)*(r2.xyzx)+(source[11].xyzx)).xyz;
    // 358: mad r4.xyz, r3.wwww, r4.xyzx, cb0[9].xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)+(source[9].xyzx)).xyz;
    // 359: mul r4.xyz, r4.xyzx, cb0[21].yyyy
    r4.xyz = ((r4.xyzx)*(source[21].yyyy)).xyz;
    // 360: mul r2.xyz, r2.xyzx, cb0[22].xxxx
    r2.xyz = ((r2.xyzx)*(source[22].xxxx)).xyz;
    // 361: mul r6.xy, v4.xyxx, cb0[10].zzzz
    r6.xy = ((v4.xyxx)*(source[10].zzzz)).xy;
    // 362: mul r6.zw, cb0[10].xxxy, cb0[21].zzzz
    r6.zw = ((source[10].xxxy)*(source[21].zzzz)).zw;
    // 363: mad r6.xy, r6.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r6.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 364: mad r6.zw, cb0[10].zzzz, v4.xxxy, r6.zzzw
    r6.zw = ((source[10].zzzz)*(v4.xxxy)+(r6.zzzw)).zw;
    // 365: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r6.xyxx, t6.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 366: mad r6.xy, r0.xxxx, cb0[21].wwww, r6.zwzz
    r6.xy = ((r0.xxxx)*(source[21].wwww)+(r6.zwzz)).xy;
    // 367: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t6.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 368: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 369: mul r6.xyz, r6.xyzx, r7.wwww
    r6.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 370: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 371: dp3 r0.x, r3.xyzx, r17.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r17.xyzx).xyz).xxxx).x;
    // 372: mul_sat r1.w, r0.x, cb0[22].y
    r1.w = (saturate((r0.xxxx)*(source[22].yyyy))).w;
    // 373: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 374: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 375: mul_sat r2.w, r17.z, cb0[22].y
    r2.w = (saturate((r17.zzzz)*(source[22].yyyy))).w;
    // 376: add r3.x, -|r17.z|, l(1.000000)
    r3.x = ((-(abs(r17.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 377: mul r0.x, r0.x, r3.x
    r0.x = ((r0.xxxx)*(r3.xxxx)).x;
    // 378: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 379: add_sat r2.w, r2.w, -cb0[22].z
    r2.w = (saturate((r2.wwww)+(-(source[22].zzzz)))).w;
    // 380: log r3.x, r2.w
    r3.x = (log2(r2.wwww)).x;
    // 381: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 382: mul r3.x, r3.x, cb0[22].w
    r3.x = ((r3.xxxx)*(source[22].wwww)).x;
    // 383: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 384: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 385: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 386: mad r3.xyz, r1.wwww, r2.xyzx, -r2.xyzx
    r3.xyz = ((r1.wwww)*(r2.xyzx)+(-(r2.xyzx))).xyz;
    // 387: mad r2.xyz, cb0[11].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[11].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 388: mad r2.xyz, r7.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 389: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 390: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 391: mad r2.xyz, cb0[23].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[23].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 392: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 393: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 394: mad r2.xyz, cb0[23].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[23].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 395: add r2.w, -r8.w, r1.w
    r2.w = ((-(r8.wwww))+(r1.wwww)).w;
    // 396: mad r3.xyz, r1.wwww, cb0[15].xyzx, -cb0[15].xyzx
    r3.xyz = ((r1.wwww)*(source[15].xyzx)+(-(source[15].xyzx))).xyz;
    // 397: mad r3.xyz, cb0[15].wwww, r3.xyzx, cb0[15].xyzx
    r3.xyz = ((source[15].wwww)*(r3.xyzx)+(source[15].xyzx)).xyz;
    // 398: mad r1.w, cb0[14].w, r2.w, r8.w
    r1.w = ((source[14].wwww)*(r2.wwww)+(r8.wwww)).w;
    // 399: mad r3.xyz, r1.wwww, cb0[14].xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(source[14].xyzx)+(r3.xyzx)).xyz;
    // 400: mad r2.xyz, r2.xyzx, r11.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 401: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 402: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 403: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 404: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 405: mul r3.xyz, r1.wwww, cb0[16].xyzx
    r3.xyz = ((r1.wwww)*(source[16].xyzx)).xyz;
    // 406: movc r3.xyz, r0.xxxx, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 407: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 408: mad r0.xyz, cb0[21].xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((source[21].xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 409: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 410: mul r2.xyz, r12.yyyy, cb0[39].xyzx
    r2.xyz = ((r12.yyyy)*(source[39].xyzx)).xyz;
    // 411: mad r2.xyz, r12.xxxx, cb0[38].xyzx, r2.xyzx
    r2.xyz = ((r12.xxxx)*(source[38].xyzx)+(r2.xyzx)).xyz;
    // 412: mul r2.xyz, r2.xyzx, cb0[40].wwww
    r2.xyz = ((r2.xyzx)*(source[40].wwww)).xyz;
    // 413: mul_sat r3.xyz, cb0[19].xyzx, cb0[19].wwww
    r3.xyz = (saturate((source[19].xyzx)*(source[19].wwww))).xyz;
    // 414: mul r4.xyz, r3.xyzx, r5.zzzz
    r4.xyz = ((r3.xyzx)*(r5.zzzz)).xyz;
    // 415: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 416: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 417: mul r3.xyz, r4.wwww, r4.xyzx
    r3.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 418: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 419: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 420: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 421: add r0.xyz, r5.xywx, r0.xyzx
    r0.xyz = ((r5.xywx)+(r0.xyzx)).xyz;
    // 422: dp3 o4.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 423: mad o0.xyz, r1.xyzx, cb0[40].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[40].xyzx)+(r0.xyzx)).xyz;
    // 424: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 425: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 426: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 427: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 428: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 429: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 430: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 431: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 432: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 433: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 434: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 435: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 436: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 437: ftou r0.x, cb0[37].z
    r0.x = (asfloat((uint4)(source[37].zzzz))).x;
    // 438: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 439: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 440: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 441: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 442: ret
    return output;
}

// source.character.equipment-native-186.v1 / source program 888b094bd453e64f96507586ac463605
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase186(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[23]=g_SourceCharacterEnvironmentColor; source[24]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0;
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
    // 7: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 8: mul r1.x, r0.x, l(0.125000)
    r1.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 9: mul r2.y, cb0[7].y, cb0[14].y
    r2.y = ((source[7].yyyy)*(source[14].yyyy)).y;
    // 10: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 11: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 12: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 13: frc r0.x, cb0[7].x
    r0.x = (frac(source[7].xxxx)).x;
    // 14: add r1.z, -r0.x, cb0[7].x
    r1.z = ((-(r0.xxxx))+(source[7].xxxx)).z;
    // 15: mul r2.z, r1.z, l(0.125000)
    r2.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 16: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 19: add r1.w, -cb0[7].w, l(1.000000)
    r1.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 21: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 22: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 23: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r2.x, cb0[7].z, l(1.500000)
    r2.x = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mad r1.w, r1.w, l(0.500000), cb0[7].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 27: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 28: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 30: mad r2.xyz, cb0[16].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[16].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 33: mad r2.xyz, cb0[17].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[17].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.w, r6.y, cb0[16].y
    r1.w = ((r6.yyyy)*(source[16].yyyy)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[4].xyzx, cb0[4].wwww
    r4.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 56: max r8.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 57: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 58: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 59: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: add r8.xyz, -r7.xyzx, r8.xyzx
    r8.xyz = ((-(r7.xyzx))+(r8.xyzx)).xyz;
    // 61: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 62: add r4.xyz, r4.xyzx, -r7.xyzx
    r4.xyz = ((r4.xyzx)+(-(r7.xyzx))).xyz;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: mad r4.xyz, r8.xxxx, r4.xyzx, r7.xyzx
    r4.xyz = ((r8.xxxx)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 65: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 66: mad r3.xyz, r8.yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((r8.yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 67: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 68: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 69: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 70: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 71: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 72: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 73: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 74: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 75: mad r3.xyz, r8.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 76: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 78: mad r4.xyz, cb0[16].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[16].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 79: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 80: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: add r3.xyz, -r4.xyzx, r2.wwww
    r3.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 82: mad r3.xyz, cb0[17].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[17].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 83: mad r4.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mad r7.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 86: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 87: mul r7.xyz, r2.xyzx, r3.xyzx
    r7.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 88: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: mad r2.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 90: mad r2.xyz, cb0[16].wwww, r2.xyzx, r7.xyzx
    r2.xyz = ((source[16].wwww)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 91: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 93: mad r2.xyz, cb0[17].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[17].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 94: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 95: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 96: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 97: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 98: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 99: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 100: max r0.x, r0.x, cb0[19].x
    r0.x = (max(r0.xxxx,source[19].xxxx)).x;
    // 101: min r0.x, r0.x, cb0[18].w
    r0.x = (min(r0.xxxx,source[18].wwww)).x;
    // 102: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: mad r0.x, r1.w, r2.x, r0.x
    r0.x = ((r1.wwww)*(r2.xxxx)+(r0.xxxx)).x;
    // 104: mul_sat r2.w, r1.w, cb2[3].w
    r2.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 105: add r1.w, r0.x, l(-1.000000)
    r1.w = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 106: mad r1.w, cb0[19].z, r1.w, l(1.000000)
    r1.w = ((source[19].zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: mul r3.xyz, r1.xyzx, r1.wwww
    r3.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 108: mul r2.x, r6.x, cb0[18].x
    r2.x = ((r6.xxxx)*(source[18].xxxx)).x;
    // 109: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 110: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 111: add_sat r2.x, r2.x, cb0[18].y
    r2.x = (saturate((r2.xxxx)+(source[18].yyyy))).x;
    // 112: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 113: mul r4.xyz, r2.yyyy, cb0[13].xyzx
    r4.xyz = ((r2.yyyy)*(source[13].xyzx)).xyz;
    // 114: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 115: mad r1.xyz, r1.wwww, r1.xyzx, -r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 116: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 117: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 118: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 119: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 120: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 121: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 122: mad r4.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 123: mad r5.xyw, r1.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r5.xyw = ((r1.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 124: mad r4.xyz, r0.xxxx, r4.xyzx, r5.xywx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)+(r5.xywx)).xyz;
    // 125: mad r3.xyz, r4.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 126: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 127: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 128: mov_sat r1.w, cb0[19].w
    r1.w = (saturate(source[19].wwww)).w;
    // 129: mad r4.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r4.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 130: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 131: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 132: mad r4.xyz, r2.wwww, r4.xyzx, r2.xxxx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xxxx)).xyz;
    // 133: mul_sat r1.w, r4.y, l(50.000000)
    r1.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 134: add r2.x, -cb0[20].w, cb0[20].z
    r2.x = ((-(source[20].wwww))+(source[20].zzzz)).x;
    // 135: mad r2.x, r8.x, r2.x, cb0[20].w
    r2.x = ((r8.xxxx)*(r2.xxxx)+(source[20].wwww)).x;
    // 136: add r2.y, -r2.x, cb0[21].y
    r2.y = ((-(r2.xxxx))+(source[21].yyyy)).y;
    // 137: mad r2.x, r8.y, r2.y, r2.x
    r2.x = ((r8.yyyy)*(r2.yyyy)+(r2.xxxx)).x;
    // 138: add r2.y, -r2.x, cb0[21].w
    r2.y = ((-(r2.xxxx))+(source[21].wwww)).y;
    // 139: mad r2.x, r8.z, r2.y, r2.x
    r2.x = ((r8.zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 140: mul r2.x, r6.z, r2.x
    r2.x = ((r6.zzzz)*(r2.xxxx)).x;
    // 141: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 142: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: movc r2.x, r5.z, l(0), r2.x
    r2.x = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 144: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 145: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 147: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 148: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 149: mul r5.xy, r2.xyxx, cb0[16].xxxx
    r5.xy = ((r2.xyxx)*(source[16].xxxx)).xy;
    // 150: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 151: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 152: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 153: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 154: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 155: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 156: div r5.xyz, r5.xyzx, r2.xxxx
    r5.xyz = ((r5.xyzx)/(r2.xxxx)).xyz;
    // 157: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 158: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 159: mul r6.xyz, r2.xxxx, r5.xyzx
    r6.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 160: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 161: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 162: mul r7.xyz, r2.xxxx, v5.xyzx
    r7.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 163: dp3 r2.x, r6.xyzx, r7.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 164: deriv_rtx_coarse r8.x, r2.x
    r8.x = (ddx_coarse(r2.xxxx)).x;
    // 165: deriv_rty_coarse r8.y, r2.x
    r8.y = (ddy_coarse(r2.xxxx)).y;
    // 166: dp2 r2.y, r8.xyxx, r8.xyxx
    r2.y = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).y;
    // 167: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 168: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 169: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 170: min r8.y, r2.y, l(1.000000)
    r8.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: add r2.y, -r8.y, l(1.000000)
    r2.y = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 172: max r9.xyz, r4.xyzx, r2.yyyy
    r9.xyz = (max(r4.xyzx,r2.yyyy)).xyz;
    // 173: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 174: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 175: mul r10.xyz, r2.xxxx, r6.xyzx
    r10.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 176: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 177: add r1.w, r10.z, l(1.000000)
    r1.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 180: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 181: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 182: mul r2.x, r2.x, cb0[1].y
    r2.x = ((r2.xxxx)*(source[1].yyyy)).x;
    // 183: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 184: mad_sat r2.x, r2.x, cb0[1].w, cb0[1].z
    r2.x = (saturate((r2.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 185: mul r2.x, r2.x, cb0[22].x
    r2.x = ((r2.xxxx)*(source[22].xxxx)).x;
    // 186: add_sat r8.x, -r1.w, r2.y
    r8.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 187: sample_indexable(texture2d)(float,float,float,float) r2.yz, r8.xyxx, t5.zxyw, s6
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 188: add r1.w, r0.x, r8.x
    r1.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 189: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 190: mul r11.xyz, r2.zzzz, r4.xyzx
    r11.xyz = ((r2.zzzz)*(r4.xyzx)).xyz;
    // 191: mad r9.xyz, r9.xyzx, r2.yyyy, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r2.yyyy)+(r11.xyzx)).xyz;
    // 192: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 193: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 194: mad r11.xyz, r4.xyzx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r4.xyzx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 195: dp3 r2.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mad r4.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 197: mad r12.xyz, -r9.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 199: mul r11.xyz, r1.xyzx, r12.xyzx
    r11.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 200: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 201: mul r11.xyz, r2.yyyy, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r11.xyzx)).xyz;
    // 202: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 203: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 204: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 205: mul r13.xyz, r3.wwww, v1.xyzx
    r13.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 206: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 207: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 208: mul r14.xyz, r3.wwww, v0.xyzx
    r14.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 209: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 210: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 211: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 212: dp3 r16.y, r15.xyzx, r6.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 213: dp3 r15.y, r15.xyzx, r10.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 214: dp3 r16.x, r14.xyzx, r6.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 215: dp3 r15.x, r14.xyzx, r10.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 216: dp2 r14.z, r16.xyxx, cb0[24].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 217: mul r8.xz, cb0[24].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[24].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 218: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 219: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 220: dp2 r17.z, r15.xyxx, cb0[24].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 221: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 222: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 223: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 224: dp4 r13.x, cb0[25].xyzw, r14.xyzw
    r13.x = (dot((source[25].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 225: dp4 r13.y, cb0[26].xyzw, r14.xyzw
    r13.y = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 226: dp4 r13.z, cb0[27].xyzw, r14.xyzw
    r13.z = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 227: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 228: dp4 r18.x, cb0[28].xyzw, r15.xyzw
    r18.x = (dot((source[28].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 229: dp4 r18.y, cb0[29].xyzw, r15.xyzw
    r18.y = (dot((source[29].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 230: dp4 r18.z, cb0[30].xyzw, r15.xyzw
    r18.z = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 231: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 232: mul r3.w, r14.y, r14.y
    r3.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 233: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 234: mad r3.w, r14.x, r14.x, -r3.w
    r3.w = ((r14.xxxx)*(r14.xxxx)+(-(r3.wwww))).w;
    // 235: mad r13.xyz, cb0[31].xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((source[31].xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 236: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 237: mul r13.xyz, r13.xyzx, cb0[23].xyzx
    r13.xyz = ((r13.xyzx)*(source[23].xyzx)).xyz;
    // 238: mul r13.xyz, r13.xyzx, cb0[24].zzzz
    r13.xyz = ((r13.xyzx)*(source[24].zzzz)).xyz;
    // 239: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[23].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[23].wwww)).xyz;
    // 240: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 241: add r13.xyz, -r3.wwww, r13.xyzx
    r13.xyz = ((-(r3.wwww))+(r13.xyzx)).xyz;
    // 242: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 243: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 244: mad r4.w, r8.y, l(2.000000), l(2.000000)
    r4.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 245: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 246: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 247: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 248: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 249: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 250: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 251: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 252: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 253: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 254: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 255: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 256: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 257: mul r11.xyz, r3.xyzx, r11.xyzx
    r11.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 258: mul r3.w, r8.y, l(5.000000)
    r3.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 259: mul r5.w, r8.y, r8.y
    r5.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 260: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 261: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 262: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 263: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 264: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 265: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t6.xyzw, s5, r3.w
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r17.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 266: mul r8.xyz, r13.xyzx, r13.wwww
    r8.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 267: mul r8.xyz, r8.xyzx, cb0[23].xyzx
    r8.xyz = ((r8.xyzx)*(source[23].xyzx)).xyz;
    // 268: mul r8.xyz, r8.xyzx, cb0[24].zzzz
    r8.xyz = ((r8.xyzx)*(source[24].zzzz)).xyz;
    // 269: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[23].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[23].wwww)).xyz;
    // 270: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 271: add r8.xyz, -r1.wwww, r8.xyzx
    r8.xyz = ((-(r1.wwww))+(r8.xyzx)).xyz;
    // 272: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 273: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 275: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 276: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 277: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 278: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 279: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 280: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 281: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 282: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 283: mul r8.xyz, r1.wwww, r8.xyzx
    r8.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 284: mul r13.xyz, r8.xyzx, r9.xyzx
    r13.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 285: mad r1.w, r0.x, r4.x, r4.y
    r1.w = ((r0.xxxx)*(r4.xxxx)+(r4.yyyy)).w;
    // 286: mad r1.w, r1.w, r0.x, r4.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r4.zzzz)).w;
    // 287: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 288: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 289: mad r4.xyz, r13.xyzx, r0.xxxx, r11.xyzx
    r4.xyz = ((r13.xyzx)*(r0.xxxx)+(r11.xyzx)).xyz;
    // 290: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 291: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 292: mul r11.xyz, r1.wwww, v6.xyzx
    r11.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 293: dp3 r1.w, r11.xyzx, r6.xyzx
    r1.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 294: dp3 r2.z, -r11.xyzx, r6.xyzx
    r2.z = (dot((-(r11.xyzx)).xyz,(r6.xyzx).xyz).xxxx).z;
    // 295: dp3 r3.w, r11.xyzx, r10.xyzx
    r3.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 296: mad r6.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 297: mad r6.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 298: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 299: mad r10.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 300: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 301: mul r10.yzw, r10.yyyy, cb0[34].xxyz
    r10.yzw = ((r10.yyyy)*(source[34].xxyz)).yzw;
    // 302: mad r10.xyz, r10.xxxx, cb0[33].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[33].xyzx)+(r10.yzwy)).xyz;
    // 303: mul r10.xyz, r10.xyzx, cb0[35].wwww
    r10.xyz = ((r10.xyzx)*(source[35].wwww)).xyz;
    // 304: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 305: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 306: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 307: mul r3.xyz, r12.xyzx, r3.xyzx
    r3.xyz = ((r12.xyzx)*(r3.xyzx)).xyz;
    // 308: mad r3.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 309: mad r3.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 310: mul r4.xyz, r6.yyyy, cb0[34].xyzx
    r4.xyz = ((r6.yyyy)*(source[34].xyzx)).xyz;
    // 311: mad r4.xyz, cb0[33].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[33].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 312: mul r4.xyz, r4.xyzx, cb0[35].wwww
    r4.xyz = ((r4.xyzx)*(source[35].wwww)).xyz;
    // 313: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 314: mul r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)*(r4.xyzx)).xyz;
    // 315: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 316: mad r3.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 317: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 318: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 319: dp3 r0.x, r5.xyzx, r7.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 320: mul_sat r1.w, r0.x, cb0[17].y
    r1.w = (saturate((r0.xxxx)*(source[17].yyyy))).w;
    // 321: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 322: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 323: mul_sat r2.z, r7.z, cb0[17].y
    r2.z = (saturate((r7.zzzz)*(source[17].yyyy))).z;
    // 324: add r2.w, -|r7.z|, l(1.000000)
    r2.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 325: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 326: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 327: add_sat r2.z, r2.z, -cb0[17].z
    r2.z = (saturate((r2.zzzz)+(-(source[17].zzzz)))).z;
    // 328: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 329: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 330: mul r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)*(source[17].wwww)).w;
    // 331: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 332: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 333: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 334: add r2.z, -r8.w, r1.w
    r2.z = ((-(r8.wwww))+(r1.wwww)).z;
    // 335: mad r4.xyz, r1.wwww, cb0[11].xyzx, -cb0[11].xyzx
    r4.xyz = ((r1.wwww)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 336: mad r4.xyz, cb0[11].wwww, r4.xyzx, cb0[11].xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(source[11].xyzx)).xyz;
    // 337: mad r1.w, cb0[10].w, r2.z, r8.w
    r1.w = ((source[10].wwww)*(r2.zzzz)+(r8.wwww)).w;
    // 338: mad r4.xyz, r1.wwww, cb0[10].xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(source[10].xyzx)+(r4.xyzx)).xyz;
    // 339: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 340: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 341: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 342: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 343: mul r5.xyz, r1.wwww, cb0[12].xyzx
    r5.xyz = ((r1.wwww)*(source[12].xyzx)).xyz;
    // 344: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 345: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 346: mad r0.xyz, cb0[16].zzzz, r0.yzwy, r4.xyzx
    r0.xyz = ((source[16].zzzz)*(r0.yzwy)+(r4.xyzx)).xyz;
    // 347: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 348: mul r4.xyz, r6.wwww, cb0[34].xyzx
    r4.xyz = ((r6.wwww)*(source[34].xyzx)).xyz;
    // 349: mad r4.xyz, r6.zzzz, cb0[33].xyzx, r4.xyzx
    r4.xyz = ((r6.zzzz)*(source[33].xyzx)+(r4.xyzx)).xyz;
    // 350: mul r4.xyz, r4.xyzx, cb0[35].wwww
    r4.xyz = ((r4.xyzx)*(source[35].wwww)).xyz;
    // 351: mul_sat r5.xyz, cb0[15].xyzx, cb0[15].wwww
    r5.xyz = (saturate((source[15].xyzx)*(source[15].wwww))).xyz;
    // 352: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 353: mul r5.xyz, r5.xyzx, cb0[22].xxxx
    r5.xyz = ((r5.xyzx)*(source[22].xxxx)).xyz;
    // 354: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 355: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 356: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 357: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 358: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 359: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 360: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 361: mad o0.xyz, r1.xyzx, cb0[35].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[35].xyzx)+(r0.xyzx)).xyz;
    // 362: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 363: dp3 r0.x, r16.xyzx, r16.xyzx
    r0.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 364: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 365: mul r0.xyz, r0.xxxx, r16.xyzx
    r0.xyz = ((r0.xxxx)*(r16.xyzx)).xyz;
    // 366: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 367: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 368: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 369: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 370: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 371: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 372: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 373: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 374: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 375: ftou r0.x, cb0[32].z
    r0.x = (asfloat((uint4)(source[32].zzzz))).x;
    // 376: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 377: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 378: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 379: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 380: ret
    return output;
}

// source.character.equipment-native-187.v1 / source program 5164c1f2a6b3d5479fb11aba50d63e00
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase187(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
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
    // 7: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 8: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 10: add r3.xyzw, v9.yzxy, cb0[0].yzxy
    r3.xyzw = ((v9.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v9.yyyy, cb1[1].xywx
    r4.xyz = ((v9.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v9.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v9.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v9.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v9.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v9.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v9.wwww)+(r4.xyzx)).xyz;
    // 15: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 16: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 17: mul r5.xyz, r0.wwww, v7.xyzx
    r5.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v5.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 19: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 20: mul r7.xy, r6.xyxx, cb0[14].xxxx
    r7.xy = ((r6.xyxx)*(source[14].xxxx)).xy;
    // 21: dp2 r0.w, r6.xyxx, r6.xyxx
    r0.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 22: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 25: add r7.z, r0.w, l(0.000010)
    r7.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: dp3 r0.w, r7.xyzx, r7.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 27: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 28: div r6.xyz, r7.xyzx, r0.wwww
    r6.xyz = ((r7.xyzx)/(r0.wwww)).xyz;
    // 29: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 31: mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v5.xyxx, t2.xyzw, s5, l(0.000000)
    r0.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 33: mov_sat r0.w, r0.w
    r0.w = (saturate(r0.wwww)).w;
    // 34: mul_sat r0.w, r0.w, cb0[17].x
    r0.w = (saturate((r0.wwww)*(source[17].xxxx))).w;
    // 35: mul r1.w, r0.w, cb0[1].w
    r1.w = ((r0.wwww)*(source[1].wwww)).w;
    // 36: add r9.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: mul r10.xyz, cb0[3].xyzx, cb0[3].wwww
    r10.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 38: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: mad r11.xyz, -cb0[3].wwww, cb0[3].xyzx, r2.wwww
    r11.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r2.wwww)).xyz;
    // 40: mad r10.xyz, cb0[14].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[14].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 41: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 42: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 43: mad r10.xyz, cb0[14].wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((source[14].wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 44: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 46: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 47: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r12.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r12.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 49: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 50: add r13.xyz, -r12.xyzx, r2.wwww
    r13.xyz = ((-(r12.xyzx))+(r2.wwww)).xyz;
    // 51: mad r12.xyz, cb0[14].zzzz, r13.xyzx, r12.xyzx
    r12.xyz = ((source[14].zzzz)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 52: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r13.xyz, -r12.xyzx, r2.wwww
    r13.xyz = ((-(r12.xyzx))+(r2.wwww)).xyz;
    // 54: mad r12.xyz, cb0[14].wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((source[14].wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 55: mul r13.xyz, r10.xyzx, r12.xyzx
    r13.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 56: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: mad r10.xyz, -r10.xyzx, r12.xyzx, r2.wwww
    r10.xyz = ((-(r10.xyzx))*(r12.xyzx)+(r2.wwww)).xyz;
    // 58: mad r10.xyz, cb0[14].zzzz, r10.xyzx, r13.xyzx
    r10.xyz = ((source[14].zzzz)*(r10.xyzx)+(r13.xyzx)).xyz;
    // 59: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 60: add r12.xyz, -r10.xyzx, r2.wwww
    r12.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 61: mad r10.xyz, cb0[14].wwww, r12.xyzx, r10.xyzx
    r10.xyz = ((source[14].wwww)*(r12.xyzx)+(r10.xyzx)).xyz;
    // 62: mul r10.xyz, r11.xyzx, r10.xyzx
    r10.xyz = ((r11.xyzx)*(r10.xyzx)).xyz;
    // 63: mul r2.w, cb0[9].z, l(1.500000)
    r2.w = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 64: add r4.w, -cb0[9].w, l(1.000000)
    r4.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: mul r4.w, r4.w, cb0[16].w
    r4.w = ((r4.wwww)*(source[16].wwww)).w;
    // 66: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 67: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 68: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 70: mad r2.w, r2.w, l(0.500000), cb0[9].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 71: frc r4.w, cb0[9].x
    r4.w = (frac(source[9].xxxx)).w;
    // 72: add r5.w, -r4.w, cb0[9].x
    r5.w = ((-(r4.wwww))+(source[9].xxxx)).w;
    // 73: mul r12.z, r5.w, l(0.125000)
    r12.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 74: mov r12.xw, l(0,0,0,0)
    r12.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 75: mul r12.y, cb0[9].y, cb0[10].y
    r12.y = ((source[9].yyyy)*(source[10].yyyy)).y;
    // 76: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 77: mul r14.x, r5.w, l(0.125000)
    r14.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 78: mov r14.y, v4.y
    r14.y = (v4.yyyy).y;
    // 79: add r12.xy, r12.xyxx, r14.xyxx
    r12.xy = ((r12.xyxx)+(r14.xyxx)).xy;
    // 80: add r12.xy, r12.xyxx, r12.zwzz
    r12.xy = ((r12.xyxx)+(r12.zwzz)).xy;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r12.xyxx, t4.xyzw, s4, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r12.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 82: mul r12.xyz, r2.wwww, r12.xyzx
    r12.xyz = ((r2.wwww)*(r12.xyzx)).xyz;
    // 83: mul r2.w, r4.w, r12.w
    r2.w = ((r4.wwww)*(r12.wwww)).w;
    // 84: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 85: mad r10.xyz, r2.wwww, r12.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r12.xyzx)+(r10.xyzx)).xyz;
    // 86: mul r2.w, cb0[11].y, cb0[16].w
    r2.w = ((source[11].yyyy)*(source[16].wwww)).w;
    // 87: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 88: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 89: mul r12.y, r2.w, l(0.020000)
    r12.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 90: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 91: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 92: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 93: mad r3.xy, cb0[11].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[11].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 94: mul r3.z, cb0[11].x, l(0.001000)
    r3.z = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 95: mov r12.x, l(0)
    r12.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 96: mad r3.xy, r3.zzzz, r3.xyxx, r12.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r12.xyxx)).xy;
    // 97: dp2 r3.z, cb0[12].xyxx, r3.xyxx
    r3.z = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 98: dp2 r3.y, cb0[13].xyxx, r3.xyxx
    r3.y = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 99: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 100: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 101: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 102: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 103: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r10.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r10.xyzx))).xyz;
    // 104: mad r3.xyz, r3.wwww, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 105: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 107: mul_sat r3.xyz, r3.xyzx, r2.wwww
    r3.xyz = (saturate((r3.xyzx)*(r2.wwww))).xyz;
    // 108: mul r12.xyz, r3.xyzx, cb0[11].zzzz
    r12.xyz = ((r3.xyzx)*(source[11].zzzz)).xyz;
    // 109: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 111: mad r3.xyz, cb0[11].zzzz, r3.xyzx, -r10.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(-(r10.xyzx))).xyz;
    // 112: mad r3.xyz, r2.wwww, r3.xyzx, r10.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 113: mul r3.xyz, r9.xyzx, r3.xyzx
    r3.xyz = ((r9.xyzx)*(r3.xyzx)).xyz;
    // 114: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 115: add r9.xyz, -r7.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r7.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 116: mad r7.xyz, cb0[15].xxxx, r9.xyzx, r7.xyzx
    r7.xyz = ((source[15].xxxx)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 117: dp3 r2.w, r7.xyzx, r7.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 118: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 119: div r7.xyz, r7.xyzx, r2.wwww
    r7.xyz = ((r7.xyzx)/(r2.wwww)).xyz;
    // 120: dp3 r9.x, r1.xyzx, r7.xyzx
    r9.x = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 121: dp3 r9.y, r2.xyzx, r7.xyzx
    r9.y = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 122: dp3 r9.z, r0.xyzx, r7.xyzx
    r9.z = (dot((r0.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 123: dp3 r7.x, r1.xyzx, r5.xyzx
    r7.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 124: dp3 r7.y, r2.xyzx, r5.xyzx
    r7.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 125: dp3 r7.z, r0.xyzx, r5.xyzx
    r7.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 126: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 127: mul r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = ((r9.xyzx)*(r2.wwww)).xyz;
    // 128: mad r7.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 129: mov r7.w, -r7.x
    r7.w = (-(r7.xxxx)).w;
    // 130: dp2 r2.w, r7.ywyy, r7.ywyy
    r2.w = (dot((r7.ywyy).xy,(r7.ywyy).xy).xxxx).w;
    // 131: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 132: div r7.xy, r7.ywyy, r2.wwww
    r7.xy = ((r7.ywyy)/(r2.wwww)).xy;
    // 133: mad r2.w, -r7.z, l(0.250000), l(0.250000)
    r2.w = ((-(r7.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 134: mad r7.xy, r2.wwww, r7.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r2.wwww)*(r7.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v5.xyxx, t1.xyzw, s1, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 136: add r2.w, -r9.w, l(1.000000)
    r2.w = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 138: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 139: mul r2.w, r2.w, cb0[14].y
    r2.w = ((r2.wwww)*(source[14].yyyy)).w;
    // 140: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 141: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 143: mul r2.w, r2.w, cb0[15].y
    r2.w = ((r2.wwww)*(source[15].yyyy)).w;
    // 144: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 145: div r4.xy, r4.xyxx, r4.zzzz
    r4.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 146: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 147: mul r4.xy, r4.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 148: deriv_rtx_coarse r4.zw, r4.xxxy
    r4.zw = (ddx_coarse(r4.xxxy)).zw;
    // 149: deriv_rty_coarse r4.xy, r4.xyxx
    r4.xy = (ddy_coarse(r4.xyxx)).xy;
    // 150: dp2 r3.w, r4.zwzz, r4.zwzz
    r3.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // 151: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 152: max r3.w, r3.w, r4.x
    r3.w = (max(r3.wwww,r4.xxxx)).w;
    // 153: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 154: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 155: rcp r4.x, |r3.w|
    r4.x = (1.0/(abs(r3.wwww))).x;
    // 156: mul r2.w, r2.w, r4.x
    r2.w = ((r2.wwww)*(r4.xxxx)).w;
    // 157: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 158: add r2.w, r2.w, |r3.w|
    r2.w = ((r2.wwww)+(abs(r3.wwww))).w;
    // 159: round_ni r2.w, r2.w
    r2.w = (floor(r2.wwww)).w;
    // 160: sample_l_indexable(texture2d)(float,float,float,float) r4.xyz, r7.xyxx, t5.xyzw, s3, r2.w
    r4.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r7.xyxx).xy, (r2.wwww).x)).xyzw).xyz;
    // 161: rcp r2.w, cb0[15].z
    r2.w = (1.0/(source[15].zzzz)).w;
    // 162: log r7.xyz, r4.xyzx
    r7.xyz = (log2(r4.xyzx)).xyz;
    // 163: mul r10.xyz, r7.xyzx, cb0[15].zzzz
    r10.xyz = ((r7.xyzx)*(source[15].zzzz)).xyz;
    // 164: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 165: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 166: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 167: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 168: mad r7.xyz, r10.xyzx, cb0[15].zzzz, r7.xyzx
    r7.xyz = ((r10.xyzx)*(source[15].zzzz)+(r7.xyzx)).xyz;
    // 169: add r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)+(r7.xyzx)).xyz;
    // 170: mul r4.xyz, r4.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 171: add r2.w, cb0[15].z, l(1.000000)
    r2.w = ((source[15].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 173: mul r4.xyz, r4.xyzx, cb0[15].wwww
    r4.xyz = ((r4.xyzx)*(source[15].wwww)).xyz;
    // 174: mul r7.xyz, r4.xyzx, r13.xyzx
    r7.xyz = ((r4.xyzx)*(r13.xyzx)).xyz;
    // 175: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: add r10.xyz, -r9.xyzx, r2.wwww
    r10.xyz = ((-(r9.xyzx))+(r2.wwww)).xyz;
    // 177: mad r9.xyz, cb0[14].zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((source[14].zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 178: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 179: add r10.xyz, -r9.xyzx, r2.wwww
    r10.xyz = ((-(r9.xyzx))+(r2.wwww)).xyz;
    // 180: mad r9.xyz, cb0[14].wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((source[14].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 181: dp3 r2.w, r6.xyzx, r5.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 182: mov_sat r3.w, r2.w
    r3.w = (saturate(r2.wwww)).w;
    // 183: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mov_sat r4.w, r5.z
    r4.w = (saturate(r5.zzzz)).w;
    // 185: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 187: max r5.xyw, |r9.xyxz|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r5.xyw = (max(abs(r9.xyxz),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 188: log r5.xyw, r5.xyxw
    r5.xyw = (log2(r5.xyxw)).xyw;
    // 189: mul r5.xyw, r5.xyxw, l(0.454545, 0.454545, 0.000000, 0.454545)
    r5.xyw = ((r5.xyxw)*(float4(0.454545,0.454545,0.000000,0.454545))).xyw;
    // 190: exp r5.xyw, r5.xyxw
    r5.xyw = (exp2(r5.xyxw)).xyw;
    // 191: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 193: mul r4.w, r4.w, cb0[16].x
    r4.w = ((r4.wwww)*(source[16].xxxx)).w;
    // 194: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 195: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 196: mad r5.x, -r4.w, r4.w, l(1.000000)
    r5.x = ((-(r4.wwww))*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 197: max r5.x, r5.x, l(0.001000)
    r5.x = (max(r5.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 198: div r5.x, cb0[16].y, r5.x
    r5.x = ((source[16].yyyy)/(r5.xxxx)).x;
    // 199: mul r5.x, r3.w, r5.x
    r5.x = ((r3.wwww)*(r5.xxxx)).x;
    // 200: mul r4.xyz, r4.xyzx, r5.xxxx
    r4.xyz = ((r4.xyzx)*(r5.xxxx)).xyz;
    // 201: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 202: mul r4.w, r4.w, cb0[16].z
    r4.w = ((r4.wwww)*(source[16].zzzz)).w;
    // 203: mad r4.xyz, r9.xyzx, r4.xyzx, -r7.xyzx
    r4.xyz = ((r9.xyzx)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 204: mad r4.xyz, r4.wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 205: dp3 r4.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 206: add r5.xyw, -r4.xyxz, r4.wwww
    r5.xyw = ((-(r4.xyxz))+(r4.wwww)).xyw;
    // 207: mad r4.xyz, cb0[14].zzzz, r5.xywx, r4.xyzx
    r4.xyz = ((source[14].zzzz)*(r5.xywx)+(r4.xyzx)).xyz;
    // 208: dp3 r4.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 209: add r5.xyw, -r4.xyxz, r4.wwww
    r5.xyw = ((-(r4.xyxz))+(r4.wwww)).xyw;
    // 210: mad r4.xyz, cb0[14].wwww, r5.xywx, r4.xyzx
    r4.xyz = ((source[14].wwww)*(r5.xywx)+(r4.xyzx)).xyz;
    // 211: mul r4.w, r3.w, cb0[6].w
    r4.w = ((r3.wwww)*(source[6].wwww)).w;
    // 212: mad r5.xyw, r3.wwww, cb0[7].xyxz, -cb0[7].xyxz
    r5.xyw = ((r3.wwww)*(source[7].xyxz)+(-(source[7].xyxz))).xyw;
    // 213: mad r5.xyw, cb0[7].wwww, r5.xyxw, cb0[7].xyxz
    r5.xyw = ((source[7].wwww)*(r5.xyxw)+(source[7].xyxz)).xyw;
    // 214: mad r5.xyw, r4.wwww, cb0[6].xyxz, r5.xyxw
    r5.xyw = ((r4.wwww)*(source[6].xyxz)+(r5.xyxw)).xyw;
    // 215: mad r4.xyz, r4.xyzx, r11.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r11.xyzx)+(r5.xywx)).xyz;
    // 216: add r3.w, -|r5.z|, l(1.000000)
    r3.w = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 217: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 219: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 220: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 221: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 222: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 223: mul r5.xyz, r2.wwww, cb0[8].xyzx
    r5.xyz = ((r2.wwww)*(source[8].xyzx)).xyz;
    // 224: movc r5.xyz, r3.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 225: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 226: add r4.xyz, r4.xyzx, cb0[2].xyzx
    r4.xyz = ((r4.xyzx)+(source[2].xyzx)).xyz;
    // 227: dp3 r2.w, v8.xyzx, v8.xyzx
    r2.w = (dot((v8.xyzx).xyz,(v8.xyzx).xyz).xxxx).w;
    // 228: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 229: mul r5.xyz, r2.wwww, v8.xyzx
    r5.xyz = ((r2.wwww)*(v8.xyzx)).xyz;
    // 230: dp3 r2.w, r5.xyzx, r8.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 231: mad r5.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 232: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 233: mul r5.yzw, r5.yyyy, cb0[20].xxyz
    r5.yzw = ((r5.yyyy)*(source[20].xxyz)).yzw;
    // 234: mad r5.xyz, r5.xxxx, cb0[19].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[19].xyzx)+(r5.yzwy)).xyz;
    // 235: mul r5.xyz, r5.xyzx, cb0[21].wwww
    r5.xyz = ((r5.xyzx)*(source[21].wwww)).xyz;
    // 236: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 237: mad r4.xyz, r5.xyzx, r3.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 238: mad r4.xyz, r3.xyzx, cb0[21].xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(source[21].xyzx)+(r4.xyzx)).xyz;
    // 239: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 240: mad r2.w, r2.w, l(-0.250000), l(0.400000)
    r2.w = ((r2.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 241: eq r3.w, cb0[22].x, l(0.000000)
    r3.w = (asfloat((uint4)((source[22].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 242: not r4.w, r3.w
    r4.w = (asfloat(~asuint(r3.wwww))).w;
    // 243: lt r5.x, r1.w, r2.w
    r5.x = (asfloat((uint4)((r1.wwww)<(r2.wwww)) * 0xffffffffu)).x;
    // 244: and r4.w, r4.w, r5.x
    r4.w = (asfloat(asuint(r4.wwww) & asuint(r5.xxxx))).w;
    // 245: discard_nz r4.w
    if ((asuint(r4.wwww)).x != 0u) { output.discarded = true; return output; }
    // 246: ge r2.w, r1.w, r2.w
    r2.w = (asfloat((uint4)((r1.wwww)>=(r2.wwww)) * 0xffffffffu)).w;
    // 247: mad r0.w, r0.w, cb0[1].w, l(-0.900000)
    r0.w = ((r0.wwww)*(source[1].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 248: mul_sat r0.w, r0.w, l(9.999998)
    r0.w = (saturate((r0.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 249: mad r4.w, r0.w, l(-2.000000), l(3.000000)
    r4.w = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 250: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 251: mul r0.w, r0.w, r4.w
    r0.w = ((r0.wwww)*(r4.wwww)).w;
    // 252: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 253: movc r0.w, r2.w, r0.w, r1.w
    r0.w = ((asuint(r2.wwww) != 0u) ? (r0.wwww) : (r1.wwww)).w;
    // 254: movc o0.w, r3.w, r0.w, r1.w
    output.targets[0].w = ((asuint(r3.wwww) != 0u) ? (r0.wwww) : (r1.wwww)).w;
    // 255: mad o0.xyz, r4.xyzx, v6.wwww, v6.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(v6.wwww)+(v6.xyzx)).xyz;
    // 256: dp3 r1.x, r1.xyzx, r8.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 257: dp3 r1.y, r2.xyzx, r8.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 258: dp3 r1.z, r0.xyzx, r8.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 259: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 260: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 261: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 262: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 263: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 264: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 265: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 266: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 267: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 268: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 269: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 270: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 271: ftou r0.x, cb0[18].z
    r0.x = (asfloat((uint4)(source[18].zzzz))).x;
    // 272: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 273: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 274: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 275: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 276: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 277: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 278: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 279: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 280: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 281: ret
    return output;
}

// source.character.equipment-native-188.v1 / source program ed1d0fa50e64cd4482674a123fc8efac
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase188(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[19]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[23].x=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[30]=g_SourceCharacterEnvironmentColor; source[31]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0, r22=0.0, r23=0.0, r24=0.0;
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
    // 7: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 8: mul r1.x, r0.x, l(0.125000)
    r1.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 9: mul r2.y, cb0[9].y, cb0[19].y
    r2.y = ((source[9].yyyy)*(source[19].yyyy)).y;
    // 10: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 11: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 12: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 13: frc r0.x, cb0[9].x
    r0.x = (frac(source[9].xxxx)).x;
    // 14: add r1.z, -r0.x, cb0[9].x
    r1.z = ((-(r0.xxxx))+(source[9].xxxx)).z;
    // 15: mul r2.z, r1.z, l(0.125000)
    r2.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 16: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t4.xyzw, s6, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 19: add r1.w, -cb0[9].w, l(1.000000)
    r1.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mul r1.w, r1.w, cb0[23].x
    r1.w = ((r1.wwww)*(source[23].xxxx)).w;
    // 21: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 22: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 23: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r2.x, cb0[9].z, l(1.500000)
    r2.x = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mad r1.w, r1.w, l(0.500000), cb0[9].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 27: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 28: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 30: mad r2.xyz, cb0[24].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[24].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 33: mad r2.xyz, cb0[25].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.w, r6.y, cb0[21].y
    r1.w = ((r6.yyyy)*(source[21].yyyy)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: add r7.xyz, -r3.xyzx, r4.xyzx
    r7.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 56: round_ni r5.yw, v7.xxxy
    r5.yw = (floor(v7.xxxy)).yw;
    // 57: dp2 r2.w, r5.ywyy, l(12.989800, 78.233002, 0.000000, 0.000000)
    r2.w = (dot((r5.ywyy).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).w;
    // 58: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 59: mul r2.w, r2.w, l(43758.546875)
    r2.w = ((r2.wwww)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).w;
    // 60: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 61: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 63: mad r2.w, r2.w, l(0.010000), r8.x
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(r8.xxxx)).w;
    // 64: lt r3.w, cb0[21].w, r2.w
    r3.w = (asfloat((uint4)((source[21].wwww)<(r2.wwww)) * 0xffffffffu)).w;
    // 65: lt r2.w, r2.w, cb0[21].z
    r2.w = (asfloat((uint4)((r2.wwww)<(source[21].zzzz)) * 0xffffffffu)).w;
    // 66: movc r2.w, r2.w, l(-1.000000), l(-0.000000)
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).w;
    // 67: and r4.w, r3.w, l(0x3f800000)
    r4.w = (asfloat(asuint(r3.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 68: movc r3.w, r3.w, l(0), l(1.000000)
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: add r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)+(r3.wwww)).w;
    // 70: mad r3.xyz, r4.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r4.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 71: add r3.w, r8.y, r4.w
    r3.w = ((r8.yyyy)+(r4.wwww)).w;
    // 72: add r3.w, r8.z, r3.w
    r3.w = ((r8.zzzz)+(r3.wwww)).w;
    // 73: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 74: mul r7.xyz, cb0[6].xyzx, cb0[6].wwww
    r7.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 75: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 76: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: max r9.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 78: add r9.xyz, -r7.xyzx, r9.xyzx
    r9.xyz = ((-(r7.xyzx))+(r9.xyzx)).xyz;
    // 79: mad r7.xyz, r1.wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 80: add r9.xyz, -r3.xyzx, r7.xyzx
    r9.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 81: mad r3.xyz, r2.wwww, r9.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r9.xyz, cb0[7].xyzx, cb0[7].wwww
    r9.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 83: max r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 84: max r9.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 85: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 86: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 87: add r10.xyz, -r9.xyzx, r10.xyzx
    r10.xyz = ((-(r9.xyzx))+(r10.xyzx)).xyz;
    // 88: mad r9.xyz, r1.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r1.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 89: add r10.xyz, -r3.xyzx, r9.xyzx
    r10.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 90: mad r3.xyz, r8.yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((r8.yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 91: mul r10.xyz, cb0[8].xyzx, cb0[8].wwww
    r10.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 92: max r11.xyz, r10.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r11.xyz = (max(r10.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 93: max r10.xyz, r10.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 94: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 95: min r11.xyz, r11.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r11.xyz = (min(r11.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 96: add r11.xyz, -r10.xyzx, r11.xyzx
    r11.xyz = ((-(r10.xyzx))+(r11.xyzx)).xyz;
    // 97: mad r10.xyz, r1.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r1.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 98: add r11.xyz, -r3.xyzx, r10.xyzx
    r11.xyz = ((-(r3.xyzx))+(r10.xyzx)).xyz;
    // 99: mad r3.xyz, r8.zzzz, r11.xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 100: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: add r11.xyz, -r3.xyzx, r4.wwww
    r11.xyz = ((-(r3.xyzx))+(r4.wwww)).xyz;
    // 102: mad r11.xyz, cb0[24].wwww, r11.xyzx, r3.xyzx
    r11.xyz = ((source[24].wwww)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 103: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 104: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 105: add r3.xyz, -r11.xyzx, r3.xxxx
    r3.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 106: mad r3.xyz, cb0[25].xxxx, r3.xyzx, r11.xyzx
    r3.xyz = ((source[25].xxxx)*(r3.xyzx)+(r11.xyzx)).xyz;
    // 107: mad r11.xyz, cb0[13].wwww, cb0[13].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[13].wwww)*(source[13].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 108: mad r12.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 109: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 110: mul r3.xyz, r3.xyzx, r11.xyzx
    r3.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 111: mul r12.xyz, r2.xyzx, r3.xyzx
    r12.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 112: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: mad r2.xyz, -r3.xyzx, r2.xyzx, r4.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r4.wwww)).xyz;
    // 114: mad r2.xyz, cb0[24].wwww, r2.xyzx, r12.xyzx
    r2.xyz = ((source[24].wwww)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 115: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: add r3.xyz, -r2.xyzx, r3.xxxx
    r3.xyz = ((-(r2.xyzx))+(r3.xxxx)).xyz;
    // 117: mad r2.xyz, cb0[25].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 118: mul r2.xyz, r11.xyzx, r2.xyzx
    r2.xyz = ((r11.xyzx)*(r2.xyzx)).xyz;
    // 119: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 120: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 121: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 122: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 123: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 124: max r0.x, r0.x, cb0[26].x
    r0.x = (max(r0.xxxx,source[26].xxxx)).x;
    // 125: min r0.x, r0.x, cb0[25].w
    r0.x = (min(r0.xxxx,source[25].wwww)).x;
    // 126: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 127: mad r0.x, r1.w, r2.x, r0.x
    r0.x = ((r1.wwww)*(r2.xxxx)+(r0.xxxx)).x;
    // 128: mul_sat r12.w, r1.w, cb2[3].w
    r12.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 129: add r1.w, r0.x, l(-1.000000)
    r1.w = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 130: mad r1.w, cb0[26].z, r1.w, l(1.000000)
    r1.w = ((source[26].zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r2.xyz, r1.xyzx, r1.wwww
    r2.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 132: mul r3.x, r6.x, cb0[25].y
    r3.x = ((r6.xxxx)*(source[25].yyyy)).x;
    // 133: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 134: movc r3.x, r5.x, l(0), r3.x
    r3.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 135: add_sat r3.x, r3.x, cb0[25].z
    r3.x = (saturate((r3.xxxx)+(source[25].zzzz))).x;
    // 136: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: mul r5.xyw, r3.yyyy, cb0[18].xyxz
    r5.xyw = ((r3.yyyy)*(source[18].xyxz)).xyw;
    // 138: mul r2.xyz, r2.xyzx, r5.xywx
    r2.xyz = ((r2.xyzx)*(r5.xywx)).xyz;
    // 139: mad r1.xyz, r1.wwww, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 140: mad r1.xyz, r3.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 141: mul_sat r0.x, r0.x, r3.x
    r0.x = (saturate((r0.xxxx)*(r3.xxxx))).x;
    // 142: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 143: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 144: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 145: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 147: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 148: dp2 r3.x, r2.yzyy, r2.yzyy
    r3.x = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).x;
    // 149: mul r13.xy, r2.yzyy, cb0[21].xxxx
    r13.xy = ((r2.yzyy)*(source[21].xxxx)).xy;
    // 150: add r2.y, -r3.x, l(1.000000)
    r2.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 151: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 152: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 153: add r13.z, r2.y, l(0.000010)
    r13.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 154: dp3 r2.y, r13.xyzx, r13.xyzx
    r2.y = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 155: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 156: div r3.xyz, r13.xyzx, r2.yyyy
    r3.xyz = ((r13.xyzx)/(r2.yyyy)).xyz;
    // 157: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 158: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 159: mul r5.xyw, r2.yyyy, r3.xyxz
    r5.xyw = ((r2.yyyy)*(r3.xyxz)).xyw;
    // 160: dp3 r2.y, v1.xyzx, v1.xyzx
    r2.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 161: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 162: mul r6.xyw, r2.yyyy, v1.xyxz
    r6.xyw = ((r2.yyyy)*(v1.xyxz)).xyw;
    // 163: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 164: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 165: mul r13.xyz, r2.yyyy, v0.xyzx
    r13.xyz = ((r2.yyyy)*(v0.xyzx)).xyz;
    // 166: mul r14.xyz, r6.wxyw, r13.yzxy
    r14.xyz = ((r6.wxyw)*(r13.yzxy)).xyz;
    // 167: mad r14.xyz, r6.ywxy, r13.zxyz, -r14.xyzx
    r14.xyz = ((r6.ywxy)*(r13.zxyz)+(-(r14.xyzx))).xyz;
    // 168: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 169: dp3 r15.y, r14.xyzx, r5.xywx
    r15.y = (dot((r14.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 170: dp3 r15.x, r13.xyzx, r5.xywx
    r15.x = (dot((r13.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 171: dp2 r16.z, r15.xyxx, cb0[31].xyxx
    r16.z = (dot((r15.xyxx).xy,(source[31].xyxx).xy).xxxx).z;
    // 172: mul r2.yz, cb0[31].yyxy, l(0.000000, 1.000000, -1.000000, 0.000000)
    r2.yz = ((source[31].yyxy)*(float4(0.000000,1.000000,-1.000000,0.000000))).yz;
    // 173: dp2 r16.x, r15.xyxx, r2.yzyy
    r16.x = (dot((r15.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 174: dp3 r16.y, r6.xywx, r5.xywx
    r16.y = (dot((r6.xywx).xyz,(r5.xywx).xyz).xxxx).y;
    // 175: mov r16.w, l(1.000000)
    r16.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 176: dp4 r17.x, cb0[32].xyzw, r16.xyzw
    r17.x = (dot((source[32].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 177: dp4 r17.y, cb0[33].xyzw, r16.xyzw
    r17.y = (dot((source[33].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 178: dp4 r17.z, cb0[34].xyzw, r16.xyzw
    r17.z = (dot((source[34].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 179: mul r18.xyzw, r16.yzzx, r16.xyzz
    r18.xyzw = ((r16.yzzx)*(r16.xyzz)).xyzw;
    // 180: dp4 r19.x, cb0[35].xyzw, r18.xyzw
    r19.x = (dot((source[35].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).x;
    // 181: dp4 r19.y, cb0[36].xyzw, r18.xyzw
    r19.y = (dot((source[36].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).y;
    // 182: dp4 r19.z, cb0[37].xyzw, r18.xyzw
    r19.z = (dot((source[37].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).z;
    // 183: add r17.xyz, r17.xyzx, r19.xyzx
    r17.xyz = ((r17.xyzx)+(r19.xyzx)).xyz;
    // 184: mul r4.w, r16.y, r16.y
    r4.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 185: mov r15.z, r16.y
    r15.z = (r16.yyyy).z;
    // 186: mad r4.w, r16.x, r16.x, -r4.w
    r4.w = ((r16.xxxx)*(r16.xxxx)+(-(r4.wwww))).w;
    // 187: mad r16.xyz, cb0[38].xyzx, r4.wwww, r17.xyzx
    r16.xyz = ((source[38].xyzx)*(r4.wwww)+(r17.xyzx)).xyz;
    // 188: max r16.xyz, r16.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r16.xyz = (max(r16.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 189: mul r16.xyz, r16.xyzx, cb0[30].xyzx
    r16.xyz = ((r16.xyzx)*(source[30].xyzx)).xyz;
    // 190: mul r16.xyz, r16.xyzx, cb0[31].zzzz
    r16.xyz = ((r16.xyzx)*(source[31].zzzz)).xyz;
    // 191: mad r16.xyz, r16.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[30].wwww
    r16.xyz = ((r16.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[30].wwww)).xyz;
    // 192: dp3 r4.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 193: add r16.xyz, -r4.wwww, r16.xyzx
    r16.xyz = ((-(r4.wwww))+(r16.xyzx)).xyz;
    // 194: mad r16.xyz, r16.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r4.wwww
    r16.xyz = ((r16.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r4.wwww)).xyz;
    // 195: dp3 r4.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: add r7.w, -cb0[27].w, cb0[27].z
    r7.w = ((-(source[27].wwww))+(source[27].zzzz)).w;
    // 197: mad r7.w, r8.x, r7.w, cb0[27].w
    r7.w = ((r8.xxxx)*(r7.wwww)+(source[27].wwww)).w;
    // 198: add r9.w, -r7.w, cb0[28].y
    r9.w = ((-(r7.wwww))+(source[28].yyyy)).w;
    // 199: mad r7.w, r8.y, r9.w, r7.w
    r7.w = ((r8.yyyy)*(r9.wwww)+(r7.wwww)).w;
    // 200: add r9.w, -r7.w, cb0[28].w
    r9.w = ((-(r7.wwww))+(source[28].wwww)).w;
    // 201: mad r7.w, r8.z, r9.w, r7.w
    r7.w = ((r8.zzzz)*(r9.wwww)+(r7.wwww)).w;
    // 202: add r9.w, -r7.w, cb0[29].y
    r9.w = ((-(r7.wwww))+(source[29].yyyy)).w;
    // 203: mad r7.w, r2.w, r9.w, r7.w
    r7.w = ((r2.wwww)*(r9.wwww)+(r7.wwww)).w;
    // 204: mul r6.z, r6.z, r7.w
    r6.z = ((r6.zzzz)*(r7.wwww)).z;
    // 205: exp r6.z, r6.z
    r6.z = (exp2(r6.zzzz)).z;
    // 206: min r6.z, r6.z, l(1.000000)
    r6.z = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: movc r5.z, r5.z, l(0), r6.z
    r5.z = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.zzzz)).z;
    // 208: max r5.z, r5.z, cb0[1].x
    r5.z = (max(r5.zzzz,source[1].xxxx)).z;
    // 209: min r12.z, r5.z, l(1.000000)
    r12.z = (min(r5.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 210: dp3 r5.z, v5.xyzx, v5.xyzx
    r5.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 211: rsq r5.z, r5.z
    r5.z = (rsqrt(r5.zzzz)).z;
    // 212: mul r17.xyz, r5.zzzz, v5.xyzx
    r17.xyz = ((r5.zzzz)*(v5.xyzx)).xyz;
    // 213: dp3 r5.z, r5.xywx, r17.xyzx
    r5.z = (dot((r5.xywx).xyz,(r17.xyzx).xyz).xxxx).z;
    // 214: deriv_rtx_coarse r12.x, r5.z
    r12.x = (ddx_coarse(r5.zzzz)).x;
    // 215: deriv_rty_coarse r12.y, r5.z
    r12.y = (ddy_coarse(r5.zzzz)).y;
    // 216: dp2 r6.z, r12.xyxx, r12.xyxx
    r6.z = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).z;
    // 217: sqrt r6.z, r6.z
    r6.z = (sqrt(r6.zzzz)).z;
    // 218: mad r6.z, r6.z, l(0.300000), r12.z
    r6.z = ((r6.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r12.zzzz)).z;
    // 219: mov o2.zw, r12.zzzw
    output.targets[2].zw = (r12.zzzw).zw;
    // 220: min r12.y, r6.z, l(1.000000)
    r12.y = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 221: mad r6.z, r12.y, l(2.000000), l(2.000000)
    r6.z = ((r12.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).z;
    // 222: div r4.w, r4.w, r6.z
    r4.w = ((r4.wwww)/(r6.zzzz)).w;
    // 223: mad r4.w, r2.x, l(5.000000), r4.w
    r4.w = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r4.wwww)).w;
    // 224: add_sat r4.w, r12.w, r4.w
    r4.w = (saturate((r12.wwww)+(r4.wwww))).w;
    // 225: mad r7.w, r4.w, l(-2.000000), l(3.000000)
    r7.w = ((r4.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 226: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 227: mul r4.w, r4.w, r7.w
    r4.w = ((r4.wwww)*(r7.wwww)).w;
    // 228: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 229: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 230: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 231: mul r16.xyz, r4.wwww, r16.xyzx
    r16.xyz = ((r4.wwww)*(r16.xyzx)).xyz;
    // 232: mov_sat r1.w, cb0[26].w
    r1.w = (saturate(source[26].wwww)).w;
    // 233: mad r18.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r18.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 234: mul r4.w, r1.w, l(0.080000)
    r4.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 235: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 236: mad r18.xyz, r12.wwww, r18.xyzx, r4.wwww
    r18.xyz = ((r12.wwww)*(r18.xyzx)+(r4.wwww)).xyz;
    // 237: mul_sat r1.w, r18.y, l(50.000000)
    r1.w = (saturate((r18.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 238: add r4.w, -r12.y, l(1.000000)
    r4.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: max r19.xyz, r18.xyzx, r4.wwww
    r19.xyz = (max(r18.xyzx,r4.wwww)).xyz;
    // 240: add r19.xyz, -r18.xyzx, r19.xyzx
    r19.xyz = ((-(r18.xyzx))+(r19.xyzx)).xyz;
    // 241: mul r19.xyz, r1.wwww, r19.xyzx
    r19.xyz = ((r1.wwww)*(r19.xyzx)).xyz;
    // 242: mul r20.xyz, r5.zzzz, r5.xywx
    r20.xyz = ((r5.zzzz)*(r5.xywx)).xyz;
    // 243: mad r20.xyz, r20.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r17.xyzx
    r20.xyz = ((r20.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r17.xyzx))).xyz;
    // 244: add r1.w, r20.z, l(1.000000)
    r1.w = ((r20.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 245: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: add r4.w, r5.z, l(1.000000)
    r4.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 247: mov_sat r5.z, r5.z
    r5.z = (saturate(r5.zzzz)).z;
    // 248: log r5.z, r5.z
    r5.z = (log2(r5.zzzz)).z;
    // 249: mul r5.z, r5.z, cb0[2].y
    r5.z = ((r5.zzzz)*(source[2].yyyy)).z;
    // 250: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 251: mad_sat r5.z, r5.z, cb0[2].w, cb0[2].z
    r5.z = (saturate((r5.zzzz)*(source[2].wwww)+(source[2].zzzz))).z;
    // 252: mul r5.z, r5.z, cb0[29].z
    r5.z = ((r5.zzzz)*(source[29].zzzz)).z;
    // 253: add_sat r12.x, -r1.w, r4.w
    r12.x = (saturate((-(r1.wwww))+(r4.wwww))).x;
    // 254: sample_indexable(texture2d)(float,float,float,float) r21.xy, r12.xyxx, t7.xyzw, s8
    r21.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 255: add r1.w, r0.x, r12.x
    r1.w = ((r0.xxxx)+(r12.xxxx)).w;
    // 256: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 257: mul r22.xyz, r18.xyzx, r21.yyyy
    r22.xyz = ((r18.xyzx)*(r21.yyyy)).xyz;
    // 258: mad r19.xyz, r19.xyzx, r21.xxxx, r22.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xxxx)+(r22.xyzx)).xyz;
    // 259: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r21.y
    r4.w = r21.y != 0.f ? 1.f / r21.y : 0.f;
    // 260: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 261: mad r21.xyz, r18.xyzx, r4.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r21.xyz = ((r18.xyzx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 262: dp3 r4.w, r18.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r18.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: mad r18.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r18.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 264: mad r22.xyz, -r19.xyzx, r21.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r22.xyz = ((-(r19.xyzx))*(r21.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 265: mul r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)*(r21.xyzx)).xyz;
    // 266: mul r21.xyz, r1.xyzx, r22.xyzx
    r21.xyz = ((r1.xyzx)*(r22.xyzx)).xyz;
    // 267: mul r22.xyz, r16.xyzx, r22.xyzx
    r22.xyz = ((r16.xyzx)*(r22.xyzx)).xyz;
    // 268: add r4.w, -r12.w, l(1.000000)
    r4.w = ((-(r12.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 269: mul r21.xyz, r4.wwww, r21.xyzx
    r21.xyz = ((r4.wwww)*(r21.xyzx)).xyz;
    // 270: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 271: mad r21.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r21.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 272: mad r23.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r23.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 273: mad r24.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r24.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 274: mad r23.xyz, r0.xxxx, r23.xyzx, r24.xyzx
    r23.xyz = ((r0.xxxx)*(r23.xyzx)+(r24.xyzx)).xyz;
    // 275: mad r21.xyz, r23.xyzx, r0.xxxx, r21.xyzx
    r21.xyz = ((r23.xyzx)*(r0.xxxx)+(r21.xyzx)).xyz;
    // 276: mul r21.xyz, r0.xxxx, r21.xyzx
    r21.xyz = ((r0.xxxx)*(r21.xyzx)).xyz;
    // 277: max r21.xyz, r0.xxxx, r21.xyzx
    r21.xyz = (max(r0.xxxx,r21.xyzx)).xyz;
    // 278: mul r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)*(r21.xyzx)).xyz;
    // 279: dp3 r14.y, r14.xyzx, r20.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 280: dp3 r14.x, r13.xyzx, r20.xyzx
    r14.x = (dot((r13.xyzx).xyz,(r20.xyzx).xyz).xxxx).x;
    // 281: dp2 r13.x, r14.xyxx, r2.yzyy
    r13.x = (dot((r14.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 282: dp2 r13.z, r14.xyxx, cb0[31].xyxx
    r13.z = (dot((r14.xyxx).xy,(source[31].xyxx).xy).xxxx).z;
    // 283: mul r2.y, r12.y, l(5.000000)
    r2.y = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 284: mul r2.z, r12.y, r12.y
    r2.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 285: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 286: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 287: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 288: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 289: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 290: dp3 r13.y, r6.xywx, r20.xyzx
    r13.y = (dot((r6.xywx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 291: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r13.xyzx, t8.xyzw, s7, r2.y
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r13.xyzx).xyz, (r2.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 292: mul r6.xyw, r13.xyxz, r13.wwww
    r6.xyw = ((r13.xyxz)*(r13.wwww)).xyw;
    // 293: mul r6.xyw, r6.xyxw, cb0[30].xyxz
    r6.xyw = ((r6.xyxw)*(source[30].xyxz)).xyw;
    // 294: mul r6.xyw, r6.xyxw, cb0[31].zzzz
    r6.xyw = ((r6.xyxw)*(source[31].zzzz)).xyw;
    // 295: mad r6.xyw, r6.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[30].wwww
    r6.xyw = ((r6.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[30].wwww)).xyw;
    // 296: dp3 r1.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 297: add r6.xyw, -r1.wwww, r6.xyxw
    r6.xyw = ((-(r1.wwww))+(r6.xyxw)).xyw;
    // 298: mad r6.xyw, r6.xyxw, l(0.800000, 0.800000, 0.000000, 0.800000), r1.wwww
    r6.xyw = ((r6.xyxw)*(float4(0.800000,0.800000,0.000000,0.800000))+(r1.wwww)).xyw;
    // 299: dp3 r1.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 300: div r1.w, r1.w, r6.z
    r1.w = ((r1.wwww)/(r6.zzzz)).w;
    // 301: mad r1.w, r2.x, l(5.000000), r1.w
    r1.w = ((r2.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 302: add_sat r1.w, r12.w, r1.w
    r1.w = (saturate((r12.wwww)+(r1.wwww))).w;
    // 303: mad r2.x, r1.w, l(-2.000000), l(3.000000)
    r2.x = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 304: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 305: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 306: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 307: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 308: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 309: mul r2.xyz, r1.wwww, r6.xywx
    r2.xyz = ((r1.wwww)*(r6.xywx)).xyz;
    // 310: mul r6.xyz, r2.xyzx, r19.xyzx
    r6.xyz = ((r2.xyzx)*(r19.xyzx)).xyz;
    // 311: mad r1.w, r0.x, r18.x, r18.y
    r1.w = ((r0.xxxx)*(r18.xxxx)+(r18.yyyy)).w;
    // 312: mad r1.w, r1.w, r0.x, r18.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r18.zzzz)).w;
    // 313: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 314: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 315: mad r6.xyz, r6.xyzx, r0.xxxx, r16.xyzx
    r6.xyz = ((r6.xyzx)*(r0.xxxx)+(r16.xyzx)).xyz;
    // 316: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 317: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 318: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 319: dp3 r1.w, r12.xyzx, r5.xywx
    r1.w = (dot((r12.xyzx).xyz,(r5.xywx).xyz).xxxx).w;
    // 320: dp3 r5.x, -r12.xyzx, r5.xywx
    r5.x = (dot((-(r12.xyzx)).xyz,(r5.xywx).xyz).xxxx).x;
    // 321: dp3 r5.y, r12.xyzx, r20.xyzx
    r5.y = (dot((r12.xyzx).xyz,(r20.xyzx).xyz).xxxx).y;
    // 322: mad r5.yw, r5.yyyy, l(0.000000, 0.500000, 0.000000, -0.500000), l(0.000000, 0.500000, 0.000000, 0.500000)
    r5.yw = ((r5.yyyy)*(float4(0.000000,0.500000,0.000000,-0.500000))+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 323: mul r5.yw, r5.yyyw, r5.yyyw
    r5.yw = ((r5.yyyw)*(r5.yyyw)).yw;
    // 324: mad r12.xy, r5.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r5.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 325: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 326: mad r13.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r13.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 327: mul r13.xy, r13.xyxx, r13.xyxx
    r13.xy = ((r13.xyxx)*(r13.xyxx)).xy;
    // 328: mul r13.yzw, r13.yyyy, cb0[41].xxyz
    r13.yzw = ((r13.yyyy)*(source[41].xxyz)).yzw;
    // 329: mad r13.xyz, r13.xxxx, cb0[40].xyzx, r13.yzwy
    r13.xyz = ((r13.xxxx)*(source[40].xyzx)+(r13.yzwy)).xyz;
    // 330: mul r13.xyz, r13.xyzx, cb0[42].wwww
    r13.xyz = ((r13.xyzx)*(source[42].wwww)).xyz;
    // 331: mul r13.xyz, r1.xyzx, r13.xyzx
    r13.xyz = ((r1.xyzx)*(r13.xyzx)).xyz;
    // 332: mul r13.xyz, r21.xyzx, r13.xyzx
    r13.xyz = ((r21.xyzx)*(r13.xyzx)).xyz;
    // 333: mul r13.xyz, r13.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 334: mul r13.xyz, r22.xyzx, r13.xyzx
    r13.xyz = ((r22.xyzx)*(r13.xyzx)).xyz;
    // 335: mad r13.xyz, -r13.xyzx, r12.wwww, r13.xyzx
    r13.xyz = ((-(r13.xyzx))*(r12.wwww)+(r13.xyzx)).xyz;
    // 336: mad r6.xyz, r6.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r13.xyzx
    r6.xyz = ((r6.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r13.xyzx)).xyz;
    // 337: mul r13.xyz, r5.wwww, cb0[41].xyzx
    r13.xyz = ((r5.wwww)*(source[41].xyzx)).xyz;
    // 338: mad r5.xyw, cb0[40].xyxz, r5.yyyy, r13.xyxz
    r5.xyw = ((source[40].xyxz)*(r5.yyyy)+(r13.xyxz)).xyw;
    // 339: mul r5.xyw, r5.xyxw, cb0[42].wwww
    r5.xyw = ((r5.xyxw)*(source[42].wwww)).xyw;
    // 340: mul r5.xyw, r0.xxxx, r5.xyxw
    r5.xyw = ((r0.xxxx)*(r5.xyxw)).xyw;
    // 341: mul r2.xyz, r2.xyzx, r5.xywx
    r2.xyz = ((r2.xyzx)*(r5.xywx)).xyz;
    // 342: mul r2.xyz, r2.xyzx, r19.xyzx
    r2.xyz = ((r2.xyzx)*(r19.xyzx)).xyz;
    // 343: mad r5.xyw, r2.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r6.xyxz
    r5.xyw = ((r2.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r6.xyxz)).xyw;
    // 344: mul r2.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 345: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 346: mad r2.xyz, -r8.xxxx, r4.xyzx, r7.xyzx
    r2.xyz = ((-(r8.xxxx))*(r4.xyzx)+(r7.xyzx)).xyz;
    // 347: mul r4.xyz, r4.xyzx, r8.xxxx
    r4.xyz = ((r4.xyzx)*(r8.xxxx)).xyz;
    // 348: mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 349: add r4.xyz, -r2.xyzx, r9.xyzx
    r4.xyz = ((-(r2.xyzx))+(r9.xyzx)).xyz;
    // 350: mad r2.xyz, r8.yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((r8.yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 351: add r4.xyz, -r2.xyzx, r10.xyzx
    r4.xyz = ((-(r2.xyzx))+(r10.xyzx)).xyz;
    // 352: mad r2.xyz, r8.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r8.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 353: add r4.xyz, -r2.xyzx, r7.xyzx
    r4.xyz = ((-(r2.xyzx))+(r7.xyzx)).xyz;
    // 354: mad r2.xyz, r2.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 355: add r4.xyz, r2.xyzx, -cb0[10].xyzx
    r4.xyz = ((r2.xyzx)+(-(source[10].xyzx))).xyz;
    // 356: add r2.xyz, r2.xyzx, -cb0[12].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[12].xyzx))).xyz;
    // 357: mad r2.xyz, r3.wwww, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r3.wwww)*(r2.xyzx)+(source[12].xyzx)).xyz;
    // 358: mad r4.xyz, r3.wwww, r4.xyzx, cb0[10].xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)+(source[10].xyzx)).xyz;
    // 359: mul r4.xyz, r4.xyzx, cb0[22].yyyy
    r4.xyz = ((r4.xyzx)*(source[22].yyyy)).xyz;
    // 360: mul r2.xyz, r2.xyzx, cb0[23].wwww
    r2.xyz = ((r2.xyzx)*(source[23].wwww)).xyz;
    // 361: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 362: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 363: mul r6.xy, v4.xyxx, cb0[11].zzzz
    r6.xy = ((v4.xyxx)*(source[11].zzzz)).xy;
    // 364: mul r7.xy, cb0[11].xyxx, cb0[23].xxxx
    r7.xy = ((source[11].xyxx)*(source[23].xxxx)).xy;
    // 365: mad r6.xy, r7.xyxx, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r7.xyxx)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 366: mad r7.xy, cb0[11].zzzz, v4.xyxx, r7.xyxx
    r7.xy = ((source[11].zzzz)*(v4.xyxx)+(r7.xyxx)).xy;
    // 367: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r6.xyxx, t6.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 368: mad r6.xy, r0.xxxx, cb0[23].zzzz, r7.xyxx
    r6.xy = ((r0.xxxx)*(source[23].zzzz)+(r7.xyxx)).xy;
    // 369: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t6.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 370: mul r6.xyz, r6.xyzx, r6.wwww
    r6.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 371: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 372: dp3 r0.x, r3.xyzx, r17.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r17.xyzx).xyz).xxxx).x;
    // 373: mul_sat r1.w, r0.x, cb0[24].x
    r1.w = (saturate((r0.xxxx)*(source[24].xxxx))).w;
    // 374: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 375: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 376: mul_sat r2.w, r17.z, cb0[24].x
    r2.w = (saturate((r17.zzzz)*(source[24].xxxx))).w;
    // 377: add r3.x, -|r17.z|, l(1.000000)
    r3.x = ((-(abs(r17.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 378: mul r0.x, r0.x, r3.x
    r0.x = ((r0.xxxx)*(r3.xxxx)).x;
    // 379: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 380: add_sat r2.w, r2.w, -cb0[24].y
    r2.w = (saturate((r2.wwww)+(-(source[24].yyyy)))).w;
    // 381: log r3.x, r2.w
    r3.x = (log2(r2.wwww)).x;
    // 382: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 383: mul r3.x, r3.x, cb0[24].z
    r3.x = ((r3.xxxx)*(source[24].zzzz)).x;
    // 384: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 385: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 386: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 387: mad r3.xyz, r1.wwww, r2.xyzx, -r2.xyzx
    r3.xyz = ((r1.wwww)*(r2.xyzx)+(-(r2.xyzx))).xyz;
    // 388: mad r2.xyz, cb0[12].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[12].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 389: add r2.w, cb0[0].y, cb0[0].x
    r2.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 390: add r2.w, r2.w, cb0[0].z
    r2.w = ((r2.wwww)+(source[0].zzzz)).w;
    // 391: add r3.x, -r2.w, l(1000.000000)
    r3.x = ((-(r2.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 392: mad r2.w, cb0[23].y, r3.x, r2.w
    r2.w = ((source[23].yyyy)*(r3.xxxx)+(r2.wwww)).w;
    // 393: mul r2.w, r2.w, l(0.010000)
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 394: mad r2.w, cb0[22].w, cb0[23].x, r2.w
    r2.w = ((source[22].wwww)*(source[23].xxxx)+(r2.wwww)).w;
    // 395: mul r3.x, r2.w, l(3.524534)
    r3.x = ((r2.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 396: sincos null, r3.x, r3.x
    r3.x = (cos(r3.xxxx)).x;
    // 397: add r2.w, r2.w, r3.x
    r2.w = ((r2.wwww)+(r3.xxxx)).w;
    // 398: mul r2.w, r2.w, l(1.328987)
    r2.w = ((r2.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 399: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 400: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 401: mad r2.w, r2.w, l(0.500000), cb0[22].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[22].zzzz)).w;
    // 402: mad r2.xyz, r2.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 403: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 404: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 405: mad r2.xyz, cb0[24].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[24].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 406: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 407: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 408: mad r2.xyz, cb0[25].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 409: add r2.w, -r8.w, r1.w
    r2.w = ((-(r8.wwww))+(r1.wwww)).w;
    // 410: mad r3.xyz, r1.wwww, cb0[16].xyzx, -cb0[16].xyzx
    r3.xyz = ((r1.wwww)*(source[16].xyzx)+(-(source[16].xyzx))).xyz;
    // 411: mad r3.xyz, cb0[16].wwww, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((source[16].wwww)*(r3.xyzx)+(source[16].xyzx)).xyz;
    // 412: mad r1.w, cb0[15].w, r2.w, r8.w
    r1.w = ((source[15].wwww)*(r2.wwww)+(r8.wwww)).w;
    // 413: mad r3.xyz, r1.wwww, cb0[15].xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(source[15].xyzx)+(r3.xyzx)).xyz;
    // 414: mad r2.xyz, r2.xyzx, r11.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 415: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 416: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 417: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 418: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 419: mul r3.xyz, r1.wwww, cb0[17].xyzx
    r3.xyz = ((r1.wwww)*(source[17].xyzx)).xyz;
    // 420: movc r3.xyz, r0.xxxx, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 421: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 422: mad r0.xyz, cb0[22].xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((source[22].xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 423: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 424: mul r2.xyz, r12.yyyy, cb0[41].xyzx
    r2.xyz = ((r12.yyyy)*(source[41].xyzx)).xyz;
    // 425: mad r2.xyz, r12.xxxx, cb0[40].xyzx, r2.xyzx
    r2.xyz = ((r12.xxxx)*(source[40].xyzx)+(r2.xyzx)).xyz;
    // 426: mul r2.xyz, r2.xyzx, cb0[42].wwww
    r2.xyz = ((r2.xyzx)*(source[42].wwww)).xyz;
    // 427: mul_sat r3.xyz, cb0[20].xyzx, cb0[20].wwww
    r3.xyz = (saturate((source[20].xyzx)*(source[20].wwww))).xyz;
    // 428: mul r4.xyz, r3.xyzx, r5.zzzz
    r4.xyz = ((r3.xyzx)*(r5.zzzz)).xyz;
    // 429: mul r3.xyz, r3.xyzx, cb0[29].zzzz
    r3.xyz = ((r3.xyzx)*(source[29].zzzz)).xyz;
    // 430: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 431: mul r3.xyz, r4.wwww, r4.xyzx
    r3.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 432: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 433: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 434: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 435: add r0.xyz, r5.xywx, r0.xyzx
    r0.xyz = ((r5.xywx)+(r0.xyzx)).xyz;
    // 436: dp3 o4.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 437: mad o0.xyz, r1.xyzx, cb0[42].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[42].xyzx)+(r0.xyzx)).xyz;
    // 438: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 439: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 440: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 441: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 442: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 443: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 444: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 445: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 446: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 447: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 448: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 449: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 450: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 451: ftou r0.x, cb0[39].z
    r0.x = (asfloat((uint4)(source[39].zzzz))).x;
    // 452: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 453: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 454: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 455: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 456: ret
    return output;
}

// source.character.equipment-native-189.v1 / source program 0145f5865751a04db5c56fd299a16ad8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase189(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[26]=g_SourceCharacterEnvironmentColor; source[27]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0;
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
    // 7: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 8: mul r1.x, r0.x, l(0.125000)
    r1.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 9: mul r2.y, cb0[8].y, cb0[16].y
    r2.y = ((source[8].yyyy)*(source[16].yyyy)).y;
    // 10: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 11: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 12: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 13: frc r0.x, cb0[8].x
    r0.x = (frac(source[8].xxxx)).x;
    // 14: add r1.z, -r0.x, cb0[8].x
    r1.z = ((-(r0.xxxx))+(source[8].xxxx)).z;
    // 15: mul r2.z, r1.z, l(0.125000)
    r2.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 16: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t4.xyzw, s5, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 19: add r1.w, -cb0[8].w, l(1.000000)
    r1.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mul r1.w, r1.w, cb0[19].z
    r1.w = ((r1.wwww)*(source[19].zzzz)).w;
    // 21: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 22: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 23: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r2.x, cb0[8].z, l(1.500000)
    r2.x = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mad r1.w, r1.w, l(0.500000), cb0[8].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 27: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 28: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 30: mad r2.xyz, cb0[20].xxxx, r2.xyzx, r0.yzwy
    r2.xyz = ((source[20].xxxx)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 33: mad r2.xyz, cb0[20].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.w, r6.y, cb0[18].y
    r1.w = ((r6.yyyy)*(source[18].yyyy)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[4].xyzx, cb0[4].wwww
    r4.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: mul r7.xyz, cb0[5].xyzx, cb0[5].wwww
    r7.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 56: max r8.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 57: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 58: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 59: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: add r8.xyz, -r7.xyzx, r8.xyzx
    r8.xyz = ((-(r7.xyzx))+(r8.xyzx)).xyz;
    // 61: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 62: add r8.xyz, -r4.xyzx, r7.xyzx
    r8.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 64: mad r4.xyz, r9.xxxx, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.xxxx)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 65: add r8.xyz, r3.xyzx, -r4.xyzx
    r8.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 66: mad r3.xyz, -r9.xxxx, r7.xyzx, r3.xyzx
    r3.xyz = ((-(r9.xxxx))*(r7.xyzx)+(r3.xyzx)).xyz;
    // 67: mul r7.xyz, r7.xyzx, r9.xxxx
    r7.xyz = ((r7.xyzx)*(r9.xxxx)).xyz;
    // 68: mad r3.xyz, r9.yyyy, r3.xyzx, r7.xyzx
    r3.xyz = ((r9.yyyy)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 69: mad r4.xyz, r9.yyyy, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.yyyy)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 70: mul r7.xyz, cb0[7].xyzx, cb0[7].wwww
    r7.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 71: max r8.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 72: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 73: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 74: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 75: add r8.xyz, -r7.xyzx, r8.xyzx
    r8.xyz = ((-(r7.xyzx))+(r8.xyzx)).xyz;
    // 76: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 77: add r8.xyz, -r4.xyzx, r7.xyzx
    r8.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 78: mad r4.xyz, r9.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 79: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 80: mad r3.xyz, r9.zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((r9.zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 81: add r3.xyz, r3.xyzx, -cb0[9].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[9].xyzx))).xyz;
    // 82: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r7.xyz, -r4.xyzx, r2.wwww
    r7.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 84: mad r7.xyz, cb0[20].xxxx, r7.xyzx, r4.xyzx
    r7.xyz = ((source[20].xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 85: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 86: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 87: add r4.xyz, -r7.xyzx, r2.wwww
    r4.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 88: mad r4.xyz, cb0[20].yyyy, r4.xyzx, r7.xyzx
    r4.xyz = ((source[20].yyyy)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 89: mad r7.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 90: mad r8.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 91: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 92: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 93: mul r8.xyz, r2.xyzx, r4.xyzx
    r8.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 94: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: mad r2.xyz, -r4.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r4.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 96: mad r2.xyz, cb0[20].xxxx, r2.xyzx, r8.xyzx
    r2.xyz = ((source[20].xxxx)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 97: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r4.xyz, -r2.xyzx, r2.wwww
    r4.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 99: mad r2.xyz, cb0[20].yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 100: mul r2.xyz, r7.xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 101: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 102: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 103: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 104: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 105: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 106: max r0.x, r0.x, cb0[22].x
    r0.x = (max(r0.xxxx,source[22].xxxx)).x;
    // 107: min r0.x, r0.x, cb0[21].w
    r0.x = (min(r0.xxxx,source[21].wwww)).x;
    // 108: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 109: mad r0.x, r1.w, r2.x, r0.x
    r0.x = ((r1.wwww)*(r2.xxxx)+(r0.xxxx)).x;
    // 110: mul_sat r2.w, r1.w, cb2[3].w
    r2.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 111: add r1.w, r0.x, l(-1.000000)
    r1.w = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 112: mad r1.w, cb0[22].z, r1.w, l(1.000000)
    r1.w = ((source[22].zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mul r4.xyz, r1.xyzx, r1.wwww
    r4.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 114: mul r2.x, r6.x, cb0[21].y
    r2.x = ((r6.xxxx)*(source[21].yyyy)).x;
    // 115: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 116: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 117: add_sat r2.x, r2.x, cb0[21].z
    r2.x = (saturate((r2.xxxx)+(source[21].zzzz))).x;
    // 118: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 119: mul r5.xyw, r2.yyyy, cb0[15].xyxz
    r5.xyw = ((r2.yyyy)*(source[15].xyxz)).xyw;
    // 120: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 121: mad r1.xyz, r1.wwww, r1.xyzx, -r4.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(-(r4.xyzx))).xyz;
    // 122: mad r1.xyz, r2.xxxx, r1.xyzx, r4.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 123: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 124: add r4.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 125: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 126: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 127: mad r4.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 128: mad r5.xyw, r1.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r5.xyw = ((r1.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 129: mad r6.xyw, r1.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r6.xyw = ((r1.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 130: mad r5.xyw, r0.xxxx, r5.xyxw, r6.xyxw
    r5.xyw = ((r0.xxxx)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 131: mad r4.xyz, r5.xywx, r0.xxxx, r4.xyzx
    r4.xyz = ((r5.xywx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // 132: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 133: max r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = (max(r0.xxxx,r4.xyzx)).xyz;
    // 134: mov_sat r1.w, cb0[22].w
    r1.w = (saturate(source[22].wwww)).w;
    // 135: mad r5.xyw, -r1.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r1.xyxz
    r5.xyw = ((-(r1.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r1.xyxz)).xyw;
    // 136: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 137: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 138: mad r5.xyw, r2.wwww, r5.xyxw, r2.xxxx
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(r2.xxxx)).xyw;
    // 139: mul_sat r1.w, r5.y, l(50.000000)
    r1.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 140: add r2.x, -cb0[23].w, cb0[23].z
    r2.x = ((-(source[23].wwww))+(source[23].zzzz)).x;
    // 141: mad r2.x, r9.x, r2.x, cb0[23].w
    r2.x = ((r9.xxxx)*(r2.xxxx)+(source[23].wwww)).x;
    // 142: add r2.y, -r2.x, cb0[24].y
    r2.y = ((-(r2.xxxx))+(source[24].yyyy)).y;
    // 143: mad r2.x, r9.y, r2.y, r2.x
    r2.x = ((r9.yyyy)*(r2.yyyy)+(r2.xxxx)).x;
    // 144: add r2.y, -r2.x, cb0[24].w
    r2.y = ((-(r2.xxxx))+(source[24].wwww)).y;
    // 145: mad r2.x, r9.z, r2.y, r2.x
    r2.x = ((r9.zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 146: mul r2.x, r6.z, r2.x
    r2.x = ((r6.zzzz)*(r2.xxxx)).x;
    // 147: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 148: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 149: movc r2.x, r5.z, l(0), r2.x
    r2.x = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 150: max r2.x, r2.x, cb0[1].x
    r2.x = (max(r2.xxxx,source[1].xxxx)).x;
    // 151: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 152: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 153: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 154: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 155: mul r6.xy, r2.xyxx, cb0[18].xxxx
    r6.xy = ((r2.xyxx)*(source[18].xxxx)).xy;
    // 156: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 157: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 158: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 159: add r6.z, r2.x, l(0.000010)
    r6.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 160: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 161: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 162: div r6.xyz, r6.xyzx, r2.xxxx
    r6.xyz = ((r6.xyzx)/(r2.xxxx)).xyz;
    // 163: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 164: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 165: mul r8.xyz, r2.xxxx, r6.xyzx
    r8.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 166: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 167: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 168: mul r10.xyz, r2.xxxx, v5.xyzx
    r10.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 169: dp3 r2.x, r8.xyzx, r10.xyzx
    r2.x = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 170: deriv_rtx_coarse r11.x, r2.x
    r11.x = (ddx_coarse(r2.xxxx)).x;
    // 171: deriv_rty_coarse r11.y, r2.x
    r11.y = (ddy_coarse(r2.xxxx)).y;
    // 172: dp2 r2.y, r11.xyxx, r11.xyxx
    r2.y = (dot((r11.xyxx).xy,(r11.xyxx).xy).xxxx).y;
    // 173: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 174: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 175: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 176: min r11.y, r2.y, l(1.000000)
    r11.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 177: add r2.y, -r11.y, l(1.000000)
    r2.y = ((-(r11.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 178: max r12.xyz, r5.xywx, r2.yyyy
    r12.xyz = (max(r5.xywx,r2.yyyy)).xyz;
    // 179: add r12.xyz, -r5.xywx, r12.xyzx
    r12.xyz = ((-(r5.xywx))+(r12.xyzx)).xyz;
    // 180: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 181: mul r13.xyz, r2.xxxx, r8.xyzx
    r13.xyz = ((r2.xxxx)*(r8.xyzx)).xyz;
    // 182: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 183: add r1.w, r13.z, l(1.000000)
    r1.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 187: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 188: mul r2.x, r2.x, cb0[2].y
    r2.x = ((r2.xxxx)*(source[2].yyyy)).x;
    // 189: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 190: mad_sat r2.x, r2.x, cb0[2].w, cb0[2].z
    r2.x = (saturate((r2.xxxx)*(source[2].wwww)+(source[2].zzzz))).x;
    // 191: mul r2.x, r2.x, cb0[25].x
    r2.x = ((r2.xxxx)*(source[25].xxxx)).x;
    // 192: add_sat r11.x, -r1.w, r2.y
    r11.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 193: sample_indexable(texture2d)(float,float,float,float) r2.yz, r11.xyxx, t6.zxyw, s7
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 194: add r1.w, r0.x, r11.x
    r1.w = ((r0.xxxx)+(r11.xxxx)).w;
    // 195: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 196: mul r11.xzw, r2.zzzz, r5.xxyw
    r11.xzw = ((r2.zzzz)*(r5.xxyw)).xzw;
    // 197: mad r11.xzw, r12.xxyz, r2.yyyy, r11.xxzw
    r11.xzw = ((r12.xxyz)*(r2.yyyy)+(r11.xxzw)).xzw;
    // 198: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 199: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 200: mad r12.xyz, r5.xywx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 201: dp3 r2.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 202: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 203: mad r14.xyz, -r11.xzwx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r11.xzwx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 204: mul r11.xzw, r11.xxzw, r12.xxyz
    r11.xzw = ((r11.xxzw)*(r12.xxyz)).xzw;
    // 205: mul r12.xyz, r1.xyzx, r14.xyzx
    r12.xyz = ((r1.xyzx)*(r14.xyzx)).xyz;
    // 206: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 207: mul r12.xyz, r2.yyyy, r12.xyzx
    r12.xyz = ((r2.yyyy)*(r12.xyzx)).xyz;
    // 208: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 209: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 210: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 211: mul r15.xyz, r3.wwww, v1.xyzx
    r15.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 212: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 213: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 214: mul r16.xyz, r3.wwww, v0.xyzx
    r16.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 215: mul r17.xyz, r15.zxyz, r16.yzxy
    r17.xyz = ((r15.zxyz)*(r16.yzxy)).xyz;
    // 216: mad r17.xyz, r15.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r15.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 217: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 218: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 219: dp3 r17.y, r17.xyzx, r13.xyzx
    r17.y = (dot((r17.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 220: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 221: dp3 r17.x, r16.xyzx, r13.xyzx
    r17.x = (dot((r16.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 222: dp2 r16.z, r18.xyxx, cb0[27].xyxx
    r16.z = (dot((r18.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 223: mul r17.zw, cb0[27].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r17.zw = ((source[27].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 224: dp2 r16.x, r18.xyxx, r17.zwzz
    r16.x = (dot((r18.xyxx).xy,(r17.zwzz).xy).xxxx).x;
    // 225: dp2 r19.x, r17.xyxx, r17.zwzz
    r19.x = (dot((r17.xyxx).xy,(r17.zwzz).xy).xxxx).x;
    // 226: dp2 r19.z, r17.xyxx, cb0[27].xyxx
    r19.z = (dot((r17.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 227: dp3 r16.y, r15.xyzx, r8.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 228: dp3 r19.y, r15.xyzx, r13.xyzx
    r19.y = (dot((r15.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 229: mov r16.w, l(1.000000)
    r16.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 230: dp4 r15.x, cb0[28].xyzw, r16.xyzw
    r15.x = (dot((source[28].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 231: dp4 r15.y, cb0[29].xyzw, r16.xyzw
    r15.y = (dot((source[29].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 232: dp4 r15.z, cb0[30].xyzw, r16.xyzw
    r15.z = (dot((source[30].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 233: mul r17.xyzw, r16.yzzx, r16.xyzz
    r17.xyzw = ((r16.yzzx)*(r16.xyzz)).xyzw;
    // 234: dp4 r20.x, cb0[31].xyzw, r17.xyzw
    r20.x = (dot((source[31].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).x;
    // 235: dp4 r20.y, cb0[32].xyzw, r17.xyzw
    r20.y = (dot((source[32].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).y;
    // 236: dp4 r20.z, cb0[33].xyzw, r17.xyzw
    r20.z = (dot((source[33].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).z;
    // 237: add r15.xyz, r15.xyzx, r20.xyzx
    r15.xyz = ((r15.xyzx)+(r20.xyzx)).xyz;
    // 238: mul r3.w, r16.y, r16.y
    r3.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 239: mov r18.z, r16.y
    r18.z = (r16.yyyy).z;
    // 240: mad r3.w, r16.x, r16.x, -r3.w
    r3.w = ((r16.xxxx)*(r16.xxxx)+(-(r3.wwww))).w;
    // 241: mad r15.xyz, cb0[34].xyzx, r3.wwww, r15.xyzx
    r15.xyz = ((source[34].xyzx)*(r3.wwww)+(r15.xyzx)).xyz;
    // 242: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 243: mul r15.xyz, r15.xyzx, cb0[26].xyzx
    r15.xyz = ((r15.xyzx)*(source[26].xyzx)).xyz;
    // 244: mul r15.xyz, r15.xyzx, cb0[27].zzzz
    r15.xyz = ((r15.xyzx)*(source[27].zzzz)).xyz;
    // 245: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[26].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[26].wwww)).xyz;
    // 246: dp3 r3.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 247: add r15.xyz, -r3.wwww, r15.xyzx
    r15.xyz = ((-(r3.wwww))+(r15.xyzx)).xyz;
    // 248: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 249: dp3 r3.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 250: mad r4.w, r11.y, l(2.000000), l(2.000000)
    r4.w = ((r11.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 251: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 252: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 253: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 254: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 255: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 256: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 257: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 258: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 259: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 260: mul r15.xyz, r3.wwww, r15.xyzx
    r15.xyz = ((r3.wwww)*(r15.xyzx)).xyz;
    // 261: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 262: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 263: mul r12.xyz, r4.xyzx, r12.xyzx
    r12.xyz = ((r4.xyzx)*(r12.xyzx)).xyz;
    // 264: mul r3.w, r11.y, l(5.000000)
    r3.w = ((r11.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 265: mul r5.w, r11.y, r11.y
    r5.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 266: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 267: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 268: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 269: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 270: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 271: sample_l_indexable(texturecube)(float,float,float,float) r15.xyzw, r19.xyzx, t7.xyzw, s6, r3.w
    r15.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r19.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 272: mul r15.xyz, r15.xyzx, r15.wwww
    r15.xyz = ((r15.xyzx)*(r15.wwww)).xyz;
    // 273: mul r15.xyz, r15.xyzx, cb0[26].xyzx
    r15.xyz = ((r15.xyzx)*(source[26].xyzx)).xyz;
    // 274: mul r15.xyz, r15.xyzx, cb0[27].zzzz
    r15.xyz = ((r15.xyzx)*(source[27].zzzz)).xyz;
    // 275: mad r15.xyz, r15.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[26].wwww
    r15.xyz = ((r15.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[26].wwww)).xyz;
    // 276: dp3 r1.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 277: add r15.xyz, -r1.wwww, r15.xyzx
    r15.xyz = ((-(r1.wwww))+(r15.xyzx)).xyz;
    // 278: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 279: dp3 r1.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 280: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 281: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 282: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 283: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 284: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 285: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 286: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 287: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 288: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 289: mul r15.xyz, r1.wwww, r15.xyzx
    r15.xyz = ((r1.wwww)*(r15.xyzx)).xyz;
    // 290: mul r16.xyz, r11.xzwx, r15.xyzx
    r16.xyz = ((r11.xzwx)*(r15.xyzx)).xyz;
    // 291: mad r1.w, r0.x, r5.x, r5.y
    r1.w = ((r0.xxxx)*(r5.xxxx)+(r5.yyyy)).w;
    // 292: mad r1.w, r1.w, r0.x, r5.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r5.zzzz)).w;
    // 293: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 294: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 295: mad r5.xyz, r16.xyzx, r0.xxxx, r12.xyzx
    r5.xyz = ((r16.xyzx)*(r0.xxxx)+(r12.xyzx)).xyz;
    // 296: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 297: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 298: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 299: dp3 r1.w, r12.xyzx, r8.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 300: dp3 r2.z, -r12.xyzx, r8.xyzx
    r2.z = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).z;
    // 301: dp3 r3.w, r12.xyzx, r13.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 302: mad r8.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 303: mad r8.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 304: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 305: mad r12.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 306: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 307: mul r12.yzw, r12.yyyy, cb0[37].xxyz
    r12.yzw = ((r12.yyyy)*(source[37].xxyz)).yzw;
    // 308: mad r12.xyz, r12.xxxx, cb0[36].xyzx, r12.yzwy
    r12.xyz = ((r12.xxxx)*(source[36].xyzx)+(r12.yzwy)).xyz;
    // 309: mul r12.xyz, r12.xyzx, cb0[38].wwww
    r12.xyz = ((r12.xyzx)*(source[38].wwww)).xyz;
    // 310: mul r12.xyz, r1.xyzx, r12.xyzx
    r12.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 311: mul r4.xyz, r4.xyzx, r12.xyzx
    r4.xyz = ((r4.xyzx)*(r12.xyzx)).xyz;
    // 312: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 313: mul r4.xyz, r14.xyzx, r4.xyzx
    r4.xyz = ((r14.xyzx)*(r4.xyzx)).xyz;
    // 314: mad r4.xyz, -r4.xyzx, r2.wwww, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(r2.wwww)+(r4.xyzx)).xyz;
    // 315: mad r4.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r4.xyzx
    r4.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r4.xyzx)).xyz;
    // 316: mul r5.xyz, r8.yyyy, cb0[37].xyzx
    r5.xyz = ((r8.yyyy)*(source[37].xyzx)).xyz;
    // 317: mad r5.xyz, cb0[36].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[36].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 318: mul r5.xyz, r5.xyzx, cb0[38].wwww
    r5.xyz = ((r5.xyzx)*(source[38].wwww)).xyz;
    // 319: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 320: mul r5.xyz, r15.xyzx, r5.xyzx
    r5.xyz = ((r15.xyzx)*(r5.xyzx)).xyz;
    // 321: mul r5.xyz, r5.xyzx, r11.xzwx
    r5.xyz = ((r5.xyzx)*(r11.xzwx)).xyz;
    // 322: mad r4.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r4.xyzx
    r4.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r4.xyzx)).xyz;
    // 323: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 324: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 325: add r0.x, r9.y, r9.x
    r0.x = ((r9.yyyy)+(r9.xxxx)).x;
    // 326: add_sat r0.x, r9.z, r0.x
    r0.x = (saturate((r9.zzzz)+(r0.xxxx))).x;
    // 327: mad r3.xyz, r0.xxxx, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)+(source[9].xyzx)).xyz;
    // 328: mul r3.xyz, r3.xyzx, cb0[18].wwww
    r3.xyz = ((r3.xyzx)*(source[18].wwww)).xyz;
    // 329: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 330: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 331: add r0.x, cb0[0].y, cb0[0].x
    r0.x = ((source[0].yyyy)+(source[0].xxxx)).x;
    // 332: add r0.x, r0.x, cb0[0].z
    r0.x = ((r0.xxxx)+(source[0].zzzz)).x;
    // 333: add r1.w, -r0.x, l(1000.000000)
    r1.w = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 334: mad r0.x, cb0[19].w, r1.w, r0.x
    r0.x = ((source[19].wwww)*(r1.wwww)+(r0.xxxx)).x;
    // 335: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 336: mad r0.x, cb0[19].y, cb0[19].z, r0.x
    r0.x = ((source[19].yyyy)*(source[19].zzzz)+(r0.xxxx)).x;
    // 337: mul r1.w, r0.x, l(3.524534)
    r1.w = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 338: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 339: add r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)+(r1.wwww)).x;
    // 340: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 341: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 342: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 343: mad r0.x, r0.x, l(0.500000), cb0[19].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[19].xxxx)).x;
    // 344: mul r5.xyz, r3.xyzx, r0.xxxx
    r5.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 345: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 346: mad r3.xyz, -r0.xxxx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r0.xxxx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 347: mad r3.xyz, cb0[20].xxxx, r3.xyzx, r5.xyzx
    r3.xyz = ((source[20].xxxx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 348: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 349: add r5.xyz, -r3.xyzx, r0.xxxx
    r5.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 350: mad r3.xyz, cb0[20].yyyy, r5.xyzx, r3.xyzx
    r3.xyz = ((source[20].yyyy)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 351: dp3 r0.x, r6.xyzx, r10.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 352: mul_sat r1.w, r0.x, cb0[20].z
    r1.w = (saturate((r0.xxxx)*(source[20].zzzz))).w;
    // 353: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 354: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 355: mul_sat r2.z, r10.z, cb0[20].z
    r2.z = (saturate((r10.zzzz)*(source[20].zzzz))).z;
    // 356: add r2.w, -|r10.z|, l(1.000000)
    r2.w = ((-(abs(r10.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 357: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 358: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 359: add_sat r2.z, r2.z, -cb0[20].w
    r2.z = (saturate((r2.zzzz)+(-(source[20].wwww)))).z;
    // 360: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 361: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 362: mul r2.w, r2.w, cb0[21].x
    r2.w = ((r2.wwww)*(source[21].xxxx)).w;
    // 363: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 364: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 365: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 366: mad r5.xyz, r1.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r5.xyz = ((r1.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 367: mul r1.w, r1.w, cb0[12].w
    r1.w = ((r1.wwww)*(source[12].wwww)).w;
    // 368: mad r5.xyz, cb0[13].wwww, r5.xyzx, cb0[13].xyzx
    r5.xyz = ((source[13].wwww)*(r5.xyzx)+(source[13].xyzx)).xyz;
    // 369: mad r5.xyz, r1.wwww, cb0[12].xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(source[12].xyzx)+(r5.xyzx)).xyz;
    // 370: mad r3.xyz, r3.xyzx, r7.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 371: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 372: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 373: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 374: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 375: mul r5.xyz, r1.wwww, cb0[14].xyzx
    r5.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 376: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 377: add r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)+(r5.xyzx)).xyz;
    // 378: mad r0.xyz, cb0[18].zzzz, r0.yzwy, r3.xyzx
    r0.xyz = ((source[18].zzzz)*(r0.yzwy)+(r3.xyzx)).xyz;
    // 379: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 380: mul r3.xyz, r8.wwww, cb0[37].xyzx
    r3.xyz = ((r8.wwww)*(source[37].xyzx)).xyz;
    // 381: mad r3.xyz, r8.zzzz, cb0[36].xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(source[36].xyzx)+(r3.xyzx)).xyz;
    // 382: mul r3.xyz, r3.xyzx, cb0[38].wwww
    r3.xyz = ((r3.xyzx)*(source[38].wwww)).xyz;
    // 383: mul_sat r5.xyz, cb0[17].xyzx, cb0[17].wwww
    r5.xyz = (saturate((source[17].xyzx)*(source[17].wwww))).xyz;
    // 384: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 385: mul r5.xyz, r5.xyzx, cb0[25].xxxx
    r5.xyz = ((r5.xyzx)*(source[25].xxxx)).xyz;
    // 386: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 387: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 388: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 389: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 390: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 391: add r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)+(r0.xyzx)).xyz;
    // 392: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 393: mad o0.xyz, r1.xyzx, cb0[38].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[38].xyzx)+(r0.xyzx)).xyz;
    // 394: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 395: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 396: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 397: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 398: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 399: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 400: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 401: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 402: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 403: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 404: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 405: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 406: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 407: ftou r0.x, cb0[35].z
    r0.x = (asfloat((uint4)(source[35].zzzz))).x;
    // 408: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 409: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 410: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 411: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 412: ret
    return output;
}

// source.character.equipment-native-190.v1 / source program 504ebc66ea75f24d96023f0c790944d6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase190(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].x=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[2].x=1.f;
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[24]=g_SourceCharacterEnvironmentColor; source[25]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
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
    // 7: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 8: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 10: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 14: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: mul r5.xy, r4.xyxx, cb0[17].xxxx
    r5.xy = ((r4.xyxx)*(source[17].xxxx)).xy;
    // 16: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 17: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 19: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 20: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 22: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 23: div r4.xyz, r5.xyzx, r0.wwww
    r4.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 24: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 27: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 28: mul r6.xyz, r0.wwww, r5.xyzx
    r6.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 29: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 31: mov_sat r7.x, r7.x
    r7.x = (saturate(r7.xxxx)).x;
    // 32: mul_sat r1.w, r7.x, cb0[20].z
    r1.w = (saturate((r7.xxxx)*(source[20].zzzz))).w;
    // 33: mul r2.w, r1.w, cb0[2].x
    r2.w = ((r1.wwww)*(source[2].xxxx)).w;
    // 34: add r8.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 36: lt r10.xyz, |r9.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (asfloat((uint4)((abs(r9.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 37: log r9.xyz, |r9.xzyx|
    r9.xyz = (log2(abs(r9.xzyx))).xyz;
    // 38: mul r3.w, r9.x, cb0[19].x
    r3.w = ((r9.xxxx)*(source[19].xxxx)).w;
    // 39: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 40: movc r3.w, r10.x, l(0), r3.w
    r3.w = ((asuint(r10.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 41: add_sat r3.w, r3.w, cb0[19].y
    r3.w = (saturate((r3.wwww)+(source[19].yyyy))).w;
    // 42: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r11.xyz, r4.wwww, cb0[14].xyzx
    r11.xyz = ((r4.wwww)*(source[14].xyzx)).xyz;
    // 44: add r4.w, r3.w, l(-1.000000)
    r4.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 45: mad r4.w, cb0[19].w, r4.w, l(1.000000)
    r4.w = ((source[19].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: mul r12.xyz, cb0[4].xyzx, cb0[4].wwww
    r12.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 47: max r13.xyz, r12.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r13.xyz = (max(r12.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 48: min r13.xyz, r13.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r13.xyz = (min(r13.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 49: max r12.xyz, r12.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: min r12.xyz, r12.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r12.xyz = (min(r12.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 51: mul r5.w, r9.y, cb0[17].y
    r5.w = ((r9.yyyy)*(source[17].yyyy)).w;
    // 52: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 53: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: movc r5.w, r10.y, l(0), r5.w
    r5.w = ((asuint(r10.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 55: add r9.xyw, -r13.xyxz, r12.xyxz
    r9.xyw = ((-(r13.xyxz))+(r12.xyxz)).xyw;
    // 56: mad r9.xyw, r5.wwww, r9.xyxw, r13.xyxz
    r9.xyw = ((r5.wwww)*(r9.xyxw)+(r13.xyxz)).xyw;
    // 57: mul r10.xyw, cb0[5].xyxz, cb0[5].wwww
    r10.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 58: max r12.xyz, r10.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r12.xyz = (max(r10.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 59: min r12.xyz, r12.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r12.xyz = (min(r12.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: max r10.xyw, r10.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r10.xyw = (max(r10.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 61: min r10.xyw, r10.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r10.xyw = (min(r10.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 62: add r10.xyw, -r12.xyxz, r10.xyxw
    r10.xyw = ((-(r12.xyxz))+(r10.xyxw)).xyw;
    // 63: mad r10.xyw, r5.wwww, r10.xyxw, r12.xyxz
    r10.xyw = ((r5.wwww)*(r10.xyxw)+(r12.xyxz)).xyw;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r12.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r12.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 65: add r10.xyw, -r9.xyxw, r10.xyxw
    r10.xyw = ((-(r9.xyxw))+(r10.xyxw)).xyw;
    // 66: mad r9.xyw, r12.xxxx, r10.xyxw, r9.xyxw
    r9.xyw = ((r12.xxxx)*(r10.xyxw)+(r9.xyxw)).xyw;
    // 67: mul r10.xyw, cb0[6].xyxz, cb0[6].wwww
    r10.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 68: max r13.xyz, r10.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r13.xyz = (max(r10.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 69: min r13.xyz, r13.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r13.xyz = (min(r13.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 70: max r10.xyw, r10.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r10.xyw = (max(r10.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 71: min r10.xyw, r10.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r10.xyw = (min(r10.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 72: add r10.xyw, -r13.xyxz, r10.xyxw
    r10.xyw = ((-(r13.xyxz))+(r10.xyxw)).xyw;
    // 73: mad r10.xyw, r5.wwww, r10.xyxw, r13.xyxz
    r10.xyw = ((r5.wwww)*(r10.xyxw)+(r13.xyxz)).xyw;
    // 74: add r10.xyw, -r9.xyxw, r10.xyxw
    r10.xyw = ((-(r9.xyxw))+(r10.xyxw)).xyw;
    // 75: mad r9.xyw, r12.yyyy, r10.xyxw, r9.xyxw
    r9.xyw = ((r12.yyyy)*(r10.xyxw)+(r9.xyxw)).xyw;
    // 76: mul r10.xyw, cb0[7].xyxz, cb0[7].wwww
    r10.xyw = ((source[7].xyxz)*(source[7].wwww)).xyw;
    // 77: max r13.xyz, r10.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r13.xyz = (max(r10.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 78: min r13.xyz, r13.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r13.xyz = (min(r13.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 79: max r10.xyw, r10.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r10.xyw = (max(r10.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 80: min r10.xyw, r10.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r10.xyw = (min(r10.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 81: add r10.xyw, -r13.xyxz, r10.xyxw
    r10.xyw = ((-(r13.xyxz))+(r10.xyxw)).xyw;
    // 82: mad r10.xyw, r5.wwww, r10.xyxw, r13.xyxz
    r10.xyw = ((r5.wwww)*(r10.xyxw)+(r13.xyxz)).xyw;
    // 83: add r10.xyw, -r9.xyxw, r10.xyxw
    r10.xyw = ((-(r9.xyxw))+(r10.xyxw)).xyw;
    // 84: mad r9.xyw, r12.zzzz, r10.xyxw, r9.xyxw
    r9.xyw = ((r12.zzzz)*(r10.xyxw)+(r9.xyxw)).xyw;
    // 85: dp3 r6.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 86: add r10.xyw, -r9.xyxw, r6.wwww
    r10.xyw = ((-(r9.xyxw))+(r6.wwww)).xyw;
    // 87: mad r10.xyw, cb0[17].wwww, r10.xyxw, r9.xyxw
    r10.xyw = ((source[17].wwww)*(r10.xyxw)+(r9.xyxw)).xyw;
    // 88: dp3 r6.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: add r13.xyz, -r10.xywx, r6.wwww
    r13.xyz = ((-(r10.xywx))+(r6.wwww)).xyz;
    // 90: mad r10.xyw, cb0[18].xxxx, r13.xyxz, r10.xyxw
    r10.xyw = ((source[18].xxxx)*(r13.xyxz)+(r10.xyxw)).xyw;
    // 91: mad r13.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mad r14.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 93: mul r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)*(r14.xyzx)).xyz;
    // 94: mul r10.xyw, r10.xyxw, r13.xyxz
    r10.xyw = ((r10.xyxw)*(r13.xyxz)).xyw;
    // 95: dp3 r6.w, r7.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r7.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r14.xyz, -r7.yzwy, r6.wwww
    r14.xyz = ((-(r7.yzwy))+(r6.wwww)).xyz;
    // 97: mad r14.xyz, cb0[17].wwww, r14.xyzx, r7.yzwy
    r14.xyz = ((source[17].wwww)*(r14.xyzx)+(r7.yzwy)).xyz;
    // 98: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 99: add r15.xyz, -r14.xyzx, r6.wwww
    r15.xyz = ((-(r14.xyzx))+(r6.wwww)).xyz;
    // 100: mad r14.xyz, cb0[18].xxxx, r15.xyzx, r14.xyzx
    r14.xyz = ((source[18].xxxx)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 101: mul r15.xyz, r10.xywx, r14.xyzx
    r15.xyz = ((r10.xywx)*(r14.xyzx)).xyz;
    // 102: dp3 r6.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: mad r10.xyw, -r10.xyxw, r14.xyxz, r6.wwww
    r10.xyw = ((-(r10.xyxw))*(r14.xyxz)+(r6.wwww)).xyw;
    // 104: mad r10.xyw, cb0[17].wwww, r10.xyxw, r15.xyxz
    r10.xyw = ((source[17].wwww)*(r10.xyxw)+(r15.xyxz)).xyw;
    // 105: dp3 r6.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r14.xyz, -r10.xywx, r6.wwww
    r14.xyz = ((-(r10.xywx))+(r6.wwww)).xyz;
    // 107: mad r10.xyw, cb0[18].xxxx, r14.xyxz, r10.xyxw
    r10.xyw = ((source[18].xxxx)*(r14.xyxz)+(r10.xyxw)).xyw;
    // 108: mul r10.xyw, r13.xyxz, r10.xyxw
    r10.xyw = ((r13.xyxz)*(r10.xyxw)).xyw;
    // 109: mul r6.w, cb0[8].z, l(1.500000)
    r6.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 110: add r7.x, -cb0[8].w, l(1.000000)
    r7.x = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: mul r7.x, r7.x, cb0[20].x
    r7.x = ((r7.xxxx)*(source[20].xxxx)).x;
    // 112: mul r7.x, r7.x, l(6.283185)
    r7.x = ((r7.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 113: sincos r7.x, null, r7.x
    r7.x = (sin(r7.xxxx)).x;
    // 114: add r7.x, r7.x, l(1.000000)
    r7.x = ((r7.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: mul r6.w, r6.w, r7.x
    r6.w = ((r6.wwww)*(r7.xxxx)).w;
    // 116: mad r6.w, r6.w, l(0.500000), cb0[8].z
    r6.w = ((r6.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 117: frc r7.x, cb0[8].x
    r7.x = (frac(source[8].xxxx)).x;
    // 118: add r8.w, -r7.x, cb0[8].x
    r8.w = ((-(r7.xxxx))+(source[8].xxxx)).w;
    // 119: mul r13.z, r8.w, l(0.125000)
    r13.z = ((r8.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 120: mov r13.xw, l(0,0,0,0)
    r13.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 121: mul r13.y, cb0[8].y, cb0[15].y
    r13.y = ((source[8].yyyy)*(source[15].yyyy)).y;
    // 122: frc r8.w, v4.x
    r8.w = (frac(v4.xxxx)).w;
    // 123: mul r14.x, r8.w, l(0.125000)
    r14.x = ((r8.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 124: mov r14.y, v4.y
    r14.y = (v4.yyyy).y;
    // 125: add r13.xy, r13.xyxx, r14.xyxx
    r13.xy = ((r13.xyxx)+(r14.xyxx)).xy;
    // 126: add r13.xy, r13.xyxx, r13.zwzz
    r13.xy = ((r13.xyxx)+(r13.zwzz)).xy;
    // 127: sample_b_indexable(texture2d)(float,float,float,float) r13.xyzw, r13.xyxx, t4.xyzw, s4, l(0.000000)
    r13.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r13.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 128: mul r13.xyz, r6.wwww, r13.xyzx
    r13.xyz = ((r6.wwww)*(r13.xyzx)).xyz;
    // 129: mul r6.w, r7.x, r13.w
    r6.w = ((r7.xxxx)*(r13.wwww)).w;
    // 130: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xywx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xywx))).xyz;
    // 131: mad r10.xyw, r6.wwww, r13.xyxz, r10.xyxw
    r10.xyw = ((r6.wwww)*(r13.xyxz)+(r10.xyxw)).xyw;
    // 132: mul r13.xyz, r4.wwww, r10.xywx
    r13.xyz = ((r4.wwww)*(r10.xywx)).xyz;
    // 133: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 134: mad r13.xyz, r4.wwww, r10.xywx, -r11.xyzx
    r13.xyz = ((r4.wwww)*(r10.xywx)+(-(r11.xyzx))).xyz;
    // 135: mad r11.xyz, r3.wwww, r13.xyzx, r11.xyzx
    r11.xyz = ((r3.wwww)*(r13.xyzx)+(r11.xyzx)).xyz;
    // 136: mul r8.xyz, r8.xyzx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 137: mad_sat r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = (saturate((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 138: mul r7.xyz, r7.yzwy, r9.xywx
    r7.xyz = ((r7.yzwy)*(r9.xywx)).xyz;
    // 139: dp3 r3.x, r4.xyzx, r3.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 140: mul_sat r3.y, r3.x, cb0[18].y
    r3.y = (saturate((r3.xxxx)*(source[18].yyyy))).y;
    // 141: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 142: mul_sat r4.x, r3.z, cb0[18].y
    r4.x = (saturate((r3.zzzz)*(source[18].yyyy))).x;
    // 143: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 144: add_sat r4.x, r4.x, -cb0[18].z
    r4.x = (saturate((r4.xxxx)+(-(source[18].zzzz)))).x;
    // 145: lt r4.y, r4.x, l(0.000001)
    r4.y = (asfloat((uint4)((r4.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 146: log r4.x, r4.x
    r4.x = (log2(r4.xxxx)).x;
    // 147: mul r4.x, r4.x, cb0[18].w
    r4.x = ((r4.xxxx)*(source[18].wwww)).x;
    // 148: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 149: mul r3.y, r3.y, r4.x
    r3.y = ((r3.yyyy)*(r4.xxxx)).y;
    // 150: movc r3.y, r4.y, l(0), r3.y
    r3.y = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).y;
    // 151: mul r4.x, r3.y, cb0[11].w
    r4.x = ((r3.yyyy)*(source[11].wwww)).x;
    // 152: mad r4.yzw, r3.yyyy, cb0[12].xxyz, -cb0[12].xxyz
    r4.yzw = ((r3.yyyy)*(source[12].xxyz)+(-(source[12].xxyz))).yzw;
    // 153: mad r4.yzw, cb0[12].wwww, r4.yyzw, cb0[12].xxyz
    r4.yzw = ((source[12].wwww)*(r4.yyzw)+(source[12].xxyz)).yzw;
    // 154: mad r4.xyz, r4.xxxx, cb0[11].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[11].xyzx)+(r4.yzwy)).xyz;
    // 155: add r3.xy, -|r3.xzxx|, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(abs(r3.xzxx)))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 156: mul r3.x, r3.x, r3.y
    r3.x = ((r3.xxxx)*(r3.yyyy)).x;
    // 157: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 158: log r3.x, |r3.x|
    r3.x = (log2(abs(r3.xxxx))).x;
    // 159: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 160: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 161: mul r9.xyw, r3.xxxx, cb0[13].xyxz
    r9.xyw = ((r3.xxxx)*(source[13].xyxz)).xyw;
    // 162: movc r3.xyz, r3.yyyy, l(0,0,0,0), r9.xywx
    r3.xyz = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xywx)).xyz;
    // 163: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 164: mad r3.xyz, cb0[17].zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((source[17].zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 165: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 166: add r4.x, -cb0[21].z, cb0[21].y
    r4.x = ((-(source[21].zzzz))+(source[21].yyyy)).x;
    // 167: mad r4.x, r12.x, r4.x, cb0[21].z
    r4.x = ((r12.xxxx)*(r4.xxxx)+(source[21].zzzz)).x;
    // 168: add r4.y, -r4.x, cb0[22].x
    r4.y = ((-(r4.xxxx))+(source[22].xxxx)).y;
    // 169: mad r4.x, r12.y, r4.y, r4.x
    r4.x = ((r12.yyyy)*(r4.yyyy)+(r4.xxxx)).x;
    // 170: add r4.y, -r4.x, cb0[22].z
    r4.y = ((-(r4.xxxx))+(source[22].zzzz)).y;
    // 171: mad r4.x, r12.z, r4.y, r4.x
    r4.x = ((r12.zzzz)*(r4.yyyy)+(r4.xxxx)).x;
    // 172: mul r4.x, r9.z, r4.x
    r4.x = ((r9.zzzz)*(r4.xxxx)).x;
    // 173: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 174: min r4.x, r4.x, l(1.000000)
    r4.x = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 175: movc r4.x, r10.z, l(0), r4.x
    r4.x = ((asuint(r10.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 176: max r4.x, r4.x, cb0[0].x
    r4.x = (max(r4.xxxx,source[0].xxxx)).x;
    // 177: min r4.z, r4.x, l(1.000000)
    r4.z = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 178: mul_sat r4.w, r5.w, cb2[3].w
    r4.w = (saturate((r5.wwww)*(passValues[3].wwww))).w;
    // 179: mov_sat r8.w, cb0[20].y
    r8.w = (saturate(source[20].yyyy)).w;
    // 180: add r4.x, r10.y, r10.x
    r4.x = ((r10.yyyy)+(r10.xxxx)).x;
    // 181: add r4.x, r10.w, r4.x
    r4.x = ((r10.wwww)+(r4.xxxx)).x;
    // 182: mul r4.x, r4.x, l(0.333330)
    r4.x = ((r4.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 183: max r4.x, r4.x, cb0[23].x
    r4.x = (max(r4.xxxx,source[23].xxxx)).x;
    // 184: min r4.x, r4.x, cb0[22].w
    r4.x = (min(r4.xxxx,source[22].wwww)).x;
    // 185: add r4.y, -r4.x, l(1.000000)
    r4.y = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: mad r4.x, r5.w, r4.y, r4.x
    r4.x = ((r5.wwww)*(r4.yyyy)+(r4.xxxx)).x;
    // 187: mul_sat r3.w, r3.w, r4.x
    r3.w = (saturate((r3.wwww)*(r4.xxxx))).w;
    // 188: mul_sat r7.xyz, cb0[16].xyzx, cb0[16].wwww
    r7.xyz = (saturate((source[16].xyzx)*(source[16].wwww))).xyz;
    // 189: mov_sat r4.x, r0.w
    r4.x = (saturate(r0.wwww)).x;
    // 190: log r4.x, r4.x
    r4.x = (log2(r4.xxxx)).x;
    // 191: mul r4.x, r4.x, cb0[1].y
    r4.x = ((r4.xxxx)*(source[1].yyyy)).x;
    // 192: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 193: mad_sat r4.x, r4.x, cb0[1].w, cb0[1].z
    r4.x = (saturate((r4.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 194: mul r4.x, r4.x, cb0[23].y
    r4.x = ((r4.xxxx)*(source[23].yyyy)).x;
    // 195: mul r9.xyz, r7.xyzx, r4.xxxx
    r9.xyz = ((r7.xyzx)*(r4.xxxx)).xyz;
    // 196: add r4.x, -r4.w, l(1.000000)
    r4.x = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 197: mul r9.xyz, r4.xxxx, r9.xyzx
    r9.xyz = ((r4.xxxx)*(r9.xyzx)).xyz;
    // 198: dp3 r4.y, v7.xyzx, v7.xyzx
    r4.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 199: rsq r4.y, r4.y
    r4.y = (rsqrt(r4.yyyy)).y;
    // 200: mul r10.xyz, r4.yyyy, v7.xyzx
    r10.xyz = ((r4.yyyy)*(v7.xyzx)).xyz;
    // 201: dp3 r4.y, r10.xyzx, r5.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 202: mad r11.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 203: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 204: mul r11.yzw, r11.yyyy, cb0[35].xxyz
    r11.yzw = ((r11.yyyy)*(source[35].xxyz)).yzw;
    // 205: mad r11.xyz, r11.xxxx, cb0[34].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[34].xyzx)+(r11.yzwy)).xyz;
    // 206: mul r11.xyz, r11.xyzx, cb0[36].wwww
    r11.xyz = ((r11.xyzx)*(source[36].wwww)).xyz;
    // 207: mul r11.xyz, r8.xyzx, r11.xyzx
    r11.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 208: dp3 r4.y, r10.xyzx, r6.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 209: mad r12.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 210: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 211: mul r12.yzw, r12.yyyy, cb0[35].xxyz
    r12.yzw = ((r12.yyyy)*(source[35].xxyz)).yzw;
    // 212: mad r12.xyz, cb0[34].xyzx, r12.xxxx, r12.yzwy
    r12.xyz = ((source[34].xyzx)*(r12.xxxx)+(r12.yzwy)).xyz;
    // 213: mul r12.xyz, r12.xyzx, cb0[36].wwww
    r12.xyz = ((r12.xyzx)*(source[36].wwww)).xyz;
    // 214: dp3 r4.y, -r10.xyzx, r5.xyzx
    r4.y = (dot((-(r10.xyzx)).xyz,(r5.xyzx).xyz).xxxx).y;
    // 215: mad r10.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 216: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 217: mul r10.yzw, r10.yyyy, cb0[35].xxyz
    r10.yzw = ((r10.yyyy)*(source[35].xxyz)).yzw;
    // 218: mad r10.xyz, r10.xxxx, cb0[34].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[34].xyzx)+(r10.yzwy)).xyz;
    // 219: mul r10.xyz, r10.xyzx, cb0[36].wwww
    r10.xyz = ((r10.xyzx)*(source[36].wwww)).xyz;
    // 220: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 221: mul r9.xyz, r8.xyzx, r9.xyzx
    r9.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 222: mul r4.y, r8.w, l(0.080000)
    r4.y = ((r8.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 223: mad r10.xyz, -r8.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r8.xyzx
    r10.xyz = ((-(r8.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r8.xyzx)).xyz;
    // 224: mad r10.xyz, r4.wwww, r10.xyzx, r4.yyyy
    r10.xyz = ((r4.wwww)*(r10.xyzx)+(r4.yyyy)).xyz;
    // 225: deriv_rtx_coarse r13.x, r0.w
    r13.x = (ddx_coarse(r0.wwww)).x;
    // 226: deriv_rty_coarse r13.y, r0.w
    r13.y = (ddy_coarse(r0.wwww)).y;
    // 227: dp2 r4.y, r13.xyxx, r13.xyxx
    r4.y = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).y;
    // 228: sqrt r4.y, r4.y
    r4.y = (sqrt(r4.yyyy)).y;
    // 229: mad r4.y, r4.y, l(0.300000), r4.z
    r4.y = ((r4.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz)).y;
    // 230: min r13.y, r4.y, l(1.000000)
    r13.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 231: add r4.y, r6.z, l(1.000000)
    r4.y = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 232: min r4.y, r4.y, l(1.000000)
    r4.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 233: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 234: add_sat r13.x, -r4.y, r0.w
    r13.x = (saturate((-(r4.yyyy))+(r0.wwww))).x;
    // 235: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 236: add r0.w, -r13.y, l(1.000000)
    r0.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: max r14.xyz, r10.xyzx, r0.wwww
    r14.xyz = (max(r10.xyzx,r0.wwww)).xyz;
    // 238: add r14.xyz, -r10.xyzx, r14.xyzx
    r14.xyz = ((-(r10.xyzx))+(r14.xyzx)).xyz;
    // 239: mul_sat r0.w, r10.y, l(50.000000)
    r0.w = (saturate((r10.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 240: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 241: mul r15.xyz, r10.xyzx, r13.wwww
    r15.xyz = ((r10.xyzx)*(r13.wwww)).xyz;
    // 242: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 243: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r0.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 244: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 245: mad r15.xyz, r10.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r10.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 246: mul r16.xyz, r14.xyzx, r15.xyzx
    r16.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 247: dp3 r17.x, r1.xyzx, r6.xyzx
    r17.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 248: dp3 r17.y, r2.xyzx, r6.xyzx
    r17.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 249: dp3 r6.y, r0.xyzx, r6.xyzx
    r6.y = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 250: mul r0.w, r13.y, l(5.000000)
    r0.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 251: mul r13.zw, cb0[25].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r13.zw = ((source[25].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 252: dp2 r6.x, r17.xyxx, r13.zwzz
    r6.x = (dot((r17.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 253: dp2 r6.z, r17.xyxx, cb0[25].xyxx
    r6.z = (dot((r17.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 254: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r6.xyzx, t6.xyzw, s5, r0.w
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r0.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 255: mul r6.xyz, r6.xyzx, r6.wwww
    r6.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 256: mul r6.xyz, r6.xyzx, cb0[24].xyzx
    r6.xyz = ((r6.xyzx)*(source[24].xyzx)).xyz;
    // 257: mul r6.xyz, r6.xyzx, cb0[25].zzzz
    r6.xyz = ((r6.xyzx)*(source[25].zzzz)).xyz;
    // 258: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[24].wwww)).xyz;
    // 259: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r6.xyz, -r0.wwww, r6.xyzx
    r6.xyz = ((-(r0.wwww))+(r6.xyzx)).xyz;
    // 261: mad r6.xyz, r6.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r0.wwww
    r6.xyz = ((r6.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r0.wwww)).xyz;
    // 262: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: mad r4.y, r13.y, l(2.000000), l(2.000000)
    r4.y = ((r13.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).y;
    // 264: div r0.w, r0.w, r4.y
    r0.w = ((r0.wwww)/(r4.yyyy)).w;
    // 265: dp3 r5.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: mad r0.w, r5.w, l(5.000000), r0.w
    r0.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.wwww)).w;
    // 267: add_sat r0.w, r4.w, r0.w
    r0.w = (saturate((r4.wwww)+(r0.wwww))).w;
    // 268: mad r6.w, r0.w, l(-2.000000), l(3.000000)
    r6.w = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 269: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 270: mul r0.w, r0.w, r6.w
    r0.w = ((r0.wwww)*(r6.wwww)).w;
    // 271: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 272: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 273: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 274: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 275: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 276: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 277: dp3 r0.y, r0.xyzx, r5.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 278: dp2 r0.x, r1.xyxx, r13.zwzz
    r0.x = (dot((r1.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 279: dp2 r0.z, r1.xyxx, cb0[25].xyxx
    r0.z = (dot((r1.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 280: mov r0.w, l(1.000000)
    r0.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 281: dp4 r2.x, cb0[26].xyzw, r0.xyzw
    r2.x = (dot((source[26].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 282: dp4 r2.y, cb0[27].xyzw, r0.xyzw
    r2.y = (dot((source[27].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).y;
    // 283: dp4 r2.z, cb0[28].xyzw, r0.xyzw
    r2.z = (dot((source[28].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).z;
    // 284: mul r17.xyzw, r0.yzzx, r0.xyzz
    r17.xyzw = ((r0.yzzx)*(r0.xyzz)).xyzw;
    // 285: dp4 r5.x, cb0[29].xyzw, r17.xyzw
    r5.x = (dot((source[29].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).x;
    // 286: dp4 r5.y, cb0[30].xyzw, r17.xyzw
    r5.y = (dot((source[30].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).y;
    // 287: dp4 r5.z, cb0[31].xyzw, r17.xyzw
    r5.z = (dot((source[31].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).z;
    // 288: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 289: mad r0.x, r0.x, r0.x, -r0.z
    r0.x = ((r0.xxxx)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 290: add r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 291: mad r0.xzw, cb0[32].xxyz, r0.xxxx, r2.xxyz
    r0.xzw = ((source[32].xxyz)*(r0.xxxx)+(r2.xxyz)).xzw;
    // 292: max r0.xzw, r0.xxzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xzw = (max(r0.xxzw,float4(0.000000,0.000000,0.000000,0.000000))).xzw;
    // 293: mul r0.xzw, r0.xxzw, cb0[24].xxyz
    r0.xzw = ((r0.xxzw)*(source[24].xxyz)).xzw;
    // 294: mul r0.xzw, r0.xxzw, cb0[25].zzzz
    r0.xzw = ((r0.xxzw)*(source[25].zzzz)).xzw;
    // 295: mad r0.xzw, r0.xxzw, l(3.141593, 0.000000, 3.141593, 3.141593), cb0[24].wwww
    r0.xzw = ((r0.xxzw)*(float4(3.141593,0.000000,3.141593,3.141593))+(source[24].wwww)).xzw;
    // 296: dp3 r2.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 297: add r0.xzw, r0.xxzw, -r2.xxxx
    r0.xzw = ((r0.xxzw)+(-(r2.xxxx))).xzw;
    // 298: mad r0.xzw, r0.xxzw, l(0.800000, 0.000000, 0.800000, 0.800000), r2.xxxx
    r0.xzw = ((r0.xxzw)*(float4(0.800000,0.000000,0.800000,0.800000))+(r2.xxxx)).xzw;
    // 299: dp3 r2.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 300: div r2.x, r2.x, r4.y
    r2.x = ((r2.xxxx)/(r4.yyyy)).x;
    // 301: mad r2.x, r5.w, l(5.000000), r2.x
    r2.x = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.xxxx)).x;
    // 302: add_sat r2.x, r4.w, r2.x
    r2.x = (saturate((r4.wwww)+(r2.xxxx))).x;
    // 303: mad r2.y, r2.x, l(-2.000000), l(3.000000)
    r2.y = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 304: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 305: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 306: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 307: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 308: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 309: mul r0.xzw, r0.xxzw, r2.xxxx
    r0.xzw = ((r0.xxzw)*(r2.xxxx)).xzw;
    // 310: mul r2.x, r13.y, r13.y
    r2.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 311: dp3 r2.y, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 312: add r2.z, r3.w, r13.x
    r2.z = ((r3.wwww)+(r13.xxxx)).z;
    // 313: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 314: mul r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)*(r2.xxxx)).x;
    // 315: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 316: add r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)+(r2.xxxx)).x;
    // 317: add_sat r2.x, r2.x, l(-1.000000)
    r2.x = (saturate((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 318: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 319: mad r2.y, r2.x, r5.x, r5.y
    r2.y = ((r2.xxxx)*(r5.xxxx)+(r5.yyyy)).y;
    // 320: mad r2.y, r2.y, r2.x, r5.z
    r2.y = ((r2.yyyy)*(r2.xxxx)+(r5.zzzz)).y;
    // 321: mul r2.y, r2.x, r2.y
    r2.y = ((r2.xxxx)*(r2.yyyy)).y;
    // 322: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 323: mad r5.xyz, r8.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r8.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 324: mad r10.xyz, r8.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r8.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 325: mad r13.xyz, r8.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r8.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 326: mad r5.xyz, r3.wwww, r5.xyzx, r10.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(r10.xyzx)).xyz;
    // 327: mad r5.xyz, r5.xyzx, r3.wwww, r13.xyzx
    r5.xyz = ((r5.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 328: mul r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 329: max r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = (max(r3.wwww,r5.xyzx)).xyz;
    // 330: mul r10.xyz, r2.xxxx, r12.xyzx
    r10.xyz = ((r2.xxxx)*(r12.xyzx)).xyz;
    // 331: mul r11.xyz, r5.xyzx, r11.xyzx
    r11.xyz = ((r5.xyzx)*(r11.xyzx)).xyz;
    // 332: mul r10.xyz, r6.xyzx, r10.xyzx
    r10.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 333: mul r10.xyz, r10.xyzx, r16.xyzx
    r10.xyz = ((r10.xyzx)*(r16.xyzx)).xyz;
    // 334: mul r12.xyz, r10.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r12.xyz = ((r10.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 335: mad r13.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 336: mul r11.xyz, r11.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 337: mul r14.xyz, r0.xzwx, r13.xyzx
    r14.xyz = ((r0.xzwx)*(r13.xyzx)).xyz;
    // 338: mul r11.xyz, r11.xyzx, r14.xyzx
    r11.xyz = ((r11.xyzx)*(r14.xyzx)).xyz;
    // 339: mad r11.xyz, -r11.xyzx, r4.wwww, r11.xyzx
    r11.xyz = ((-(r11.xyzx))*(r4.wwww)+(r11.xyzx)).xyz;
    // 340: mul r6.xyz, r6.xyzx, r16.xyzx
    r6.xyz = ((r6.xyzx)*(r16.xyzx)).xyz;
    // 341: mul r13.xyz, r8.xyzx, r13.xyzx
    r13.xyz = ((r8.xyzx)*(r13.xyzx)).xyz;
    // 342: mul r13.xyz, r4.xxxx, r13.xyzx
    r13.xyz = ((r4.xxxx)*(r13.xyzx)).xyz;
    // 343: mul r0.xzw, r0.xxzw, r13.xxyz
    r0.xzw = ((r0.xxzw)*(r13.xxyz)).xzw;
    // 344: mul r0.xzw, r5.xxyz, r0.xxzw
    r0.xzw = ((r5.xxyz)*(r0.xxzw)).xzw;
    // 345: mad r0.xzw, r6.xxyz, r2.xxxx, r0.xxzw
    r0.xzw = ((r6.xxyz)*(r2.xxxx)+(r0.xxzw)).xzw;
    // 346: mad r0.xzw, r0.xxzw, l(0.400000, 0.000000, 0.400000, 0.400000), r11.xxyz
    r0.xzw = ((r0.xxzw)*(float4(0.400000,0.000000,0.400000,0.400000))+(r11.xxyz)).xzw;
    // 347: mad r2.xyz, r9.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r2.xyz = ((r9.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 348: mad r0.xzw, r10.xxyz, l(0.600000, 0.000000, 0.600000, 0.600000), r0.xxzw
    r0.xzw = ((r10.xxyz)*(float4(0.600000,0.000000,0.600000,0.600000))+(r0.xxzw)).xzw;
    // 349: add r2.xyz, r0.xzwx, r2.xyzx
    r2.xyz = ((r0.xzwx)+(r2.xyzx)).xyz;
    // 350: mad r2.xyz, r8.xyzx, cb0[36].xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)*(source[36].xyzx)+(r2.xyzx)).xyz;
    // 351: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 352: mad r3.x, r3.x, l(-0.250000), l(0.400000)
    r3.x = ((r3.xxxx)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).x;
    // 353: eq r3.y, cb0[37].x, l(0.000000)
    r3.y = (asfloat((uint4)((source[37].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 354: not r3.z, r3.y
    r3.z = (asfloat(~asuint(r3.yyyy))).z;
    // 355: lt r4.x, r2.w, r3.x
    r4.x = (asfloat((uint4)((r2.wwww)<(r3.xxxx)) * 0xffffffffu)).x;
    // 356: and r3.z, r3.z, r4.x
    r3.z = (asfloat(asuint(r3.zzzz) & asuint(r4.xxxx))).z;
    // 357: discard_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 358: ge r3.x, r2.w, r3.x
    r3.x = (asfloat((uint4)((r2.wwww)>=(r3.xxxx)) * 0xffffffffu)).x;
    // 359: mad r1.w, r1.w, cb0[2].x, l(-0.900000)
    r1.w = ((r1.wwww)*(source[2].xxxx)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 360: mul_sat r1.w, r1.w, l(9.999998)
    r1.w = (saturate((r1.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 361: mad r3.z, r1.w, l(-2.000000), l(3.000000)
    r3.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 362: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 363: mul r1.w, r1.w, r3.z
    r1.w = ((r1.wwww)*(r3.zzzz)).w;
    // 364: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 365: movc r1.w, r3.x, r1.w, r2.w
    r1.w = ((asuint(r3.xxxx) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 366: movc o0.w, r3.y, r1.w, r2.w
    output.targets[0].w = ((asuint(r3.yyyy) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 367: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 368: mov r1.z, r0.y
    r1.z = (r0.yyyy).z;
    // 369: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 370: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 371: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 372: dp3 r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r0.y = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).y;
    // 373: div r1.xy, r1.xyxx, r0.yyyy
    r1.xy = ((r1.xyxx)/(r0.yyyy)).xy;
    // 374: ge r0.y, l(0.000000), r1.z
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).y;
    // 375: ge r1.zw, r1.xxxy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.zw = (asfloat((uint4)((r1.xxxy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).zw;
    // 376: movc r1.zw, r1.zzzw, l(0,0,1.000000,1.000000), l(0,0,-1.000000,-1.000000)
    r1.zw = ((asuint(r1.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),1.000000,1.000000)) : (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000))).zw;
    // 377: mad r1.zw, -|r1.yyyx|, r1.zzzw, r1.zzzw
    r1.zw = ((-(abs(r1.yyyx)))*(r1.zzzw)+(r1.zzzw)).zw;
    // 378: movc r1.xy, r0.yyyy, r1.zwzz, r1.xyxx
    r1.xy = ((asuint(r0.yyyy) != 0u) ? (r1.zwzz) : (r1.xyxx)).xy;
    // 379: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 380: dp3 o4.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 381: dp3 o4.y, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 382: mul r0.xyz, r7.xyzx, cb0[23].yyyy
    r0.xyz = ((r7.xyzx)*(source[23].yyyy)).xyz;
    // 383: ftou r0.w, cb0[33].z
    r0.w = (asfloat((uint4)(source[33].zzzz))).w;
    // 384: and r0.w, r0.w, l(31)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(31u,31u,31u,31u))).w;
    // 385: utof r0.w, r0.w
    r0.w = ((float4)(asuint(r0.wwww))).w;
    // 386: mul o5.w, r0.w, l(0.003922)
    output.targets[5].w = ((r0.wwww)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 387: dp3_sat o5.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 388: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 389: mov o3.xyzw, r8.xyzw
    output.targets[3].xyzw = (r8.xyzw).xyzw;
    // 390: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 391: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 392: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 393: ret
    return output;
}

// source.character.equipment-native-191.v1 / source program 9f017e8881705c49a8e561bf2f297193
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase191(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[22]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[24]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[25]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[31].y=(g_SourceCharacterTime.xxxx).x;
    source[31].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[31].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 22: add r1.y, -cb0[28].y, cb0[28].x
    r1.y = ((-(source[28].yyyy))+(source[28].xxxx)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: mad r1.y, r2.x, r1.y, cb0[28].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[28].yyyy)).y;
    // 25: add r1.z, -r1.y, cb0[28].z
    r1.z = ((-(r1.yyyy))+(source[28].zzzz)).z;
    // 26: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 27: add r1.z, -r1.y, cb0[28].w
    r1.z = ((-(r1.yyyy))+(source[28].wwww)).z;
    // 28: mad r1.y, r2.z, r1.z, r1.y
    r1.y = ((r2.zzzz)*(r1.zzzz)+(r1.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: add r1.z, -r3.w, l(1.000000)
    r1.z = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 32: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 33: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 34: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 35: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 37: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 38: add r1.w, -cb0[27].y, cb0[27].x
    r1.w = ((-(source[27].yyyy))+(source[27].xxxx)).w;
    // 39: mad r1.w, r2.x, r1.w, cb0[27].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[27].yyyy)).w;
    // 40: add r3.w, -r1.w, cb0[27].z
    r3.w = ((-(r1.wwww))+(source[27].zzzz)).w;
    // 41: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 42: add r3.w, -r1.w, cb0[27].w
    r3.w = ((-(r1.wwww))+(source[27].wwww)).w;
    // 43: mad r1.w, r2.z, r3.w, r1.w
    r1.w = ((r2.zzzz)*(r3.wwww)+(r1.wwww)).w;
    // 44: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 45: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 46: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 47: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 48: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 50: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 51: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 52: mul r4.xy, r1.xzxx, cb0[26].xxxx
    r4.xy = ((r1.xzxx)*(source[26].xxxx)).xy;
    // 53: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 55: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 56: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 57: add r1.xzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 58: mad r1.xzw, cb0[26].wwww, r1.xxzw, r4.xxyz
    r1.xzw = ((source[26].wwww)*(r1.xxzw)+(r4.xxyz)).xzw;
    // 59: dp3 r3.w, r1.xzwx, r1.xzwx
    r3.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: div r1.xzw, r1.xxzw, r3.wwww
    r1.xzw = ((r1.xxzw)/(r3.wwww)).xzw;
    // 62: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 63: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 64: mul r5.xyz, r3.wwww, v0.xyzx
    r5.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 65: dp3 r6.x, r5.xyzx, r1.xzwx
    r6.x = (dot((r5.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 66: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 67: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 68: mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 69: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 70: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 71: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 72: dp3 r6.y, r8.xyzx, r1.xzwx
    r6.y = (dot((r8.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 73: dp3 r6.z, r7.xyzx, r1.xzwx
    r6.z = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 74: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 75: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 76: mul r9.xyz, r1.xxxx, v5.xyzx
    r9.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 77: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 78: dp3 r10.y, r8.xyzx, r9.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 79: dp3 r10.x, r5.xyzx, r9.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 80: dp3 r10.z, r7.xyzx, r9.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 81: dp3 r3.w, r6.xyzx, r10.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 82: mul r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)*(r3.wwww)).xyz;
    // 83: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 84: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 85: dp2 r3.w, r6.ywyy, r6.ywyy
    r3.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 86: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 87: div r6.xy, r6.ywyy, r3.wwww
    r6.xy = ((r6.ywyy)/(r3.wwww)).xy;
    // 88: mad r3.w, -r6.z, l(0.250000), l(0.250000)
    r3.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 89: add r4.w, r6.z, l(1.000000)
    r4.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 91: mad r6.xy, r3.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t5.xyzw, s5, r0.x
    r6.xyz = ((g_SourceCharacterTexture5.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 93: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 94: rcp r0.x, cb0[29].x
    r0.x = (1.0/(source[29].xxxx)).x;
    // 95: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 96: mul r10.xyz, r10.xyzx, cb0[29].xxxx
    r10.xyz = ((r10.xyzx)*(source[29].xxxx)).xyz;
    // 97: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 98: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 99: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 100: mad r10.xyz, r10.xyzx, cb0[29].xxxx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[29].xxxx)+(r11.xyzx)).xyz;
    // 101: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 102: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 103: add r0.x, cb0[29].x, l(1.000000)
    r0.x = ((source[29].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 105: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 106: add r6.xyz, -cb0[17].xyzx, cb0[18].xyzx
    r6.xyz = ((-(source[17].xyzx))+(source[18].xyzx)).xyz;
    // 107: mad r6.xyz, r4.wwww, r6.xyzx, cb0[17].xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)+(source[17].xyzx)).xyz;
    // 108: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 109: mul r6.xyz, r6.xyzx, cb0[29].yyyy
    r6.xyz = ((r6.xyzx)*(source[29].yyyy)).xyz;
    // 110: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 111: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 112: mad r3.xyz, cb0[26].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[26].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 113: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 115: mad r3.xyz, cb0[26].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[26].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 116: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 117: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 118: mul r10.xyz, r10.xyzx, cb0[29].zzzz
    r10.xyz = ((r10.xyzx)*(source[29].zzzz)).xyz;
    // 119: add r0.x, r2.y, r2.x
    r0.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 120: add r0.x, r2.z, r0.x
    r0.x = ((r2.zzzz)+(r0.xxxx)).x;
    // 121: add_sat r0.x, r2.w, r0.x
    r0.x = (saturate((r2.wwww)+(r0.xxxx))).x;
    // 122: mad r3.xyz, r0.xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 123: max r10.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 124: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 125: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 126: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 127: dp3 r0.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 128: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 129: mul r0.x, r0.x, cb0[30].z
    r0.x = ((r0.xxxx)*(source[30].zzzz)).x;
    // 130: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 131: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 134: div r2.w, cb0[30].w, r2.w
    r2.w = ((source[30].wwww)/(r2.wwww)).w;
    // 135: dp3 r3.w, r4.xyzx, r4.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 136: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 137: div r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)/(r3.wwww)).xyz;
    // 138: dp3 r3.w, r4.xyzx, r9.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 139: mul_sat r4.w, r3.w, cb0[29].w
    r4.w = (saturate((r3.wwww)*(source[29].wwww))).w;
    // 140: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mul_sat r5.w, r9.z, cb0[29].w
    r5.w = (saturate((r9.zzzz)*(source[29].wwww))).w;
    // 143: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: add_sat r5.w, r5.w, -cb0[30].x
    r5.w = (saturate((r5.wwww)+(-(source[30].xxxx)))).w;
    // 145: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 146: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 147: mul r6.w, r6.w, cb0[30].y
    r6.w = ((r6.wwww)*(source[30].yyyy)).w;
    // 148: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 149: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 150: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 151: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 152: mul r10.xyz, r6.xyzx, r2.wwww
    r10.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 153: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 154: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 155: mad r0.yzw, cb0[26].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[26].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 156: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 157: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 158: mad r0.yzw, cb0[26].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[26].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 159: add r11.xy, v4.zwzz, l(-0.050000, -0.050000, 0.000000, 0.000000)
    r11.xy = ((v4.zwzz)+(float4(-0.050000,-0.050000,0.000000,0.000000))).xy;
    // 160: mul_sat r11.xy, r11.xyxx, l(256.000000, 256.000000, 0.000000, 0.000000)
    r11.xy = (saturate((r11.xyxx)*(float4(256.000000,256.000000,0.000000,0.000000)))).xy;
    // 161: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 162: mad r2.w, -r11.x, r11.y, l(1.000000)
    r2.w = ((-(r11.xxxx))*(r11.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mul r11.xy, v4.wzww, l(4.000000, 4.000000, 0.000000, 0.000000)
    r11.xy = ((v4.wzww)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 164: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r11.xyxx, t2.xyzw, s1, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r11.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 165: dp4 r5.w, r11.xyzw, cb0[11].xyzw
    r5.w = (dot((r11.xyzw).xyzw,(source[11].xyzw).xyzw).xxxx).w;
    // 166: mul r5.w, r2.w, r5.w
    r5.w = ((r2.wwww)*(r5.wwww)).w;
    // 167: mul r5.w, r5.w, cb0[7].y
    r5.w = ((r5.wwww)*(source[7].yyyy)).w;
    // 168: mul r12.xyz, cb0[9].xyzx, cb0[9].wwww
    r12.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 169: mad r13.xyz, cb0[10].wwww, cb0[10].xyzx, -r12.xyzx
    r13.xyz = ((source[10].wwww)*(source[10].xyzx)+(-(r12.xyzx))).xyz;
    // 170: mad r12.xyz, r5.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r5.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 171: dp4 r5.w, r11.xyzw, cb0[8].xyzw
    r5.w = (dot((r11.xyzw).xyzw,(source[8].xyzw).xyzw).xxxx).w;
    // 172: dp4 r6.w, r11.xyzw, cb0[14].xyzw
    r6.w = (dot((r11.xyzw).xyzw,(source[14].xyzw).xyzw).xxxx).w;
    // 173: mul r6.w, r2.w, r6.w
    r6.w = ((r2.wwww)*(r6.wwww)).w;
    // 174: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 175: mul r2.w, r2.w, cb0[7].x
    r2.w = ((r2.wwww)*(source[7].xxxx)).w;
    // 176: mul r5.w, r6.w, cb0[7].z
    r5.w = ((r6.wwww)*(source[7].zzzz)).w;
    // 177: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 178: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 179: mad r11.xyz, r2.wwww, r13.xyzx, r11.xyzx
    r11.xyz = ((r2.wwww)*(r13.xyzx)+(r11.xyzx)).xyz;
    // 180: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r11.xyzx
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r11.xyzx)).xyz;
    // 181: mul r13.xyz, cb0[4].xyzx, cb0[4].wwww
    r13.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 182: mad r11.xyz, r2.xxxx, r11.xyzx, r13.xyzx
    r11.xyz = ((r2.xxxx)*(r11.xyzx)+(r13.xyzx)).xyz;
    // 183: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 184: mad r2.xyw, r2.yyyy, r12.xyxz, r11.xyxz
    r2.xyw = ((r2.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 185: mul r11.xyz, cb0[12].xyzx, cb0[12].wwww
    r11.xyz = ((source[12].xyzx)*(source[12].wwww)).xyz;
    // 186: mad r12.xyz, cb0[13].wwww, cb0[13].xyzx, -r11.xyzx
    r12.xyz = ((source[13].wwww)*(source[13].xyzx)+(-(r11.xyzx))).xyz;
    // 187: mad r11.xyz, r5.wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((r5.wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 188: add r11.xyz, -r2.xywx, r11.xyzx
    r11.xyz = ((-(r2.xywx))+(r11.xyzx)).xyz;
    // 189: mad r2.xyz, r2.zzzz, r11.xyzx, r2.xywx
    r2.xyz = ((r2.zzzz)*(r11.xyzx)+(r2.xywx)).xyz;
    // 190: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 191: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 192: mad r2.xyz, cb0[26].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[26].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 193: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 194: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 195: mad r2.xyz, cb0[26].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[26].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 196: mad r11.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 197: mad r12.xyz, cb0[16].wwww, cb0[16].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[16].wwww)*(source[16].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 199: mul r2.xyz, r2.xyzx, r11.xyzx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 200: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 201: mul r2.xyz, r6.xyzx, r0.yzwy
    r2.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 202: mad r3.xyz, r3.xyzx, r10.xyzx, -r2.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r2.xyzx))).xyz;
    // 203: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r2.w, r2.w, cb0[31].x
    r2.w = ((r2.wwww)*(source[31].xxxx)).w;
    // 205: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 206: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 207: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 208: div r1.xzw, r1.xxzw, r3.xxxx
    r1.xzw = ((r1.xxzw)/(r3.xxxx)).xzw;
    // 209: dp3 r1.x, r1.xzwx, r9.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 210: add r1.z, -|r9.z|, l(1.000000)
    r1.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 211: mul r1.z, r3.w, r1.z
    r1.z = ((r3.wwww)*(r1.zzzz)).z;
    // 212: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: mul r1.w, |r1.x|, |r1.x|
    r1.w = ((abs(r1.xxxx))*(abs(r1.xxxx))).w;
    // 214: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 215: mul r1.w, r1.w, |r1.x|
    r1.w = ((r1.wwww)*(abs(r1.xxxx))).w;
    // 216: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 217: movc r1.x, r1.x, l(0), r1.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // 218: add r1.w, r1.x, l(-0.027778)
    r1.w = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 219: mad r1.x, r1.x, r1.w, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 220: div_sat r1.x, r1.x, r2.w
    r1.x = (saturate((r1.xxxx)/(r2.wwww))).x;
    // 221: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 222: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 223: mad r1.xyw, r1.xxxx, r2.xyxz, -r0.yzyw
    r1.xyw = ((r1.xxxx)*(r2.xyxz)+(-(r0.yzyw))).xyw;
    // 224: mad r0.xyz, r0.xxxx, r1.xywx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xywx)+(r0.yzwy)).xyz;
    // 225: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 226: add r1.xyw, -r0.xyxz, r0.wwww
    r1.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 227: mad r0.xyz, cb0[26].yyyy, r1.xywx, r0.xyzx
    r0.xyz = ((source[26].yyyy)*(r1.xywx)+(r0.xyzx)).xyz;
    // 228: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r1.xyw, -r0.xyxz, r0.wwww
    r1.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 230: mad r0.xyz, cb0[26].zzzz, r1.xywx, r0.xyzx
    r0.xyz = ((source[26].zzzz)*(r1.xywx)+(r0.xyzx)).xyz;
    // 231: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 232: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 233: mul r0.w, r0.w, cb0[31].y
    r0.w = ((r0.wwww)*(source[31].yyyy)).w;
    // 234: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 235: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 236: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r1.x, cb0[3].z, l(1.500000)
    r1.x = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 238: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 239: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 240: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 241: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 242: mul r3.y, cb0[3].y, cb0[22].y
    r3.y = ((source[3].yyyy)*(source[22].yyyy)).y;
    // 243: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 244: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 245: add r1.xy, r1.xyxx, r3.xyxx
    r1.xy = ((r1.xyxx)+(r3.xyxx)).xy;
    // 246: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 247: add r2.w, -r1.w, cb0[3].x
    r2.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 248: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 249: add r1.xy, r1.xyxx, r3.zwzz
    r1.xy = ((r1.xyxx)+(r3.zwzz)).xy;
    // 250: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 251: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 252: mul r0.w, r1.w, r3.w
    r0.w = ((r1.wwww)*(r3.wwww)).w;
    // 253: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 254: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 255: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 256: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 257: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 258: add r1.yw, -r3.xxxy, l(0.000000, 1.000000, 0.000000, 1.000000)
    r1.yw = ((-(r3.xxxy))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 259: add r1.yw, -r3.zzzw, r1.yyyw
    r1.yw = ((-(r3.zzzw))+(r1.yyyw)).yw;
    // 260: mad r1.yw, cb0[23].wwww, r1.yyyw, r3.zzzw
    r1.yw = ((source[23].wwww)*(r1.yyyw)+(r3.zzzw)).yw;
    // 261: mul r0.w, cb0[23].y, cb0[31].y
    r0.w = ((source[23].yyyy)*(source[31].yyyy)).w;
    // 262: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 263: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 264: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 265: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 266: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 267: mul r2.w, cb0[23].x, l(0.001000)
    r2.w = ((source[23].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 268: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 269: mad r1.yw, r2.wwww, r1.yyyw, r3.xxxy
    r1.yw = ((r2.wwww)*(r1.yyyw)+(r3.xxxy)).yw;
    // 270: dp2 r2.w, cb0[24].xyxx, r1.ywyy
    r2.w = (dot((source[24].xyxx).xy,(r1.ywyy).xy).xxxx).w;
    // 271: dp2 r3.y, cb0[25].xyxx, r1.ywyy
    r3.y = (dot((source[25].xyxx).xy,(r1.ywyy).xy).xxxx).y;
    // 272: frc r1.y, r2.w
    r1.y = (frac(r2.wwww)).y;
    // 273: mul r3.x, r1.y, l(0.125000)
    r3.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 274: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 275: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 276: mul r1.y, r3.w, l(0.900000)
    r1.y = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 277: mad r3.xyz, r1.yyyy, r3.xyzx, r0.xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 278: mul_sat r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx))).xyz;
    // 279: mad r6.xyz, cb0[23].zzzz, r3.xyzx, -r0.xyzx
    r6.xyz = ((source[23].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 280: mul r3.xyz, r3.xyzx, cb0[23].zzzz
    r3.xyz = ((r3.xyzx)*(source[23].zzzz)).xyz;
    // 281: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 282: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 283: mad r0.xyz, r0.wwww, r6.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 284: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 285: mul r3.xyz, r2.xyzx, r1.xxxx
    r3.xyz = ((r2.xyzx)*(r1.xxxx)).xyz;
    // 286: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 287: mad r1.xyw, -r1.xxxx, r2.xyxz, r0.wwww
    r1.xyw = ((-(r1.xxxx))*(r2.xyxz)+(r0.wwww)).xyw;
    // 288: mad r1.xyw, cb0[26].yyyy, r1.xyxw, r3.xyxz
    r1.xyw = ((source[26].yyyy)*(r1.xyxw)+(r3.xyxz)).xyw;
    // 289: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 290: add r2.xyz, -r1.xywx, r0.wwww
    r2.xyz = ((-(r1.xywx))+(r0.wwww)).xyz;
    // 291: mad r1.xyw, cb0[26].zzzz, r2.xyxz, r1.xyxw
    r1.xyw = ((source[26].zzzz)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 292: mad r2.xyz, r4.wwww, cb0[20].xyzx, -cb0[20].xyzx
    r2.xyz = ((r4.wwww)*(source[20].xyzx)+(-(source[20].xyzx))).xyz;
    // 293: mul r0.w, r4.w, cb0[19].w
    r0.w = ((r4.wwww)*(source[19].wwww)).w;
    // 294: mad r2.xyz, cb0[20].wwww, r2.xyzx, cb0[20].xyzx
    r2.xyz = ((source[20].wwww)*(r2.xyzx)+(source[20].xyzx)).xyz;
    // 295: mad r2.xyz, r0.wwww, cb0[19].xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(source[19].xyzx)+(r2.xyzx)).xyz;
    // 296: mad r1.xyw, r1.xyxw, r11.xyxz, r2.xyxz
    r1.xyw = ((r1.xyxw)*(r11.xyxz)+(r2.xyxz)).xyw;
    // 297: log r0.w, |r1.z|
    r0.w = (log2(abs(r1.zzzz))).w;
    // 298: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 299: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 300: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 301: mul r2.xyz, r0.wwww, cb0[21].xyzx
    r2.xyz = ((r0.wwww)*(source[21].xyzx)).xyz;
    // 302: movc r2.xyz, r1.zzzz, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 303: add r1.xyz, r1.xywx, r2.xyzx
    r1.xyz = ((r1.xywx)+(r2.xyzx)).xyz;
    // 304: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 305: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 306: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 307: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 308: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 309: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 310: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 311: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 312: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 313: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 314: mul r3.yzw, r3.yyyy, cb0[33].xxyz
    r3.yzw = ((r3.yyyy)*(source[33].xxyz)).yzw;
    // 315: mad r3.xyz, r3.xxxx, cb0[32].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[32].xyzx)+(r3.yzwy)).xyz;
    // 316: mul r3.xyz, r3.xyzx, cb0[34].wwww
    r3.xyz = ((r3.xyzx)*(source[34].wwww)).xyz;
    // 317: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 318: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 319: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 320: mad o0.xyz, r0.xyzx, cb0[34].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[34].xyzx)+(r1.xyzx)).xyz;
    // 321: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 322: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 323: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 324: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 325: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 326: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 327: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 328: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 329: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 330: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 331: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 332: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 333: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 334: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 335: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 336: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 337: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 338: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 339: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 340: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 341: ret
    return output;
}

// source.character.equipment-native-192.v1 / source program 1197647575df5c4e84030f14b1aad34b
