SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase700(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].z=(g_SourceCharacterTime.xxxx).x;
    source[23].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: add r0.x, v4.w, l(0.500000)
    r0.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 2: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 3: mad r0.z, cb0[17].y, l(-3.500000), l(5.000000)
    r0.z = ((source[17].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 4: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 5: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 6: add r1.x, -r0.w, v4.z
    r1.x = ((-(r0.wwww))+(v4.zzzz)).x;
    // 7: mad r0.w, cb0[18].y, r1.x, r0.w
    r0.w = ((source[18].yyyy)*(r1.xxxx)+(r0.wwww)).w;
    // 8: mul r1.x, r0.w, cb0[18].z
    r1.x = ((r0.wwww)*(source[18].zzzz)).x;
    // 9: mad r0.w, r1.x, l(0.750000), r0.w
    r0.w = ((r1.xxxx)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 10: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 11: mad_sat r0.w, cb0[19].x, r0.w, r0.w
    r0.w = (saturate((source[19].xxxx)*(r0.wwww)+(r0.wwww))).w;
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
    // 18: mad r2.z, cb0[20].x, r2.x, r2.y
    r2.z = ((source[20].xxxx)*(r2.xxxx)+(r2.yyyy)).z;
    // 19: mad r2.x, cb0[19].z, r2.x, r2.y
    r2.x = ((source[19].zzzz)*(r2.xxxx)+(r2.yyyy)).x;
    // 20: add r2.xy, -r0.wwww, r2.xzxx
    r2.xy = ((-(r0.wwww))+(r2.xzxx)).xy;
    // 21: mul r2.z, cb0[18].w, l(0.700000)
    r2.z = ((source[18].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).z;
    // 22: mad r2.y, r2.z, r2.y, r0.w
    r2.y = ((r2.zzzz)*(r2.yyyy)+(r0.wwww)).y;
    // 23: mad r0.w, r2.z, r2.x, r0.w
    r0.w = ((r2.zzzz)*(r2.xxxx)+(r0.wwww)).w;
    // 24: div r0.w, r0.w, cb0[19].y
    r0.w = ((r0.wwww)/(source[19].yyyy)).w;
    // 25: add r0.yw, -r0.xxxw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = ((-(r0.xxxw))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 26: mul r0.w, r0.w, r0.z
    r0.w = ((r0.wwww)*(r0.zzzz)).w;
    // 27: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 28: mul_sat r0.x, r0.x, r0.w
    r0.x = (saturate((r0.xxxx)*(r0.wwww))).x;
    // 29: div r0.w, r2.y, cb0[19].w
    r0.w = ((r2.yyyy)/(source[19].wwww)).w;
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
    // 36: mad r0.x, cb0[20].y, r0.x, r1.y
    r0.x = ((source[20].yyyy)*(r0.xxxx)+(r1.yyyy)).x;
    // 37: add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // 38: mul r0.yzw, r0.yyzw, cb0[17].xxxx
    r0.yzw = ((r0.yyzw)*(source[17].xxxx)).yzw;
    // 39: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // 40: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 42: mad r0.xyz, cb0[20].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 43: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 45: mad r0.xyz, cb0[20].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[20].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
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
    // 63: mul r1.xyz, r1.xyzx, cb0[21].xxxx
    r1.xyz = ((r1.xyzx)*(source[21].xxxx)).xyz;
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
    // 73: add r2.x, -|r0.w|, l(1.000000)
    r2.x = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 74: add r2.y, -|r2.z|, l(1.000000)
    r2.y = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 76: mad r2.xyw, r2.xxxx, cb0[9].xyxz, -cb0[9].xyxz
    r2.xyw = ((r2.xxxx)*(source[9].xyxz)+(-(source[9].xyxz))).xyw;
    // 77: mad r2.xyw, cb0[9].wwww, r2.xyxw, cb0[9].xyxz
    r2.xyw = ((source[9].wwww)*(r2.xyxw)+(source[9].xyxz)).xyw;
    // 78: add r3.w, -r0.w, l(1.000000)
    r3.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: mul_sat r0.w, r0.w, cb0[22].z
    r0.w = (saturate((r0.wwww)*(source[22].zzzz))).w;
    // 80: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mad r2.xyw, r3.wwww, cb0[8].xyxz, r2.xyxw
    r2.xyw = ((r3.wwww)*(source[8].xyxz)+(r2.xyxw)).xyw;
    // 82: mul_sat r3.w, r2.z, cb0[22].z
    r3.w = (saturate((r2.zzzz)*(source[22].zzzz))).w;
    // 83: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: add_sat r3.w, r3.w, -cb0[22].w
    r3.w = (saturate((r3.wwww)+(-(source[22].wwww)))).w;
    // 85: log r4.x, r3.w
    r4.x = (log2(r3.wwww)).x;
    // 86: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 87: mul r4.x, r4.x, cb0[23].x
    r4.x = ((r4.xxxx)*(source[23].xxxx)).x;
    // 88: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 89: mul r0.w, r0.w, r4.x
    r0.w = ((r0.wwww)*(r4.xxxx)).w;
    // 90: mul r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)*(source[10].wwww)).w;
    // 91: mul r4.xyz, r0.wwww, cb0[10].xyzx
    r4.xyz = ((r0.wwww)*(source[10].xyzx)).xyz;
    // 92: movc r4.xyz, r3.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 93: add r2.xyw, r2.xyxw, r4.xyxz
    r2.xyw = ((r2.xyxw)+(r4.xyxz)).xyw;
    // 94: add r4.xyz, v8.xyzx, cb0[0].yzwy
    r4.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 95: add r5.xyz, -r4.xyzx, cb0[0].yzwy
    r5.xyz = ((-(r4.xyzx))+(source[0].yzwy)).xyz;
    // 96: add r4.xyzw, r4.yzxy, -cb0[1].yzxy
    r4.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 97: mul r6.xyz, r2.zzzz, r5.xyzx
    r6.xyz = ((r2.zzzz)*(r5.xyzx)).xyz;
    // 98: mad r5.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r5.xyzx)).xyz;
    // 99: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 100: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 101: div r0.w, r5.z, r0.w
    r0.w = ((r5.zzzz)/(r0.wwww)).w;
    // 102: add r0.w, r0.w, cb0[7].z
    r0.w = ((r0.wwww)+(source[7].zzzz)).w;
    // 103: dp3 r2.z, v1.xyzx, v1.xyzx
    r2.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 104: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 105: mul r5.xyz, r2.zzzz, v1.xyzx
    r5.xyz = ((r2.zzzz)*(v1.xyzx)).xyz;
    // 106: dp3 r2.z, r5.xyzx, r3.xyzx
    r2.z = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 107: add r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)+(r2.zzzz)).w;
    // 108: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 109: add r0.w, r0.w, l(-0.500000)
    r0.w = ((r0.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 110: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 111: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: log r2.z, r0.w
    r2.z = (log2(r0.wwww)).z;
    // 113: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 114: mad r3.w, cb0[21].w, l(4.500000), l(0.500000)
    r3.w = ((source[21].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 115: mul r3.w, r3.w, cb0[22].x
    r3.w = ((r3.wwww)*(source[22].xxxx)).w;
    // 116: mul r3.w, r3.w, l(0.050000)
    r3.w = ((r3.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 117: mul r2.z, r2.z, r3.w
    r2.z = ((r2.zzzz)*(r3.wwww)).z;
    // 118: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 119: mul r2.z, r2.z, cb0[22].y
    r2.z = ((r2.zzzz)*(source[22].yyyy)).z;
    // 120: movc r0.w, r0.w, l(0), r2.z
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 121: mad r1.xyz, r0.wwww, r1.xyzx, r2.xywx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xywx)).xyz;
    // 122: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 123: add r2.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 124: add r2.xy, -r4.zwzz, r2.xyxx
    r2.xy = ((-(r4.zwzz))+(r2.xyxx)).xy;
    // 125: mad r2.xy, cb0[14].wwww, r2.xyxx, r4.zwzz
    r2.xy = ((source[14].wwww)*(r2.xyxx)+(r4.zwzz)).xy;
    // 126: mul r0.w, cb0[14].y, cb0[23].z
    r0.w = ((source[14].yyyy)*(source[23].zzzz)).w;
    // 127: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 128: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 129: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 130: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 132: mul r2.z, cb0[14].x, l(0.001000)
    r2.z = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 133: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 134: mad r2.xy, r2.zzzz, r2.xyxx, r4.xyxx
    r2.xy = ((r2.zzzz)*(r2.xyxx)+(r4.xyxx)).xy;
    // 135: dp2 r4.y, cb0[16].xyxx, r2.xyxx
    r4.y = (dot((source[16].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 136: dp2 r2.x, cb0[15].xyxx, r2.xyxx
    r2.x = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 137: frc r2.x, r2.x
    r2.x = (frac(r2.xxxx)).x;
    // 138: mul r4.x, r2.x, l(0.125000)
    r4.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 139: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 140: mul r2.w, r2.w, l(0.900000)
    r2.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 141: add r3.w, -cb0[12].w, l(1.000000)
    r3.w = ((-(source[12].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mul r3.w, r3.w, cb0[23].z
    r3.w = ((r3.wwww)*(source[23].zzzz)).w;
    // 143: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 144: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 145: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: mul r4.x, cb0[12].z, l(1.500000)
    r4.x = ((source[12].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 147: mul r3.w, r3.w, r4.x
    r3.w = ((r3.wwww)*(r4.xxxx)).w;
    // 148: mad r3.w, r3.w, l(0.500000), cb0[12].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[12].zzzz)).w;
    // 149: frc r4.x, v4.x
    r4.x = (frac(v4.xxxx)).x;
    // 150: mul r4.x, r4.x, l(0.125000)
    r4.x = ((r4.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 151: mul r6.y, cb0[12].y, cb0[13].y
    r6.y = ((source[12].yyyy)*(source[13].yyyy)).y;
    // 152: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 153: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 154: add r4.xy, r4.xyxx, r6.xyxx
    r4.xy = ((r4.xyxx)+(r6.xyxx)).xy;
    // 155: frc r4.z, cb0[12].x
    r4.z = (frac(source[12].xxxx)).z;
    // 156: add r4.w, -r4.z, cb0[12].x
    r4.w = ((-(r4.zzzz))+(source[12].xxxx)).w;
    // 157: mul r6.z, r4.w, l(0.125000)
    r6.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 158: add r4.xy, r4.xyxx, r6.zwzz
    r4.xy = ((r4.xyxx)+(r6.zwzz)).xy;
    // 159: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 160: mul r4.xyw, r3.wwww, r6.xyxz
    r4.xyw = ((r3.wwww)*(r6.xyxz)).xyw;
    // 161: mul r3.w, r4.z, r6.w
    r3.w = ((r4.zzzz)*(r6.wwww)).w;
    // 162: mad r4.xyz, r4.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 163: mad r0.xyz, r3.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 164: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 165: mad r2.xyz, r2.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 166: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 167: mad r4.xyz, cb0[14].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[14].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 168: mul r2.xyz, r2.xyzx, cb0[14].zzzz
    r2.xyz = ((r2.xyzx)*(source[14].zzzz)).xyz;
    // 169: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 171: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 172: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 173: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 174: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 175: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 176: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 177: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 178: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 179: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 180: mul r2.xyz, r2.xyzx, cb0[0].xxxx
    r2.xyz = ((r2.xyzx)*(source[0].xxxx)).xyz;
    // 181: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 182: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 183: mul r3.yzw, r3.yyyy, cb0[25].xxyz
    r3.yzw = ((r3.yyyy)*(source[25].xxyz)).yzw;
    // 184: mad r3.xyz, r3.xxxx, cb0[24].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[24].xyzx)+(r3.yzwy)).xyz;
    // 185: mul r3.xyz, r3.xyzx, cb0[26].wwww
    r3.xyz = ((r3.xyzx)*(source[26].wwww)).xyz;
    // 186: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 187: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 188: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 189: mad r1.xyz, r0.xyzx, cb0[26].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 190: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 191: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 192: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 193: dp3 r0.x, r0.xyzx, cb0[11].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[11].xyzx).xyz).xxxx).x;
    // 194: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[11].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[11].xyzx).xyz).xxxx)).y;
    // 195: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 196: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 197: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 198: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 199: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 200: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 201: mul r0.xyz, r0.xxxx, v0.xyzx
    r0.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 202: mul r1.xyz, r0.yzxy, r5.zxyz
    r1.xyz = ((r0.yzxy)*(r5.zxyz)).xyz;
    // 203: mad r1.xyz, r5.yzxy, r0.zxyz, -r1.xyzx
    r1.xyz = ((r5.yzxy)*(r0.zxyz)+(-(r1.xyzx))).xyz;
    // 204: dp3 r3.z, r5.xyzx, r2.xyzx
    r3.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 205: dp3 r3.x, r0.xyzx, r2.xyzx
    r3.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 206: mul r0.xyz, r1.xyzx, v1.wwww
    r0.xyz = ((r1.xyzx)*(v1.wwww)).xyz;
    // 207: dp3 r3.y, r0.xyzx, r2.xyzx
    r3.y = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 208: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 209: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 210: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 211: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 212: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 213: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 214: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 215: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 216: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 217: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 218: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 219: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 220: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 221: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 222: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 223: ret
    return output;
}

// source.character.selection-native-701.v1 / source program dea8fd54f818c441b66600ac13b9ee61
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase701(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
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

// source.character.selection-native-702.v1 / source program 3b3abe5b3d623749aeec90310df73939
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase702(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 2: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 3: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 4: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 5: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 6: mul r0.x, r0.x, v7.z
    r0.x = ((r0.xxxx)*(v7.zzzz)).x;
    // 7: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 8: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 9: mul r0.yzw, r0.yyyy, cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)).yzw;
    // 10: mad r0.xyz, r0.xxxx, cb0[5].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[5].xyzx)+(r0.yzwy)).xyz;
    // 11: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 12: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 13: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 14: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 15: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 17: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 18: mad r0.xyz, r1.xyzx, cb0[7].xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)*(source[7].xyzx)+(r2.xyzx)).xyz;
    // 19: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 20: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 21: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 22: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 23: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 24: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 27: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 28: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 29: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 30: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 31: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 32: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 33: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 34: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 35: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 36: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 37: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 38: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 39: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 40: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 41: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 43: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 44: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 45: ftou r0.x, cb0[4].z
    r0.x = (asfloat((uint4)(source[4].zzzz))).x;
    // 46: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 47: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 48: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 49: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 50: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 51: ret
    return output;
}

// source.character.selection-native-703.v1 / source program 9ec6920c42dcfd49b5987ad76c6c908a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase703(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 20: mul r0.w, r0.w, cb0[19].x
    r0.w = ((r0.wwww)*(source[19].xxxx)).w;
    // 21: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 22: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 24: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 25: mul r0.w, r0.w, cb0[19].y
    r0.w = ((r0.wwww)*(source[19].yyyy)).w;
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
    // 33: mul r2.xy, r0.ywyy, cb0[18].xxxx
    r2.xy = ((r0.ywyy)*(source[18].xxxx)).xy;
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
    // 39: mad r3.xyz, cb0[18].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[18].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 75: rcp r1.w, cb0[19].z
    r1.w = (1.0/(source[19].zzzz)).w;
    // 76: mul r9.xyz, r5.xyzx, r1.wwww
    r9.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 77: mul r5.xyz, r5.xyzx, cb0[19].zzzz
    r5.xyz = ((r5.xyzx)*(source[19].zzzz)).xyz;
    // 78: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 79: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 80: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 81: mad r5.xyz, r5.xyzx, cb0[19].zzzz, r9.xyzx
    r5.xyz = ((r5.xyzx)*(source[19].zzzz)+(r9.xyzx)).xyz;
    // 82: add r0.xyw, r0.xyxw, r5.xyxz
    r0.xyw = ((r0.xyxw)+(r5.xyxz)).xyw;
    // 83: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 84: add r1.w, cb0[19].z, l(1.000000)
    r1.w = ((source[19].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 86: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 87: add r5.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r5.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 88: mad r5.xyz, r2.wwww, r5.xyzx, cb0[8].xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(source[8].xyzx)).xyz;
    // 89: mul r0.xyw, r0.xxxx, r5.xyxz
    r0.xyw = ((r0.xxxx)*(r5.xyxz)).xyw;
    // 90: mul r0.xyw, r0.xyxw, cb0[19].wwww
    r0.xyw = ((r0.xyxw)*(source[19].wwww)).xyw;
    // 91: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 92: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 93: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 94: dp3 r1.w, r2.xyzx, r3.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 95: mul_sat r2.w, r1.w, cb0[20].y
    r2.w = (saturate((r1.wwww)*(source[20].yyyy))).w;
    // 96: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul_sat r3.w, r3.z, cb0[20].y
    r3.w = (saturate((r3.zzzz)*(source[20].yyyy))).w;
    // 99: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: add_sat r3.w, r3.w, -cb0[20].z
    r3.w = (saturate((r3.wwww)+(-(source[20].zzzz)))).w;
    // 101: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 102: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 103: mul r4.w, r4.w, cb0[20].w
    r4.w = ((r4.wwww)*(source[20].wwww)).w;
    // 104: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 105: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 106: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 107: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 108: add r5.xyz, -r1.xyzx, r3.wwww
    r5.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 109: mad r1.xyz, cb0[18].yyyy, r5.xyzx, r1.xyzx
    r1.xyz = ((source[18].yyyy)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 110: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 111: add r5.xyz, -r1.xyzx, r3.wwww
    r5.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 112: mad r1.xyz, cb0[18].zzzz, r5.xyzx, r1.xyzx
    r1.xyz = ((source[18].zzzz)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 113: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r5.xyz, -r1.xyzx, r3.wwww
    r5.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 115: mul r5.xyz, r5.xyzx, cb0[20].xxxx
    r5.xyz = ((r5.xyzx)*(source[20].xxxx)).xyz;
    // 116: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 117: add r3.w, r9.y, r9.x
    r3.w = ((r9.yyyy)+(r9.xxxx)).w;
    // 118: add r3.w, r9.z, r3.w
    r3.w = ((r9.zzzz)+(r3.wwww)).w;
    // 119: add_sat r3.w, r9.w, r3.w
    r3.w = (saturate((r9.wwww)+(r3.wwww))).w;
    // 120: mad r1.xyz, r3.wwww, r5.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 121: max r5.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 122: log r5.xyz, r5.xyzx
    r5.xyz = (log2(r5.xyzx)).xyz;
    // 123: mul r5.xyz, r5.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 124: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 125: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 127: mul r3.w, r3.w, cb0[21].x
    r3.w = ((r3.wwww)*(source[21].xxxx)).w;
    // 128: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 129: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 132: div r4.w, cb0[21].y, r4.w
    r4.w = ((source[21].yyyy)/(r4.wwww)).w;
    // 133: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 134: mul r5.xyz, r0.xywx, r4.wwww
    r5.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r9.xyw, v4.xyxx, t1.xywz, s1, l(0.000000)
    r9.xyw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // 136: dp3 r4.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: add r10.xyz, -r9.xywx, r4.wwww
    r10.xyz = ((-(r9.xywx))+(r4.wwww)).xyz;
    // 138: mad r9.xyw, cb0[18].yyyy, r10.xyxz, r9.xyxw
    r9.xyw = ((source[18].yyyy)*(r10.xyxz)+(r9.xyxw)).xyw;
    // 139: dp3 r4.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: add r10.xyz, -r9.xywx, r4.wwww
    r10.xyz = ((-(r9.xywx))+(r4.wwww)).xyz;
    // 141: mad r9.xyw, cb0[18].zzzz, r10.xyxz, r9.xyxw
    r9.xyw = ((source[18].zzzz)*(r10.xyxz)+(r9.xyxw)).xyw;
    // 142: mul r10.xyz, cb0[5].xyzx, cb0[5].wwww
    r10.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 143: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 144: mad r11.xyz, -cb0[5].wwww, cb0[5].xyzx, r4.wwww
    r11.xyz = ((-(source[5].wwww))*(source[5].xyzx)+(r4.wwww)).xyz;
    // 145: mad r10.xyz, cb0[18].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[18].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 146: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 147: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 148: mad r10.xyz, cb0[18].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[18].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 149: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 150: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 151: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 152: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 153: mul r9.xyw, r9.xyxw, r10.xyxz
    r9.xyw = ((r9.xyxw)*(r10.xyxz)).xyw;
    // 154: mul r0.xyw, r0.xyxw, r9.xyxw
    r0.xyw = ((r0.xyxw)*(r9.xyxw)).xyw;
    // 155: mad r1.xyz, r1.xyzx, r5.xyzx, -r0.xywx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(-(r0.xywx))).xyz;
    // 156: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: mul r4.w, r4.w, cb0[21].z
    r4.w = ((r4.wwww)*(source[21].zzzz)).w;
    // 158: mad r0.xyw, r4.wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 159: dp3 r1.x, r8.xyzx, r8.xyzx
    r1.x = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 160: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 161: div r5.xyz, r8.xyzx, r1.yyyy
    r5.xyz = ((r8.xyzx)/(r1.yyyy)).xyz;
    // 162: dp3 r1.y, r5.xyzx, r3.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 163: add r1.z, -|r3.z|, l(1.000000)
    r1.z = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 164: mul r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)*(r1.zzzz)).z;
    // 165: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 166: mul r1.w, |r1.y|, |r1.y|
    r1.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 167: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 168: mul r1.w, r1.w, |r1.y|
    r1.w = ((r1.wwww)*(abs(r1.yyyy))).w;
    // 169: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 170: movc r1.y, r1.y, l(0), r1.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 171: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 172: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 173: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 174: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 175: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 176: mad r1.xyw, r0.zzzz, r0.xyxw, -r9.xyxw
    r1.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r9.xyxw))).xyw;
    // 177: mad r1.xyw, r3.wwww, r1.xyxw, r9.xyxw
    r1.xyw = ((r3.wwww)*(r1.xyxw)+(r9.xyxw)).xyw;
    // 178: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r3.xyz, -r1.xywx, r0.zzzz
    r3.xyz = ((-(r1.xywx))+(r0.zzzz)).xyz;
    // 180: mad r1.xyw, cb0[18].yyyy, r3.xyxz, r1.xyxw
    r1.xyw = ((source[18].yyyy)*(r3.xyxz)+(r1.xyxw)).xyw;
    // 181: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 182: add r3.xyz, -r1.xywx, r0.zzzz
    r3.xyz = ((-(r1.xywx))+(r0.zzzz)).xyz;
    // 183: mad r1.xyw, cb0[18].zzzz, r3.xyxz, r1.xyxw
    r1.xyw = ((source[18].zzzz)*(r3.xyxz)+(r1.xyxw)).xyw;
    // 184: mul r1.xyw, r11.xyxz, r1.xyxw
    r1.xyw = ((r11.xyxz)*(r1.xyxw)).xyw;
    // 185: add r0.z, -cb0[4].w, l(1.000000)
    r0.z = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 186: mul r0.z, r0.z, cb0[22].z
    r0.z = ((r0.zzzz)*(source[22].zzzz)).z;
    // 187: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 188: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 189: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 190: mul r3.x, cb0[4].z, l(1.500000)
    r3.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 191: mul r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)*(r3.xxxx)).z;
    // 192: mad r0.z, r0.z, l(0.500000), cb0[4].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).z;
    // 193: frc r3.x, v4.x
    r3.x = (frac(v4.xxxx)).x;
    // 194: mul r3.x, r3.x, l(0.125000)
    r3.x = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 195: mul r5.y, cb0[4].y, cb0[14].y
    r5.y = ((source[4].yyyy)*(source[14].yyyy)).y;
    // 196: mov r3.y, v4.y
    r3.y = (v4.yyyy).y;
    // 197: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 198: add r3.xy, r3.xyxx, r5.xyxx
    r3.xy = ((r3.xyxx)+(r5.xyxx)).xy;
    // 199: frc r3.z, cb0[4].x
    r3.z = (frac(source[4].xxxx)).z;
    // 200: add r3.w, -r3.z, cb0[4].x
    r3.w = ((-(r3.zzzz))+(source[4].xxxx)).w;
    // 201: mul r5.z, r3.w, l(0.125000)
    r5.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 202: add r3.xy, r3.xyxx, r5.zwzz
    r3.xy = ((r3.xyxx)+(r5.zwzz)).xy;
    // 203: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 204: mul r3.xyw, r0.zzzz, r5.xyxz
    r3.xyw = ((r0.zzzz)*(r5.xyxz)).xyw;
    // 205: mul r0.z, r3.z, r5.w
    r0.z = ((r3.zzzz)*(r5.wwww)).z;
    // 206: add r3.z, -r3.z, l(1.000000)
    r3.z = ((-(r3.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: mad r3.xyw, r3.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxw
    r3.xyw = ((r3.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxw))).xyw;
    // 208: mad r1.xyw, r0.zzzz, r3.xyxw, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r3.xyxw)+(r1.xyxw)).xyw;
    // 209: add r5.xyzw, v7.yzxy, cb0[0].yzxy
    r5.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 210: add r5.xyzw, r5.xyzw, -cb0[1].yzxy
    r5.xyzw = ((r5.xyzw)+(-(source[1].yzxy))).xyzw;
    // 211: add r3.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 212: add r3.xy, -r5.zwzz, r3.xyxx
    r3.xy = ((-(r5.zwzz))+(r3.xyxx)).xy;
    // 213: mad r3.xy, cb0[15].wwww, r3.xyxx, r5.zwzz
    r3.xy = ((source[15].wwww)*(r3.xyxx)+(r5.zwzz)).xy;
    // 214: mul r0.z, cb0[15].y, cb0[22].z
    r0.z = ((source[15].yyyy)*(source[22].zzzz)).z;
    // 215: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 216: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 217: mul r5.y, r0.z, l(0.020000)
    r5.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 218: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 219: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 220: mul r3.w, cb0[15].x, l(0.001000)
    r3.w = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 221: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 222: mad r3.xy, r3.wwww, r3.xyxx, r5.xyxx
    r3.xy = ((r3.wwww)*(r3.xyxx)+(r5.xyxx)).xy;
    // 223: dp2 r3.w, cb0[16].xyxx, r3.xyxx
    r3.w = (dot((source[16].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 224: dp2 r3.y, cb0[17].xyxx, r3.xyxx
    r3.y = (dot((source[17].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 225: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 226: mul r3.x, r3.w, l(0.125000)
    r3.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 227: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 228: mad r3.xyw, r5.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxw
    r3.xyw = ((r5.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxw))).xyw;
    // 229: mul r4.w, r5.w, l(0.900000)
    r4.w = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 230: mad r3.xyw, r4.wwww, r3.xyxw, r1.xyxw
    r3.xyw = ((r4.wwww)*(r3.xyxw)+(r1.xyxw)).xyw;
    // 231: mul_sat r3.xyw, r0.zzzz, r3.xyxw
    r3.xyw = (saturate((r0.zzzz)*(r3.xyxw))).xyw;
    // 232: mad r5.xyz, cb0[15].zzzz, r3.xywx, -r1.xywx
    r5.xyz = ((source[15].zzzz)*(r3.xywx)+(-(r1.xywx))).xyz;
    // 233: mul r3.xyw, r3.xyxw, cb0[15].zzzz
    r3.xyw = ((r3.xyxw)*(source[15].zzzz)).xyw;
    // 234: dp3 r0.z, r3.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 235: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 236: mad r1.xyw, r0.zzzz, r5.xyxz, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r5.xyxz)+(r1.xyxw)).xyw;
    // 237: mad r1.xyw, r1.xyxw, cb2[3].wwww, cb2[3].xyxz
    r1.xyw = ((r1.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 238: add r0.z, r2.w, -r9.z
    r0.z = ((r2.wwww)+(-(r9.zzzz))).z;
    // 239: mad r3.xyw, r2.wwww, cb0[12].xyxz, -cb0[12].xyxz
    r3.xyw = ((r2.wwww)*(source[12].xyxz)+(-(source[12].xyxz))).xyw;
    // 240: mad r3.xyw, cb0[12].wwww, r3.xyxw, cb0[12].xyxz
    r3.xyw = ((source[12].wwww)*(r3.xyxw)+(source[12].xyxz)).xyw;
    // 241: mad r0.z, cb0[11].w, r0.z, r9.z
    r0.z = ((source[11].wwww)*(r0.zzzz)+(r9.zzzz)).z;
    // 242: mad r3.xyw, r0.zzzz, cb0[11].xyxz, r3.xyxw
    r3.xyw = ((r0.zzzz)*(source[11].xyxz)+(r3.xyxw)).xyw;
    // 243: add r0.z, cb0[2].y, cb0[2].x
    r0.z = ((source[2].yyyy)+(source[2].xxxx)).z;
    // 244: add r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)+(source[2].zzzz)).z;
    // 245: mul r0.z, r0.z, l(0.010000)
    r0.z = ((r0.zzzz)*(float4(0.010000,0.010000,0.010000,0.010000))).z;
    // 246: mad r0.z, cb0[22].y, cb0[22].z, r0.z
    r0.z = ((source[22].yyyy)*(source[22].zzzz)+(r0.zzzz)).z;
    // 247: mul r2.w, r0.z, l(3.524534)
    r2.w = ((r0.zzzz)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 248: sincos null, r2.w, r2.w
    r2.w = (cos(r2.wwww)).w;
    // 249: add r0.z, r0.z, r2.w
    r0.z = ((r0.zzzz)+(r2.wwww)).z;
    // 250: mul r0.z, r0.z, l(1.328987)
    r0.z = ((r0.zzzz)*(float4(1.328987,1.328987,1.328987,1.328987))).z;
    // 251: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 252: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 253: mad r0.z, r0.z, l(0.500000), cb0[22].x
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[22].xxxx)).z;
    // 254: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 255: mul r8.xyz, cb0[10].xyzx, cb0[21].wwww
    r8.xyz = ((source[10].xyzx)*(source[21].wwww)).xyz;
    // 256: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 257: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 258: mad r0.xyz, r3.zzzz, r0.xywx, r5.xyzx
    r0.xyz = ((r3.zzzz)*(r0.xywx)+(r5.xyzx)).xyz;
    // 259: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r5.xyz, -r0.xyzx, r0.wwww
    r5.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 261: mad r0.xyz, cb0[18].yyyy, r5.xyzx, r0.xyzx
    r0.xyz = ((source[18].yyyy)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 262: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: add r5.xyz, -r0.xyzx, r0.wwww
    r5.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 264: mad r0.xyz, cb0[18].zzzz, r5.xyzx, r0.xyzx
    r0.xyz = ((source[18].zzzz)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 265: mad r0.xyz, r0.xyzx, r11.xyzx, r3.xywx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r3.xywx)).xyz;
    // 266: log r0.w, |r1.z|
    r0.w = (log2(abs(r1.zzzz))).w;
    // 267: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 268: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 269: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 270: mul r3.xyz, r0.wwww, cb0[13].xyzx
    r3.xyz = ((r0.wwww)*(source[13].xyzx)).xyz;
    // 271: movc r3.xyz, r1.zzzz, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 272: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 273: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 274: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 275: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 276: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 277: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 278: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 279: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 280: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 281: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 282: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 283: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 284: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 285: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 286: mad r0.xyz, r3.xyzx, r1.xywx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xywx)+(r0.xyzx)).xyz;
    // 287: mul r3.xyz, r1.xywx, r3.xyzx
    r3.xyz = ((r1.xywx)*(r3.xyzx)).xyz;
    // 288: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 289: mad o0.xyz, r1.xywx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xywx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 290: mov o3.xyz, r1.xywx
    output.targets[3].xyz = (r1.xywx).xyz;
    // 291: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 292: dp3 r0.x, r4.xyzx, r2.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 293: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 294: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 295: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 296: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 297: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 298: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 299: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 300: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 301: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 302: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 303: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 304: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 305: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 306: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 307: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 308: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 309: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 310: ret
    return output;
}

// source.character.equipment-native-800.v1 / source program e95dd62780e50244ba30af26a3184188
