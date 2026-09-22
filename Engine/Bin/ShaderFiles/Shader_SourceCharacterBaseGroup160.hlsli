SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase160(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].z=(g_SourceCharacterTime.xxxx).x;
    source[23].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
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

// source.character.equipment-native-161.v1 / source program b6e7c450323bd34f9ff05ced5554239d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase161(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[23].x=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[23].y=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[23].z=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: dp3 r0.x, r0.xyzx, cb0[16].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[16].xyzx).xyz).xxxx).x;
    // 3: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[16].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[16].xyzx).xyz).xxxx)).y;
    // 4: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 7: add_sat r0.y, r1.w, -cb0[24].w
    r0.y = (saturate((r1.wwww)+(-(source[24].wwww)))).y;
    // 8: mad r0.x, r0.y, r0.x, l(-0.001000)
    r0.x = ((r0.yyyy)*(r0.xxxx)+(float4(-0.001000,-0.001000,-0.001000,-0.001000))).x;
    // 9: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 11: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 12: add r0.x, -v4.z, l(1.000000)
    r0.x = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: add r0.y, -r0.x, v4.z
    r0.y = ((-(r0.xxxx))+(v4.zzzz)).y;
    // 14: mad r0.x, cb0[18].y, r0.y, r0.x
    r0.x = ((source[18].yyyy)*(r0.yyyy)+(r0.xxxx)).x;
    // 15: mul r0.y, r0.x, cb0[18].z
    r0.y = ((r0.xxxx)*(source[18].zzzz)).y;
    // 16: mad r0.x, r0.y, l(0.750000), r0.x
    r0.x = ((r0.yyyy)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.xxxx)).x;
    // 17: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 18: mad_sat r0.x, cb0[19].x, r0.x, r0.x
    r0.x = (saturate((source[19].xxxx)*(r0.xxxx)+(r0.xxxx))).x;
    // 19: add r0.y, -r1.y, l(1.000000)
    r0.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 21: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: mad r0.z, r2.x, r1.x, l(0.200000)
    r0.z = ((r2.xxxx)*(r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 23: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 24: mad r0.w, cb0[20].x, r0.y, r0.z
    r0.w = ((source[20].xxxx)*(r0.yyyy)+(r0.zzzz)).w;
    // 25: mad r0.y, cb0[19].z, r0.y, r0.z
    r0.y = ((source[19].zzzz)*(r0.yyyy)+(r0.zzzz)).y;
    // 26: add r0.yz, -r0.xxxx, r0.yywy
    r0.yz = ((-(r0.xxxx))+(r0.yywy)).yz;
    // 27: mul r0.w, cb0[18].w, l(0.700000)
    r0.w = ((source[18].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 28: mad r0.z, r0.w, r0.z, r0.x
    r0.z = ((r0.wwww)*(r0.zzzz)+(r0.xxxx)).z;
    // 29: mad r0.x, r0.w, r0.y, r0.x
    r0.x = ((r0.wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 30: div r0.xy, r0.xzxx, cb0[19].ywyy
    r0.xy = ((r0.xzxx)/(source[19].ywyy)).xy;
    // 31: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 32: mad r0.z, cb0[17].y, l(-3.500000), l(5.000000)
    r0.z = ((source[17].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 33: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 34: mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // 35: mul r0.xy, r0.xyxx, l(4.000000, 4.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 36: add r0.z, v4.w, l(0.500000)
    r0.z = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 37: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 38: add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul_sat r0.xy, r0.zwzz, r0.xyxx
    r0.xy = (saturate((r0.zwzz)*(r0.xyxx))).xy;
    // 40: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 41: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 42: mad r0.x, cb0[20].y, r0.x, r1.y
    r0.x = ((source[20].yyyy)*(r0.xxxx)+(r1.yyyy)).x;
    // 43: add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // 44: mul r0.yzw, r0.yyzw, cb0[17].xxxx
    r0.yzw = ((r0.yyzw)*(source[17].xxxx)).yzw;
    // 45: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // 46: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 47: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 48: mad r0.xyz, cb0[20].zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 49: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 50: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 51: mad r0.xyz, cb0[20].wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((source[20].wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 52: mad r3.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 55: mul r4.xyz, r0.xyzx, r3.xyzx
    r4.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 56: mad r0.xyz, r0.xyzx, r3.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 57: mul r1.xyw, r1.xxxx, r4.xyxz
    r1.xyw = ((r1.xxxx)*(r4.xyxz)).xyw;
    // 58: add r0.w, -cb0[9].w, l(1.000000)
    r0.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul r0.w, r0.w, cb0[22].z
    r0.w = ((r0.wwww)*(source[22].zzzz)).w;
    // 60: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 61: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 62: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 63: mul r2.w, cb0[9].z, l(1.500000)
    r2.w = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 64: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 65: mad r0.w, r0.w, l(0.500000), cb0[9].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
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
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t3.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 77: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 78: mul r0.w, r2.w, r3.w
    r0.w = ((r2.wwww)*(r3.wwww)).w;
    // 79: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xywx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xywx))).xyz;
    // 80: mad r1.xyw, r0.wwww, r3.xyxz, r1.xyxw
    r1.xyw = ((r0.wwww)*(r3.xyxz)+(r1.xyxw)).xyw;
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
    // 87: mul r0.w, cb0[11].y, cb0[22].z
    r0.w = ((source[11].yyyy)*(source[22].zzzz)).w;
    // 88: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 89: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 90: mul r5.y, r0.w, l(0.020000)
    r5.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 91: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
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
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xywx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xywx))).xyz;
    // 102: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 103: mad r4.xyz, r2.wwww, r4.xyzx, r1.xywx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r1.xywx)).xyz;
    // 104: mul_sat r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = (saturate((r0.wwww)*(r4.xyzx))).xyz;
    // 105: mad r5.xyz, cb0[11].zzzz, r4.xyzx, -r1.xywx
    r5.xyz = ((source[11].zzzz)*(r4.xyzx)+(-(r1.xywx))).xyz;
    // 106: mul r4.xyz, r4.xyzx, cb0[11].zzzz
    r4.xyz = ((r4.xyzx)*(source[11].zzzz)).xyz;
    // 107: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 108: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 109: mad r1.xyw, r0.wwww, r5.xyxz, r1.xyxw
    r1.xyw = ((r0.wwww)*(r5.xyxz)+(r1.xyxw)).xyw;
    // 110: mul r1.xyw, r1.xyxw, cb0[23].wwww
    r1.xyw = ((r1.xyxw)*(source[23].wwww)).xyw;
    // 111: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 112: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 114: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 115: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 116: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 117: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 118: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 119: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 120: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 121: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 122: dp3 r0.w, r2.xyzx, r4.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 123: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mad r4.xyw, r2.wwww, cb0[8].xyxz, r1.xyxw
    r4.xyw = ((r2.wwww)*(source[8].xyxz)+(r1.xyxw)).xyw;
    // 125: mad r1.xyw, r1.xyxw, cb2[3].wwww, cb2[3].xyxz
    r1.xyw = ((r1.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 126: add r2.w, -|r0.w|, l(1.000000)
    r2.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: mul_sat r0.w, r0.w, cb0[24].x
    r0.w = (saturate((r0.wwww)*(source[24].xxxx))).w;
    // 128: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 134: mul_sat r2.w, r4.z, cb0[24].x
    r2.w = (saturate((r4.zzzz)*(source[24].xxxx))).w;
    // 135: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: add_sat r2.w, r2.w, -cb0[24].y
    r2.w = (saturate((r2.wwww)+(-(source[24].yyyy)))).w;
    // 137: log r3.w, r2.w
    r3.w = (log2(r2.wwww)).w;
    // 138: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: mul r3.w, r3.w, cb0[24].z
    r3.w = ((r3.wwww)*(source[24].zzzz)).w;
    // 140: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 141: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 142: mul r0.w, r0.w, cb0[15].w
    r0.w = ((r0.wwww)*(source[15].wwww)).w;
    // 143: mul r5.xyz, r0.wwww, cb0[15].xyzx
    r5.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 144: movc r5.xyz, r2.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 145: add r4.xyw, r4.xyxw, r5.xyxz
    r4.xyw = ((r4.xyxw)+(r5.xyxz)).xyw;
    // 146: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 147: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 148: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 149: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 150: mov_sat r0.w, r4.z
    r0.w = (saturate(r4.zzzz)).w;
    // 151: mul r5.xyz, r3.xyzx, r4.zzzz
    r5.xyz = ((r3.xyzx)*(r4.zzzz)).xyz;
    // 152: mad r3.xyz, r5.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r3.xyzx)).xyz;
    // 153: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 154: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 155: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 156: mul r0.xyz, r0.xyzx, cb0[21].xxxx
    r0.xyz = ((r0.xyzx)*(source[21].xxxx)).xyz;
    // 157: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 158: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 159: div r0.w, r3.z, r0.w
    r0.w = ((r3.zzzz)/(r0.wwww)).w;
    // 160: add r0.w, r0.w, cb0[7].z
    r0.w = ((r0.wwww)+(source[7].zzzz)).w;
    // 161: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 162: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 163: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 164: dp3 r1.z, r3.xyzx, r2.xyzx
    r1.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 165: add r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)+(r1.zzzz)).w;
    // 166: frc r0.w, r0.w
    r0.w = (frac(r0.wwww)).w;
    // 167: add r0.w, r0.w, l(-0.500000)
    r0.w = ((r0.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 168: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 169: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: log r1.z, r0.w
    r1.z = (log2(r0.wwww)).z;
    // 171: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 172: mad r2.w, cb0[21].w, l(4.500000), l(0.500000)
    r2.w = ((source[21].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 173: mul r2.w, r2.w, cb0[22].x
    r2.w = ((r2.wwww)*(source[22].xxxx)).w;
    // 174: mul r2.w, r2.w, l(0.050000)
    r2.w = ((r2.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 175: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 176: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 177: mul r1.z, r1.z, cb0[22].y
    r1.z = ((r1.zzzz)*(source[22].yyyy)).z;
    // 178: movc r0.w, r0.w, l(0), r1.z
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).w;
    // 179: mad r0.xyz, r0.wwww, r0.xyzx, r4.xywx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xywx)).xyz;
    // 180: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 181: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 182: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 183: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 184: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 185: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 186: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 187: dp3 r0.w, r4.xyzx, r2.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 188: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 189: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 190: mul r4.yzw, r4.yyyy, cb0[26].xxyz
    r4.yzw = ((r4.yyyy)*(source[26].xxyz)).yzw;
    // 191: mad r4.xyz, r4.xxxx, cb0[25].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[25].xyzx)+(r4.yzwy)).xyz;
    // 192: mul r4.xyz, r4.xyzx, cb0[27].wwww
    r4.xyz = ((r4.xyzx)*(source[27].wwww)).xyz;
    // 193: mad r0.xyz, r4.xyzx, r1.xywx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r1.xywx)+(r0.xyzx)).xyz;
    // 194: mul r4.xyz, r1.xywx, r4.xyzx
    r4.xyz = ((r1.xywx)*(r4.xyzx)).xyz;
    // 195: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mad o0.xyz, r1.xywx, cb0[27].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xywx)*(source[27].xyzx)+(r0.xyzx)).xyz;
    // 197: mov o3.xyz, r1.xywx
    output.targets[3].xyz = (r1.xywx).xyz;
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

// source.character.equipment-native-162.v1 / source program ef3ed8accecee5408d9ec5722c094cca
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase162(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[19]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[26].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 22: add r1.y, -cb0[21].y, cb0[21].x
    r1.y = ((-(source[21].yyyy))+(source[21].xxxx)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: mad r1.y, r2.x, r1.y, cb0[21].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[21].yyyy)).y;
    // 25: add r1.z, -r1.y, cb0[21].z
    r1.z = ((-(r1.yyyy))+(source[21].zzzz)).z;
    // 26: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 27: add r1.z, -r1.y, cb0[21].w
    r1.z = ((-(r1.yyyy))+(source[21].wwww)).z;
    // 28: mad r1.y, r2.z, r1.z, r1.y
    r1.y = ((r2.zzzz)*(r1.zzzz)+(r1.yyyy)).y;
    // 29: add r1.z, -r1.y, cb0[22].x
    r1.z = ((-(r1.yyyy))+(source[22].xxxx)).z;
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
    // 40: add r1.w, -r1.z, cb0[24].x
    r1.w = ((-(r1.zzzz))+(source[24].xxxx)).w;
    // 41: mad r1.z, r2.w, r1.w, r1.z
    r1.z = ((r2.wwww)*(r1.wwww)+(r1.zzzz)).z;
    // 42: add r1.w, -cb0[23].y, cb0[23].x
    r1.w = ((-(source[23].yyyy))+(source[23].xxxx)).w;
    // 43: mad r1.w, r2.x, r1.w, cb0[23].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[23].yyyy)).w;
    // 44: add r3.w, -r1.w, cb0[23].z
    r3.w = ((-(r1.wwww))+(source[23].zzzz)).w;
    // 45: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 46: add r3.w, -r1.w, cb0[23].w
    r3.w = ((-(r1.wwww))+(source[23].wwww)).w;
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
    // 60: mul r5.xy, r4.xyxx, cb0[20].xxxx
    r5.xy = ((r4.xyxx)*(source[20].xxxx)).xy;
    // 61: mad r4.xy, cb0[20].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[20].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 62: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 63: mad r1.xzw, r2.wwww, r4.xxyz, r5.xxyz
    r1.xzw = ((r2.wwww)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 64: add r4.xyz, -r1.xzwx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xzwx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 65: mad r4.xyz, cb0[22].wwww, r4.xyzx, r1.xzwx
    r4.xyz = ((source[22].wwww)*(r4.xyzx)+(r1.xzwx)).xyz;
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
    // 99: sample_l_indexable(texture2d)(float,float,float,float) r8.xyz, r8.xyxx, t4.xyzw, s4, r0.x
    r8.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r8.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 100: log r10.xyz, r8.xyzx
    r10.xyz = (log2(r8.xyzx)).xyz;
    // 101: rcp r0.x, cb0[24].y
    r0.x = (1.0/(source[24].yyyy)).x;
    // 102: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 103: mul r10.xyz, r10.xyzx, cb0[24].yyyy
    r10.xyz = ((r10.xyzx)*(source[24].yyyy)).xyz;
    // 104: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 105: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 106: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 107: mad r10.xyz, r10.xyzx, cb0[24].yyyy, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[24].yyyy)+(r11.xyzx)).xyz;
    // 108: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 109: mul r8.xyz, r8.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 110: add r0.x, cb0[24].y, l(1.000000)
    r0.x = ((source[24].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 112: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: add r8.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r8.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 114: mad r8.xyz, r4.wwww, r8.xyzx, cb0[11].xyzx
    r8.xyz = ((r4.wwww)*(r8.xyzx)+(source[11].xyzx)).xyz;
    // 115: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 116: mul r8.xyz, r8.xyzx, cb0[24].zzzz
    r8.xyz = ((r8.xyzx)*(source[24].zzzz)).xyz;
    // 117: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 118: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 119: mad r3.yzw, cb0[22].yyyy, r10.xxyz, r3.xxyz
    r3.yzw = ((source[22].yyyy)*(r10.xxyz)+(r3.xxyz)).yzw;
    // 120: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r10.xyz, -r3.yzwy, r0.xxxx
    r10.xyz = ((-(r3.yzwy))+(r0.xxxx)).xyz;
    // 122: mad r3.yzw, cb0[22].zzzz, r10.xxyz, r3.yyzw
    r3.yzw = ((source[22].zzzz)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 123: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 124: add r10.xyz, -r3.yzwy, r0.xxxx
    r10.xyz = ((-(r3.yzwy))+(r0.xxxx)).xyz;
    // 125: mul r10.xyz, r10.xyzx, cb0[24].wwww
    r10.xyz = ((r10.xyzx)*(source[24].wwww)).xyz;
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
    // 138: add r3.w, cb0[25].w, -cb0[26].x
    r3.w = ((source[25].wwww)+(-(source[26].xxxx))).w;
    // 139: mad r3.w, r2.w, r3.w, cb0[26].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[26].xxxx)).w;
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
    // 145: div r3.w, cb0[26].y, r3.w
    r3.w = ((source[26].yyyy)/(r3.wwww)).w;
    // 146: dp3 r4.w, r1.xzwx, r1.xzwx
    r4.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 147: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 148: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 149: dp3 r4.w, r1.xzwx, r4.xyzx
    r4.w = (dot((r1.xzwx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 150: mul_sat r5.w, r4.w, cb0[25].x
    r5.w = (saturate((r4.wwww)*(source[25].xxxx))).w;
    // 151: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mul_sat r6.w, r4.z, cb0[25].x
    r6.w = (saturate((r4.zzzz)*(source[25].xxxx))).w;
    // 154: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: add_sat r6.w, r6.w, -cb0[25].y
    r6.w = (saturate((r6.wwww)+(-(source[25].yyyy)))).w;
    // 156: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 157: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 158: mul r7.w, r7.w, cb0[25].z
    r7.w = ((r7.wwww)*(source[25].zzzz)).w;
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
    // 166: mad r0.yzw, cb0[22].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[22].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 167: dp3 r3.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 168: add r11.xyz, -r0.yzwy, r3.wwww
    r11.xyz = ((-(r0.yzwy))+(r3.wwww)).xyz;
    // 169: mad r0.yzw, cb0[22].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[22].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 170: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 171: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 172: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 173: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 174: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 175: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 176: mad r2.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r2.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 177: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 179: mad r2.xyz, cb0[22].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 180: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 182: mad r2.xyz, cb0[22].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 183: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 186: mul r12.xyz, r2.xyzx, r11.xyzx
    r12.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 187: mad r2.xyz, -r2.xyzx, r11.xyzx, cb0[10].xyzx
    r2.xyz = ((-(r2.xyzx))*(r11.xyzx)+(source[10].xyzx)).xyz;
    // 188: mad r2.xyz, r2.wwww, r2.xyzx, r12.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 189: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 190: mul r2.xyz, r8.xyzx, r0.yzwy
    r2.xyz = ((r8.xyzx)*(r0.yzwy)).xyz;
    // 191: mad r3.xyz, r3.xyzx, r10.xyzx, -r2.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r2.xyzx))).xyz;
    // 192: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: mul r2.w, r2.w, cb0[26].z
    r2.w = ((r2.wwww)*(source[26].zzzz)).w;
    // 194: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 195: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 196: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 197: div r3.xyz, r9.xyzx, r3.xxxx
    r3.xyz = ((r9.xyzx)/(r3.xxxx)).xyz;
    // 198: dp3 r3.x, r3.xyzx, r4.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 199: add r3.y, -|r4.z|, l(1.000000)
    r3.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: mul r3.y, r4.w, r3.y
    r3.y = ((r4.wwww)*(r3.yyyy)).y;
    // 201: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 202: mul r3.z, |r3.x|, |r3.x|
    r3.z = ((abs(r3.xxxx))*(abs(r3.xxxx))).z;
    // 203: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 204: mul r3.z, r3.z, |r3.x|
    r3.z = ((r3.zzzz)*(abs(r3.xxxx))).z;
    // 205: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 206: movc r3.x, r3.x, l(0), r3.z
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).x;
    // 207: add r3.z, r3.x, l(-0.027778)
    r3.z = ((r3.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 208: mad r3.x, r3.x, r3.z, l(0.027778)
    r3.x = ((r3.xxxx)*(r3.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 209: div_sat r2.w, r3.x, r2.w
    r2.w = (saturate((r3.xxxx)/(r2.wwww))).w;
    // 210: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 211: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 212: mad r3.xzw, r1.yyyy, r2.xxyz, -r0.yyzw
    r3.xzw = ((r1.yyyy)*(r2.xxyz)+(-(r0.yyzw))).xzw;
    // 213: mad r0.xyz, r0.xxxx, r3.xzwx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xzwx)+(r0.yzwy)).xyz;
    // 214: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 215: add r3.xzw, -r0.xxyz, r0.wwww
    r3.xzw = ((-(r0.xxyz))+(r0.wwww)).xzw;
    // 216: mad r0.xyz, cb0[22].yyyy, r3.xzwx, r0.xyzx
    r0.xyz = ((source[22].yyyy)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 217: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 218: add r3.xzw, -r0.xxyz, r0.wwww
    r3.xzw = ((-(r0.xxyz))+(r0.wwww)).xzw;
    // 219: mad r0.xyz, cb0[22].zzzz, r3.xzwx, r0.xyzx
    r0.xyz = ((source[22].zzzz)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 220: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 221: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 222: mul r0.w, r0.w, cb0[26].w
    r0.w = ((r0.wwww)*(source[26].wwww)).w;
    // 223: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 224: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 225: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 227: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 228: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 229: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 230: mul r4.x, r1.y, l(0.125000)
    r4.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 231: mul r8.y, cb0[3].y, cb0[16].y
    r8.y = ((source[3].yyyy)*(source[16].yyyy)).y;
    // 232: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 233: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 234: add r3.xz, r4.xxyx, r8.xxyx
    r3.xz = ((r4.xxyx)+(r8.xxyx)).xz;
    // 235: frc r1.y, cb0[3].x
    r1.y = (frac(source[3].xxxx)).y;
    // 236: add r2.w, -r1.y, cb0[3].x
    r2.w = ((-(r1.yyyy))+(source[3].xxxx)).w;
    // 237: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: add r3.xz, r3.xxzx, r8.zzwz
    r3.xz = ((r3.xxzx)+(r8.zzwz)).xz;
    // 239: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xzxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 240: mul r3.xzw, r0.wwww, r4.xxyz
    r3.xzw = ((r0.wwww)*(r4.xxyz)).xzw;
    // 241: mul r0.w, r1.y, r4.w
    r0.w = ((r1.yyyy)*(r4.wwww)).w;
    // 242: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 243: mad r3.xzw, r3.xxzw, l(2.000000, 0.000000, 2.000000, 2.000000), -r0.xxyz
    r3.xzw = ((r3.xxzw)*(float4(2.000000,0.000000,2.000000,2.000000))+(-(r0.xxyz))).xzw;
    // 244: mad r0.xyz, r0.wwww, r3.xzwx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 245: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 246: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 247: add r3.xz, -r4.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r3.xz = ((-(r4.xxyx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 248: add r3.xz, -r4.zzwz, r3.xxzx
    r3.xz = ((-(r4.zzwz))+(r3.xxzx)).xz;
    // 249: mad r3.xz, cb0[17].wwww, r3.xxzx, r4.zzwz
    r3.xz = ((source[17].wwww)*(r3.xxzx)+(r4.zzwz)).xz;
    // 250: mul r0.w, cb0[17].y, cb0[26].w
    r0.w = ((source[17].yyyy)*(source[26].wwww)).w;
    // 251: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 252: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 253: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 254: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 255: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 256: mul r2.w, cb0[17].x, l(0.001000)
    r2.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 257: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 258: mad r3.xz, r2.wwww, r3.xxzx, r4.xxyx
    r3.xz = ((r2.wwww)*(r3.xxzx)+(r4.xxyx)).xz;
    // 259: dp2 r2.w, cb0[18].xyxx, r3.xzxx
    r2.w = (dot((source[18].xyxx).xy,(r3.xzxx).xy).xxxx).w;
    // 260: dp2 r4.y, cb0[19].xyxx, r3.xzxx
    r4.y = (dot((source[19].xyxx).xy,(r3.xzxx).xy).xxxx).y;
    // 261: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 262: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 263: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 264: mad r3.xzw, r4.xxyz, l(3.500000, 0.000000, 3.500000, 3.500000), -r0.xxyz
    r3.xzw = ((r4.xxyz)*(float4(3.500000,0.000000,3.500000,3.500000))+(-(r0.xxyz))).xzw;
    // 265: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 266: mad r3.xzw, r2.wwww, r3.xxzw, r0.xxyz
    r3.xzw = ((r2.wwww)*(r3.xxzw)+(r0.xxyz)).xzw;
    // 267: mul_sat r3.xzw, r0.wwww, r3.xxzw
    r3.xzw = (saturate((r0.wwww)*(r3.xxzw))).xzw;
    // 268: mad r4.xyz, cb0[17].zzzz, r3.xzwx, -r0.xyzx
    r4.xyz = ((source[17].zzzz)*(r3.xzwx)+(-(r0.xyzx))).xyz;
    // 269: mul r3.xzw, r3.xxzw, cb0[17].zzzz
    r3.xzw = ((r3.xxzw)*(source[17].zzzz)).xzw;
    // 270: dp3 r0.w, r3.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 271: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 272: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 273: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 274: mul r3.xzw, r2.xxyz, r1.yyyy
    r3.xzw = ((r2.xxyz)*(r1.yyyy)).xzw;
    // 275: dp3 r0.w, r3.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 276: mad r2.xyz, -r1.yyyy, r2.xyzx, r0.wwww
    r2.xyz = ((-(r1.yyyy))*(r2.xyzx)+(r0.wwww)).xyz;
    // 277: mad r2.xyz, cb0[22].yyyy, r2.xyzx, r3.xzwx
    r2.xyz = ((source[22].yyyy)*(r2.xyzx)+(r3.xzwx)).xyz;
    // 278: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 279: add r3.xzw, -r2.xxyz, r0.wwww
    r3.xzw = ((-(r2.xxyz))+(r0.wwww)).xzw;
    // 280: mad r2.xyz, cb0[22].zzzz, r3.xzwx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r3.xzwx)+(r2.xyzx)).xyz;
    // 281: mad r3.xzw, r5.wwww, cb0[14].xxyz, -cb0[14].xxyz
    r3.xzw = ((r5.wwww)*(source[14].xxyz)+(-(source[14].xxyz))).xzw;
    // 282: mul r0.w, r5.w, cb0[13].w
    r0.w = ((r5.wwww)*(source[13].wwww)).w;
    // 283: mad r3.xzw, cb0[14].wwww, r3.xxzw, cb0[14].xxyz
    r3.xzw = ((source[14].wwww)*(r3.xxzw)+(source[14].xxyz)).xzw;
    // 284: mad r3.xzw, r0.wwww, cb0[13].xxyz, r3.xxzw
    r3.xzw = ((r0.wwww)*(source[13].xxyz)+(r3.xxzw)).xzw;
    // 285: mad r2.xyz, r2.xyzx, r11.xyzx, r3.xzwx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)+(r3.xzwx)).xyz;
    // 286: log r0.w, |r3.y|
    r0.w = (log2(abs(r3.yyyy))).w;
    // 287: lt r1.y, |r3.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 288: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 289: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 290: mul r3.xyz, r0.wwww, cb0[15].xyzx
    r3.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 291: movc r3.xyz, r1.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 292: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 293: add r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)+(source[2].xyzx)).xyz;
    // 294: dp3 r0.w, r1.xzwx, r1.xzwx
    r0.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 295: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 296: mul r1.xyz, r0.wwww, r1.xzwx
    r1.xyz = ((r0.wwww)*(r1.xzwx)).xyz;
    // 297: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 298: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 299: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 300: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 301: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 302: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 303: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 304: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 305: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 306: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 307: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 308: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 309: mad o0.xyz, r0.xyzx, cb0[29].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[29].xyzx)+(r2.xyzx)).xyz;
    // 310: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 311: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 312: dp3 r0.x, r6.xyzx, r1.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 313: dp3 r0.z, r5.xyzx, r1.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 314: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 315: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 316: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 317: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 318: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 319: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 320: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 321: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 322: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 323: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 324: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 325: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 326: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 327: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 328: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 329: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 330: ret
    return output;
}

// source.character.equipment-native-163.v1 / source program 2fb3a18e10197b4fa718ab4b0e052435
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase163(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[20]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[28].x=(g_SourceCharacterTime.xxxx).x;
    source[28].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[28].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s7, l(0.000000)
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 33: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 34: add r1.w, -cb0[23].y, cb0[23].x
    r1.w = ((-(source[23].yyyy))+(source[23].xxxx)).w;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mad r1.w, r3.w, r1.w, cb0[23].y
    r1.w = ((r3.wwww)*(r1.wwww)+(source[23].yyyy)).w;
    // 37: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 38: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 39: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 40: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 41: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 42: add r1.w, -r0.w, cb0[24].y
    r1.w = ((-(r0.wwww))+(source[24].yyyy)).w;
    // 43: mad r0.w, r3.w, r1.w, r0.w
    r0.w = ((r3.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 44: mul r0.w, r0.w, cb0[24].z
    r0.w = ((r0.wwww)*(source[24].zzzz)).w;
    // 45: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 46: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 47: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 48: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 50: mad r4.xyzw, r0.ywyw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r0.ywyw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 51: dp2 r0.y, r4.zwzz, r4.zwzz
    r0.y = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).y;
    // 52: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 54: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 55: add r5.z, r0.y, l(0.000010)
    r5.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 56: mul r5.xy, r4.xyxx, cb0[22].xxxx
    r5.xy = ((r4.xyxx)*(source[22].xxxx)).xy;
    // 57: mad r4.xy, cb0[22].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[22].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 58: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 59: mad r4.xyz, r3.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 60: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: mad r5.xyz, cb0[24].xxxx, r5.xyzx, r4.xyzx
    r5.xyz = ((source[24].xxxx)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 63: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 64: div r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)/(r0.yyyy)).xyz;
    // 65: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 68: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r7.xyz, r0.yyyy, v0.xyzx
    r7.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 71: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 72: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 73: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 74: dp3 r9.y, r8.xyzx, r5.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 75: dp3 r9.x, r7.xyzx, r5.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 76: dp3 r9.z, r6.xyzx, r5.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 77: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 78: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 79: mul r5.xyz, r0.yyyy, v5.xyzx
    r5.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 80: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 81: dp3 r11.y, r8.xyzx, r5.xyzx
    r11.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 82: dp3 r11.x, r7.xyzx, r5.xyzx
    r11.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 83: dp3 r11.z, r6.xyzx, r5.xyzx
    r11.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 84: dp3 r0.y, r9.xyzx, r11.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 85: mul r9.xyz, r9.xyzx, r0.yyyy
    r9.xyz = ((r9.xyzx)*(r0.yyyy)).xyz;
    // 86: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 87: mov r9.w, -r9.x
    r9.w = (-(r9.xxxx)).w;
    // 88: dp2 r0.y, r9.ywyy, r9.ywyy
    r0.y = (dot((r9.ywyy).xy,(r9.ywyy).xy).xxxx).y;
    // 89: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 90: div r0.yw, r9.yyyw, r0.yyyy
    r0.yw = ((r9.yyyw)/(r0.yyyy)).yw;
    // 91: mad r1.w, -r9.z, l(0.250000), l(0.250000)
    r1.w = ((-(r9.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 92: add r2.w, r9.z, l(1.000000)
    r2.w = ((r9.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 94: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 95: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 96: log r9.xyz, r0.xywx
    r9.xyz = (log2(r0.xywx)).xyz;
    // 97: rcp r1.w, cb0[24].w
    r1.w = (1.0/(source[24].wwww)).w;
    // 98: mul r11.xyz, r9.xyzx, r1.wwww
    r11.xyz = ((r9.xyzx)*(r1.wwww)).xyz;
    // 99: mul r9.xyz, r9.xyzx, cb0[24].wwww
    r9.xyz = ((r9.xyzx)*(source[24].wwww)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 102: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 103: mad r9.xyz, r9.xyzx, cb0[24].wwww, r11.xyzx
    r9.xyz = ((r9.xyzx)*(source[24].wwww)+(r11.xyzx)).xyz;
    // 104: add r0.xyw, r0.xyxw, r9.xyxz
    r0.xyw = ((r0.xyxw)+(r9.xyxz)).xyw;
    // 105: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 106: add r1.w, cb0[24].w, l(1.000000)
    r1.w = ((source[24].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 108: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r9.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r9.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 110: mad r9.xyz, r2.wwww, r9.xyzx, cb0[11].xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(source[11].xyzx)).xyz;
    // 111: mul r0.xyw, r0.xxxx, r9.xyxz
    r0.xyw = ((r0.xxxx)*(r9.xyxz)).xyw;
    // 112: mul r0.xyw, r0.xyxw, cb0[25].xxxx
    r0.xyw = ((r0.xyxw)*(source[25].xxxx)).xyw;
    // 113: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r9.xyz, -r2.xyzx, r1.wwww
    r9.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 115: mad r2.yzw, cb0[23].zzzz, r9.xxyz, r2.xxyz
    r2.yzw = ((source[23].zzzz)*(r9.xxyz)+(r2.xxyz)).yzw;
    // 116: dp3 r1.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 117: add r9.xyz, -r2.yzwy, r1.wwww
    r9.xyz = ((-(r2.yzwy))+(r1.wwww)).xyz;
    // 118: mad r2.yzw, cb0[23].wwww, r9.xxyz, r2.yyzw
    r2.yzw = ((source[23].wwww)*(r9.xxyz)+(r2.yyzw)).yzw;
    // 119: dp3 r1.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 120: add r9.xyz, -r2.yzwy, r1.wwww
    r9.xyz = ((-(r2.yzwy))+(r1.wwww)).xyz;
    // 121: mul r9.xyz, r9.xyzx, cb0[25].yyyy
    r9.xyz = ((r9.xyzx)*(source[25].yyyy)).xyz;
    // 122: add r1.w, r3.y, r3.x
    r1.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 123: add r1.w, r3.z, r1.w
    r1.w = ((r3.zzzz)+(r1.wwww)).w;
    // 124: add_sat r1.w, r3.w, r1.w
    r1.w = (saturate((r3.wwww)+(r1.wwww))).w;
    // 125: mad r2.yzw, r1.wwww, r9.xxyz, r2.yyzw
    r2.yzw = ((r1.wwww)*(r9.xxyz)+(r2.yyzw)).yzw;
    // 126: add r9.xyz, -r2.yzwy, r2.xxxx
    r9.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 127: mad r2.xyz, r3.wwww, r9.xyzx, r2.yzwy
    r2.xyz = ((r3.wwww)*(r9.xyzx)+(r2.yzwy)).xyz;
    // 128: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 129: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 130: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 131: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 132: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 133: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 134: add r2.w, -cb0[26].z, cb0[26].y
    r2.w = ((-(source[26].zzzz))+(source[26].yyyy)).w;
    // 135: mad r2.w, r3.w, r2.w, cb0[26].z
    r2.w = ((r3.wwww)*(r2.wwww)+(source[26].zzzz)).w;
    // 136: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 137: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 138: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 141: div r2.w, cb0[26].w, r2.w
    r2.w = ((source[26].wwww)/(r2.wwww)).w;
    // 142: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 143: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 144: div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // 145: dp3 r4.w, r4.xyzx, r5.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 146: mul_sat r5.w, r4.w, cb0[25].z
    r5.w = (saturate((r4.wwww)*(source[25].zzzz))).w;
    // 147: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mul_sat r6.w, r5.z, cb0[25].z
    r6.w = (saturate((r5.zzzz)*(source[25].zzzz))).w;
    // 150: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: add_sat r6.w, r6.w, -cb0[25].w
    r6.w = (saturate((r6.wwww)+(-(source[25].wwww)))).w;
    // 152: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 153: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 154: mul r7.w, r7.w, cb0[26].x
    r7.w = ((r7.wwww)*(source[26].xxxx)).w;
    // 155: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 156: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 157: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 158: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 159: mul r9.xyz, r0.xywx, r2.wwww
    r9.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 160: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 162: mad r1.xyz, cb0[23].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[23].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 163: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 165: mad r1.xyz, cb0[23].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[23].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 166: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 167: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 168: mad r11.xyz, r3.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 169: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 170: mad r11.xyz, r3.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 171: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 173: mad r11.xyz, cb0[23].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[23].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 174: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 176: mad r11.xyz, cb0[23].wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((source[23].wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 177: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mad r13.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 180: mul r13.xyz, r11.xyzx, r12.xyzx
    r13.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 181: mad r11.xyz, -r11.xyzx, r12.xyzx, cb0[10].xyzx
    r11.xyz = ((-(r11.xyzx))*(r12.xyzx)+(source[10].xyzx)).xyz;
    // 182: mad r3.xyw, r3.wwww, r11.xyxz, r13.xyxz
    r3.xyw = ((r3.wwww)*(r11.xyxz)+(r13.xyxz)).xyw;
    // 183: mul r1.xyz, r1.xyzx, r3.xywx
    r1.xyz = ((r1.xyzx)*(r3.xywx)).xyz;
    // 184: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 185: mad r2.xyz, r2.xyzx, r9.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r0.xywx))).xyz;
    // 186: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 187: mul r2.w, r2.w, cb0[27].x
    r2.w = ((r2.wwww)*(source[27].xxxx)).w;
    // 188: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 189: dp3 r2.x, r10.xyzx, r10.xyzx
    r2.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 190: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 191: div r2.yzw, r10.xxyz, r2.yyyy
    r2.yzw = ((r10.xxyz)/(r2.yyyy)).yzw;
    // 192: dp3 r2.y, r2.yzwy, r5.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // 193: add r2.z, -|r5.z|, l(1.000000)
    r2.z = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 194: mul r2.z, r4.w, r2.z
    r2.z = ((r4.wwww)*(r2.zzzz)).z;
    // 195: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 196: mul r2.w, |r2.y|, |r2.y|
    r2.w = ((abs(r2.yyyy))*(abs(r2.yyyy))).w;
    // 197: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 198: mul r2.w, r2.w, |r2.y|
    r2.w = ((r2.wwww)*(abs(r2.yyyy))).w;
    // 199: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 200: movc r2.y, r2.y, l(0), r2.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 201: add r2.w, r2.y, l(-0.027778)
    r2.w = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 202: mad r2.y, r2.y, r2.w, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 203: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 204: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 206: mad r2.xyw, r0.zzzz, r0.xyxw, -r1.xyxz
    r2.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r1.xyxz))).xyw;
    // 207: mad r1.xyz, r1.wwww, r2.xywx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 208: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 209: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 210: mad r1.xyz, cb0[23].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[23].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 211: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 212: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 213: mad r1.xyz, cb0[23].wwww, r2.xywx, r1.xyzx
    r1.xyz = ((source[23].wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 214: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 215: add r0.z, -cb0[4].w, l(1.000000)
    r0.z = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 216: mul r0.z, r0.z, cb0[28].x
    r0.z = ((r0.zzzz)*(source[28].xxxx)).z;
    // 217: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 218: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 219: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 220: mul r1.w, cb0[4].z, l(1.500000)
    r1.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 221: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 222: mad r0.z, r0.z, l(0.500000), cb0[4].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).z;
    // 223: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 224: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 225: mul r9.y, cb0[4].y, cb0[17].y
    r9.y = ((source[4].yyyy)*(source[17].yyyy)).y;
    // 226: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 227: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 228: add r2.xy, r2.xyxx, r9.xyxx
    r2.xy = ((r2.xyxx)+(r9.xyxx)).xy;
    // 229: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 230: add r2.w, -r1.w, cb0[4].x
    r2.w = ((-(r1.wwww))+(source[4].xxxx)).w;
    // 231: mul r9.z, r2.w, l(0.125000)
    r9.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 232: add r2.xy, r2.xyxx, r9.zwzz
    r2.xy = ((r2.xyxx)+(r9.zwzz)).xy;
    // 233: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 234: mul r2.xyw, r0.zzzz, r9.xyxz
    r2.xyw = ((r0.zzzz)*(r9.xyxz)).xyw;
    // 235: mul r0.z, r1.w, r9.w
    r0.z = ((r1.wwww)*(r9.wwww)).z;
    // 236: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mad r2.xyw, r2.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxz
    r2.xyw = ((r2.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxz))).xyw;
    // 238: mad r1.xyz, r0.zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 239: add r9.xyzw, v7.yzxy, cb0[0].yzxy
    r9.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 240: add r9.xyzw, r9.xyzw, -cb0[1].yzxy
    r9.xyzw = ((r9.xyzw)+(-(source[1].yzxy))).xyzw;
    // 241: add r2.xy, -r9.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r9.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 242: add r2.xy, -r9.zwzz, r2.xyxx
    r2.xy = ((-(r9.zwzz))+(r2.xyxx)).xy;
    // 243: mad r2.xy, cb0[18].wwww, r2.xyxx, r9.zwzz
    r2.xy = ((source[18].wwww)*(r2.xyxx)+(r9.zwzz)).xy;
    // 244: mul r0.z, cb0[18].y, cb0[28].x
    r0.z = ((source[18].yyyy)*(source[28].xxxx)).z;
    // 245: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 246: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 247: mul r3.y, r0.z, l(0.020000)
    r3.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 248: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 249: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 250: mul r2.w, cb0[18].x, l(0.001000)
    r2.w = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 251: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 252: mad r2.xy, r2.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 253: dp2 r2.w, cb0[19].xyxx, r2.xyxx
    r2.w = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 254: dp2 r2.y, cb0[20].xyxx, r2.xyxx
    r2.y = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 255: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 256: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 257: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 258: mad r2.xyw, r9.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r9.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 259: mul r3.x, r9.w, l(0.900000)
    r3.x = ((r9.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 260: mad r2.xyw, r3.xxxx, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.xxxx)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 261: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 262: mad r3.xyw, cb0[18].zzzz, r2.xyxw, -r1.xyxz
    r3.xyw = ((source[18].zzzz)*(r2.xyxw)+(-(r1.xyxz))).xyw;
    // 263: mul r2.xyw, r2.xyxw, cb0[18].zzzz
    r2.xyw = ((r2.xyxw)*(source[18].zzzz)).xyw;
    // 264: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 265: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 266: mad r1.xyz, r0.zzzz, r3.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xywx)+(r1.xyzx)).xyz;
    // 267: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 268: add r0.z, -r3.z, r5.w
    r0.z = ((-(r3.zzzz))+(r5.wwww)).z;
    // 269: mad r2.xyw, r5.wwww, cb0[15].xyxz, -cb0[15].xyxz
    r2.xyw = ((r5.wwww)*(source[15].xyxz)+(-(source[15].xyxz))).xyw;
    // 270: mad r2.xyw, cb0[15].wwww, r2.xyxw, cb0[15].xyxz
    r2.xyw = ((source[15].wwww)*(r2.xyxw)+(source[15].xyxz)).xyw;
    // 271: mad r0.z, cb0[14].w, r0.z, r3.z
    r0.z = ((source[14].wwww)*(r0.zzzz)+(r3.zzzz)).z;
    // 272: mad r2.xyw, r0.zzzz, cb0[14].xyxz, r2.xyxw
    r2.xyw = ((r0.zzzz)*(source[14].xyxz)+(r2.xyxw)).xyw;
    // 273: add r0.z, cb0[2].y, cb0[2].x
    r0.z = ((source[2].yyyy)+(source[2].xxxx)).z;
    // 274: add r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)+(source[2].zzzz)).z;
    // 275: add r3.x, -r0.z, l(1000.000000)
    r3.x = ((-(r0.zzzz))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 276: mad r0.z, cb0[28].y, r3.x, r0.z
    r0.z = ((source[28].yyyy)*(r3.xxxx)+(r0.zzzz)).z;
    // 277: mul r0.z, r0.z, l(0.010000)
    r0.z = ((r0.zzzz)*(float4(0.010000,0.010000,0.010000,0.010000))).z;
    // 278: mad r0.z, cb0[27].w, cb0[28].x, r0.z
    r0.z = ((source[27].wwww)*(source[28].xxxx)+(r0.zzzz)).z;
    // 279: mul r3.x, r0.z, l(3.524534)
    r3.x = ((r0.zzzz)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 280: sincos null, r3.x, r3.x
    r3.x = (cos(r3.xxxx)).x;
    // 281: add r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)+(r3.xxxx)).z;
    // 282: mul r0.z, r0.z, l(1.328987)
    r0.z = ((r0.zzzz)*(float4(1.328987,1.328987,1.328987,1.328987))).z;
    // 283: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 284: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 285: mad r0.z, r0.z, l(0.500000), cb0[27].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[27].zzzz)).z;
    // 286: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t7.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 287: mul r5.xyz, cb0[13].xyzx, cb0[27].yyyy
    r5.xyz = ((source[13].xyzx)*(source[27].yyyy)).xyz;
    // 288: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 289: mul r3.xyz, r0.zzzz, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)).xyz;
    // 290: mad r0.xyz, r1.wwww, r0.xywx, r3.xyzx
    r0.xyz = ((r1.wwww)*(r0.xywx)+(r3.xyzx)).xyz;
    // 291: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 292: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 293: mad r0.xyz, cb0[23].zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 294: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 295: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 296: mad r0.xyz, cb0[23].wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((source[23].wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 297: mad r0.xyz, r0.xyzx, r12.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)+(r2.xywx)).xyz;
    // 298: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 299: lt r1.w, |r2.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 300: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 301: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 302: mul r2.xyz, r0.wwww, cb0[16].xyzx
    r2.xyz = ((r0.wwww)*(source[16].xyzx)).xyz;
    // 303: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 304: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 305: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 306: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 307: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 308: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
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
    // 315: mul r3.yzw, r3.yyyy, cb0[30].xxyz
    r3.yzw = ((r3.yyyy)*(source[30].xxyz)).yzw;
    // 316: mad r3.xyz, r3.xxxx, cb0[29].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[29].xyzx)+(r3.yzwy)).xyz;
    // 317: mul r3.xyz, r3.xyzx, cb0[31].wwww
    r3.xyz = ((r3.xyzx)*(source[31].wwww)).xyz;
    // 318: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 319: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 320: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 321: mad o0.xyz, r1.xyzx, cb0[31].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[31].xyzx)+(r0.xyzx)).xyz;
    // 322: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 323: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 324: dp3 r0.x, r7.xyzx, r2.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 325: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
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

// source.character.equipment-native-164.v1 / source program 0b3be796bcf5b144a1761c47690de70a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase164(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[25].y=(g_SourceCharacterTime.xxxx).x;
    source[25].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[25].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 33: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 34: add r1.w, -cb0[21].y, cb0[21].x
    r1.w = ((-(source[21].yyyy))+(source[21].xxxx)).w;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mad r1.w, r3.w, r1.w, cb0[21].y
    r1.w = ((r3.wwww)*(r1.wwww)+(source[21].yyyy)).w;
    // 37: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 38: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 39: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 40: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 41: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 42: add r1.w, -r0.w, cb0[22].y
    r1.w = ((-(r0.wwww))+(source[22].yyyy)).w;
    // 43: mad r0.w, r3.w, r1.w, r0.w
    r0.w = ((r3.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 44: mul r0.w, r0.w, cb0[22].z
    r0.w = ((r0.wwww)*(source[22].zzzz)).w;
    // 45: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 46: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 47: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 48: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 50: mad r4.xyzw, r0.ywyw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r0.ywyw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 51: dp2 r0.y, r4.zwzz, r4.zwzz
    r0.y = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).y;
    // 52: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 54: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 55: add r5.z, r0.y, l(0.000010)
    r5.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 56: mul r5.xy, r4.xyxx, cb0[20].xxxx
    r5.xy = ((r4.xyxx)*(source[20].xxxx)).xy;
    // 57: mad r4.xy, cb0[20].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[20].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 58: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 59: mad r4.xyz, r3.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 60: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: mad r5.xyz, cb0[22].xxxx, r5.xyzx, r4.xyzx
    r5.xyz = ((source[22].xxxx)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 63: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 64: div r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)/(r0.yyyy)).xyz;
    // 65: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 68: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r7.xyz, r0.yyyy, v0.xyzx
    r7.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 71: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 72: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 73: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 74: dp3 r9.y, r8.xyzx, r5.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 75: dp3 r9.x, r7.xyzx, r5.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 76: dp3 r9.z, r6.xyzx, r5.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 77: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 78: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 79: mul r5.xyz, r0.yyyy, v5.xyzx
    r5.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 80: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 81: dp3 r11.y, r8.xyzx, r5.xyzx
    r11.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 82: dp3 r11.x, r7.xyzx, r5.xyzx
    r11.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 83: dp3 r11.z, r6.xyzx, r5.xyzx
    r11.z = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 84: dp3 r0.y, r9.xyzx, r11.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 85: mul r9.xyz, r9.xyzx, r0.yyyy
    r9.xyz = ((r9.xyzx)*(r0.yyyy)).xyz;
    // 86: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 87: mov r9.w, -r9.x
    r9.w = (-(r9.xxxx)).w;
    // 88: dp2 r0.y, r9.ywyy, r9.ywyy
    r0.y = (dot((r9.ywyy).xy,(r9.ywyy).xy).xxxx).y;
    // 89: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 90: div r0.yw, r9.yyyw, r0.yyyy
    r0.yw = ((r9.yyyw)/(r0.yyyy)).yw;
    // 91: mad r1.w, -r9.z, l(0.250000), l(0.250000)
    r1.w = ((-(r9.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 92: add r2.w, r9.z, l(1.000000)
    r2.w = ((r9.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 94: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 95: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 96: log r9.xyz, r0.xywx
    r9.xyz = (log2(r0.xywx)).xyz;
    // 97: rcp r1.w, cb0[22].w
    r1.w = (1.0/(source[22].wwww)).w;
    // 98: mul r11.xyz, r9.xyzx, r1.wwww
    r11.xyz = ((r9.xyzx)*(r1.wwww)).xyz;
    // 99: mul r9.xyz, r9.xyzx, cb0[22].wwww
    r9.xyz = ((r9.xyzx)*(source[22].wwww)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 102: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 103: mad r9.xyz, r9.xyzx, cb0[22].wwww, r11.xyzx
    r9.xyz = ((r9.xyzx)*(source[22].wwww)+(r11.xyzx)).xyz;
    // 104: add r0.xyw, r0.xyxw, r9.xyxz
    r0.xyw = ((r0.xyxw)+(r9.xyxz)).xyw;
    // 105: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 106: add r1.w, cb0[22].w, l(1.000000)
    r1.w = ((source[22].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 108: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r9.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r9.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 110: mad r9.xyz, r2.wwww, r9.xyzx, cb0[10].xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(source[10].xyzx)).xyz;
    // 111: mul r0.xyw, r0.xxxx, r9.xyxz
    r0.xyw = ((r0.xxxx)*(r9.xyxz)).xyw;
    // 112: mul r0.xyw, r0.xyxw, cb0[23].xxxx
    r0.xyw = ((r0.xyxw)*(source[23].xxxx)).xyw;
    // 113: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r9.xyz, -r2.xyzx, r1.wwww
    r9.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 115: mad r2.yzw, cb0[21].zzzz, r9.xxyz, r2.xxyz
    r2.yzw = ((source[21].zzzz)*(r9.xxyz)+(r2.xxyz)).yzw;
    // 116: dp3 r1.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 117: add r9.xyz, -r2.yzwy, r1.wwww
    r9.xyz = ((-(r2.yzwy))+(r1.wwww)).xyz;
    // 118: mad r2.yzw, cb0[21].wwww, r9.xxyz, r2.yyzw
    r2.yzw = ((source[21].wwww)*(r9.xxyz)+(r2.yyzw)).yzw;
    // 119: dp3 r1.w, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 120: add r9.xyz, -r2.yzwy, r1.wwww
    r9.xyz = ((-(r2.yzwy))+(r1.wwww)).xyz;
    // 121: mul r9.xyz, r9.xyzx, cb0[23].yyyy
    r9.xyz = ((r9.xyzx)*(source[23].yyyy)).xyz;
    // 122: add r1.w, r3.y, r3.x
    r1.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 123: add r1.w, r3.z, r1.w
    r1.w = ((r3.zzzz)+(r1.wwww)).w;
    // 124: add_sat r1.w, r3.w, r1.w
    r1.w = (saturate((r3.wwww)+(r1.wwww))).w;
    // 125: mad r2.yzw, r1.wwww, r9.xxyz, r2.yyzw
    r2.yzw = ((r1.wwww)*(r9.xxyz)+(r2.yyzw)).yzw;
    // 126: add r9.xyz, -r2.yzwy, r2.xxxx
    r9.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 127: mad r2.xyz, r3.wwww, r9.xyzx, r2.yzwy
    r2.xyz = ((r3.wwww)*(r9.xyzx)+(r2.yzwy)).xyz;
    // 128: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 129: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 130: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 131: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 132: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 133: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 134: add r2.w, -cb0[24].z, cb0[24].y
    r2.w = ((-(source[24].zzzz))+(source[24].yyyy)).w;
    // 135: mad r2.w, r3.w, r2.w, cb0[24].z
    r2.w = ((r3.wwww)*(r2.wwww)+(source[24].zzzz)).w;
    // 136: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 137: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 138: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 141: div r2.w, cb0[24].w, r2.w
    r2.w = ((source[24].wwww)/(r2.wwww)).w;
    // 142: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 143: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 144: div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // 145: dp3 r4.w, r4.xyzx, r5.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 146: mul_sat r5.w, r4.w, cb0[23].z
    r5.w = (saturate((r4.wwww)*(source[23].zzzz))).w;
    // 147: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mul_sat r6.w, r5.z, cb0[23].z
    r6.w = (saturate((r5.zzzz)*(source[23].zzzz))).w;
    // 150: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: add_sat r6.w, r6.w, -cb0[23].w
    r6.w = (saturate((r6.wwww)+(-(source[23].wwww)))).w;
    // 152: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 153: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 154: mul r7.w, r7.w, cb0[24].x
    r7.w = ((r7.wwww)*(source[24].xxxx)).w;
    // 155: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 156: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 157: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 158: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 159: mul r9.xyz, r0.xywx, r2.wwww
    r9.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 160: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 162: mad r1.xyz, cb0[21].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[21].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 163: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 165: mad r1.xyz, cb0[21].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[21].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 166: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 167: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 168: mad r11.xyz, r3.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 169: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 170: mad r11.xyz, r3.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 171: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 173: mad r11.xyz, cb0[21].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[21].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 174: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 176: mad r11.xyz, cb0[21].wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((source[21].wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 177: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 180: mul r13.xyz, r11.xyzx, r12.xyzx
    r13.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 181: mad r11.xyz, -r11.xyzx, r12.xyzx, cb0[9].xyzx
    r11.xyz = ((-(r11.xyzx))*(r12.xyzx)+(source[9].xyzx)).xyz;
    // 182: mad r3.xyw, r3.wwww, r11.xyxz, r13.xyxz
    r3.xyw = ((r3.wwww)*(r11.xyxz)+(r13.xyxz)).xyw;
    // 183: mul r1.xyz, r1.xyzx, r3.xywx
    r1.xyz = ((r1.xyzx)*(r3.xywx)).xyz;
    // 184: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 185: mad r2.xyz, r2.xyzx, r9.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r0.xywx))).xyz;
    // 186: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 187: mul r2.w, r2.w, cb0[25].x
    r2.w = ((r2.wwww)*(source[25].xxxx)).w;
    // 188: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 189: dp3 r2.x, r10.xyzx, r10.xyzx
    r2.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 190: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 191: div r2.yzw, r10.xxyz, r2.yyyy
    r2.yzw = ((r10.xxyz)/(r2.yyyy)).yzw;
    // 192: dp3 r2.y, r2.yzwy, r5.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // 193: add r2.z, -|r5.z|, l(1.000000)
    r2.z = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 194: mul r2.z, r4.w, r2.z
    r2.z = ((r4.wwww)*(r2.zzzz)).z;
    // 195: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 196: mul r2.w, |r2.y|, |r2.y|
    r2.w = ((abs(r2.yyyy))*(abs(r2.yyyy))).w;
    // 197: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 198: mul r2.w, r2.w, |r2.y|
    r2.w = ((r2.wwww)*(abs(r2.yyyy))).w;
    // 199: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 200: movc r2.y, r2.y, l(0), r2.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 201: add r2.w, r2.y, l(-0.027778)
    r2.w = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 202: mad r2.y, r2.y, r2.w, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 203: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 204: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 206: mad r2.xyw, r0.zzzz, r0.xyxw, -r1.xyxz
    r2.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r1.xyxz))).xyw;
    // 207: mad r1.xyz, r1.wwww, r2.xywx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 208: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 209: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 210: mad r1.xyz, cb0[21].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[21].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 211: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 212: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 213: mad r1.xyz, cb0[21].wwww, r2.xywx, r1.xyzx
    r1.xyz = ((source[21].wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 214: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 215: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 216: mul r0.z, r0.z, cb0[25].y
    r0.z = ((r0.zzzz)*(source[25].yyyy)).z;
    // 217: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 218: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 219: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 220: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 221: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 222: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 223: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 224: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 225: mul r9.y, cb0[3].y, cb0[15].y
    r9.y = ((source[3].yyyy)*(source[15].yyyy)).y;
    // 226: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 227: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 228: add r2.xy, r2.xyxx, r9.xyxx
    r2.xy = ((r2.xyxx)+(r9.xyxx)).xy;
    // 229: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 230: add r2.w, -r1.w, cb0[3].x
    r2.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 231: mul r9.z, r2.w, l(0.125000)
    r9.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 232: add r2.xy, r2.xyxx, r9.zwzz
    r2.xy = ((r2.xyxx)+(r9.zwzz)).xy;
    // 233: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 234: mul r2.xyw, r0.zzzz, r9.xyxz
    r2.xyw = ((r0.zzzz)*(r9.xyxz)).xyw;
    // 235: mul r0.z, r1.w, r9.w
    r0.z = ((r1.wwww)*(r9.wwww)).z;
    // 236: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mad r2.xyw, r2.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxz
    r2.xyw = ((r2.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxz))).xyw;
    // 238: mad r1.xyz, r0.zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 239: add r9.xyzw, v7.yzxy, cb0[0].yzxy
    r9.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 240: add r9.xyzw, r9.xyzw, -cb0[1].yzxy
    r9.xyzw = ((r9.xyzw)+(-(source[1].yzxy))).xyzw;
    // 241: add r2.xy, -r9.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r9.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 242: add r2.xy, -r9.zwzz, r2.xyxx
    r2.xy = ((-(r9.zwzz))+(r2.xyxx)).xy;
    // 243: mad r2.xy, cb0[16].wwww, r2.xyxx, r9.zwzz
    r2.xy = ((source[16].wwww)*(r2.xyxx)+(r9.zwzz)).xy;
    // 244: mul r0.z, cb0[16].y, cb0[25].y
    r0.z = ((source[16].yyyy)*(source[25].yyyy)).z;
    // 245: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 246: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 247: mul r3.y, r0.z, l(0.020000)
    r3.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 248: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 249: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 250: mul r2.w, cb0[16].x, l(0.001000)
    r2.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 251: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 252: mad r2.xy, r2.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 253: dp2 r2.w, cb0[17].xyxx, r2.xyxx
    r2.w = (dot((source[17].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 254: dp2 r2.y, cb0[18].xyxx, r2.xyxx
    r2.y = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 255: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 256: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 257: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 258: mad r2.xyw, r9.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r9.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 259: mul r3.x, r9.w, l(0.900000)
    r3.x = ((r9.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 260: mad r2.xyw, r3.xxxx, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.xxxx)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 261: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 262: mad r3.xyw, cb0[16].zzzz, r2.xyxw, -r1.xyxz
    r3.xyw = ((source[16].zzzz)*(r2.xyxw)+(-(r1.xyxz))).xyw;
    // 263: mul r2.xyw, r2.xyxw, cb0[16].zzzz
    r2.xyw = ((r2.xyxw)*(source[16].zzzz)).xyw;
    // 264: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 265: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 266: mad r1.xyz, r0.zzzz, r3.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xywx)+(r1.xyzx)).xyz;
    // 267: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 268: add r0.z, -r3.z, r5.w
    r0.z = ((-(r3.zzzz))+(r5.wwww)).z;
    // 269: mad r2.xyw, r5.wwww, cb0[13].xyxz, -cb0[13].xyxz
    r2.xyw = ((r5.wwww)*(source[13].xyxz)+(-(source[13].xyxz))).xyw;
    // 270: mad r2.xyw, cb0[13].wwww, r2.xyxw, cb0[13].xyxz
    r2.xyw = ((source[13].wwww)*(r2.xyxw)+(source[13].xyxz)).xyw;
    // 271: mad r0.z, cb0[12].w, r0.z, r3.z
    r0.z = ((source[12].wwww)*(r0.zzzz)+(r3.zzzz)).z;
    // 272: mad r2.xyw, r0.zzzz, cb0[12].xyxz, r2.xyxw
    r2.xyw = ((r0.zzzz)*(source[12].xyxz)+(r2.xyxw)).xyw;
    // 273: mul r3.xyz, r0.xywx, r1.wwww
    r3.xyz = ((r0.xywx)*(r1.wwww)).xyz;
    // 274: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 275: mad r0.xyz, -r1.wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(r1.wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 276: mad r0.xyz, cb0[21].zzzz, r0.xyzx, r3.xyzx
    r0.xyz = ((source[21].zzzz)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 277: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 278: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 279: mad r0.xyz, cb0[21].wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((source[21].wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 280: mad r0.xyz, r0.xyzx, r12.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)+(r2.xywx)).xyz;
    // 281: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 282: lt r1.w, |r2.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 283: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 284: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 285: mul r2.xyz, r0.wwww, cb0[14].xyzx
    r2.xyz = ((r0.wwww)*(source[14].xyzx)).xyz;
    // 286: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 287: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 288: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 289: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 290: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 291: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
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
    // 298: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 299: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 300: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 301: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 302: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 303: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 304: mad o0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 305: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 306: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 307: dp3 r0.x, r7.xyzx, r2.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 308: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
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

// source.character.equipment-native-165.v1 / source program f6829c9577cf2544b72fa2305b447f75
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase165(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].y=(g_SourceCharacterTime.xxxx).x;
    source[24].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[24].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
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
    // 18: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 19: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 20: add r1.w, -cb0[20].y, cb0[20].x
    r1.w = ((-(source[20].yyyy))+(source[20].xxxx)).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mad r1.w, r2.w, r1.w, cb0[20].y
    r1.w = ((r2.wwww)*(r1.wwww)+(source[20].yyyy)).w;
    // 23: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 24: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 25: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 26: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 27: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 28: add r1.w, -r0.w, cb0[21].y
    r1.w = ((-(r0.wwww))+(source[21].yyyy)).w;
    // 29: mad r0.w, r2.w, r1.w, r0.w
    r0.w = ((r2.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 30: mul r0.w, r0.w, cb0[21].z
    r0.w = ((r0.wwww)*(source[21].zzzz)).w;
    // 31: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 32: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 33: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 34: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 36: mad r3.xyzw, r0.ywyw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r3.xyzw = ((r0.ywyw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 37: dp2 r0.y, r3.zwzz, r3.zwzz
    r0.y = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).y;
    // 38: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 40: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 41: add r4.z, r0.y, l(0.000010)
    r4.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: mul r4.xy, r3.xyxx, cb0[19].xxxx
    r4.xy = ((r3.xyxx)*(source[19].xxxx)).xy;
    // 43: mad r3.xy, cb0[19].wwww, r3.zwzz, -r4.xyxx
    r3.xy = ((source[19].wwww)*(r3.zwzz)+(-(r4.xyxx))).xy;
    // 44: mov r3.z, l(0)
    r3.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 45: mad r3.xyz, r2.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 46: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 47: mad r4.xyz, cb0[21].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[21].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 49: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 50: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 51: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 52: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 53: mul r5.xyz, r0.yyyy, v1.xyzx
    r5.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 54: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 55: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 56: mul r6.xyz, r0.yyyy, v0.xyzx
    r6.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 57: mul r7.xyz, r5.zxyz, r6.yzxy
    r7.xyz = ((r5.zxyz)*(r6.yzxy)).xyz;
    // 58: mad r7.xyz, r5.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r5.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 59: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 60: dp3 r8.y, r7.xyzx, r4.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 61: dp3 r8.x, r6.xyzx, r4.xyzx
    r8.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 62: dp3 r8.z, r5.xyzx, r4.xyzx
    r8.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 63: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 64: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 65: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 66: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 67: dp3 r10.y, r7.xyzx, r4.xyzx
    r10.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 68: dp3 r10.x, r6.xyzx, r4.xyzx
    r10.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 69: dp3 r10.z, r5.xyzx, r4.xyzx
    r10.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 70: dp3 r0.y, r8.xyzx, r10.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 71: mul r8.xyz, r8.xyzx, r0.yyyy
    r8.xyz = ((r8.xyzx)*(r0.yyyy)).xyz;
    // 72: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 73: mov r8.w, -r8.x
    r8.w = (-(r8.xxxx)).w;
    // 74: dp2 r0.y, r8.ywyy, r8.ywyy
    r0.y = (dot((r8.ywyy).xy,(r8.ywyy).xy).xxxx).y;
    // 75: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 76: div r0.yw, r8.yyyw, r0.yyyy
    r0.yw = ((r8.yyyw)/(r0.yyyy)).yw;
    // 77: mad r1.w, -r8.z, l(0.250000), l(0.250000)
    r1.w = ((-(r8.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 78: add r3.w, r8.z, l(1.000000)
    r3.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 80: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 81: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t4.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 82: log r8.xyz, r0.xywx
    r8.xyz = (log2(r0.xywx)).xyz;
    // 83: rcp r1.w, cb0[21].w
    r1.w = (1.0/(source[21].wwww)).w;
    // 84: mul r10.xyz, r8.xyzx, r1.wwww
    r10.xyz = ((r8.xyzx)*(r1.wwww)).xyz;
    // 85: mul r8.xyz, r8.xyzx, cb0[21].wwww
    r8.xyz = ((r8.xyzx)*(source[21].wwww)).xyz;
    // 86: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 87: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 88: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 89: mad r8.xyz, r8.xyzx, cb0[21].wwww, r10.xyzx
    r8.xyz = ((r8.xyzx)*(source[21].wwww)+(r10.xyzx)).xyz;
    // 90: add r0.xyw, r0.xyxw, r8.xyxz
    r0.xyw = ((r0.xyxw)+(r8.xyxz)).xyw;
    // 91: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 92: add r1.w, cb0[21].w, l(1.000000)
    r1.w = ((source[21].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 94: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 95: add r8.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r8.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 96: mad r8.xyz, r3.wwww, r8.xyzx, cb0[10].xyzx
    r8.xyz = ((r3.wwww)*(r8.xyzx)+(source[10].xyzx)).xyz;
    // 97: mul r0.xyw, r0.xxxx, r8.xyxz
    r0.xyw = ((r0.xxxx)*(r8.xyxz)).xyw;
    // 98: mul r0.xyw, r0.xyxw, cb0[22].xxxx
    r0.xyw = ((r0.xyxw)*(source[22].xxxx)).xyw;
    // 99: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: add r8.xyz, -r1.xyzx, r1.wwww
    r8.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 101: mad r1.yzw, cb0[20].zzzz, r8.xxyz, r1.xxyz
    r1.yzw = ((source[20].zzzz)*(r8.xxyz)+(r1.xxyz)).yzw;
    // 102: dp3 r3.w, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r8.xyz, -r1.yzwy, r3.wwww
    r8.xyz = ((-(r1.yzwy))+(r3.wwww)).xyz;
    // 104: mad r1.yzw, cb0[20].wwww, r8.xxyz, r1.yyzw
    r1.yzw = ((source[20].wwww)*(r8.xxyz)+(r1.yyzw)).yzw;
    // 105: dp3 r3.w, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r8.xyz, -r1.yzwy, r3.wwww
    r8.xyz = ((-(r1.yzwy))+(r3.wwww)).xyz;
    // 107: mul r8.xyz, r8.xyzx, cb0[22].yyyy
    r8.xyz = ((r8.xyzx)*(source[22].yyyy)).xyz;
    // 108: add r3.w, r2.y, r2.x
    r3.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 109: add r3.w, r2.z, r3.w
    r3.w = ((r2.zzzz)+(r3.wwww)).w;
    // 110: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 111: mad r1.yzw, r3.wwww, r8.xxyz, r1.yyzw
    r1.yzw = ((r3.wwww)*(r8.xxyz)+(r1.yyzw)).yzw;
    // 112: add r8.xyz, -r1.yzwy, r1.xxxx
    r8.xyz = ((-(r1.yzwy))+(r1.xxxx)).xyz;
    // 113: mad r1.xyz, r2.wwww, r8.xyzx, r1.yzwy
    r1.xyz = ((r2.wwww)*(r8.xyzx)+(r1.yzwy)).xyz;
    // 114: max r8.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r8.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 115: log r8.xyz, r8.xyzx
    r8.xyz = (log2(r8.xyzx)).xyz;
    // 116: mul r8.xyz, r8.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 117: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 118: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 120: add r3.w, -cb0[23].z, cb0[23].y
    r3.w = ((-(source[23].zzzz))+(source[23].yyyy)).w;
    // 121: mad r3.w, r2.w, r3.w, cb0[23].z
    r3.w = ((r2.wwww)*(r3.wwww)+(source[23].zzzz)).w;
    // 122: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 123: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 124: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mad r3.w, -r1.w, r1.w, l(1.000000)
    r3.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: max r3.w, r3.w, l(0.001000)
    r3.w = (max(r3.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 127: div r3.w, cb0[23].w, r3.w
    r3.w = ((source[23].wwww)/(r3.wwww)).w;
    // 128: dp3 r4.w, r3.xyzx, r3.xyzx
    r4.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 129: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 130: div r3.xyz, r3.xyzx, r4.wwww
    r3.xyz = ((r3.xyzx)/(r4.wwww)).xyz;
    // 131: dp3 r4.w, r3.xyzx, r4.xyzx
    r4.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 132: mul_sat r5.w, r4.w, cb0[22].z
    r5.w = (saturate((r4.wwww)*(source[22].zzzz))).w;
    // 133: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul_sat r6.w, r4.z, cb0[22].z
    r6.w = (saturate((r4.zzzz)*(source[22].zzzz))).w;
    // 136: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: add_sat r6.w, r6.w, -cb0[22].w
    r6.w = (saturate((r6.wwww)+(-(source[22].wwww)))).w;
    // 138: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 139: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 140: mul r7.w, r7.w, cb0[23].x
    r7.w = ((r7.wwww)*(source[23].xxxx)).w;
    // 141: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 142: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 143: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 144: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 145: mul r8.xyz, r0.xywx, r3.wwww
    r8.xyz = ((r0.xywx)*(r3.wwww)).xyz;
    // 146: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 147: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r10.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r10.xyzx))).xyz;
    // 148: mad r10.xyz, r2.xxxx, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.xxxx)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 149: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, -r10.xyzx
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r10.xyzx))).xyz;
    // 150: mad r10.xyz, r2.yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 151: dp3 r2.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 152: add r11.xyz, -r10.xyzx, r2.xxxx
    r11.xyz = ((-(r10.xyzx))+(r2.xxxx)).xyz;
    // 153: mad r10.xyz, cb0[20].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 154: dp3 r2.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 155: add r11.xyz, -r10.xyzx, r2.xxxx
    r11.xyz = ((-(r10.xyzx))+(r2.xxxx)).xyz;
    // 156: mad r10.xyz, cb0[20].wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((source[20].wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 157: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 158: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 160: mul r12.xyz, r10.xyzx, r11.xyzx
    r12.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 161: mad r10.xyz, -r10.xyzx, r11.xyzx, cb0[9].xyzx
    r10.xyz = ((-(r10.xyzx))*(r11.xyzx)+(source[9].xyzx)).xyz;
    // 162: mad r2.xyw, r2.wwww, r10.xyxz, r12.xyxz
    r2.xyw = ((r2.wwww)*(r10.xyxz)+(r12.xyxz)).xyw;
    // 163: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 164: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 165: add r12.xyz, -r10.xyzx, r3.wwww
    r12.xyz = ((-(r10.xyzx))+(r3.wwww)).xyz;
    // 166: mad r10.xyz, cb0[20].zzzz, r12.xyzx, r10.xyzx
    r10.xyz = ((source[20].zzzz)*(r12.xyzx)+(r10.xyzx)).xyz;
    // 167: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 168: add r12.xyz, -r10.xyzx, r3.wwww
    r12.xyz = ((-(r10.xyzx))+(r3.wwww)).xyz;
    // 169: mad r10.xyz, cb0[20].wwww, r12.xyzx, r10.xyzx
    r10.xyz = ((source[20].wwww)*(r12.xyzx)+(r10.xyzx)).xyz;
    // 170: mul r2.xyw, r2.xyxw, r10.xyxz
    r2.xyw = ((r2.xyxw)*(r10.xyxz)).xyw;
    // 171: mul r0.xyw, r0.xyxw, r2.xyxw
    r0.xyw = ((r0.xyxw)*(r2.xyxw)).xyw;
    // 172: mad r1.xyz, r1.xyzx, r8.xyzx, -r0.xywx
    r1.xyz = ((r1.xyzx)*(r8.xyzx)+(-(r0.xywx))).xyz;
    // 173: add r3.w, -r1.w, l(1.000000)
    r3.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: mul r3.w, r3.w, cb0[24].x
    r3.w = ((r3.wwww)*(source[24].xxxx)).w;
    // 175: mad r0.xyw, r3.wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((r3.wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 176: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 177: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 178: div r8.xyz, r9.xyzx, r1.yyyy
    r8.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 179: dp3 r1.y, r8.xyzx, r4.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 180: add r1.z, -|r4.z|, l(1.000000)
    r1.z = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 181: mul r1.z, r4.w, r1.z
    r1.z = ((r4.wwww)*(r1.zzzz)).z;
    // 182: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 183: mul r3.w, |r1.y|, |r1.y|
    r3.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 184: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 185: mul r3.w, |r1.y|, r3.w
    r3.w = ((abs(r1.yyyy))*(r3.wwww)).w;
    // 186: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 187: movc r1.y, r1.y, l(0), r3.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).y;
    // 188: add r3.w, r1.y, l(-0.027778)
    r3.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 189: mad r1.y, r1.y, r3.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r3.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 190: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 191: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 192: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 193: mad r4.xyz, r0.zzzz, r0.xywx, -r2.xywx
    r4.xyz = ((r0.zzzz)*(r0.xywx)+(-(r2.xywx))).xyz;
    // 194: mad r1.xyw, r1.wwww, r4.xyxz, r2.xyxw
    r1.xyw = ((r1.wwww)*(r4.xyxz)+(r2.xyxw)).xyw;
    // 195: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r2.xyw, -r1.xyxw, r0.zzzz
    r2.xyw = ((-(r1.xyxw))+(r0.zzzz)).xyw;
    // 197: mad r1.xyw, cb0[20].zzzz, r2.xyxw, r1.xyxw
    r1.xyw = ((source[20].zzzz)*(r2.xyxw)+(r1.xyxw)).xyw;
    // 198: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 199: add r2.xyw, -r1.xyxw, r0.zzzz
    r2.xyw = ((-(r1.xyxw))+(r0.zzzz)).xyw;
    // 200: mad r1.xyw, cb0[20].wwww, r2.xyxw, r1.xyxw
    r1.xyw = ((source[20].wwww)*(r2.xyxw)+(r1.xyxw)).xyw;
    // 201: mul r1.xyw, r11.xyxz, r1.xyxw
    r1.xyw = ((r11.xyxz)*(r1.xyxw)).xyw;
    // 202: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 203: mul r0.z, r0.z, cb0[24].y
    r0.z = ((r0.zzzz)*(source[24].yyyy)).z;
    // 204: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 205: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 206: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: mul r2.x, cb0[3].z, l(1.500000)
    r2.x = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 208: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 209: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 210: frc r2.x, v4.x
    r2.x = (frac(v4.xxxx)).x;
    // 211: mul r2.x, r2.x, l(0.125000)
    r2.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 212: mul r4.y, cb0[3].y, cb0[15].y
    r4.y = ((source[3].yyyy)*(source[15].yyyy)).y;
    // 213: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 214: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 215: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 216: frc r2.w, cb0[3].x
    r2.w = (frac(source[3].xxxx)).w;
    // 217: add r3.w, -r2.w, cb0[3].x
    r3.w = ((-(r2.wwww))+(source[3].xxxx)).w;
    // 218: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 219: add r2.xy, r2.xyxx, r4.zwzz
    r2.xy = ((r2.xyxx)+(r4.zwzz)).xy;
    // 220: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 221: mul r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = ((r0.zzzz)*(r4.xyzx)).xyz;
    // 222: mul r0.z, r2.w, r4.w
    r0.z = ((r2.wwww)*(r4.wwww)).z;
    // 223: add r2.x, -r2.w, l(1.000000)
    r2.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xywx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xywx))).xyz;
    // 225: mad r1.xyw, r0.zzzz, r4.xyxz, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r4.xyxz)+(r1.xyxw)).xyw;
    // 226: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 227: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 228: add r2.yw, -r4.xxxy, l(0.000000, 1.000000, 0.000000, 1.000000)
    r2.yw = ((-(r4.xxxy))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 229: add r2.yw, -r4.zzzw, r2.yyyw
    r2.yw = ((-(r4.zzzw))+(r2.yyyw)).yw;
    // 230: mad r2.yw, cb0[16].wwww, r2.yyyw, r4.zzzw
    r2.yw = ((source[16].wwww)*(r2.yyyw)+(r4.zzzw)).yw;
    // 231: mul r0.z, cb0[16].y, cb0[24].y
    r0.z = ((source[16].yyyy)*(source[24].yyyy)).z;
    // 232: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 233: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 234: mul r4.y, r0.z, l(0.020000)
    r4.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 235: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 236: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 237: mul r3.w, cb0[16].x, l(0.001000)
    r3.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 238: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 239: mad r2.yw, r3.wwww, r2.yyyw, r4.xxxy
    r2.yw = ((r3.wwww)*(r2.yyyw)+(r4.xxxy)).yw;
    // 240: dp2 r3.w, cb0[17].xyxx, r2.ywyy
    r3.w = (dot((source[17].xyxx).xy,(r2.ywyy).xy).xxxx).w;
    // 241: dp2 r4.y, cb0[18].xyxx, r2.ywyy
    r4.y = (dot((source[18].xyxx).xy,(r2.ywyy).xy).xxxx).y;
    // 242: frc r2.y, r3.w
    r2.y = (frac(r3.wwww)).y;
    // 243: mul r4.x, r2.y, l(0.125000)
    r4.x = ((r2.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 244: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 245: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xywx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xywx))).xyz;
    // 246: mul r2.y, r4.w, l(0.900000)
    r2.y = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 247: mad r4.xyz, r2.yyyy, r4.xyzx, r1.xywx
    r4.xyz = ((r2.yyyy)*(r4.xyzx)+(r1.xywx)).xyz;
    // 248: mul_sat r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = (saturate((r0.zzzz)*(r4.xyzx))).xyz;
    // 249: mad r8.xyz, cb0[16].zzzz, r4.xyzx, -r1.xywx
    r8.xyz = ((source[16].zzzz)*(r4.xyzx)+(-(r1.xywx))).xyz;
    // 250: mul r4.xyz, r4.xyzx, cb0[16].zzzz
    r4.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 251: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 252: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 253: mad r1.xyw, r0.zzzz, r8.xyxz, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r8.xyxz)+(r1.xyxw)).xyw;
    // 254: mad r1.xyw, r1.xyxw, cb2[3].wwww, cb2[3].xyxz
    r1.xyw = ((r1.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 255: add r0.z, -r2.z, r5.w
    r0.z = ((-(r2.zzzz))+(r5.wwww)).z;
    // 256: mad r4.xyz, r5.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r4.xyz = ((r5.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 257: mad r4.xyz, cb0[13].wwww, r4.xyzx, cb0[13].xyzx
    r4.xyz = ((source[13].wwww)*(r4.xyzx)+(source[13].xyzx)).xyz;
    // 258: mad r0.z, cb0[12].w, r0.z, r2.z
    r0.z = ((source[12].wwww)*(r0.zzzz)+(r2.zzzz)).z;
    // 259: mad r2.yzw, r0.zzzz, cb0[12].xxyz, r4.xxyz
    r2.yzw = ((r0.zzzz)*(source[12].xxyz)+(r4.xxyz)).yzw;
    // 260: mul r4.xyz, r0.xywx, r2.xxxx
    r4.xyz = ((r0.xywx)*(r2.xxxx)).xyz;
    // 261: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 262: mad r0.xyz, -r2.xxxx, r0.xywx, r0.zzzz
    r0.xyz = ((-(r2.xxxx))*(r0.xywx)+(r0.zzzz)).xyz;
    // 263: mad r0.xyz, cb0[20].zzzz, r0.xyzx, r4.xyzx
    r0.xyz = ((source[20].zzzz)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 264: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 265: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 266: mad r0.xyz, cb0[20].wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((source[20].wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 267: mad r0.xyz, r0.xyzx, r11.xyzx, r2.yzwy
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 268: log r0.w, |r1.z|
    r0.w = (log2(abs(r1.zzzz))).w;
    // 269: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 270: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 271: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 272: mul r2.xyz, r0.wwww, cb0[14].xyzx
    r2.xyz = ((r0.wwww)*(source[14].xyzx)).xyz;
    // 273: movc r2.xyz, r1.zzzz, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 274: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 275: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 276: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 277: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 278: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 279: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 280: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 281: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 282: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 283: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 284: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 285: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 286: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 287: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 288: mad r0.xyz, r3.xyzx, r1.xywx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xywx)+(r0.xyzx)).xyz;
    // 289: mul r3.xyz, r1.xywx, r3.xyzx
    r3.xyz = ((r1.xywx)*(r3.xyzx)).xyz;
    // 290: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 291: mad o0.xyz, r1.xywx, cb0[27].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xywx)*(source[27].xyzx)+(r0.xyzx)).xyz;
    // 292: mov o3.xyz, r1.xywx
    output.targets[3].xyz = (r1.xywx).xyz;
    // 293: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 294: dp3 r0.x, r6.xyzx, r2.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 295: dp3 r0.z, r5.xyzx, r2.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 296: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 297: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 298: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 299: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 300: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 301: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 302: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 303: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 304: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 305: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 306: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 307: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 308: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 309: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 310: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 311: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 312: ret
    return output;
}

// source.character.equipment-native-166.v1 / source program 670709783f72e643877ddd992ceb8285
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase166(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
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
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: mad r4.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r3.xyw, r4.xyxz, r0.wwww
    r3.xyw = ((r4.xyxz)/(r0.wwww)).xyw;
    // 18: dp3 r0.w, r3.xywx, r3.xywx
    r0.w = (dot((r3.xywx).xyz,(r3.xywx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r4.yzw, r0.wwww, r3.xxyw
    r4.yzw = ((r0.wwww)*(r3.xxyw)).yzw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 23: dp3 r0.w, r6.xyzx, cb0[12].xyzx
    r0.w = (dot((r6.xyzx).xyz,(source[12].xyzx).xyz).xxxx).w;
    // 24: dp3_sat r1.w, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[12].xyzx
    r1.w = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[12].xyzx).xyz).xxxx)).w;
    // 25: add r0.w, r0.w, -r1.w
    r0.w = ((r0.wwww)+(-(r1.wwww))).w;
    // 26: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: mul r0.w, r0.w, r5.w
    r0.w = ((r0.wwww)*(r5.wwww)).w;
    // 28: mul r1.w, r0.w, cb0[1].w
    r1.w = ((r0.wwww)*(source[1].wwww)).w;
    // 29: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 30: mul r6.xyz, r6.xyzx, cb0[18].xxxx
    r6.xyz = ((r6.xyzx)*(source[18].xxxx)).xyz;
    // 31: mad r2.w, cb0[18].y, l(-3.500000), l(5.000000)
    r2.w = ((source[18].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 32: mul r2.w, r2.w, cb0[19].x
    r2.w = ((r2.wwww)*(source[19].xxxx)).w;
    // 33: add r5.w, -v4.z, l(1.000000)
    r5.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: add r6.w, -r5.w, v4.z
    r6.w = ((-(r5.wwww))+(v4.zzzz)).w;
    // 35: mad r5.w, cb0[19].y, r6.w, r5.w
    r5.w = ((source[19].yyyy)*(r6.wwww)+(r5.wwww)).w;
    // 36: mul r6.w, r5.w, cb0[19].z
    r6.w = ((r5.wwww)*(source[19].zzzz)).w;
    // 37: mad r5.w, r6.w, l(0.750000), r5.w
    r5.w = ((r6.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r5.wwww)).w;
    // 38: mul r5.w, r5.w, l(0.500000)
    r5.w = ((r5.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 39: mad_sat r5.w, cb0[20].x, r5.w, r5.w
    r5.w = (saturate((source[20].xxxx)*(r5.wwww)+(r5.wwww))).w;
    // 40: mad r4.x, r4.x, r5.x, l(0.200000)
    r4.x = ((r4.xxxx)*(r5.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 41: add r6.w, -r5.y, l(1.000000)
    r6.w = ((-(r5.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: add r6.w, -r4.x, r6.w
    r6.w = ((-(r4.xxxx))+(r6.wwww)).w;
    // 43: mad r7.x, cb0[20].z, r6.w, r4.x
    r7.x = ((source[20].zzzz)*(r6.wwww)+(r4.xxxx)).x;
    // 44: mul r7.y, cb0[19].w, l(0.700000)
    r7.y = ((source[19].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).y;
    // 45: add r7.x, -r5.w, r7.x
    r7.x = ((-(r5.wwww))+(r7.xxxx)).x;
    // 46: mad r7.x, r7.y, r7.x, r5.w
    r7.x = ((r7.yyyy)*(r7.xxxx)+(r5.wwww)).x;
    // 47: div r7.x, r7.x, cb0[20].y
    r7.x = ((r7.xxxx)/(source[20].yyyy)).x;
    // 48: add r7.x, -r7.x, l(1.000000)
    r7.x = ((-(r7.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 49: mul r7.x, r2.w, r7.x
    r7.x = ((r2.wwww)*(r7.xxxx)).x;
    // 50: mul r7.x, r7.x, l(4.000000)
    r7.x = ((r7.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 51: add r7.z, v4.w, l(0.500000)
    r7.z = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 52: round_ni r7.z, r7.z
    r7.z = (floor(r7.zzzz)).z;
    // 53: mul_sat r7.x, r7.z, r7.x
    r7.x = (saturate((r7.zzzz)*(r7.xxxx))).x;
    // 54: mad r4.x, cb0[21].x, r6.w, r4.x
    r4.x = ((source[21].xxxx)*(r6.wwww)+(r4.xxxx)).x;
    // 55: add r4.x, -r5.w, r4.x
    r4.x = ((-(r5.wwww))+(r4.xxxx)).x;
    // 56: mad r4.x, r7.y, r4.x, r5.w
    r4.x = ((r7.yyyy)*(r4.xxxx)+(r5.wwww)).x;
    // 57: div r4.x, r4.x, cb0[20].w
    r4.x = ((r4.xxxx)/(source[20].wwww)).x;
    // 58: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: mul r2.w, r2.w, r4.x
    r2.w = ((r2.wwww)*(r4.xxxx)).w;
    // 60: mul r2.w, r2.w, l(4.000000)
    r2.w = ((r2.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 61: add r4.x, -r7.z, l(1.000000)
    r4.x = ((-(r7.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 62: mul_sat r2.w, r2.w, r4.x
    r2.w = (saturate((r2.wwww)*(r4.xxxx))).w;
    // 63: add r2.w, r2.w, r7.x
    r2.w = ((r2.wwww)+(r7.xxxx)).w;
    // 64: add r2.w, -r5.y, r2.w
    r2.w = ((-(r5.yyyy))+(r2.wwww)).w;
    // 65: mad r2.w, cb0[21].y, r2.w, r5.y
    r2.w = ((source[21].yyyy)*(r2.wwww)+(r5.yyyy)).w;
    // 66: mad r6.xyz, r2.wwww, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 67: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 68: add r7.xyz, -r6.xyzx, r2.wwww
    r7.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 69: mad r6.xyz, cb0[21].zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((source[21].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 70: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: add r7.xyz, -r6.xyzx, r2.wwww
    r7.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 72: mad r6.xyz, cb0[21].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[21].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 73: mad r7.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: mad r8.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 76: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 77: mul r9.xyz, r5.xxxx, r8.xyzx
    r9.xyz = ((r5.xxxx)*(r8.xyzx)).xyz;
    // 78: lt r2.w, |r3.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r3.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 79: mul r3.z, |r3.z|, |r3.z|
    r3.z = ((abs(r3.zzzz))*(abs(r3.zzzz))).z;
    // 80: movc r2.w, r2.w, l(0), r3.z
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).w;
    // 81: mad r10.xyz, cb0[7].xyzx, r5.xyzx, -r5.xyzx
    r10.xyz = ((source[7].xyzx)*(r5.xyzx)+(-(r5.xyzx))).xyz;
    // 82: mad r10.xyz, r2.wwww, r10.xyzx, r5.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)+(r5.xyzx)).xyz;
    // 83: mad r5.xyw, -r5.xxxx, r8.xyxz, r10.xyxz
    r5.xyw = ((-(r5.xxxx))*(r8.xyxz)+(r10.xyxz)).xyw;
    // 84: mad r5.xyw, r2.wwww, r5.xyxw, r9.xyxz
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(r9.xyxz)).xyw;
    // 85: mul r3.z, cb0[13].z, l(1.500000)
    r3.z = ((source[13].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 86: add r4.x, -cb0[13].w, l(1.000000)
    r4.x = ((-(source[13].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 87: mul r4.x, r4.x, cb0[24].w
    r4.x = ((r4.xxxx)*(source[24].wwww)).x;
    // 88: mul r4.x, r4.x, l(6.283185)
    r4.x = ((r4.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 89: sincos r4.x, null, r4.x
    r4.x = (sin(r4.xxxx)).x;
    // 90: add r4.x, r4.x, l(1.000000)
    r4.x = ((r4.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: mul r3.z, r3.z, r4.x
    r3.z = ((r3.zzzz)*(r4.xxxx)).z;
    // 92: mad r3.z, r3.z, l(0.500000), cb0[13].z
    r3.z = ((r3.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[13].zzzz)).z;
    // 93: frc r4.x, cb0[13].x
    r4.x = (frac(source[13].xxxx)).x;
    // 94: add r6.w, -r4.x, cb0[13].x
    r6.w = ((-(r4.xxxx))+(source[13].xxxx)).w;
    // 95: mul r8.z, r6.w, l(0.125000)
    r8.z = ((r6.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 96: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 97: mul r8.y, cb0[13].y, cb0[14].y
    r8.y = ((source[13].yyyy)*(source[14].yyyy)).y;
    // 98: mul r9.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r9.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 99: frc r6.w, r9.x
    r6.w = (frac(r9.xxxx)).w;
    // 100: mul r9.y, r6.w, l(0.125000)
    r9.y = ((r6.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 101: add r8.xy, r8.xyxx, r9.yzyy
    r8.xy = ((r8.xyxx)+(r9.yzyy)).xy;
    // 102: add r8.xy, r8.xyxx, r8.zwzz
    r8.xy = ((r8.xyxx)+(r8.zwzz)).xy;
    // 103: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r8.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 104: mul r8.xyz, r3.zzzz, r8.xyzx
    r8.xyz = ((r3.zzzz)*(r8.xyzx)).xyz;
    // 105: mul r3.z, r4.x, r8.w
    r3.z = ((r4.xxxx)*(r8.wwww)).z;
    // 106: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xywx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xywx))).xyz;
    // 107: mad r5.xyw, r3.zzzz, r8.xyxz, r5.xyxw
    r5.xyw = ((r3.zzzz)*(r8.xyxz)+(r5.xyxw)).xyw;
    // 108: mul r3.z, cb0[15].y, cb0[24].w
    r3.z = ((source[15].yyyy)*(source[24].wwww)).z;
    // 109: mul r3.z, r3.z, l(0.628319)
    r3.z = ((r3.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 110: sincos r3.z, null, r3.z
    r3.z = (sin(r3.zzzz)).z;
    // 111: mul r8.y, r3.z, l(0.020000)
    r8.y = ((r3.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 112: add r9.xyzw, r1.yzxy, -cb0[1].yzxy
    r9.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 113: add r8.zw, -r9.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r8.zw = ((-(r9.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 114: add r8.zw, -r9.zzzw, r8.zzzw
    r8.zw = ((-(r9.zzzw))+(r8.zzzw)).zw;
    // 115: mad r8.zw, cb0[15].wwww, r8.zzzw, r9.zzzw
    r8.zw = ((source[15].wwww)*(r8.zzzw)+(r9.zzzw)).zw;
    // 116: mul r4.x, cb0[15].x, l(0.001000)
    r4.x = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 117: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 118: mad r8.xy, r4.xxxx, r8.zwzz, r8.xyxx
    r8.xy = ((r4.xxxx)*(r8.zwzz)+(r8.xyxx)).xy;
    // 119: dp2 r4.x, cb0[16].xyxx, r8.xyxx
    r4.x = (dot((source[16].xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 120: dp2 r8.y, cb0[17].xyxx, r8.xyxx
    r8.y = (dot((source[17].xyxx).xy,(r8.xyxx).xy).xxxx).y;
    // 121: frc r4.x, r4.x
    r4.x = (frac(r4.xxxx)).x;
    // 122: mul r8.x, r4.x, l(0.125000)
    r8.x = ((r4.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 123: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r8.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 124: mul r4.x, r8.w, l(0.900000)
    r4.x = ((r8.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 125: mad r8.xyz, r8.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r5.xywx
    r8.xyz = ((r8.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r5.xywx))).xyz;
    // 126: mad r8.xyz, r4.xxxx, r8.xyzx, r5.xywx
    r8.xyz = ((r4.xxxx)*(r8.xyzx)+(r5.xywx)).xyz;
    // 127: add r3.z, r3.z, l(1.000000)
    r3.z = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 128: mul r3.z, r3.z, l(0.500000)
    r3.z = ((r3.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 129: mul_sat r8.xyz, r8.xyzx, r3.zzzz
    r8.xyz = (saturate((r8.xyzx)*(r3.zzzz))).xyz;
    // 130: mul r9.xyz, r8.xyzx, cb0[15].zzzz
    r9.xyz = ((r8.xyzx)*(source[15].zzzz)).xyz;
    // 131: dp3 r3.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 132: mul r3.z, r3.z, l(3.000000)
    r3.z = ((r3.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 133: mad r8.xyz, cb0[15].zzzz, r8.xyzx, -r5.xywx
    r8.xyz = ((source[15].zzzz)*(r8.xyzx)+(-(r5.xywx))).xyz;
    // 134: mad r5.xyw, r3.zzzz, r8.xyxz, r5.xyxw
    r5.xyw = ((r3.zzzz)*(r8.xyxz)+(r5.xyxw)).xyw;
    // 135: mad r5.xyw, r5.xyxw, cb2[3].wwww, cb2[3].xyxz
    r5.xyw = ((r5.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 136: mad r6.xyz, r6.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r6.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 137: dp3 r3.z, r6.xyzx, r6.xyzx
    r3.z = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 138: sqrt r3.z, r3.z
    r3.z = (sqrt(r3.zzzz)).z;
    // 139: div r6.xyz, r6.xyzx, r3.zzzz
    r6.xyz = ((r6.xyzx)/(r3.zzzz)).xyz;
    // 140: mul r6.xyz, r5.zzzz, r6.xyzx
    r6.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 141: mov_sat r3.z, r2.z
    r3.z = (saturate(r2.zzzz)).z;
    // 142: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 143: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 144: mul r6.xyz, r6.xyzx, r3.zzzz
    r6.xyz = ((r6.xyzx)*(r3.zzzz)).xyz;
    // 145: mul r6.xyz, r6.xyzx, cb0[22].xxxx
    r6.xyz = ((r6.xyzx)*(source[22].xxxx)).xyz;
    // 146: mad r7.xyz, cb0[22].yyyy, r10.xyzx, -r6.xyzx
    r7.xyz = ((source[22].yyyy)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 147: mad r6.xyz, r2.wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((r2.wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 148: add r1.xyz, -r1.xyzx, cb0[0].yzwy
    r1.xyz = ((-(r1.xyzx))+(source[0].yzwy)).xyz;
    // 149: mul r7.xyz, r2.zzzz, r1.xyzx
    r7.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 150: mad r1.xyz, r7.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r7.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 151: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 152: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 153: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 154: add r1.x, r1.x, cb0[8].z
    r1.x = ((r1.xxxx)+(source[8].zzzz)).x;
    // 155: dp3 r1.y, r0.xyzx, r3.xywx
    r1.y = (dot((r0.xyzx).xyz,(r3.xywx).xyz).xxxx).y;
    // 156: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 157: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 158: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 159: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 160: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 161: add r1.y, -cb0[23].x, l(0.200000)
    r1.y = ((-(source[23].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 162: mad r1.y, r2.w, r1.y, cb0[23].x
    r1.y = ((r2.wwww)*(r1.yyyy)+(source[23].xxxx)).y;
    // 163: mad r1.y, r1.y, l(4.500000), l(0.500000)
    r1.y = ((r1.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 164: mul r1.y, r1.y, cb0[23].y
    r1.y = ((r1.yyyy)*(source[23].yyyy)).y;
    // 165: mul r1.y, r1.y, l(0.050000)
    r1.y = ((r1.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 166: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 167: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 168: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 169: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 170: mul r1.x, r1.x, cb0[23].z
    r1.x = ((r1.xxxx)*(source[23].zzzz)).x;
    // 171: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 172: dp3 r1.y, r3.xywx, r2.xyzx
    r1.y = (dot((r3.xywx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 173: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 174: add r2.x, -|r2.z|, l(1.000000)
    r2.x = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 175: add r2.y, -|r1.y|, l(1.000000)
    r2.y = ((-(abs(r1.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 176: mul r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)*(r2.xxxx)).x;
    // 177: mad r2.xyw, r2.xxxx, cb0[10].xyxz, -cb0[10].xyxz
    r2.xyw = ((r2.xxxx)*(source[10].xyxz)+(-(source[10].xyxz))).xyw;
    // 178: mad r2.xyw, cb0[10].wwww, r2.xyxw, cb0[10].xyxz
    r2.xyw = ((source[10].wwww)*(r2.xyxw)+(source[10].xyxz)).xyw;
    // 179: mad r2.xyw, r1.zzzz, cb0[9].xyxz, r2.xyxw
    r2.xyw = ((r1.zzzz)*(source[9].xyxz)+(r2.xyxw)).xyw;
    // 180: mul_sat r1.y, r1.y, cb0[23].w
    r1.y = (saturate((r1.yyyy)*(source[23].wwww))).y;
    // 181: mul_sat r1.z, r2.z, cb0[23].w
    r1.z = (saturate((r2.zzzz)*(source[23].wwww))).z;
    // 182: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 183: add_sat r1.z, r1.z, -cb0[24].x
    r1.z = (saturate((r1.zzzz)+(-(source[24].xxxx)))).z;
    // 184: lt r2.z, r1.z, l(0.000001)
    r2.z = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 185: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 186: mul r1.z, r1.z, cb0[24].y
    r1.z = ((r1.zzzz)*(source[24].yyyy)).z;
    // 187: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 188: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 189: mul r1.y, r1.y, cb0[11].w
    r1.y = ((r1.yyyy)*(source[11].wwww)).y;
    // 190: mul r3.xyz, r1.yyyy, cb0[11].xyzx
    r3.xyz = ((r1.yyyy)*(source[11].xyzx)).xyz;
    // 191: movc r3.xyz, r2.zzzz, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 192: add r2.xyz, r2.xywx, r3.xyzx
    r2.xyz = ((r2.xywx)+(r3.xyzx)).xyz;
    // 193: mad r1.xyz, r1.xxxx, r6.xyzx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 194: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 195: dp3 r2.x, v7.xyzx, v7.xyzx
    r2.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 196: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 197: mul r2.xyz, r2.xxxx, v7.xyzx
    r2.xyz = ((r2.xxxx)*(v7.xyzx)).xyz;
    // 198: dp3 r2.x, r2.xyzx, r4.yzwy
    r2.x = (dot((r2.xyzx).xyz,(r4.yzwy).xyz).xxxx).x;
    // 199: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 200: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 201: mul r2.yzw, r2.yyyy, cb0[26].xxyz
    r2.yzw = ((r2.yyyy)*(source[26].xxyz)).yzw;
    // 202: mad r2.xyz, r2.xxxx, cb0[25].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[25].xyzx)+(r2.yzwy)).xyz;
    // 203: mul r2.xyz, r2.xyzx, cb0[27].wwww
    r2.xyz = ((r2.xyzx)*(source[27].wwww)).xyz;
    // 204: mul r3.xyz, r5.xywx, r2.xyzx
    r3.xyz = ((r5.xywx)*(r2.xyzx)).xyz;
    // 205: mad r1.xyz, r2.xyzx, r5.xywx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r5.xywx)+(r1.xyzx)).xyz;
    // 206: mad r1.xyz, r5.xywx, cb0[27].xyzx, r1.xyzx
    r1.xyz = ((r5.xywx)*(source[27].xyzx)+(r1.xyzx)).xyz;
    // 207: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 208: mad r2.x, r2.x, l(-0.250000), l(0.400000)
    r2.x = ((r2.xxxx)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).x;
    // 209: eq r2.y, cb0[28].x, l(0.000000)
    r2.y = (asfloat((uint4)((source[28].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 210: not r2.z, r2.y
    r2.z = (asfloat(~asuint(r2.yyyy))).z;
    // 211: lt r2.w, r1.w, r2.x
    r2.w = (asfloat((uint4)((r1.wwww)<(r2.xxxx)) * 0xffffffffu)).w;
    // 212: and r2.z, r2.w, r2.z
    r2.z = (asfloat(asuint(r2.wwww) & asuint(r2.zzzz))).z;
    // 213: discard_nz r2.z
    if ((asuint(r2.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 214: dp3 r2.z, v0.xyzx, v0.xyzx
    r2.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 215: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 216: mul r6.xyz, r2.zzzz, v0.xyzx
    r6.xyz = ((r2.zzzz)*(v0.xyzx)).xyz;
    // 217: mul r7.xyz, r0.zxyz, r6.yzxy
    r7.xyz = ((r0.zxyz)*(r6.yzxy)).xyz;
    // 218: mad r7.xyz, r0.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r0.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 219: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 220: movc r2.z, v9.x, l(1.000000), l(-1.000000)
    r2.z = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 221: mul r2.z, r2.z, cb0[0].x
    r2.z = ((r2.zzzz)*(source[0].xxxx)).z;
    // 222: mul r4.xyz, r2.zzzz, r4.yzwy
    r4.xyz = ((r2.zzzz)*(r4.yzwy)).xyz;
    // 223: ge r2.x, r1.w, r2.x
    r2.x = (asfloat((uint4)((r1.wwww)>=(r2.xxxx)) * 0xffffffffu)).x;
    // 224: mad r0.w, r0.w, cb0[1].w, l(-0.900000)
    r0.w = ((r0.wwww)*(source[1].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 225: mul_sat r0.w, r0.w, l(9.999998)
    r0.w = (saturate((r0.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 226: mad r2.z, r0.w, l(-2.000000), l(3.000000)
    r2.z = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 227: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 228: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 229: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 230: movc r0.w, r2.x, r0.w, r1.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (r0.wwww) : (r1.wwww)).w;
    // 231: movc o0.w, r2.y, r0.w, r1.w
    output.targets[0].w = ((asuint(r2.yyyy) != 0u) ? (r0.wwww) : (r1.wwww)).w;
    // 232: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 233: dp3 r1.x, r6.xyzx, r4.xyzx
    r1.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 234: dp3 r1.y, r7.xyzx, r4.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 235: dp3 r1.z, r0.xyzx, r4.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 236: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 237: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 238: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 239: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 240: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 241: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 242: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 243: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 244: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 245: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 246: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 247: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 248: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 249: mov o3.xyz, r5.xywx
    output.targets[3].xyz = (r5.xywx).xyz;
    // 250: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 251: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 252: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 253: ret
    return output;
}

// source.character.equipment-native-167.v1 / source program 12b672ec528d324da62c372f6a036774
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase167(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: dp3 r0.x, r0.xyzx, cb0[17].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[17].xyzx).xyz).xxxx).x;
    // 3: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[17].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[17].xyzx).xyz).xxxx)).y;
    // 4: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 7: add_sat r0.y, r1.w, -cb0[25].x
    r0.y = (saturate((r1.wwww)+(-(source[25].xxxx)))).y;
    // 8: mad r0.x, r0.y, r0.x, l(-0.001000)
    r0.x = ((r0.yyyy)*(r0.xxxx)+(float4(-0.001000,-0.001000,-0.001000,-0.001000))).x;
    // 9: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 11: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 12: add r0.x, v4.w, l(0.500000)
    r0.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 13: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 14: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mad r0.z, cb0[18].y, l(-3.500000), l(5.000000)
    r0.z = ((source[18].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 16: mul r0.z, r0.z, cb0[19].x
    r0.z = ((r0.zzzz)*(source[19].xxxx)).z;
    // 17: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: add r1.w, -r0.w, v4.z
    r1.w = ((-(r0.wwww))+(v4.zzzz)).w;
    // 19: mad r0.w, cb0[19].y, r1.w, r0.w
    r0.w = ((source[19].yyyy)*(r1.wwww)+(r0.wwww)).w;
    // 20: mul r1.w, r0.w, cb0[19].z
    r1.w = ((r0.wwww)*(source[19].zzzz)).w;
    // 21: mad r0.w, r1.w, l(0.750000), r0.w
    r0.w = ((r1.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 22: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 23: mad_sat r0.w, cb0[20].x, r0.w, r0.w
    r0.w = (saturate((source[20].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 24: mul r1.w, cb0[19].w, l(0.700000)
    r1.w = ((source[19].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 25: add r2.x, -r1.y, l(1.000000)
    r2.x = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yzw;
    // 27: mad r3.xy, r2.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r2.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 28: mad r2.y, r3.x, r1.x, l(0.200000)
    r2.y = ((r3.xxxx)*(r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 29: add r2.x, -r2.y, r2.x
    r2.x = ((-(r2.yyyy))+(r2.xxxx)).x;
    // 30: mad r2.z, cb0[21].x, r2.x, r2.y
    r2.z = ((source[21].xxxx)*(r2.xxxx)+(r2.yyyy)).z;
    // 31: mad r2.x, cb0[20].z, r2.x, r2.y
    r2.x = ((source[20].zzzz)*(r2.xxxx)+(r2.yyyy)).x;
    // 32: add r2.x, -r0.w, r2.x
    r2.x = ((-(r0.wwww))+(r2.xxxx)).x;
    // 33: mad r2.x, r1.w, r2.x, r0.w
    r2.x = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).x;
    // 34: div r2.x, r2.x, cb0[20].y
    r2.x = ((r2.xxxx)/(source[20].yyyy)).x;
    // 35: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mul r2.x, r0.z, r2.x
    r2.x = ((r0.zzzz)*(r2.xxxx)).x;
    // 37: mul r2.x, r2.x, l(4.000000)
    r2.x = ((r2.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 38: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 39: add r2.x, -r0.w, r2.z
    r2.x = ((-(r0.wwww))+(r2.zzzz)).x;
    // 40: mad r0.w, r1.w, r2.x, r0.w
    r0.w = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).w;
    // 41: div r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)/(source[20].wwww)).w;
    // 42: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 44: mul r0.z, r0.z, l(4.000000)
    r0.z = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).z;
    // 45: mul_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)*(r0.zzzz))).y;
    // 46: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 47: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 48: mad r0.x, cb0[21].y, r0.x, r1.y
    r0.x = ((source[21].yyyy)*(r0.xxxx)+(r1.yyyy)).x;
    // 49: add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // 50: mul r0.yzw, r0.yyzw, cb0[18].xxxx
    r0.yzw = ((r0.yyzw)*(source[18].xxxx)).yzw;
    // 51: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // 52: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 54: mad r0.xyz, cb0[21].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[21].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 55: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 57: mad r0.xyz, cb0[21].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[21].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 58: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 59: mad r4.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 60: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 61: mul r4.xyz, r0.xyzx, r2.xyzx
    r4.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 62: mad r0.xyz, r0.xyzx, r2.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 63: mad r2.xyz, cb0[9].xyzx, r1.xyzx, -r1.xyzx
    r2.xyz = ((source[9].xyzx)*(r1.xyzx)+(-(r1.xyzx))).xyz;
    // 64: mad r2.xyz, r2.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 65: mad r2.xyz, -r1.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xxxx))*(r4.xyzx)+(r2.xyzx)).xyz;
    // 66: mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 67: mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 68: add r0.w, -cb0[10].w, l(1.000000)
    r0.w = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: mul r0.w, r0.w, cb0[23].w
    r0.w = ((r0.wwww)*(source[23].wwww)).w;
    // 70: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 71: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 72: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul r1.w, cb0[10].z, l(1.500000)
    r1.w = ((source[10].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 74: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 75: mad r0.w, r0.w, l(0.500000), cb0[10].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 76: mul r4.y, cb0[10].y, cb0[11].y
    r4.y = ((source[10].yyyy)*(source[11].yyyy)).y;
    // 77: mul r5.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r5.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 78: frc r1.w, r5.x
    r1.w = (frac(r5.xxxx)).w;
    // 79: mul r5.y, r1.w, l(0.125000)
    r5.y = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 80: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 81: add r4.xy, r4.xyxx, r5.yzyy
    r4.xy = ((r4.xyxx)+(r5.yzyy)).xy;
    // 82: frc r1.w, cb0[10].x
    r1.w = (frac(source[10].xxxx)).w;
    // 83: add r3.w, -r1.w, cb0[10].x
    r3.w = ((-(r1.wwww))+(source[10].xxxx)).w;
    // 84: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 85: add r4.xy, r4.xyxx, r4.zwzz
    r4.xy = ((r4.xyxx)+(r4.zwzz)).xy;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 87: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 88: mul r0.w, r1.w, r4.w
    r0.w = ((r1.wwww)*(r4.wwww)).w;
    // 89: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 90: mad r2.xyz, r0.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 91: add r4.xyz, v7.xyzx, cb0[0].yzwy
    r4.xyz = ((v7.xyzx)+(source[0].yzwy)).xyz;
    // 92: add r5.xyzw, r4.yzxy, -cb0[1].yzxy
    r5.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 93: add r4.xyz, -r4.xyzx, cb0[0].yzwy
    r4.xyz = ((-(r4.xyzx))+(source[0].yzwy)).xyz;
    // 94: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 95: add r5.xy, -r5.zwzz, r5.xyxx
    r5.xy = ((-(r5.zwzz))+(r5.xyxx)).xy;
    // 96: mad r5.xy, cb0[12].wwww, r5.xyxx, r5.zwzz
    r5.xy = ((source[12].wwww)*(r5.xyxx)+(r5.zwzz)).xy;
    // 97: mul r0.w, cb0[12].y, cb0[23].w
    r0.w = ((source[12].yyyy)*(source[23].wwww)).w;
    // 98: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 99: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 100: mul r6.y, r0.w, l(0.020000)
    r6.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 101: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 103: mul r1.w, cb0[12].x, l(0.001000)
    r1.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 104: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 105: mad r5.xy, r1.wwww, r5.xyxx, r6.xyxx
    r5.xy = ((r1.wwww)*(r5.xyxx)+(r6.xyxx)).xy;
    // 106: dp2 r1.w, cb0[13].xyxx, r5.xyxx
    r1.w = (dot((source[13].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 107: dp2 r5.y, cb0[14].xyxx, r5.xyxx
    r5.y = (dot((source[14].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 108: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 109: mul r5.x, r1.w, l(0.125000)
    r5.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 111: mad r5.xyz, r5.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r2.xyzx
    r5.xyz = ((r5.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r2.xyzx))).xyz;
    // 112: mul r1.w, r5.w, l(0.900000)
    r1.w = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 113: mad r5.xyz, r1.wwww, r5.xyzx, r2.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 114: mul_sat r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = (saturate((r0.wwww)*(r5.xyzx))).xyz;
    // 115: mad r6.xyz, cb0[12].zzzz, r5.xyzx, -r2.xyzx
    r6.xyz = ((source[12].zzzz)*(r5.xyzx)+(-(r2.xyzx))).xyz;
    // 116: mul r5.xyz, r5.xyzx, cb0[12].zzzz
    r5.xyz = ((r5.xyzx)*(source[12].zzzz)).xyz;
    // 117: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 118: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 119: mad r2.xyz, r0.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 120: mul r2.xyz, r2.xyzx, cb0[24].xxxx
    r2.xyz = ((r2.xyzx)*(source[24].xxxx)).xyz;
    // 121: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 122: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 125: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 126: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 127: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 128: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 129: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 130: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 131: mul r5.xyz, r0.wwww, v5.xyzx
    r5.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 132: dp3 r0.w, r3.xyzx, r5.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 133: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: mad r5.xyw, r1.wwww, cb0[8].xyxz, r2.xyxz
    r5.xyw = ((r1.wwww)*(source[8].xyxz)+(r2.xyxz)).xyw;
    // 135: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 136: add r1.w, -|r5.z|, l(1.000000)
    r1.w = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: add r3.w, -|r0.w|, l(1.000000)
    r3.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: mul_sat r0.w, r0.w, cb0[24].y
    r0.w = (saturate((r0.wwww)*(source[24].yyyy))).w;
    // 139: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 141: mad r6.xyz, r1.wwww, cb0[15].xyzx, -cb0[15].xyzx
    r6.xyz = ((r1.wwww)*(source[15].xyzx)+(-(source[15].xyzx))).xyz;
    // 142: mad r6.xyz, cb0[15].wwww, r6.xyzx, cb0[15].xyzx
    r6.xyz = ((source[15].wwww)*(r6.xyzx)+(source[15].xyzx)).xyz;
    // 143: add r5.xyw, r5.xyxw, r6.xyxz
    r5.xyw = ((r5.xyxw)+(r6.xyxz)).xyw;
    // 144: mul_sat r1.w, r5.z, cb0[24].y
    r1.w = (saturate((r5.zzzz)*(source[24].yyyy))).w;
    // 145: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: add_sat r1.w, r1.w, -cb0[24].z
    r1.w = (saturate((r1.wwww)+(-(source[24].zzzz)))).w;
    // 147: log r3.w, r1.w
    r3.w = (log2(r1.wwww)).w;
    // 148: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 149: mul r3.w, r3.w, cb0[24].w
    r3.w = ((r3.wwww)*(source[24].wwww)).w;
    // 150: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 151: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 152: mul r0.w, r0.w, cb0[16].w
    r0.w = ((r0.wwww)*(source[16].wwww)).w;
    // 153: mul r6.xyz, r0.wwww, cb0[16].xyzx
    r6.xyz = ((r0.wwww)*(source[16].xyzx)).xyz;
    // 154: movc r6.xyz, r1.wwww, l(0,0,0,0), r6.xyzx
    r6.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xyzx)).xyz;
    // 155: add r5.xyw, r5.xyxw, r6.xyxz
    r5.xyw = ((r5.xyxw)+(r6.xyxz)).xyw;
    // 156: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 157: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 158: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 159: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 160: mov_sat r0.w, r5.z
    r0.w = (saturate(r5.zzzz)).w;
    // 161: mul r6.xyz, r4.xyzx, r5.zzzz
    r6.xyz = ((r4.xyzx)*(r5.zzzz)).xyz;
    // 162: mad r4.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r4.xyzx)).xyz;
    // 163: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 164: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 165: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 166: mul r0.xyz, r0.xyzx, cb0[22].xxxx
    r0.xyz = ((r0.xyzx)*(source[22].xxxx)).xyz;
    // 167: mad r1.xyz, cb0[22].yyyy, r1.xyzx, -r0.xyzx
    r1.xyz = ((source[22].yyyy)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 168: mad r0.xyz, r2.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 169: add r0.w, -cb0[23].x, l(0.200000)
    r0.w = ((-(source[23].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 170: mad r0.w, r2.w, r0.w, cb0[23].x
    r0.w = ((r2.wwww)*(r0.wwww)+(source[23].xxxx)).w;
    // 171: mad r0.w, r0.w, l(4.500000), l(0.500000)
    r0.w = ((r0.wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 172: mul r0.w, r0.w, cb0[23].y
    r0.w = ((r0.wwww)*(source[23].yyyy)).w;
    // 173: mul r0.w, r0.w, l(0.050000)
    r0.w = ((r0.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 174: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 175: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 176: div r1.x, r4.z, r1.x
    r1.x = ((r4.zzzz)/(r1.xxxx)).x;
    // 177: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 178: dp3 r1.y, v1.xyzx, v1.xyzx
    r1.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 179: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 180: mul r1.yzw, r1.yyyy, v1.xxyz
    r1.yzw = ((r1.yyyy)*(v1.xxyz)).yzw;
    // 181: dp3 r2.w, r1.yzwy, r3.xyzx
    r2.w = (dot((r1.yzwy).xyz,(r3.xyzx).xyz).xxxx).w;
    // 182: add r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)+(r2.wwww)).x;
    // 183: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 184: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 185: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 186: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: log r2.w, r1.x
    r2.w = (log2(r1.xxxx)).w;
    // 188: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 189: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 190: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 191: mul r0.w, r0.w, cb0[23].z
    r0.w = ((r0.wwww)*(source[23].zzzz)).w;
    // 192: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 193: mad r0.xyz, r0.wwww, r0.xyzx, r5.xywx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xywx)).xyz;
    // 194: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 195: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 196: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 197: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 198: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 199: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 200: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 201: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 202: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 203: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 204: mul r4.yzw, r4.yyyy, cb0[27].xxyz
    r4.yzw = ((r4.yyyy)*(source[27].xxyz)).yzw;
    // 205: mad r4.xyz, r4.xxxx, cb0[26].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[26].xyzx)+(r4.yzwy)).xyz;
    // 206: mul r4.xyz, r4.xyzx, cb0[28].wwww
    r4.xyz = ((r4.xyzx)*(source[28].wwww)).xyz;
    // 207: mad r0.xyz, r4.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 208: mul r4.xyz, r2.xyzx, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 209: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 210: mad o0.xyz, r2.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 211: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 212: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 213: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 214: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 215: mul r0.xyz, r0.xxxx, v0.xyzx
    r0.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 216: mul r2.xyz, r0.yzxy, r1.wyzw
    r2.xyz = ((r0.yzxy)*(r1.wyzw)).xyz;
    // 217: mad r2.xyz, r1.zwyz, r0.zxyz, -r2.xyzx
    r2.xyz = ((r1.zwyz)*(r0.zxyz)+(-(r2.xyzx))).xyz;
    // 218: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 219: movc r0.w, v8.x, l(1.000000), l(-1.000000)
    r0.w = ((asuint(v8.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 220: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 221: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 222: dp3 r2.y, r2.xyzx, r3.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 223: dp3 r2.z, r1.yzwy, r3.xyzx
    r2.z = (dot((r1.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 224: dp3 r2.x, r0.xyzx, r3.xyzx
    r2.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 225: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 226: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 227: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 228: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 229: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 230: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 231: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 232: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 233: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 234: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 235: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 236: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 237: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 238: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 239: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 240: ret
    return output;
}

// source.character.equipment-native-168.v1 / source program 040a63ec2e3e5e42a8f2194c6622723a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase168(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
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

// source.character.equipment-native-169.v1 / source program 2d30b2ff02c1c64db77f2e1dac687cd1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase169(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
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
    // 4: mad r0.z, cb0[17].y, l(-3.500000), l(5.000000)
    r0.z = ((source[17].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 5: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 6: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: add r1.x, -r0.w, v4.z
    r1.x = ((-(r0.wwww))+(v4.zzzz)).x;
    // 8: mad r0.w, cb0[18].y, r1.x, r0.w
    r0.w = ((source[18].yyyy)*(r1.xxxx)+(r0.wwww)).w;
    // 9: mul r1.x, r0.w, cb0[18].z
    r1.x = ((r0.wwww)*(source[18].zzzz)).x;
    // 10: mad r0.w, r1.x, l(0.750000), r0.w
    r0.w = ((r1.xxxx)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 11: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 12: mad_sat r0.w, cb0[19].x, r0.w, r0.w
    r0.w = (saturate((source[19].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 13: mul r1.x, cb0[18].w, l(0.700000)
    r1.x = ((source[18].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
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
    // 20: mad r1.w, cb0[20].x, r1.y, r1.z
    r1.w = ((source[20].xxxx)*(r1.yyyy)+(r1.zzzz)).w;
    // 21: mad r1.y, cb0[19].z, r1.y, r1.z
    r1.y = ((source[19].zzzz)*(r1.yyyy)+(r1.zzzz)).y;
    // 22: add r1.y, -r0.w, r1.y
    r1.y = ((-(r0.wwww))+(r1.yyyy)).y;
    // 23: mad r1.y, r1.x, r1.y, r0.w
    r1.y = ((r1.xxxx)*(r1.yyyy)+(r0.wwww)).y;
    // 24: div r1.y, r1.y, cb0[19].y
    r1.y = ((r1.yyyy)/(source[19].yyyy)).y;
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
    // 31: div r0.w, r0.w, cb0[19].w
    r0.w = ((r0.wwww)/(source[19].wwww)).w;
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
    // 38: mad r0.x, cb0[20].y, r0.x, r2.y
    r0.x = ((source[20].yyyy)*(r0.xxxx)+(r2.yyyy)).x;
    // 39: add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // 40: mul r0.yzw, r0.yyzw, cb0[17].xxxx
    r0.yzw = ((r0.yyzw)*(source[17].xxxx)).yzw;
    // 41: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // 42: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 43: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 44: mad r0.xyz, cb0[20].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 45: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 47: mad r0.xyz, cb0[20].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[20].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
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
    // 64: mul r1.xyz, r1.xyzx, cb0[21].xxxx
    r1.xyz = ((r1.xyzx)*(source[21].xxxx)).xyz;
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
    // 70: mad r6.xyz, cb0[21].yyyy, r5.xyzx, -r1.xyzx
    r6.xyz = ((source[21].yyyy)*(r5.xyzx)+(-(r1.xyzx))).xyz;
    // 71: mad r1.xyz, r0.wwww, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 72: mad r5.xyz, -r2.xxxx, r0.xyzx, r5.xyzx
    r5.xyz = ((-(r2.xxxx))*(r0.xyzx)+(r5.xyzx)).xyz;
    // 73: mul r0.xyz, r0.xyzx, r2.xxxx
    r0.xyz = ((r0.xyzx)*(r2.xxxx)).xyz;
    // 74: mul o0.w, r2.w, cb0[1].w
    output.targets[0].w = ((r2.wwww)*(source[1].wwww)).w;
    // 75: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 76: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 77: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 78: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 79: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 80: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 81: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 82: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 83: div r2.xyz, r4.xyzx, r1.wwww
    r2.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 84: dp3 r1.w, r2.xyzx, r3.xywx
    r1.w = (dot((r2.xyzx).xyz,(r3.xywx).xyz).xxxx).w;
    // 85: add r2.w, -|r1.w|, l(1.000000)
    r2.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: add r3.x, -|r3.w|, l(1.000000)
    r3.x = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 87: mul r2.w, r2.w, r3.x
    r2.w = ((r2.wwww)*(r3.xxxx)).w;
    // 88: mad r3.xyz, r2.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r3.xyz = ((r2.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 89: mad r3.xyz, cb0[10].wwww, r3.xyzx, cb0[10].xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(source[10].xyzx)).xyz;
    // 90: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: mul_sat r1.w, r1.w, cb0[22].w
    r1.w = (saturate((r1.wwww)*(source[22].wwww))).w;
    // 92: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: mad r3.xyz, r2.wwww, cb0[9].xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(source[9].xyzx)+(r3.xyzx)).xyz;
    // 94: mul_sat r2.w, r3.w, cb0[22].w
    r2.w = (saturate((r3.wwww)*(source[22].wwww))).w;
    // 95: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: add_sat r2.w, r2.w, -cb0[23].x
    r2.w = (saturate((r2.wwww)+(-(source[23].xxxx)))).w;
    // 97: log r4.x, r2.w
    r4.x = (log2(r2.wwww)).x;
    // 98: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 99: mul r4.x, r4.x, cb0[23].y
    r4.x = ((r4.xxxx)*(source[23].yyyy)).x;
    // 100: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 101: mul r1.w, r1.w, r4.x
    r1.w = ((r1.wwww)*(r4.xxxx)).w;
    // 102: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 103: mul r4.xyz, r1.wwww, cb0[11].xyzx
    r4.xyz = ((r1.wwww)*(source[11].xyzx)).xyz;
    // 104: movc r4.xyz, r2.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 105: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 106: add r1.w, -cb0[22].x, l(0.200000)
    r1.w = ((-(source[22].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 107: mad r0.w, r0.w, r1.w, cb0[22].x
    r0.w = ((r0.wwww)*(r1.wwww)+(source[22].xxxx)).w;
    // 108: mad r0.w, r0.w, l(4.500000), l(0.500000)
    r0.w = ((r0.wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 109: mul r0.w, r0.w, cb0[22].y
    r0.w = ((r0.wwww)*(source[22].yyyy)).w;
    // 110: mul r0.w, r0.w, l(0.050000)
    r0.w = ((r0.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 111: add r4.xyz, v8.xyzx, cb0[0].yzwy
    r4.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 112: add r5.xyz, -r4.xyzx, cb0[0].yzwy
    r5.xyz = ((-(r4.xyzx))+(source[0].yzwy)).xyz;
    // 113: add r4.xyzw, r4.yzxy, -cb0[1].yzxy
    r4.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 114: mul r6.xyz, r3.wwww, r5.xyzx
    r6.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 115: mad r5.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r5.xyzx)).xyz;
    // 116: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 117: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 118: div r1.w, r5.z, r1.w
    r1.w = ((r5.zzzz)/(r1.wwww)).w;
    // 119: add r1.w, r1.w, cb0[8].z
    r1.w = ((r1.wwww)+(source[8].zzzz)).w;
    // 120: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 121: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 122: mul r5.xyz, r2.wwww, v1.xyzx
    r5.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 123: dp3 r2.w, r5.xyzx, r2.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 124: add r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)+(r2.wwww)).w;
    // 125: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 126: add r1.w, r1.w, l(-0.500000)
    r1.w = ((r1.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 127: add r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)+(r1.wwww)).w;
    // 128: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 130: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 131: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 132: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 133: mul r0.w, r0.w, cb0[22].z
    r0.w = ((r0.wwww)*(source[22].zzzz)).w;
    // 134: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 135: mad r1.xyz, r0.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 136: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 137: add r3.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 138: add r3.xy, -r4.zwzz, r3.xyxx
    r3.xy = ((-(r4.zwzz))+(r3.xyxx)).xy;
    // 139: mad r3.xy, cb0[14].wwww, r3.xyxx, r4.zwzz
    r3.xy = ((source[14].wwww)*(r3.xyxx)+(r4.zwzz)).xy;
    // 140: mul r0.w, cb0[14].y, cb0[23].w
    r0.w = ((source[14].yyyy)*(source[23].wwww)).w;
    // 141: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 142: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 143: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 144: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 146: mul r1.w, cb0[14].x, l(0.001000)
    r1.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 147: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 148: mad r3.xy, r1.wwww, r3.xyxx, r4.xyxx
    r3.xy = ((r1.wwww)*(r3.xyxx)+(r4.xyxx)).xy;
    // 149: dp2 r4.y, cb0[16].xyxx, r3.xyxx
    r4.y = (dot((source[16].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 150: dp2 r1.w, cb0[15].xyxx, r3.xyxx
    r1.w = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 151: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 152: mul r4.x, r1.w, l(0.125000)
    r4.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 153: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 154: mul r1.w, r3.w, l(0.900000)
    r1.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 155: add r2.w, -cb0[12].w, l(1.000000)
    r2.w = ((-(source[12].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: mul r2.w, r2.w, cb0[23].w
    r2.w = ((r2.wwww)*(source[23].wwww)).w;
    // 157: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 158: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 159: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: mul r3.w, cb0[12].z, l(1.500000)
    r3.w = ((source[12].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 161: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 162: mad r2.w, r2.w, l(0.500000), cb0[12].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[12].zzzz)).w;
    // 163: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 164: mul r4.x, r3.w, l(0.125000)
    r4.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 165: mul r6.y, cb0[12].y, cb0[13].y
    r6.y = ((source[12].yyyy)*(source[13].yyyy)).y;
    // 166: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 167: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 168: add r4.xy, r4.xyxx, r6.xyxx
    r4.xy = ((r4.xyxx)+(r6.xyxx)).xy;
    // 169: frc r3.w, cb0[12].x
    r3.w = (frac(source[12].xxxx)).w;
    // 170: add r4.z, -r3.w, cb0[12].x
    r4.z = ((-(r3.wwww))+(source[12].xxxx)).z;
    // 171: mul r6.z, r4.z, l(0.125000)
    r6.z = ((r4.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 172: add r4.xy, r4.xyxx, r6.zwzz
    r4.xy = ((r4.xyxx)+(r6.zwzz)).xy;
    // 173: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 174: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 175: mul r2.w, r3.w, r4.w
    r2.w = ((r3.wwww)*(r4.wwww)).w;
    // 176: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 177: mad r0.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 178: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 179: mad r3.xyz, r1.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 180: mul_sat r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx))).xyz;
    // 181: mad r4.xyz, cb0[14].zzzz, r3.xyzx, -r0.xyzx
    r4.xyz = ((source[14].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 182: mul r3.xyz, r3.xyzx, cb0[14].zzzz
    r3.xyz = ((r3.xyzx)*(source[14].zzzz)).xyz;
    // 183: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 184: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 185: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 186: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 187: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 188: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 189: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 190: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 191: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 192: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 193: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 194: mul r2.xyz, r2.xyzx, cb0[0].xxxx
    r2.xyz = ((r2.xyzx)*(source[0].xxxx)).xyz;
    // 195: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 196: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 197: mul r3.yzw, r3.yyyy, cb0[25].xxyz
    r3.yzw = ((r3.yyyy)*(source[25].xxyz)).yzw;
    // 198: mad r3.xyz, r3.xxxx, cb0[24].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[24].xyzx)+(r3.yzwy)).xyz;
    // 199: mul r3.xyz, r3.xyzx, cb0[26].wwww
    r3.xyz = ((r3.xyzx)*(source[26].wwww)).xyz;
    // 200: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 201: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 202: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 203: mad r1.xyz, r0.xyzx, cb0[26].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 204: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 205: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 206: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 207: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 208: mul r0.xyz, r0.xxxx, v0.xyzx
    r0.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 209: mul r1.xyz, r0.yzxy, r5.zxyz
    r1.xyz = ((r0.yzxy)*(r5.zxyz)).xyz;
    // 210: mad r1.xyz, r5.yzxy, r0.zxyz, -r1.xyzx
    r1.xyz = ((r5.yzxy)*(r0.zxyz)+(-(r1.xyzx))).xyz;
    // 211: dp3 r3.z, r5.xyzx, r2.xyzx
    r3.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 212: dp3 r3.x, r0.xyzx, r2.xyzx
    r3.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 213: mul r0.xyz, r1.xyzx, v1.wwww
    r0.xyz = ((r1.xyzx)*(v1.wwww)).xyz;
    // 214: dp3 r3.y, r0.xyzx, r2.xyzx
    r3.y = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 215: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 216: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 217: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 218: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 219: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 220: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 221: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 222: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 223: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 224: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 225: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 226: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 227: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 228: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 229: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 230: ret
    return output;
}

// source.character.equipment-native-170.v1 / source program 08b63ec08e5d634a97d4c22d45900cd7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase170(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
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
    // 22: mul r0.w, r5.w, cb0[1].w
    r0.w = ((r5.wwww)*(source[1].wwww)).w;
    // 23: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 24: mul r6.xyz, r6.xyzx, cb0[16].xxxx
    r6.xyz = ((r6.xyzx)*(source[16].xxxx)).xyz;
    // 25: mad r1.w, cb0[16].y, l(-3.500000), l(5.000000)
    r1.w = ((source[16].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 26: mul r1.w, r1.w, cb0[17].x
    r1.w = ((r1.wwww)*(source[17].xxxx)).w;
    // 27: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: add r4.w, -r2.w, v4.z
    r4.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 29: mad r2.w, cb0[17].y, r4.w, r2.w
    r2.w = ((source[17].yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 30: mul r4.w, r2.w, cb0[17].z
    r4.w = ((r2.wwww)*(source[17].zzzz)).w;
    // 31: mad r2.w, r4.w, l(0.750000), r2.w
    r2.w = ((r4.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 32: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 33: mad_sat r2.w, cb0[18].x, r2.w, r2.w
    r2.w = (saturate((source[18].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 34: mad r3.x, r3.x, r5.x, l(0.200000)
    r3.x = ((r3.xxxx)*(r5.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 35: add r4.w, -r5.y, l(1.000000)
    r4.w = ((-(r5.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: add r4.w, -r3.x, r4.w
    r4.w = ((-(r3.xxxx))+(r4.wwww)).w;
    // 37: mad r6.w, cb0[18].z, r4.w, r3.x
    r6.w = ((source[18].zzzz)*(r4.wwww)+(r3.xxxx)).w;
    // 38: mul r7.x, cb0[17].w, l(0.700000)
    r7.x = ((source[17].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 39: add r6.w, -r2.w, r6.w
    r6.w = ((-(r2.wwww))+(r6.wwww)).w;
    // 40: mad r6.w, r7.x, r6.w, r2.w
    r6.w = ((r7.xxxx)*(r6.wwww)+(r2.wwww)).w;
    // 41: div r6.w, r6.w, cb0[18].y
    r6.w = ((r6.wwww)/(source[18].yyyy)).w;
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
    // 48: mad r3.x, cb0[19].x, r4.w, r3.x
    r3.x = ((source[19].xxxx)*(r4.wwww)+(r3.xxxx)).x;
    // 49: add r3.x, -r2.w, r3.x
    r3.x = ((-(r2.wwww))+(r3.xxxx)).x;
    // 50: mad r2.w, r7.x, r3.x, r2.w
    r2.w = ((r7.xxxx)*(r3.xxxx)+(r2.wwww)).w;
    // 51: div r2.w, r2.w, cb0[18].w
    r2.w = ((r2.wwww)/(source[18].wwww)).w;
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
    // 59: mad r1.w, cb0[19].y, r1.w, r5.y
    r1.w = ((source[19].yyyy)*(r1.wwww)+(r5.yyyy)).w;
    // 60: mad r6.xyz, r1.wwww, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 61: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 63: mad r6.xyz, cb0[19].zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((source[19].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 64: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 66: mad r6.xyz, cb0[19].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[19].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
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
    // 74: mul r2.w, r2.w, cb0[22].z
    r2.w = ((r2.wwww)*(source[22].zzzz)).w;
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
    // 95: mul r1.w, cb0[13].y, cb0[22].z
    r1.w = ((source[13].yyyy)*(source[22].zzzz)).w;
    // 96: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 97: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 98: mul r5.y, r1.w, l(0.020000)
    r5.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 99: add r9.xyzw, r1.yzxy, -cb0[1].yzxy
    r9.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 100: add r9.xy, -r9.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r9.xy = ((-(r9.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 101: add r9.xy, -r9.zwzz, r9.xyxx
    r9.xy = ((-(r9.zwzz))+(r9.xyxx)).xy;
    // 102: mad r9.xy, cb0[13].wwww, r9.xyxx, r9.zwzz
    r9.xy = ((source[13].wwww)*(r9.xyxx)+(r9.zwzz)).xy;
    // 103: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 104: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 105: mad r5.xy, r2.wwww, r9.xyxx, r5.xyxx
    r5.xy = ((r2.wwww)*(r9.xyxx)+(r5.xyxx)).xy;
    // 106: dp2 r2.w, cb0[14].xyxx, r5.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 107: dp2 r5.y, cb0[15].xyxx, r5.xyxx
    r5.y = (dot((source[15].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 108: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 109: mul r5.x, r2.w, l(0.125000)
    r5.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 111: mul r2.w, r9.w, l(0.900000)
    r2.w = ((r9.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 112: mad r9.xyz, r9.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r8.xyzx))).xyz;
    // 113: mad r9.xyz, r2.wwww, r9.xyzx, r8.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 114: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 116: mul_sat r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = (saturate((r9.xyzx)*(r1.wwww))).xyz;
    // 117: mul r10.xyz, r9.xyzx, cb0[13].zzzz
    r10.xyz = ((r9.xyzx)*(source[13].zzzz)).xyz;
    // 118: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 120: mad r9.xyz, cb0[13].zzzz, r9.xyzx, -r8.xyzx
    r9.xyz = ((source[13].zzzz)*(r9.xyzx)+(-(r8.xyzx))).xyz;
    // 121: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 122: mul r8.xyz, r8.xyzx, cb0[23].zzzz
    r8.xyz = ((r8.xyzx)*(source[23].zzzz)).xyz;
    // 123: mad r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = ((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 124: mad r6.xyz, r6.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r6.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 125: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 126: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 127: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 128: mul r5.xyz, r5.zzzz, r6.xyzx
    r5.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 129: mov_sat r1.w, r2.z
    r1.w = (saturate(r2.zzzz)).w;
    // 130: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 131: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 132: mul r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 133: mul r5.xyz, r5.xyzx, cb0[20].xxxx
    r5.xyz = ((r5.xyzx)*(source[20].xxxx)).xyz;
    // 134: add r1.xyz, -r1.xyzx, cb0[0].yzwy
    r1.xyz = ((-(r1.xyzx))+(source[0].yzwy)).xyz;
    // 135: mul r6.xyz, r2.zzzz, r1.xyzx
    r6.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 136: mad r1.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 137: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 138: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 139: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 140: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 141: dp3 r1.y, r0.xyzx, r3.yzwy
    r1.y = (dot((r0.xyzx).xyz,(r3.yzwy).xyz).xxxx).y;
    // 142: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 143: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 144: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 145: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 146: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 147: mad r1.y, -cb0[20].z, cb0[20].w, l(1.000000)
    r1.y = ((-(source[20].zzzz))*(source[20].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 148: mad r1.y, r1.y, l(4.500000), l(0.500000)
    r1.y = ((r1.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 149: mul r1.y, r1.y, cb0[21].x
    r1.y = ((r1.yyyy)*(source[21].xxxx)).y;
    // 150: mul r1.y, r1.y, l(0.050000)
    r1.y = ((r1.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 151: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 152: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 153: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 154: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 155: mul r1.x, r1.x, cb0[21].y
    r1.x = ((r1.xxxx)*(source[21].yyyy)).x;
    // 156: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 157: dp3 r1.y, r3.yzwy, r2.xyzx
    r1.y = (dot((r3.yzwy).xyz,(r2.xyzx).xyz).xxxx).y;
    // 158: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 159: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: add r2.x, -|r1.y|, l(1.000000)
    r2.x = ((-(abs(r1.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 161: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 162: mad r2.xyw, r1.wwww, cb0[9].xyxz, -cb0[9].xyxz
    r2.xyw = ((r1.wwww)*(source[9].xyxz)+(-(source[9].xyxz))).xyw;
    // 163: mad r2.xyw, cb0[9].wwww, r2.xyxw, cb0[9].xyxz
    r2.xyw = ((source[9].wwww)*(r2.xyxw)+(source[9].xyxz)).xyw;
    // 164: mad r2.xyw, r1.zzzz, cb0[8].xyxz, r2.xyxw
    r2.xyw = ((r1.zzzz)*(source[8].xyxz)+(r2.xyxw)).xyw;
    // 165: mul_sat r1.y, r1.y, cb0[21].z
    r1.y = (saturate((r1.yyyy)*(source[21].zzzz))).y;
    // 166: mul_sat r1.z, r2.z, cb0[21].z
    r1.z = (saturate((r2.zzzz)*(source[21].zzzz))).z;
    // 167: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 168: add_sat r1.z, r1.z, -cb0[21].w
    r1.z = (saturate((r1.zzzz)+(-(source[21].wwww)))).z;
    // 169: lt r1.w, r1.z, l(0.000001)
    r1.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 170: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 171: mul r1.z, r1.z, cb0[22].x
    r1.z = ((r1.zzzz)*(source[22].xxxx)).z;
    // 172: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 173: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 174: mul r1.y, r1.y, cb0[10].w
    r1.y = ((r1.yyyy)*(source[10].wwww)).y;
    // 175: mul r3.xyz, r1.yyyy, cb0[10].xyzx
    r3.xyz = ((r1.yyyy)*(source[10].xyzx)).xyz;
    // 176: movc r1.yzw, r1.wwww, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 177: add r1.yzw, r1.yyzw, r2.xxyw
    r1.yzw = ((r1.yyzw)+(r2.xxyw)).yzw;
    // 178: mad r1.xyz, r1.xxxx, r5.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(r1.yzwy)).xyz;
    // 179: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 180: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 181: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 182: mul r2.xyz, r1.wwww, v7.xyzx
    r2.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 183: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 184: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 185: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 186: mul r2.yzw, r2.yyyy, cb0[25].xxyz
    r2.yzw = ((r2.yyyy)*(source[25].xxyz)).yzw;
    // 187: mad r2.xyz, r2.xxxx, cb0[24].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[24].xyzx)+(r2.yzwy)).xyz;
    // 188: mul r2.xyz, r2.xyzx, cb0[26].wwww
    r2.xyz = ((r2.xyzx)*(source[26].wwww)).xyz;
    // 189: mul r3.xyz, r8.xyzx, r2.xyzx
    r3.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 190: mad r1.xyz, r2.xyzx, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 191: mad r1.xyz, r8.xyzx, cb0[26].xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 192: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 193: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 194: eq r2.x, cb0[27].x, l(0.000000)
    r2.x = (asfloat((uint4)((source[27].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 195: not r2.y, r2.x
    r2.y = (asfloat(~asuint(r2.xxxx))).y;
    // 196: lt r2.z, r0.w, r1.w
    r2.z = (asfloat((uint4)((r0.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 197: and r2.y, r2.z, r2.y
    r2.y = (asfloat(asuint(r2.zzzz) & asuint(r2.yyyy))).y;
    // 198: discard_nz r2.y
    if ((asuint(r2.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 199: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 200: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 201: mul r2.yzw, r2.yyyy, v0.xxyz
    r2.yzw = ((r2.yyyy)*(v0.xxyz)).yzw;
    // 202: mul r5.xyz, r0.zxyz, r2.zwyz
    r5.xyz = ((r0.zxyz)*(r2.zwyz)).xyz;
    // 203: mad r5.xyz, r0.yzxy, r2.wyzw, -r5.xyzx
    r5.xyz = ((r0.yzxy)*(r2.wyzw)+(-(r5.xyzx))).xyz;
    // 204: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 205: movc r3.w, v9.x, l(1.000000), l(-1.000000)
    r3.w = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 206: mul r3.w, r3.w, cb0[0].x
    r3.w = ((r3.wwww)*(source[0].xxxx)).w;
    // 207: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 208: ge r1.w, r0.w, r1.w
    r1.w = (asfloat((uint4)((r0.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 209: mad r3.w, r5.w, cb0[1].w, l(-0.900000)
    r3.w = ((r5.wwww)*(source[1].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 210: mul_sat r3.w, r3.w, l(9.999998)
    r3.w = (saturate((r3.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 211: mad r4.w, r3.w, l(-2.000000), l(3.000000)
    r4.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 212: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 213: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 214: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 215: movc r1.w, r1.w, r3.w, r0.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r3.wwww) : (r0.wwww)).w;
    // 216: movc o0.w, r2.x, r1.w, r0.w
    output.targets[0].w = ((asuint(r2.xxxx) != 0u) ? (r1.wwww) : (r0.wwww)).w;
    // 217: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 218: dp3 r1.x, r2.yzwy, r4.xyzx
    r1.x = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).x;
    // 219: dp3 r1.y, r5.xyzx, r4.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 220: dp3 r1.z, r0.xyzx, r4.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 221: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 222: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 223: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 224: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 225: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 226: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 227: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 228: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 229: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 230: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 231: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 232: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 233: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 234: mov o3.xyz, r8.xyzx
    output.targets[3].xyz = (r8.xyzx).xyz;
    // 235: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 236: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 237: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 238: ret
    return output;
}

// source.character.equipment-native-171.v1 / source program f8e1231ac2be8a40a804e0e90acc5729
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase171(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].y=(g_SourceCharacterTime.xxxx).x;
    source[20].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[20].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
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
    // 15: mul_sat r1.y, r0.w, cb0[19].y
    r1.y = (saturate((r0.wwww)*(source[19].yyyy))).y;
    // 16: mul_sat r1.z, r1.x, cb0[19].y
    r1.z = (saturate((r1.xxxx)*(source[19].yyyy))).z;
    // 17: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 18: add_sat r1.z, r1.z, -cb0[19].z
    r1.z = (saturate((r1.zzzz)+(-(source[19].zzzz)))).z;
    // 19: log r1.w, r1.z
    r1.w = (log2(r1.zzzz)).w;
    // 20: lt r1.z, r1.z, l(0.000001)
    r1.z = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 21: mul r1.w, r1.w, cb0[19].w
    r1.w = ((r1.wwww)*(source[19].wwww)).w;
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
    // 58: mad r4.x, cb0[18].z, l(4.500000), l(0.500000)
    r4.x = ((source[18].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 59: mul r4.x, r4.x, cb0[18].w
    r4.x = ((r4.xxxx)*(source[18].wwww)).x;
    // 60: mul r4.x, r4.x, l(0.050000)
    r4.x = ((r4.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))).x;
    // 61: mul r3.w, r3.w, r4.x
    r3.w = ((r3.wwww)*(r4.xxxx)).w;
    // 62: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 63: mul r3.w, r3.w, cb0[19].x
    r3.w = ((r3.wwww)*(source[19].xxxx)).w;
    // 64: movc r1.x, r1.x, l(0), r3.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).x;
    // 65: add r4.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r4.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 66: mul r4.xyz, r4.xyzx, cb0[17].xxxx
    r4.xyz = ((r4.xyzx)*(source[17].xxxx)).xyz;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 68: mad r4.xyz, r5.yyyy, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r5.yyyy)*(r4.xyzx)+(source[3].xyzx)).xyz;
    // 69: dp3 r3.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: add r6.xyz, -r4.xyzx, r3.wwww
    r6.xyz = ((-(r4.xyzx))+(r3.wwww)).xyz;
    // 71: mad r4.xyz, cb0[17].yyyy, r6.xyzx, r4.xyzx
    r4.xyz = ((source[17].yyyy)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 72: dp3 r3.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r6.xyz, -r4.xyzx, r3.wwww
    r6.xyz = ((-(r4.xyzx))+(r3.wwww)).xyz;
    // 74: mad r4.xyz, cb0[17].zzzz, r6.xyzx, r4.xyzx
    r4.xyz = ((source[17].zzzz)*(r6.xyzx)+(r4.xyzx)).xyz;
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
    // 85: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 86: mul r5.xyz, r5.xyzx, cb0[17].wwww
    r5.xyz = ((r5.xyzx)*(source[17].wwww)).xyz;
    // 87: mad r1.xyz, r1.xxxx, r5.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(r1.yzwy)).xyz;
    // 88: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 89: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 90: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 91: mad r2.xy, cb0[14].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[14].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 92: mul r0.w, cb0[14].y, cb0[20].y
    r0.w = ((source[14].yyyy)*(source[20].yyyy)).w;
    // 93: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 94: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 95: mul r5.y, r0.w, l(0.020000)
    r5.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 96: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 98: mul r1.w, cb0[14].x, l(0.001000)
    r1.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 99: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 100: mad r2.xy, r1.wwww, r2.xyxx, r5.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r5.xyxx)).xy;
    // 101: dp2 r5.y, cb0[16].xyxx, r2.xyxx
    r5.y = (dot((source[16].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 102: dp2 r1.w, cb0[15].xyxx, r2.xyxx
    r1.w = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 103: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 104: mul r5.x, r1.w, l(0.125000)
    r5.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 106: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 107: add r2.w, -cb0[12].w, l(1.000000)
    r2.w = ((-(source[12].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: mul r2.w, r2.w, cb0[20].y
    r2.w = ((r2.wwww)*(source[20].yyyy)).w;
    // 109: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 110: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 111: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul r3.w, cb0[12].z, l(1.500000)
    r3.w = ((source[12].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 113: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 114: mad r2.w, r2.w, l(0.500000), cb0[12].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[12].zzzz)).w;
    // 115: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 116: mul r5.x, r3.w, l(0.125000)
    r5.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 117: mul r6.y, cb0[12].y, cb0[13].y
    r6.y = ((source[12].yyyy)*(source[13].yyyy)).y;
    // 118: mov r5.y, v4.y
    r5.y = (v4.yyyy).y;
    // 119: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 120: add r5.xy, r5.xyxx, r6.xyxx
    r5.xy = ((r5.xyxx)+(r6.xyxx)).xy;
    // 121: frc r3.w, cb0[12].x
    r3.w = (frac(source[12].xxxx)).w;
    // 122: add r4.w, -r3.w, cb0[12].x
    r4.w = ((-(r3.wwww))+(source[12].xxxx)).w;
    // 123: mul r6.z, r4.w, l(0.125000)
    r6.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 124: add r5.xy, r5.xyxx, r6.zwzz
    r5.xy = ((r5.xyxx)+(r6.zwzz)).xy;
    // 125: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 126: mul r5.xyz, r2.wwww, r6.xyzx
    r5.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 127: mul r2.w, r3.w, r6.w
    r2.w = ((r3.wwww)*(r6.wwww)).w;
    // 128: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 129: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 130: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r4.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r4.xyzx))).xyz;
    // 131: mad r2.xyz, r1.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 132: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 133: mad r5.xyz, cb0[14].zzzz, r2.xyzx, -r4.xyzx
    r5.xyz = ((source[14].zzzz)*(r2.xyzx)+(-(r4.xyzx))).xyz;
    // 134: mul r2.xyz, r2.xyzx, cb0[14].zzzz
    r2.xyz = ((r2.xyzx)*(source[14].zzzz)).xyz;
    // 135: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 136: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 137: mad r2.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r2.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 138: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 139: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 140: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 141: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 142: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 143: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 144: mul r4.xyz, r0.wwww, v7.xyzx
    r4.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 145: dp3 r0.w, r4.xyzx, r0.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 146: mul r0.xyz, r0.xyzx, cb0[0].xxxx
    r0.xyz = ((r0.xyzx)*(source[0].xxxx)).xyz;
    // 147: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 148: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 149: mul r4.yzw, r4.yyyy, cb0[22].xxyz
    r4.yzw = ((r4.yyyy)*(source[22].xxyz)).yzw;
    // 150: mad r4.xyz, r4.xxxx, cb0[21].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[21].xyzx)+(r4.yzwy)).xyz;
    // 151: mul r4.xyz, r4.xyzx, cb0[23].wwww
    r4.xyz = ((r4.xyzx)*(source[23].wwww)).xyz;
    // 152: mad r1.xyz, r4.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 153: mul r4.xyz, r2.xyzx, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 154: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 155: mad r1.xyz, r2.xyzx, cb0[23].xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 156: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 157: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 158: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 159: dp3 r0.w, r1.xyzx, cb0[11].xyzx
    r0.w = (dot((r1.xyzx).xyz,(source[11].xyzx).xyz).xxxx).w;
    // 160: dp3_sat r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[11].xyzx
    r1.x = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[11].xyzx).xyz).xxxx)).x;
    // 161: add r0.w, r0.w, -r1.x
    r0.w = ((r0.wwww)+(-(r1.xxxx))).w;
    // 162: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mul r0.w, r0.w, r5.w
    r0.w = ((r0.wwww)*(r5.wwww)).w;
    // 164: mul o0.w, r0.w, cb0[1].w
    output.targets[0].w = ((r0.wwww)*(source[1].wwww)).w;
    // 165: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 166: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 167: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 168: mul r2.xyz, r1.yzxy, r3.zxyz
    r2.xyz = ((r1.yzxy)*(r3.zxyz)).xyz;
    // 169: mad r2.xyz, r3.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r3.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 170: dp3 r3.z, r3.xyzx, r0.xyzx
    r3.z = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 171: dp3 r3.x, r1.xyzx, r0.xyzx
    r3.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 172: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 173: dp3 r3.y, r1.xyzx, r0.xyzx
    r3.y = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 174: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 175: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 176: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 177: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 178: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 179: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 180: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 181: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 182: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 183: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 184: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 185: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 186: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 187: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 188: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 189: ret
    return output;
}

// source.character.equipment-native-172.v1 / source program d2bd493fc7a29f4385d8d4cdf897e264
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase172(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].w=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: mad r4.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r3.xyw, r4.xyxz, r0.wwww
    r3.xyw = ((r4.xyxz)/(r0.wwww)).xyw;
    // 18: dp3 r0.w, r3.xywx, r3.xywx
    r0.w = (dot((r3.xywx).xyz,(r3.xywx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r4.yzw, r0.wwww, r3.xxyw
    r4.yzw = ((r0.wwww)*(r3.xxyw)).yzw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r0.w, r5.w, cb0[1].w
    r0.w = ((r5.wwww)*(source[1].wwww)).w;
    // 23: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 24: mul r6.xyz, r6.xyzx, cb0[17].xxxx
    r6.xyz = ((r6.xyzx)*(source[17].xxxx)).xyz;
    // 25: mad r1.w, cb0[17].y, l(-3.500000), l(5.000000)
    r1.w = ((source[17].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 26: mul r1.w, r1.w, cb0[18].x
    r1.w = ((r1.wwww)*(source[18].xxxx)).w;
    // 27: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: add r6.w, -r2.w, v4.z
    r6.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 29: mad r2.w, cb0[18].y, r6.w, r2.w
    r2.w = ((source[18].yyyy)*(r6.wwww)+(r2.wwww)).w;
    // 30: mul r6.w, r2.w, cb0[18].z
    r6.w = ((r2.wwww)*(source[18].zzzz)).w;
    // 31: mad r2.w, r6.w, l(0.750000), r2.w
    r2.w = ((r6.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 32: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 33: mad_sat r2.w, cb0[19].x, r2.w, r2.w
    r2.w = (saturate((source[19].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 34: mad r4.x, r4.x, r5.x, l(0.200000)
    r4.x = ((r4.xxxx)*(r5.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 35: add r6.w, -r5.y, l(1.000000)
    r6.w = ((-(r5.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: add r6.w, -r4.x, r6.w
    r6.w = ((-(r4.xxxx))+(r6.wwww)).w;
    // 37: mad r7.x, cb0[19].z, r6.w, r4.x
    r7.x = ((source[19].zzzz)*(r6.wwww)+(r4.xxxx)).x;
    // 38: mul r7.y, cb0[18].w, l(0.700000)
    r7.y = ((source[18].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).y;
    // 39: add r7.x, -r2.w, r7.x
    r7.x = ((-(r2.wwww))+(r7.xxxx)).x;
    // 40: mad r7.x, r7.y, r7.x, r2.w
    r7.x = ((r7.yyyy)*(r7.xxxx)+(r2.wwww)).x;
    // 41: div r7.x, r7.x, cb0[19].y
    r7.x = ((r7.xxxx)/(source[19].yyyy)).x;
    // 42: add r7.x, -r7.x, l(1.000000)
    r7.x = ((-(r7.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mul r7.x, r1.w, r7.x
    r7.x = ((r1.wwww)*(r7.xxxx)).x;
    // 44: mul r7.x, r7.x, l(4.000000)
    r7.x = ((r7.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 45: add r7.z, v4.w, l(0.500000)
    r7.z = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 46: round_ni r7.z, r7.z
    r7.z = (floor(r7.zzzz)).z;
    // 47: mul_sat r7.x, r7.z, r7.x
    r7.x = (saturate((r7.zzzz)*(r7.xxxx))).x;
    // 48: mad r4.x, cb0[20].x, r6.w, r4.x
    r4.x = ((source[20].xxxx)*(r6.wwww)+(r4.xxxx)).x;
    // 49: add r4.x, -r2.w, r4.x
    r4.x = ((-(r2.wwww))+(r4.xxxx)).x;
    // 50: mad r2.w, r7.y, r4.x, r2.w
    r2.w = ((r7.yyyy)*(r4.xxxx)+(r2.wwww)).w;
    // 51: div r2.w, r2.w, cb0[19].w
    r2.w = ((r2.wwww)/(source[19].wwww)).w;
    // 52: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 54: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 55: add r2.w, -r7.z, l(1.000000)
    r2.w = ((-(r7.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 57: add r1.w, r1.w, r7.x
    r1.w = ((r1.wwww)+(r7.xxxx)).w;
    // 58: add r1.w, -r5.y, r1.w
    r1.w = ((-(r5.yyyy))+(r1.wwww)).w;
    // 59: mad r1.w, cb0[20].y, r1.w, r5.y
    r1.w = ((source[20].yyyy)*(r1.wwww)+(r5.yyyy)).w;
    // 60: mad r6.xyz, r1.wwww, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 61: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 63: mad r6.xyz, cb0[20].zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((source[20].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 64: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 66: mad r6.xyz, cb0[20].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[20].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 67: mad r7.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 71: mul r9.xyz, r5.xxxx, r8.xyzx
    r9.xyz = ((r5.xxxx)*(r8.xyzx)).xyz;
    // 72: lt r1.w, |r3.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r3.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 73: mul r2.w, |r3.z|, |r3.z|
    r2.w = ((abs(r3.zzzz))*(abs(r3.zzzz))).w;
    // 74: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 75: mad r10.xyz, cb0[7].xyzx, r5.xyzx, -r5.xyzx
    r10.xyz = ((source[7].xyzx)*(r5.xyzx)+(-(r5.xyzx))).xyz;
    // 76: mad r10.xyz, r1.wwww, r10.xyzx, r5.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)+(r5.xyzx)).xyz;
    // 77: mad r8.xyz, -r5.xxxx, r8.xyzx, r10.xyzx
    r8.xyz = ((-(r5.xxxx))*(r8.xyzx)+(r10.xyzx)).xyz;
    // 78: mad r8.xyz, r1.wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((r1.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 79: mul r2.w, cb0[12].z, l(1.500000)
    r2.w = ((source[12].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 80: add r3.z, -cb0[12].w, l(1.000000)
    r3.z = ((-(source[12].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 81: mul r3.z, r3.z, cb0[23].w
    r3.z = ((r3.zzzz)*(source[23].wwww)).z;
    // 82: mul r3.z, r3.z, l(6.283185)
    r3.z = ((r3.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 83: sincos r3.z, null, r3.z
    r3.z = (sin(r3.zzzz)).z;
    // 84: add r3.z, r3.z, l(1.000000)
    r3.z = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 85: mul r2.w, r2.w, r3.z
    r2.w = ((r2.wwww)*(r3.zzzz)).w;
    // 86: mad r2.w, r2.w, l(0.500000), cb0[12].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[12].zzzz)).w;
    // 87: frc r3.z, cb0[12].x
    r3.z = (frac(source[12].xxxx)).z;
    // 88: add r4.x, -r3.z, cb0[12].x
    r4.x = ((-(r3.zzzz))+(source[12].xxxx)).x;
    // 89: mul r9.z, r4.x, l(0.125000)
    r9.z = ((r4.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 90: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 91: mul r9.y, cb0[12].y, cb0[13].y
    r9.y = ((source[12].yyyy)*(source[13].yyyy)).y;
    // 92: mul r11.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r11.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 93: frc r4.x, r11.x
    r4.x = (frac(r11.xxxx)).x;
    // 94: mul r11.y, r4.x, l(0.125000)
    r11.y = ((r4.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 95: add r5.xy, r9.xyxx, r11.yzyy
    r5.xy = ((r9.xyxx)+(r11.yzyy)).xy;
    // 96: add r5.xy, r5.xyxx, r9.zwzz
    r5.xy = ((r5.xyxx)+(r9.zwzz)).xy;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 98: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 99: mul r2.w, r3.z, r9.w
    r2.w = ((r3.zzzz)*(r9.wwww)).w;
    // 100: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 101: mad r8.xyz, r2.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r2.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 102: mul r2.w, cb0[14].y, cb0[23].w
    r2.w = ((source[14].yyyy)*(source[23].wwww)).w;
    // 103: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 104: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 105: mul r5.y, r2.w, l(0.020000)
    r5.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 106: add r9.xyzw, r1.yzxy, -cb0[1].yzxy
    r9.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 107: add r9.xy, -r9.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r9.xy = ((-(r9.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 108: add r9.xy, -r9.zwzz, r9.xyxx
    r9.xy = ((-(r9.zwzz))+(r9.xyxx)).xy;
    // 109: mad r9.xy, cb0[14].wwww, r9.xyxx, r9.zwzz
    r9.xy = ((source[14].wwww)*(r9.xyxx)+(r9.zwzz)).xy;
    // 110: mul r3.z, cb0[14].x, l(0.001000)
    r3.z = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 111: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 112: mad r5.xy, r3.zzzz, r9.xyxx, r5.xyxx
    r5.xy = ((r3.zzzz)*(r9.xyxx)+(r5.xyxx)).xy;
    // 113: dp2 r3.z, cb0[15].xyxx, r5.xyxx
    r3.z = (dot((source[15].xyxx).xy,(r5.xyxx).xy).xxxx).z;
    // 114: dp2 r5.y, cb0[16].xyxx, r5.xyxx
    r5.y = (dot((source[16].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 115: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 116: mul r5.x, r3.z, l(0.125000)
    r5.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 117: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 118: mul r3.z, r9.w, l(0.900000)
    r3.z = ((r9.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).z;
    // 119: mad r9.xyz, r9.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r8.xyzx))).xyz;
    // 120: mad r9.xyz, r3.zzzz, r9.xyzx, r8.xyzx
    r9.xyz = ((r3.zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 121: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 123: mul_sat r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = (saturate((r9.xyzx)*(r2.wwww))).xyz;
    // 124: mul r11.xyz, r9.xyzx, cb0[14].zzzz
    r11.xyz = ((r9.xyzx)*(source[14].zzzz)).xyz;
    // 125: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 127: mad r9.xyz, cb0[14].zzzz, r9.xyzx, -r8.xyzx
    r9.xyz = ((source[14].zzzz)*(r9.xyzx)+(-(r8.xyzx))).xyz;
    // 128: mad r8.xyz, r2.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r2.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 129: mad r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = ((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 130: mad r6.xyz, r6.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r6.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 131: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 132: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 133: div r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)/(r2.wwww)).xyz;
    // 134: mul r5.xyz, r5.zzzz, r6.xyzx
    r5.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 135: mov_sat r2.w, r2.z
    r2.w = (saturate(r2.zzzz)).w;
    // 136: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 137: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 138: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 139: mul r5.xyz, r5.xyzx, cb0[21].xxxx
    r5.xyz = ((r5.xyzx)*(source[21].xxxx)).xyz;
    // 140: mad r6.xyz, cb0[21].yyyy, r10.xyzx, -r5.xyzx
    r6.xyz = ((source[21].yyyy)*(r10.xyzx)+(-(r5.xyzx))).xyz;
    // 141: mad r5.xyz, r1.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 142: add r1.xyz, -r1.xyzx, cb0[0].yzwy
    r1.xyz = ((-(r1.xyzx))+(source[0].yzwy)).xyz;
    // 143: mul r6.xyz, r2.zzzz, r1.xyzx
    r6.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 144: mad r1.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 145: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 146: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 147: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 148: add r1.x, r1.x, cb0[8].z
    r1.x = ((r1.xxxx)+(source[8].zzzz)).x;
    // 149: dp3 r1.y, r0.xyzx, r3.xywx
    r1.y = (dot((r0.xyzx).xyz,(r3.xywx).xyz).xxxx).y;
    // 150: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 151: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 152: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 153: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 154: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 155: add r1.y, -cb0[22].x, l(0.200000)
    r1.y = ((-(source[22].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 156: mad r1.y, r1.w, r1.y, cb0[22].x
    r1.y = ((r1.wwww)*(r1.yyyy)+(source[22].xxxx)).y;
    // 157: mad r1.y, r1.y, l(4.500000), l(0.500000)
    r1.y = ((r1.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 158: mul r1.y, r1.y, cb0[22].y
    r1.y = ((r1.yyyy)*(source[22].yyyy)).y;
    // 159: mul r1.y, r1.y, l(0.050000)
    r1.y = ((r1.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 160: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 161: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 162: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 163: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 164: mul r1.x, r1.x, cb0[22].z
    r1.x = ((r1.xxxx)*(source[22].zzzz)).x;
    // 165: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 166: dp3 r1.y, r3.xywx, r2.xyzx
    r1.y = (dot((r3.xywx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 167: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 168: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: add r2.x, -|r1.y|, l(1.000000)
    r2.x = ((-(abs(r1.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 170: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 171: mad r2.xyw, r1.wwww, cb0[10].xyxz, -cb0[10].xyxz
    r2.xyw = ((r1.wwww)*(source[10].xyxz)+(-(source[10].xyxz))).xyw;
    // 172: mad r2.xyw, cb0[10].wwww, r2.xyxw, cb0[10].xyxz
    r2.xyw = ((source[10].wwww)*(r2.xyxw)+(source[10].xyxz)).xyw;
    // 173: mad r2.xyw, r1.zzzz, cb0[9].xyxz, r2.xyxw
    r2.xyw = ((r1.zzzz)*(source[9].xyxz)+(r2.xyxw)).xyw;
    // 174: mul_sat r1.y, r1.y, cb0[22].w
    r1.y = (saturate((r1.yyyy)*(source[22].wwww))).y;
    // 175: mul_sat r1.z, r2.z, cb0[22].w
    r1.z = (saturate((r2.zzzz)*(source[22].wwww))).z;
    // 176: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 177: add_sat r1.z, r1.z, -cb0[23].x
    r1.z = (saturate((r1.zzzz)+(-(source[23].xxxx)))).z;
    // 178: lt r1.w, r1.z, l(0.000001)
    r1.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 179: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 180: mul r1.z, r1.z, cb0[23].y
    r1.z = ((r1.zzzz)*(source[23].yyyy)).z;
    // 181: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 182: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 183: mul r1.y, r1.y, cb0[11].w
    r1.y = ((r1.yyyy)*(source[11].wwww)).y;
    // 184: mul r3.xyz, r1.yyyy, cb0[11].xyzx
    r3.xyz = ((r1.yyyy)*(source[11].xyzx)).xyz;
    // 185: movc r1.yzw, r1.wwww, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 186: add r1.yzw, r1.yyzw, r2.xxyw
    r1.yzw = ((r1.yyzw)+(r2.xxyw)).yzw;
    // 187: mad r1.xyz, r1.xxxx, r5.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(r1.yzwy)).xyz;
    // 188: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 189: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 190: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 191: mul r2.xyz, r1.wwww, v7.xyzx
    r2.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 192: dp3 r1.w, r2.xyzx, r4.yzwy
    r1.w = (dot((r2.xyzx).xyz,(r4.yzwy).xyz).xxxx).w;
    // 193: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 194: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 195: mul r2.yzw, r2.yyyy, cb0[25].xxyz
    r2.yzw = ((r2.yyyy)*(source[25].xxyz)).yzw;
    // 196: mad r2.xyz, r2.xxxx, cb0[24].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[24].xyzx)+(r2.yzwy)).xyz;
    // 197: mul r2.xyz, r2.xyzx, cb0[26].wwww
    r2.xyz = ((r2.xyzx)*(source[26].wwww)).xyz;
    // 198: mul r3.xyz, r8.xyzx, r2.xyzx
    r3.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 199: mad r1.xyz, r2.xyzx, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 200: mad r1.xyz, r8.xyzx, cb0[26].xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 201: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 202: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 203: eq r2.x, cb0[27].x, l(0.000000)
    r2.x = (asfloat((uint4)((source[27].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 204: not r2.y, r2.x
    r2.y = (asfloat(~asuint(r2.xxxx))).y;
    // 205: lt r2.z, r0.w, r1.w
    r2.z = (asfloat((uint4)((r0.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 206: and r2.y, r2.z, r2.y
    r2.y = (asfloat(asuint(r2.zzzz) & asuint(r2.yyyy))).y;
    // 207: discard_nz r2.y
    if ((asuint(r2.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 208: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 209: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 210: mul r2.yzw, r2.yyyy, v0.xxyz
    r2.yzw = ((r2.yyyy)*(v0.xxyz)).yzw;
    // 211: mul r5.xyz, r0.zxyz, r2.zwyz
    r5.xyz = ((r0.zxyz)*(r2.zwyz)).xyz;
    // 212: mad r5.xyz, r0.yzxy, r2.wyzw, -r5.xyzx
    r5.xyz = ((r0.yzxy)*(r2.wyzw)+(-(r5.xyzx))).xyz;
    // 213: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 214: movc r3.w, v9.x, l(1.000000), l(-1.000000)
    r3.w = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 215: mul r3.w, r3.w, cb0[0].x
    r3.w = ((r3.wwww)*(source[0].xxxx)).w;
    // 216: mul r4.xyz, r3.wwww, r4.yzwy
    r4.xyz = ((r3.wwww)*(r4.yzwy)).xyz;
    // 217: ge r1.w, r0.w, r1.w
    r1.w = (asfloat((uint4)((r0.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 218: mad r3.w, r5.w, cb0[1].w, l(-0.900000)
    r3.w = ((r5.wwww)*(source[1].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 219: mul_sat r3.w, r3.w, l(9.999998)
    r3.w = (saturate((r3.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 220: mad r4.w, r3.w, l(-2.000000), l(3.000000)
    r4.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 221: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 222: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 223: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 224: movc r1.w, r1.w, r3.w, r0.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r3.wwww) : (r0.wwww)).w;
    // 225: movc o0.w, r2.x, r1.w, r0.w
    output.targets[0].w = ((asuint(r2.xxxx) != 0u) ? (r1.wwww) : (r0.wwww)).w;
    // 226: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 227: dp3 r1.x, r2.yzwy, r4.xyzx
    r1.x = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).x;
    // 228: dp3 r1.y, r5.xyzx, r4.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 229: dp3 r1.z, r0.xyzx, r4.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 230: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 231: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 232: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 233: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 234: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 235: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 236: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 237: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 238: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 239: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 240: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 241: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 242: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 243: mov o3.xyz, r8.xyzx
    output.targets[3].xyz = (r8.xyzx).xyz;
    // 244: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 245: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 246: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 247: ret
    return output;
}

// source.character.equipment-native-173.v1 / source program d65e7d6b90f23747be81e5bd2b157357
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase173(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[20]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[28].x=(g_SourceCharacterTime.xxxx).x;
    source[28].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[28].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
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
    // 30: add r0.z, -cb0[24].y, cb0[24].x
    r0.z = ((-(source[24].yyyy))+(source[24].xxxx)).z;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
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
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: add r0.w, -r3.w, l(1.000000)
    r0.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 40: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 41: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 42: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 43: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 44: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 45: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 46: add r1.w, -cb0[23].y, cb0[23].x
    r1.w = ((-(source[23].yyyy))+(source[23].xxxx)).w;
    // 47: mad r1.w, r2.x, r1.w, cb0[23].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[23].yyyy)).w;
    // 48: add r3.w, -r1.w, cb0[23].z
    r3.w = ((-(r1.wwww))+(source[23].zzzz)).w;
    // 49: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 50: add r3.w, -r1.w, cb0[23].w
    r3.w = ((-(r1.wwww))+(source[23].wwww)).w;
    // 51: mad r1.w, r2.z, r3.w, r1.w
    r1.w = ((r2.zzzz)*(r3.wwww)+(r1.wwww)).w;
    // 52: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 53: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 54: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 55: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 56: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 58: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 59: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 60: mul r4.xy, r0.ywyy, cb0[22].xxxx
    r4.xy = ((r0.ywyy)*(source[22].xxxx)).xy;
    // 61: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 62: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 63: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 64: add r4.z, r0.y, l(0.000010)
    r4.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 65: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 66: mad r5.xyz, cb0[22].wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((source[22].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 67: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 68: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 69: div r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)/(r0.yyyy)).xyz;
    // 70: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 71: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 72: mul r6.xyz, r0.yyyy, v0.xyzx
    r6.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 73: dp3 r7.x, r6.xyzx, r5.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 74: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 75: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 76: mul r8.xyz, r0.yyyy, v1.xyzx
    r8.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 77: mul r9.xyz, r6.yzxy, r8.zxyz
    r9.xyz = ((r6.yzxy)*(r8.zxyz)).xyz;
    // 78: mad r9.xyz, r8.yzxy, r6.zxyz, -r9.xyzx
    r9.xyz = ((r8.yzxy)*(r6.zxyz)+(-(r9.xyzx))).xyz;
    // 79: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 80: dp3 r7.y, r9.xyzx, r5.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 81: dp3 r7.z, r8.xyzx, r5.xyzx
    r7.z = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 82: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 83: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 84: mul r5.xyz, r0.yyyy, v5.xyzx
    r5.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 85: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 86: dp3 r11.y, r9.xyzx, r5.xyzx
    r11.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 87: dp3 r11.x, r6.xyzx, r5.xyzx
    r11.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 88: dp3 r11.z, r8.xyzx, r5.xyzx
    r11.z = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 89: dp3 r0.y, r7.xyzx, r11.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 90: mul r7.xyz, r7.xyzx, r0.yyyy
    r7.xyz = ((r7.xyzx)*(r0.yyyy)).xyz;
    // 91: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 92: mov r7.w, -r7.x
    r7.w = (-(r7.xxxx)).w;
    // 93: dp2 r0.y, r7.ywyy, r7.ywyy
    r0.y = (dot((r7.ywyy).xy,(r7.ywyy).xy).xxxx).y;
    // 94: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 95: div r0.yw, r7.yyyw, r0.yyyy
    r0.yw = ((r7.yyyw)/(r0.yyyy)).yw;
    // 96: mad r1.w, -r7.z, l(0.250000), l(0.250000)
    r1.w = ((-(r7.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 97: add r3.w, r7.z, l(1.000000)
    r3.w = ((r7.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 99: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 100: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 101: log r7.xyz, r0.xywx
    r7.xyz = (log2(r0.xywx)).xyz;
    // 102: rcp r1.w, cb0[25].x
    r1.w = (1.0/(source[25].xxxx)).w;
    // 103: mul r11.xyz, r7.xyzx, r1.wwww
    r11.xyz = ((r7.xyzx)*(r1.wwww)).xyz;
    // 104: mul r7.xyz, r7.xyzx, cb0[25].xxxx
    r7.xyz = ((r7.xyzx)*(source[25].xxxx)).xyz;
    // 105: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 106: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 107: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 108: mad r7.xyz, r7.xyzx, cb0[25].xxxx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(source[25].xxxx)+(r11.xyzx)).xyz;
    // 109: add r0.xyw, r0.xyxw, r7.xyxz
    r0.xyw = ((r0.xyxw)+(r7.xyxz)).xyw;
    // 110: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 111: add r1.w, cb0[25].x, l(1.000000)
    r1.w = ((source[25].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 113: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r7.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r7.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 115: mad r7.xyz, r3.wwww, r7.xyzx, cb0[11].xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)+(source[11].xyzx)).xyz;
    // 116: mul r0.xyw, r0.xxxx, r7.xyxz
    r0.xyw = ((r0.xxxx)*(r7.xyxz)).xyw;
    // 117: mul r0.xyw, r0.xyxw, cb0[25].yyyy
    r0.xyw = ((r0.xyxw)*(source[25].yyyy)).xyw;
    // 118: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r7.xyz, -r3.xyzx, r1.wwww
    r7.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 120: mad r3.xyz, cb0[22].yyyy, r7.xyzx, r3.xyzx
    r3.xyz = ((source[22].yyyy)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 121: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r7.xyz, -r3.xyzx, r1.wwww
    r7.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 123: mad r3.xyz, cb0[22].zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((source[22].zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 124: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r7.xyz, -r3.xyzx, r1.wwww
    r7.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 126: mul r7.xyz, r7.xyzx, cb0[25].zzzz
    r7.xyz = ((r7.xyzx)*(source[25].zzzz)).xyz;
    // 127: add r1.w, r2.y, r2.x
    r1.w = ((r2.yyyy)+(r2.xxxx)).w;
    // 128: add r1.w, r2.z, r1.w
    r1.w = ((r2.zzzz)+(r1.wwww)).w;
    // 129: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 130: mad r3.xyz, r1.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 131: max r7.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 132: log r7.xyz, r7.xyzx
    r7.xyz = (log2(r7.xyzx)).xyz;
    // 133: mul r7.xyz, r7.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 134: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 135: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 136: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 137: mul r1.w, r1.w, cb0[26].z
    r1.w = ((r1.wwww)*(source[26].zzzz)).w;
    // 138: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 139: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 142: div r2.w, cb0[26].w, r2.w
    r2.w = ((source[26].wwww)/(r2.wwww)).w;
    // 143: dp3 r3.w, r4.xyzx, r4.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 144: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 145: div r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)/(r3.wwww)).xyz;
    // 146: dp3 r3.w, r4.xyzx, r5.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 147: mul_sat r4.w, r3.w, cb0[25].w
    r4.w = (saturate((r3.wwww)*(source[25].wwww))).w;
    // 148: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mul_sat r5.w, r5.z, cb0[25].w
    r5.w = (saturate((r5.zzzz)*(source[25].wwww))).w;
    // 151: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: add_sat r5.w, r5.w, -cb0[26].x
    r5.w = (saturate((r5.wwww)+(-(source[26].xxxx)))).w;
    // 153: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 154: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 155: mul r6.w, r6.w, cb0[26].y
    r6.w = ((r6.wwww)*(source[26].yyyy)).w;
    // 156: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 157: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 158: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 159: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 160: mul r7.xyz, r0.xywx, r2.wwww
    r7.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 161: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 163: mad r1.xyz, cb0[22].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 164: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 165: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 166: mad r1.xyz, cb0[22].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 167: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 168: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 169: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 170: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 171: mad r2.xyw, r2.yyyy, r12.xyxz, r11.xyxz
    r2.xyw = ((r2.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 172: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, -r2.xywx
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r2.xywx))).xyz;
    // 173: mad r2.xyz, r2.zzzz, r11.xyzx, r2.xywx
    r2.xyz = ((r2.zzzz)*(r11.xyzx)+(r2.xywx)).xyz;
    // 174: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 176: mad r2.xyz, cb0[22].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 177: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 179: mad r2.xyz, cb0[22].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 180: mad r11.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mad r12.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 182: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 183: mul r2.xyz, r2.xyzx, r11.xyzx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 184: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 185: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 186: mad r2.xyz, r3.xyzx, r7.xyzx, -r0.xywx
    r2.xyz = ((r3.xyzx)*(r7.xyzx)+(-(r0.xywx))).xyz;
    // 187: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 188: mul r2.w, r2.w, cb0[27].x
    r2.w = ((r2.wwww)*(source[27].xxxx)).w;
    // 189: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 190: dp3 r2.x, r10.xyzx, r10.xyzx
    r2.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 191: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 192: div r2.yzw, r10.xxyz, r2.yyyy
    r2.yzw = ((r10.xxyz)/(r2.yyyy)).yzw;
    // 193: dp3 r2.y, r2.yzwy, r5.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // 194: add r2.z, -|r5.z|, l(1.000000)
    r2.z = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 195: mul r2.z, r3.w, r2.z
    r2.z = ((r3.wwww)*(r2.zzzz)).z;
    // 196: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 197: mul r2.w, |r2.y|, |r2.y|
    r2.w = ((abs(r2.yyyy))*(abs(r2.yyyy))).w;
    // 198: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 199: mul r2.w, r2.w, |r2.y|
    r2.w = ((r2.wwww)*(abs(r2.yyyy))).w;
    // 200: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 201: movc r2.y, r2.y, l(0), r2.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 202: add r2.w, r2.y, l(-0.027778)
    r2.w = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 203: mad r2.y, r2.y, r2.w, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 204: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 205: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 206: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 207: mad r2.xyw, r0.zzzz, r0.xyxw, -r1.xyxz
    r2.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r1.xyxz))).xyw;
    // 208: mad r1.xyz, r1.wwww, r2.xywx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 209: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 210: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 211: mad r1.xyz, cb0[22].yyyy, r2.xywx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r2.xywx)+(r1.xyzx)).xyz;
    // 212: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 213: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 214: mad r1.xyz, cb0[22].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 215: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 216: add r0.z, -cb0[4].w, l(1.000000)
    r0.z = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 217: mul r0.z, r0.z, cb0[28].x
    r0.z = ((r0.zzzz)*(source[28].xxxx)).z;
    // 218: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 219: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 220: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 221: mul r1.w, cb0[4].z, l(1.500000)
    r1.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 222: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 223: mad r0.z, r0.z, l(0.500000), cb0[4].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).z;
    // 224: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 225: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 226: mul r3.y, cb0[4].y, cb0[17].y
    r3.y = ((source[4].yyyy)*(source[17].yyyy)).y;
    // 227: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 228: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 229: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 230: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 231: add r2.w, -r1.w, cb0[4].x
    r2.w = ((-(r1.wwww))+(source[4].xxxx)).w;
    // 232: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 233: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 234: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 235: mul r2.xyw, r0.zzzz, r3.xyxz
    r2.xyw = ((r0.zzzz)*(r3.xyxz)).xyw;
    // 236: mul r0.z, r1.w, r3.w
    r0.z = ((r1.wwww)*(r3.wwww)).z;
    // 237: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 238: mad r2.xyw, r2.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxz
    r2.xyw = ((r2.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxz))).xyw;
    // 239: mad r1.xyz, r0.zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 240: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 241: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 242: add r2.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 243: add r2.xy, -r3.zwzz, r2.xyxx
    r2.xy = ((-(r3.zwzz))+(r2.xyxx)).xy;
    // 244: mad r2.xy, cb0[18].wwww, r2.xyxx, r3.zwzz
    r2.xy = ((source[18].wwww)*(r2.xyxx)+(r3.zwzz)).xy;
    // 245: mul r0.z, cb0[18].y, cb0[28].x
    r0.z = ((source[18].yyyy)*(source[28].xxxx)).z;
    // 246: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 247: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 248: mul r3.y, r0.z, l(0.020000)
    r3.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 249: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 250: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 251: mul r2.w, cb0[18].x, l(0.001000)
    r2.w = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 252: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 253: mad r2.xy, r2.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 254: dp2 r2.w, cb0[19].xyxx, r2.xyxx
    r2.w = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 255: dp2 r2.y, cb0[20].xyxx, r2.xyxx
    r2.y = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 256: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 257: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 258: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 259: mad r2.xyw, r3.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r3.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 260: mul r3.x, r3.w, l(0.900000)
    r3.x = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 261: mad r2.xyw, r3.xxxx, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.xxxx)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 262: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 263: mad r3.xyz, cb0[18].zzzz, r2.xywx, -r1.xyzx
    r3.xyz = ((source[18].zzzz)*(r2.xywx)+(-(r1.xyzx))).xyz;
    // 264: mul r2.xyw, r2.xyxw, cb0[18].zzzz
    r2.xyw = ((r2.xyxw)*(source[18].zzzz)).xyw;
    // 265: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 266: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 267: mad r1.xyz, r0.zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 268: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 269: mad r2.xyw, r4.wwww, cb0[15].xyxz, -cb0[15].xyxz
    r2.xyw = ((r4.wwww)*(source[15].xyxz)+(-(source[15].xyxz))).xyw;
    // 270: mul r0.z, r4.w, cb0[14].w
    r0.z = ((r4.wwww)*(source[14].wwww)).z;
    // 271: mad r2.xyw, cb0[15].wwww, r2.xyxw, cb0[15].xyxz
    r2.xyw = ((source[15].wwww)*(r2.xyxw)+(source[15].xyxz)).xyw;
    // 272: mad r2.xyw, r0.zzzz, cb0[14].xyxz, r2.xyxw
    r2.xyw = ((r0.zzzz)*(source[14].xyxz)+(r2.xyxw)).xyw;
    // 273: add r0.z, cb0[2].y, cb0[2].x
    r0.z = ((source[2].yyyy)+(source[2].xxxx)).z;
    // 274: add r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)+(source[2].zzzz)).z;
    // 275: add r3.x, -r0.z, l(1000.000000)
    r3.x = ((-(r0.zzzz))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 276: mad r0.z, cb0[28].y, r3.x, r0.z
    r0.z = ((source[28].yyyy)*(r3.xxxx)+(r0.zzzz)).z;
    // 277: mul r0.z, r0.z, l(0.010000)
    r0.z = ((r0.zzzz)*(float4(0.010000,0.010000,0.010000,0.010000))).z;
    // 278: mad r0.z, cb0[27].w, cb0[28].x, r0.z
    r0.z = ((source[27].wwww)*(source[28].xxxx)+(r0.zzzz)).z;
    // 279: mul r3.x, r0.z, l(3.524534)
    r3.x = ((r0.zzzz)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 280: sincos null, r3.x, r3.x
    r3.x = (cos(r3.xxxx)).x;
    // 281: add r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)+(r3.xxxx)).z;
    // 282: mul r0.z, r0.z, l(1.328987)
    r0.z = ((r0.zzzz)*(float4(1.328987,1.328987,1.328987,1.328987))).z;
    // 283: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 284: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 285: mad r0.z, r0.z, l(0.500000), cb0[27].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[27].zzzz)).z;
    // 286: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t7.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 287: mul r5.xyz, cb0[13].xyzx, cb0[27].yyyy
    r5.xyz = ((source[13].xyzx)*(source[27].yyyy)).xyz;
    // 288: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 289: mul r3.xyz, r0.zzzz, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)).xyz;
    // 290: mad r0.xyz, r1.wwww, r0.xywx, r3.xyzx
    r0.xyz = ((r1.wwww)*(r0.xywx)+(r3.xyzx)).xyz;
    // 291: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 292: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 293: mad r0.xyz, cb0[22].yyyy, r3.xyzx, r0.xyzx
    r0.xyz = ((source[22].yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 294: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 295: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 296: mad r0.xyz, cb0[22].zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((source[22].zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 297: mad r0.xyz, r0.xyzx, r11.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.xywx)).xyz;
    // 298: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 299: lt r1.w, |r2.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 300: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 301: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 302: mul r2.xyz, r0.wwww, cb0[16].xyzx
    r2.xyz = ((r0.wwww)*(source[16].xyzx)).xyz;
    // 303: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 304: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 305: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 306: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 307: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 308: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
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
    // 315: mul r3.yzw, r3.yyyy, cb0[30].xxyz
    r3.yzw = ((r3.yyyy)*(source[30].xxyz)).yzw;
    // 316: mad r3.xyz, r3.xxxx, cb0[29].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[29].xyzx)+(r3.yzwy)).xyz;
    // 317: mul r3.xyz, r3.xyzx, cb0[31].wwww
    r3.xyz = ((r3.xyzx)*(source[31].wwww)).xyz;
    // 318: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 319: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 320: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 321: mad o0.xyz, r1.xyzx, cb0[31].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[31].xyzx)+(r0.xyzx)).xyz;
    // 322: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 323: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 324: dp3 r0.x, r6.xyzx, r2.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 325: dp3 r0.z, r8.xyzx, r2.xyzx
    r0.z = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 326: dp3 r0.y, r9.xyzx, r2.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
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

// source.character.equipment-native-174.v1 / source program 273762e670c4084bb723fa4a5eb43374
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase174(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].y=(g_SourceCharacterTime.xxxx).x;
    source[22].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[23].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[23].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
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
    // 16: add r0.z, -cb0[19].y, cb0[19].x
    r0.z = ((-(source[19].yyyy))+(source[19].xxxx)).z;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mad r0.z, r1.x, r0.z, cb0[19].y
    r0.z = ((r1.xxxx)*(r0.zzzz)+(source[19].yyyy)).z;
    // 19: add r0.w, -r0.z, cb0[19].z
    r0.w = ((-(r0.zzzz))+(source[19].zzzz)).w;
    // 20: mad r0.z, r1.y, r0.w, r0.z
    r0.z = ((r1.yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 21: add r0.w, -r0.z, cb0[19].w
    r0.w = ((-(r0.zzzz))+(source[19].wwww)).w;
    // 22: mad r0.z, r1.z, r0.w, r0.z
    r0.z = ((r1.zzzz)*(r0.wwww)+(r0.zzzz)).z;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 26: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 27: mul r0.z, r0.z, r2.w
    r0.z = ((r0.zzzz)*(r2.wwww)).z;
    // 28: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 29: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 31: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 32: add r2.w, -cb0[18].y, cb0[18].x
    r2.w = ((-(source[18].yyyy))+(source[18].xxxx)).w;
    // 33: mad r2.w, r1.x, r2.w, cb0[18].y
    r2.w = ((r1.xxxx)*(r2.wwww)+(source[18].yyyy)).w;
    // 34: add r3.x, -r2.w, cb0[18].z
    r3.x = ((-(r2.wwww))+(source[18].zzzz)).x;
    // 35: mad r2.w, r1.y, r3.x, r2.w
    r2.w = ((r1.yyyy)*(r3.xxxx)+(r2.wwww)).w;
    // 36: add r3.x, -r2.w, cb0[18].w
    r3.x = ((-(r2.wwww))+(source[18].wwww)).x;
    // 37: mad r2.w, r1.z, r3.x, r2.w
    r2.w = ((r1.zzzz)*(r3.xxxx)+(r2.wwww)).w;
    // 38: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 39: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 40: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 41: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 42: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 44: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 45: dp2 r2.w, r0.ywyy, r0.ywyy
    r2.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 46: mul r3.xy, r0.ywyy, cb0[17].xxxx
    r3.xy = ((r0.ywyy)*(source[17].xxxx)).xy;
    // 47: add r0.y, -r2.w, l(1.000000)
    r0.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 49: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 50: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 51: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 52: mad r4.xyz, cb0[17].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[17].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 53: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 54: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 55: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 56: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 57: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 58: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 59: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 60: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 61: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 62: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 63: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 64: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 65: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 66: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 67: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 68: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r4.xyz, r0.yyyy, v6.xyzx
    r4.xyz = ((r0.yyyy)*(v6.xyzx)).xyz;
    // 71: mad r9.xyz, v6.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v6.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 72: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 73: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 74: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 75: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 76: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 77: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 78: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 79: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 80: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 81: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 82: mad r2.w, -r6.z, l(0.250000), l(0.250000)
    r2.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 83: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 85: mad r0.yw, r2.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r2.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 86: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 87: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 88: rcp r2.w, cb0[20].x
    r2.w = (1.0/(source[20].xxxx)).w;
    // 89: mul r10.xyz, r6.xyzx, r2.wwww
    r10.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 90: mul r6.xyz, r6.xyzx, cb0[20].xxxx
    r6.xyz = ((r6.xyzx)*(source[20].xxxx)).xyz;
    // 91: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 92: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 93: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 94: mad r6.xyz, r6.xyzx, cb0[20].xxxx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[20].xxxx)+(r10.xyzx)).xyz;
    // 95: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 96: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 97: add r2.w, cb0[20].x, l(1.000000)
    r2.w = ((source[20].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r0.xyw, r0.xyxw, r2.wwww
    r0.xyw = ((r0.xyxw)*(r2.wwww)).xyw;
    // 99: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 100: add r6.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r6.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 101: mad r6.xyz, r3.wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 102: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 103: mul r0.xyw, r0.xyxw, cb0[20].yyyy
    r0.xyw = ((r0.xyxw)*(source[20].yyyy)).xyw;
    // 104: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 105: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 106: add_sat r1.x, r1.w, r1.x
    r1.x = (saturate((r1.wwww)+(r1.xxxx))).x;
    // 107: dp3 r1.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 108: add r1.yzw, -r2.xxyz, r1.yyyy
    r1.yzw = ((-(r2.xxyz))+(r1.yyyy)).yzw;
    // 109: mad r1.yzw, cb0[17].yyyy, r1.yyzw, r2.xxyz
    r1.yzw = ((source[17].yyyy)*(r1.yyzw)+(r2.xxyz)).yzw;
    // 110: dp3 r2.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 111: add r2.xyz, -r1.yzwy, r2.xxxx
    r2.xyz = ((-(r1.yzwy))+(r2.xxxx)).xyz;
    // 112: mad r1.yzw, cb0[17].zzzz, r2.xxyz, r1.yyzw
    r1.yzw = ((source[17].zzzz)*(r2.xxyz)+(r1.yyzw)).yzw;
    // 113: dp3 r2.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r2.xyz, -r1.yzwy, r2.xxxx
    r2.xyz = ((-(r1.yzwy))+(r2.xxxx)).xyz;
    // 115: mul r2.xyz, r2.xyzx, cb0[20].zzzz
    r2.xyz = ((r2.xyzx)*(source[20].zzzz)).xyz;
    // 116: mad r1.xyz, r1.xxxx, r2.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 117: max r2.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 118: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 119: mul r2.xyz, r2.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 120: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 121: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 123: mul r1.w, r1.w, cb0[21].z
    r1.w = ((r1.wwww)*(source[21].zzzz)).w;
    // 124: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 125: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mad r2.x, -r1.w, r1.w, l(1.000000)
    r2.x = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 127: max r2.x, r2.x, l(0.001000)
    r2.x = (max(r2.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 128: div r2.x, cb0[21].w, r2.x
    r2.x = ((source[21].wwww)/(r2.xxxx)).x;
    // 129: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 130: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 131: div r2.yzw, r3.xxyz, r2.yyyy
    r2.yzw = ((r3.xxyz)/(r2.yyyy)).yzw;
    // 132: dp3 r3.x, r2.yzwy, r4.xyzx
    r3.x = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).x;
    // 133: mul_sat r3.y, r3.x, cb0[20].w
    r3.y = (saturate((r3.xxxx)*(source[20].wwww))).y;
    // 134: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: mul_sat r3.z, r4.z, cb0[20].w
    r3.z = (saturate((r4.zzzz)*(source[20].wwww))).z;
    // 136: add r3.yz, -r3.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r3.yz = ((-(r3.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 137: add_sat r3.z, r3.z, -cb0[21].x
    r3.z = (saturate((r3.zzzz)+(-(source[21].xxxx)))).z;
    // 138: log r3.w, r3.z
    r3.w = (log2(r3.zzzz)).w;
    // 139: lt r3.z, r3.z, l(0.000001)
    r3.z = (asfloat((uint4)((r3.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 140: mul r3.w, r3.w, cb0[21].y
    r3.w = ((r3.wwww)*(source[21].yyyy)).w;
    // 141: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 142: mul r3.y, r3.w, r3.y
    r3.y = ((r3.wwww)*(r3.yyyy)).y;
    // 143: movc r3.y, r3.z, l(0), r3.y
    r3.y = ((asuint(r3.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).y;
    // 144: mul r2.x, r2.x, r3.y
    r2.x = ((r2.xxxx)*(r3.yyyy)).x;
    // 145: mul r6.xyz, r0.xywx, r2.xxxx
    r6.xyz = ((r0.xywx)*(r2.xxxx)).xyz;
    // 146: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 147: dp3 r2.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 148: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.xxxx
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.xxxx)).xyz;
    // 149: mad r10.xyz, cb0[17].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 150: dp3 r2.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 151: add r11.xyz, -r10.xyzx, r2.xxxx
    r11.xyz = ((-(r10.xyzx))+(r2.xxxx)).xyz;
    // 152: mad r10.xyz, cb0[17].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 153: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 154: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 156: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 157: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 158: dp3 r2.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 159: add r13.xyz, -r12.xyzx, r2.xxxx
    r13.xyz = ((-(r12.xyzx))+(r2.xxxx)).xyz;
    // 160: mad r12.xyz, cb0[17].yyyy, r13.xyzx, r12.xyzx
    r12.xyz = ((source[17].yyyy)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 161: dp3 r2.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 162: add r13.xyz, -r12.xyzx, r2.xxxx
    r13.xyz = ((-(r12.xyzx))+(r2.xxxx)).xyz;
    // 163: mad r12.xyz, cb0[17].zzzz, r13.xyzx, r12.xyzx
    r12.xyz = ((source[17].zzzz)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 164: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 165: mul r0.xyw, r0.xyxw, r10.xyxz
    r0.xyw = ((r0.xyxw)*(r10.xyxz)).xyw;
    // 166: mad r1.xyz, r1.xyzx, r6.xyzx, -r0.xywx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 167: add r2.x, -r1.w, l(1.000000)
    r2.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 168: mul r2.x, r2.x, cb0[22].x
    r2.x = ((r2.xxxx)*(source[22].xxxx)).x;
    // 169: mad r0.xyw, r2.xxxx, r1.xyxz, r0.xyxw
    r0.xyw = ((r2.xxxx)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 170: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 171: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 172: div r6.xyz, r9.xyzx, r1.yyyy
    r6.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 173: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 174: add r1.z, -|r4.z|, l(1.000000)
    r1.z = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mul r1.z, r3.x, r1.z
    r1.z = ((r3.xxxx)*(r1.zzzz)).z;
    // 176: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 177: mul r2.x, |r1.y|, |r1.y|
    r2.x = ((abs(r1.yyyy))*(abs(r1.yyyy))).x;
    // 178: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 179: mul r2.x, |r1.y|, r2.x
    r2.x = ((abs(r1.yyyy))*(r2.xxxx)).x;
    // 180: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 181: movc r1.y, r1.y, l(0), r2.x
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).y;
    // 182: add r2.x, r1.y, l(-0.027778)
    r2.x = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).x;
    // 183: mad r1.y, r1.y, r2.x, l(0.027778)
    r1.y = ((r1.yyyy)*(r2.xxxx)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 184: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 185: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 186: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 187: mad r3.xzw, r0.zzzz, r0.xxyw, -r10.xxyz
    r3.xzw = ((r0.zzzz)*(r0.xxyw)+(-(r10.xxyz))).xzw;
    // 188: mad r1.xyw, r1.wwww, r3.xzxw, r10.xyxz
    r1.xyw = ((r1.wwww)*(r3.xzxw)+(r10.xyxz)).xyw;
    // 189: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r3.xzw, -r1.xxyw, r0.zzzz
    r3.xzw = ((-(r1.xxyw))+(r0.zzzz)).xzw;
    // 191: mad r1.xyw, cb0[17].yyyy, r3.xzxw, r1.xyxw
    r1.xyw = ((source[17].yyyy)*(r3.xzxw)+(r1.xyxw)).xyw;
    // 192: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r3.xzw, -r1.xxyw, r0.zzzz
    r3.xzw = ((-(r1.xxyw))+(r0.zzzz)).xzw;
    // 194: mad r1.xyw, cb0[17].zzzz, r3.xzxw, r1.xyxw
    r1.xyw = ((source[17].zzzz)*(r3.xzxw)+(r1.xyxw)).xyw;
    // 195: mul r1.xyw, r11.xyxz, r1.xyxw
    r1.xyw = ((r11.xyxz)*(r1.xyxw)).xyw;
    // 196: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 197: mul r0.z, r0.z, cb0[22].y
    r0.z = ((r0.zzzz)*(source[22].yyyy)).z;
    // 198: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 199: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 200: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 201: mul r2.x, cb0[3].z, l(1.500000)
    r2.x = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 202: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 203: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 204: frc r2.x, v4.x
    r2.x = (frac(v4.xxxx)).x;
    // 205: mul r4.x, r2.x, l(0.125000)
    r4.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 206: mul r6.y, cb0[3].y, cb0[12].y
    r6.y = ((source[3].yyyy)*(source[12].yyyy)).y;
    // 207: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 208: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 209: add r3.xz, r4.xxyx, r6.xxyx
    r3.xz = ((r4.xxyx)+(r6.xxyx)).xz;
    // 210: frc r2.x, cb0[3].x
    r2.x = (frac(source[3].xxxx)).x;
    // 211: add r3.w, -r2.x, cb0[3].x
    r3.w = ((-(r2.xxxx))+(source[3].xxxx)).w;
    // 212: mul r6.z, r3.w, l(0.125000)
    r6.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 213: add r3.xz, r3.xxzx, r6.zzwz
    r3.xz = ((r3.xxzx)+(r6.zzwz)).xz;
    // 214: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xzxx, t6.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 215: mul r3.xzw, r0.zzzz, r4.xxyz
    r3.xzw = ((r0.zzzz)*(r4.xxyz)).xzw;
    // 216: mul r0.z, r2.x, r4.w
    r0.z = ((r2.xxxx)*(r4.wwww)).z;
    // 217: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 218: mad r3.xzw, r3.xxzw, l(2.000000, 0.000000, 2.000000, 2.000000), -r1.xxyw
    r3.xzw = ((r3.xxzw)*(float4(2.000000,0.000000,2.000000,2.000000))+(-(r1.xxyw))).xzw;
    // 219: mad r1.xyw, r0.zzzz, r3.xzxw, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r3.xzxw)+(r1.xyxw)).xyw;
    // 220: add r4.xyzw, v8.yzxy, cb0[0].yzxy
    r4.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 221: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 222: add r3.xz, -r4.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r3.xz = ((-(r4.xxyx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 223: add r3.xz, -r4.zzwz, r3.xxzx
    r3.xz = ((-(r4.zzwz))+(r3.xxzx)).xz;
    // 224: mad r3.xz, cb0[13].wwww, r3.xxzx, r4.zzwz
    r3.xz = ((source[13].wwww)*(r3.xxzx)+(r4.zzwz)).xz;
    // 225: mul r0.z, cb0[13].y, cb0[22].y
    r0.z = ((source[13].yyyy)*(source[22].yyyy)).z;
    // 226: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 227: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 228: mul r4.y, r0.z, l(0.020000)
    r4.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 229: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 230: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 231: mul r3.w, cb0[13].x, l(0.001000)
    r3.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 232: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 233: mad r3.xz, r3.wwww, r3.xxzx, r4.xxyx
    r3.xz = ((r3.wwww)*(r3.xxzx)+(r4.xxyx)).xz;
    // 234: dp2 r3.w, cb0[14].xyxx, r3.xzxx
    r3.w = (dot((source[14].xyxx).xy,(r3.xzxx).xy).xxxx).w;
    // 235: dp2 r4.y, cb0[15].xyxx, r3.xzxx
    r4.y = (dot((source[15].xyxx).xy,(r3.xzxx).xy).xxxx).y;
    // 236: frc r3.x, r3.w
    r3.x = (frac(r3.wwww)).x;
    // 237: mul r4.x, r3.x, l(0.125000)
    r4.x = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 238: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t6.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 239: mad r3.xzw, r4.xxyz, l(3.500000, 0.000000, 3.500000, 3.500000), -r1.xxyw
    r3.xzw = ((r4.xxyz)*(float4(3.500000,0.000000,3.500000,3.500000))+(-(r1.xxyw))).xzw;
    // 240: mul r4.x, r4.w, l(0.900000)
    r4.x = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 241: mad r3.xzw, r4.xxxx, r3.xxzw, r1.xxyw
    r3.xzw = ((r4.xxxx)*(r3.xxzw)+(r1.xxyw)).xzw;
    // 242: mul_sat r3.xzw, r0.zzzz, r3.xxzw
    r3.xzw = (saturate((r0.zzzz)*(r3.xxzw))).xzw;
    // 243: mad r4.xyz, cb0[13].zzzz, r3.xzwx, -r1.xywx
    r4.xyz = ((source[13].zzzz)*(r3.xzwx)+(-(r1.xywx))).xyz;
    // 244: mul r3.xzw, r3.xxzw, cb0[13].zzzz
    r3.xzw = ((r3.xxzw)*(source[13].zzzz)).xzw;
    // 245: dp3 r0.z, r3.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 246: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 247: mad r1.xyw, r0.zzzz, r4.xyxz, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r4.xyxz)+(r1.xyxw)).xyw;
    // 248: mad r1.xyw, r1.xyxw, cb2[3].wwww, cb2[3].xyxz
    r1.xyw = ((r1.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 249: mad r3.xzw, r3.yyyy, cb0[10].xxyz, -cb0[10].xxyz
    r3.xzw = ((r3.yyyy)*(source[10].xxyz)+(-(source[10].xxyz))).xzw;
    // 250: mul r0.z, r3.y, cb0[9].w
    r0.z = ((r3.yyyy)*(source[9].wwww)).z;
    // 251: mad r3.xyz, cb0[10].wwww, r3.xzwx, cb0[10].xyzx
    r3.xyz = ((source[10].wwww)*(r3.xzwx)+(source[10].xyzx)).xyz;
    // 252: mad r3.xyz, r0.zzzz, cb0[9].xyzx, r3.xyzx
    r3.xyz = ((r0.zzzz)*(source[9].xyzx)+(r3.xyzx)).xyz;
    // 253: mul r4.xyz, r0.xywx, r2.xxxx
    r4.xyz = ((r0.xywx)*(r2.xxxx)).xyz;
    // 254: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 255: mad r0.xyz, -r2.xxxx, r0.xywx, r0.zzzz
    r0.xyz = ((-(r2.xxxx))*(r0.xywx)+(r0.zzzz)).xyz;
    // 256: mad r0.xyz, cb0[17].yyyy, r0.xyzx, r4.xyzx
    r0.xyz = ((source[17].yyyy)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 257: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 259: mad r0.xyz, cb0[17].zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 260: mad r0.xyz, r0.xyzx, r11.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r3.xyzx)).xyz;
    // 261: log r0.w, |r1.z|
    r0.w = (log2(abs(r1.zzzz))).w;
    // 262: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 263: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 264: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 265: mul r3.xyz, r0.wwww, cb0[11].xyzx
    r3.xyz = ((r0.wwww)*(source[11].xyzx)).xyz;
    // 266: movc r3.xyz, r1.zzzz, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 267: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 268: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 269: dp3 r0.w, r2.yzwy, r2.yzwy
    r0.w = (dot((r2.yzwy).xyz,(r2.yzwy).xyz).xxxx).w;
    // 270: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 271: mul r2.xyz, r0.wwww, r2.yzwy
    r2.xyz = ((r0.wwww)*(r2.yzwy)).xyz;
    // 272: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 273: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 274: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 275: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 276: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 277: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 278: mul r3.yzw, r3.yyyy, cb0[25].xxyz
    r3.yzw = ((r3.yyyy)*(source[25].xxyz)).yzw;
    // 279: mad r3.xyz, r3.xxxx, cb0[24].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[24].xyzx)+(r3.yzwy)).xyz;
    // 280: mul r3.xyz, r3.xyzx, cb0[26].wwww
    r3.xyz = ((r3.xyzx)*(source[26].wwww)).xyz;
    // 281: mad r0.xyz, r3.xyzx, r1.xywx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xywx)+(r0.xyzx)).xyz;
    // 282: mul r3.xyz, r1.xywx, r3.xyzx
    r3.xyz = ((r1.xywx)*(r3.xyzx)).xyz;
    // 283: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 284: mad r0.xyz, r1.xywx, cb0[26].xyzx, r0.xyzx
    r0.xyz = ((r1.xywx)*(source[26].xyzx)+(r0.xyzx)).xyz;
    // 285: mov o3.xyz, r1.xywx
    output.targets[3].xyz = (r1.xywx).xyz;
    // 286: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mul r1.xyzw, r0.xyzw, cb0[16].xyzw
    r1.xyzw = ((r0.xyzw)*(source[16].xyzw)).xyzw;
    // 289: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 290: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 291: add r0.yz, r1.yywy, r1.xxzx
    r0.yz = ((r1.yywy)+(r1.xxzx)).yz;
    // 292: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 293: add r0.y, r0.y, l(-1.000000)
    r0.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 294: mad_sat r0.x, r0.x, r0.y, l(1.000000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 295: mul_sat r0.x, r12.w, r0.x
    r0.x = (saturate((r12.wwww)*(r0.xxxx))).x;
    // 296: mul_sat r0.x, r0.x, cb0[23].z
    r0.x = (saturate((r0.xxxx)*(source[23].zzzz))).x;
    // 297: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 298: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 299: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 300: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 301: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 302: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 303: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 304: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 305: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 306: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 307: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 308: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 309: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 310: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 311: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 312: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 313: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 314: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 315: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 316: ret
    return output;
}

// source.character.equipment-native-175.v1 / source program 04c7805c5d23d84281292679821bda18
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase175(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[21]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[29].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 22: add r1.y, -cb0[23].y, cb0[23].x
    r1.y = ((-(source[23].yyyy))+(source[23].xxxx)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: mad r1.y, r2.x, r1.y, cb0[23].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[23].yyyy)).y;
    // 25: add r1.z, -r1.y, cb0[23].z
    r1.z = ((-(r1.yyyy))+(source[23].zzzz)).z;
    // 26: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 27: add r1.z, -r1.y, cb0[23].w
    r1.z = ((-(r1.yyyy))+(source[23].wwww)).z;
    // 28: mad r1.y, r2.z, r1.z, r1.y
    r1.y = ((r2.zzzz)*(r1.zzzz)+(r1.yyyy)).y;
    // 29: add r1.z, -r1.y, cb0[24].x
    r1.z = ((-(r1.yyyy))+(source[24].xxxx)).z;
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
    // 40: add r1.w, -r1.z, cb0[26].x
    r1.w = ((-(r1.zzzz))+(source[26].xxxx)).w;
    // 41: mad r1.z, r2.w, r1.w, r1.z
    r1.z = ((r2.wwww)*(r1.wwww)+(r1.zzzz)).z;
    // 42: add r1.w, -cb0[25].y, cb0[25].x
    r1.w = ((-(source[25].yyyy))+(source[25].xxxx)).w;
    // 43: mad r1.w, r2.x, r1.w, cb0[25].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[25].yyyy)).w;
    // 44: add r3.w, -r1.w, cb0[25].z
    r3.w = ((-(r1.wwww))+(source[25].zzzz)).w;
    // 45: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 46: add r3.w, -r1.w, cb0[25].w
    r3.w = ((-(r1.wwww))+(source[25].wwww)).w;
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
    // 60: mul r5.xy, r4.xyxx, cb0[22].xxxx
    r5.xy = ((r4.xyxx)*(source[22].xxxx)).xy;
    // 61: mad r4.xy, cb0[22].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[22].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 62: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 63: mad r1.xzw, r2.wwww, r4.xxyz, r5.xxyz
    r1.xzw = ((r2.wwww)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 64: add r4.xyz, -r1.xzwx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xzwx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 65: mad r4.xyz, cb0[24].wwww, r4.xyzx, r1.xzwx
    r4.xyz = ((source[24].wwww)*(r4.xyzx)+(r1.xzwx)).xyz;
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
    // 99: sample_l_indexable(texture2d)(float,float,float,float) r8.xyz, r8.xyxx, t4.xyzw, s4, r0.x
    r8.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r8.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 100: log r10.xyz, r8.xyzx
    r10.xyz = (log2(r8.xyzx)).xyz;
    // 101: rcp r0.x, cb0[26].y
    r0.x = (1.0/(source[26].yyyy)).x;
    // 102: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 103: mul r10.xyz, r10.xyzx, cb0[26].yyyy
    r10.xyz = ((r10.xyzx)*(source[26].yyyy)).xyz;
    // 104: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 105: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 106: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 107: mad r10.xyz, r10.xyzx, cb0[26].yyyy, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[26].yyyy)+(r11.xyzx)).xyz;
    // 108: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 109: mul r8.xyz, r8.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 110: add r0.x, cb0[26].y, l(1.000000)
    r0.x = ((source[26].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 112: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: add r8.xyz, -cb0[12].xyzx, cb0[13].xyzx
    r8.xyz = ((-(source[12].xyzx))+(source[13].xyzx)).xyz;
    // 114: mad r8.xyz, r4.wwww, r8.xyzx, cb0[12].xyzx
    r8.xyz = ((r4.wwww)*(r8.xyzx)+(source[12].xyzx)).xyz;
    // 115: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 116: mul r8.xyz, r8.xyzx, cb0[26].zzzz
    r8.xyz = ((r8.xyzx)*(source[26].zzzz)).xyz;
    // 117: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 118: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 119: mad r3.yzw, cb0[24].yyyy, r10.xxyz, r3.xxyz
    r3.yzw = ((source[24].yyyy)*(r10.xxyz)+(r3.xxyz)).yzw;
    // 120: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r10.xyz, -r3.yzwy, r0.xxxx
    r10.xyz = ((-(r3.yzwy))+(r0.xxxx)).xyz;
    // 122: mad r3.yzw, cb0[24].zzzz, r10.xxyz, r3.yyzw
    r3.yzw = ((source[24].zzzz)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 123: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 124: add r10.xyz, -r3.yzwy, r0.xxxx
    r10.xyz = ((-(r3.yzwy))+(r0.xxxx)).xyz;
    // 125: mul r10.xyz, r10.xyzx, cb0[26].wwww
    r10.xyz = ((r10.xyzx)*(source[26].wwww)).xyz;
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
    // 138: add r3.w, cb0[27].w, -cb0[28].x
    r3.w = ((source[27].wwww)+(-(source[28].xxxx))).w;
    // 139: mad r3.w, r2.w, r3.w, cb0[28].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[28].xxxx)).w;
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
    // 145: div r3.w, cb0[28].y, r3.w
    r3.w = ((source[28].yyyy)/(r3.wwww)).w;
    // 146: dp3 r4.w, r1.xzwx, r1.xzwx
    r4.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 147: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 148: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 149: dp3 r4.w, r1.xzwx, r4.xyzx
    r4.w = (dot((r1.xzwx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 150: mul_sat r5.w, r4.w, cb0[27].x
    r5.w = (saturate((r4.wwww)*(source[27].xxxx))).w;
    // 151: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mul_sat r6.w, r4.z, cb0[27].x
    r6.w = (saturate((r4.zzzz)*(source[27].xxxx))).w;
    // 154: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: add_sat r6.w, r6.w, -cb0[27].y
    r6.w = (saturate((r6.wwww)+(-(source[27].yyyy)))).w;
    // 156: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 157: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 158: mul r7.w, r7.w, cb0[27].z
    r7.w = ((r7.wwww)*(source[27].zzzz)).w;
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
    // 166: mad r0.yzw, cb0[24].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[24].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 167: dp3 r3.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 168: add r11.xyz, -r0.yzwy, r3.wwww
    r11.xyz = ((-(r0.yzwy))+(r3.wwww)).xyz;
    // 169: mad r0.yzw, cb0[24].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[24].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 170: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 171: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 172: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 173: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 174: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 175: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, -r11.xyzx
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r11.xyzx))).xyz;
    // 176: mad r2.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r2.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 177: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 179: mad r2.xyz, cb0[24].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[24].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 180: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 182: mad r2.xyz, cb0[24].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[24].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 183: mad r11.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mad r12.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 186: mul r12.xyz, r2.xyzx, r11.xyzx
    r12.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 187: mad r2.xyz, -r2.xyzx, r11.xyzx, cb0[11].xyzx
    r2.xyz = ((-(r2.xyzx))*(r11.xyzx)+(source[11].xyzx)).xyz;
    // 188: mad r2.xyz, r2.wwww, r2.xyzx, r12.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r12.xyzx)).xyz;
    // 189: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 190: mul r2.xyz, r8.xyzx, r0.yzwy
    r2.xyz = ((r8.xyzx)*(r0.yzwy)).xyz;
    // 191: mad r3.xyz, r3.xyzx, r10.xyzx, -r2.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r2.xyzx))).xyz;
    // 192: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: mul r2.w, r2.w, cb0[28].z
    r2.w = ((r2.wwww)*(source[28].zzzz)).w;
    // 194: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 195: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 196: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 197: div r3.xyz, r9.xyzx, r3.xxxx
    r3.xyz = ((r9.xyzx)/(r3.xxxx)).xyz;
    // 198: dp3 r3.x, r3.xyzx, r4.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 199: add r3.y, -|r4.z|, l(1.000000)
    r3.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: mul r3.y, r4.w, r3.y
    r3.y = ((r4.wwww)*(r3.yyyy)).y;
    // 201: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 202: mul r3.z, |r3.x|, |r3.x|
    r3.z = ((abs(r3.xxxx))*(abs(r3.xxxx))).z;
    // 203: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 204: mul r3.z, r3.z, |r3.x|
    r3.z = ((r3.zzzz)*(abs(r3.xxxx))).z;
    // 205: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 206: movc r3.x, r3.x, l(0), r3.z
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).x;
    // 207: add r3.z, r3.x, l(-0.027778)
    r3.z = ((r3.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 208: mad r3.x, r3.x, r3.z, l(0.027778)
    r3.x = ((r3.xxxx)*(r3.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 209: div_sat r2.w, r3.x, r2.w
    r2.w = (saturate((r3.xxxx)/(r2.wwww))).w;
    // 210: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 211: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 212: mad r3.xzw, r1.yyyy, r2.xxyz, -r0.yyzw
    r3.xzw = ((r1.yyyy)*(r2.xxyz)+(-(r0.yyzw))).xzw;
    // 213: mad r0.xyz, r0.xxxx, r3.xzwx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xzwx)+(r0.yzwy)).xyz;
    // 214: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 215: add r3.xzw, -r0.xxyz, r0.wwww
    r3.xzw = ((-(r0.xxyz))+(r0.wwww)).xzw;
    // 216: mad r0.xyz, cb0[24].yyyy, r3.xzwx, r0.xyzx
    r0.xyz = ((source[24].yyyy)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 217: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 218: add r3.xzw, -r0.xxyz, r0.wwww
    r3.xzw = ((-(r0.xxyz))+(r0.wwww)).xzw;
    // 219: mad r0.xyz, cb0[24].zzzz, r3.xzwx, r0.xyzx
    r0.xyz = ((source[24].zzzz)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 220: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 221: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 222: mul r0.w, r0.w, cb0[29].z
    r0.w = ((r0.wwww)*(source[29].zzzz)).w;
    // 223: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 224: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 225: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mul r1.y, cb0[4].z, l(1.500000)
    r1.y = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 227: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 228: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 229: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 230: mul r4.x, r1.y, l(0.125000)
    r4.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 231: mul r8.y, cb0[4].y, cb0[18].y
    r8.y = ((source[4].yyyy)*(source[18].yyyy)).y;
    // 232: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 233: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 234: add r3.xz, r4.xxyx, r8.xxyx
    r3.xz = ((r4.xxyx)+(r8.xxyx)).xz;
    // 235: frc r1.y, cb0[4].x
    r1.y = (frac(source[4].xxxx)).y;
    // 236: add r2.w, -r1.y, cb0[4].x
    r2.w = ((-(r1.yyyy))+(source[4].xxxx)).w;
    // 237: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: add r3.xz, r3.xxzx, r8.zzwz
    r3.xz = ((r3.xxzx)+(r8.zzwz)).xz;
    // 239: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xzxx, t5.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 240: mul r3.xzw, r0.wwww, r4.xxyz
    r3.xzw = ((r0.wwww)*(r4.xxyz)).xzw;
    // 241: mul r0.w, r1.y, r4.w
    r0.w = ((r1.yyyy)*(r4.wwww)).w;
    // 242: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 243: mad r3.xzw, r3.xxzw, l(2.000000, 0.000000, 2.000000, 2.000000), -r0.xxyz
    r3.xzw = ((r3.xxzw)*(float4(2.000000,0.000000,2.000000,2.000000))+(-(r0.xxyz))).xzw;
    // 244: mad r0.xyz, r0.wwww, r3.xzwx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 245: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 246: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 247: add r3.xz, -r4.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r3.xz = ((-(r4.xxyx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 248: add r3.xz, -r4.zzwz, r3.xxzx
    r3.xz = ((-(r4.zzwz))+(r3.xxzx)).xz;
    // 249: mad r3.xz, cb0[19].wwww, r3.xxzx, r4.zzwz
    r3.xz = ((source[19].wwww)*(r3.xxzx)+(r4.zzwz)).xz;
    // 250: mul r0.w, cb0[19].y, cb0[29].z
    r0.w = ((source[19].yyyy)*(source[29].zzzz)).w;
    // 251: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 252: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 253: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 254: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 255: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 256: mul r2.w, cb0[19].x, l(0.001000)
    r2.w = ((source[19].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 257: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 258: mad r3.xz, r2.wwww, r3.xxzx, r4.xxyx
    r3.xz = ((r2.wwww)*(r3.xxzx)+(r4.xxyx)).xz;
    // 259: dp2 r2.w, cb0[20].xyxx, r3.xzxx
    r2.w = (dot((source[20].xyxx).xy,(r3.xzxx).xy).xxxx).w;
    // 260: dp2 r4.y, cb0[21].xyxx, r3.xzxx
    r4.y = (dot((source[21].xyxx).xy,(r3.xzxx).xy).xxxx).y;
    // 261: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 262: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 263: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 264: mad r3.xzw, r4.xxyz, l(3.500000, 0.000000, 3.500000, 3.500000), -r0.xxyz
    r3.xzw = ((r4.xxyz)*(float4(3.500000,0.000000,3.500000,3.500000))+(-(r0.xxyz))).xzw;
    // 265: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 266: mad r3.xzw, r2.wwww, r3.xxzw, r0.xxyz
    r3.xzw = ((r2.wwww)*(r3.xxzw)+(r0.xxyz)).xzw;
    // 267: mul_sat r3.xzw, r0.wwww, r3.xxzw
    r3.xzw = (saturate((r0.wwww)*(r3.xxzw))).xzw;
    // 268: mad r4.xyz, cb0[19].zzzz, r3.xzwx, -r0.xyzx
    r4.xyz = ((source[19].zzzz)*(r3.xzwx)+(-(r0.xyzx))).xyz;
    // 269: mul r3.xzw, r3.xxzw, cb0[19].zzzz
    r3.xzw = ((r3.xxzw)*(source[19].zzzz)).xzw;
    // 270: dp3 r0.w, r3.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 271: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 272: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 273: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 274: mad r3.xzw, r5.wwww, cb0[16].xxyz, -cb0[16].xxyz
    r3.xzw = ((r5.wwww)*(source[16].xxyz)+(-(source[16].xxyz))).xzw;
    // 275: mul r0.w, r5.w, cb0[15].w
    r0.w = ((r5.wwww)*(source[15].wwww)).w;
    // 276: mad r3.xzw, cb0[16].wwww, r3.xxzw, cb0[16].xxyz
    r3.xzw = ((source[16].wwww)*(r3.xxzw)+(source[16].xxyz)).xzw;
    // 277: mad r3.xzw, r0.wwww, cb0[15].xxyz, r3.xxzw
    r3.xzw = ((r0.wwww)*(source[15].xxyz)+(r3.xxzw)).xzw;
    // 278: add r0.w, cb0[2].y, cb0[2].x
    r0.w = ((source[2].yyyy)+(source[2].xxxx)).w;
    // 279: add r0.w, r0.w, cb0[2].z
    r0.w = ((r0.wwww)+(source[2].zzzz)).w;
    // 280: add r2.w, -r0.w, l(1000.000000)
    r2.w = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 281: mad r0.w, cb0[29].w, r2.w, r0.w
    r0.w = ((source[29].wwww)*(r2.wwww)+(r0.wwww)).w;
    // 282: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 283: mad r0.w, cb0[29].y, cb0[29].z, r0.w
    r0.w = ((source[29].yyyy)*(source[29].zzzz)+(r0.wwww)).w;
    // 284: mul r2.w, r0.w, l(3.524534)
    r2.w = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 285: sincos null, r2.w, r2.w
    r2.w = (cos(r2.wwww)).w;
    // 286: add r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)+(r2.wwww)).w;
    // 287: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 288: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 289: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 290: mad r0.w, r0.w, l(0.500000), cb0[29].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[29].xxxx)).w;
    // 291: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 292: mul r8.xyz, cb0[14].xyzx, cb0[28].wwww
    r8.xyz = ((source[14].xyzx)*(source[28].wwww)).xyz;
    // 293: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 294: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 295: mad r2.xyz, r1.yyyy, r2.xyzx, r4.xyzx
    r2.xyz = ((r1.yyyy)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 296: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 297: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 298: mad r2.xyz, cb0[24].yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((source[24].yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 299: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 300: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 301: mad r2.xyz, cb0[24].zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((source[24].zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 302: mad r2.xyz, r2.xyzx, r11.xyzx, r3.xzwx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)+(r3.xzwx)).xyz;
    // 303: log r0.w, |r3.y|
    r0.w = (log2(abs(r3.yyyy))).w;
    // 304: lt r1.y, |r3.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 305: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 306: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 307: mul r3.xyz, r0.wwww, cb0[17].xyzx
    r3.xyz = ((r0.wwww)*(source[17].xyzx)).xyz;
    // 308: movc r3.xyz, r1.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 309: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 310: add r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 311: dp3 r0.w, r1.xzwx, r1.xzwx
    r0.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 312: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 313: mul r1.xyz, r0.wwww, r1.xzwx
    r1.xyz = ((r0.wwww)*(r1.xzwx)).xyz;
    // 314: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 315: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 316: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 317: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 318: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 319: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 320: mul r3.yzw, r3.yyyy, cb0[31].xxyz
    r3.yzw = ((r3.yyyy)*(source[31].xxyz)).yzw;
    // 321: mad r3.xyz, r3.xxxx, cb0[30].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[30].xyzx)+(r3.yzwy)).xyz;
    // 322: mul r3.xyz, r3.xyzx, cb0[32].wwww
    r3.xyz = ((r3.xyzx)*(source[32].wwww)).xyz;
    // 323: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 324: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 325: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 326: mad o0.xyz, r0.xyzx, cb0[32].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[32].xyzx)+(r2.xyzx)).xyz;
    // 327: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 328: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 329: dp3 r0.x, r6.xyzx, r1.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 330: dp3 r0.z, r5.xyzx, r1.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 331: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 332: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 333: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 334: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 335: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 336: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 337: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 338: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 339: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 340: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 341: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 342: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 343: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 344: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 345: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 346: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 347: ret
    return output;
}

// source.character.equipment-native-176.v1 / source program fb2ab658b1245644bacd9715fe77050e
