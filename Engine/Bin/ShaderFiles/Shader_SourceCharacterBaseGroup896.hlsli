SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase901(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17].z=(g_SourceCharacterTime.xxxx).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[20]=g_SourceCharacterEnvironmentColor; source[21]=g_SourceCharacterEnvironmentRotation; }
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
    // 7: add r0.x, -cb0[6].w, l(1.000000)
    r0.x = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul r0.x, r0.x, cb0[17].z
    r0.x = ((r0.xxxx)*(source[17].zzzz)).x;
    // 9: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 10: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 11: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r1.x, cb0[6].z, l(1.500000)
    r1.x = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 13: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.500000), cb0[6].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).x;
    // 15: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 16: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[6].y, cb0[13].y
    r2.y = ((source[6].yyyy)*(source[13].yyyy)).y;
    // 18: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 21: frc r1.z, cb0[6].x
    r1.z = (frac(source[6].xxxx)).z;
    // 22: add r1.w, -r1.z, cb0[6].x
    r1.w = ((-(r1.zzzz))+(source[6].xxxx)).w;
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
    // 30: mad r2.xyz, cb0[15].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[15].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[16].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 43: mul r1.z, r6.y, cb0[15].y
    r1.z = ((r6.yyyy)*(source[15].yyyy)).z;
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
    // 67: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 68: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 69: mad r4.xyz, cb0[15].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[15].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 70: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 71: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 72: add r3.xyz, -r4.xyzx, r2.wwww
    r3.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 73: mad r3.xyz, cb0[16].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[16].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 74: mad r4.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 76: mul r4.xyz, r4.xyzx, r7.xywx
    r4.xyz = ((r4.xyzx)*(r7.xywx)).xyz;
    // 77: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 78: mul r7.xyw, r2.xyxz, r3.xyxz
    r7.xyw = ((r2.xyxz)*(r3.xyxz)).xyw;
    // 79: dp3 r2.w, r7.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: mad r2.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 81: mad r2.xyz, cb0[15].wwww, r2.xyzx, r7.xywx
    r2.xyz = ((source[15].wwww)*(r2.xyzx)+(r7.xywx)).xyz;
    // 82: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 84: mad r2.xyz, cb0[16].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 85: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 86: mad r1.xyw, r1.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r2.xyxz
    r1.xyw = ((r1.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r2.xyxz))).xyw;
    // 87: mad r1.xyw, r0.xxxx, r1.xyxw, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 88: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 89: add r0.x, r1.w, r0.x
    r0.x = ((r1.wwww)+(r0.xxxx)).x;
    // 90: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 91: max r0.x, r0.x, cb0[18].x
    r0.x = (max(r0.xxxx,source[18].xxxx)).x;
    // 92: min r0.x, r0.x, cb0[17].w
    r0.x = (min(r0.xxxx,source[17].wwww)).x;
    // 93: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 94: mad r0.x, r1.z, r2.x, r0.x
    r0.x = ((r1.zzzz)*(r2.xxxx)+(r0.xxxx)).x;
    // 95: mul_sat r2.w, r1.z, cb2[3].w
    r2.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 96: add r1.z, r0.x, l(-1.000000)
    r1.z = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 97: mad r1.z, cb0[18].z, r1.z, l(1.000000)
    r1.z = ((source[18].zzzz)*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: mul r3.xyz, r1.xywx, r1.zzzz
    r3.xyz = ((r1.xywx)*(r1.zzzz)).xyz;
    // 99: mul r2.x, r6.x, cb0[17].x
    r2.x = ((r6.xxxx)*(source[17].xxxx)).x;
    // 100: mul r2.y, r6.z, cb0[19].x
    r2.y = ((r6.zzzz)*(source[19].xxxx)).y;
    // 101: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 102: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 103: movc r2.y, r5.z, l(0), r2.y
    r2.y = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 104: max r2.y, r2.y, cb0[0].x
    r2.y = (max(r2.yyyy,source[0].xxxx)).y;
    // 105: min r2.z, r2.y, l(1.000000)
    r2.z = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 107: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 108: add_sat r2.x, r2.x, cb0[17].y
    r2.x = (saturate((r2.xxxx)+(source[17].yyyy))).x;
    // 109: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 110: mul r4.xyz, r2.yyyy, cb0[12].xyzx
    r4.xyz = ((r2.yyyy)*(source[12].xyzx)).xyz;
    // 111: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 112: mad r1.xyz, r1.zzzz, r1.xywx, -r3.xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(-(r3.xyzx))).xyz;
    // 113: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 114: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 115: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 117: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 118: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 119: mad r4.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 120: mad r5.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 121: mad r4.xyz, r0.xxxx, r4.xyzx, r5.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 122: mad r3.xyz, r4.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 123: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 124: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 125: mov_sat r1.w, cb0[18].w
    r1.w = (saturate(source[18].wwww)).w;
    // 126: mad r4.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r4.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 127: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 128: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 129: mad r4.xyz, r2.wwww, r4.xyzx, r2.xxxx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xxxx)).xyz;
    // 130: mul_sat r1.w, r4.y, l(50.000000)
    r1.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 132: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 133: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 134: mul r5.xy, r2.xyxx, cb0[15].xxxx
    r5.xy = ((r2.xyxx)*(source[15].xxxx)).xy;
    // 135: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 136: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 137: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 138: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 139: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 140: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 141: div r5.xyz, r5.xyzx, r2.xxxx
    r5.xyz = ((r5.xyzx)/(r2.xxxx)).xyz;
    // 142: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 143: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 144: mul r6.xyz, r2.xxxx, r5.xyzx
    r6.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 145: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 146: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 147: mul r7.xyw, r2.xxxx, v5.xyxz
    r7.xyw = ((r2.xxxx)*(v5.xyxz)).xyw;
    // 148: dp3 r2.x, r6.xyzx, r7.xywx
    r2.x = (dot((r6.xyzx).xyz,(r7.xywx).xyz).xxxx).x;
    // 149: deriv_rtx_coarse r8.x, r2.x
    r8.x = (ddx_coarse(r2.xxxx)).x;
    // 150: deriv_rty_coarse r8.y, r2.x
    r8.y = (ddy_coarse(r2.xxxx)).y;
    // 151: dp2 r2.y, r8.xyxx, r8.xyxx
    r2.y = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).y;
    // 152: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 153: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 154: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 155: min r8.y, r2.y, l(1.000000)
    r8.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 156: add r2.y, -r8.y, l(1.000000)
    r2.y = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 157: max r9.xyz, r4.xyzx, r2.yyyy
    r9.xyz = (max(r4.xyzx,r2.yyyy)).xyz;
    // 158: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 159: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 160: mul r10.xyz, r2.xxxx, r6.xyzx
    r10.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 161: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xywx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xywx))).xyz;
    // 162: add r1.w, r10.z, l(1.000000)
    r1.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 166: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 167: mul r2.x, r2.x, cb0[1].y
    r2.x = ((r2.xxxx)*(source[1].yyyy)).x;
    // 168: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 169: mad_sat r2.x, r2.x, cb0[1].w, cb0[1].z
    r2.x = (saturate((r2.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 170: mul r2.x, r2.x, cb0[19].y
    r2.x = ((r2.xxxx)*(source[19].yyyy)).x;
    // 171: add_sat r8.x, -r1.w, r2.y
    r8.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 172: sample_indexable(texture2d)(float,float,float,float) r2.yz, r8.xyxx, t5.zxyw, s6
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 173: add r1.w, r0.x, r8.x
    r1.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 174: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 175: mul r8.xzw, r2.zzzz, r4.xxyz
    r8.xzw = ((r2.zzzz)*(r4.xxyz)).xzw;
    // 176: mad r8.xzw, r9.xxyz, r2.yyyy, r8.xxzw
    r8.xzw = ((r9.xxyz)*(r2.yyyy)+(r8.xxzw)).xzw;
    // 177: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 178: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 179: mad r9.xyz, r4.xyzx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((r4.xyzx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: dp3 r2.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 181: mad r4.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 182: mad r11.xyz, -r8.xzwx, r9.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r8.xzwx))*(r9.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mul r8.xzw, r8.xxzw, r9.xxyz
    r8.xzw = ((r8.xxzw)*(r9.xxyz)).xzw;
    // 184: mul r9.xyz, r1.xyzx, r11.xyzx
    r9.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 185: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: mul r9.xyz, r2.yyyy, r9.xyzx
    r9.xyz = ((r2.yyyy)*(r9.xyzx)).xyz;
    // 187: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 188: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 189: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 190: mul r12.xyz, r3.wwww, v1.xyzx
    r12.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 191: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 192: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 193: mul r13.xyz, r3.wwww, v0.xyzx
    r13.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 194: mul r14.xyz, r12.zxyz, r13.yzxy
    r14.xyz = ((r12.zxyz)*(r13.yzxy)).xyz;
    // 195: mad r14.xyz, r12.yzxy, r13.zxyz, -r14.xyzx
    r14.xyz = ((r12.yzxy)*(r13.zxyz)+(-(r14.xyzx))).xyz;
    // 196: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 197: dp3 r15.y, r14.xyzx, r6.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 198: dp3 r14.y, r14.xyzx, r10.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 199: dp3 r15.x, r13.xyzx, r6.xyzx
    r15.x = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 200: dp3 r14.x, r13.xyzx, r10.xyzx
    r14.x = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 201: dp2 r13.z, r15.xyxx, cb0[21].xyxx
    r13.z = (dot((r15.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 202: mul r14.zw, cb0[21].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r14.zw = ((source[21].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 203: dp2 r13.x, r15.xyxx, r14.zwzz
    r13.x = (dot((r15.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 204: dp2 r16.x, r14.xyxx, r14.zwzz
    r16.x = (dot((r14.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 205: dp2 r16.z, r14.xyxx, cb0[21].xyxx
    r16.z = (dot((r14.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 206: dp3 r13.y, r12.xyzx, r6.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 207: dp3 r16.y, r12.xyzx, r10.xyzx
    r16.y = (dot((r12.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 208: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 209: dp4 r12.x, cb0[22].xyzw, r13.xyzw
    r12.x = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 210: dp4 r12.y, cb0[23].xyzw, r13.xyzw
    r12.y = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 211: dp4 r12.z, cb0[24].xyzw, r13.xyzw
    r12.z = (dot((source[24].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 212: mul r14.xyzw, r13.yzzx, r13.xyzz
    r14.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 213: dp4 r17.x, cb0[25].xyzw, r14.xyzw
    r17.x = (dot((source[25].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 214: dp4 r17.y, cb0[26].xyzw, r14.xyzw
    r17.y = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 215: dp4 r17.z, cb0[27].xyzw, r14.xyzw
    r17.z = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 216: add r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)+(r17.xyzx)).xyz;
    // 217: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 218: mov r15.z, r13.y
    r15.z = (r13.yyyy).z;
    // 219: mad r3.w, r13.x, r13.x, -r3.w
    r3.w = ((r13.xxxx)*(r13.xxxx)+(-(r3.wwww))).w;
    // 220: mad r12.xyz, cb0[28].xyzx, r3.wwww, r12.xyzx
    r12.xyz = ((source[28].xyzx)*(r3.wwww)+(r12.xyzx)).xyz;
    // 221: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 222: mul r12.xyz, r12.xyzx, cb0[20].xyzx
    r12.xyz = ((r12.xyzx)*(source[20].xyzx)).xyz;
    // 223: mul r12.xyz, r12.xyzx, cb0[21].zzzz
    r12.xyz = ((r12.xyzx)*(source[21].zzzz)).xyz;
    // 224: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[20].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[20].wwww)).xyz;
    // 225: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 226: add r12.xyz, -r3.wwww, r12.xyzx
    r12.xyz = ((-(r3.wwww))+(r12.xyzx)).xyz;
    // 227: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 228: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: mad r4.w, r8.y, l(2.000000), l(2.000000)
    r4.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 230: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 231: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 232: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 233: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 234: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 235: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 236: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 237: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 238: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 239: mul r12.xyz, r3.wwww, r12.xyzx
    r12.xyz = ((r3.wwww)*(r12.xyzx)).xyz;
    // 240: mul r9.xyz, r9.xyzx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r12.xyzx)).xyz;
    // 241: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 242: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 243: mul r3.w, r8.y, l(5.000000)
    r3.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 244: mul r5.w, r8.y, r8.y
    r5.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 245: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 246: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 247: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 248: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 249: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 250: sample_l_indexable(texturecube)(float,float,float,float) r12.xyzw, r16.xyzx, t6.xyzw, s5, r3.w
    r12.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r16.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 251: mul r12.xyz, r12.xyzx, r12.wwww
    r12.xyz = ((r12.xyzx)*(r12.wwww)).xyz;
    // 252: mul r12.xyz, r12.xyzx, cb0[20].xyzx
    r12.xyz = ((r12.xyzx)*(source[20].xyzx)).xyz;
    // 253: mul r12.xyz, r12.xyzx, cb0[21].zzzz
    r12.xyz = ((r12.xyzx)*(source[21].zzzz)).xyz;
    // 254: mad r12.xyz, r12.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[20].wwww
    r12.xyz = ((r12.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[20].wwww)).xyz;
    // 255: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 256: add r12.xyz, -r1.wwww, r12.xyzx
    r12.xyz = ((-(r1.wwww))+(r12.xyzx)).xyz;
    // 257: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 258: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 259: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 260: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 261: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 262: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 263: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 264: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 265: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 266: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 267: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 268: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 269: mul r13.xyz, r8.xzwx, r12.xyzx
    r13.xyz = ((r8.xzwx)*(r12.xyzx)).xyz;
    // 270: mad r1.w, r0.x, r4.x, r4.y
    r1.w = ((r0.xxxx)*(r4.xxxx)+(r4.yyyy)).w;
    // 271: mad r1.w, r1.w, r0.x, r4.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r4.zzzz)).w;
    // 272: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 273: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 274: mad r4.xyz, r13.xyzx, r0.xxxx, r9.xyzx
    r4.xyz = ((r13.xyzx)*(r0.xxxx)+(r9.xyzx)).xyz;
    // 275: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 276: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 277: mul r9.xyz, r1.wwww, v6.xyzx
    r9.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 278: dp3 r1.w, r9.xyzx, r6.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 279: dp3 r2.z, -r9.xyzx, r6.xyzx
    r2.z = (dot((-(r9.xyzx)).xyz,(r6.xyzx).xyz).xxxx).z;
    // 280: dp3 r3.w, r9.xyzx, r10.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 281: mad r6.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 282: mad r6.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 283: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 284: mad r9.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 285: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 286: mul r9.yzw, r9.yyyy, cb0[31].xxyz
    r9.yzw = ((r9.yyyy)*(source[31].xxyz)).yzw;
    // 287: mad r9.xyz, r9.xxxx, cb0[30].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[30].xyzx)+(r9.yzwy)).xyz;
    // 288: mul r9.xyz, r9.xyzx, cb0[32].wwww
    r9.xyz = ((r9.xyzx)*(source[32].wwww)).xyz;
    // 289: mul r9.xyz, r1.xyzx, r9.xyzx
    r9.xyz = ((r1.xyzx)*(r9.xyzx)).xyz;
    // 290: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 291: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 292: mul r3.xyz, r11.xyzx, r3.xyzx
    r3.xyz = ((r11.xyzx)*(r3.xyzx)).xyz;
    // 293: mad r3.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 294: mad r3.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 295: mul r4.xyz, r6.yyyy, cb0[31].xyzx
    r4.xyz = ((r6.yyyy)*(source[31].xyzx)).xyz;
    // 296: mad r4.xyz, cb0[30].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[30].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 297: mul r4.xyz, r4.xyzx, cb0[32].wwww
    r4.xyz = ((r4.xyzx)*(source[32].wwww)).xyz;
    // 298: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 299: mul r4.xyz, r12.xyzx, r4.xyzx
    r4.xyz = ((r12.xyzx)*(r4.xyzx)).xyz;
    // 300: mul r4.xyz, r4.xyzx, r8.xzwx
    r4.xyz = ((r4.xyzx)*(r8.xzwx)).xyz;
    // 301: mad r3.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 302: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 303: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 304: dp3 r0.x, r5.xyzx, r7.xywx
    r0.x = (dot((r5.xyzx).xyz,(r7.xywx).xyz).xxxx).x;
    // 305: mul_sat r1.w, r0.x, cb0[16].y
    r1.w = (saturate((r0.xxxx)*(source[16].yyyy))).w;
    // 306: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 307: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 308: mul_sat r2.z, r7.w, cb0[16].y
    r2.z = (saturate((r7.wwww)*(source[16].yyyy))).z;
    // 309: add r2.w, -|r7.w|, l(1.000000)
    r2.w = ((-(abs(r7.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 310: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 311: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 312: add_sat r2.z, r2.z, -cb0[16].z
    r2.z = (saturate((r2.zzzz)+(-(source[16].zzzz)))).z;
    // 313: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 314: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 315: mul r2.w, r2.w, cb0[16].w
    r2.w = ((r2.wwww)*(source[16].wwww)).w;
    // 316: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 317: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 318: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 319: add r2.z, -r7.z, r1.w
    r2.z = ((-(r7.zzzz))+(r1.wwww)).z;
    // 320: mad r4.xyz, r1.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r4.xyz = ((r1.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 321: mad r4.xyz, cb0[10].wwww, r4.xyzx, cb0[10].xyzx
    r4.xyz = ((source[10].wwww)*(r4.xyzx)+(source[10].xyzx)).xyz;
    // 322: mad r1.w, cb0[9].w, r2.z, r7.z
    r1.w = ((source[9].wwww)*(r2.zzzz)+(r7.zzzz)).w;
    // 323: mad r4.xyz, r1.wwww, cb0[9].xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(source[9].xyzx)+(r4.xyzx)).xyz;
    // 324: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 325: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 326: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 327: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 328: mul r5.xyz, r1.wwww, cb0[11].xyzx
    r5.xyz = ((r1.wwww)*(source[11].xyzx)).xyz;
    // 329: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 330: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 331: mad r0.xyz, cb0[15].zzzz, r0.yzwy, r4.xyzx
    r0.xyz = ((source[15].zzzz)*(r0.yzwy)+(r4.xyzx)).xyz;
    // 332: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 333: mul r4.xyz, r6.wwww, cb0[31].xyzx
    r4.xyz = ((r6.wwww)*(source[31].xyzx)).xyz;
    // 334: mad r4.xyz, r6.zzzz, cb0[30].xyzx, r4.xyzx
    r4.xyz = ((r6.zzzz)*(source[30].xyzx)+(r4.xyzx)).xyz;
    // 335: mul r4.xyz, r4.xyzx, cb0[32].wwww
    r4.xyz = ((r4.xyzx)*(source[32].wwww)).xyz;
    // 336: mul_sat r5.xyz, cb0[14].xyzx, cb0[14].wwww
    r5.xyz = (saturate((source[14].xyzx)*(source[14].wwww))).xyz;
    // 337: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 338: mul r5.xyz, r5.xyzx, cb0[19].yyyy
    r5.xyz = ((r5.xyzx)*(source[19].yyyy)).xyz;
    // 339: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 340: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 341: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 342: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 343: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 344: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 345: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 346: mad o0.xyz, r1.xyzx, cb0[32].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[32].xyzx)+(r0.xyzx)).xyz;
    // 347: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 348: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 349: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 350: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 351: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 352: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 353: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 354: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 355: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 356: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 357: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 358: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 359: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 360: ftou r0.x, cb0[29].z
    r0.x = (asfloat((uint4)(source[29].zzzz))).x;
    // 361: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 362: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 363: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 364: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 365: ret
    return output;
}

// source.character.equipment-native-902.v1 / source program 8f70f70fcada4b4cb6fcd9ca0f3ef1dc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase902(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].y=(g_SourceCharacterTime.xxxx).x;
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
    // 7: add r0.x, -cb0[7].w, l(1.000000)
    r0.x = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul r0.x, r0.x, cb0[20].y
    r0.x = ((r0.xxxx)*(source[20].yyyy)).x;
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
    // 17: mul r2.y, cb0[7].y, cb0[15].y
    r2.y = ((source[7].yyyy)*(source[15].yyyy)).y;
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
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t4.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r1.xyw, r0.xxxx, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r2.xyxz)).xyw;
    // 27: mul r0.x, r1.z, r2.w
    r0.x = ((r1.zzzz)*(r2.wwww)).x;
    // 28: dp3 r1.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 29: add r2.xyz, -r0.yzwy, r1.zzzz
    r2.xyz = ((-(r0.yzwy))+(r1.zzzz)).xyz;
    // 30: mad r2.xyz, cb0[18].xxxx, r2.xyzx, r0.yzwy
    r2.xyz = ((source[18].xxxx)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[18].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[18].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 74: mul_sat r8.w, r1.z, cb2[3].w
    r8.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 75: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 76: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 77: dp3 r1.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 78: add r4.xyz, -r3.xyzx, r1.zzzz
    r4.xyz = ((-(r3.xyzx))+(r1.zzzz)).xyz;
    // 79: mad r4.xyz, cb0[18].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[18].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 80: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 81: dp3 r1.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 82: add r3.xyz, -r4.xyzx, r1.zzzz
    r3.xyz = ((-(r4.xyzx))+(r1.zzzz)).xyz;
    // 83: mad r3.xyz, cb0[18].yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((source[18].yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 84: mad r4.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mad r9.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 86: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 87: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 88: mul r9.xyz, r2.xyzx, r3.xyzx
    r9.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 89: dp3 r1.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 90: mad r2.xyz, -r3.xyzx, r2.xyzx, r1.zzzz
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r1.zzzz)).xyz;
    // 91: mad r2.xyz, cb0[18].xxxx, r2.xyzx, r9.xyzx
    r2.xyz = ((source[18].xxxx)*(r2.xyzx)+(r9.xyzx)).xyz;
    // 92: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 93: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 94: mad r2.xyz, cb0[18].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[18].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 95: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 96: mad r1.xyz, r1.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 97: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 98: mul r0.x, r6.x, cb0[19].y
    r0.x = ((r6.xxxx)*(source[19].yyyy)).x;
    // 99: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 100: movc r0.x, r5.x, l(0), r0.x
    r0.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 101: add_sat r0.x, r0.x, cb0[19].z
    r0.x = (saturate((r0.xxxx)+(source[19].zzzz))).x;
    // 102: add r1.w, -r0.x, l(1.000000)
    r1.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r2.xyz, r1.wwww, cb0[14].xyzx
    r2.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 104: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 105: mad r1.xyz, -r2.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r2.xyzx))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 106: mad r1.xyz, r0.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 107: add r2.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 108: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 109: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 110: mad r2.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r2.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 111: mad r3.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 112: mad r2.xyz, r0.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 113: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 114: mad r2.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 115: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 116: max r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (max(r0.xxxx,r2.xyzx)).xyz;
    // 117: mov_sat r1.w, cb0[20].z
    r1.w = (saturate(source[20].zzzz)).w;
    // 118: mad r3.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r3.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 119: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 120: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 121: mad r3.xyz, r8.wwww, r3.xyzx, r2.wwww
    r3.xyz = ((r8.wwww)*(r3.xyzx)+(r2.wwww)).xyz;
    // 122: add r1.w, -cb0[21].z, cb0[21].y
    r1.w = ((-(source[21].zzzz))+(source[21].yyyy)).w;
    // 123: mad r1.w, r7.x, r1.w, cb0[21].z
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[21].zzzz)).w;
    // 124: add r2.w, -r1.w, cb0[22].x
    r2.w = ((-(r1.wwww))+(source[22].xxxx)).w;
    // 125: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 126: add r2.w, -r1.w, cb0[22].z
    r2.w = ((-(r1.wwww))+(source[22].zzzz)).w;
    // 127: mad r1.w, r7.z, r2.w, r1.w
    r1.w = ((r7.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 128: mul r1.w, r6.z, r1.w
    r1.w = ((r6.zzzz)*(r1.wwww)).w;
    // 129: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 130: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: movc r1.w, r5.z, l(0), r1.w
    r1.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 132: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 133: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 135: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 136: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 137: mul r5.xy, r5.xyxx, cb0[17].xxxx
    r5.xy = ((r5.xyxx)*(source[17].xxxx)).xy;
    // 138: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 140: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 141: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 142: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 143: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 144: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 145: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 146: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 147: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 148: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 149: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 150: mul r7.xyz, r1.wwww, v5.xyzx
    r7.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 151: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 152: deriv_rtx_coarse r8.x, r1.w
    r8.x = (ddx_coarse(r1.wwww)).x;
    // 153: deriv_rty_coarse r8.y, r1.w
    r8.y = (ddy_coarse(r1.wwww)).y;
    // 154: dp2 r2.w, r8.xyxx, r8.xyxx
    r2.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 155: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 156: mad r2.w, r2.w, l(0.300000), r8.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz)).w;
    // 157: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 158: min r8.y, r2.w, l(1.000000)
    r8.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 159: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: max r9.xyz, r3.xyzx, r2.wwww
    r9.xyz = (max(r3.xyzx,r2.wwww)).xyz;
    // 161: add r9.xyz, -r3.xyzx, r9.xyzx
    r9.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 162: mul_sat r2.w, r3.y, l(50.000000)
    r2.w = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 163: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 164: mul r10.xyz, r1.wwww, r6.xyzx
    r10.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 165: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 166: add r2.w, r10.z, l(1.000000)
    r2.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 170: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 171: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 172: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 173: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 174: mul r1.w, r1.w, cb0[22].w
    r1.w = ((r1.wwww)*(source[22].wwww)).w;
    // 175: add_sat r8.x, -r2.w, r3.w
    r8.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 176: sample_indexable(texture2d)(float,float,float,float) r11.xy, r8.xyxx, t6.xyzw, s7
    r11.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 177: add r2.w, r0.x, r8.x
    r2.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 178: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 179: mul r12.xyz, r3.xyzx, r11.yyyy
    r12.xyz = ((r3.xyzx)*(r11.yyyy)).xyz;
    // 180: mad r9.xyz, r9.xyzx, r11.xxxx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xxxx)+(r12.xyzx)).xyz;
    // 181: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r3.w = r11.y != 0.f ? 1.f / r11.y : 0.f;
    // 182: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 183: mad r11.xyz, r3.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r3.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: dp3 r3.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 185: mad r3.xyz, r3.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r3.xyz = ((r3.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 186: mad r12.xyz, -r9.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 188: mul r11.xyz, r1.xyzx, r12.xyzx
    r11.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 189: add r3.w, -r8.w, l(1.000000)
    r3.w = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: mul r11.xyz, r3.wwww, r11.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)).xyz;
    // 191: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 193: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 194: mul r13.xyz, r5.wwww, v1.xyzx
    r13.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 195: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 196: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 197: mul r14.xyz, r5.wwww, v0.xyzx
    r14.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 198: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 199: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 200: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 201: dp3 r16.y, r15.xyzx, r6.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 202: dp3 r15.y, r15.xyzx, r10.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 203: dp3 r16.x, r14.xyzx, r6.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 204: dp3 r15.x, r14.xyzx, r10.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 205: dp2 r14.z, r16.xyxx, cb0[24].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 206: mul r8.xz, cb0[24].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[24].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 207: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 208: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 209: dp2 r17.z, r15.xyxx, cb0[24].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 210: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 211: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 212: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 213: dp4 r13.x, cb0[25].xyzw, r14.xyzw
    r13.x = (dot((source[25].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 214: dp4 r13.y, cb0[26].xyzw, r14.xyzw
    r13.y = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 215: dp4 r13.z, cb0[27].xyzw, r14.xyzw
    r13.z = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 216: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 217: dp4 r18.x, cb0[28].xyzw, r15.xyzw
    r18.x = (dot((source[28].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 218: dp4 r18.y, cb0[29].xyzw, r15.xyzw
    r18.y = (dot((source[29].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 219: dp4 r18.z, cb0[30].xyzw, r15.xyzw
    r18.z = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 220: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 221: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 222: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 223: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 224: mad r13.xyz, cb0[31].xyzx, r5.wwww, r13.xyzx
    r13.xyz = ((source[31].xyzx)*(r5.wwww)+(r13.xyzx)).xyz;
    // 225: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 226: mul r13.xyz, r13.xyzx, cb0[23].xyzx
    r13.xyz = ((r13.xyzx)*(source[23].xyzx)).xyz;
    // 227: mul r13.xyz, r13.xyzx, cb0[24].zzzz
    r13.xyz = ((r13.xyzx)*(source[24].zzzz)).xyz;
    // 228: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[23].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[23].wwww)).xyz;
    // 229: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 230: add r13.xyz, -r5.wwww, r13.xyzx
    r13.xyz = ((-(r5.wwww))+(r13.xyzx)).xyz;
    // 231: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 232: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: mad r6.w, r8.y, l(2.000000), l(2.000000)
    r6.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 234: div r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)/(r6.wwww)).w;
    // 235: mad r5.w, r4.w, l(5.000000), r5.w
    r5.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 236: add_sat r5.w, r8.w, r5.w
    r5.w = (saturate((r8.wwww)+(r5.wwww))).w;
    // 237: mad r7.w, r5.w, l(-2.000000), l(3.000000)
    r7.w = ((r5.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 238: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 239: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 240: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 241: mul r5.w, r5.w, l(1.500000)
    r5.w = ((r5.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 242: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 243: mul r13.xyz, r5.wwww, r13.xyzx
    r13.xyz = ((r5.wwww)*(r13.xyzx)).xyz;
    // 244: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 245: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 246: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 247: mul r5.w, r8.y, l(5.000000)
    r5.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 248: mul r7.w, r8.y, r8.y
    r7.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 249: mul r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)*(r7.wwww)).w;
    // 250: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 251: add r2.w, r0.x, r2.w
    r2.w = ((r0.xxxx)+(r2.wwww)).w;
    // 252: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 253: add_sat r0.x, r2.w, l(-1.000000)
    r0.x = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 254: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t7.xyzw, s6, r5.w
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r17.xyzx).xyz, (r5.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 255: mul r8.xyz, r13.xyzx, r13.wwww
    r8.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 256: mul r8.xyz, r8.xyzx, cb0[23].xyzx
    r8.xyz = ((r8.xyzx)*(source[23].xyzx)).xyz;
    // 257: mul r8.xyz, r8.xyzx, cb0[24].zzzz
    r8.xyz = ((r8.xyzx)*(source[24].zzzz)).xyz;
    // 258: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[23].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[23].wwww)).xyz;
    // 259: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r8.xyz, -r2.wwww, r8.xyzx
    r8.xyz = ((-(r2.wwww))+(r8.xyzx)).xyz;
    // 261: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 262: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: div r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)/(r6.wwww)).w;
    // 264: mad r2.w, r4.w, l(5.000000), r2.w
    r2.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 265: add_sat r2.w, r8.w, r2.w
    r2.w = (saturate((r8.wwww)+(r2.wwww))).w;
    // 266: mad r4.w, r2.w, l(-2.000000), l(3.000000)
    r4.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 267: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 268: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 269: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 270: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 271: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 272: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 273: mul r13.xyz, r8.xyzx, r9.xyzx
    r13.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 274: mad r2.w, r0.x, r3.x, r3.y
    r2.w = ((r0.xxxx)*(r3.xxxx)+(r3.yyyy)).w;
    // 275: mad r2.w, r2.w, r0.x, r3.z
    r2.w = ((r2.wwww)*(r0.xxxx)+(r3.zzzz)).w;
    // 276: mul r2.w, r0.x, r2.w
    r2.w = ((r0.xxxx)*(r2.wwww)).w;
    // 277: max r0.x, r0.x, r2.w
    r0.x = (max(r0.xxxx,r2.wwww)).x;
    // 278: mad r3.xyz, r13.xyzx, r0.xxxx, r11.xyzx
    r3.xyz = ((r13.xyzx)*(r0.xxxx)+(r11.xyzx)).xyz;
    // 279: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 280: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 281: mul r11.xyz, r2.wwww, v6.xyzx
    r11.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 282: dp3 r2.w, r11.xyzx, r6.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 283: dp3 r4.w, -r11.xyzx, r6.xyzx
    r4.w = (dot((-(r11.xyzx)).xyz,(r6.xyzx).xyz).xxxx).w;
    // 284: dp3 r5.w, r11.xyzx, r10.xyzx
    r5.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 285: mad r6.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 286: mad r6.zw, r4.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r4.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 287: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 288: mad r10.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 289: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 290: mul r10.yzw, r10.yyyy, cb0[34].xxyz
    r10.yzw = ((r10.yyyy)*(source[34].xxyz)).yzw;
    // 291: mad r10.xyz, r10.xxxx, cb0[33].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[33].xyzx)+(r10.yzwy)).xyz;
    // 292: mul r10.xyz, r10.xyzx, cb0[35].wwww
    r10.xyz = ((r10.xyzx)*(source[35].wwww)).xyz;
    // 293: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 294: mul r2.xyz, r2.xyzx, r10.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)).xyz;
    // 295: mul r2.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 296: mul r2.xyz, r12.xyzx, r2.xyzx
    r2.xyz = ((r12.xyzx)*(r2.xyzx)).xyz;
    // 297: mad r2.xyz, -r2.xyzx, r8.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r8.wwww)+(r2.xyzx)).xyz;
    // 298: mad r2.xyz, r3.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r2.xyzx)).xyz;
    // 299: mul r3.xyz, r6.yyyy, cb0[34].xyzx
    r3.xyz = ((r6.yyyy)*(source[34].xyzx)).xyz;
    // 300: mad r3.xyz, cb0[33].xyzx, r6.xxxx, r3.xyzx
    r3.xyz = ((source[33].xyzx)*(r6.xxxx)+(r3.xyzx)).xyz;
    // 301: mul r3.xyz, r3.xyzx, cb0[35].wwww
    r3.xyz = ((r3.xyzx)*(source[35].wwww)).xyz;
    // 302: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 303: mul r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)*(r3.xyzx)).xyz;
    // 304: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 305: mad r2.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r2.xyzx)).xyz;
    // 306: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 307: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 308: dp3 r0.x, r5.xyzx, r7.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 309: mul_sat r2.w, r0.x, cb0[18].z
    r2.w = (saturate((r0.xxxx)*(source[18].zzzz))).w;
    // 310: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 311: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 312: mul_sat r3.x, r7.z, cb0[18].z
    r3.x = (saturate((r7.zzzz)*(source[18].zzzz))).x;
    // 313: add r3.y, -|r7.z|, l(1.000000)
    r3.y = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 314: mul r0.x, r0.x, r3.y
    r0.x = ((r0.xxxx)*(r3.yyyy)).x;
    // 315: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 316: add_sat r3.x, r3.x, -cb0[18].w
    r3.x = (saturate((r3.xxxx)+(-(source[18].wwww)))).x;
    // 317: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 318: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 319: mul r3.y, r3.y, cb0[19].x
    r3.y = ((r3.yyyy)*(source[19].xxxx)).y;
    // 320: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 321: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 322: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 323: mad r3.xyz, r2.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r3.xyz = ((r2.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 324: mul r2.w, r2.w, cb0[11].w
    r2.w = ((r2.wwww)*(source[11].wwww)).w;
    // 325: mad r3.xyz, cb0[12].wwww, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((source[12].wwww)*(r3.xyzx)+(source[12].xyzx)).xyz;
    // 326: mad r3.xyz, r2.wwww, cb0[11].xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(source[11].xyzx)+(r3.xyzx)).xyz;
    // 327: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 328: mul r7.xyz, cb0[8].xyzx, cb0[17].wwww
    r7.xyz = ((source[8].xyzx)*(source[17].wwww)).xyz;
    // 329: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 330: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 331: mad r5.xyz, -r5.xyzx, r7.xyzx, r2.wwww
    r5.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r2.wwww)).xyz;
    // 332: mad r5.xyz, cb0[18].xxxx, r5.xyzx, r8.xyzx
    r5.xyz = ((source[18].xxxx)*(r5.xyzx)+(r8.xyzx)).xyz;
    // 333: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 334: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 335: mad r5.xyz, cb0[18].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[18].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 336: mad r3.xyz, r5.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 337: log r2.w, |r0.x|
    r2.w = (log2(abs(r0.xxxx))).w;
    // 338: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 339: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 340: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 341: mul r4.xyz, r2.wwww, cb0[13].xyzx
    r4.xyz = ((r2.wwww)*(source[13].xyzx)).xyz;
    // 342: movc r4.xyz, r0.xxxx, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 343: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 344: mad r0.xyz, cb0[17].zzzz, r0.yzwy, r3.xyzx
    r0.xyz = ((source[17].zzzz)*(r0.yzwy)+(r3.xyzx)).xyz;
    // 345: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 346: mul r3.xyz, r6.wwww, cb0[34].xyzx
    r3.xyz = ((r6.wwww)*(source[34].xyzx)).xyz;
    // 347: mad r3.xyz, r6.zzzz, cb0[33].xyzx, r3.xyzx
    r3.xyz = ((r6.zzzz)*(source[33].xyzx)+(r3.xyzx)).xyz;
    // 348: mul r3.xyz, r3.xyzx, cb0[35].wwww
    r3.xyz = ((r3.xyzx)*(source[35].wwww)).xyz;
    // 349: mul_sat r4.xyz, cb0[16].xyzx, cb0[16].wwww
    r4.xyz = (saturate((source[16].xyzx)*(source[16].wwww))).xyz;
    // 350: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 351: mul r4.xyz, r4.xyzx, cb0[22].wwww
    r4.xyz = ((r4.xyzx)*(source[22].wwww)).xyz;
    // 352: dp3_sat o5.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 353: mul r4.xyz, r3.wwww, r5.xyzx
    r4.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 354: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 355: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 356: mad r0.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 357: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 358: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 359: mad o0.xyz, r1.xyzx, cb0[35].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[35].xyzx)+(r0.xyzx)).xyz;
    // 360: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 361: dp3 r0.x, r16.xyzx, r16.xyzx
    r0.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 362: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 363: mul r0.xyz, r0.xxxx, r16.xyzx
    r0.xyz = ((r0.xxxx)*(r16.xyzx)).xyz;
    // 364: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 365: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 366: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 367: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 368: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 369: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 370: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 371: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 372: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 373: ftou r0.x, cb0[32].z
    r0.x = (asfloat((uint4)(source[32].zzzz))).x;
    // 374: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 375: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 376: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 377: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 378: ret
    return output;
}

// source.character.equipment-native-903.v1 / source program 61b39083b2e22047b9e0764234841a3f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase903(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18].x=(g_SourceCharacterTime.xxxx).x;
    source[0].y=1.f; // Engine primitive opacity, not a MIC uniform.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[24]=g_SourceCharacterEnvironmentColor; source[25]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0;
    // 1: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 2: mul r0.x, r0.x, l(0.125000)
    r0.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 3: mul r1.y, cb0[6].y, cb0[16].y
    r1.y = ((source[6].yyyy)*(source[16].yyyy)).y;
    // 4: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 5: mov r1.xw, l(0,0,0,0)
    r1.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 6: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 7: frc r0.z, cb0[6].x
    r0.z = (frac(source[6].xxxx)).z;
    // 8: add r0.w, -r0.z, cb0[6].x
    r0.w = ((-(r0.zzzz))+(source[6].xxxx)).w;
    // 9: mul r1.z, r0.w, l(0.125000)
    r1.z = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 10: add r0.xy, r0.xyxx, r1.zwzz
    r0.xy = ((r0.xyxx)+(r1.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t4.xyzw, s6, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 12: mul r0.x, r0.z, r1.w
    r0.x = ((r0.zzzz)*(r1.wwww)).x;
    // 13: add r0.y, -cb0[6].w, l(1.000000)
    r0.y = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: mul r0.y, r0.y, cb0[18].x
    r0.y = ((r0.yyyy)*(source[18].xxxx)).y;
    // 15: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 16: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 17: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 18: mul r0.z, cb0[6].z, l(1.500000)
    r0.z = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 19: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 20: mad r0.y, r0.y, l(0.500000), cb0[6].z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).y;
    // 21: mul r0.yzw, r1.xxyz, r0.yyyy
    r0.yzw = ((r1.xxyz)*(r0.yyyy)).yzw;
    // 22: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 23: max r2.xyz, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = (max(r1.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 24: max r1.xyz, r1.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 25: min r1.xyz, r1.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 26: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 27: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 29: log r4.xyz, |r3.xzyx|
    r4.xyz = (log2(abs(r3.xzyx))).xyz;
    // 30: lt r3.xyz, |r3.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (asfloat((uint4)((abs(r3.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 31: mul r1.w, r4.y, cb0[17].y
    r1.w = ((r4.yyyy)*(source[17].yyyy)).w;
    // 32: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 33: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: movc r1.w, r3.y, l(0), r1.w
    r1.w = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 35: mad r1.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 36: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 37: max r5.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r5.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 38: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 39: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 40: min r5.xyz, r5.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 41: add r5.xyz, -r2.xyzx, r5.xyzx
    r5.xyz = ((-(r2.xyzx))+(r5.xyzx)).xyz;
    // 42: mad r2.xyz, r1.wwww, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 43: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: mad r1.xyz, r5.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r5.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 46: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 47: max r6.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 48: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 49: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 50: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 51: add r6.xyz, -r2.xyzx, r6.xyzx
    r6.xyz = ((-(r2.xyzx))+(r6.xyzx)).xyz;
    // 52: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 53: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 54: mad r1.xyz, r5.yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 55: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 56: max r6.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 57: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 58: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 59: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: add r6.xyz, -r2.xyzx, r6.xyzx
    r6.xyz = ((-(r2.xyzx))+(r6.xyzx)).xyz;
    // 61: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 62: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 63: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 64: mad r1.xyz, r5.zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 65: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 66: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 67: mad r2.xyz, cb0[19].zzzz, r2.xyzx, r1.xyzx
    r2.xyz = ((source[19].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 68: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 69: add r7.xyz, -r2.xyzx, r1.wwww
    r7.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 70: mad r2.xyz, cb0[19].wwww, r7.xyzx, r2.xyzx
    r2.xyz = ((source[19].wwww)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 71: mad r7.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: mad r8.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 73: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 74: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 76: dp3 r1.w, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r9.xyz, -r8.yzwy, r1.wwww
    r9.xyz = ((-(r8.yzwy))+(r1.wwww)).xyz;
    // 78: mad r9.xyz, cb0[19].zzzz, r9.xyzx, r8.yzwy
    r9.xyz = ((source[19].zzzz)*(r9.xyzx)+(r8.yzwy)).xyz;
    // 79: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: add r10.xyz, -r9.xyzx, r1.wwww
    r10.xyz = ((-(r9.xyzx))+(r1.wwww)).xyz;
    // 81: mad r9.xyz, cb0[19].wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((source[19].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 82: mul r10.xyz, r2.xyzx, r9.xyzx
    r10.xyz = ((r2.xyzx)*(r9.xyzx)).xyz;
    // 83: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 84: mad r2.xyz, -r2.xyzx, r9.xyzx, r1.wwww
    r2.xyz = ((-(r2.xyzx))*(r9.xyzx)+(r1.wwww)).xyz;
    // 85: mad r2.xyz, cb0[19].zzzz, r2.xyzx, r10.xyzx
    r2.xyz = ((source[19].zzzz)*(r2.xyzx)+(r10.xyzx)).xyz;
    // 86: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 87: add r9.xyz, -r2.xyzx, r1.wwww
    r9.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 88: mad r2.xyz, cb0[19].wwww, r9.xyzx, r2.xyzx
    r2.xyz = ((source[19].wwww)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 89: mul r2.xyz, r7.xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 90: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r2.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r2.xxyz))).yzw;
    // 91: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 92: mul r0.w, r4.x, cb0[20].x
    r0.w = ((r4.xxxx)*(source[20].xxxx)).w;
    // 93: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 94: movc r0.w, r3.x, l(0), r0.w
    r0.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 95: add_sat r0.w, r0.w, cb0[20].y
    r0.w = (saturate((r0.wwww)+(source[20].yyyy))).w;
    // 96: add r1.w, r0.w, l(-1.000000)
    r1.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 97: mad r1.w, cb0[20].w, r1.w, l(1.000000)
    r1.w = ((source[20].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r2.xyz, r0.xyzx, r1.wwww
    r2.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 99: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mul r3.xyw, r2.wwww, cb0[15].xyxz
    r3.xyw = ((r2.wwww)*(source[15].xyxz)).xyw;
    // 101: mul r2.xyz, r2.xyzx, r3.xywx
    r2.xyz = ((r2.xyzx)*(r3.xywx)).xyz;
    // 102: mad r0.xyz, r1.wwww, r0.xyzx, -r2.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 103: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 104: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 105: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 106: mad_sat r2.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 107: mad r0.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r0.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 108: mad r3.xyw, r2.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r3.xyw = ((r2.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 109: mad r0.xyz, r0.wwww, r0.xyzx, r3.xywx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r3.xywx)).xyz;
    // 110: mad r3.xyw, r2.xyxz, l(2.755200, 2.755200, 0.000000, 2.755200), l(0.690300, 0.690300, 0.000000, 0.690300)
    r3.xyw = ((r2.xyxz)*(float4(2.755200,2.755200,0.000000,2.755200))+(float4(0.690300,0.690300,0.000000,0.690300))).xyw;
    // 111: mad r0.xyz, r0.xyzx, r0.wwww, r3.xywx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r3.xywx)).xyz;
    // 112: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 113: max r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (max(r0.xyzx,r0.wwww)).xyz;
    // 114: mov_sat r2.w, cb0[21].x
    r2.w = (saturate(source[21].xxxx)).w;
    // 115: mad r3.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r3.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 116: mul r1.w, r2.w, l(0.080000)
    r1.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 117: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 118: mad r3.xyw, r6.wwww, r3.xyxw, r1.wwww
    r3.xyw = ((r6.wwww)*(r3.xyxw)+(r1.wwww)).xyw;
    // 119: add r1.w, -cb0[22].y, cb0[22].x
    r1.w = ((-(source[22].yyyy))+(source[22].xxxx)).w;
    // 120: mad r1.w, r5.x, r1.w, cb0[22].y
    r1.w = ((r5.xxxx)*(r1.wwww)+(source[22].yyyy)).w;
    // 121: add r2.w, -r1.w, cb0[22].w
    r2.w = ((-(r1.wwww))+(source[22].wwww)).w;
    // 122: mad r1.w, r5.y, r2.w, r1.w
    r1.w = ((r5.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 123: add r2.w, -r1.w, cb0[23].y
    r2.w = ((-(r1.wwww))+(source[23].yyyy)).w;
    // 124: mad r1.w, r5.z, r2.w, r1.w
    r1.w = ((r5.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 125: mul r1.w, r4.z, r1.w
    r1.w = ((r4.zzzz)*(r1.wwww)).w;
    // 126: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 127: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: movc r1.w, r3.z, l(0), r1.w
    r1.w = ((asuint(r3.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 129: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 130: min r6.z, r1.w, l(1.000000)
    r6.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 132: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 133: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 134: mul r4.xy, r4.xyxx, cb0[17].xxxx
    r4.xy = ((r4.xyxx)*(source[17].xxxx)).xy;
    // 135: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 137: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 138: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 139: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 140: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 141: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 142: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 143: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 144: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 145: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 146: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 147: mul r9.xyz, r1.wwww, v6.xyzx
    r9.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 148: dp3 r1.w, r5.xyzx, r9.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 149: deriv_rtx_coarse r6.x, r1.w
    r6.x = (ddx_coarse(r1.wwww)).x;
    // 150: deriv_rty_coarse r6.y, r1.w
    r6.y = (ddy_coarse(r1.wwww)).y;
    // 151: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 152: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 153: mad r2.w, r2.w, l(0.300000), r6.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz)).w;
    // 154: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 155: min r6.y, r2.w, l(1.000000)
    r6.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 156: add r2.w, -r6.y, l(1.000000)
    r2.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r10.xyz, r3.xywx, r2.wwww
    r10.xyz = (max(r3.xywx,r2.wwww)).xyz;
    // 158: add r10.xyz, -r3.xywx, r10.xyzx
    r10.xyz = ((-(r3.xywx))+(r10.xyzx)).xyz;
    // 159: mul_sat r2.w, r3.y, l(50.000000)
    r2.w = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 160: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 161: mul r11.xyz, r1.wwww, r5.xyzx
    r11.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 162: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 164: add r2.w, r11.z, l(1.000000)
    r2.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: add_sat r6.x, r1.w, -r2.w
    r6.x = (saturate((r1.wwww)+(-(r2.wwww)))).x;
    // 167: sample_indexable(texture2d)(float,float,float,float) r12.xy, r6.xyxx, t7.xyzw, s8
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 168: add r1.w, r0.w, r6.x
    r1.w = ((r0.wwww)+(r6.xxxx)).w;
    // 169: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 170: mul r13.xyz, r3.xywx, r12.yyyy
    r13.xyz = ((r3.xywx)*(r12.yyyy)).xyz;
    // 171: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 172: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r2.w = r12.y != 0.f ? 1.f / r12.y : 0.f;
    // 173: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 174: mad r12.xyz, r3.xywx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r3.xywx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 175: dp3 r2.w, r3.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: mad r3.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r3.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 177: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 179: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 180: add r2.w, -r6.w, l(1.000000)
    r2.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: mul r12.xyz, r2.wwww, r12.xyzx
    r12.xyz = ((r2.wwww)*(r12.xyzx)).xyz;
    // 182: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 183: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 184: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 185: mul r14.xyz, r3.wwww, v1.xyzx
    r14.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 186: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 187: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 188: mul r15.xyz, r3.wwww, v0.xyzx
    r15.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 189: mul r16.xyz, r14.zxyz, r15.yzxy
    r16.xyz = ((r14.zxyz)*(r15.yzxy)).xyz;
    // 190: mad r16.xyz, r14.yzxy, r15.zxyz, -r16.xyzx
    r16.xyz = ((r14.yzxy)*(r15.zxyz)+(-(r16.xyzx))).xyz;
    // 191: mul r16.xyz, r16.xyzx, v1.wwww
    r16.xyz = ((r16.xyzx)*(v1.wwww)).xyz;
    // 192: dp3 r17.y, r16.xyzx, r5.xyzx
    r17.y = (dot((r16.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 193: dp3 r16.y, r16.xyzx, r11.xyzx
    r16.y = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 194: dp3 r17.x, r15.xyzx, r5.xyzx
    r17.x = (dot((r15.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 195: dp3 r16.x, r15.xyzx, r11.xyzx
    r16.x = (dot((r15.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 196: dp2 r15.z, r17.xyxx, cb0[25].xyxx
    r15.z = (dot((r17.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 197: mul r6.xz, cb0[25].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r6.xz = ((source[25].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 198: dp2 r15.x, r17.xyxx, r6.xzxx
    r15.x = (dot((r17.xyxx).xy,(r6.xzxx).xy).xxxx).x;
    // 199: dp2 r18.x, r16.xyxx, r6.xzxx
    r18.x = (dot((r16.xyxx).xy,(r6.xzxx).xy).xxxx).x;
    // 200: dp2 r18.z, r16.xyxx, cb0[25].xyxx
    r18.z = (dot((r16.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 201: dp3 r15.y, r14.xyzx, r5.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 202: dp3 r18.y, r14.xyzx, r11.xyzx
    r18.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 203: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 204: dp4 r14.x, cb0[26].xyzw, r15.xyzw
    r14.x = (dot((source[26].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 205: dp4 r14.y, cb0[27].xyzw, r15.xyzw
    r14.y = (dot((source[27].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 206: dp4 r14.z, cb0[28].xyzw, r15.xyzw
    r14.z = (dot((source[28].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 207: mul r16.xyzw, r15.yzzx, r15.xyzz
    r16.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 208: dp4 r19.x, cb0[29].xyzw, r16.xyzw
    r19.x = (dot((source[29].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 209: dp4 r19.y, cb0[30].xyzw, r16.xyzw
    r19.y = (dot((source[30].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 210: dp4 r19.z, cb0[31].xyzw, r16.xyzw
    r19.z = (dot((source[31].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 211: add r14.xyz, r14.xyzx, r19.xyzx
    r14.xyz = ((r14.xyzx)+(r19.xyzx)).xyz;
    // 212: mul r3.w, r15.y, r15.y
    r3.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 213: mov r17.z, r15.y
    r17.z = (r15.yyyy).z;
    // 214: mad r3.w, r15.x, r15.x, -r3.w
    r3.w = ((r15.xxxx)*(r15.xxxx)+(-(r3.wwww))).w;
    // 215: mad r14.xyz, cb0[32].xyzx, r3.wwww, r14.xyzx
    r14.xyz = ((source[32].xyzx)*(r3.wwww)+(r14.xyzx)).xyz;
    // 216: max r14.xyz, r14.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r14.xyz = (max(r14.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 217: mul r14.xyz, r14.xyzx, cb0[24].xyzx
    r14.xyz = ((r14.xyzx)*(source[24].xyzx)).xyz;
    // 218: mul r14.xyz, r14.xyzx, cb0[25].zzzz
    r14.xyz = ((r14.xyzx)*(source[25].zzzz)).xyz;
    // 219: mad r14.xyz, r14.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[24].wwww
    r14.xyz = ((r14.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[24].wwww)).xyz;
    // 220: dp3 r3.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 221: add r14.xyz, -r3.wwww, r14.xyzx
    r14.xyz = ((-(r3.wwww))+(r14.xyzx)).xyz;
    // 222: mad r14.xyz, r14.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r14.xyz = ((r14.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 223: dp3 r3.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 224: mad r4.w, r6.y, l(2.000000), l(2.000000)
    r4.w = ((r6.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 225: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 226: mad r3.w, r2.w, l(5.000000), r3.w
    r3.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 227: add_sat r3.w, r6.w, r3.w
    r3.w = (saturate((r6.wwww)+(r3.wwww))).w;
    // 228: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 229: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 230: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 231: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 232: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 233: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 234: mul r14.xyz, r3.wwww, r14.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)).xyz;
    // 235: mul r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)*(r14.xyzx)).xyz;
    // 236: mul r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)*(r14.xyzx)).xyz;
    // 237: mul r12.xyz, r0.xyzx, r12.xyzx
    r12.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 238: mul r3.w, r6.y, l(5.000000)
    r3.w = ((r6.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 239: mul r5.w, r6.y, r6.y
    r5.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 240: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 241: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 242: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 243: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 244: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 245: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r18.xyzx, t8.xyzw, s7, r3.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r18.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 246: mul r6.xyz, r14.xyzx, r14.wwww
    r6.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 247: mul r6.xyz, r6.xyzx, cb0[24].xyzx
    r6.xyz = ((r6.xyzx)*(source[24].xyzx)).xyz;
    // 248: mul r6.xyz, r6.xyzx, cb0[25].zzzz
    r6.xyz = ((r6.xyzx)*(source[25].zzzz)).xyz;
    // 249: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[24].wwww)).xyz;
    // 250: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 251: add r6.xyz, -r1.wwww, r6.xyzx
    r6.xyz = ((-(r1.wwww))+(r6.xyzx)).xyz;
    // 252: mad r6.xyz, r6.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r6.xyz = ((r6.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 253: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 254: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 255: mad r1.w, r2.w, l(5.000000), r1.w
    r1.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 256: add_sat r1.w, r6.w, r1.w
    r1.w = (saturate((r6.wwww)+(r1.wwww))).w;
    // 257: mad r2.w, r1.w, l(-2.000000), l(3.000000)
    r2.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 258: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 259: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 260: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 261: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 262: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 263: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 264: mul r14.xyz, r6.xyzx, r10.xyzx
    r14.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 265: mad r1.w, r0.w, r3.x, r3.y
    r1.w = ((r0.wwww)*(r3.xxxx)+(r3.yyyy)).w;
    // 266: mad r1.w, r1.w, r0.w, r3.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r3.zzzz)).w;
    // 267: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 268: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 269: mad r3.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r3.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 270: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 271: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 272: mul r12.xyz, r1.wwww, v7.xyzx
    r12.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 273: dp3 r1.w, r12.xyzx, r5.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 274: dp3 r2.w, r12.xyzx, r11.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 275: mad r5.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 276: mad r5.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r5.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 277: mul r5.xyzw, r5.xyzw, r5.xyzw
    r5.xyzw = ((r5.xyzw)*(r5.xyzw)).xyzw;
    // 278: mul r11.xyz, r5.wwww, cb0[35].xyzx
    r11.xyz = ((r5.wwww)*(source[35].xyzx)).xyz;
    // 279: mad r11.xyz, r5.zzzz, cb0[34].xyzx, r11.xyzx
    r11.xyz = ((r5.zzzz)*(source[34].xyzx)+(r11.xyzx)).xyz;
    // 280: mul r11.xyz, r11.xyzx, cb0[36].wwww
    r11.xyz = ((r11.xyzx)*(source[36].wwww)).xyz;
    // 281: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 282: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 283: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 284: mul r0.xyz, r13.xyzx, r0.xyzx
    r0.xyz = ((r13.xyzx)*(r0.xyzx)).xyz;
    // 285: mad r0.xyz, -r0.xyzx, r6.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r6.wwww)+(r0.xyzx)).xyz;
    // 286: mad r0.xyz, r3.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 287: mul r3.xyz, r5.yyyy, cb0[35].xyzx
    r3.xyz = ((r5.yyyy)*(source[35].xyzx)).xyz;
    // 288: mad r3.xyz, cb0[34].xyzx, r5.xxxx, r3.xyzx
    r3.xyz = ((source[34].xyzx)*(r5.xxxx)+(r3.xyzx)).xyz;
    // 289: mul r3.xyz, r3.xyzx, cb0[36].wwww
    r3.xyz = ((r3.xyzx)*(source[36].wwww)).xyz;
    // 290: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 291: mul r3.xyz, r6.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 292: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 293: mad r0.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 294: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 295: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 296: dp3 r0.w, r4.xyzx, r9.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 297: mul_sat r1.w, r0.w, cb0[18].w
    r1.w = (saturate((r0.wwww)*(source[18].wwww))).w;
    // 298: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 299: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 300: mul_sat r2.w, r9.z, cb0[18].w
    r2.w = (saturate((r9.zzzz)*(source[18].wwww))).w;
    // 301: add r3.x, -|r9.z|, l(1.000000)
    r3.x = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 302: mul r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)*(r3.xxxx)).w;
    // 303: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 304: add_sat r2.w, r2.w, -cb0[19].x
    r2.w = (saturate((r2.wwww)+(-(source[19].xxxx)))).w;
    // 305: log r3.x, r2.w
    r3.x = (log2(r2.wwww)).x;
    // 306: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 307: mul r3.x, r3.x, cb0[19].y
    r3.x = ((r3.xxxx)*(source[19].yyyy)).x;
    // 308: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 309: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 310: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 311: mad r3.xyz, r1.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r3.xyz = ((r1.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 312: mad r3.xyz, cb0[13].wwww, r3.xyzx, cb0[13].xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(source[13].xyzx)).xyz;
    // 313: mul r2.w, r1.w, cb0[12].w
    r2.w = ((r1.wwww)*(source[12].wwww)).w;
    // 314: mad r3.xyz, r2.wwww, cb0[12].xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 315: mul r4.xy, v4.xyxx, cb0[8].zzzz
    r4.xy = ((v4.xyxx)*(source[8].zzzz)).xy;
    // 316: mul r4.zw, cb0[8].xxxy, cb0[18].xxxx
    r4.zw = ((source[8].xxxy)*(source[18].xxxx)).zw;
    // 317: mad r4.xy, r4.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r4.xyxx
    r4.xy = ((r4.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r4.xyxx)).xy;
    // 318: mad r4.zw, cb0[8].zzzz, v4.xxxy, r4.zzzw
    r4.zw = ((source[8].zzzz)*(v4.xxxy)+(r4.zzzw)).zw;
    // 319: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r4.xyxx, t6.yzwx, s5, l(0.000000)
    r2.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 320: mad r4.xy, r2.wwww, cb0[18].yyyy, r4.zwzz
    r4.xy = ((r2.wwww)*(source[18].yyyy)+(r4.zwzz)).xy;
    // 321: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 322: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 323: mul r4.xyz, r4.xyzx, r5.wwww
    r4.xyz = ((r4.xyzx)*(r5.wwww)).xyz;
    // 324: mul r6.xyz, cb0[9].xyzx, cb0[18].zzzz
    r6.xyz = ((source[9].xyzx)*(source[18].zzzz)).xyz;
    // 325: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 326: mad r6.xyz, r1.wwww, r4.xyzx, -r4.xyzx
    r6.xyz = ((r1.wwww)*(r4.xyzx)+(-(r4.xyzx))).xyz;
    // 327: mad r4.xyz, cb0[9].wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((source[9].wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 328: mul r6.xyz, cb0[7].xyzx, cb0[17].wwww
    r6.xyz = ((source[7].xyzx)*(source[17].wwww)).xyz;
    // 329: mad r4.xyz, r5.xyzx, r6.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 330: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 331: add r5.xyz, -r4.xyzx, r1.wwww
    r5.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 332: mad r4.xyz, cb0[19].zzzz, r5.xyzx, r4.xyzx
    r4.xyz = ((source[19].zzzz)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 333: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 334: add r5.xyz, -r4.xyzx, r1.wwww
    r5.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 335: mad r4.xyz, cb0[19].wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((source[19].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 336: mad r3.xyz, r4.xyzx, r7.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 337: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 338: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 339: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 340: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 341: mul r4.xyz, r1.wwww, cb0[14].xyzx
    r4.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 342: movc r4.xyz, r0.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 343: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 344: mul r1.xyz, r1.xyzx, r8.yzwy
    r1.xyz = ((r1.xyzx)*(r8.yzwy)).xyz;
    // 345: mov_sat r8.x, r8.x
    r8.x = (saturate(r8.xxxx)).x;
    // 346: mul_sat r0.w, r8.x, cb0[21].y
    r0.w = (saturate((r8.xxxx)*(source[21].yyyy))).w;
    // 347: mul o0.w, r0.w, cb0[0].y
    output.targets[0].w = ((r0.wwww)*(source[0].yyyy)).w;
    // 348: mad r1.xyz, cb0[17].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[17].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 349: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 350: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 351: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 352: mad r0.xyz, r2.xyzx, cb0[36].xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(source[36].xyzx)+(r1.xyzx)).xyz;
    // 353: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 354: dp3 r0.x, r17.xyzx, r17.xyzx
    r0.x = (dot((r17.xyzx).xyz,(r17.xyzx).xyz).xxxx).x;
    // 355: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 356: mul r0.xyz, r0.xxxx, r17.xyzx
    r0.xyz = ((r0.xxxx)*(r17.xyzx)).xyz;
    // 357: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 358: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 359: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 360: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 361: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 362: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 363: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 364: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 365: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 366: ftou r0.x, cb0[33].z
    r0.x = (asfloat((uint4)(source[33].zzzz))).x;
    // 367: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 368: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 369: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 370: mov o5.xz, l(0,0,0.450000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),0.450000,asfloat(0u))).xz;
    // 371: ret
    return output;
}

// source.character.equipment-native-904.v1 / source program e407a0ba738dc149957914cc631abfdc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase904(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v6.z
    r0.x = ((r0.xxxx)*(v6.zzzz)).x;
    // 4: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: mul r0.yzw, r0.yyyy, cb0[2].xxyz
    r0.yzw = ((r0.yyyy)*(source[2].xxyz)).yzw;
    // 7: mad r0.xyz, r0.xxxx, cb0[1].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[1].xyzx)+(r0.yzwy)).xyz;
    // 8: mul r0.xyz, r0.xyzx, cb0[3].wwww
    r0.xyz = ((r0.xyzx)*(source[3].wwww)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 11: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[0].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[0].xyzx)).xyz;
    // 12: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 13: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 14: mad o0.xyz, r1.xyzx, cb0[3].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[3].xyzx)+(r2.xyzx)).xyz;
    // 15: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 16: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 17: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 18: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 19: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 20: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 23: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 24: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 25: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 26: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 27: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 28: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 29: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 30: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 31: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 32: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 33: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 34: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 35: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 36: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 37: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 38: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 39: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 40: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 41: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 42: ret
    return output;
}

// source.character.static-map-native-1100.v1 / source program 247fd3b3462a9548b081b5174f7a2f8a
