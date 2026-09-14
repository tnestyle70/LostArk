SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase84(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0].x = 1.0; // Source primitive opacity multiplier.
    source[7]=SourceCharacterAppend(frac((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),frac((g_SourceCharacterTime.xxxx*float4(0,0,0,0))),1u);
    source[8]=SourceCharacterAppend(frac((g_SourceCharacterTime.xxxx*float4(0,0,0,0))),frac((g_SourceCharacterTime.xxxx*float4(0.300000012,0,0,0))),1u);
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].z=(g_SourceCharacterTime.xxxx).x;
    source[15].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[16].x=(frac((g_SourceCharacterTime.xxxx*float4(0.300000012,0,0,0)))).x;
    source[17].x=(((float4(1.5,0,0,0)+sin(((float4(0.5,0,0,0)*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[17].z=(((source[16].wwww*float4(2,0,0,0))*((float4(1.5,0,0,0)+sin(((float4(0.5,0,0,0)*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 8: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 9: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 10: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 12: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 13: mul r2.xyz, r0.wwww, r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 15: add r4.xy, v4.xyxx, cb0[7].xyxx
    r4.xy = ((v4.xyxx)+(source[7].xyxx)).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 17: add r5.xy, v4.xyxx, cb0[8].xyxx
    r5.xy = ((v4.xyxx)+(source[8].xyxx)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 19: add r4.xyzw, r4.xxyz, r5.xxyz
    r4.xyzw = ((r4.xxyz)+(r5.xxyz)).xyzw;
    // 20: max r0.w, r0.z, l(0.000000)
    r0.w = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 23: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 24: mul r0.w, r0.w, l(0.300000)
    r0.w = ((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))).w;
    // 25: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 26: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 27: lt r1.w, r0.w, l(0.000001)
    r1.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 29: mul r0.w, r0.w, cb0[17].w
    r0.w = ((r0.wwww)*(source[17].wwww)).w;
    // 30: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 31: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: movc r0.w, r1.w, l(1.000000), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.wwww)).w;
    // 33: add r0.w, -r4.x, r0.w
    r0.w = ((-(r4.xxxx))+(r0.wwww)).w;
    // 34: mad r0.w, r0.w, l(0.800000), r4.x
    r0.w = ((r0.wwww)*(float4(0.800000,0.800000,0.800000,0.800000))+(r4.xxxx)).w;
    // 35: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 36: frc r1.w, cb0[11].x
    r1.w = (frac(source[11].xxxx)).w;
    // 37: add r2.w, -cb0[18].x, cb0[18].y
    r2.w = ((-(source[18].xxxx))+(source[18].yyyy)).w;
    // 38: mad r2.w, r1.w, r2.w, cb0[18].x
    r2.w = ((r1.wwww)*(r2.wwww)+(source[18].xxxx)).w;
    // 39: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 40: mul_sat r0.w, r0.w, cb0[18].w
    r0.w = (saturate((r0.wwww)*(source[18].wwww))).w;
    // 41: mul r2.w, r0.w, cb0[0].x
    r2.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 42: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 44: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 45: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 47: mul r0.z, |r0.x|, |r0.x|
    r0.z = ((abs(r0.xxxx))*(abs(r0.xxxx))).z;
    // 48: mul r1.x, r0.z, |r0.x|
    r1.x = ((r0.zzzz)*(abs(r0.xxxx))).x;
    // 49: movc r1.x, r0.y, l(0), r1.x
    r1.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 50: lt r1.y, r1.x, l(0.000001)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: log r1.z, r1.x
    r1.z = (log2(r1.xxxx)).z;
    // 52: mul r1.z, r1.z, cb0[15].x
    r1.z = ((r1.zzzz)*(source[15].xxxx)).z;
    // 53: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 54: mul r6.xyz, r1.zzzz, cb0[4].xyzx
    r6.xyz = ((r1.zzzz)*(source[4].xyzx)).xyz;
    // 55: mul r6.xyz, r6.xyzx, cb0[15].yyyy
    r6.xyz = ((r6.xyzx)*(source[15].yyyy)).xyz;
    // 56: movc r6.xyz, r1.yyyy, l(0,0,0,0), r6.xyzx
    r6.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xyzx)).xyz;
    // 57: mul r7.xyz, r3.xyzx, cb0[5].xyzx
    r7.xyz = ((r3.xyzx)*(source[5].xyzx)).xyz;
    // 58: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 59: mul r0.x, r0.x, l(0.800000)
    r0.x = ((r0.xxxx)*(float4(0.800000,0.800000,0.800000,0.800000))).x;
    // 60: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 61: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 62: movc r0.x, r0.y, l(1.000000), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (r0.xxxx)).x;
    // 63: mul r8.xyz, r4.yzwy, cb0[6].xyzx
    r8.xyz = ((r4.yzwy)*(source[6].xyzx)).xyz;
    // 64: mad r7.xyz, r0.xxxx, r8.xyzx, r7.xyzx
    r7.xyz = ((r0.xxxx)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 65: add r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)+(r7.xyzx)).xyz;
    // 66: mad r3.xyz, r3.xyzx, cb0[5].xyzx, -r6.xyzx
    r3.xyz = ((r3.xyzx)*(source[5].xyzx)+(-(r6.xyzx))).xyz;
    // 67: mad r3.xyz, r3.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r6.xyzx
    r3.xyz = ((r3.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r6.xyzx)).xyz;
    // 68: mul r6.xyz, r3.xyzx, cb0[5].xyzx
    r6.xyz = ((r3.xyzx)*(source[5].xyzx)).xyz;
    // 69: dp3 r1.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 70: mad r3.xyz, -r3.xyzx, cb0[5].xyzx, r1.yyyy
    r3.xyz = ((-(r3.xyzx))*(source[5].xyzx)+(r1.yyyy)).xyz;
    // 71: mad r3.xyz, cb0[16].yyyy, r3.xyzx, r6.xyzx
    r3.xyz = ((source[16].yyyy)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 72: dp3 r1.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 73: add r6.xyz, -r3.xyzx, r1.yyyy
    r6.xyz = ((-(r3.xyzx))+(r1.yyyy)).xyz;
    // 74: mad r3.xyz, cb0[16].zzzz, r6.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 75: mad r6.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mad r7.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 78: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 79: mul r1.y, cb0[11].z, l(1.500000)
    r1.y = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 80: add r1.z, -cb0[11].w, l(1.000000)
    r1.z = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 81: mul r1.z, r1.z, cb0[15].z
    r1.z = ((r1.zzzz)*(source[15].zzzz)).z;
    // 82: mul r1.z, r1.z, l(6.283185)
    r1.z = ((r1.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 83: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 84: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 85: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 86: mad r1.y, r1.y, l(0.500000), cb0[11].z
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).y;
    // 87: add r1.z, -r1.w, cb0[11].x
    r1.z = ((-(r1.wwww))+(source[11].xxxx)).z;
    // 88: mul r6.z, r1.z, l(0.125000)
    r6.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 89: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 90: mul r6.y, cb0[11].y, cb0[12].y
    r6.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 91: frc r1.z, v4.x
    r1.z = (frac(v4.xxxx)).z;
    // 92: mul r7.x, r1.z, l(0.125000)
    r7.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 93: mov r7.y, v4.y
    r7.y = (v4.yyyy).y;
    // 94: add r6.xy, r6.xyxx, r7.xyxx
    r6.xy = ((r6.xyxx)+(r7.xyxx)).xy;
    // 95: add r6.xy, r6.xyxx, r6.zwzz
    r6.xy = ((r6.xyxx)+(r6.zwzz)).xy;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 97: mul r6.xyz, r1.yyyy, r6.xyzx
    r6.xyz = ((r1.yyyy)*(r6.xyzx)).xyz;
    // 98: mul r1.y, r1.w, r6.w
    r1.y = ((r1.wwww)*(r6.wwww)).y;
    // 99: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 100: mad r1.yzw, r1.yyyy, r6.xxyz, r3.xxyz
    r1.yzw = ((r1.yyyy)*(r6.xxyz)+(r3.xxyz)).yzw;
    // 101: mul r3.xyz, r5.xyzx, r1.yzwy
    r3.xyz = ((r5.xyzx)*(r1.yzwy)).xyz;
    // 102: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 103: mad r5.xyz, r1.xxxx, cb0[2].xyzx, -cb0[2].xyzx
    r5.xyz = ((r1.xxxx)*(source[2].xyzx)+(-(source[2].xyzx))).xyz;
    // 104: mad r5.xyz, cb0[2].wwww, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((source[2].wwww)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 105: mad r6.xyz, r1.xxxx, cb0[3].xyzx, -cb0[3].xyzx
    r6.xyz = ((r1.xxxx)*(source[3].xyzx)+(-(source[3].xyzx))).xyz;
    // 106: mad r6.xyz, cb0[3].wwww, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((source[3].wwww)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 107: add r1.yzw, r1.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r1.yzw = ((r1.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 108: dp3 r3.w, r1.yzwy, r1.yzwy
    r3.w = (dot((r1.yzwy).xyz,(r1.yzwy).xyz).xxxx).w;
    // 109: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 110: div r1.yzw, r1.yyzw, r3.wwww
    r1.yzw = ((r1.yyzw)/(r3.wwww)).yzw;
    // 111: dp3 r3.w, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 112: add r7.xyz, -r1.yzwy, r3.wwww
    r7.xyz = ((-(r1.yzwy))+(r3.wwww)).xyz;
    // 113: add r1.yzw, r1.yyzw, -r7.xxyz
    r1.yzw = ((r1.yyzw)+(-(r7.xxyz))).yzw;
    // 114: mul r1.x, r1.x, cb0[17].z
    r1.x = ((r1.xxxx)*(source[17].zzzz)).x;
    // 115: mad r1.xyz, r1.xxxx, r1.yzwy, r6.xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(r6.xyzx)).xyz;
    // 116: add r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)+(r5.xyzx)).xyz;
    // 117: mul r5.xyz, r0.xxxx, cb0[13].xyzx
    r5.xyz = ((r0.xxxx)*(source[13].xyzx)).xyz;
    // 118: movc r0.x, r0.y, l(0), r0.z
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 119: mul r0.xyz, r5.xyzx, r0.xxxx
    r0.xyz = ((r5.xyzx)*(r0.xxxx)).xyz;
    // 120: mad r0.xyz, r0.xyzx, l(1.200000, 1.200000, 1.200000, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(1.200000,1.200000,1.200000,0.000000))+(r1.xyzx)).xyz;
    // 121: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 122: dp3 r1.x, v7.xyzx, v7.xyzx
    r1.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 123: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 124: mul r1.xyz, r1.xxxx, v7.xyzx
    r1.xyz = ((r1.xxxx)*(v7.xyzx)).xyz;
    // 125: dp3 r1.x, r1.xyzx, r2.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 126: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 127: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 128: mul r1.yzw, r1.yyyy, cb0[21].xxyz
    r1.yzw = ((r1.yyyy)*(source[21].xxyz)).yzw;
    // 129: mad r1.xyz, r1.xxxx, cb0[20].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[20].xyzx)+(r1.yzwy)).xyz;
    // 130: mul r1.xyz, r1.xyzx, cb0[22].wwww
    r1.xyz = ((r1.xyzx)*(source[22].wwww)).xyz;
    // 131: mul r5.xyz, r3.xyzx, r1.xyzx
    r5.xyz = ((r3.xyzx)*(r1.xyzx)).xyz;
    // 132: mad r0.xyz, r1.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 133: mad r0.xyz, r3.xyzx, cb0[22].xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(source[22].xyzx)+(r0.xyzx)).xyz;
    // 134: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 135: mad r1.x, r1.x, l(-0.250000), l(0.400000)
    r1.x = ((r1.xxxx)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).x;
    // 136: eq r1.y, cb0[23].x, l(0.000000)
    r1.y = (asfloat((uint4)((source[23].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 137: not r1.z, r1.y
    r1.z = (asfloat(~asuint(r1.yyyy))).z;
    // 138: lt r1.w, r2.w, r1.x
    r1.w = (asfloat((uint4)((r2.wwww)<(r1.xxxx)) * 0xffffffffu)).w;
    // 139: and r1.z, r1.w, r1.z
    r1.z = (asfloat(asuint(r1.wwww) & asuint(r1.zzzz))).z;
    // 140: discard_nz r1.z
    if ((asuint(r1.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 141: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 142: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 143: mul r6.xyz, r1.zzzz, v1.xyzx
    r6.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 144: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 145: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 146: mul r7.xyz, r1.zzzz, v0.xyzx
    r7.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 147: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 148: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 149: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 150: mul r9.xyz, r4.yzwy, cb0[14].xyzx
    r9.xyz = ((r4.yzwy)*(source[14].xyzx)).xyz;
    // 151: dp3 r1.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 152: mad r4.xyz, -r4.yzwy, cb0[14].xyzx, r1.zzzz
    r4.xyz = ((-(r4.yzwy))*(source[14].xyzx)+(r1.zzzz)).xyz;
    // 153: mad r4.xyz, cb0[16].yyyy, r4.xyzx, r9.xyzx
    r4.xyz = ((source[16].yyyy)*(r4.xyzx)+(r9.xyzx)).xyz;
    // 154: dp3 r1.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 155: add r9.xyz, -r4.xyzx, r1.zzzz
    r9.xyz = ((-(r4.xyzx))+(r1.zzzz)).xyz;
    // 156: mad r4.xyz, cb0[16].zzzz, r9.xyzx, r4.xyzx
    r4.xyz = ((source[16].zzzz)*(r9.xyzx)+(r4.xyzx)).xyz;
    // 157: mad r4.xyz, r4.xyzx, cb2[4].wwww, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 158: ge r1.x, r2.w, r1.x
    r1.x = (asfloat((uint4)((r2.wwww)>=(r1.xxxx)) * 0xffffffffu)).x;
    // 159: mad r0.w, r0.w, cb0[0].x, l(-0.900000)
    r0.w = ((r0.wwww)*(source[0].xxxx)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 160: mul_sat r0.w, r0.w, l(9.999998)
    r0.w = (saturate((r0.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 161: mad r1.z, r0.w, l(-2.000000), l(3.000000)
    r1.z = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 162: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 163: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 164: mul r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)*(r0.wwww)).w;
    // 165: movc r0.w, r1.x, r0.w, r2.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (r0.wwww) : (r2.wwww)).w;
    // 166: movc o0.w, r1.y, r0.w, r2.w
    output.targets[0].w = ((asuint(r1.yyyy) != 0u) ? (r0.wwww) : (r2.wwww)).w;
    // 167: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 168: dp3 r0.x, r7.xyzx, r2.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 169: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 170: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 171: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 172: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 173: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 174: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 175: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 176: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 177: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 178: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 179: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 180: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 181: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 182: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 183: ftou r0.x, cb0[19].z
    r0.x = (asfloat((uint4)(source[19].zzzz))).x;
    // 184: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 185: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 186: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 187: mul_sat r0.xyz, r4.xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((r4.xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 188: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 189: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 190: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 191: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 192: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 193: ret
    return output;
}

// source.vehicle.terpeion-body.v1 / source program 2b41020e9d64484f9b0c844745af2143
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase85(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[21]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[26].z=(g_SourceCharacterTime.xxxx).x;
    source[27].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[28].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[28].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[28].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[28].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
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
    // 7: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 8: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 9: add r1.x, -r0.x, l(1000.000000)
    r1.x = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 10: mad r0.x, cb0[26].w, r1.x, r0.x
    r0.x = ((source[26].wwww)*(r1.xxxx)+(r0.xxxx)).x;
    // 11: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 12: mad r0.x, cb0[26].y, cb0[26].z, r0.x
    r0.x = ((source[26].yyyy)*(source[26].zzzz)+(r0.xxxx)).x;
    // 13: mul r1.x, r0.x, l(3.524534)
    r1.x = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 14: sincos null, r1.x, r1.x
    r1.x = (cos(r1.xxxx)).x;
    // 15: add r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 16: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 17: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 18: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 19: mad r0.x, r0.x, l(0.500000), cb0[26].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[26].xxxx)).x;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 21: mul r2.xyz, cb0[13].xyzx, cb0[25].wwww
    r2.xyz = ((source[13].xyzx)*(source[25].wwww)).xyz;
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
    // 30: mul r2.xy, r2.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
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
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: log r3.x, |r2.w|
    r3.x = (log2(abs(r2.wwww))).x;
    // 42: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 43: mul r3.x, r3.x, cb0[23].x
    r3.x = ((r3.xxxx)*(source[23].xxxx)).x;
    // 44: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 45: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: movc r2.w, r2.w, l(0), r3.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 47: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 48: mul r3.x, r3.x, cb0[23].y
    r3.x = ((r3.xxxx)*(source[23].yyyy)).x;
    // 49: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: add r0.x, |r0.x|, r1.w
    r0.x = ((abs(r0.xxxx))+(r1.wwww)).x;
    // 52: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 54: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 55: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 56: mul r3.xy, r3.xyxx, cb0[22].xxxx
    r3.xy = ((r3.xyxx)*(source[22].xxxx)).xy;
    // 57: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 59: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 60: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 61: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r4.xyz, cb0[22].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[22].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
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
    // 81: mad r9.xyz, v5.xyzx, r1.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r1.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
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
    // 87: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 88: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 89: dp2 r1.w, r6.ywyy, r6.ywyy
    r1.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 90: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 91: div r6.xy, r6.ywyy, r1.wwww
    r6.xy = ((r6.ywyy)/(r1.wwww)).xy;
    // 92: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 93: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 95: mad r6.xy, r1.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, r0.x
    r6.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 97: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 98: rcp r0.x, cb0[23].z
    r0.x = (1.0/(source[23].zzzz)).x;
    // 99: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 100: mul r10.xyz, r10.xyzx, cb0[23].zzzz
    r10.xyz = ((r10.xyzx)*(source[23].zzzz)).xyz;
    // 101: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 102: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 103: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 104: mad r10.xyz, r10.xyzx, cb0[23].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[23].zzzz)+(r11.xyzx)).xyz;
    // 105: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 106: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 107: add r0.x, cb0[23].z, l(1.000000)
    r0.x = ((source[23].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 109: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r6.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r6.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 111: mad r6.xyz, r3.wwww, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[11].xyzx)).xyz;
    // 112: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 113: mul r6.xyz, r6.xyzx, cb0[23].wwww
    r6.xyz = ((r6.xyzx)*(source[23].wwww)).xyz;
    // 114: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 115: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 116: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 117: dp3 r0.x, r3.xyzx, r4.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 118: mul_sat r1.w, r0.x, cb0[24].y
    r1.w = (saturate((r0.xxxx)*(source[24].yyyy))).w;
    // 119: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 120: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul_sat r3.w, r4.z, cb0[24].y
    r3.w = (saturate((r4.zzzz)*(source[24].yyyy))).w;
    // 122: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add_sat r3.w, r3.w, -cb0[24].z
    r3.w = (saturate((r3.wwww)+(-(source[24].zzzz)))).w;
    // 124: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 125: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 126: mul r4.w, r4.w, cb0[24].w
    r4.w = ((r4.wwww)*(source[24].wwww)).w;
    // 127: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 128: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 129: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 130: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 132: mad r2.xyz, cb0[22].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 133: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 134: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 135: mad r2.xyz, cb0[22].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 136: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: add r10.xyz, -r2.xyzx, r3.wwww
    r10.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 138: mul r10.xyz, r10.xyzx, cb0[24].xxxx
    r10.xyz = ((r10.xyzx)*(source[24].xxxx)).xyz;
    // 139: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 140: add r3.w, r11.y, r11.x
    r3.w = ((r11.yyyy)+(r11.xxxx)).w;
    // 141: add r3.w, r11.z, r3.w
    r3.w = ((r11.zzzz)+(r3.wwww)).w;
    // 142: add_sat r3.w, r11.w, r3.w
    r3.w = (saturate((r11.wwww)+(r3.wwww))).w;
    // 143: mad r2.xyz, r3.wwww, r10.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 144: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 145: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 146: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 147: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 148: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 150: mul r3.w, r3.w, cb0[25].x
    r3.w = ((r3.wwww)*(source[25].xxxx)).w;
    // 151: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 152: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 155: div r4.w, cb0[25].y, r4.w
    r4.w = ((source[25].yyyy)/(r4.wwww)).w;
    // 156: mul r4.w, r1.w, r4.w
    r4.w = ((r1.wwww)*(r4.wwww)).w;
    // 157: mul r10.xyz, r6.xyzx, r4.wwww
    r10.xyz = ((r6.xyzx)*(r4.wwww)).xyz;
    // 158: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: add r12.xyz, -r0.yzwy, r4.wwww
    r12.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 160: mad r0.yzw, cb0[22].yyyy, r12.xxyz, r0.yyzw
    r0.yzw = ((source[22].yyyy)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 161: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r12.xyz, -r0.yzwy, r4.wwww
    r12.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 163: mad r0.yzw, cb0[22].zzzz, r12.xxyz, r0.yyzw
    r0.yzw = ((source[22].zzzz)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 164: mul r12.xyz, cb0[5].xyzx, cb0[5].wwww
    r12.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 165: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, -r12.xyzx
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r12.xyzx))).xyz;
    // 166: mad r12.xyz, r11.xxxx, r13.xyzx, r12.xyzx
    r12.xyz = ((r11.xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 167: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, -r12.xyzx
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r12.xyzx))).xyz;
    // 168: mad r11.xyw, r11.yyyy, r13.xyxz, r12.xyxz
    r11.xyw = ((r11.yyyy)*(r13.xyxz)+(r12.xyxz)).xyw;
    // 169: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, -r11.xywx
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r11.xywx))).xyz;
    // 170: mad r11.xyz, r11.zzzz, r12.xyzx, r11.xywx
    r11.xyz = ((r11.zzzz)*(r12.xyzx)+(r11.xywx)).xyz;
    // 171: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r12.xyz, -r11.xyzx, r4.wwww
    r12.xyz = ((-(r11.xyzx))+(r4.wwww)).xyz;
    // 173: mad r11.xyz, cb0[22].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 174: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: add r12.xyz, -r11.xyzx, r4.wwww
    r12.xyz = ((-(r11.xyzx))+(r4.wwww)).xyz;
    // 176: mad r11.xyz, cb0[22].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 177: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mad r13.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 180: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 181: mul r13.xyz, r0.yzwy, r11.xyzx
    r13.xyz = ((r0.yzwy)*(r11.xyzx)).xyz;
    // 182: mad r0.yzw, r11.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r11.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 183: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 184: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 185: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: mul r4.w, r4.w, cb0[25].z
    r4.w = ((r4.wwww)*(source[25].zzzz)).w;
    // 187: mad r2.xyz, r4.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r4.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 188: frc r4.w, cb0[4].x
    r4.w = (frac(source[4].xxxx)).w;
    // 189: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: mad r1.xyz, r5.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 191: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 193: mad r1.xyz, cb0[22].yyyy, r6.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 194: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 195: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 196: mad r1.xyz, cb0[22].zzzz, r6.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 197: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 198: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 199: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 200: dp3 r5.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 201: add r6.xyz, -r0.yzwy, r5.wwww
    r6.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 202: add r0.yzw, r0.yyzw, -r6.xxyz
    r0.yzw = ((r0.yyzw)+(-(r6.xxyz))).yzw;
    // 203: mul r6.xyz, cb0[16].xyzx, cb0[27].yyyy
    r6.xyz = ((source[16].xyzx)*(source[27].yyyy)).xyz;
    // 204: mul r6.xyz, r6.xyzx, cb0[28].wwww
    r6.xyz = ((r6.xyzx)*(source[28].wwww)).xyz;
    // 205: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 206: mad r10.xyz, r1.wwww, cb0[15].xyzx, -cb0[15].xyzx
    r10.xyz = ((r1.wwww)*(source[15].xyzx)+(-(source[15].xyzx))).xyz;
    // 207: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 208: mad r1.w, cb0[14].w, r1.w, l(1.000000)
    r1.w = ((source[14].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: mad r10.xyz, cb0[15].wwww, r10.xyzx, cb0[15].xyzx
    r10.xyz = ((source[15].wwww)*(r10.xyzx)+(source[15].xyzx)).xyz;
    // 210: mad r0.yzw, r0.yyzw, r6.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r6.xxyz)+(r10.xxyz)).yzw;
    // 211: mad r0.yzw, r1.wwww, cb0[14].xxyz, r0.yyzw
    r0.yzw = ((r1.wwww)*(source[14].xxyz)+(r0.yyzw)).yzw;
    // 212: mad r0.yzw, r1.xxyz, r12.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r12.xxyz)+(r0.yyzw)).yzw;
    // 213: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 214: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 215: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 216: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 217: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 218: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 219: mul r1.xyz, r1.xxxx, cb0[17].xyzx
    r1.xyz = ((r1.xxxx)*(source[17].xyzx)).xyz;
    // 220: movc r1.xyz, r0.xxxx, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 221: add r0.xyz, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 222: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 223: dp3 r0.w, r9.xyzx, r9.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 224: sqrt r1.x, r0.w
    r1.x = (sqrt(r0.wwww)).x;
    // 225: div r1.xyz, r9.xyzx, r1.xxxx
    r1.xyz = ((r9.xyzx)/(r1.xxxx)).xyz;
    // 226: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 227: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 228: mul r1.y, |r1.x|, |r1.x|
    r1.y = ((abs(r1.xxxx))*(abs(r1.xxxx))).y;
    // 229: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 230: mul r1.y, r1.y, |r1.x|
    r1.y = ((r1.yyyy)*(abs(r1.xxxx))).y;
    // 231: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 232: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 233: add r1.y, r1.x, l(-0.027778)
    r1.y = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 234: mad r1.x, r1.x, r1.y, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 235: div_sat r0.w, r1.x, r0.w
    r0.w = (saturate((r1.xxxx)/(r0.wwww))).w;
    // 236: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 238: mad r1.xyz, r0.wwww, r2.xyzx, -r13.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 239: mad r1.xyz, r3.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 240: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 241: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 242: mad r1.xyz, cb0[22].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 243: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 244: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 245: mad r1.xyz, cb0[22].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 246: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 247: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 248: mul r0.w, r0.w, cb0[26].z
    r0.w = ((r0.wwww)*(source[26].zzzz)).w;
    // 249: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 250: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 251: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 252: mul r1.w, cb0[4].z, l(1.500000)
    r1.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 253: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 254: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 255: add r1.w, -r4.w, cb0[4].x
    r1.w = ((-(r4.wwww))+(source[4].xxxx)).w;
    // 256: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 257: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 258: mul r4.x, r1.w, l(0.125000)
    r4.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 259: mul r2.y, cb0[4].y, cb0[18].y
    r2.y = ((source[4].yyyy)*(source[18].yyyy)).y;
    // 260: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 261: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 262: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 263: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 264: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 265: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 266: mul r0.w, r4.w, r2.w
    r0.w = ((r4.wwww)*(r2.wwww)).w;
    // 267: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 268: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 269: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 270: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 271: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 272: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 273: mad r2.xy, cb0[19].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[19].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 274: mul r0.w, cb0[19].y, cb0[26].z
    r0.w = ((source[19].yyyy)*(source[26].zzzz)).w;
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
    // 280: mul r1.w, cb0[19].x, l(0.001000)
    r1.w = ((source[19].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 281: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 282: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 283: dp2 r1.w, cb0[20].xyxx, r2.xyxx
    r1.w = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 284: dp2 r2.y, cb0[21].xyxx, r2.xyxx
    r2.y = (dot((source[21].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 285: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 286: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 289: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 290: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 291: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 292: mad r4.xyz, cb0[19].zzzz, r2.xyzx, -r1.xyzx
    r4.xyz = ((source[19].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 293: mul r2.xyz, r2.xyzx, cb0[19].zzzz
    r2.xyz = ((r2.xyzx)*(source[19].zzzz)).xyz;
    // 294: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 295: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 296: mad r1.xyz, r0.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 297: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 298: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 299: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 300: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 301: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 302: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 303: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 304: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 305: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 306: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 307: mul r3.yzw, r3.yyyy, cb0[30].xxyz
    r3.yzw = ((r3.yyyy)*(source[30].xxyz)).yzw;
    // 308: mad r3.xyz, r3.xxxx, cb0[29].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[29].xyzx)+(r3.yzwy)).xyz;
    // 309: mul r3.xyz, r3.xyzx, cb0[31].wwww
    r3.xyz = ((r3.xyzx)*(source[31].wwww)).xyz;
    // 310: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 311: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 312: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 313: mad o0.xyz, r1.xyzx, cb0[31].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[31].xyzx)+(r0.xyzx)).xyz;
    // 314: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 315: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 316: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 317: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 318: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 319: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 320: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 321: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 322: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 323: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 324: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 325: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 326: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 327: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 328: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 329: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 330: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 331: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 332: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 333: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 334: ret
    return output;
}

// source.vehicle.starlight-body.v1 / source program 4c053d1ae73e1446a61cb74ebcd27874
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase86(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].z=(g_SourceCharacterTime.xxxx).x;
    source[23].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0;
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
    // 9: mul r2.y, cb0[8].y, cb0[17].y
    r2.y = ((source[8].yyyy)*(source[17].yyyy)).y;
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
    // 20: mul r1.w, r1.w, cb0[20].z
    r1.w = ((r1.wwww)*(source[20].zzzz)).w;
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
    // 30: mad r2.xyz, cb0[21].xxxx, r2.xyzx, r0.yzwy
    r2.xyz = ((source[21].xxxx)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 33: mad r2.xyz, cb0[21].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 43: mul r1.w, r6.y, cb0[19].y
    r1.w = ((r6.yyyy)*(source[19].yyyy)).w;
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
    // 77: mul_sat r8.w, r1.w, cb2[3].w
    r8.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 78: add r10.xyz, -r4.xyzx, r7.xyzx
    r10.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 79: mad r4.xyz, r9.zzzz, r10.xyzx, r4.xyzx
    r4.xyz = ((r9.zzzz)*(r10.xyzx)+(r4.xyzx)).xyz;
    // 80: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 81: mad r3.xyz, r9.zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((r9.zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 82: add r3.xyz, r3.xyzx, -cb0[9].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[9].xyzx))).xyz;
    // 83: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 84: add r7.xyz, -r4.xyzx, r1.wwww
    r7.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 85: mad r7.xyz, cb0[21].xxxx, r7.xyzx, r4.xyzx
    r7.xyz = ((source[21].xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 86: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 87: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 88: add r4.xyz, -r7.xyzx, r1.wwww
    r4.xyz = ((-(r7.xyzx))+(r1.wwww)).xyz;
    // 89: mad r4.xyz, cb0[21].yyyy, r4.xyzx, r7.xyzx
    r4.xyz = ((source[21].yyyy)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 90: mad r7.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 91: mad r10.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 93: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 94: mul r10.xyz, r2.xyzx, r4.xyzx
    r10.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 95: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: mad r11.xyz, -r4.xyzx, r2.xyzx, r1.wwww
    r11.xyz = ((-(r4.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 97: mad r2.xyz, r4.xyzx, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = ((r4.xyzx)*(r2.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 98: mad r4.xyz, cb0[21].xxxx, r11.xyzx, r10.xyzx
    r4.xyz = ((source[21].xxxx)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 99: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: add r10.xyz, -r4.xyzx, r1.wwww
    r10.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 101: mad r4.xyz, cb0[21].yyyy, r10.xyzx, r4.xyzx
    r4.xyz = ((source[21].yyyy)*(r10.xyzx)+(r4.xyzx)).xyz;
    // 102: mul r4.xyz, r7.xyzx, r4.xyzx
    r4.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 103: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 104: mad r1.xyz, r0.xxxx, r1.xyzx, r4.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 105: mul r0.x, r6.x, cb0[23].y
    r0.x = ((r6.xxxx)*(source[23].yyyy)).x;
    // 106: mul r1.w, r6.z, cb0[24].z
    r1.w = ((r6.zzzz)*(source[24].zzzz)).w;
    // 107: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 108: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: movc r1.w, r5.z, l(0), r1.w
    r1.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 110: max r1.w, r1.w, cb0[1].x
    r1.w = (max(r1.wwww,source[1].xxxx)).w;
    // 111: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 112: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 113: movc r0.x, r5.x, l(0), r0.x
    r0.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 114: add_sat r0.x, r0.x, cb0[23].z
    r0.x = (saturate((r0.xxxx)+(source[23].zzzz))).x;
    // 115: add r1.w, -r0.x, l(1.000000)
    r1.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: mul r4.xyz, r1.wwww, cb0[16].xyzx
    r4.xyz = ((r1.wwww)*(source[16].xyzx)).xyz;
    // 117: mul r5.xyz, r1.xyzx, r4.xyzx
    r5.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 118: mad r1.xyz, -r4.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r4.xyzx))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 119: mad r1.xyz, r0.xxxx, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 120: add r4.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 121: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 122: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 123: mad r4.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 124: mad r5.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 125: mad r4.xyz, r0.xxxx, r4.xyzx, r5.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 126: mad r5.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 127: mad r4.xyz, r4.xyzx, r0.xxxx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r0.xxxx)+(r5.xyzx)).xyz;
    // 128: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 129: max r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = (max(r0.xxxx,r4.xyzx)).xyz;
    // 130: mov_sat r1.w, cb0[24].y
    r1.w = (saturate(source[24].yyyy)).w;
    // 131: mad r5.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r5.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 132: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 133: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 134: mad r5.xyz, r8.wwww, r5.xyzx, r2.wwww
    r5.xyz = ((r8.wwww)*(r5.xyzx)+(r2.wwww)).xyz;
    // 135: mul_sat r1.w, r5.y, l(50.000000)
    r1.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 136: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 137: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 138: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 139: mul r6.xy, r6.xyxx, cb0[19].xxxx
    r6.xy = ((r6.xyxx)*(source[19].xxxx)).xy;
    // 140: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 142: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 143: add r6.z, r2.w, l(0.000010)
    r6.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 144: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 145: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 146: div r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)/(r2.wwww)).xyz;
    // 147: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 148: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 149: mul r10.xyz, r2.wwww, r6.xyzx
    r10.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 150: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 151: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 152: mul r11.xyz, r2.wwww, v5.xyzx
    r11.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 153: dp3 r2.w, r10.xyzx, r11.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 154: deriv_rtx_coarse r8.x, r2.w
    r8.x = (ddx_coarse(r2.wwww)).x;
    // 155: deriv_rty_coarse r8.y, r2.w
    r8.y = (ddy_coarse(r2.wwww)).y;
    // 156: dp2 r3.w, r8.xyxx, r8.xyxx
    r3.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 157: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 158: mad r3.w, r3.w, l(0.300000), r8.z
    r3.w = ((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz)).w;
    // 159: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 160: min r8.y, r3.w, l(1.000000)
    r8.y = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 161: add r3.w, -r8.y, l(1.000000)
    r3.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: max r12.xyz, r5.xyzx, r3.wwww
    r12.xyz = (max(r5.xyzx,r3.wwww)).xyz;
    // 163: add r12.xyz, -r5.xyzx, r12.xyzx
    r12.xyz = ((-(r5.xyzx))+(r12.xyzx)).xyz;
    // 164: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 165: mul r13.xyz, r2.wwww, r10.xyzx
    r13.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 166: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 167: add r1.w, r13.z, l(1.000000)
    r1.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: add r3.w, r2.w, l(1.000000)
    r3.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: mov_sat r2.w, r2.w
    r2.w = (saturate(r2.wwww)).w;
    // 171: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 172: mul r2.w, r2.w, cb0[2].y
    r2.w = ((r2.wwww)*(source[2].yyyy)).w;
    // 173: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 174: mad_sat r2.w, r2.w, cb0[2].w, cb0[2].z
    r2.w = (saturate((r2.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 175: mul r2.w, r2.w, cb0[24].w
    r2.w = ((r2.wwww)*(source[24].wwww)).w;
    // 176: add_sat r8.x, -r1.w, r3.w
    r8.x = (saturate((-(r1.wwww))+(r3.wwww))).x;
    // 177: sample_indexable(texture2d)(float,float,float,float) r14.xy, r8.xyxx, t6.xyzw, s7
    r14.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 178: add r1.w, r0.x, r8.x
    r1.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 179: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 180: mul r15.xyz, r5.xyzx, r14.yyyy
    r15.xyz = ((r5.xyzx)*(r14.yyyy)).xyz;
    // 181: mad r12.xyz, r12.xyzx, r14.xxxx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r14.xxxx)+(r15.xyzx)).xyz;
    // 182: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r14.y
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r14.yyyy)).w;
    // 183: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 184: mad r14.xyz, r5.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((r5.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: mad r5.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 187: mad r15.xyz, -r12.xyzx, r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r12.xyzx))*(r14.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)*(r14.xyzx)).xyz;
    // 189: mul r14.xyz, r1.xyzx, r15.xyzx
    r14.xyz = ((r1.xyzx)*(r15.xyzx)).xyz;
    // 190: add r3.w, -r8.w, l(1.000000)
    r3.w = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mul r14.xyz, r3.wwww, r14.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)).xyz;
    // 192: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 193: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 194: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 195: mul r16.xyz, r5.wwww, v1.xyzx
    r16.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 196: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 197: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 198: mul r17.xyz, r5.wwww, v0.xyzx
    r17.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 199: mul r18.xyz, r16.zxyz, r17.yzxy
    r18.xyz = ((r16.zxyz)*(r17.yzxy)).xyz;
    // 200: mad r18.xyz, r16.yzxy, r17.zxyz, -r18.xyzx
    r18.xyz = ((r16.yzxy)*(r17.zxyz)+(-(r18.xyzx))).xyz;
    // 201: mul r18.xyz, r18.xyzx, v1.wwww
    r18.xyz = ((r18.xyzx)*(v1.wwww)).xyz;
    // 202: dp3 r19.y, r18.xyzx, r10.xyzx
    r19.y = (dot((r18.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 203: dp3 r18.y, r18.xyzx, r13.xyzx
    r18.y = (dot((r18.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 204: dp3 r19.x, r17.xyzx, r10.xyzx
    r19.x = (dot((r17.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 205: dp3 r18.x, r17.xyzx, r13.xyzx
    r18.x = (dot((r17.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 206: dp2 r17.z, r19.xyxx, cb0[26].xyxx
    r17.z = (dot((r19.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 207: mul r8.xz, cb0[26].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[26].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 208: dp2 r17.x, r19.xyxx, r8.xzxx
    r17.x = (dot((r19.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 209: dp2 r20.x, r18.xyxx, r8.xzxx
    r20.x = (dot((r18.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 210: dp2 r20.z, r18.xyxx, cb0[26].xyxx
    r20.z = (dot((r18.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 211: dp3 r17.y, r16.xyzx, r10.xyzx
    r17.y = (dot((r16.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 212: dp3 r20.y, r16.xyzx, r13.xyzx
    r20.y = (dot((r16.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 213: mov r17.w, l(1.000000)
    r17.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 214: dp4 r16.x, cb0[27].xyzw, r17.xyzw
    r16.x = (dot((source[27].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).x;
    // 215: dp4 r16.y, cb0[28].xyzw, r17.xyzw
    r16.y = (dot((source[28].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).y;
    // 216: dp4 r16.z, cb0[29].xyzw, r17.xyzw
    r16.z = (dot((source[29].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).z;
    // 217: mul r18.xyzw, r17.yzzx, r17.xyzz
    r18.xyzw = ((r17.yzzx)*(r17.xyzz)).xyzw;
    // 218: dp4 r21.x, cb0[30].xyzw, r18.xyzw
    r21.x = (dot((source[30].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).x;
    // 219: dp4 r21.y, cb0[31].xyzw, r18.xyzw
    r21.y = (dot((source[31].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).y;
    // 220: dp4 r21.z, cb0[32].xyzw, r18.xyzw
    r21.z = (dot((source[32].xyzw).xyzw,(r18.xyzw).xyzw).xxxx).z;
    // 221: add r16.xyz, r16.xyzx, r21.xyzx
    r16.xyz = ((r16.xyzx)+(r21.xyzx)).xyz;
    // 222: mul r5.w, r17.y, r17.y
    r5.w = ((r17.yyyy)*(r17.yyyy)).w;
    // 223: mov r19.z, r17.y
    r19.z = (r17.yyyy).z;
    // 224: mad r5.w, r17.x, r17.x, -r5.w
    r5.w = ((r17.xxxx)*(r17.xxxx)+(-(r5.wwww))).w;
    // 225: mad r16.xyz, cb0[33].xyzx, r5.wwww, r16.xyzx
    r16.xyz = ((source[33].xyzx)*(r5.wwww)+(r16.xyzx)).xyz;
    // 226: max r16.xyz, r16.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r16.xyz = (max(r16.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 227: mul r16.xyz, r16.xyzx, cb0[25].xyzx
    r16.xyz = ((r16.xyzx)*(source[25].xyzx)).xyz;
    // 228: mul r16.xyz, r16.xyzx, cb0[26].zzzz
    r16.xyz = ((r16.xyzx)*(source[26].zzzz)).xyz;
    // 229: mad r16.xyz, r16.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[25].wwww
    r16.xyz = ((r16.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[25].wwww)).xyz;
    // 230: dp3 r5.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 231: add r16.xyz, -r5.wwww, r16.xyzx
    r16.xyz = ((-(r5.wwww))+(r16.xyzx)).xyz;
    // 232: mad r16.xyz, r16.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r16.xyz = ((r16.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 233: dp3 r5.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 234: mad r6.w, r8.y, l(2.000000), l(2.000000)
    r6.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 235: div r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)/(r6.wwww)).w;
    // 236: mad r5.w, r4.w, l(5.000000), r5.w
    r5.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 237: add_sat r5.w, r8.w, r5.w
    r5.w = (saturate((r8.wwww)+(r5.wwww))).w;
    // 238: mad r7.w, r5.w, l(-2.000000), l(3.000000)
    r7.w = ((r5.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 239: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 240: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 241: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 242: mul r5.w, r5.w, l(1.500000)
    r5.w = ((r5.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 243: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 244: mul r16.xyz, r5.wwww, r16.xyzx
    r16.xyz = ((r5.wwww)*(r16.xyzx)).xyz;
    // 245: mul r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r16.xyzx)).xyz;
    // 246: mul r15.xyz, r15.xyzx, r16.xyzx
    r15.xyz = ((r15.xyzx)*(r16.xyzx)).xyz;
    // 247: mul r14.xyz, r4.xyzx, r14.xyzx
    r14.xyz = ((r4.xyzx)*(r14.xyzx)).xyz;
    // 248: mul r5.w, r8.y, l(5.000000)
    r5.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 249: mul r7.w, r8.y, r8.y
    r7.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 250: mul r1.w, r1.w, r7.w
    r1.w = ((r1.wwww)*(r7.wwww)).w;
    // 251: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 252: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 253: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 254: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 255: sample_l_indexable(texturecube)(float,float,float,float) r16.xyzw, r20.xyzx, t7.xyzw, s6, r5.w
    r16.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r20.xyzx).xyz, (r5.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 256: mul r8.xyz, r16.xyzx, r16.wwww
    r8.xyz = ((r16.xyzx)*(r16.wwww)).xyz;
    // 257: mul r8.xyz, r8.xyzx, cb0[25].xyzx
    r8.xyz = ((r8.xyzx)*(source[25].xyzx)).xyz;
    // 258: mul r8.xyz, r8.xyzx, cb0[26].zzzz
    r8.xyz = ((r8.xyzx)*(source[26].zzzz)).xyz;
    // 259: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[25].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[25].wwww)).xyz;
    // 260: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 261: add r8.xyz, -r1.wwww, r8.xyzx
    r8.xyz = ((-(r1.wwww))+(r8.xyzx)).xyz;
    // 262: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 263: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 264: div r1.w, r1.w, r6.w
    r1.w = ((r1.wwww)/(r6.wwww)).w;
    // 265: mad r1.w, r4.w, l(5.000000), r1.w
    r1.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 266: add_sat r1.w, r8.w, r1.w
    r1.w = (saturate((r8.wwww)+(r1.wwww))).w;
    // 267: mad r4.w, r1.w, l(-2.000000), l(3.000000)
    r4.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 268: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 269: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 270: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 271: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 272: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 273: mul r8.xyz, r1.wwww, r8.xyzx
    r8.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 274: mul r16.xyz, r8.xyzx, r12.xyzx
    r16.xyz = ((r8.xyzx)*(r12.xyzx)).xyz;
    // 275: mad r1.w, r0.x, r5.x, r5.y
    r1.w = ((r0.xxxx)*(r5.xxxx)+(r5.yyyy)).w;
    // 276: mad r1.w, r1.w, r0.x, r5.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r5.zzzz)).w;
    // 277: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 278: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 279: mad r5.xyz, r16.xyzx, r0.xxxx, r14.xyzx
    r5.xyz = ((r16.xyzx)*(r0.xxxx)+(r14.xyzx)).xyz;
    // 280: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 281: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 282: mul r14.xyz, r1.wwww, v6.xyzx
    r14.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 283: dp3 r1.w, r14.xyzx, r10.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 284: dp3 r4.w, -r14.xyzx, r10.xyzx
    r4.w = (dot((-(r14.xyzx)).xyz,(r10.xyzx).xyz).xxxx).w;
    // 285: dp3 r5.w, r14.xyzx, r13.xyzx
    r5.w = (dot((r14.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 286: mad r10.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 287: mad r10.zw, r4.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r10.zw = ((r4.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 288: mul r10.xyzw, r10.xyzw, r10.xyzw
    r10.xyzw = ((r10.xyzw)*(r10.xyzw)).xyzw;
    // 289: mad r13.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r13.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 290: mul r13.xy, r13.xyxx, r13.xyxx
    r13.xy = ((r13.xyxx)*(r13.xyxx)).xy;
    // 291: mul r13.yzw, r13.yyyy, cb0[36].xxyz
    r13.yzw = ((r13.yyyy)*(source[36].xxyz)).yzw;
    // 292: mad r13.xyz, r13.xxxx, cb0[35].xyzx, r13.yzwy
    r13.xyz = ((r13.xxxx)*(source[35].xyzx)+(r13.yzwy)).xyz;
    // 293: mul r13.xyz, r13.xyzx, cb0[37].wwww
    r13.xyz = ((r13.xyzx)*(source[37].wwww)).xyz;
    // 294: mul r13.xyz, r1.xyzx, r13.xyzx
    r13.xyz = ((r1.xyzx)*(r13.xyzx)).xyz;
    // 295: mul r4.xyz, r4.xyzx, r13.xyzx
    r4.xyz = ((r4.xyzx)*(r13.xyzx)).xyz;
    // 296: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 297: mul r4.xyz, r15.xyzx, r4.xyzx
    r4.xyz = ((r15.xyzx)*(r4.xyzx)).xyz;
    // 298: mad r4.xyz, -r4.xyzx, r8.wwww, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(r8.wwww)+(r4.xyzx)).xyz;
    // 299: mad r4.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r4.xyzx
    r4.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r4.xyzx)).xyz;
    // 300: mul r5.xyz, r10.yyyy, cb0[36].xyzx
    r5.xyz = ((r10.yyyy)*(source[36].xyzx)).xyz;
    // 301: mad r5.xyz, cb0[35].xyzx, r10.xxxx, r5.xyzx
    r5.xyz = ((source[35].xyzx)*(r10.xxxx)+(r5.xyzx)).xyz;
    // 302: mul r5.xyz, r5.xyzx, cb0[37].wwww
    r5.xyz = ((r5.xyzx)*(source[37].wwww)).xyz;
    // 303: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 304: mul r5.xyz, r8.xyzx, r5.xyzx
    r5.xyz = ((r8.xyzx)*(r5.xyzx)).xyz;
    // 305: mul r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)*(r12.xyzx)).xyz;
    // 306: mad r4.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r4.xyzx
    r4.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r4.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 308: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 309: add r0.x, r9.y, r9.x
    r0.x = ((r9.yyyy)+(r9.xxxx)).x;
    // 310: add_sat r0.x, r9.z, r0.x
    r0.x = (saturate((r9.zzzz)+(r0.xxxx))).x;
    // 311: mad r3.xyz, r0.xxxx, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)+(source[9].xyzx)).xyz;
    // 312: mul r3.xyz, r3.xyzx, cb0[19].wwww
    r3.xyz = ((r3.xyzx)*(source[19].wwww)).xyz;
    // 313: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 314: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 315: add r0.x, cb0[0].y, cb0[0].x
    r0.x = ((source[0].yyyy)+(source[0].xxxx)).x;
    // 316: add r0.x, r0.x, cb0[0].z
    r0.x = ((r0.xxxx)+(source[0].zzzz)).x;
    // 317: add r1.w, -r0.x, l(1000.000000)
    r1.w = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 318: mad r0.x, cb0[20].w, r1.w, r0.x
    r0.x = ((source[20].wwww)*(r1.wwww)+(r0.xxxx)).x;
    // 319: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 320: mad r0.x, cb0[20].y, cb0[20].z, r0.x
    r0.x = ((source[20].yyyy)*(source[20].zzzz)+(r0.xxxx)).x;
    // 321: mul r1.w, r0.x, l(3.524534)
    r1.w = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 322: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 323: add r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)+(r1.wwww)).x;
    // 324: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 325: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 326: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 327: mad r0.x, r0.x, l(0.500000), cb0[20].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[20].xxxx)).x;
    // 328: mul r5.xyz, r3.xyzx, r0.xxxx
    r5.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 329: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 330: mad r3.xyz, -r0.xxxx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r0.xxxx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 331: mad r3.xyz, cb0[21].xxxx, r3.xyzx, r5.xyzx
    r3.xyz = ((source[21].xxxx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 332: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 333: add r5.xyz, -r3.xyzx, r0.xxxx
    r5.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 334: mad r3.xyz, cb0[21].yyyy, r5.xyzx, r3.xyzx
    r3.xyz = ((source[21].yyyy)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 335: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 336: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 337: div r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 338: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 339: add r5.xyz, -r2.xyzx, r0.xxxx
    r5.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 340: add r2.xyz, r2.xyzx, -r5.xyzx
    r2.xyz = ((r2.xyzx)+(-(r5.xyzx))).xyz;
    // 341: dp3 r0.x, r6.xyzx, r11.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 342: mul_sat r1.w, r0.x, cb0[21].z
    r1.w = (saturate((r0.xxxx)*(source[21].zzzz))).w;
    // 343: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 344: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 345: mul_sat r4.w, r11.z, cb0[21].z
    r4.w = (saturate((r11.zzzz)*(source[21].zzzz))).w;
    // 346: add r5.x, -|r11.z|, l(1.000000)
    r5.x = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 347: mul r0.x, r0.x, r5.x
    r0.x = ((r0.xxxx)*(r5.xxxx)).x;
    // 348: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 349: add_sat r4.w, r4.w, -cb0[21].w
    r4.w = (saturate((r4.wwww)+(-(source[21].wwww)))).w;
    // 350: log r5.x, r4.w
    r5.x = (log2(r4.wwww)).x;
    // 351: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 352: mul r5.x, r5.x, cb0[22].x
    r5.x = ((r5.xxxx)*(source[22].xxxx)).x;
    // 353: exp r5.x, r5.x
    r5.x = (exp2(r5.xxxx)).x;
    // 354: mul r1.w, r1.w, r5.x
    r1.w = ((r1.wwww)*(r5.xxxx)).w;
    // 355: movc r1.w, r4.w, l(0), r1.w
    r1.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 356: mul r5.xyz, cb0[14].xyzx, cb0[22].zzzz
    r5.xyz = ((source[14].xyzx)*(source[22].zzzz)).xyz;
    // 357: mul r5.xyz, r5.xyzx, cb0[23].xxxx
    r5.xyz = ((r5.xyzx)*(source[23].xxxx)).xyz;
    // 358: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 359: mad r6.xyz, r1.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r6.xyz = ((r1.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 360: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 361: mad r1.w, cb0[12].w, r1.w, l(1.000000)
    r1.w = ((source[12].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 362: mad r6.xyz, cb0[13].wwww, r6.xyzx, cb0[13].xyzx
    r6.xyz = ((source[13].wwww)*(r6.xyzx)+(source[13].xyzx)).xyz;
    // 363: mad r2.xyz, r2.xyzx, r5.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 364: mad r2.xyz, r1.wwww, cb0[12].xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(source[12].xyzx)+(r2.xyzx)).xyz;
    // 365: mad r2.xyz, r3.xyzx, r7.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 366: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 367: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 368: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 369: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 370: mul r3.xyz, r1.wwww, cb0[15].xyzx
    r3.xyz = ((r1.wwww)*(source[15].xyzx)).xyz;
    // 371: movc r3.xyz, r0.xxxx, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 372: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 373: mad r0.xyz, cb0[19].zzzz, r0.yzwy, r2.xyzx
    r0.xyz = ((source[19].zzzz)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 374: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 375: mul r2.xyz, r10.wwww, cb0[36].xyzx
    r2.xyz = ((r10.wwww)*(source[36].xyzx)).xyz;
    // 376: mad r2.xyz, r10.zzzz, cb0[35].xyzx, r2.xyzx
    r2.xyz = ((r10.zzzz)*(source[35].xyzx)+(r2.xyzx)).xyz;
    // 377: mul r2.xyz, r2.xyzx, cb0[37].wwww
    r2.xyz = ((r2.xyzx)*(source[37].wwww)).xyz;
    // 378: mul_sat r3.xyz, cb0[18].xyzx, cb0[18].wwww
    r3.xyz = (saturate((source[18].xyzx)*(source[18].wwww))).xyz;
    // 379: mul r5.xyz, r2.wwww, r3.xyzx
    r5.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 380: mul r3.xyz, r3.xyzx, cb0[24].wwww
    r3.xyz = ((r3.xyzx)*(source[24].wwww)).xyz;
    // 381: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 382: mul r3.xyz, r3.wwww, r5.xyzx
    r3.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 383: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 384: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 385: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 386: add r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)+(r0.xyzx)).xyz;
    // 387: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 388: mad o0.xyz, r1.xyzx, cb0[37].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[37].xyzx)+(r0.xyzx)).xyz;
    // 389: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 390: dp3 r0.x, r19.xyzx, r19.xyzx
    r0.x = (dot((r19.xyzx).xyz,(r19.xyzx).xyz).xxxx).x;
    // 391: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 392: mul r0.xyz, r0.xxxx, r19.xyzx
    r0.xyz = ((r0.xxxx)*(r19.xyzx)).xyz;
    // 393: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 394: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 395: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 396: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 397: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 398: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 399: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 400: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 401: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 402: ftou r0.x, cb0[34].z
    r0.x = (asfloat((uint4)(source[34].zzzz))).x;
    // 403: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 404: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 405: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 406: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 407: ret
    return output;
}

// source.vehicle.mokoboard-vfx.v1 / source program 1f0ef7bb5a749741af867a3490a293c3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase87(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[27]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[29]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[30]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[31].w=(g_SourceCharacterTime.xxxx).x;
    source[35].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[35].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[35].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 7: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 8: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 9: mad r0.xyz, cb0[31].yyyy, r1.xyzx, r0.yzwy
    r0.xyz = ((source[31].yyyy)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 10: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 11: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 12: mad r0.xyz, cb0[31].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[31].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 13: add r0.w, -v4.x, v4.y
    r0.w = ((-(v4.xxxx))+(v4.yyyy)).w;
    // 14: mad r0.w, cb0[21].z, r0.w, v4.x
    r0.w = ((source[21].zzzz)*(r0.wwww)+(v4.xxxx)).w;
    // 15: mul r0.w, r0.w, cb0[21].x
    r0.w = ((r0.wwww)*(source[21].xxxx)).w;
    // 16: mul r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 17: mad r0.w, cb0[21].y, cb0[31].w, r0.w
    r0.w = ((source[21].yyyy)*(source[31].wwww)+(r0.wwww)).w;
    // 18: add r1.xyz, r0.wwww, l(0.000000, 0.330000, 0.660000, 0.000000)
    r1.xyz = ((r0.wwww)+(float4(0.000000,0.330000,0.660000,0.000000))).xyz;
    // 19: mul r1.xyz, r1.xyzx, l(6.283185, 6.283185, 6.283185, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(6.283185,6.283185,6.283185,0.000000))).xyz;
    // 20: sincos null, r1.xyz, r1.xyzx
    r1.xyz = (cos(r1.xyzx)).xyz;
    // 21: mad r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(0.500000, 0.500000, 0.500000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 22: add r2.xy, v4.xyxx, cb0[17].xyxx
    r2.xy = ((v4.xyxx)+(source[17].xyxx)).xy;
    // 23: mul r3.xyz, v7.yyyy, cb1[1].xywx
    r3.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 24: mad r3.xyz, cb1[0].xywx, v7.xxxx, r3.xyzx
    r3.xyz = ((projection[0].xywx)*(v7.xxxx)+(r3.xyzx)).xyz;
    // 25: mad r3.xyz, cb1[2].xywx, v7.zzzz, r3.xyzx
    r3.xyz = ((projection[2].xywx)*(v7.zzzz)+(r3.xyzx)).xyz;
    // 26: mad r3.xyz, cb1[3].xywx, v7.wwww, r3.xyzx
    r3.xyz = ((projection[3].xywx)*(v7.wwww)+(r3.xyzx)).xyz;
    // 27: div r2.zw, r3.xxxy, r3.zzzz
    r2.zw = ((r3.xxxy)/(r3.zzzz)).zw;
    // 28: mad r2.zw, r2.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r2.zw = ((r2.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 29: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 30: mad r2.xy, cb0[17].zzzz, r2.xyxx, r2.zwzz
    r2.xy = ((source[17].zzzz)*(r2.xyxx)+(r2.zwzz)).xy;
    // 31: mul r2.xy, r2.xyxx, cb0[16].xyxx
    r2.xy = ((r2.xyxx)*(source[16].xyxx)).xy;
    // 32: mad r2.xy, cb0[31].wwww, cb0[16].zwzz, r2.xyxx
    r2.xy = ((source[31].wwww)*(source[16].zwzz)+(r2.xyxx)).xy;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t3.xzwy, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).w;
    // 34: mul r3.xyz, r0.wwww, cb0[18].xyzx
    r3.xyz = ((r0.wwww)*(source[18].xyzx)).xyz;
    // 35: mul r3.xyz, r3.xyzx, cb0[18].wwww
    r3.xyz = ((r3.xyzx)*(source[18].wwww)).xyz;
    // 36: add r2.xy, v4.xyxx, cb0[14].xyxx
    r2.xy = ((v4.xyxx)+(source[14].xyxx)).xy;
    // 37: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 38: mad r2.xy, cb0[14].zzzz, r2.xyxx, r2.zwzz
    r2.xy = ((source[14].zzzz)*(r2.xyxx)+(r2.zwzz)).xy;
    // 39: mul r2.zw, r2.zzzw, l(0.000000, 0.000000, 700.000000, 700.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,700.000000,700.000000))).zw;
    // 40: mul r2.xy, r2.xyxx, cb0[13].xyxx
    r2.xy = ((r2.xyxx)*(source[13].xyxx)).xy;
    // 41: mad r2.xy, cb0[31].wwww, cb0[13].zwzz, r2.xyxx
    r2.xy = ((source[31].wwww)*(source[13].zwzz)+(r2.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t3.yzwx, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 43: mul r4.xyz, r0.wwww, cb0[15].xyzx
    r4.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 44: mad r5.xyz, cb0[15].wwww, r4.xyzx, r3.xyzx
    r5.xyz = ((source[15].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 45: mul r4.xyz, r4.xyzx, cb0[15].wwww
    r4.xyz = ((r4.xyzx)*(source[15].wwww)).xyz;
    // 46: mad r3.xyz, r4.xyzx, r3.xyzx, -r5.xyzx
    r3.xyz = ((r4.xyzx)*(r3.xyzx)+(-(r5.xyzx))).xyz;
    // 47: mad r3.xyz, cb0[32].xxxx, r3.xyzx, r5.xyzx
    r3.xyz = ((source[32].xxxx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 48: add r4.xyz, -r3.xyzx, cb0[19].xyzx
    r4.xyz = ((-(r3.xyzx))+(source[19].xyzx)).xyz;
    // 49: mad r2.xy, cb0[31].wwww, cb0[20].zwzz, v4.xyxx
    r2.xy = ((source[31].wwww)*(source[20].zwzz)+(v4.xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 51: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 52: dp2 r0.w, r5.xyxx, r5.xyxx
    r0.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 53: mul r5.xy, r5.xyxx, cb0[31].xxxx
    r5.xy = ((r5.xyxx)*(source[31].xxxx)).xy;
    // 54: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 56: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 57: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 58: add r6.xyz, -r5.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r5.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 59: mad r6.xyz, cb0[32].yyyy, r6.xyzx, r5.xyzx
    r6.xyz = ((source[32].yyyy)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 60: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 61: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 62: div r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)/(r0.wwww)).xyz;
    // 63: add r7.xyz, -r6.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r6.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 64: mad r7.xyz, r7.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r6.xyzx
    r7.xyz = ((r7.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r6.xyzx)).xyz;
    // 65: mul r8.xyz, r7.yyyy, cb0[3].xyzx
    r8.xyz = ((r7.yyyy)*(source[3].xyzx)).xyz;
    // 66: mad r7.xyw, cb0[2].xyxz, r7.xxxx, r8.xyxz
    r7.xyw = ((source[2].xyxz)*(r7.xxxx)+(r8.xyxz)).xyw;
    // 67: mad r7.xyz, cb0[4].xyzx, r7.zzzz, r7.xywx
    r7.xyz = ((source[4].xyzx)*(r7.zzzz)+(r7.xywx)).xyz;
    // 68: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 69: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 70: mul r8.xyz, r0.wwww, v5.xyzx
    r8.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 71: mad r9.xyz, v5.xyzx, r0.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 72: dp3 r0.w, r8.xyzx, r7.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 73: mul r7.xy, r6.xyxx, r0.wwww
    r7.xy = ((r6.xyxx)*(r0.wwww)).xy;
    // 74: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r8.xyxx
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r8.xyxx))).xy;
    // 75: mad r2.xy, r2.xyxx, cb0[20].xyxx, r7.xyxx
    r2.xy = ((r2.xyxx)*(source[20].xyxx)+(r7.xyxx)).xy;
    // 76: add r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t3.xywz, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).w;
    // 78: mul_sat r0.w, r0.w, cb0[19].w
    r0.w = (saturate((r0.wwww)*(source[19].wwww))).w;
    // 79: mad r3.xyz, r0.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 80: mad r1.xyz, cb0[21].wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((source[21].wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 81: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 83: mad r1.xyz, cb0[31].yyyy, r3.xyzx, r1.xyzx
    r1.xyz = ((source[31].yyyy)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 84: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 86: mad r1.xyz, cb0[31].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[31].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 87: mad r3.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 88: mad r4.xyz, cb0[12].wwww, cb0[12].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[12].wwww)*(source[12].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 89: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 90: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 91: mad r1.xyz, r0.xyzx, r1.xyzx, -r0.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 92: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 93: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 94: mul r1.xyz, cb0[7].xyzx, cb0[7].wwww
    r1.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 95: mad r4.xyz, cb0[8].wwww, cb0[8].xyzx, -r1.xyzx
    r4.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r1.xyzx))).xyz;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 97: mov_sat r7.xyz, r7.xyzx
    r7.xyz = (saturate(r7.xyzx)).xyz;
    // 98: mad r1.xyz, r7.xxxx, r4.xyzx, r1.xyzx
    r1.xyz = ((r7.xxxx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 99: mad r4.xyz, cb0[9].wwww, cb0[9].xyzx, -r1.xyzx
    r4.xyz = ((source[9].wwww)*(source[9].xyzx)+(-(r1.xyzx))).xyz;
    // 100: mad r1.xyz, r7.yyyy, r4.xyzx, r1.xyzx
    r1.xyz = ((r7.yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 101: mad r4.xyz, cb0[10].wwww, cb0[10].xyzx, -r1.xyzx
    r4.xyz = ((source[10].wwww)*(source[10].xyzx)+(-(r1.xyzx))).xyz;
    // 102: mad r1.xyz, r7.zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((r7.zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 103: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 104: add r4.xyz, -r1.xyzx, r0.wwww
    r4.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 105: mad r1.xyz, cb0[31].yyyy, r4.xyzx, r1.xyzx
    r1.xyz = ((source[31].yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 106: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 107: add r4.xyz, -r1.xyzx, r0.wwww
    r4.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 108: mad r1.xyz, cb0[31].zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((source[31].zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 109: mul r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.xyzx)).xyz;
    // 110: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 111: deriv_rtx_coarse r1.xy, r2.zwzz
    r1.xy = (ddx_coarse(r2.zwzz)).xy;
    // 112: deriv_rty_coarse r1.zw, r2.zzzw
    r1.zw = (ddy_coarse(r2.zzzw)).zw;
    // 113: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 114: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 115: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 116: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 117: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 118: rcp r1.x, |r0.w|
    r1.x = (1.0/(abs(r0.wwww))).x;
    // 119: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 120: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 121: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 122: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 123: mul r1.z, r1.z, cb0[32].z
    r1.z = ((r1.zzzz)*(source[32].zzzz)).z;
    // 124: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 125: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 126: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 127: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 128: mul r1.z, r1.z, cb0[32].w
    r1.z = ((r1.zzzz)*(source[32].wwww)).z;
    // 129: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 130: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 131: add r0.w, |r0.w|, r1.x
    r0.w = ((abs(r0.wwww))+(r1.xxxx)).w;
    // 132: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 133: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 134: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 135: mul r1.xzw, r1.xxxx, v0.xxyz
    r1.xzw = ((r1.xxxx)*(v0.xxyz)).xzw;
    // 136: dp3 r3.x, r1.xzwx, r6.xyzx
    r3.x = (dot((r1.xzwx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 137: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 138: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 139: mul r4.xyz, r2.wwww, v1.xyzx
    r4.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 140: dp3 r3.z, r4.xyzx, r6.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 141: mul r10.xyz, r1.zwxz, r4.zxyz
    r10.xyz = ((r1.zwxz)*(r4.zxyz)).xyz;
    // 142: mad r10.xyz, r4.yzxy, r1.wxzw, -r10.xyzx
    r10.xyz = ((r4.yzxy)*(r1.wxzw)+(-(r10.xyzx))).xyz;
    // 143: mul r10.xyz, r10.xyzx, v1.wwww
    r10.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 144: dp3 r3.y, r10.xyzx, r6.xyzx
    r3.y = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 145: dp3 r6.y, r10.xyzx, r8.xyzx
    r6.y = (dot((r10.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 146: dp3 r6.x, r1.xzwx, r8.xyzx
    r6.x = (dot((r1.xzwx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 147: dp3 r6.z, r4.xyzx, r8.xyzx
    r6.z = (dot((r4.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 148: dp3 r2.w, r3.xyzx, r6.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 149: mul r3.xyz, r3.xyzx, r2.wwww
    r3.xyz = ((r3.xyzx)*(r2.wwww)).xyz;
    // 150: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 151: mov r3.w, -r3.x
    r3.w = (-(r3.xxxx)).w;
    // 152: dp2 r2.w, r3.ywyy, r3.ywyy
    r2.w = (dot((r3.ywyy).xy,(r3.ywyy).xy).xxxx).w;
    // 153: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 154: div r3.xy, r3.ywyy, r2.wwww
    r3.xy = ((r3.ywyy)/(r2.wwww)).xy;
    // 155: mad r2.w, -r3.z, l(0.250000), l(0.250000)
    r2.w = ((-(r3.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 156: add r3.z, r3.z, l(1.000000)
    r3.z = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 157: mul r3.z, r3.z, l(0.500000)
    r3.z = ((r3.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 158: mad r3.xy, r2.wwww, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.wwww)*(r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 159: sample_l_indexable(texture2d)(float,float,float,float) r3.xyw, r3.xyxx, t5.xywz, s5, r0.w
    r3.xyw = ((g_SourceCharacterTexture5.SampleLevel(SourceCharacterLookupSampler, (r3.xyxx).xy, (r0.wwww).x)).xywz).xyw;
    // 160: log r6.xyz, r3.xywx
    r6.xyz = (log2(r3.xywx)).xyz;
    // 161: rcp r0.w, cb0[33].x
    r0.w = (1.0/(source[33].xxxx)).w;
    // 162: mul r11.xyz, r6.xyzx, r0.wwww
    r11.xyz = ((r6.xyzx)*(r0.wwww)).xyz;
    // 163: mul r6.xyz, r6.xyzx, cb0[33].xxxx
    r6.xyz = ((r6.xyzx)*(source[33].xxxx)).xyz;
    // 164: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 165: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 166: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 167: mad r6.xyz, r6.xyzx, cb0[33].xxxx, r11.xyzx
    r6.xyz = ((r6.xyzx)*(source[33].xxxx)+(r11.xyzx)).xyz;
    // 168: add r3.xyw, r3.xyxw, r6.xyxz
    r3.xyw = ((r3.xyxw)+(r6.xyxz)).xyw;
    // 169: mul r3.xyw, r3.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r3.xyw = ((r3.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 170: add r0.w, cb0[33].x, l(1.000000)
    r0.w = ((source[33].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: mul r3.xyw, r0.wwww, r3.xyxw
    r3.xyw = ((r0.wwww)*(r3.xyxw)).xyw;
    // 172: dp3 r0.w, r3.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 173: add r3.xyw, -cb0[22].xyxz, cb0[23].xyxz
    r3.xyw = ((-(source[22].xyxz))+(source[23].xyxz)).xyw;
    // 174: mad r3.xyz, r3.zzzz, r3.xywx, cb0[22].xyzx
    r3.xyz = ((r3.zzzz)*(r3.xywx)+(source[22].xyzx)).xyz;
    // 175: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 176: mul r3.xyz, r3.xyzx, cb0[33].yyyy
    r3.xyz = ((r3.xyzx)*(source[33].yyyy)).xyz;
    // 177: mul r6.xyz, r0.xyzx, r3.xyzx
    r6.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 178: add r0.w, r7.y, r7.x
    r0.w = ((r7.yyyy)+(r7.xxxx)).w;
    // 179: add r0.w, r7.z, r0.w
    r0.w = ((r7.zzzz)+(r0.wwww)).w;
    // 180: add_sat r0.w, r7.w, r0.w
    r0.w = (saturate((r7.wwww)+(r0.wwww))).w;
    // 181: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 182: add r7.xyz, -r2.xyzx, r2.wwww
    r7.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 183: mad r2.xyz, cb0[31].yyyy, r7.xyzx, r2.xyzx
    r2.xyz = ((source[31].yyyy)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 184: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 185: add r7.xyz, -r2.xyzx, r2.wwww
    r7.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 186: mad r2.xyz, cb0[31].zzzz, r7.xyzx, r2.xyzx
    r2.xyz = ((source[31].zzzz)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 187: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 188: add r7.xyz, -r2.xyzx, r2.wwww
    r7.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 189: mul r7.xyz, r7.xyzx, cb0[33].zzzz
    r7.xyz = ((r7.xyzx)*(source[33].zzzz)).xyz;
    // 190: mad r2.xyz, r0.wwww, r7.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 191: max r7.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 192: log r7.xyz, r7.xyzx
    r7.xyz = (log2(r7.xyzx)).xyz;
    // 193: mul r7.xyz, r7.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 194: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 195: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 197: mul r0.w, r0.w, cb0[34].z
    r0.w = ((r0.wwww)*(source[34].zzzz)).w;
    // 198: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 199: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 200: mad r2.w, -r0.w, r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 201: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 202: div r2.w, cb0[34].w, r2.w
    r2.w = ((source[34].wwww)/(r2.wwww)).w;
    // 203: dp3 r3.w, r5.xyzx, r5.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 204: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 205: div r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)/(r3.wwww)).xyz;
    // 206: dp3 r3.w, r5.xyzx, r8.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 207: mul_sat r4.w, r3.w, cb0[33].w
    r4.w = (saturate((r3.wwww)*(source[33].wwww))).w;
    // 208: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: mul_sat r5.w, r8.z, cb0[33].w
    r5.w = (saturate((r8.zzzz)*(source[33].wwww))).w;
    // 211: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 212: add_sat r5.w, r5.w, -cb0[34].x
    r5.w = (saturate((r5.wwww)+(-(source[34].xxxx)))).w;
    // 213: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 214: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 215: mul r6.w, r6.w, cb0[34].y
    r6.w = ((r6.wwww)*(source[34].yyyy)).w;
    // 216: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 217: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 218: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 219: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 220: mul r3.xyz, r3.xyzx, r2.wwww
    r3.xyz = ((r3.xyzx)*(r2.wwww)).xyz;
    // 221: mad r2.xyz, r2.xyzx, r3.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 222: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r2.w, r2.w, cb0[35].x
    r2.w = ((r2.wwww)*(source[35].xxxx)).w;
    // 224: mad r2.xyz, r2.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 225: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 226: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 227: div r3.xyz, r9.xyzx, r3.xxxx
    r3.xyz = ((r9.xyzx)/(r3.xxxx)).xyz;
    // 228: dp3 r3.x, r3.xyzx, r8.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 229: add r3.y, -|r8.z|, l(1.000000)
    r3.y = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 230: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 231: mul r3.z, |r3.x|, |r3.x|
    r3.z = ((abs(r3.xxxx))*(abs(r3.xxxx))).z;
    // 232: mul r3.yz, r3.wwzw, r3.yyzy
    r3.yz = ((r3.wwzw)*(r3.yyzy)).yz;
    // 233: mul r3.z, r3.z, |r3.x|
    r3.z = ((r3.zzzz)*(abs(r3.xxxx))).z;
    // 234: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 235: movc r3.x, r3.x, l(0), r3.z
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).x;
    // 236: add r3.z, r3.x, l(-0.027778)
    r3.z = ((r3.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 237: mad r3.x, r3.x, r3.z, l(0.027778)
    r3.x = ((r3.xxxx)*(r3.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 238: div_sat r2.w, r3.x, r2.w
    r2.w = (saturate((r3.xxxx)/(r2.wwww))).w;
    // 239: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 240: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 241: mad r3.xzw, r1.yyyy, r2.xxyz, -r0.xxyz
    r3.xzw = ((r1.yyyy)*(r2.xxyz)+(-(r0.xxyz))).xzw;
    // 242: mad r0.xyz, r0.wwww, r3.xzwx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 243: add r0.w, -cb0[6].w, l(1.000000)
    r0.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 244: mul r0.w, r0.w, cb0[31].w
    r0.w = ((r0.wwww)*(source[31].wwww)).w;
    // 245: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 246: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 247: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 248: mul r1.y, cb0[6].z, l(1.500000)
    r1.y = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 249: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 250: mad r0.w, r0.w, l(0.500000), cb0[6].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 251: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 252: mul r6.x, r1.y, l(0.125000)
    r6.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 253: mul r7.y, cb0[6].y, cb0[27].y
    r7.y = ((source[6].yyyy)*(source[27].yyyy)).y;
    // 254: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 255: mov r7.xw, l(0,0,0,0)
    r7.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 256: add r3.xz, r6.xxyx, r7.xxyx
    r3.xz = ((r6.xxyx)+(r7.xxyx)).xz;
    // 257: frc r1.y, cb0[6].x
    r1.y = (frac(source[6].xxxx)).y;
    // 258: add r2.w, -r1.y, cb0[6].x
    r2.w = ((-(r1.yyyy))+(source[6].xxxx)).w;
    // 259: mul r7.z, r2.w, l(0.125000)
    r7.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 260: add r3.xz, r3.xxzx, r7.zzwz
    r3.xz = ((r3.xxzx)+(r7.zzwz)).xz;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r3.xzxx, t6.xyzw, s6, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 262: mul r3.xzw, r0.wwww, r6.xxyz
    r3.xzw = ((r0.wwww)*(r6.xxyz)).xzw;
    // 263: mul r0.w, r1.y, r6.w
    r0.w = ((r1.yyyy)*(r6.wwww)).w;
    // 264: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 265: mad r3.xzw, r3.xxzw, l(2.000000, 0.000000, 2.000000, 2.000000), -r0.xxyz
    r3.xzw = ((r3.xxzw)*(float4(2.000000,0.000000,2.000000,2.000000))+(-(r0.xxyz))).xzw;
    // 266: mad r0.xyz, r0.wwww, r3.xzwx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xzwx)+(r0.xyzx)).xyz;
    // 267: add r6.xyzw, v7.yzxy, cb0[0].yzxy
    r6.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 268: add r6.xyzw, r6.xyzw, -cb0[1].yzxy
    r6.xyzw = ((r6.xyzw)+(-(source[1].yzxy))).xyzw;
    // 269: add r3.xz, -r6.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r3.xz = ((-(r6.xxyx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 270: add r3.xz, -r6.zzwz, r3.xxzx
    r3.xz = ((-(r6.zzwz))+(r3.xxzx)).xz;
    // 271: mad r3.xz, cb0[28].wwww, r3.xxzx, r6.zzwz
    r3.xz = ((source[28].wwww)*(r3.xxzx)+(r6.zzwz)).xz;
    // 272: mul r0.w, cb0[28].y, cb0[31].w
    r0.w = ((source[28].yyyy)*(source[31].wwww)).w;
    // 273: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 274: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 275: mul r6.y, r0.w, l(0.020000)
    r6.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 276: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 277: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 278: mul r2.w, cb0[28].x, l(0.001000)
    r2.w = ((source[28].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 279: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 280: mad r3.xz, r2.wwww, r3.xxzx, r6.xxyx
    r3.xz = ((r2.wwww)*(r3.xxzx)+(r6.xxyx)).xz;
    // 281: dp2 r2.w, cb0[29].xyxx, r3.xzxx
    r2.w = (dot((source[29].xyxx).xy,(r3.xzxx).xy).xxxx).w;
    // 282: dp2 r6.y, cb0[30].xyxx, r3.xzxx
    r6.y = (dot((source[30].xyxx).xy,(r3.xzxx).xy).xxxx).y;
    // 283: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 284: mul r6.x, r2.w, l(0.125000)
    r6.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 285: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t6.xyzw, s6, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 286: mad r3.xzw, r6.xxyz, l(3.500000, 0.000000, 3.500000, 3.500000), -r0.xxyz
    r3.xzw = ((r6.xxyz)*(float4(3.500000,0.000000,3.500000,3.500000))+(-(r0.xxyz))).xzw;
    // 287: mul r2.w, r6.w, l(0.900000)
    r2.w = ((r6.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 288: mad r3.xzw, r2.wwww, r3.xxzw, r0.xxyz
    r3.xzw = ((r2.wwww)*(r3.xxzw)+(r0.xxyz)).xzw;
    // 289: mul_sat r3.xzw, r0.wwww, r3.xxzw
    r3.xzw = (saturate((r0.wwww)*(r3.xxzw))).xzw;
    // 290: mad r6.xyz, cb0[28].zzzz, r3.xzwx, -r0.xyzx
    r6.xyz = ((source[28].zzzz)*(r3.xzwx)+(-(r0.xyzx))).xyz;
    // 291: mul r3.xzw, r3.xxzw, cb0[28].zzzz
    r3.xzw = ((r3.xxzw)*(source[28].zzzz)).xzw;
    // 292: dp3 r0.w, r3.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 293: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 294: mad r0.xyz, r0.wwww, r6.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 295: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 296: mad r3.xzw, r4.wwww, cb0[25].xxyz, -cb0[25].xxyz
    r3.xzw = ((r4.wwww)*(source[25].xxyz)+(-(source[25].xxyz))).xzw;
    // 297: mad r3.xzw, cb0[25].wwww, r3.xxzw, cb0[25].xxyz
    r3.xzw = ((source[25].wwww)*(r3.xxzw)+(source[25].xxyz)).xzw;
    // 298: mad r3.xzw, r4.wwww, cb0[24].xxyz, r3.xxzw
    r3.xzw = ((r4.wwww)*(source[24].xxyz)+(r3.xxzw)).xzw;
    // 299: mad r2.xyz, r1.yyyy, r2.xyzx, r3.xzwx
    r2.xyz = ((r1.yyyy)*(r2.xyzx)+(r3.xzwx)).xyz;
    // 300: log r0.w, |r3.y|
    r0.w = (log2(abs(r3.yyyy))).w;
    // 301: lt r1.y, |r3.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 302: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 303: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 304: mul r3.xyz, r0.wwww, cb0[26].xyzx
    r3.xyz = ((r0.wwww)*(source[26].xyzx)).xyz;
    // 305: movc r3.xyz, r1.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 306: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 307: add r2.xyz, r2.xyzx, cb0[5].xyzx
    r2.xyz = ((r2.xyzx)+(source[5].xyzx)).xyz;
    // 308: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 309: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 310: mul r3.xyz, r0.wwww, r5.xyzx
    r3.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 311: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 312: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 313: mul r5.xyz, r0.wwww, v6.xyzx
    r5.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 314: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 315: mad r5.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 316: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 317: mul r5.yzw, r5.yyyy, cb0[37].xxyz
    r5.yzw = ((r5.yyyy)*(source[37].xxyz)).yzw;
    // 318: mad r5.xyz, r5.xxxx, cb0[36].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[36].xyzx)+(r5.yzwy)).xyz;
    // 319: mul r5.xyz, r5.xyzx, cb0[38].wwww
    r5.xyz = ((r5.xyzx)*(source[38].wwww)).xyz;
    // 320: mad r2.xyz, r5.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 321: mul r5.xyz, r0.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 322: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 323: mad o0.xyz, r0.xyzx, cb0[38].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[38].xyzx)+(r2.xyzx)).xyz;
    // 324: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 325: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 326: dp3 r0.x, r1.xzwx, r3.xyzx
    r0.x = (dot((r1.xzwx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 327: dp3 r0.z, r4.xyzx, r3.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 328: dp3 r0.y, r10.xyzx, r3.xyzx
    r0.y = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 329: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 330: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 331: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 332: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 333: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 334: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 335: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 336: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 337: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 338: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 339: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 340: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 341: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 342: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 343: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 344: ret
    return output;
}

// source.vehicle.starlight-shell-translucent.v1 / source program e0cd09e59a19024a9c0e0a530bb65f35
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase88(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16].w=(g_SourceCharacterTime.xxxx).x;
    // Existing source draw uses full material coverage and identity colour scale.
    source[0].w = 1.f; source[21].x = 1.f;
    // Sky-light rows are engine owned; the forward pass supplies scene ambient.
    source[18] = float4(input.lightColor, 1.f); source[19] = source[18]; source[20].w = 1.f;
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
    // 15: mul r5.xy, r4.xyxx, cb0[13].xxxx
    r5.xy = ((r4.xyxx)*(source[13].xxxx)).xy;
    // 16: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 17: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 19: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 20: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 24: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 25: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 26: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 27: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 28: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 29: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 31: mov_sat r7.x, r7.x
    r7.x = (saturate(r7.xxxx)).x;
    // 32: mul_sat r1.w, r7.x, cb0[17].w
    r1.w = (saturate((r7.xxxx)*(source[17].wwww))).w;
    // 33: mul r2.w, r1.w, cb0[0].w
    r2.w = ((r1.wwww)*(source[0].wwww)).w;
    // 34: mul r8.xyz, cb0[3].xyzx, cb0[3].wwww
    r8.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 35: dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 36: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r3.wwww
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r3.wwww)).xyz;
    // 37: mad r8.xyz, cb0[13].yyyy, r9.xyzx, r8.xyzx
    r8.xyz = ((source[13].yyyy)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 38: dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: add r9.xyz, -r8.xyzx, r3.wwww
    r9.xyz = ((-(r8.xyzx))+(r3.wwww)).xyz;
    // 40: mad r8.xyz, cb0[13].zzzz, r9.xyzx, r8.xyzx
    r8.xyz = ((source[13].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 41: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 42: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 44: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 45: dp3 r3.w, r7.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: add r9.xyz, -r7.yzwy, r3.wwww
    r9.xyz = ((-(r7.yzwy))+(r3.wwww)).xyz;
    // 47: mad r7.xyz, cb0[13].yyyy, r9.xyzx, r7.yzwy
    r7.xyz = ((source[13].yyyy)*(r9.xyzx)+(r7.yzwy)).xyz;
    // 48: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 49: add r9.xyz, -r7.xyzx, r3.wwww
    r9.xyz = ((-(r7.xyzx))+(r3.wwww)).xyz;
    // 50: mad r7.xyz, cb0[13].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[13].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 51: dp3 r9.x, r1.xyzx, r6.xyzx
    r9.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 52: dp3 r9.y, r2.xyzx, r6.xyzx
    r9.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 53: mul r6.xy, cb0[0].xyxx, l(0.001000, 0.001000, 0.000000, 0.000000)
    r6.xy = ((source[0].xyxx)*(float4(0.001000,0.001000,0.000000,0.000000))).xy;
    // 54: mad r6.xy, cb0[13].wwww, r9.xyxx, r6.xyxx
    r6.xy = ((source[13].wwww)*(r9.xyxx)+(r6.xyxx)).xy;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 56: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: add r9.xyz, -r6.xyzx, r3.wwww
    r9.xyz = ((-(r6.xyzx))+(r3.wwww)).xyz;
    // 58: mad r6.xyz, cb0[14].xxxx, r9.xyzx, r6.xyzx
    r6.xyz = ((source[14].xxxx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 59: mul r6.xyz, r6.xyzx, cb0[6].xyzx
    r6.xyz = ((r6.xyzx)*(source[6].xyzx)).xyz;
    // 60: mad r6.xyz, cb0[14].yyyy, r6.xyzx, r6.xyzx
    r6.xyz = ((source[14].yyyy)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 61: add r6.xyz, r6.xyzx, -cb0[14].yyyy
    r6.xyz = ((r6.xyzx)+(-(source[14].yyyy))).xyz;
    // 62: mov_sat r9.xyz, r6.xyzx
    r9.xyz = (saturate(r6.xyzx)).xyz;
    // 63: add r9.xyz, r9.xyzx, cb0[14].zzzz
    r9.xyz = ((r9.xyzx)+(source[14].zzzz)).xyz;
    // 64: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 65: add r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 66: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 67: mul r6.xyz, r6.xyzx, cb0[14].wwww
    r6.xyz = ((r6.xyzx)*(source[14].wwww)).xyz;
    // 68: max r6.xyz, r6.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r6.xyz = (max(r6.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 69: min r6.xyz, r6.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 70: mad r7.xyz, r8.xyzx, r7.xyzx, r6.xyzx
    r7.xyz = ((r8.xyzx)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r9.xyz, -r8.xyzx, r3.wwww
    r9.xyz = ((-(r8.xyzx))+(r3.wwww)).xyz;
    // 74: mad r8.xyz, cb0[13].yyyy, r9.xyzx, r8.xyzx
    r8.xyz = ((source[13].yyyy)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 75: dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 76: add r9.xyz, -r8.xyzx, r3.wwww
    r9.xyz = ((-(r8.xyzx))+(r3.wwww)).xyz;
    // 77: mad r8.xyz, cb0[13].zzzz, r9.xyzx, r8.xyzx
    r8.xyz = ((source[13].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 78: dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 79: add r9.xyz, -r8.xyzx, r3.wwww
    r9.xyz = ((-(r8.xyzx))+(r3.wwww)).xyz;
    // 80: mul r9.xyz, r9.xyzx, cb0[15].xxxx
    r9.xyz = ((r9.xyzx)*(source[15].xxxx)).xyz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 82: mov_sat r10.xyz, r10.xyzx
    r10.xyz = (saturate(r10.xyzx)).xyz;
    // 83: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 84: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 85: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 86: mad r8.xyz, r3.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r3.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 87: dp3 r3.w, r4.xyzx, r3.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 88: mul_sat r4.xy, r3.wzww, cb0[15].yyyy
    r4.xy = (saturate((r3.wzww)*(source[15].yyyy))).xy;
    // 89: add r4.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r4.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 90: add_sat r4.y, r4.y, -cb0[15].z
    r4.y = (saturate((r4.yyyy)+(-(source[15].zzzz)))).y;
    // 91: lt r4.z, r4.y, l(0.000001)
    r4.z = (asfloat((uint4)((r4.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 92: log r4.y, r4.y
    r4.y = (log2(r4.yyyy)).y;
    // 93: mul r4.y, r4.y, cb0[15].w
    r4.y = ((r4.yyyy)*(source[15].wwww)).y;
    // 94: exp r4.y, r4.y
    r4.y = (exp2(r4.yyyy)).y;
    // 95: mul r4.x, r4.y, r4.x
    r4.x = ((r4.yyyy)*(r4.xxxx)).x;
    // 96: movc r4.x, r4.z, l(0), r4.x
    r4.x = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 97: max r4.yzw, |r8.xxyz|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r4.yzw = (max(abs(r8.xxyz),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 98: log r4.yzw, r4.yyzw
    r4.yzw = (log2(r4.yyzw)).yzw;
    // 99: mul r4.yzw, r4.yyzw, l(0.000000, 0.454545, 0.454545, 0.454545)
    r4.yzw = ((r4.yyzw)*(float4(0.000000,0.454545,0.454545,0.454545))).yzw;
    // 100: exp r4.yzw, r4.yyzw
    r4.yzw = (exp2(r4.yyzw)).yzw;
    // 101: dp3 r4.y, r4.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.y = (dot((r4.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 102: log r4.y, r4.y
    r4.y = (log2(r4.yyyy)).y;
    // 103: mul r4.y, r4.y, cb0[16].x
    r4.y = ((r4.yyyy)*(source[16].xxxx)).y;
    // 104: exp r4.y, r4.y
    r4.y = (exp2(r4.yyyy)).y;
    // 105: min r4.y, r4.y, l(1.000000)
    r4.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 106: mad r4.z, -r4.y, r4.y, l(1.000000)
    r4.z = ((-(r4.yyyy))*(r4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 107: max r4.z, r4.z, l(0.001000)
    r4.z = (max(r4.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 108: div r4.z, cb0[16].y, r4.z
    r4.z = ((source[16].yyyy)/(r4.zzzz)).z;
    // 109: mul r4.z, r4.z, r4.x
    r4.z = ((r4.zzzz)*(r4.xxxx)).z;
    // 110: add r4.w, -r4.y, l(1.000000)
    r4.w = ((-(r4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: mul r4.w, r4.w, cb0[16].z
    r4.w = ((r4.wwww)*(source[16].zzzz)).w;
    // 112: mad r8.xyz, r4.zzzz, r8.xyzx, -r7.xyzx
    r8.xyz = ((r4.zzzz)*(r8.xyzx)+(-(r7.xyzx))).xyz;
    // 113: mad r8.xyz, r4.wwww, r8.xyzx, r7.xyzx
    r8.xyz = ((r4.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 114: add r4.z, -r8.w, l(1.000000)
    r4.z = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 115: lt r4.w, |r4.z|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r4.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 116: log r4.z, |r4.z|
    r4.z = (log2(abs(r4.zzzz))).z;
    // 117: mul r4.z, r4.z, cb0[17].z
    r4.z = ((r4.zzzz)*(source[17].zzzz)).z;
    // 118: exp r4.z, r4.z
    r4.z = (exp2(r4.zzzz)).z;
    // 119: min r4.z, r4.z, l(1.000000)
    r4.z = (min(r4.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 120: mad r9.xyz, v6.xyzx, r0.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v6.xyzx)*(r0.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 121: dp3 r0.w, r9.xyzx, r9.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 122: sqrt r5.w, r0.w
    r5.w = (sqrt(r0.wwww)).w;
    // 123: div r9.xyz, r9.xyzx, r5.wwww
    r9.xyz = ((r9.xyzx)/(r5.wwww)).xyz;
    // 124: dp3 r3.x, r9.xyzx, r3.xyzx
    r3.x = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 125: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 126: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 127: mul r5.w, |r3.x|, |r3.x|
    r5.w = ((abs(r3.xxxx))*(abs(r3.xxxx))).w;
    // 128: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 129: mul r3.x, |r3.x|, r5.w
    r3.x = ((abs(r3.xxxx))*(r5.wwww)).x;
    // 130: movc r3.x, r3.y, l(0), r3.x
    r3.x = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 131: add r3.y, r3.x, l(-0.027778)
    r3.y = ((r3.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 132: mad r3.x, r3.x, r3.y, l(0.027778)
    r3.x = ((r3.xxxx)*(r3.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 133: div_sat r0.w, r3.x, r0.w
    r0.w = (saturate((r3.xxxx)/(r0.wwww))).w;
    // 134: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r0.w, r0.w, r4.z
    r0.w = ((r0.wwww)*(r4.zzzz)).w;
    // 136: movc r0.w, r4.w, l(0), r0.w
    r0.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 137: mad r9.xyz, r0.wwww, r8.xyzx, -r7.xyzx
    r9.xyz = ((r0.wwww)*(r8.xyzx)+(-(r7.xyzx))).xyz;
    // 138: mad r4.yzw, r4.yyyy, r9.xxyz, r7.xxyz
    r4.yzw = ((r4.yyyy)*(r9.xxyz)+(r7.xxyz)).yzw;
    // 139: mul r0.w, cb0[2].z, l(1.500000)
    r0.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 140: add r3.x, -cb0[2].w, l(1.000000)
    r3.x = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 141: mul r3.x, r3.x, cb0[16].w
    r3.x = ((r3.xxxx)*(source[16].wwww)).x;
    // 142: mul r3.x, r3.x, l(6.283185)
    r3.x = ((r3.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 143: sincos r3.x, null, r3.x
    r3.x = (sin(r3.xxxx)).x;
    // 144: add r3.x, r3.x, l(1.000000)
    r3.x = ((r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 145: mul r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)*(r3.xxxx)).w;
    // 146: mad r0.w, r0.w, l(0.500000), cb0[2].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 147: frc r3.x, cb0[2].x
    r3.x = (frac(source[2].xxxx)).x;
    // 148: add r3.y, -r3.x, cb0[2].x
    r3.y = ((-(r3.xxxx))+(source[2].xxxx)).y;
    // 149: mul r7.z, r3.y, l(0.125000)
    r7.z = ((r3.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 150: mov r7.xw, l(0,0,0,0)
    r7.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 151: mul r7.y, cb0[2].y, cb0[12].y
    r7.y = ((source[2].yyyy)*(source[12].yyyy)).y;
    // 152: frc r3.y, v4.x
    r3.y = (frac(v4.xxxx)).y;
    // 153: mul r9.x, r3.y, l(0.125000)
    r9.x = ((r3.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 154: mov r9.y, v4.y
    r9.y = (v4.yyyy).y;
    // 155: add r7.xy, r7.xyxx, r9.xyxx
    r7.xy = ((r7.xyxx)+(r9.xyxx)).xy;
    // 156: add r7.xy, r7.xyxx, r7.zwzz
    r7.xy = ((r7.xyxx)+(r7.zwzz)).xy;
    // 157: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t5.xyzw, s6, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 158: mul r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 159: mul r0.w, r3.x, r7.w
    r0.w = ((r3.xxxx)*(r7.wwww)).w;
    // 160: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.yzwy
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.yzwy))).xyz;
    // 161: mad r4.yzw, r0.wwww, r7.xxyz, r4.yyzw
    r4.yzw = ((r0.wwww)*(r7.xxyz)+(r4.yyzw)).yzw;
    // 162: mad r4.yzw, r4.yyzw, cb2[3].wwww, cb2[3].xxyz
    r4.yzw = ((r4.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 163: add r0.w, -r3.x, l(1.000000)
    r0.w = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: mul r3.xy, v4.wzww, cb0[7].zzzz
    r3.xy = ((v4.wzww)*(source[7].zzzz)).xy;
    // 165: mul r7.xy, cb0[7].xyxx, cb0[16].wwww
    r7.xy = ((source[7].xyxx)*(source[16].wwww)).xy;
    // 166: mad r7.zw, cb0[7].zzzz, v4.wwwz, r7.xxxy
    r7.zw = ((source[7].zzzz)*(v4.wwwz)+(r7.xxxy)).zw;
    // 167: mad r3.xy, r7.xyxx, l(-0.500000, 0.500000, 0.000000, 0.000000), r3.xyxx
    r3.xy = ((r7.xyxx)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r3.xyxx)).xy;
    // 168: sample_b_indexable(texture2d)(float,float,float,float) r3.x, r3.xyxx, t6.xyzw, s5, l(0.000000)
    r3.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 169: mad r3.xy, r3.xxxx, cb0[17].xxxx, r7.zwzz
    r3.xy = ((r3.xxxx)*(source[17].xxxx)+(r7.zwzz)).xy;
    // 170: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r3.xyxx, t6.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 171: mul r9.xyz, cb0[8].xyzx, cb0[17].yyyy
    r9.xyz = ((source[8].xyzx)*(source[17].yyyy)).xyz;
    // 172: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 173: mad r9.xyz, r4.xxxx, r7.xyzx, -r7.xyzx
    r9.xyz = ((r4.xxxx)*(r7.xyzx)+(-(r7.xyzx))).xyz;
    // 174: mad r7.xyz, cb0[8].wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((source[8].wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 175: mad r7.xyz, r0.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r0.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 176: add r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)+(r7.xyzx)).xyz;
    // 177: mad r7.xyz, r4.xxxx, cb0[10].xyzx, -cb0[10].xyzx
    r7.xyz = ((r4.xxxx)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 178: mad r7.xyz, cb0[10].wwww, r7.xyzx, cb0[10].xyzx
    r7.xyz = ((source[10].wwww)*(r7.xyzx)+(source[10].xyzx)).xyz;
    // 179: mad r7.xyz, r4.xxxx, cb0[9].xyzx, r7.xyzx
    r7.xyz = ((r4.xxxx)*(source[9].xyzx)+(r7.xyzx)).xyz;
    // 180: add r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)+(r7.xyzx)).xyz;
    // 181: add r0.w, -|r3.z|, l(1.000000)
    r0.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 182: add r3.x, -|r3.w|, l(1.000000)
    r3.x = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mul r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)*(r3.xxxx)).w;
    // 184: lt r3.x, |r0.w|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 185: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 186: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 187: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 188: mul r3.yzw, r0.wwww, cb0[11].xxyz
    r3.yzw = ((r0.wwww)*(source[11].xxyz)).yzw;
    // 189: movc r3.xyz, r3.xxxx, l(0,0,0,0), r3.yzwy
    r3.xyz = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yzwy)).xyz;
    // 190: add r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)+(r6.xyzx)).xyz;
    // 191: add r3.xyz, r3.xyzx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)+(source[1].xyzx)).xyz;
    // 192: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 193: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 194: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 195: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 196: mad r6.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 197: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 198: mul r6.yzw, r6.yyyy, cb0[19].xxyz
    r6.yzw = ((r6.yyyy)*(source[19].xxyz)).yzw;
    // 199: mad r6.xyz, r6.xxxx, cb0[18].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[18].xyzx)+(r6.yzwy)).xyz;
    // 200: mul r6.xyz, r6.xyzx, cb0[20].wwww
    r6.xyz = ((r6.xyzx)*(source[20].wwww)).xyz;
    // 201: mul r7.xyz, r4.yzwy, r6.xyzx
    r7.xyz = ((r4.yzwy)*(r6.xyzx)).xyz;
    // 202: mad r3.xyz, r6.xyzx, r4.yzwy, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r4.yzwy)+(r3.xyzx)).xyz;
    // 203: mad r3.xyz, r4.yzwy, cb0[20].xyzx, r3.xyzx
    r3.xyz = ((r4.yzwy)*(source[20].xyzx)+(r3.xyzx)).xyz;
    // 204: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 205: mad r0.w, r0.w, l(-0.250000), l(0.400000)
    r0.w = ((r0.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 206: eq r3.w, cb0[21].x, l(0.000000)
    r3.w = (asfloat((uint4)((source[21].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 207: not r4.x, r3.w
    r4.x = (asfloat(~asuint(r3.wwww))).x;
    // 208: lt r5.w, r2.w, r0.w
    r5.w = (asfloat((uint4)((r2.wwww)<(r0.wwww)) * 0xffffffffu)).w;
    // 209: and r4.x, r4.x, r5.w
    r4.x = (asfloat(asuint(r4.xxxx) & asuint(r5.wwww))).x;
    // 210: discard_nz r4.x
    if ((asuint(r4.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 211: ge r0.w, r2.w, r0.w
    r0.w = (asfloat((uint4)((r2.wwww)>=(r0.wwww)) * 0xffffffffu)).w;
    // 212: mad r1.w, r1.w, cb0[0].w, l(-0.900000)
    r1.w = ((r1.wwww)*(source[0].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 213: mul_sat r1.w, r1.w, l(9.999998)
    r1.w = (saturate((r1.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 214: mad r4.x, r1.w, l(-2.000000), l(3.000000)
    r4.x = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 215: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 216: mul r1.w, r1.w, r4.x
    r1.w = ((r1.wwww)*(r4.xxxx)).w;
    // 217: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 218: movc r0.w, r0.w, r1.w, r2.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 219: movc o0.w, r3.w, r0.w, r2.w
    output.targets[0].w = ((asuint(r3.wwww) != 0u) ? (r0.wwww) : (r2.wwww)).w;
    // 220: mad o0.xyz, r3.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 221: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 222: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 223: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 224: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 225: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 226: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 227: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 228: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 229: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 230: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 231: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 232: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 233: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 234: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 235: dp3 o4.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 236: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 237: mov o3.xyz, r4.yzwy
    output.targets[3].xyz = (r4.yzwy).xyz;
    // 238: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 239: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 240: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 241: ret
    return output;
}


