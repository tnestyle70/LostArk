SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase25(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[19]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[25].y=(g_SourceCharacterTime.xxxx).x;
    source[25].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[26].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[26].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[26].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[26].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[20].xyzw
    r1.xyzw = ((r0.xyzw)*(source[20].xyzw)).xyzw;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 33: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: mul r0.w, r0.w, cb0[22].x
    r0.w = ((r0.wwww)*(source[22].xxxx)).w;
    // 35: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 36: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 38: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 39: mul r0.w, r0.w, cb0[22].y
    r0.w = ((r0.wwww)*(source[22].yyyy)).w;
    // 40: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 41: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 42: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 43: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 45: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 46: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 47: mul r3.xy, r0.ywyy, cb0[21].xxxx
    r3.xy = ((r0.ywyy)*(source[21].xxxx)).xy;
    // 48: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 50: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 51: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[21].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[21].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 54: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 55: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 56: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 57: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 60: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 61: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 64: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 65: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 66: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 67: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 68: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 69: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 70: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 71: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 72: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 74: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 75: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 76: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 77: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 78: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 79: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 80: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 81: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 82: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 83: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 84: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 86: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 88: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 89: rcp r1.w, cb0[22].z
    r1.w = (1.0/(source[22].zzzz)).w;
    // 90: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 91: mul r6.xyz, r6.xyzx, cb0[22].zzzz
    r6.xyz = ((r6.xyzx)*(source[22].zzzz)).xyz;
    // 92: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 93: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 94: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 95: mad r6.xyz, r6.xyzx, cb0[22].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[22].zzzz)+(r10.xyzx)).xyz;
    // 96: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 97: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 98: add r1.w, cb0[22].z, l(1.000000)
    r1.w = ((source[22].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 100: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 102: mad r6.xyz, r2.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 103: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 104: mul r0.xyw, r0.xyxw, cb0[22].wwww
    r0.xyw = ((r0.xyxw)*(source[22].wwww)).xyw;
    // 105: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 106: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 107: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 108: dp3 r1.w, r3.xyzx, r4.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 109: mul_sat r2.w, r1.w, cb0[23].y
    r2.w = (saturate((r1.wwww)*(source[23].yyyy))).w;
    // 110: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul_sat r3.w, r4.z, cb0[23].y
    r3.w = (saturate((r4.zzzz)*(source[23].yyyy))).w;
    // 113: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: add_sat r3.w, r3.w, -cb0[23].z
    r3.w = (saturate((r3.wwww)+(-(source[23].zzzz)))).w;
    // 115: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 116: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 117: mul r4.w, r4.w, cb0[23].w
    r4.w = ((r4.wwww)*(source[23].wwww)).w;
    // 118: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 119: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 120: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 121: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 123: mad r2.xyz, cb0[21].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 124: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 126: mad r2.xyz, cb0[21].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[21].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 127: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 129: mul r6.xyz, r6.xyzx, cb0[23].xxxx
    r6.xyz = ((r6.xyzx)*(source[23].xxxx)).xyz;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 131: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 132: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 133: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 134: mad r2.xyz, r3.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 135: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 136: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 137: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 138: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 139: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 141: mul r3.w, r3.w, cb0[24].x
    r3.w = ((r3.wwww)*(source[24].xxxx)).w;
    // 142: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 143: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: div r4.w, cb0[24].y, r4.w
    r4.w = ((source[24].yyyy)/(r4.wwww)).w;
    // 147: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 148: mul r6.xyz, r0.xywx, r4.wwww
    r6.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 149: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 151: mad r1.xyz, cb0[21].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[21].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 152: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 154: mad r1.xyz, cb0[21].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[21].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 155: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 156: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 157: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 158: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 159: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 160: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r10.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r10.xywx))).xyz;
    // 161: mad r10.xyz, r10.zzzz, r11.xyzx, r10.xywx
    r10.xyz = ((r10.zzzz)*(r11.xyzx)+(r10.xywx)).xyz;
    // 162: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 164: mad r10.xyz, cb0[21].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[21].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 165: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 166: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 167: mad r10.xyz, cb0[21].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[21].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 171: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 172: mul r12.xyz, r1.xyzx, r10.xyzx
    r12.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 173: mad r1.xyz, r10.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 174: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 175: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 176: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r4.w, r4.w, cb0[24].z
    r4.w = ((r4.wwww)*(source[24].zzzz)).w;
    // 178: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 179: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 180: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 181: mul r6.xyz, r0.xywx, r2.yyyy
    r6.xyz = ((r0.xywx)*(r2.yyyy)).xyz;
    // 182: dp3 r2.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: mad r10.xyz, -r2.yyyy, r0.xywx, r2.zzzz
    r10.xyz = ((-(r2.yyyy))*(r0.xywx)+(r2.zzzz)).xyz;
    // 184: mad r6.xyz, cb0[21].yyyy, r10.xyzx, r6.xyzx
    r6.xyz = ((source[21].yyyy)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 185: dp3 r2.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 186: add r10.xyz, -r6.xyzx, r2.yyyy
    r10.xyz = ((-(r6.xyzx))+(r2.yyyy)).xyz;
    // 187: mad r6.xyz, cb0[21].zzzz, r10.xyzx, r6.xyzx
    r6.xyz = ((source[21].zzzz)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 188: dp3 r2.y, r1.xyzx, r1.xyzx
    r2.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 189: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 190: div r1.xyz, r1.xyzx, r2.yyyy
    r1.xyz = ((r1.xyzx)/(r2.yyyy)).xyz;
    // 191: dp3 r2.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 192: add r10.xyz, -r1.xyzx, r2.yyyy
    r10.xyz = ((-(r1.xyzx))+(r2.yyyy)).xyz;
    // 193: add r1.xyz, r1.xyzx, -r10.xyzx
    r1.xyz = ((r1.xyzx)+(-(r10.xyzx))).xyz;
    // 194: mul r10.xyz, cb0[14].xyzx, cb0[25].xxxx
    r10.xyz = ((source[14].xyzx)*(source[25].xxxx)).xyz;
    // 195: mul r10.xyz, r10.xyzx, cb0[26].wwww
    r10.xyz = ((r10.xyzx)*(source[26].wwww)).xyz;
    // 196: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 197: mad r13.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r13.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 198: add r2.y, r2.w, l(-1.000000)
    r2.y = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 199: mad r2.y, cb0[12].w, r2.y, l(1.000000)
    r2.y = ((source[12].wwww)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: mad r13.xyz, cb0[13].wwww, r13.xyzx, cb0[13].xyzx
    r13.xyz = ((source[13].wwww)*(r13.xyzx)+(source[13].xyzx)).xyz;
    // 201: mad r1.xyz, r1.xyzx, r10.xyzx, r13.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)+(r13.xyzx)).xyz;
    // 202: mad r1.xyz, r2.yyyy, cb0[12].xyzx, r1.xyzx
    r1.xyz = ((r2.yyyy)*(source[12].xyzx)+(r1.xyzx)).xyz;
    // 203: mad r1.xyz, r6.xyzx, r11.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 204: add r2.y, -|r4.z|, l(1.000000)
    r2.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 205: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 206: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 207: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 208: mul r2.y, r2.y, l(1.500000)
    r2.y = ((r2.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 209: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 210: mul r2.yzw, r2.yyyy, cb0[15].xxyz
    r2.yzw = ((r2.yyyy)*(source[15].xxyz)).yzw;
    // 211: movc r2.yzw, r1.wwww, l(0,0,0,0), r2.yyzw
    r2.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyzw)).yzw;
    // 212: add r1.xyz, r1.xyzx, r2.yzwy
    r1.xyz = ((r1.xyzx)+(r2.yzwy)).xyz;
    // 213: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 214: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 215: sqrt r2.y, r1.w
    r2.y = (sqrt(r1.wwww)).y;
    // 216: div r2.yzw, r9.xxyz, r2.yyyy
    r2.yzw = ((r9.xxyz)/(r2.yyyy)).yzw;
    // 217: dp3 r2.y, r2.yzwy, r4.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).y;
    // 218: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 219: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 220: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 221: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 222: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 223: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 224: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 225: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 226: div_sat r1.w, r2.y, r1.w
    r1.w = (saturate((r2.yyyy)/(r1.wwww))).w;
    // 227: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 228: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 229: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 230: mad r0.xyz, r3.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 231: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 232: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 233: mad r0.xyz, cb0[21].yyyy, r2.yzwy, r0.xyzx
    r0.xyz = ((source[21].yyyy)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 234: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 235: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 236: mad r0.xyz, cb0[21].zzzz, r2.yzwy, r0.xyzx
    r0.xyz = ((source[21].zzzz)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 237: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 238: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r0.w, r0.w, cb0[25].y
    r0.w = ((r0.wwww)*(source[25].yyyy)).w;
    // 240: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 241: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 242: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 244: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 245: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 246: add r1.w, -r2.x, cb0[3].x
    r1.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 247: mul r4.z, r1.w, l(0.125000)
    r4.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 248: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 249: mul r6.x, r1.w, l(0.125000)
    r6.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 250: mul r4.y, cb0[3].y, cb0[16].y
    r4.y = ((source[3].yyyy)*(source[16].yyyy)).y;
    // 251: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 252: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 253: add r2.yz, r4.xxyx, r6.xxyx
    r2.yz = ((r4.xxyx)+(r6.xxyx)).yz;
    // 254: add r2.yz, r2.yyzy, r4.zzwz
    r2.yz = ((r2.yyzy)+(r4.zzwz)).yz;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.yzyy, t6.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 256: mul r2.yzw, r0.wwww, r4.xxyz
    r2.yzw = ((r0.wwww)*(r4.xxyz)).yzw;
    // 257: mul r0.w, r2.x, r4.w
    r0.w = ((r2.xxxx)*(r4.wwww)).w;
    // 258: mad r2.xyz, r2.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 259: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 260: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 261: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 262: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 263: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 264: mad r2.xy, cb0[17].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 265: mul r0.w, cb0[17].y, cb0[25].y
    r0.w = ((source[17].yyyy)*(source[25].yyyy)).w;
    // 266: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 267: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 268: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 269: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 271: mul r1.w, cb0[17].x, l(0.001000)
    r1.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 272: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 273: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 274: dp2 r1.w, cb0[18].xyxx, r2.xyxx
    r1.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 275: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 276: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 277: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 278: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 279: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 280: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 281: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 282: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 283: mad r4.xyz, cb0[17].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[17].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 284: mul r2.xyz, r2.xyzx, cb0[17].zzzz
    r2.xyz = ((r2.xyzx)*(source[17].zzzz)).xyz;
    // 285: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 286: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 287: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 288: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 289: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 290: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 291: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 292: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 293: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 294: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 295: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 296: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 297: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 298: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 299: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 300: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 301: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 302: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 303: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 304: mad o0.xyz, r0.xyzx, cb0[29].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[29].xyzx)+(r1.xyzx)).xyz;
    // 305: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 306: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 307: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 308: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 309: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 310: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 311: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 312: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 313: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 314: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 315: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 316: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 317: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 318: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 319: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 320: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 321: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 322: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 323: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 324: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 325: ret
    return output;
}

// source.character.monster-6ff78ae19259.v1 / source program e5ff54c5c354204e951b91b524341cb3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase26(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[22].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[22].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[22].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[22].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 25: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r1.z, r1.z, cb0[18].x
    r1.z = ((r1.zzzz)*(source[18].xxxx)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 30: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 31: mul r1.z, r1.z, cb0[18].y
    r1.z = ((r1.zzzz)*(source[18].yyyy)).z;
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
    // 39: mul r3.xy, r1.xzxx, cb0[17].xxxx
    r3.xy = ((r1.xzxx)*(source[17].xxxx)).xy;
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
    // 45: mad r1.xzw, cb0[17].wwww, r1.xxzw, r3.xxyz
    r1.xzw = ((source[17].wwww)*(r1.xxzw)+(r3.xxyz)).xzw;
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
    // 79: sample_l_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s3, r0.x
    r5.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r5.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 80: log r9.xyz, r5.xyzx
    r9.xyz = (log2(r5.xyzx)).xyz;
    // 81: rcp r0.x, cb0[18].z
    r0.x = (1.0/(source[18].zzzz)).x;
    // 82: mul r10.xyz, r9.xyzx, r0.xxxx
    r10.xyz = ((r9.xyzx)*(r0.xxxx)).xyz;
    // 83: mul r9.xyz, r9.xyzx, cb0[18].zzzz
    r9.xyz = ((r9.xyzx)*(source[18].zzzz)).xyz;
    // 84: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 85: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 86: mul r10.xyz, r0.xxxx, r10.xyzx
    r10.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 87: mad r9.xyz, r9.xyzx, cb0[18].zzzz, r10.xyzx
    r9.xyz = ((r9.xyzx)*(source[18].zzzz)+(r10.xyzx)).xyz;
    // 88: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 89: mul r5.xyz, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 90: add r0.x, cb0[18].z, l(1.000000)
    r0.x = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 92: dp3 r0.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r5.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r5.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 94: mad r5.xyz, r3.wwww, r5.xyzx, cb0[7].xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(source[7].xyzx)).xyz;
    // 95: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 96: mul r5.xyz, r5.xyzx, cb0[18].wwww
    r5.xyz = ((r5.xyzx)*(source[18].wwww)).xyz;
    // 97: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 98: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 99: mad r2.xyz, cb0[17].yyyy, r9.xyzx, r2.xyzx
    r2.xyz = ((source[17].yyyy)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 100: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 102: mad r2.xyz, cb0[17].zzzz, r9.xyzx, r2.xyzx
    r2.xyz = ((source[17].zzzz)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 103: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 104: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 105: mul r9.xyz, r9.xyzx, cb0[19].xxxx
    r9.xyz = ((r9.xyzx)*(source[19].xxxx)).xyz;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 107: add r0.x, r10.y, r10.x
    r0.x = ((r10.yyyy)+(r10.xxxx)).x;
    // 108: add r0.x, r10.z, r0.x
    r0.x = ((r10.zzzz)+(r0.xxxx)).x;
    // 109: add_sat r0.x, r10.w, r0.x
    r0.x = (saturate((r10.wwww)+(r0.xxxx))).x;
    // 110: mad r2.xyz, r0.xxxx, r9.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 111: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 112: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 113: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 114: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 115: dp3 r0.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 117: mul r0.x, r0.x, cb0[20].x
    r0.x = ((r0.xxxx)*(source[20].xxxx)).x;
    // 118: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 119: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 120: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 122: div r2.w, cb0[20].y, r2.w
    r2.w = ((source[20].yyyy)/(r2.wwww)).w;
    // 123: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 124: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 125: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 126: dp3 r3.w, r3.xyzx, r8.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 127: mul_sat r4.w, r3.w, cb0[19].y
    r4.w = (saturate((r3.wwww)*(source[19].yyyy))).w;
    // 128: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul_sat r5.w, r8.z, cb0[19].y
    r5.w = (saturate((r8.zzzz)*(source[19].yyyy))).w;
    // 131: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: add_sat r5.w, r5.w, -cb0[19].z
    r5.w = (saturate((r5.wwww)+(-(source[19].zzzz)))).w;
    // 133: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 134: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 135: mul r6.w, r6.w, cb0[19].w
    r6.w = ((r6.wwww)*(source[19].wwww)).w;
    // 136: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 137: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 138: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 139: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 140: mul r9.xyz, r5.xyzx, r2.wwww
    r9.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 141: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r10.xyz, -r0.yzwy, r2.wwww
    r10.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 143: mad r0.yzw, cb0[17].yyyy, r10.xxyz, r0.yyzw
    r0.yzw = ((source[17].yyyy)*(r10.xxyz)+(r0.yyzw)).yzw;
    // 144: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: add r10.xyz, -r0.yzwy, r2.wwww
    r10.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 146: mad r0.yzw, cb0[17].zzzz, r10.xxyz, r0.yyzw
    r0.yzw = ((source[17].zzzz)*(r10.xxyz)+(r0.yyzw)).yzw;
    // 147: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 148: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 150: mad r10.xyz, cb0[17].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 151: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 152: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 153: mad r10.xyz, cb0[17].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 154: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 157: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 158: mul r12.xyz, r0.yzwy, r10.xyzx
    r12.xyz = ((r0.yzwy)*(r10.xyzx)).xyz;
    // 159: mad r0.yzw, r10.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r10.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 160: mul r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)*(r12.xyzx)).xyz;
    // 161: mad r2.xyz, r2.xyzx, r9.xyzx, -r5.xyzx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 162: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 164: mad r2.xyz, r2.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 165: frc r2.w, cb0[3].x
    r2.w = (frac(source[3].xxxx)).w;
    // 166: add r5.x, -r2.w, l(1.000000)
    r5.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 167: mul r5.yzw, r2.xxyz, r5.xxxx
    r5.yzw = ((r2.xxyz)*(r5.xxxx)).yzw;
    // 168: dp3 r6.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 169: mad r9.xyz, -r5.xxxx, r2.xyzx, r6.wwww
    r9.xyz = ((-(r5.xxxx))*(r2.xyzx)+(r6.wwww)).xyz;
    // 170: mad r5.xyz, cb0[17].yyyy, r9.xyzx, r5.yzwy
    r5.xyz = ((source[17].yyyy)*(r9.xyzx)+(r5.yzwy)).xyz;
    // 171: dp3 r5.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r9.xyz, -r5.xyzx, r5.wwww
    r9.xyz = ((-(r5.xyzx))+(r5.wwww)).xyz;
    // 173: mad r5.xyz, cb0[17].zzzz, r9.xyzx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 174: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 175: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 176: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 177: dp3 r5.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r9.xyz, -r0.yzwy, r5.wwww
    r9.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 179: add r0.yzw, r0.yyzw, -r9.xxyz
    r0.yzw = ((r0.yyzw)+(-(r9.xxyz))).yzw;
    // 180: mul r9.xyz, cb0[11].xyzx, cb0[21].xxxx
    r9.xyz = ((source[11].xyzx)*(source[21].xxxx)).xyz;
    // 181: mul r9.xyz, r9.xyzx, cb0[22].wwww
    r9.xyz = ((r9.xyzx)*(source[22].wwww)).xyz;
    // 182: mul r9.xyz, r4.wwww, r9.xyzx
    r9.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 183: mad r10.xyz, r4.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r10.xyz = ((r4.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 184: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 185: mad r4.w, cb0[9].w, r4.w, l(1.000000)
    r4.w = ((source[9].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: mad r10.xyz, cb0[10].wwww, r10.xyzx, cb0[10].xyzx
    r10.xyz = ((source[10].wwww)*(r10.xyzx)+(source[10].xyzx)).xyz;
    // 187: mad r0.yzw, r0.yyzw, r9.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r9.xxyz)+(r10.xxyz)).yzw;
    // 188: mad r0.yzw, r4.wwww, cb0[9].xxyz, r0.yyzw
    r0.yzw = ((r4.wwww)*(source[9].xxyz)+(r0.yyzw)).yzw;
    // 189: mad r0.yzw, r5.xxyz, r11.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 190: add r4.w, -|r8.z|, l(1.000000)
    r4.w = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 192: log r4.w, |r3.w|
    r4.w = (log2(abs(r3.wwww))).w;
    // 193: lt r3.w, |r3.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 194: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 195: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 196: mul r5.xyz, r4.wwww, cb0[12].xyzx
    r5.xyz = ((r4.wwww)*(source[12].xyzx)).xyz;
    // 197: movc r5.xyz, r3.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 198: add r0.yzw, r0.yyzw, r5.xxyz
    r0.yzw = ((r0.yyzw)+(r5.xxyz)).yzw;
    // 199: add r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)+(source[2].xxyz)).yzw;
    // 200: dp3 r3.w, r1.xzwx, r1.xzwx
    r3.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 201: sqrt r4.w, r3.w
    r4.w = (sqrt(r3.wwww)).w;
    // 202: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 203: dp3 r1.x, r1.xzwx, r8.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 204: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 206: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 207: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 208: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 209: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 210: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 211: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 212: div_sat r1.x, r1.x, r3.w
    r1.x = (saturate((r1.xxxx)/(r3.wwww))).x;
    // 213: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 214: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 215: mad r1.xyz, r1.xxxx, r2.xyzx, -r12.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r12.xyzx))).xyz;
    // 216: mad r1.xyz, r0.xxxx, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 217: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 219: mad r1.xyz, cb0[17].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 220: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 221: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 222: mad r1.xyz, cb0[17].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 223: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 224: add r0.x, -cb0[3].w, l(1.000000)
    r0.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 225: mul r0.x, r0.x, cb0[21].y
    r0.x = ((r0.xxxx)*(source[21].yyyy)).x;
    // 226: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 227: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 228: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 229: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 230: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 231: mad r0.x, r0.x, l(0.500000), cb0[3].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 232: add r1.w, -r2.w, cb0[3].x
    r1.w = ((-(r2.wwww))+(source[3].xxxx)).w;
    // 233: mul r5.z, r1.w, l(0.125000)
    r5.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 234: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 235: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 236: mul r5.y, cb0[3].y, cb0[13].y
    r5.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 237: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 238: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 239: add r2.xy, r2.xyxx, r5.xyxx
    r2.xy = ((r2.xyxx)+(r5.xyxx)).xy;
    // 240: add r2.xy, r2.xyxx, r5.zwzz
    r2.xy = ((r2.xyxx)+(r5.zwzz)).xy;
    // 241: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 242: mul r2.xyz, r0.xxxx, r5.xyzx
    r2.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 243: mul r0.x, r2.w, r5.w
    r0.x = ((r2.wwww)*(r5.wwww)).x;
    // 244: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 245: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 246: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 247: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 248: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 249: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 250: mad r2.xy, cb0[14].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[14].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 251: mul r0.x, cb0[14].y, cb0[21].y
    r0.x = ((source[14].yyyy)*(source[21].yyyy)).x;
    // 252: mul r0.x, r0.x, l(0.628319)
    r0.x = ((r0.xxxx)*(float4(0.628319,0.628319,0.628319,0.628319))).x;
    // 253: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 254: mul r5.y, r0.x, l(0.020000)
    r5.y = ((r0.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 255: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 256: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 257: mul r1.w, cb0[14].x, l(0.001000)
    r1.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 258: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 259: mad r2.xy, r1.wwww, r2.xyxx, r5.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r5.xyxx)).xy;
    // 260: dp2 r1.w, cb0[15].xyxx, r2.xyxx
    r1.w = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 261: dp2 r2.y, cb0[16].xyxx, r2.xyxx
    r2.y = (dot((source[16].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 262: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 263: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 264: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 265: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 266: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 267: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 268: mul_sat r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (saturate((r0.xxxx)*(r2.xyzx))).xyz;
    // 269: mad r5.xyz, cb0[14].zzzz, r2.xyzx, -r1.xyzx
    r5.xyz = ((source[14].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 270: mul r2.xyz, r2.xyzx, cb0[14].zzzz
    r2.xyz = ((r2.xyzx)*(source[14].zzzz)).xyz;
    // 271: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 272: mul r0.x, r0.x, l(3.000000)
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 273: mad r1.xyz, r0.xxxx, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 274: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 275: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 276: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 277: mul r2.xyz, r0.xxxx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 278: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 279: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 280: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 281: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 282: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 283: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 284: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 285: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 286: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 287: mad r0.xyz, r3.xyzx, r1.xyzx, r0.yzwy
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 288: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 289: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 290: mad o0.xyz, r1.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 291: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 292: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 293: dp3 r0.x, r4.xyzx, r2.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 294: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 295: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 296: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 297: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 298: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 299: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 300: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 301: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 302: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 303: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 304: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 305: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 306: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 307: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 308: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 309: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 310: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 311: ret
    return output;
}

// source.character.monster-4184f1980abb.v1 / source program 8b60d6d832ee434185ba9e07af60211c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase27(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[20]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[26].z=(g_SourceCharacterTime.xxxx).x;
    source[27].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[27].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[27].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[27].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[21].xyzw
    r1.xyzw = ((r0.xyzw)*(source[21].xyzw)).xyzw;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 33: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: mul r0.w, r0.w, cb0[23].x
    r0.w = ((r0.wwww)*(source[23].xxxx)).w;
    // 35: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 36: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 38: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 39: mul r0.w, r0.w, cb0[23].y
    r0.w = ((r0.wwww)*(source[23].yyyy)).w;
    // 40: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 41: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 42: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 43: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 45: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 46: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 47: mul r3.xy, r0.ywyy, cb0[22].xxxx
    r3.xy = ((r0.ywyy)*(source[22].xxxx)).xy;
    // 48: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 50: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 51: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[22].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[22].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 54: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 55: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 56: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 57: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 60: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 61: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 64: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 65: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 66: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 67: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 68: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 69: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 70: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 71: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 72: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 74: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 75: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 76: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 77: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 78: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 79: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 80: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 81: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 82: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 83: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 84: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 86: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 88: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 89: rcp r1.w, cb0[23].z
    r1.w = (1.0/(source[23].zzzz)).w;
    // 90: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 91: mul r6.xyz, r6.xyzx, cb0[23].zzzz
    r6.xyz = ((r6.xyzx)*(source[23].zzzz)).xyz;
    // 92: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 93: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 94: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 95: mad r6.xyz, r6.xyzx, cb0[23].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[23].zzzz)+(r10.xyzx)).xyz;
    // 96: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 97: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 98: add r1.w, cb0[23].z, l(1.000000)
    r1.w = ((source[23].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 100: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 102: mad r6.xyz, r2.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 103: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 104: mul r0.xyw, r0.xyxw, cb0[23].wwww
    r0.xyw = ((r0.xyxw)*(source[23].wwww)).xyw;
    // 105: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 106: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 107: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 108: dp3 r1.w, r3.xyzx, r4.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 109: mul_sat r2.w, r1.w, cb0[24].y
    r2.w = (saturate((r1.wwww)*(source[24].yyyy))).w;
    // 110: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul_sat r3.w, r4.z, cb0[24].y
    r3.w = (saturate((r4.zzzz)*(source[24].yyyy))).w;
    // 113: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: add_sat r3.w, r3.w, -cb0[24].z
    r3.w = (saturate((r3.wwww)+(-(source[24].zzzz)))).w;
    // 115: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 116: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 117: mul r4.w, r4.w, cb0[24].w
    r4.w = ((r4.wwww)*(source[24].wwww)).w;
    // 118: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 119: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 120: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 121: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 123: mad r2.xyz, cb0[22].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 124: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 126: mad r2.xyz, cb0[22].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 127: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 129: mul r6.xyz, r6.xyzx, cb0[24].xxxx
    r6.xyz = ((r6.xyzx)*(source[24].xxxx)).xyz;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 131: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 132: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 133: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 134: mad r2.xyz, r3.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 135: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 136: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 137: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 138: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 139: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 141: mul r3.w, r3.w, cb0[25].x
    r3.w = ((r3.wwww)*(source[25].xxxx)).w;
    // 142: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 143: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: div r4.w, cb0[25].y, r4.w
    r4.w = ((source[25].yyyy)/(r4.wwww)).w;
    // 147: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 148: mul r6.xyz, r0.xywx, r4.wwww
    r6.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 149: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 151: mad r1.xyz, cb0[22].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 152: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 154: mad r1.xyz, cb0[22].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 155: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 156: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 157: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 158: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 159: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 160: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r10.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r10.xywx))).xyz;
    // 161: mad r10.xyz, r10.zzzz, r11.xyzx, r10.xywx
    r10.xyz = ((r10.zzzz)*(r11.xyzx)+(r10.xywx)).xyz;
    // 162: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 164: mad r10.xyz, cb0[22].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[22].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 165: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 166: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 167: mad r10.xyz, cb0[22].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[22].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 171: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 172: mul r12.xyz, r1.xyzx, r10.xyzx
    r12.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 173: mad r1.xyz, r10.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 174: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 175: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 176: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r4.w, r4.w, cb0[25].z
    r4.w = ((r4.wwww)*(source[25].zzzz)).w;
    // 178: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 179: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t7.xyzw, s5, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 180: mul r6.xyz, cb0[12].xyzx, cb0[25].wwww
    r6.xyz = ((source[12].xyzx)*(source[25].wwww)).xyz;
    // 181: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 182: frc r4.w, cb0[3].x
    r4.w = (frac(source[3].xxxx)).w;
    // 183: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mad r2.xyz, r5.wwww, r0.xywx, r2.xyzx
    r2.xyz = ((r5.wwww)*(r0.xywx)+(r2.xyzx)).xyz;
    // 185: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: add r6.xyz, -r2.xyzx, r5.wwww
    r6.xyz = ((-(r2.xyzx))+(r5.wwww)).xyz;
    // 187: mad r2.xyz, cb0[22].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 188: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: add r6.xyz, -r2.xyzx, r5.wwww
    r6.xyz = ((-(r2.xyzx))+(r5.wwww)).xyz;
    // 190: mad r2.xyz, cb0[22].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 191: dp3 r5.w, r1.xyzx, r1.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 192: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 193: div r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)/(r5.wwww)).xyz;
    // 194: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 195: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 196: add r1.xyz, r1.xyzx, -r6.xyzx
    r1.xyz = ((r1.xyzx)+(-(r6.xyzx))).xyz;
    // 197: mul r6.xyz, cb0[15].xyzx, cb0[26].yyyy
    r6.xyz = ((source[15].xyzx)*(source[26].yyyy)).xyz;
    // 198: mul r6.xyz, r6.xyzx, cb0[27].xxxx
    r6.xyz = ((r6.xyzx)*(source[27].xxxx)).xyz;
    // 199: mul r6.xyz, r2.wwww, r6.xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 200: mad r10.xyz, r2.wwww, cb0[14].xyzx, -cb0[14].xyzx
    r10.xyz = ((r2.wwww)*(source[14].xyzx)+(-(source[14].xyzx))).xyz;
    // 201: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 202: mad r2.w, cb0[13].w, r2.w, l(1.000000)
    r2.w = ((source[13].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 203: mad r10.xyz, cb0[14].wwww, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((source[14].wwww)*(r10.xyzx)+(source[14].xyzx)).xyz;
    // 204: mad r1.xyz, r1.xyzx, r6.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(r10.xyzx)).xyz;
    // 205: mad r1.xyz, r2.wwww, cb0[13].xyzx, r1.xyzx
    r1.xyz = ((r2.wwww)*(source[13].xyzx)+(r1.xyzx)).xyz;
    // 206: mad r1.xyz, r2.xyzx, r11.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 207: add r2.x, -|r4.z|, l(1.000000)
    r2.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 208: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 209: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 210: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 211: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 212: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 213: mul r2.xyz, r2.xxxx, cb0[16].xyzx
    r2.xyz = ((r2.xxxx)*(source[16].xyzx)).xyz;
    // 214: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 215: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 216: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 217: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 218: sqrt r2.x, r1.w
    r2.x = (sqrt(r1.wwww)).x;
    // 219: div r2.xyz, r9.xyzx, r2.xxxx
    r2.xyz = ((r9.xyzx)/(r2.xxxx)).xyz;
    // 220: dp3 r2.x, r2.xyzx, r4.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 221: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 222: mul r2.y, |r2.x|, |r2.x|
    r2.y = ((abs(r2.xxxx))*(abs(r2.xxxx))).y;
    // 223: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 224: mul r2.y, r2.y, |r2.x|
    r2.y = ((r2.yyyy)*(abs(r2.xxxx))).y;
    // 225: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 226: movc r2.x, r2.x, l(0), r2.y
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).x;
    // 227: add r2.y, r2.x, l(-0.027778)
    r2.y = ((r2.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 228: mad r2.x, r2.x, r2.y, l(0.027778)
    r2.x = ((r2.xxxx)*(r2.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 229: div_sat r1.w, r2.x, r1.w
    r1.w = (saturate((r2.xxxx)/(r1.wwww))).w;
    // 230: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 231: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 232: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 233: mad r0.xyz, r3.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 234: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 235: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 236: mad r0.xyz, cb0[22].yyyy, r2.xyzx, r0.xyzx
    r0.xyz = ((source[22].yyyy)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 237: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 238: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 239: mad r0.xyz, cb0[22].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[22].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 240: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 241: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 242: mul r0.w, r0.w, cb0[26].z
    r0.w = ((r0.wwww)*(source[26].zzzz)).w;
    // 243: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 244: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 245: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 247: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 248: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 249: add r1.w, -r4.w, cb0[3].x
    r1.w = ((-(r4.wwww))+(source[3].xxxx)).w;
    // 250: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 251: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 252: mul r4.x, r1.w, l(0.125000)
    r4.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 253: mul r2.y, cb0[3].y, cb0[17].y
    r2.y = ((source[3].yyyy)*(source[17].yyyy)).y;
    // 254: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 255: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 256: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 257: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 258: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 259: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 260: mul r0.w, r4.w, r2.w
    r0.w = ((r4.wwww)*(r2.wwww)).w;
    // 261: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 262: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 263: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 264: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 265: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 266: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 267: mad r2.xy, cb0[18].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[18].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 268: mul r0.w, cb0[18].y, cb0[26].z
    r0.w = ((source[18].yyyy)*(source[26].zzzz)).w;
    // 269: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 270: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 271: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 272: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 273: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 274: mul r1.w, cb0[18].x, l(0.001000)
    r1.w = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 275: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 276: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 277: dp2 r1.w, cb0[19].xyxx, r2.xyxx
    r1.w = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 278: dp2 r2.y, cb0[20].xyxx, r2.xyxx
    r2.y = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 279: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 280: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 281: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 282: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 283: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 284: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 285: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 286: mad r4.xyz, cb0[18].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[18].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 287: mul r2.xyz, r2.xyzx, cb0[18].zzzz
    r2.xyz = ((r2.xyzx)*(source[18].zzzz)).xyz;
    // 288: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 289: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 290: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 291: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 292: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 293: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 294: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 295: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 296: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 297: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 298: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 299: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 300: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 301: mul r3.yzw, r3.yyyy, cb0[29].xxyz
    r3.yzw = ((r3.yyyy)*(source[29].xxyz)).yzw;
    // 302: mad r3.xyz, r3.xxxx, cb0[28].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[28].xyzx)+(r3.yzwy)).xyz;
    // 303: mul r3.xyz, r3.xyzx, cb0[30].wwww
    r3.xyz = ((r3.xyzx)*(source[30].wwww)).xyz;
    // 304: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 305: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 306: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 307: mad o0.xyz, r0.xyzx, cb0[30].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[30].xyzx)+(r1.xyzx)).xyz;
    // 308: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 309: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
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
    // 316: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 317: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 318: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 319: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 320: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 321: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 322: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 323: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 324: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 325: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 326: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 327: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 328: ret
    return output;
}

// source.character.monster-ce6425dcbdeb.v1 / source program b00453a918c9664896f17968a88db080
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase28(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].z=(g_SourceCharacterTime.xxxx).x;
    source[25].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[26].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[26].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[26].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[26].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[19].xyzw
    r1.xyzw = ((r0.xyzw)*(source[19].xyzw)).xyzw;
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
    // 15: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 16: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 17: add r0.y, -r0.x, l(1000.000000)
    r0.y = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).y;
    // 18: mad r0.x, cb0[24].w, r0.y, r0.x
    r0.x = ((source[24].wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 19: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 20: mad r0.x, cb0[24].y, cb0[24].z, r0.x
    r0.x = ((source[24].yyyy)*(source[24].zzzz)+(r0.xxxx)).x;
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
    // 27: mad r0.x, r0.x, l(0.500000), cb0[24].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[24].xxxx)).x;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v4.xyxx, t7.wxyz, s5, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 29: mul r2.xyz, cb0[10].xyzx, cb0[23].wwww
    r2.xyz = ((source[10].xyzx)*(source[23].wwww)).xyz;
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
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: log r3.x, |r2.w|
    r3.x = (log2(abs(r2.wwww))).x;
    // 50: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 51: mul r3.x, r3.x, cb0[21].x
    r3.x = ((r3.xxxx)*(source[21].xxxx)).x;
    // 52: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 53: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: movc r2.w, r2.w, l(0), r3.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 55: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 56: mul r3.x, r3.x, cb0[21].y
    r3.x = ((r3.xxxx)*(source[21].yyyy)).x;
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
    // 64: mul r3.xy, r3.xyxx, cb0[20].xxxx
    r3.xy = ((r3.xyxx)*(source[20].xxxx)).xy;
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
    // 70: mad r4.xyz, cb0[20].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
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
    // 104: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s3, r0.w
    r6.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.wwww).x)).xyzw).xyz;
    // 105: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 106: rcp r0.w, cb0[21].z
    r0.w = (1.0/(source[21].zzzz)).w;
    // 107: mul r11.xyz, r10.xyzx, r0.wwww
    r11.xyz = ((r10.xyzx)*(r0.wwww)).xyz;
    // 108: mul r10.xyz, r10.xyzx, cb0[21].zzzz
    r10.xyz = ((r10.xyzx)*(source[21].zzzz)).xyz;
    // 109: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 110: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 111: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 112: mad r10.xyz, r10.xyzx, cb0[21].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[21].zzzz)+(r11.xyzx)).xyz;
    // 113: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 114: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 115: add r0.w, cb0[21].z, l(1.000000)
    r0.w = ((source[21].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 117: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 118: add r6.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r6.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 119: mad r6.xyz, r3.wwww, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[8].xyzx)).xyz;
    // 120: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 121: mul r6.xyz, r6.xyzx, cb0[21].wwww
    r6.xyz = ((r6.xyzx)*(source[21].wwww)).xyz;
    // 122: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: add r10.xyz, -r2.xyzx, r0.wwww
    r10.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 124: mad r2.xyz, cb0[20].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 125: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r10.xyz, -r2.xyzx, r0.wwww
    r10.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 127: mad r2.xyz, cb0[20].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 128: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: add r10.xyz, -r2.xyzx, r0.wwww
    r10.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 130: mul r10.xyz, r10.xyzx, cb0[22].xxxx
    r10.xyz = ((r10.xyzx)*(source[22].xxxx)).xyz;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 132: add r0.w, r11.y, r11.x
    r0.w = ((r11.yyyy)+(r11.xxxx)).w;
    // 133: add r0.w, r11.z, r0.w
    r0.w = ((r11.zzzz)+(r0.wwww)).w;
    // 134: add_sat r0.w, r11.w, r0.w
    r0.w = (saturate((r11.wwww)+(r0.wwww))).w;
    // 135: mad r2.xyz, r0.wwww, r10.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 136: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 137: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 138: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 139: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 140: dp3 r0.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 141: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 142: mul r0.w, r0.w, cb0[23].x
    r0.w = ((r0.wwww)*(source[23].xxxx)).w;
    // 143: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 144: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mad r1.w, -r0.w, r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 147: div r1.w, cb0[23].y, r1.w
    r1.w = ((source[23].yyyy)/(r1.wwww)).w;
    // 148: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 149: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 150: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 151: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 152: mul_sat r4.w, r3.w, cb0[22].y
    r4.w = (saturate((r3.wwww)*(source[22].yyyy))).w;
    // 153: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: mul_sat r5.w, r4.z, cb0[22].y
    r5.w = (saturate((r4.zzzz)*(source[22].yyyy))).w;
    // 156: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: add_sat r5.w, r5.w, -cb0[22].z
    r5.w = (saturate((r5.wwww)+(-(source[22].zzzz)))).w;
    // 158: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 159: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 160: mul r6.w, r6.w, cb0[22].w
    r6.w = ((r6.wwww)*(source[22].wwww)).w;
    // 161: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 162: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 163: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 164: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 165: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 166: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 167: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 168: mad r1.xyz, cb0[20].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 169: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 171: mad r1.xyz, cb0[20].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 172: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 173: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 174: mad r12.xyz, -cb0[5].wwww, cb0[5].xyzx, r1.wwww
    r12.xyz = ((-(source[5].wwww))*(source[5].xyzx)+(r1.wwww)).xyz;
    // 175: mad r11.xyz, cb0[20].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[20].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 176: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 177: add r12.xyz, -r11.xyzx, r1.wwww
    r12.xyz = ((-(r11.xyzx))+(r1.wwww)).xyz;
    // 178: mad r11.xyz, cb0[20].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[20].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 179: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 182: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 183: mul r13.xyz, r1.xyzx, r11.xyzx
    r13.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 184: mad r1.xyz, r11.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r11.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 185: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 186: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 187: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 188: mul r1.w, r1.w, cb0[23].z
    r1.w = ((r1.wwww)*(source[23].zzzz)).w;
    // 189: mad r2.xyz, r1.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 190: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 191: add r5.w, -r1.w, l(1.000000)
    r5.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 192: mad r0.xyz, r5.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r5.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 193: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 194: add r6.xyz, -r0.xyzx, r5.wwww
    r6.xyz = ((-(r0.xyzx))+(r5.wwww)).xyz;
    // 195: mad r0.xyz, cb0[20].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[20].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 196: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 197: add r6.xyz, -r0.xyzx, r5.wwww
    r6.xyz = ((-(r0.xyzx))+(r5.wwww)).xyz;
    // 198: mad r0.xyz, cb0[20].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 199: dp3 r5.w, r1.xyzx, r1.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 200: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 201: div r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)/(r5.wwww)).xyz;
    // 202: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 203: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 204: add r1.xyz, r1.xyzx, -r6.xyzx
    r1.xyz = ((r1.xyzx)+(-(r6.xyzx))).xyz;
    // 205: mul r6.xyz, cb0[13].xyzx, cb0[25].yyyy
    r6.xyz = ((source[13].xyzx)*(source[25].yyyy)).xyz;
    // 206: mul r6.xyz, r6.xyzx, cb0[26].wwww
    r6.xyz = ((r6.xyzx)*(source[26].wwww)).xyz;
    // 207: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 208: mad r10.xyz, r4.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r10.xyz = ((r4.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 209: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 210: mad r4.w, cb0[11].w, r4.w, l(1.000000)
    r4.w = ((source[11].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 211: mad r10.xyz, cb0[12].wwww, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((source[12].wwww)*(r10.xyzx)+(source[12].xyzx)).xyz;
    // 212: mad r1.xyz, r1.xyzx, r6.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(r10.xyzx)).xyz;
    // 213: mad r1.xyz, r4.wwww, cb0[11].xyzx, r1.xyzx
    r1.xyz = ((r4.wwww)*(source[11].xyzx)+(r1.xyzx)).xyz;
    // 214: mad r0.xyz, r0.xyzx, r12.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)+(r1.xyzx)).xyz;
    // 215: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 216: mul r1.x, r3.w, r1.x
    r1.x = ((r3.wwww)*(r1.xxxx)).x;
    // 217: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 218: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 219: mul r1.y, r1.y, l(1.500000)
    r1.y = ((r1.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 220: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 221: mul r6.xyz, r1.yyyy, cb0[14].xyzx
    r6.xyz = ((r1.yyyy)*(source[14].xyzx)).xyz;
    // 222: movc r1.xyz, r1.xxxx, l(0,0,0,0), r6.xyzx
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xyzx)).xyz;
    // 223: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 224: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 225: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 226: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 227: div r6.xyz, r9.xyzx, r1.yyyy
    r6.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 228: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 229: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 230: mul r1.z, |r1.y|, |r1.y|
    r1.z = ((abs(r1.yyyy))*(abs(r1.yyyy))).z;
    // 231: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 232: mul r1.z, r1.z, |r1.y|
    r1.z = ((r1.zzzz)*(abs(r1.yyyy))).z;
    // 233: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 234: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 235: add r1.z, r1.y, l(-0.027778)
    r1.z = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 236: mad r1.y, r1.y, r1.z, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 237: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 238: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 239: mul r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)*(r2.wwww)).x;
    // 240: mad r1.xyz, r1.xxxx, r2.xyzx, -r13.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 241: mad r1.xyz, r0.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 242: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 243: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 244: mad r1.xyz, cb0[20].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 245: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 246: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 247: mad r1.xyz, cb0[20].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 248: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 249: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 250: mul r0.w, r0.w, cb0[24].z
    r0.w = ((r0.wwww)*(source[24].zzzz)).w;
    // 251: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 252: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 253: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 254: mul r2.x, cb0[4].z, l(1.500000)
    r2.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 255: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 256: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 257: add r2.x, -r1.w, cb0[4].x
    r2.x = ((-(r1.wwww))+(source[4].xxxx)).x;
    // 258: mul r2.z, r2.x, l(0.125000)
    r2.z = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 259: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 260: mul r4.x, r3.w, l(0.125000)
    r4.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 261: mul r2.y, cb0[4].y, cb0[15].y
    r2.y = ((source[4].yyyy)*(source[15].yyyy)).y;
    // 262: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 263: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 264: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 265: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 266: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 267: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 268: mul r0.w, r1.w, r2.w
    r0.w = ((r1.wwww)*(r2.wwww)).w;
    // 269: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 270: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 271: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 272: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 273: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 274: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 275: mad r2.xy, cb0[16].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[16].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 276: mul r0.w, cb0[16].y, cb0[24].z
    r0.w = ((source[16].yyyy)*(source[24].zzzz)).w;
    // 277: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 278: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 279: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 280: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 281: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 282: mul r1.w, cb0[16].x, l(0.001000)
    r1.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 283: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 284: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 285: dp2 r1.w, cb0[17].xyxx, r2.xyxx
    r1.w = (dot((source[17].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 286: dp2 r2.y, cb0[18].xyxx, r2.xyxx
    r2.y = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 287: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 288: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 289: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 290: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 291: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 292: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 293: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 294: mad r4.xyz, cb0[16].zzzz, r2.xyzx, -r1.xyzx
    r4.xyz = ((source[16].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 295: mul r2.xyz, r2.xyzx, cb0[16].zzzz
    r2.xyz = ((r2.xyzx)*(source[16].zzzz)).xyz;
    // 296: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 297: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 298: mad r1.xyz, r0.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 299: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 300: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 301: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 302: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 303: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 304: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 305: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 306: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 307: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 308: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 309: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 310: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 311: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 312: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 313: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 314: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 315: mad o0.xyz, r1.xyzx, cb0[29].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[29].xyzx)+(r0.xyzx)).xyz;
    // 316: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 317: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 318: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 319: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 320: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 321: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 322: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 323: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 324: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 325: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 326: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 327: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 328: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 329: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 330: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 331: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 332: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 333: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 334: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 335: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 336: ret
    return output;
}

// source.character.monster-63e86c4fac91.v1 / source program 2621fdf5b9a2b34a992dda44d574fff8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase29(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0].xy = 1.0; source[1].w = 1.0;
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[22].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[22].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[22].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[22].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: mul r0.xyz, v8.yyyy, cb1[1].xywx
    r0.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 2: mad r0.xyz, cb1[0].xywx, v8.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v8.xxxx)+(r0.xyzx)).xyz;
    // 3: mad r0.xyz, cb1[2].xywx, v8.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v8.zzzz)+(r0.xyzx)).xyz;
    // 4: mad r0.xyz, cb1[3].xywx, v8.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v8.wwww)+(r0.xyzx)).xyz;
    // 5: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 6: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 7: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 8: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 9: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 10: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 11: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 12: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 15: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 17: add r0.z, -r1.w, l(1.000000)
    r0.z = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 19: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 20: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 21: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 22: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 24: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 25: mul r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)*(source[18].yyyy)).w;
    // 26: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 27: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 28: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 29: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 31: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 32: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 33: mul r2.xy, r0.ywyy, cb0[17].xxxx
    r2.xy = ((r0.ywyy)*(source[17].xxxx)).xy;
    // 34: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 36: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 37: add r2.z, r0.y, l(0.000010)
    r2.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: add r3.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 39: mad r3.xyz, cb0[17].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[17].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 40: dp3 r0.y, r3.xyzx, r3.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 41: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 42: div r3.xyz, r3.xyzx, r0.yyyy
    r3.xyz = ((r3.xyzx)/(r0.yyyy)).xyz;
    // 43: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 44: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 45: mul r4.xyz, r0.yyyy, v0.xyzx
    r4.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 46: dp3 r5.x, r4.xyzx, r3.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 47: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 48: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 49: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 50: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 51: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 52: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 53: dp3 r5.y, r7.xyzx, r3.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 54: dp3 r5.z, r6.xyzx, r3.xyzx
    r5.z = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 55: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 56: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 57: mul r3.xyz, r0.yyyy, v6.xyzx
    r3.xyz = ((r0.yyyy)*(v6.xyzx)).xyz;
    // 58: mad r8.xyz, v6.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((v6.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 59: dp3 r9.y, r7.xyzx, r3.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 60: dp3 r9.x, r4.xyzx, r3.xyzx
    r9.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 61: dp3 r9.z, r6.xyzx, r3.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 62: dp3 r0.y, r5.xyzx, r9.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 63: mul r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)*(r0.yyyy)).xyz;
    // 64: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 65: mov r5.w, -r5.x
    r5.w = (-(r5.xxxx)).w;
    // 66: dp2 r0.y, r5.ywyy, r5.ywyy
    r0.y = (dot((r5.ywyy).xy,(r5.ywyy).xy).xxxx).y;
    // 67: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 68: div r0.yw, r5.yyyw, r0.yyyy
    r0.yw = ((r5.yyyw)/(r0.yyyy)).yw;
    // 69: mad r1.w, -r5.z, l(0.250000), l(0.250000)
    r1.w = ((-(r5.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 70: add r2.w, r5.z, l(1.000000)
    r2.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 72: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 73: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t3.xywz, s3, r0.x
    r0.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 74: log r5.xyz, r0.xywx
    r5.xyz = (log2(r0.xywx)).xyz;
    // 75: rcp r1.w, cb0[18].z
    r1.w = (1.0/(source[18].zzzz)).w;
    // 76: mul r9.xyz, r5.xyzx, r1.wwww
    r9.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 77: mul r5.xyz, r5.xyzx, cb0[18].zzzz
    r5.xyz = ((r5.xyzx)*(source[18].zzzz)).xyz;
    // 78: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 79: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 80: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 81: mad r5.xyz, r5.xyzx, cb0[18].zzzz, r9.xyzx
    r5.xyz = ((r5.xyzx)*(source[18].zzzz)+(r9.xyzx)).xyz;
    // 82: add r0.xyw, r0.xyxw, r5.xyxz
    r0.xyw = ((r0.xyxw)+(r5.xyxz)).xyw;
    // 83: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 84: add r1.w, cb0[18].z, l(1.000000)
    r1.w = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 86: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 87: add r5.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r5.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 88: mad r5.xyz, r2.wwww, r5.xyzx, cb0[7].xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(source[7].xyzx)).xyz;
    // 89: mul r0.xyw, r0.xxxx, r5.xyxz
    r0.xyw = ((r0.xxxx)*(r5.xyxz)).xyw;
    // 90: mul r0.xyw, r0.xyxw, cb0[18].wwww
    r0.xyw = ((r0.xyxw)*(source[18].wwww)).xyw;
    // 91: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r5.xyz, -r1.xyzx, r1.wwww
    r5.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 93: mad r1.xyz, cb0[17].yyyy, r5.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 94: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: add r5.xyz, -r1.xyzx, r1.wwww
    r5.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 96: mad r1.xyz, cb0[17].zzzz, r5.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 97: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r5.xyz, -r1.xyzx, r1.wwww
    r5.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 99: mul r5.xyz, r5.xyzx, cb0[19].xxxx
    r5.xyz = ((r5.xyzx)*(source[19].xxxx)).xyz;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: add r1.w, r9.y, r9.x
    r1.w = ((r9.yyyy)+(r9.xxxx)).w;
    // 102: add r1.w, r9.z, r1.w
    r1.w = ((r9.zzzz)+(r1.wwww)).w;
    // 103: add_sat r1.w, r9.w, r1.w
    r1.w = (saturate((r9.wwww)+(r1.wwww))).w;
    // 104: mad r1.xyz, r1.wwww, r5.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 105: max r5.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 106: log r5.xyz, r5.xyzx
    r5.xyz = (log2(r5.xyzx)).xyz;
    // 107: mul r5.xyz, r5.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 108: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 109: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 111: mul r1.w, r1.w, cb0[20].x
    r1.w = ((r1.wwww)*(source[20].xxxx)).w;
    // 112: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 113: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 116: div r2.w, cb0[20].y, r2.w
    r2.w = ((source[20].yyyy)/(r2.wwww)).w;
    // 117: dp3 r3.w, r2.xyzx, r2.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 118: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 119: div r2.xyz, r2.xyzx, r3.wwww
    r2.xyz = ((r2.xyzx)/(r3.wwww)).xyz;
    // 120: dp3 r3.w, r2.xyzx, r3.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 121: mul_sat r4.w, r3.w, cb0[19].y
    r4.w = (saturate((r3.wwww)*(source[19].yyyy))).w;
    // 122: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mul_sat r5.x, r3.z, cb0[19].y
    r5.x = (saturate((r3.zzzz)*(source[19].yyyy))).x;
    // 125: add r5.x, -r5.x, l(1.000000)
    r5.x = ((-(r5.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 126: add_sat r5.x, r5.x, -cb0[19].z
    r5.x = (saturate((r5.xxxx)+(-(source[19].zzzz)))).x;
    // 127: log r5.y, r5.x
    r5.y = (log2(r5.xxxx)).y;
    // 128: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 129: mul r5.y, r5.y, cb0[19].w
    r5.y = ((r5.yyyy)*(source[19].wwww)).y;
    // 130: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 131: mul r4.w, r4.w, r5.y
    r4.w = ((r4.wwww)*(r5.yyyy)).w;
    // 132: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 133: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 134: mul r5.xyz, r0.xywx, r2.wwww
    r5.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 135: mul r9.xyz, cb0[4].xyzx, cb0[4].wwww
    r9.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 136: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: mad r10.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r10.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 138: mad r9.xyz, cb0[17].yyyy, r10.xyzx, r9.xyzx
    r9.xyz = ((source[17].yyyy)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 139: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: add r10.xyz, -r9.xyzx, r2.wwww
    r10.xyz = ((-(r9.xyzx))+(r2.wwww)).xyz;
    // 141: mad r9.xyz, cb0[17].zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((source[17].zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 142: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 143: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 144: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 145: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 147: dp3 r2.w, r11.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 148: add r12.xyz, -r11.yzwy, r2.wwww
    r12.xyz = ((-(r11.yzwy))+(r2.wwww)).xyz;
    // 149: mad r11.yzw, cb0[17].yyyy, r12.xxyz, r11.yyzw
    r11.yzw = ((source[17].yyyy)*(r12.xxyz)+(r11.yyzw)).yzw;
    // 150: mov_sat r11.x, r11.x
    r11.x = (saturate(r11.xxxx)).x;
    // 151: mul_sat r2.w, r11.x, cb0[23].x
    r2.w = (saturate((r11.xxxx)*(source[23].xxxx))).w;
    // 152: mul o0.w, r2.w, cb0[1].w
    output.targets[0].w = ((r2.wwww)*(source[1].wwww)).w;
    // 153: dp3 r2.w, r11.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 154: add r12.xyz, -r11.yzwy, r2.wwww
    r12.xyz = ((-(r11.yzwy))+(r2.wwww)).xyz;
    // 155: mad r11.xyz, cb0[17].zzzz, r12.xyzx, r11.yzwy
    r11.xyz = ((source[17].zzzz)*(r12.xyzx)+(r11.yzwy)).xyz;
    // 156: mul r12.xyz, r9.xyzx, r11.xyzx
    r12.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 157: mad r9.xyz, r9.xyzx, r11.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = ((r9.xyzx)*(r11.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 158: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 159: mad r1.xyz, r1.xyzx, r5.xyzx, -r0.xywx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(-(r0.xywx))).xyz;
    // 160: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 162: mad r0.xyw, r2.wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 163: frc r1.x, cb0[3].x
    r1.x = (frac(source[3].xxxx)).x;
    // 164: add r1.y, -r1.x, l(1.000000)
    r1.y = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: mul r5.xyz, r0.xywx, r1.yyyy
    r5.xyz = ((r0.xywx)*(r1.yyyy)).xyz;
    // 166: dp3 r1.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 167: mad r11.xyz, -r1.yyyy, r0.xywx, r1.zzzz
    r11.xyz = ((-(r1.yyyy))*(r0.xywx)+(r1.zzzz)).xyz;
    // 168: mad r5.xyz, cb0[17].yyyy, r11.xyzx, r5.xyzx
    r5.xyz = ((source[17].yyyy)*(r11.xyzx)+(r5.xyzx)).xyz;
    // 169: dp3 r1.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 170: add r11.xyz, -r5.xyzx, r1.yyyy
    r11.xyz = ((-(r5.xyzx))+(r1.yyyy)).xyz;
    // 171: mad r5.xyz, cb0[17].zzzz, r11.xyzx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r11.xyzx)+(r5.xyzx)).xyz;
    // 172: dp3 r1.y, r9.xyzx, r9.xyzx
    r1.y = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 173: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 174: div r9.xyz, r9.xyzx, r1.yyyy
    r9.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 175: dp3 r1.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 176: add r11.xyz, -r9.xyzx, r1.yyyy
    r11.xyz = ((-(r9.xyzx))+(r1.yyyy)).xyz;
    // 177: add r9.xyz, r9.xyzx, -r11.xyzx
    r9.xyz = ((r9.xyzx)+(-(r11.xyzx))).xyz;
    // 178: mul r11.xyz, cb0[11].xyzx, cb0[21].xxxx
    r11.xyz = ((source[11].xyzx)*(source[21].xxxx)).xyz;
    // 179: mul r11.xyz, r11.xyzx, cb0[22].wwww
    r11.xyz = ((r11.xyzx)*(source[22].wwww)).xyz;
    // 180: mul r11.xyz, r4.wwww, r11.xyzx
    r11.xyz = ((r4.wwww)*(r11.xyzx)).xyz;
    // 181: mad r13.xyz, r4.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r13.xyz = ((r4.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 182: add r1.y, r4.w, l(-1.000000)
    r1.y = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 183: mad r1.y, cb0[9].w, r1.y, l(1.000000)
    r1.y = ((source[9].wwww)*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 184: mad r13.xyz, cb0[10].wwww, r13.xyzx, cb0[10].xyzx
    r13.xyz = ((source[10].wwww)*(r13.xyzx)+(source[10].xyzx)).xyz;
    // 185: mad r9.xyz, r9.xyzx, r11.xyzx, r13.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)+(r13.xyzx)).xyz;
    // 186: mad r9.xyz, r1.yyyy, cb0[9].xyzx, r9.xyzx
    r9.xyz = ((r1.yyyy)*(source[9].xyzx)+(r9.xyzx)).xyz;
    // 187: mad r5.xyz, r5.xyzx, r10.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 188: add r1.y, -|r3.z|, l(1.000000)
    r1.y = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 189: mul r1.y, r3.w, r1.y
    r1.y = ((r3.wwww)*(r1.yyyy)).y;
    // 190: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 191: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 192: mul r1.z, r1.z, l(1.500000)
    r1.z = ((r1.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 193: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 194: mul r9.xyz, r1.zzzz, cb0[12].xyzx
    r9.xyz = ((r1.zzzz)*(source[12].xyzx)).xyz;
    // 195: movc r9.xyz, r1.yyyy, l(0,0,0,0), r9.xyzx
    r9.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xyzx)).xyz;
    // 196: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 197: add r5.xyz, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r5.xyzx)+(source[2].xyzx)).xyz;
    // 198: dp3 r1.y, r8.xyzx, r8.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 199: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 200: div r8.xyz, r8.xyzx, r1.zzzz
    r8.xyz = ((r8.xyzx)/(r1.zzzz)).xyz;
    // 201: dp3 r1.z, r8.xyzx, r3.xyzx
    r1.z = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 202: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 203: mul r2.w, |r1.z|, |r1.z|
    r2.w = ((abs(r1.zzzz))*(abs(r1.zzzz))).w;
    // 204: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 205: mul r2.w, |r1.z|, r2.w
    r2.w = ((abs(r1.zzzz))*(r2.wwww)).w;
    // 206: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 207: movc r1.z, r1.z, l(0), r2.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 208: add r2.w, r1.z, l(-0.027778)
    r2.w = ((r1.zzzz)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 209: mad r1.z, r1.z, r2.w, l(0.027778)
    r1.z = ((r1.zzzz)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).z;
    // 210: div_sat r1.y, r1.z, r1.y
    r1.y = (saturate((r1.zzzz)/(r1.yyyy))).y;
    // 211: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 212: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 213: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 214: mad r0.xyz, r1.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 215: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 216: add r1.yzw, -r0.xxyz, r0.wwww
    r1.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 217: mad r0.xyz, cb0[17].yyyy, r1.yzwy, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r1.yzwy)+(r0.xyzx)).xyz;
    // 218: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 219: add r1.yzw, -r0.xxyz, r0.wwww
    r1.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 220: mad r0.xyz, cb0[17].zzzz, r1.yzwy, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r1.yzwy)+(r0.xyzx)).xyz;
    // 221: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 222: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 224: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 225: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 226: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 227: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 228: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 229: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 230: add r1.y, -r1.x, cb0[3].x
    r1.y = ((-(r1.xxxx))+(source[3].xxxx)).y;
    // 231: mul r3.z, r1.y, l(0.125000)
    r3.z = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 232: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 233: mul r8.x, r1.y, l(0.125000)
    r8.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 234: mul r3.y, cb0[3].y, cb0[13].y
    r3.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 235: mov r8.y, v4.y
    r8.y = (v4.yyyy).y;
    // 236: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 237: add r1.yz, r3.xxyx, r8.xxyx
    r1.yz = ((r3.xxyx)+(r8.xxyx)).yz;
    // 238: add r1.yz, r1.yyzy, r3.zzwz
    r1.yz = ((r1.yyzy)+(r3.zzwz)).yz;
    // 239: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.yzyy, t5.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 240: mul r1.yzw, r0.wwww, r3.xxyz
    r1.yzw = ((r0.wwww)*(r3.xxyz)).yzw;
    // 241: mul r0.w, r1.x, r3.w
    r0.w = ((r1.xxxx)*(r3.wwww)).w;
    // 242: mad r1.xyz, r1.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 243: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 244: add r1.xyzw, v8.yzxy, cb0[0].yzxy
    r1.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 245: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 246: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 247: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 248: mad r1.xy, cb0[14].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[14].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 249: mul r0.w, cb0[14].y, cb0[21].y
    r0.w = ((source[14].yyyy)*(source[21].yyyy)).w;
    // 250: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 251: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 252: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 253: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 254: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 255: mul r1.z, cb0[14].x, l(0.001000)
    r1.z = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 256: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 257: mad r1.xy, r1.zzzz, r1.xyxx, r3.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r3.xyxx)).xy;
    // 258: dp2 r1.z, cb0[15].xyxx, r1.xyxx
    r1.z = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 259: dp2 r1.y, cb0[16].xyxx, r1.xyxx
    r1.y = (dot((source[16].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 260: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 261: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 262: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 263: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 264: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 265: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 266: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 267: mad r3.xyz, cb0[14].zzzz, r1.xyzx, -r0.xyzx
    r3.xyz = ((source[14].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 268: mul r1.xyz, r1.xyzx, cb0[14].zzzz
    r1.xyz = ((r1.xyzx)*(source[14].zzzz)).xyz;
    // 269: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 270: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 271: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 272: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 273: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 274: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 275: mul r1.xyz, r0.wwww, r2.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 276: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 277: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 278: mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 279: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 280: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 281: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 282: mul r2.yzw, r2.yyyy, cb0[25].xxyz
    r2.yzw = ((r2.yyyy)*(source[25].xxyz)).yzw;
    // 283: mad r2.xyz, r2.xxxx, cb0[24].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[24].xyzx)+(r2.yzwy)).xyz;
    // 284: mul r2.xyz, r2.xyzx, cb0[26].wwww
    r2.xyz = ((r2.xyzx)*(source[26].wwww)).xyz;
    // 285: mad r3.xyz, r2.xyzx, r0.xyzx, r5.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 286: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 287: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 288: mad r2.xyz, r0.xyzx, cb0[26].xyzx, r3.xyzx
    r2.xyz = ((r0.xyzx)*(source[26].xyzx)+(r3.xyzx)).xyz;
    // 289: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 290: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 291: dp3 r0.x, r4.xyzx, r1.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 292: dp3 r0.z, r6.xyzx, r1.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 293: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 294: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 295: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 296: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 297: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 298: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 299: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 300: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 301: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 302: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 303: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 304: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 305: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 306: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 307: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 308: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 309: ret
    return output;
}

// source.character.monster-3b27d7fb1b53.v1 / source program bb722695f40b6b418ade278b9eb72ce1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase30(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[22].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[22].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[22].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[22].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 2: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 3: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 4: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 5: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 6: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 7: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 8: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 9: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 10: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 11: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 12: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 15: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 17: add r0.z, -r1.w, l(1.000000)
    r0.z = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 18: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 19: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 20: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 21: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 22: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 24: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 25: mul r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)*(source[18].yyyy)).w;
    // 26: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 27: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 28: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 29: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 31: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 32: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 33: mul r2.xy, r0.ywyy, cb0[17].xxxx
    r2.xy = ((r0.ywyy)*(source[17].xxxx)).xy;
    // 34: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 36: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 37: add r2.z, r0.y, l(0.000010)
    r2.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: add r3.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 39: mad r3.xyz, cb0[17].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[17].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 40: dp3 r0.y, r3.xyzx, r3.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 41: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 42: div r3.xyz, r3.xyzx, r0.yyyy
    r3.xyz = ((r3.xyzx)/(r0.yyyy)).xyz;
    // 43: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 44: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 45: mul r4.xyz, r0.yyyy, v0.xyzx
    r4.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 46: dp3 r5.x, r4.xyzx, r3.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 47: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 48: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 49: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 50: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 51: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 52: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 53: dp3 r5.y, r7.xyzx, r3.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 54: dp3 r5.z, r6.xyzx, r3.xyzx
    r5.z = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 55: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 56: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 57: mul r3.xyz, r0.yyyy, v5.xyzx
    r3.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 58: mad r8.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 59: dp3 r9.y, r7.xyzx, r3.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 60: dp3 r9.x, r4.xyzx, r3.xyzx
    r9.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 61: dp3 r9.z, r6.xyzx, r3.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 62: dp3 r0.y, r5.xyzx, r9.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 63: mul r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)*(r0.yyyy)).xyz;
    // 64: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 65: mov r5.w, -r5.x
    r5.w = (-(r5.xxxx)).w;
    // 66: dp2 r0.y, r5.ywyy, r5.ywyy
    r0.y = (dot((r5.ywyy).xy,(r5.ywyy).xy).xxxx).y;
    // 67: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 68: div r0.yw, r5.yyyw, r0.yyyy
    r0.yw = ((r5.yyyw)/(r0.yyyy)).yw;
    // 69: mad r1.w, -r5.z, l(0.250000), l(0.250000)
    r1.w = ((-(r5.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 70: add r2.w, r5.z, l(1.000000)
    r2.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 72: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 73: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t3.xywz, s3, r0.x
    r0.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 74: log r5.xyz, r0.xywx
    r5.xyz = (log2(r0.xywx)).xyz;
    // 75: rcp r1.w, cb0[18].z
    r1.w = (1.0/(source[18].zzzz)).w;
    // 76: mul r9.xyz, r5.xyzx, r1.wwww
    r9.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 77: mul r5.xyz, r5.xyzx, cb0[18].zzzz
    r5.xyz = ((r5.xyzx)*(source[18].zzzz)).xyz;
    // 78: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 79: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 80: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 81: mad r5.xyz, r5.xyzx, cb0[18].zzzz, r9.xyzx
    r5.xyz = ((r5.xyzx)*(source[18].zzzz)+(r9.xyzx)).xyz;
    // 82: add r0.xyw, r0.xyxw, r5.xyxz
    r0.xyw = ((r0.xyxw)+(r5.xyxz)).xyw;
    // 83: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 84: add r1.w, cb0[18].z, l(1.000000)
    r1.w = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 86: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 87: add r5.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r5.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 88: mad r5.xyz, r2.wwww, r5.xyzx, cb0[7].xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(source[7].xyzx)).xyz;
    // 89: mul r0.xyw, r0.xxxx, r5.xyxz
    r0.xyw = ((r0.xxxx)*(r5.xyxz)).xyw;
    // 90: mul r0.xyw, r0.xyxw, cb0[18].wwww
    r0.xyw = ((r0.xyxw)*(source[18].wwww)).xyw;
    // 91: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r5.xyz, -r1.xyzx, r1.wwww
    r5.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 93: mad r1.xyz, cb0[17].yyyy, r5.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 94: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: add r5.xyz, -r1.xyzx, r1.wwww
    r5.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 96: mad r1.xyz, cb0[17].zzzz, r5.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 97: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r5.xyz, -r1.xyzx, r1.wwww
    r5.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 99: mul r5.xyz, r5.xyzx, cb0[19].xxxx
    r5.xyz = ((r5.xyzx)*(source[19].xxxx)).xyz;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: add r1.w, r9.y, r9.x
    r1.w = ((r9.yyyy)+(r9.xxxx)).w;
    // 102: add r1.w, r9.z, r1.w
    r1.w = ((r9.zzzz)+(r1.wwww)).w;
    // 103: add_sat r1.w, r9.w, r1.w
    r1.w = (saturate((r9.wwww)+(r1.wwww))).w;
    // 104: mad r1.xyz, r1.wwww, r5.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 105: max r5.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 106: log r5.xyz, r5.xyzx
    r5.xyz = (log2(r5.xyzx)).xyz;
    // 107: mul r5.xyz, r5.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 108: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 109: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 111: mul r1.w, r1.w, cb0[20].x
    r1.w = ((r1.wwww)*(source[20].xxxx)).w;
    // 112: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 113: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 116: div r2.w, cb0[20].y, r2.w
    r2.w = ((source[20].yyyy)/(r2.wwww)).w;
    // 117: dp3 r3.w, r2.xyzx, r2.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 118: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 119: div r2.xyz, r2.xyzx, r3.wwww
    r2.xyz = ((r2.xyzx)/(r3.wwww)).xyz;
    // 120: dp3 r3.w, r2.xyzx, r3.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 121: mul_sat r4.w, r3.w, cb0[19].y
    r4.w = (saturate((r3.wwww)*(source[19].yyyy))).w;
    // 122: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mul_sat r5.x, r3.z, cb0[19].y
    r5.x = (saturate((r3.zzzz)*(source[19].yyyy))).x;
    // 125: add r5.x, -r5.x, l(1.000000)
    r5.x = ((-(r5.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 126: add_sat r5.x, r5.x, -cb0[19].z
    r5.x = (saturate((r5.xxxx)+(-(source[19].zzzz)))).x;
    // 127: log r5.y, r5.x
    r5.y = (log2(r5.xxxx)).y;
    // 128: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 129: mul r5.y, r5.y, cb0[19].w
    r5.y = ((r5.yyyy)*(source[19].wwww)).y;
    // 130: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 131: mul r4.w, r4.w, r5.y
    r4.w = ((r4.wwww)*(r5.yyyy)).w;
    // 132: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 133: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 134: mul r5.xyz, r0.xywx, r2.wwww
    r5.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 136: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: add r10.xyz, -r9.xyzx, r2.wwww
    r10.xyz = ((-(r9.xyzx))+(r2.wwww)).xyz;
    // 138: mad r9.xyz, cb0[17].yyyy, r10.xyzx, r9.xyzx
    r9.xyz = ((source[17].yyyy)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 139: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: add r10.xyz, -r9.xyzx, r2.wwww
    r10.xyz = ((-(r9.xyzx))+(r2.wwww)).xyz;
    // 141: mad r9.xyz, cb0[17].zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((source[17].zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 142: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 143: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 144: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 145: mad r10.xyz, cb0[17].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 146: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 147: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 148: mad r10.xyz, cb0[17].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 149: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 150: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 151: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 152: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 153: mul r12.xyz, r9.xyzx, r10.xyzx
    r12.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 154: mad r9.xyz, r10.xyzx, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = ((r10.xyzx)*(r9.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 155: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 156: mad r1.xyz, r1.xyzx, r5.xyzx, -r0.xywx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(-(r0.xywx))).xyz;
    // 157: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 159: mad r0.xyw, r2.wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 160: frc r1.x, cb0[3].x
    r1.x = (frac(source[3].xxxx)).x;
    // 161: add r1.y, -r1.x, l(1.000000)
    r1.y = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 162: mul r5.xyz, r0.xywx, r1.yyyy
    r5.xyz = ((r0.xywx)*(r1.yyyy)).xyz;
    // 163: dp3 r1.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 164: mad r10.xyz, -r1.yyyy, r0.xywx, r1.zzzz
    r10.xyz = ((-(r1.yyyy))*(r0.xywx)+(r1.zzzz)).xyz;
    // 165: mad r5.xyz, cb0[17].yyyy, r10.xyzx, r5.xyzx
    r5.xyz = ((source[17].yyyy)*(r10.xyzx)+(r5.xyzx)).xyz;
    // 166: dp3 r1.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 167: add r10.xyz, -r5.xyzx, r1.yyyy
    r10.xyz = ((-(r5.xyzx))+(r1.yyyy)).xyz;
    // 168: mad r5.xyz, cb0[17].zzzz, r10.xyzx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r10.xyzx)+(r5.xyzx)).xyz;
    // 169: dp3 r1.y, r9.xyzx, r9.xyzx
    r1.y = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 170: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 171: div r9.xyz, r9.xyzx, r1.yyyy
    r9.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 172: dp3 r1.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 173: add r10.xyz, -r9.xyzx, r1.yyyy
    r10.xyz = ((-(r9.xyzx))+(r1.yyyy)).xyz;
    // 174: add r9.xyz, r9.xyzx, -r10.xyzx
    r9.xyz = ((r9.xyzx)+(-(r10.xyzx))).xyz;
    // 175: mul r10.xyz, cb0[11].xyzx, cb0[21].xxxx
    r10.xyz = ((source[11].xyzx)*(source[21].xxxx)).xyz;
    // 176: mul r10.xyz, r10.xyzx, cb0[22].wwww
    r10.xyz = ((r10.xyzx)*(source[22].wwww)).xyz;
    // 177: mul r10.xyz, r4.wwww, r10.xyzx
    r10.xyz = ((r4.wwww)*(r10.xyzx)).xyz;
    // 178: mad r13.xyz, r4.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r13.xyz = ((r4.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 179: add r1.y, r4.w, l(-1.000000)
    r1.y = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 180: mad r1.y, cb0[9].w, r1.y, l(1.000000)
    r1.y = ((source[9].wwww)*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 181: mad r13.xyz, cb0[10].wwww, r13.xyzx, cb0[10].xyzx
    r13.xyz = ((source[10].wwww)*(r13.xyzx)+(source[10].xyzx)).xyz;
    // 182: mad r9.xyz, r9.xyzx, r10.xyzx, r13.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)+(r13.xyzx)).xyz;
    // 183: mad r9.xyz, r1.yyyy, cb0[9].xyzx, r9.xyzx
    r9.xyz = ((r1.yyyy)*(source[9].xyzx)+(r9.xyzx)).xyz;
    // 184: mad r5.xyz, r5.xyzx, r11.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 185: add r1.y, -|r3.z|, l(1.000000)
    r1.y = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: mul r1.y, r3.w, r1.y
    r1.y = ((r3.wwww)*(r1.yyyy)).y;
    // 187: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 188: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 189: mul r1.z, r1.z, l(1.500000)
    r1.z = ((r1.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 190: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 191: mul r9.xyz, r1.zzzz, cb0[12].xyzx
    r9.xyz = ((r1.zzzz)*(source[12].xyzx)).xyz;
    // 192: movc r9.xyz, r1.yyyy, l(0,0,0,0), r9.xyzx
    r9.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xyzx)).xyz;
    // 193: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 194: add r5.xyz, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r5.xyzx)+(source[2].xyzx)).xyz;
    // 195: dp3 r1.y, r8.xyzx, r8.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 196: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 197: div r8.xyz, r8.xyzx, r1.zzzz
    r8.xyz = ((r8.xyzx)/(r1.zzzz)).xyz;
    // 198: dp3 r1.z, r8.xyzx, r3.xyzx
    r1.z = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 199: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 200: mul r2.w, |r1.z|, |r1.z|
    r2.w = ((abs(r1.zzzz))*(abs(r1.zzzz))).w;
    // 201: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 202: mul r2.w, |r1.z|, r2.w
    r2.w = ((abs(r1.zzzz))*(r2.wwww)).w;
    // 203: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 204: movc r1.z, r1.z, l(0), r2.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 205: add r2.w, r1.z, l(-0.027778)
    r2.w = ((r1.zzzz)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 206: mad r1.z, r1.z, r2.w, l(0.027778)
    r1.z = ((r1.zzzz)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).z;
    // 207: div_sat r1.y, r1.z, r1.y
    r1.y = (saturate((r1.zzzz)/(r1.yyyy))).y;
    // 208: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 209: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 210: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 211: mad r0.xyz, r1.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 212: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 213: add r1.yzw, -r0.xxyz, r0.wwww
    r1.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 214: mad r0.xyz, cb0[17].yyyy, r1.yzwy, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r1.yzwy)+(r0.xyzx)).xyz;
    // 215: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 216: add r1.yzw, -r0.xxyz, r0.wwww
    r1.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 217: mad r0.xyz, cb0[17].zzzz, r1.yzwy, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r1.yzwy)+(r0.xyzx)).xyz;
    // 218: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 219: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 221: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 222: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 223: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 224: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 225: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 226: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 227: add r1.y, -r1.x, cb0[3].x
    r1.y = ((-(r1.xxxx))+(source[3].xxxx)).y;
    // 228: mul r3.z, r1.y, l(0.125000)
    r3.z = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 229: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 230: mul r8.x, r1.y, l(0.125000)
    r8.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 231: mul r3.y, cb0[3].y, cb0[13].y
    r3.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 232: mov r8.y, v4.y
    r8.y = (v4.yyyy).y;
    // 233: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 234: add r1.yz, r3.xxyx, r8.xxyx
    r1.yz = ((r3.xxyx)+(r8.xxyx)).yz;
    // 235: add r1.yz, r1.yyzy, r3.zzwz
    r1.yz = ((r1.yyzy)+(r3.zzwz)).yz;
    // 236: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.yzyy, t5.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 237: mul r1.yzw, r0.wwww, r3.xxyz
    r1.yzw = ((r0.wwww)*(r3.xxyz)).yzw;
    // 238: mul r0.w, r1.x, r3.w
    r0.w = ((r1.xxxx)*(r3.wwww)).w;
    // 239: mad r1.xyz, r1.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 240: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 241: add r1.xyzw, v7.yzxy, cb0[0].yzxy
    r1.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 242: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 243: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 244: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 245: mad r1.xy, cb0[14].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[14].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 246: mul r0.w, cb0[14].y, cb0[21].y
    r0.w = ((source[14].yyyy)*(source[21].yyyy)).w;
    // 247: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 248: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 249: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 250: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 252: mul r1.z, cb0[14].x, l(0.001000)
    r1.z = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 253: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 254: mad r1.xy, r1.zzzz, r1.xyxx, r3.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r3.xyxx)).xy;
    // 255: dp2 r1.z, cb0[15].xyxx, r1.xyxx
    r1.z = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 256: dp2 r1.y, cb0[16].xyxx, r1.xyxx
    r1.y = (dot((source[16].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 257: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 258: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 259: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 260: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 261: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 262: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 263: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 264: mad r3.xyz, cb0[14].zzzz, r1.xyzx, -r0.xyzx
    r3.xyz = ((source[14].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 265: mul r1.xyz, r1.xyzx, cb0[14].zzzz
    r1.xyz = ((r1.xyzx)*(source[14].zzzz)).xyz;
    // 266: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 267: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 268: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 269: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 270: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 271: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 272: mul r1.xyz, r0.wwww, r2.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 273: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 274: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 275: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 276: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 277: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 278: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 279: mul r2.yzw, r2.yyyy, cb0[24].xxyz
    r2.yzw = ((r2.yyyy)*(source[24].xxyz)).yzw;
    // 280: mad r2.xyz, r2.xxxx, cb0[23].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[23].xyzx)+(r2.yzwy)).xyz;
    // 281: mul r2.xyz, r2.xyzx, cb0[25].wwww
    r2.xyz = ((r2.xyzx)*(source[25].wwww)).xyz;
    // 282: mad r3.xyz, r2.xyzx, r0.xyzx, r5.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 283: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 284: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 285: mad o0.xyz, r0.xyzx, cb0[25].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)+(r3.xyzx)).xyz;
    // 286: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 287: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 288: dp3 r0.x, r4.xyzx, r1.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 289: dp3 r0.z, r6.xyzx, r1.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 290: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 291: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 292: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 293: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 294: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 295: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 296: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 297: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 298: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 299: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 300: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 301: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 302: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 303: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 304: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 305: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 306: ret
    return output;
}

// source.character.monster-d1e4d915a43a.v1 / source program c8c0b58f1e6f934b84c61dd182e19796
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase31(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].w=(g_SourceCharacterTime.xxxx).x;
    source[24].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[24].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[24].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[24].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.wxyz, s3, l(0.000000)
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 25: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: add r1.w, -cb0[19].y, cb0[19].x
    r1.w = ((-(source[19].yyyy))+(source[19].xxxx)).w;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 28: mad r1.w, r3.w, r1.w, cb0[19].y
    r1.w = ((r3.wwww)*(r1.wwww)+(source[19].yyyy)).w;
    // 29: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 30: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 31: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 33: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 34: add r1.w, -r1.z, cb0[20].y
    r1.w = ((-(r1.zzzz))+(source[20].yyyy)).w;
    // 35: mad r1.z, r3.w, r1.w, r1.z
    r1.z = ((r3.wwww)*(r1.wwww)+(r1.zzzz)).z;
    // 36: mul r1.z, r1.z, cb0[20].z
    r1.z = ((r1.zzzz)*(source[20].zzzz)).z;
    // 37: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 38: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 39: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 40: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 42: mad r4.xyzw, r1.xzxz, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r1.xzxz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 43: dp2 r1.x, r4.zwzz, r4.zwzz
    r1.x = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).x;
    // 44: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 46: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 47: add r5.z, r1.x, l(0.000010)
    r5.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: mul r5.xy, r4.xyxx, cb0[18].xxxx
    r5.xy = ((r4.xyxx)*(source[18].xxxx)).xy;
    // 49: mad r4.xy, cb0[18].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[18].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 50: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 51: mad r1.xzw, r3.wwww, r4.xxyz, r5.xxyz
    r1.xzw = ((r3.wwww)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 52: add r4.xyz, -r1.xzwx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xzwx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[20].xxxx, r4.xyzx, r1.xzwx
    r4.xyz = ((source[20].xxxx)*(r4.xyzx)+(r1.xzwx)).xyz;
    // 54: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 55: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 56: div r4.xyz, r4.xyzx, r2.wwww
    r4.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 57: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 58: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 59: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 60: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 61: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 62: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 63: mul r7.xyz, r2.wwww, v1.xyzx
    r7.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 64: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 65: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 66: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 67: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 68: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 69: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 70: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 71: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 72: mad r9.xyz, v5.xyzx, r2.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r2.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 74: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 75: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 76: dp3 r2.w, r6.xyzx, r10.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 77: mul r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 78: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 79: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 80: dp2 r2.w, r6.ywyy, r6.ywyy
    r2.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 81: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 82: div r6.xy, r6.ywyy, r2.wwww
    r6.xy = ((r6.ywyy)/(r2.wwww)).xy;
    // 83: mad r2.w, -r6.z, l(0.250000), l(0.250000)
    r2.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 84: add r4.w, r6.z, l(1.000000)
    r4.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 86: mad r6.xy, r2.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, r0.x
    r6.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 88: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 89: rcp r0.x, cb0[20].w
    r0.x = (1.0/(source[20].wwww)).x;
    // 90: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 91: mul r10.xyz, r10.xyzx, cb0[20].wwww
    r10.xyz = ((r10.xyzx)*(source[20].wwww)).xyz;
    // 92: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 93: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 94: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 95: mad r10.xyz, r10.xyzx, cb0[20].wwww, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[20].wwww)+(r11.xyzx)).xyz;
    // 96: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 97: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 98: add r0.x, cb0[20].w, l(1.000000)
    r0.x = ((source[20].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 100: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r6.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r6.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 102: mad r6.xyz, r4.wwww, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)+(source[8].xyzx)).xyz;
    // 103: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 104: mul r6.xyz, r6.xyzx, cb0[21].xxxx
    r6.xyz = ((r6.xyzx)*(source[21].xxxx)).xyz;
    // 105: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 106: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 107: mad r2.yzw, cb0[19].zzzz, r10.xxyz, r2.xxyz
    r2.yzw = ((source[19].zzzz)*(r10.xxyz)+(r2.xxyz)).yzw;
    // 108: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 110: mad r2.yzw, cb0[19].wwww, r10.xxyz, r2.yyzw
    r2.yzw = ((source[19].wwww)*(r10.xxyz)+(r2.yyzw)).yzw;
    // 111: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 113: mul r10.xyz, r10.xyzx, cb0[21].yyyy
    r10.xyz = ((r10.xyzx)*(source[21].yyyy)).xyz;
    // 114: add r0.x, r3.y, r3.x
    r0.x = ((r3.yyyy)+(r3.xxxx)).x;
    // 115: add r0.x, r3.z, r0.x
    r0.x = ((r3.zzzz)+(r0.xxxx)).x;
    // 116: add_sat r0.x, r3.w, r0.x
    r0.x = (saturate((r3.wwww)+(r0.xxxx))).x;
    // 117: mad r2.yzw, r0.xxxx, r10.xxyz, r2.yyzw
    r2.yzw = ((r0.xxxx)*(r10.xxyz)+(r2.yyzw)).yzw;
    // 118: add r3.xyz, -r2.yzwy, r2.xxxx
    r3.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 119: mad r2.xyz, r3.wwww, r3.xyzx, r2.yzwy
    r2.xyz = ((r3.wwww)*(r3.xyzx)+(r2.yzwy)).xyz;
    // 120: max r3.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 121: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 122: mul r3.xyz, r3.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 123: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 124: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 125: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 126: add r2.w, -cb0[22].z, cb0[22].y
    r2.w = ((-(source[22].zzzz))+(source[22].yyyy)).w;
    // 127: mad r2.w, r3.w, r2.w, cb0[22].z
    r2.w = ((r3.wwww)*(r2.wwww)+(source[22].zzzz)).w;
    // 128: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 129: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 130: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 131: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 133: div r2.w, cb0[22].w, r2.w
    r2.w = ((source[22].wwww)/(r2.wwww)).w;
    // 134: dp3 r3.x, r1.xzwx, r1.xzwx
    r3.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 135: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 136: div r1.xzw, r1.xxzw, r3.xxxx
    r1.xzw = ((r1.xxzw)/(r3.xxxx)).xzw;
    // 137: dp3 r3.x, r1.xzwx, r4.xyzx
    r3.x = (dot((r1.xzwx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 138: mul_sat r3.y, r3.x, cb0[21].z
    r3.y = (saturate((r3.xxxx)*(source[21].zzzz))).y;
    // 139: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 140: mul_sat r3.z, r4.z, cb0[21].z
    r3.z = (saturate((r4.zzzz)*(source[21].zzzz))).z;
    // 141: add r3.yz, -r3.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r3.yz = ((-(r3.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 142: add_sat r3.z, r3.z, -cb0[21].w
    r3.z = (saturate((r3.zzzz)+(-(source[21].wwww)))).z;
    // 143: log r4.w, r3.z
    r4.w = (log2(r3.zzzz)).w;
    // 144: lt r3.z, r3.z, l(0.000001)
    r3.z = (asfloat((uint4)((r3.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 145: mul r4.w, r4.w, cb0[22].x
    r4.w = ((r4.wwww)*(source[22].xxxx)).w;
    // 146: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 147: mul r3.y, r3.y, r4.w
    r3.y = ((r3.yyyy)*(r4.wwww)).y;
    // 148: movc r3.y, r3.z, l(0), r3.y
    r3.y = ((asuint(r3.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).y;
    // 149: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 150: mul r10.xyz, r6.xyzx, r2.wwww
    r10.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 151: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 152: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 153: mad r0.yzw, cb0[19].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 154: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 156: mad r0.yzw, cb0[19].wwww, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].wwww)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 157: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 158: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: mad r12.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r12.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 160: mad r11.xyz, cb0[19].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 161: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 163: mad r11.xyz, cb0[19].wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 164: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 166: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 167: mul r13.xyz, r11.xyzx, r12.xyzx
    r13.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 168: mad r11.xyz, -r11.xyzx, r12.xyzx, cb0[7].xyzx
    r11.xyz = ((-(r11.xyzx))*(r12.xyzx)+(source[7].xyzx)).xyz;
    // 169: mad r11.xyz, r3.wwww, r11.xyzx, r13.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r13.xyzx)).xyz;
    // 170: mul r13.xyz, r0.yzwy, r11.xyzx
    r13.xyz = ((r0.yzwy)*(r11.xyzx)).xyz;
    // 171: mad r0.yzw, r11.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r11.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 172: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 173: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 174: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mul r2.w, r2.w, cb0[23].x
    r2.w = ((r2.wwww)*(source[23].xxxx)).w;
    // 176: mad r2.xyz, r2.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 177: frc r2.w, cb0[3].x
    r2.w = (frac(source[3].xxxx)).w;
    // 178: add r3.z, -r2.w, l(1.000000)
    r3.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 179: mul r6.xyz, r2.xyzx, r3.zzzz
    r6.xyz = ((r2.xyzx)*(r3.zzzz)).xyz;
    // 180: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: mad r10.xyz, -r3.zzzz, r2.xyzx, r3.wwww
    r10.xyz = ((-(r3.zzzz))*(r2.xyzx)+(r3.wwww)).xyz;
    // 182: mad r6.xyz, cb0[19].zzzz, r10.xyzx, r6.xyzx
    r6.xyz = ((source[19].zzzz)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 183: dp3 r3.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 184: add r10.xyz, -r6.xyzx, r3.zzzz
    r10.xyz = ((-(r6.xyzx))+(r3.zzzz)).xyz;
    // 185: mad r6.xyz, cb0[19].wwww, r10.xyzx, r6.xyzx
    r6.xyz = ((source[19].wwww)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 186: dp3 r3.z, r0.yzwy, r0.yzwy
    r3.z = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).z;
    // 187: sqrt r3.z, r3.z
    r3.z = (sqrt(r3.zzzz)).z;
    // 188: div r0.yzw, r0.yyzw, r3.zzzz
    r0.yzw = ((r0.yyzw)/(r3.zzzz)).yzw;
    // 189: dp3 r3.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r10.xyz, -r0.yzwy, r3.zzzz
    r10.xyz = ((-(r0.yzwy))+(r3.zzzz)).xyz;
    // 191: add r0.yzw, r0.yyzw, -r10.xxyz
    r0.yzw = ((r0.yyzw)+(-(r10.xxyz))).yzw;
    // 192: mul r10.xyz, cb0[12].xyzx, cb0[23].zzzz
    r10.xyz = ((source[12].xyzx)*(source[23].zzzz)).xyz;
    // 193: mul r10.xyz, r10.xyzx, cb0[24].yyyy
    r10.xyz = ((r10.xyzx)*(source[24].yyyy)).xyz;
    // 194: mul r10.xyz, r3.yyyy, r10.xyzx
    r10.xyz = ((r3.yyyy)*(r10.xyzx)).xyz;
    // 195: mad r11.xyz, r3.yyyy, cb0[11].xyzx, -cb0[11].xyzx
    r11.xyz = ((r3.yyyy)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 196: add r3.y, r3.y, l(-1.000000)
    r3.y = ((r3.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 197: mad r3.y, cb0[10].w, r3.y, l(1.000000)
    r3.y = ((source[10].wwww)*(r3.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 198: mad r11.xyz, cb0[11].wwww, r11.xyzx, cb0[11].xyzx
    r11.xyz = ((source[11].wwww)*(r11.xyzx)+(source[11].xyzx)).xyz;
    // 199: mad r0.yzw, r0.yyzw, r10.xxyz, r11.xxyz
    r0.yzw = ((r0.yyzw)*(r10.xxyz)+(r11.xxyz)).yzw;
    // 200: mad r0.yzw, r3.yyyy, cb0[10].xxyz, r0.yyzw
    r0.yzw = ((r3.yyyy)*(source[10].xxyz)+(r0.yyzw)).yzw;
    // 201: mad r0.yzw, r6.xxyz, r12.xxyz, r0.yyzw
    r0.yzw = ((r6.xxyz)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 202: add r3.y, -|r4.z|, l(1.000000)
    r3.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 203: mul r3.x, r3.x, r3.y
    r3.x = ((r3.xxxx)*(r3.yyyy)).x;
    // 204: log r3.y, |r3.x|
    r3.y = (log2(abs(r3.xxxx))).y;
    // 205: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 206: mul r3.y, r3.y, l(1.500000)
    r3.y = ((r3.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 207: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 208: mul r3.yzw, r3.yyyy, cb0[13].xxyz
    r3.yzw = ((r3.yyyy)*(source[13].xxyz)).yzw;
    // 209: movc r3.xyz, r3.xxxx, l(0,0,0,0), r3.yzwy
    r3.xyz = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yzwy)).xyz;
    // 210: add r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)+(r3.xxyz)).yzw;
    // 211: add r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)+(source[2].xxyz)).yzw;
    // 212: dp3 r3.x, r9.xyzx, r9.xyzx
    r3.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 213: sqrt r3.y, r3.x
    r3.y = (sqrt(r3.xxxx)).y;
    // 214: div r3.yzw, r9.xxyz, r3.yyyy
    r3.yzw = ((r9.xxyz)/(r3.yyyy)).yzw;
    // 215: dp3 r3.y, r3.yzwy, r4.xyzx
    r3.y = (dot((r3.yzwy).xyz,(r4.xyzx).xyz).xxxx).y;
    // 216: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 217: mul r3.z, |r3.y|, |r3.y|
    r3.z = ((abs(r3.yyyy))*(abs(r3.yyyy))).z;
    // 218: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 219: mul r3.z, r3.z, |r3.y|
    r3.z = ((r3.zzzz)*(abs(r3.yyyy))).z;
    // 220: lt r3.y, |r3.y|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 221: movc r3.y, r3.y, l(0), r3.z
    r3.y = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).y;
    // 222: add r3.z, r3.y, l(-0.027778)
    r3.z = ((r3.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 223: mad r3.y, r3.y, r3.z, l(0.027778)
    r3.y = ((r3.yyyy)*(r3.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 224: div_sat r3.x, r3.y, r3.x
    r3.x = (saturate((r3.yyyy)/(r3.xxxx))).x;
    // 225: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 226: mul r1.y, r1.y, r3.x
    r1.y = ((r1.yyyy)*(r3.xxxx)).y;
    // 227: mad r2.xyz, r1.yyyy, r2.xyzx, -r13.xyzx
    r2.xyz = ((r1.yyyy)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 228: mad r2.xyz, r0.xxxx, r2.xyzx, r13.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)+(r13.xyzx)).xyz;
    // 229: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 230: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 231: mad r2.xyz, cb0[19].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 232: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 233: add r3.xyz, -r2.xyzx, r0.xxxx
    r3.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 234: mad r2.xyz, cb0[19].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[19].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 235: mul r2.xyz, r12.xyzx, r2.xyzx
    r2.xyz = ((r12.xyzx)*(r2.xyzx)).xyz;
    // 236: add r0.x, -cb0[3].w, l(1.000000)
    r0.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 237: mul r0.x, r0.x, cb0[23].w
    r0.x = ((r0.xxxx)*(source[23].wwww)).x;
    // 238: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 239: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 240: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 241: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 242: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 243: mad r0.x, r0.x, l(0.500000), cb0[3].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 244: add r1.y, -r2.w, cb0[3].x
    r1.y = ((-(r2.wwww))+(source[3].xxxx)).y;
    // 245: mul r3.z, r1.y, l(0.125000)
    r3.z = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 246: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 247: mul r4.x, r1.y, l(0.125000)
    r4.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 248: mul r3.y, cb0[3].y, cb0[14].y
    r3.y = ((source[3].yyyy)*(source[14].yyyy)).y;
    // 249: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 250: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 251: add r3.xy, r3.xyxx, r4.xyxx
    r3.xy = ((r3.xyxx)+(r4.xyxx)).xy;
    // 252: add r3.xy, r3.xyxx, r3.zwzz
    r3.xy = ((r3.xyxx)+(r3.zwzz)).xy;
    // 253: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 254: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 255: mul r0.x, r2.w, r3.w
    r0.x = ((r2.wwww)*(r3.wwww)).x;
    // 256: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 257: mad r2.xyz, r0.xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 258: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 259: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 260: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 261: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 262: mad r3.xy, cb0[15].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[15].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 263: mul r0.x, cb0[15].y, cb0[23].w
    r0.x = ((source[15].yyyy)*(source[23].wwww)).x;
    // 264: mul r0.x, r0.x, l(0.628319)
    r0.x = ((r0.xxxx)*(float4(0.628319,0.628319,0.628319,0.628319))).x;
    // 265: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 266: mul r4.y, r0.x, l(0.020000)
    r4.y = ((r0.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 267: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 268: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 269: mul r1.y, cb0[15].x, l(0.001000)
    r1.y = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 270: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 271: mad r3.xy, r1.yyyy, r3.xyxx, r4.xyxx
    r3.xy = ((r1.yyyy)*(r3.xyxx)+(r4.xyxx)).xy;
    // 272: dp2 r1.y, cb0[16].xyxx, r3.xyxx
    r1.y = (dot((source[16].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 273: dp2 r3.y, cb0[17].xyxx, r3.xyxx
    r3.y = (dot((source[17].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 274: frc r1.y, r1.y
    r1.y = (frac(r1.yyyy)).y;
    // 275: mul r3.x, r1.y, l(0.125000)
    r3.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 276: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 277: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r2.xyzx))).xyz;
    // 278: mul r1.y, r3.w, l(0.900000)
    r1.y = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 279: mad r3.xyz, r1.yyyy, r3.xyzx, r2.xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 280: mul_sat r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (saturate((r0.xxxx)*(r3.xyzx))).xyz;
    // 281: mad r4.xyz, cb0[15].zzzz, r3.xyzx, -r2.xyzx
    r4.xyz = ((source[15].zzzz)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 282: mul r3.xyz, r3.xyzx, cb0[15].zzzz
    r3.xyz = ((r3.xyzx)*(source[15].zzzz)).xyz;
    // 283: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 284: mul r0.x, r0.x, l(3.000000)
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 285: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 286: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 287: dp3 r0.x, r1.xzwx, r1.xzwx
    r0.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 288: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 289: mul r1.xyz, r0.xxxx, r1.xzwx
    r1.xyz = ((r0.xxxx)*(r1.xzwx)).xyz;
    // 290: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 291: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 292: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 293: dp3 r0.x, r3.xyzx, r1.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 294: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 296: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 297: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 298: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 299: mad r0.xyz, r3.xyzx, r2.xyzx, r0.yzwy
    r0.xyz = ((r3.xyzx)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 300: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 301: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 302: mad o0.xyz, r2.xyzx, cb0[27].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[27].xyzx)+(r0.xyzx)).xyz;
    // 303: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 304: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 305: dp3 r0.x, r5.xyzx, r1.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 306: dp3 r0.z, r7.xyzx, r1.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 307: dp3 r0.y, r8.xyzx, r1.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 308: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 309: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 310: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 311: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 312: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 313: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 314: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 315: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 316: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 317: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 318: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 319: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 320: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 321: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 322: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 323: ret
    return output;
}

// source.character.monster-411db5c836e7.v1 / source program 10215f76a54bc242a767938c5056e6fd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase32(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[22].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[22].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[22].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[22].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[23].xxxx
    r0.xy = ((v4.xyxx)*(source[23].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s6, l(0.000000)
    r0.x = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[23].y
    r0.x = ((r0.xxxx)+(-(source[23].yyyy))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 8: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 9: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 10: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 11: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 13: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 14: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 15: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 16: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 17: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 18: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 19: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 20: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 21: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 22: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 25: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 27: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 28: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 29: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 30: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 31: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 32: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 34: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 35: mul r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)*(source[18].yyyy)).w;
    // 36: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 37: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 38: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 39: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 41: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 42: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 43: mul r3.xy, r0.ywyy, cb0[17].xxxx
    r3.xy = ((r0.ywyy)*(source[17].xxxx)).xy;
    // 44: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 45: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 46: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 47: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 49: mad r4.xyz, cb0[17].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[17].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 50: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 51: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 52: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 53: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 54: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 55: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 56: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 57: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 60: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 61: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 62: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 63: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 64: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 65: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 68: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 69: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 70: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 71: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 72: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 73: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 74: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 75: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 76: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 77: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 78: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 79: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 80: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 82: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t4.xywz, s3, r0.x
    r0.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 84: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 85: rcp r1.w, cb0[18].z
    r1.w = (1.0/(source[18].zzzz)).w;
    // 86: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 87: mul r6.xyz, r6.xyzx, cb0[18].zzzz
    r6.xyz = ((r6.xyzx)*(source[18].zzzz)).xyz;
    // 88: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 89: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 90: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 91: mad r6.xyz, r6.xyzx, cb0[18].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[18].zzzz)+(r10.xyzx)).xyz;
    // 92: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 93: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 94: add r1.w, cb0[18].z, l(1.000000)
    r1.w = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 96: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 97: add r6.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r6.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 98: mad r6.xyz, r2.wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 99: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 100: mul r0.xyw, r0.xyxw, cb0[18].wwww
    r0.xyw = ((r0.xyxw)*(source[18].wwww)).xyw;
    // 101: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: add r6.xyz, -r2.xyzx, r1.wwww
    r6.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 103: mad r2.xyz, cb0[17].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[17].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 104: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r6.xyz, -r2.xyzx, r1.wwww
    r6.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 106: mad r2.xyz, cb0[17].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[17].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 107: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 108: add r6.xyz, -r2.xyzx, r1.wwww
    r6.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 109: mul r6.xyz, r6.xyzx, cb0[19].xxxx
    r6.xyz = ((r6.xyzx)*(source[19].xxxx)).xyz;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 111: add r1.w, r10.y, r10.x
    r1.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 112: add r1.w, r10.z, r1.w
    r1.w = ((r10.zzzz)+(r1.wwww)).w;
    // 113: add_sat r1.w, r10.w, r1.w
    r1.w = (saturate((r10.wwww)+(r1.wwww))).w;
    // 114: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 115: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 116: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 117: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 118: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 119: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 120: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 121: mul r1.w, r1.w, cb0[20].x
    r1.w = ((r1.wwww)*(source[20].xxxx)).w;
    // 122: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 123: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 126: div r2.w, cb0[20].y, r2.w
    r2.w = ((source[20].yyyy)/(r2.wwww)).w;
    // 127: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 128: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 129: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 130: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 131: mul_sat r4.w, r3.w, cb0[19].y
    r4.w = (saturate((r3.wwww)*(source[19].yyyy))).w;
    // 132: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: mul_sat r5.w, r4.z, cb0[19].y
    r5.w = (saturate((r4.zzzz)*(source[19].yyyy))).w;
    // 135: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: add_sat r5.w, r5.w, -cb0[19].z
    r5.w = (saturate((r5.wwww)+(-(source[19].zzzz)))).w;
    // 137: log r6.x, r5.w
    r6.x = (log2(r5.wwww)).x;
    // 138: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: mul r6.x, r6.x, cb0[19].w
    r6.x = ((r6.xxxx)*(source[19].wwww)).x;
    // 140: exp r6.x, r6.x
    r6.x = (exp2(r6.xxxx)).x;
    // 141: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 142: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 143: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 144: mul r6.xyz, r0.xywx, r2.wwww
    r6.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 145: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 146: add r10.xyz, -r1.xyzx, r2.wwww
    r10.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 147: mad r1.xyz, cb0[17].yyyy, r10.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 148: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: add r10.xyz, -r1.xyzx, r2.wwww
    r10.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 150: mad r1.xyz, cb0[17].zzzz, r10.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 151: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 152: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 154: mad r10.xyz, cb0[17].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 155: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 157: mad r10.xyz, cb0[17].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 158: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 161: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 162: mul r12.xyz, r1.xyzx, r10.xyzx
    r12.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 163: mad r1.xyz, r10.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 164: mul r0.xyw, r0.xyxw, r12.xyxz
    r0.xyw = ((r0.xyxw)*(r12.xyxz)).xyw;
    // 165: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 166: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 168: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 169: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 170: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: mul r6.xyz, r0.xywx, r2.yyyy
    r6.xyz = ((r0.xywx)*(r2.yyyy)).xyz;
    // 172: dp3 r2.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 173: mad r2.yzw, -r2.yyyy, r0.xxyw, r2.zzzz
    r2.yzw = ((-(r2.yyyy))*(r0.xxyw)+(r2.zzzz)).yzw;
    // 174: mad r2.yzw, cb0[17].yyyy, r2.yyzw, r6.xxyz
    r2.yzw = ((source[17].yyyy)*(r2.yyzw)+(r6.xxyz)).yzw;
    // 175: dp3 r5.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: add r6.xyz, -r2.yzwy, r5.wwww
    r6.xyz = ((-(r2.yzwy))+(r5.wwww)).xyz;
    // 177: mad r2.yzw, cb0[17].zzzz, r6.xxyz, r2.yyzw
    r2.yzw = ((source[17].zzzz)*(r6.xxyz)+(r2.yyzw)).yzw;
    // 178: dp3 r5.w, r1.xyzx, r1.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 179: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 180: div r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)/(r5.wwww)).xyz;
    // 181: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 182: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 183: add r1.xyz, r1.xyzx, -r6.xyzx
    r1.xyz = ((r1.xyzx)+(-(r6.xyzx))).xyz;
    // 184: mul r6.xyz, cb0[11].xyzx, cb0[21].xxxx
    r6.xyz = ((source[11].xyzx)*(source[21].xxxx)).xyz;
    // 185: mul r6.xyz, r6.xyzx, cb0[22].wwww
    r6.xyz = ((r6.xyzx)*(source[22].wwww)).xyz;
    // 186: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 187: mad r10.xyz, r4.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r10.xyz = ((r4.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 188: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 189: mad r4.w, cb0[9].w, r4.w, l(1.000000)
    r4.w = ((source[9].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: mad r10.xyz, cb0[10].wwww, r10.xyzx, cb0[10].xyzx
    r10.xyz = ((source[10].wwww)*(r10.xyzx)+(source[10].xyzx)).xyz;
    // 191: mad r1.xyz, r1.xyzx, r6.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(r10.xyzx)).xyz;
    // 192: mad r1.xyz, r4.wwww, cb0[9].xyzx, r1.xyzx
    r1.xyz = ((r4.wwww)*(source[9].xyzx)+(r1.xyzx)).xyz;
    // 193: mad r1.xyz, r2.yzwy, r11.xyzx, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 194: add r2.y, -|r4.z|, l(1.000000)
    r2.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 195: mul r2.y, r3.w, r2.y
    r2.y = ((r3.wwww)*(r2.yyyy)).y;
    // 196: log r2.z, |r2.y|
    r2.z = (log2(abs(r2.yyyy))).z;
    // 197: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 198: mul r2.z, r2.z, l(1.500000)
    r2.z = ((r2.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 199: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 200: mul r6.xyz, r2.zzzz, cb0[12].xyzx
    r6.xyz = ((r2.zzzz)*(source[12].xyzx)).xyz;
    // 201: movc r2.yzw, r2.yyyy, l(0,0,0,0), r6.xxyz
    r2.yzw = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxyz)).yzw;
    // 202: add r1.xyz, r1.xyzx, r2.yzwy
    r1.xyz = ((r1.xyzx)+(r2.yzwy)).xyz;
    // 203: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 204: dp3 r2.y, r9.xyzx, r9.xyzx
    r2.y = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 205: sqrt r2.z, r2.y
    r2.z = (sqrt(r2.yyyy)).z;
    // 206: div r6.xyz, r9.xyzx, r2.zzzz
    r6.xyz = ((r9.xyzx)/(r2.zzzz)).xyz;
    // 207: dp3 r2.z, r6.xyzx, r4.xyzx
    r2.z = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 208: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 209: mul r2.w, |r2.z|, |r2.z|
    r2.w = ((abs(r2.zzzz))*(abs(r2.zzzz))).w;
    // 210: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 211: mul r2.w, r2.w, |r2.z|
    r2.w = ((r2.wwww)*(abs(r2.zzzz))).w;
    // 212: lt r2.z, |r2.z|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 213: movc r2.z, r2.z, l(0), r2.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).z;
    // 214: add r2.w, r2.z, l(-0.027778)
    r2.w = ((r2.zzzz)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 215: mad r2.z, r2.z, r2.w, l(0.027778)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).z;
    // 216: div_sat r2.y, r2.z, r2.y
    r2.y = (saturate((r2.zzzz)/(r2.yyyy))).y;
    // 217: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 218: mul r0.z, r0.z, r2.y
    r0.z = ((r0.zzzz)*(r2.yyyy)).z;
    // 219: mad r0.xyz, r0.zzzz, r0.xywx, -r12.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(-(r12.xyzx))).xyz;
    // 220: mad r0.xyz, r1.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 221: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 222: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 223: mad r0.xyz, cb0[17].yyyy, r2.yzwy, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 224: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 225: add r2.yzw, -r0.xxyz, r0.wwww
    r2.yzw = ((-(r0.xxyz))+(r0.wwww)).yzw;
    // 226: mad r0.xyz, cb0[17].zzzz, r2.yzwy, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r2.yzwy)+(r0.xyzx)).xyz;
    // 227: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 228: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 229: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 230: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 231: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 232: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 233: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 234: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 235: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 236: add r1.w, -r2.x, cb0[3].x
    r1.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 237: mul r4.z, r1.w, l(0.125000)
    r4.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 239: mul r6.x, r1.w, l(0.125000)
    r6.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 240: mul r4.y, cb0[3].y, cb0[13].y
    r4.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 241: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 242: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 243: add r2.yz, r4.xxyx, r6.xxyx
    r2.yz = ((r4.xxyx)+(r6.xxyx)).yz;
    // 244: add r2.yz, r2.yyzy, r4.zzwz
    r2.yz = ((r2.yyzy)+(r4.zzwz)).yz;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.yzyy, t6.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 246: mul r2.yzw, r0.wwww, r4.xxyz
    r2.yzw = ((r0.wwww)*(r4.xxyz)).yzw;
    // 247: mul r0.w, r2.x, r4.w
    r0.w = ((r2.xxxx)*(r4.wwww)).w;
    // 248: mad r2.xyz, r2.yzwy, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.yzwy)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 249: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 250: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 251: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 252: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 253: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 254: mad r2.xy, cb0[14].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[14].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 255: mul r0.w, cb0[14].y, cb0[21].y
    r0.w = ((source[14].yyyy)*(source[21].yyyy)).w;
    // 256: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 257: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 258: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 259: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 260: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 261: mul r1.w, cb0[14].x, l(0.001000)
    r1.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 262: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 263: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 264: dp2 r1.w, cb0[15].xyxx, r2.xyxx
    r1.w = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 265: dp2 r2.y, cb0[16].xyxx, r2.xyxx
    r2.y = (dot((source[16].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 266: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 267: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 268: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 269: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 270: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 271: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 272: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 273: mad r4.xyz, cb0[14].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[14].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 274: mul r2.xyz, r2.xyzx, cb0[14].zzzz
    r2.xyz = ((r2.xyzx)*(source[14].zzzz)).xyz;
    // 275: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 276: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 277: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 278: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 279: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 280: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 281: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 282: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 283: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 284: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 285: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 286: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 287: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 288: mul r3.yzw, r3.yyyy, cb0[25].xxyz
    r3.yzw = ((r3.yyyy)*(source[25].xxyz)).yzw;
    // 289: mad r3.xyz, r3.xxxx, cb0[24].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[24].xyzx)+(r3.yzwy)).xyz;
    // 290: mul r3.xyz, r3.xyzx, cb0[26].wwww
    r3.xyz = ((r3.xyzx)*(source[26].wwww)).xyz;
    // 291: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 292: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 293: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 294: mad o0.xyz, r0.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 295: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 296: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 297: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 298: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 299: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 300: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 301: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 302: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 303: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 304: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 305: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 306: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 307: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 308: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 309: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 310: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 311: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 312: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 313: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 314: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 315: ret
    return output;
}



// source.character.monster-862fa1000fe2.v1 / source program 4aafe4637a7cdf40a21e97be6293ed98
