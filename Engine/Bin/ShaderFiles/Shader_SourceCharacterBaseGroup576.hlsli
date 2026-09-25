SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase600(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].w=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: add r0.x, v4.w, l(0.500000)
    r0.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 2: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 3: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 4: mad r0.z, cb0[18].y, l(-3.500000), l(5.000000)
    r0.z = ((source[18].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 5: mul r0.z, r0.z, cb0[19].x
    r0.z = ((r0.zzzz)*(source[19].xxxx)).z;
    // 6: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: add r1.x, -r0.w, v4.z
    r1.x = ((-(r0.wwww))+(v4.zzzz)).x;
    // 8: mad r0.w, cb0[19].y, r1.x, r0.w
    r0.w = ((source[19].yyyy)*(r1.xxxx)+(r0.wwww)).w;
    // 9: mul r1.x, r0.w, cb0[19].z
    r1.x = ((r0.wwww)*(source[19].zzzz)).x;
    // 10: mad r0.w, r1.x, l(0.750000), r0.w
    r0.w = ((r1.xxxx)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 11: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 12: mad_sat r0.w, cb0[20].x, r0.w, r0.w
    r0.w = (saturate((source[20].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 13: mul r1.x, cb0[19].w, l(0.700000)
    r1.x = ((source[19].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 15: add r1.y, -r2.y, l(1.000000)
    r1.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r4.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: mad r1.z, r4.x, r2.x, l(0.200000)
    r1.z = ((r4.xxxx)*(r2.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 19: add r1.y, -r1.z, r1.y
    r1.y = ((-(r1.zzzz))+(r1.yyyy)).y;
    // 20: mad r1.w, cb0[21].x, r1.y, r1.z
    r1.w = ((source[21].xxxx)*(r1.yyyy)+(r1.zzzz)).w;
    // 21: mad r1.y, cb0[20].z, r1.y, r1.z
    r1.y = ((source[20].zzzz)*(r1.yyyy)+(r1.zzzz)).y;
    // 22: add r1.y, -r0.w, r1.y
    r1.y = ((-(r0.wwww))+(r1.yyyy)).y;
    // 23: mad r1.y, r1.x, r1.y, r0.w
    r1.y = ((r1.xxxx)*(r1.yyyy)+(r0.wwww)).y;
    // 24: div r1.y, r1.y, cb0[20].y
    r1.y = ((r1.yyyy)/(source[20].yyyy)).y;
    // 25: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mul r1.y, r0.z, r1.y
    r1.y = ((r0.zzzz)*(r1.yyyy)).y;
    // 27: mul r1.y, r1.y, l(4.000000)
    r1.y = ((r1.yyyy)*(float4(4.000000,4.000000,4.000000,4.000000))).y;
    // 28: mul_sat r0.x, r0.x, r1.y
    r0.x = (saturate((r0.xxxx)*(r1.yyyy))).x;
    // 29: add r1.y, -r0.w, r1.w
    r1.y = ((-(r0.wwww))+(r1.wwww)).y;
    // 30: mad r0.w, r1.x, r1.y, r0.w
    r0.w = ((r1.xxxx)*(r1.yyyy)+(r0.wwww)).w;
    // 31: div r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)/(source[20].wwww)).w;
    // 32: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 34: mul r0.z, r0.z, l(4.000000)
    r0.z = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).z;
    // 35: mul_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)*(r0.zzzz))).y;
    // 36: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 37: add r0.x, -r2.y, r0.x
    r0.x = ((-(r2.yyyy))+(r0.xxxx)).x;
    // 38: mad r0.x, cb0[21].y, r0.x, r2.y
    r0.x = ((source[21].yyyy)*(r0.xxxx)+(r2.yyyy)).x;
    // 39: add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // 40: mul r0.yzw, r0.yyzw, cb0[18].xxxx
    r0.yzw = ((r0.yyzw)*(source[18].xxxx)).yzw;
    // 41: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // 42: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 43: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 44: mad r0.xyz, cb0[21].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[21].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 45: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 47: mad r0.xyz, cb0[21].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[21].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 48: mad r1.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: mad r3.xyw, cb0[6].wwww, cb0[6].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r3.xyw = ((source[6].wwww)*(source[6].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 50: mul r1.xyz, r1.xyzx, r3.xywx
    r1.xyz = ((r1.xyzx)*(r3.xywx)).xyz;
    // 51: mad r3.xyw, r0.xyxz, r1.xyxz, l(0.001000, 0.001000, 0.000000, 0.001000)
    r3.xyw = ((r0.xyxz)*(r1.xyxz)+(float4(0.001000,0.001000,0.000000,0.001000))).xyw;
    // 52: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 53: dp3 r0.w, r3.xywx, r3.xywx
    r0.w = (dot((r3.xywx).xyz,(r3.xywx).xyz).xxxx).w;
    // 54: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 55: div r1.xyz, r3.xywx, r0.wwww
    r1.xyz = ((r3.xywx)/(r0.wwww)).xyz;
    // 56: mul r1.xyz, r1.xyzx, r2.zzzz
    r1.xyz = ((r1.xyzx)*(r2.zzzz)).xyz;
    // 57: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 58: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 59: mul r3.xyw, r0.wwww, v6.xyxz
    r3.xyw = ((r0.wwww)*(v6.xyxz)).xyw;
    // 60: mov_sat r0.w, r3.w
    r0.w = (saturate(r3.wwww)).w;
    // 61: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 62: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 63: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 64: mul r1.xyz, r1.xyzx, cb0[22].xxxx
    r1.xyz = ((r1.xyzx)*(source[22].xxxx)).xyz;
    // 65: lt r0.w, |r3.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r3.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 66: mul r1.w, |r3.z|, |r3.z|
    r1.w = ((abs(r3.zzzz))*(abs(r3.zzzz))).w;
    // 67: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 68: mad r5.xyz, cb0[7].xyzx, r2.xyzx, -r2.xyzx
    r5.xyz = ((source[7].xyzx)*(r2.xyzx)+(-(r2.xyzx))).xyz;
    // 69: mad r5.xyz, r0.wwww, r5.xyzx, r2.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 70: mad r6.xyz, cb0[22].yyyy, r5.xyzx, -r1.xyzx
    r6.xyz = ((source[22].yyyy)*(r5.xyzx)+(-(r1.xyzx))).xyz;
    // 71: mad r1.xyz, r0.wwww, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 72: mad r5.xyz, -r2.xxxx, r0.xyzx, r5.xyzx
    r5.xyz = ((-(r2.xxxx))*(r0.xyzx)+(r5.xyzx)).xyz;
    // 73: mul r0.xyz, r0.xyzx, r2.xxxx
    r0.xyz = ((r0.xyzx)*(r2.xxxx)).xyz;
    // 74: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 75: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 76: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 78: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 79: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 80: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 81: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 82: div r2.xyz, r4.xyzx, r1.wwww
    r2.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 83: dp3 r1.w, r2.xyzx, r3.xywx
    r1.w = (dot((r2.xyzx).xyz,(r3.xywx).xyz).xxxx).w;
    // 84: add r3.x, -|r1.w|, l(1.000000)
    r3.x = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: add r3.y, -|r3.w|, l(1.000000)
    r3.y = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 86: mul r3.x, r3.x, r3.y
    r3.x = ((r3.xxxx)*(r3.yyyy)).x;
    // 87: mad r3.xyz, r3.xxxx, cb0[10].xyzx, -cb0[10].xyzx
    r3.xyz = ((r3.xxxx)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 88: mad r3.xyz, cb0[10].wwww, r3.xyzx, cb0[10].xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(source[10].xyzx)).xyz;
    // 89: add r4.x, -r1.w, l(1.000000)
    r4.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 90: mul_sat r1.w, r1.w, cb0[23].w
    r1.w = (saturate((r1.wwww)*(source[23].wwww))).w;
    // 91: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: mad r3.xyz, r4.xxxx, cb0[9].xyzx, r3.xyzx
    r3.xyz = ((r4.xxxx)*(source[9].xyzx)+(r3.xyzx)).xyz;
    // 93: mul_sat r4.x, r3.w, cb0[23].w
    r4.x = (saturate((r3.wwww)*(source[23].wwww))).x;
    // 94: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: add_sat r4.x, r4.x, -cb0[24].x
    r4.x = (saturate((r4.xxxx)+(-(source[24].xxxx)))).x;
    // 96: log r4.y, r4.x
    r4.y = (log2(r4.xxxx)).y;
    // 97: lt r4.x, r4.x, l(0.000001)
    r4.x = (asfloat((uint4)((r4.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 98: mul r4.y, r4.y, cb0[24].y
    r4.y = ((r4.yyyy)*(source[24].yyyy)).y;
    // 99: exp r4.y, r4.y
    r4.y = (exp2(r4.yyyy)).y;
    // 100: mul r1.w, r1.w, r4.y
    r1.w = ((r1.wwww)*(r4.yyyy)).w;
    // 101: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 102: mul r4.yzw, r1.wwww, cb0[11].xxyz
    r4.yzw = ((r1.wwww)*(source[11].xxyz)).yzw;
    // 103: movc r4.xyz, r4.xxxx, l(0,0,0,0), r4.yzwy
    r4.xyz = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.yzwy)).xyz;
    // 104: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 105: add r1.w, -cb0[23].x, l(0.200000)
    r1.w = ((-(source[23].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 106: mad r0.w, r0.w, r1.w, cb0[23].x
    r0.w = ((r0.wwww)*(r1.wwww)+(source[23].xxxx)).w;
    // 107: mad r0.w, r0.w, l(4.500000), l(0.500000)
    r0.w = ((r0.wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 108: mul r0.w, r0.w, cb0[23].y
    r0.w = ((r0.wwww)*(source[23].yyyy)).w;
    // 109: mul r0.w, r0.w, l(0.050000)
    r0.w = ((r0.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 110: add r4.xyz, v8.xyzx, cb0[0].yzwy
    r4.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 111: add r5.xyz, -r4.xyzx, cb0[0].yzwy
    r5.xyz = ((-(r4.xyzx))+(source[0].yzwy)).xyz;
    // 112: add r4.xyzw, r4.yzxy, -cb0[1].yzxy
    r4.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 113: mul r6.xyz, r3.wwww, r5.xyzx
    r6.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 114: mad r5.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r5.xyzx)).xyz;
    // 115: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 116: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 117: div r1.w, r5.z, r1.w
    r1.w = ((r5.zzzz)/(r1.wwww)).w;
    // 118: add r1.w, r1.w, cb0[8].z
    r1.w = ((r1.wwww)+(source[8].zzzz)).w;
    // 119: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 120: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 121: mul r5.xyz, r3.wwww, v1.xyzx
    r5.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 122: dp3 r3.w, r5.xyzx, r2.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 123: add r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)+(r3.wwww)).w;
    // 124: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 125: add r1.w, r1.w, l(-0.500000)
    r1.w = ((r1.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 126: add r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)+(r1.wwww)).w;
    // 127: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: log r3.w, r1.w
    r3.w = (log2(r1.wwww)).w;
    // 129: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 130: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 131: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 132: mul r0.w, r0.w, cb0[23].z
    r0.w = ((r0.wwww)*(source[23].zzzz)).w;
    // 133: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 134: mad r1.xyz, r0.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 135: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 136: add r3.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 137: add r3.xy, -r4.zwzz, r3.xyxx
    r3.xy = ((-(r4.zwzz))+(r3.xyxx)).xy;
    // 138: mad r3.xy, cb0[15].wwww, r3.xyxx, r4.zwzz
    r3.xy = ((source[15].wwww)*(r3.xyxx)+(r4.zwzz)).xy;
    // 139: mul r0.w, cb0[15].y, cb0[24].w
    r0.w = ((source[15].yyyy)*(source[24].wwww)).w;
    // 140: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 141: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 142: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 143: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 145: mul r1.w, cb0[15].x, l(0.001000)
    r1.w = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 147: mad r3.xy, r1.wwww, r3.xyxx, r4.xyxx
    r3.xy = ((r1.wwww)*(r3.xyxx)+(r4.xyxx)).xy;
    // 148: dp2 r4.y, cb0[17].xyxx, r3.xyxx
    r4.y = (dot((source[17].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 149: dp2 r1.w, cb0[16].xyxx, r3.xyxx
    r1.w = (dot((source[16].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 150: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 151: mul r4.x, r1.w, l(0.125000)
    r4.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 152: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 153: mul r1.w, r3.w, l(0.900000)
    r1.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 154: add r3.w, -cb0[13].w, l(1.000000)
    r3.w = ((-(source[13].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: mul r3.w, r3.w, cb0[24].w
    r3.w = ((r3.wwww)*(source[24].wwww)).w;
    // 156: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 157: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 158: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: mul r4.x, cb0[13].z, l(1.500000)
    r4.x = ((source[13].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 160: mul r3.w, r3.w, r4.x
    r3.w = ((r3.wwww)*(r4.xxxx)).w;
    // 161: mad r3.w, r3.w, l(0.500000), cb0[13].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[13].zzzz)).w;
    // 162: frc r4.x, v4.x
    r4.x = (frac(v4.xxxx)).x;
    // 163: mul r4.x, r4.x, l(0.125000)
    r4.x = ((r4.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 164: mul r6.y, cb0[13].y, cb0[14].y
    r6.y = ((source[13].yyyy)*(source[14].yyyy)).y;
    // 165: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 166: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 167: add r4.xy, r4.xyxx, r6.xyxx
    r4.xy = ((r4.xyxx)+(r6.xyxx)).xy;
    // 168: frc r4.z, cb0[13].x
    r4.z = (frac(source[13].xxxx)).z;
    // 169: add r4.w, -r4.z, cb0[13].x
    r4.w = ((-(r4.zzzz))+(source[13].xxxx)).w;
    // 170: mul r6.z, r4.w, l(0.125000)
    r6.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 171: add r4.xy, r4.xyxx, r6.zwzz
    r4.xy = ((r4.xyxx)+(r6.zwzz)).xy;
    // 172: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 173: mul r4.xyw, r3.wwww, r6.xyxz
    r4.xyw = ((r3.wwww)*(r6.xyxz)).xyw;
    // 174: mul r3.w, r4.z, r6.w
    r3.w = ((r4.zzzz)*(r6.wwww)).w;
    // 175: mad r4.xyz, r4.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 176: mad r0.xyz, r3.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 177: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 178: mad r3.xyz, r1.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 179: mul_sat r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx))).xyz;
    // 180: mad r4.xyz, cb0[15].zzzz, r3.xyzx, -r0.xyzx
    r4.xyz = ((source[15].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 181: mul r3.xyz, r3.xyzx, cb0[15].zzzz
    r3.xyz = ((r3.xyzx)*(source[15].zzzz)).xyz;
    // 182: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 183: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 184: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 185: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 186: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 187: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 188: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 189: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 190: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 191: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 192: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 193: mul r2.xyz, r2.xyzx, cb0[0].xxxx
    r2.xyz = ((r2.xyzx)*(source[0].xxxx)).xyz;
    // 194: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 195: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 196: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 197: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 198: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 199: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 200: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 201: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 202: mad r1.xyz, r0.xyzx, cb0[27].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[27].xyzx)+(r1.xyzx)).xyz;
    // 203: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 204: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 205: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 206: dp3 r0.x, r0.xyzx, cb0[12].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[12].xyzx).xyz).xxxx).x;
    // 207: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[12].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[12].xyzx).xyz).xxxx)).y;
    // 208: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 209: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 210: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 211: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 212: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 213: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 214: mul r0.xyz, r0.xxxx, v0.xyzx
    r0.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 215: mul r1.xyz, r0.yzxy, r5.zxyz
    r1.xyz = ((r0.yzxy)*(r5.zxyz)).xyz;
    // 216: mad r1.xyz, r5.yzxy, r0.zxyz, -r1.xyzx
    r1.xyz = ((r5.yzxy)*(r0.zxyz)+(-(r1.xyzx))).xyz;
    // 217: dp3 r3.z, r5.xyzx, r2.xyzx
    r3.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 218: dp3 r3.x, r0.xyzx, r2.xyzx
    r3.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 219: mul r0.xyz, r1.xyzx, v1.wwww
    r0.xyz = ((r1.xyzx)*(v1.wwww)).xyz;
    // 220: dp3 r3.y, r0.xyzx, r2.xyzx
    r3.y = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 221: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 222: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 223: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 224: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 225: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 226: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 227: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 228: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 229: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 230: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 231: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 232: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 233: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 234: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 235: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 236: ret
    return output;
}

// source.character.selection-native-601.v1 / source program 9859dedeace20041ba271c94538f478a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase601(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v7.z
    r0.x = ((r0.xxxx)*(v7.zzzz)).x;
    // 4: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: mul r0.yzw, r0.yyyy, cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)).yzw;
    // 7: mad r0.xyz, r0.xxxx, cb0[5].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[5].xyzx)+(r0.yzwy)).xyz;
    // 8: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 9: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 11: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 12: mul r0.w, r2.w, cb0[3].x
    r0.w = ((r2.wwww)*(source[3].xxxx)).w;
    // 13: mul o0.w, r0.w, cb0[0].y
    output.targets[0].w = ((r0.wwww)*(source[0].yyyy)).w;
    // 14: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 15: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 16: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 17: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 18: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 19: mad r0.xyz, r1.xyzx, cb0[7].xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)*(source[7].xyzx)+(r2.xyzx)).xyz;
    // 20: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 22: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 23: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 24: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 25: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 26: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 27: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 28: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 29: mad r0.x, r0.x, r1.y, -r0.y
    r0.x = ((r0.xxxx)*(r1.yyyy)+(-(r0.yyyy))).x;
    // 30: mul r0.x, r0.x, v1.w
    r0.x = ((r0.xxxx)*(v1.wwww)).x;
    // 31: movc r0.y, v9.x, l(1.000000), l(-1.000000)
    r0.y = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 32: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 33: mul r2.x, r0.y, r1.z
    r2.x = ((r0.yyyy)*(r1.zzzz)).x;
    // 34: mul r2.yz, r0.yyyy, r0.xxzx
    r2.yz = ((r0.yyyy)*(r0.xxzx)).yz;
    // 35: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 36: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 37: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 38: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 39: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 40: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 41: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 42: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 43: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 44: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 45: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 46: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 47: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 48: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 49: ftou r0.x, cb0[4].z
    r0.x = (asfloat((uint4)(source[4].zzzz))).x;
    // 50: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 51: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 52: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 53: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 54: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 55: ret
    return output;
}

// source.character.selection-native-700.v1 / source program d33da204b9a7a84189d02d8ac3544fd8
