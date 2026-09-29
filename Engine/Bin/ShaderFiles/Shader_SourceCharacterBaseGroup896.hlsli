#ifndef SOURCE_CHARACTER_BASE_DISPATCH_CASES
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

// source.character.mokoko-av036-905.v1 / source program 7d17844b3bb9a546828dcab0427fff0a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase905(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].y=(g_SourceCharacterTime.xxxx).x;
    source[22].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
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
    // 24: lt r1.z, |r1.y|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 25: log r1.y, |r1.y|
    r1.y = (log2(abs(r1.yyyy))).y;
    // 26: add r1.w, -cb0[18].y, cb0[18].x
    r1.w = ((-(source[18].yyyy))+(source[18].xxxx)).w;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 28: mad r1.w, r3.w, r1.w, cb0[18].y
    r1.w = ((r3.wwww)*(r1.wwww)+(source[18].yyyy)).w;
    // 29: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 30: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 31: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 33: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 34: add r1.w, -r1.z, cb0[19].y
    r1.w = ((-(r1.zzzz))+(source[19].yyyy)).w;
    // 35: mad r1.z, r3.w, r1.w, r1.z
    r1.z = ((r3.wwww)*(r1.wwww)+(r1.zzzz)).z;
    // 36: mul r1.z, r1.z, cb0[19].z
    r1.z = ((r1.zzzz)*(source[19].zzzz)).z;
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
    // 48: mul r5.xy, r4.xyxx, cb0[17].xxxx
    r5.xy = ((r4.xyxx)*(source[17].xxxx)).xy;
    // 49: mad r4.xy, cb0[17].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[17].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 50: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 51: mad r1.xzw, r3.wwww, r4.xxyz, r5.xxyz
    r1.xzw = ((r3.wwww)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 52: add r4.xyz, -r1.xzwx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xzwx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[19].xxxx, r4.xyzx, r1.xzwx
    r4.xyz = ((source[19].xxxx)*(r4.xyzx)+(r1.xzwx)).xyz;
    // 54: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 55: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 56: div r4.xyz, r4.xyzx, r2.wwww
    r4.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 57: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 58: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 59: mul r5.xyz, r2.wwww, v1.xyzx
    r5.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 60: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 61: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 62: mul r6.xyz, r2.wwww, v0.xyzx
    r6.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 63: mul r7.xyz, r5.zxyz, r6.yzxy
    r7.xyz = ((r5.zxyz)*(r6.yzxy)).xyz;
    // 64: mad r7.xyz, r5.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r5.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 65: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 66: dp3 r8.y, r7.xyzx, r4.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 67: dp3 r8.x, r6.xyzx, r4.xyzx
    r8.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 68: dp3 r8.z, r5.xyzx, r4.xyzx
    r8.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 69: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 70: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 71: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 72: mad r9.xyz, v5.xyzx, r2.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r2.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r10.y, r7.xyzx, r4.xyzx
    r10.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 74: dp3 r10.x, r6.xyzx, r4.xyzx
    r10.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 75: dp3 r10.z, r5.xyzx, r4.xyzx
    r10.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 76: dp3 r2.w, r8.xyzx, r10.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 77: mul r8.xyz, r8.xyzx, r2.wwww
    r8.xyz = ((r8.xyzx)*(r2.wwww)).xyz;
    // 78: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 79: mov r8.w, -r8.x
    r8.w = (-(r8.xxxx)).w;
    // 80: dp2 r2.w, r8.ywyy, r8.ywyy
    r2.w = (dot((r8.ywyy).xy,(r8.ywyy).xy).xxxx).w;
    // 81: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 82: div r8.xy, r8.ywyy, r2.wwww
    r8.xy = ((r8.ywyy)/(r2.wwww)).xy;
    // 83: mad r2.w, -r8.z, l(0.250000), l(0.250000)
    r2.w = ((-(r8.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 84: add r4.w, r8.z, l(1.000000)
    r4.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 86: mad r8.xy, r2.wwww, r8.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r2.wwww)*(r8.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r8.xyz, r8.xyxx, t4.xyzw, s4, r0.x
    r8.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r8.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 88: log r10.xyz, r8.xyzx
    r10.xyz = (log2(r8.xyzx)).xyz;
    // 89: rcp r0.x, cb0[19].w
    r0.x = (1.0/(source[19].wwww)).x;
    // 90: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 91: mul r10.xyz, r10.xyzx, cb0[19].wwww
    r10.xyz = ((r10.xyzx)*(source[19].wwww)).xyz;
    // 92: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 93: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 94: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 95: mad r10.xyz, r10.xyzx, cb0[19].wwww, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[19].wwww)+(r11.xyzx)).xyz;
    // 96: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 97: mul r8.xyz, r8.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 98: add r0.x, cb0[19].w, l(1.000000)
    r0.x = ((source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 100: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r8.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r8.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 102: mad r8.xyz, r4.wwww, r8.xyzx, cb0[8].xyzx
    r8.xyz = ((r4.wwww)*(r8.xyzx)+(source[8].xyzx)).xyz;
    // 103: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 104: mul r8.xyz, r8.xyzx, cb0[20].xxxx
    r8.xyz = ((r8.xyzx)*(source[20].xxxx)).xyz;
    // 105: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 106: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 107: mad r2.yzw, cb0[18].zzzz, r10.xxyz, r2.xxyz
    r2.yzw = ((source[18].zzzz)*(r10.xxyz)+(r2.xxyz)).yzw;
    // 108: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 110: mad r2.yzw, cb0[18].wwww, r10.xxyz, r2.yyzw
    r2.yzw = ((source[18].wwww)*(r10.xxyz)+(r2.yyzw)).yzw;
    // 111: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 113: mul r10.xyz, r10.xyzx, cb0[20].yyyy
    r10.xyz = ((r10.xyzx)*(source[20].yyyy)).xyz;
    // 114: add r0.x, r3.y, r3.x
    r0.x = ((r3.yyyy)+(r3.xxxx)).x;
    // 115: add r0.x, r3.z, r0.x
    r0.x = ((r3.zzzz)+(r0.xxxx)).x;
    // 116: add_sat r0.x, r3.w, r0.x
    r0.x = (saturate((r3.wwww)+(r0.xxxx))).x;
    // 117: mad r2.yzw, r0.xxxx, r10.xxyz, r2.yyzw
    r2.yzw = ((r0.xxxx)*(r10.xxyz)+(r2.yyzw)).yzw;
    // 118: add r10.xyz, -r2.yzwy, r2.xxxx
    r10.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 119: mad r2.xyz, r3.wwww, r10.xyzx, r2.yzwy
    r2.xyz = ((r3.wwww)*(r10.xyzx)+(r2.yzwy)).xyz;
    // 120: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 121: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 122: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 123: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 124: dp3 r0.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 125: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 126: add r2.w, -cb0[21].z, cb0[21].y
    r2.w = ((-(source[21].zzzz))+(source[21].yyyy)).w;
    // 127: mad r2.w, r3.w, r2.w, cb0[21].z
    r2.w = ((r3.wwww)*(r2.wwww)+(source[21].zzzz)).w;
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
    // 133: div r2.w, cb0[21].w, r2.w
    r2.w = ((source[21].wwww)/(r2.wwww)).w;
    // 134: dp3 r3.x, r1.xzwx, r1.xzwx
    r3.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 135: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 136: div r1.xzw, r1.xxzw, r3.xxxx
    r1.xzw = ((r1.xxzw)/(r3.xxxx)).xzw;
    // 137: dp3 r3.x, r1.xzwx, r4.xyzx
    r3.x = (dot((r1.xzwx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 138: mul_sat r3.y, r3.x, cb0[20].z
    r3.y = (saturate((r3.xxxx)*(source[20].zzzz))).y;
    // 139: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 140: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 141: mul_sat r4.w, r4.z, cb0[20].z
    r4.w = (saturate((r4.zzzz)*(source[20].zzzz))).w;
    // 142: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: add_sat r4.w, r4.w, -cb0[20].w
    r4.w = (saturate((r4.wwww)+(-(source[20].wwww)))).w;
    // 144: log r5.w, r4.w
    r5.w = (log2(r4.wwww)).w;
    // 145: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 146: mul r5.w, r5.w, cb0[21].x
    r5.w = ((r5.wwww)*(source[21].xxxx)).w;
    // 147: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 148: mul r3.y, r3.y, r5.w
    r3.y = ((r3.yyyy)*(r5.wwww)).y;
    // 149: movc r3.y, r4.w, l(0), r3.y
    r3.y = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).y;
    // 150: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 151: mul r10.xyz, r8.xyzx, r2.wwww
    r10.xyz = ((r8.xyzx)*(r2.wwww)).xyz;
    // 152: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 154: mad r0.yzw, cb0[18].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[18].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 155: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 157: mad r0.yzw, cb0[18].wwww, r11.xxyz, r0.yyzw
    r0.yzw = ((source[18].wwww)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 158: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 159: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 160: mad r12.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r12.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 161: mad r11.xyz, cb0[18].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[18].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 162: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 164: mad r11.xyz, cb0[18].wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((source[18].wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 165: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 166: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 168: mul r13.xyz, r11.xyzx, r12.xyzx
    r13.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 169: mad r11.xyz, -r11.xyzx, r12.xyzx, cb0[7].xyzx
    r11.xyz = ((-(r11.xyzx))*(r12.xyzx)+(source[7].xyzx)).xyz;
    // 170: mad r11.xyz, r3.wwww, r11.xyzx, r13.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r13.xyzx)).xyz;
    // 171: mul r0.yzw, r0.yyzw, r11.xxyz
    r0.yzw = ((r0.yyzw)*(r11.xxyz)).yzw;
    // 172: mul r8.xyz, r8.xyzx, r0.yzwy
    r8.xyz = ((r8.xyzx)*(r0.yzwy)).xyz;
    // 173: mad r2.xyz, r2.xyzx, r10.xyzx, -r8.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r8.xyzx))).xyz;
    // 174: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mul r2.w, r2.w, cb0[22].x
    r2.w = ((r2.wwww)*(source[22].xxxx)).w;
    // 176: mad r2.xyz, r2.wwww, r2.xyzx, r8.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 177: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 178: sqrt r3.w, r2.w
    r3.w = (sqrt(r2.wwww)).w;
    // 179: div r8.xyz, r9.xyzx, r3.wwww
    r8.xyz = ((r9.xyzx)/(r3.wwww)).xyz;
    // 180: dp3 r3.w, r8.xyzx, r4.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 181: add r4.x, -|r4.z|, l(1.000000)
    r4.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 182: mul r3.x, r3.x, r4.x
    r3.x = ((r3.xxxx)*(r4.xxxx)).x;
    // 183: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mul r4.x, |r3.w|, |r3.w|
    r4.x = ((abs(r3.wwww))*(abs(r3.wwww))).x;
    // 185: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 186: mul r4.x, |r3.w|, r4.x
    r4.x = ((abs(r3.wwww))*(r4.xxxx)).x;
    // 187: lt r3.w, |r3.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 188: movc r3.w, r3.w, l(0), r4.x
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).w;
    // 189: add r4.x, r3.w, l(-0.027778)
    r4.x = ((r3.wwww)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).x;
    // 190: mad r3.w, r3.w, r4.x, l(0.027778)
    r3.w = ((r3.wwww)*(r4.xxxx)+(float4(0.027778,0.027778,0.027778,0.027778))).w;
    // 191: div_sat r2.w, r3.w, r2.w
    r2.w = (saturate((r3.wwww)/(r2.wwww))).w;
    // 192: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 194: mad r4.xyz, r1.yyyy, r2.xyzx, -r0.yzwy
    r4.xyz = ((r1.yyyy)*(r2.xyzx)+(-(r0.yzwy))).xyz;
    // 195: mad r0.xyz, r0.xxxx, r4.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r4.xyzx)+(r0.yzwy)).xyz;
    // 196: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 197: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 198: mad r0.xyz, cb0[18].zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((source[18].zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 199: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 200: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 201: mad r0.xyz, cb0[18].wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((source[18].wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 202: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 203: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r0.w, r0.w, cb0[22].y
    r0.w = ((r0.wwww)*(source[22].yyyy)).w;
    // 205: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 206: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 207: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 208: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 209: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 210: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 211: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 212: mul r4.x, r1.y, l(0.125000)
    r4.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 213: mul r8.y, cb0[3].y, cb0[13].y
    r8.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 214: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 215: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 216: add r4.xy, r4.xyxx, r8.xyxx
    r4.xy = ((r4.xyxx)+(r8.xyxx)).xy;
    // 217: frc r1.y, cb0[3].x
    r1.y = (frac(source[3].xxxx)).y;
    // 218: add r2.w, -r1.y, cb0[3].x
    r2.w = ((-(r1.yyyy))+(source[3].xxxx)).w;
    // 219: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 220: add r4.xy, r4.xyxx, r8.zwzz
    r4.xy = ((r4.xyxx)+(r8.zwzz)).xy;
    // 221: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 222: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 223: mul r0.w, r1.y, r4.w
    r0.w = ((r1.yyyy)*(r4.wwww)).w;
    // 224: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 225: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 226: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 227: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 228: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 229: add r4.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r4.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 230: add r4.xy, -r4.zwzz, r4.xyxx
    r4.xy = ((-(r4.zwzz))+(r4.xyxx)).xy;
    // 231: mad r4.xy, cb0[14].wwww, r4.xyxx, r4.zwzz
    r4.xy = ((source[14].wwww)*(r4.xyxx)+(r4.zwzz)).xy;
    // 232: mul r0.w, cb0[14].y, cb0[22].y
    r0.w = ((source[14].yyyy)*(source[22].yyyy)).w;
    // 233: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 234: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 235: mul r8.y, r0.w, l(0.020000)
    r8.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 236: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 238: mul r2.w, cb0[14].x, l(0.001000)
    r2.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 239: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 240: mad r4.xy, r2.wwww, r4.xyxx, r8.xyxx
    r4.xy = ((r2.wwww)*(r4.xyxx)+(r8.xyxx)).xy;
    // 241: dp2 r2.w, cb0[15].xyxx, r4.xyxx
    r2.w = (dot((source[15].xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 242: dp2 r4.y, cb0[16].xyxx, r4.xyxx
    r4.y = (dot((source[16].xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // 243: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 244: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 246: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 247: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 248: mad r4.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 249: mul_sat r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = (saturate((r0.wwww)*(r4.xyzx))).xyz;
    // 250: mad r8.xyz, cb0[14].zzzz, r4.xyzx, -r0.xyzx
    r8.xyz = ((source[14].zzzz)*(r4.xyzx)+(-(r0.xyzx))).xyz;
    // 251: mul r4.xyz, r4.xyzx, cb0[14].zzzz
    r4.xyz = ((r4.xyzx)*(source[14].zzzz)).xyz;
    // 252: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 253: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 254: mad r0.xyz, r0.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 255: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 256: add r0.w, -r3.z, r3.y
    r0.w = ((-(r3.zzzz))+(r3.yyyy)).w;
    // 257: mad r4.xyz, r3.yyyy, cb0[11].xyzx, -cb0[11].xyzx
    r4.xyz = ((r3.yyyy)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 258: mad r4.xyz, cb0[11].wwww, r4.xyzx, cb0[11].xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(source[11].xyzx)).xyz;
    // 259: mad r0.w, cb0[10].w, r0.w, r3.z
    r0.w = ((source[10].wwww)*(r0.wwww)+(r3.zzzz)).w;
    // 260: mad r3.yzw, r0.wwww, cb0[10].xxyz, r4.xxyz
    r3.yzw = ((r0.wwww)*(source[10].xxyz)+(r4.xxyz)).yzw;
    // 261: mul r4.xyz, r2.xyzx, r1.yyyy
    r4.xyz = ((r2.xyzx)*(r1.yyyy)).xyz;
    // 262: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: mad r2.xyz, -r1.yyyy, r2.xyzx, r0.wwww
    r2.xyz = ((-(r1.yyyy))*(r2.xyzx)+(r0.wwww)).xyz;
    // 264: mad r2.xyz, cb0[18].zzzz, r2.xyzx, r4.xyzx
    r2.xyz = ((source[18].zzzz)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 265: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 267: mad r2.xyz, cb0[18].wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((source[18].wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 268: mad r2.xyz, r2.xyzx, r12.xyzx, r3.yzwy
    r2.xyz = ((r2.xyzx)*(r12.xyzx)+(r3.yzwy)).xyz;
    // 269: log r0.w, |r3.x|
    r0.w = (log2(abs(r3.xxxx))).w;
    // 270: lt r1.y, |r3.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 271: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 272: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 273: mul r3.xyz, r0.wwww, cb0[12].xyzx
    r3.xyz = ((r0.wwww)*(source[12].xyzx)).xyz;
    // 274: movc r3.xyz, r1.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 275: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 276: add r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)+(source[2].xyzx)).xyz;
    // 277: dp3 r0.w, r1.xzwx, r1.xzwx
    r0.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 278: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 279: mul r1.xyz, r0.wwww, r1.xzwx
    r1.xyz = ((r0.wwww)*(r1.xzwx)).xyz;
    // 280: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 281: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 282: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 283: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 284: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 285: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 286: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 287: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 288: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 289: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 290: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 291: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 292: mad o0.xyz, r0.xyzx, cb0[25].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)+(r2.xyzx)).xyz;
    // 293: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 294: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 295: dp3 r0.x, r6.xyzx, r1.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 296: dp3 r0.z, r5.xyzx, r1.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 297: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 298: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 299: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 300: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 301: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 302: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 303: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 304: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 305: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 306: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 307: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 308: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 309: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 310: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 311: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 312: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 313: ret
    return output;
}

// source.character.mokoko-av036-906.v1 / source program 730977fec0ecc445aeb774018fc1181c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase906(SOURCE_CHARACTER_NATIVE_INPUT input)
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

// source.character.mokoko-av036-907.v1 / source program 7b5fb28cef42da4d9b46638e5ad60ce4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase907(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[21]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].w=(g_SourceCharacterTime.xxxx).x;
    source[25].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[25].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[25].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
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
    // 7: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 8: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 9: mad r0.xyz, cb0[22].yyyy, r1.xyzx, r0.yzwy
    r0.xyz = ((source[22].yyyy)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 10: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 11: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 12: mad r0.xyz, cb0[22].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[22].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 13: add r1.xy, v4.xyxx, cb0[12].xyxx
    r1.xy = ((v4.xyxx)+(source[12].xyxx)).xy;
    // 14: mul r2.xyz, v7.yyyy, cb1[1].xywx
    r2.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 15: mad r2.xyz, cb1[0].xywx, v7.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v7.xxxx)+(r2.xyzx)).xyz;
    // 16: mad r2.xyz, cb1[2].xywx, v7.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v7.zzzz)+(r2.xyzx)).xyz;
    // 17: mad r2.xyz, cb1[3].xywx, v7.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v7.wwww)+(r2.xyzx)).xyz;
    // 18: div r1.zw, r2.xxxy, r2.zzzz
    r1.zw = ((r2.xxxy)/(r2.zzzz)).zw;
    // 19: mad r1.zw, r1.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r1.zw = ((r1.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 20: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 21: mad r1.xy, cb0[12].zzzz, r1.xyxx, r1.zwzz
    r1.xy = ((source[12].zzzz)*(r1.xyxx)+(r1.zwzz)).xy;
    // 22: mul r1.xy, r1.xyxx, cb0[11].xyxx
    r1.xy = ((r1.xyxx)*(source[11].xyxx)).xy;
    // 23: mad r1.xy, cb0[22].wwww, cb0[11].zwzz, r1.xyxx
    r1.xy = ((source[22].wwww)*(source[11].zwzz)+(r1.xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.xzwy, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).w;
    // 25: mul r2.xyz, r0.wwww, cb0[13].xyzx
    r2.xyz = ((r0.wwww)*(source[13].xyzx)).xyz;
    // 26: mul r2.xyz, r2.xyzx, cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(source[13].wwww)).xyz;
    // 27: add r1.xy, v4.xyxx, cb0[9].xyxx
    r1.xy = ((v4.xyxx)+(source[9].xyxx)).xy;
    // 28: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 29: mad r1.xy, cb0[9].zzzz, r1.xyxx, r1.zwzz
    r1.xy = ((source[9].zzzz)*(r1.xyxx)+(r1.zwzz)).xy;
    // 30: mul r1.xy, r1.xyxx, cb0[8].xyxx
    r1.xy = ((r1.xyxx)*(source[8].xyxx)).xy;
    // 31: mad r1.xy, cb0[22].wwww, cb0[8].zwzz, r1.xyxx
    r1.xy = ((source[22].wwww)*(source[8].zwzz)+(r1.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 33: mul r1.xyz, r0.wwww, cb0[10].xyzx
    r1.xyz = ((r0.wwww)*(source[10].xyzx)).xyz;
    // 34: mad r3.xyz, cb0[10].wwww, r1.xyzx, r2.xyzx
    r3.xyz = ((source[10].wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 36: mad r1.xyz, r1.xyzx, r2.xyzx, -r3.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // 37: mad r1.xyz, cb0[23].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[23].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 39: add r3.xy, r2.ywyy, -cb0[2].zyzz
    r3.xy = ((r2.ywyy)+(-(source[2].zyzz))).xy;
    // 40: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 41: add r0.w, -r3.x, l(1.000000)
    r0.w = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: add r0.w, -r3.y, r0.w
    r0.w = ((-(r3.yyyy))+(r0.wwww)).w;
    // 43: mad r0.w, cb0[14].z, r0.w, r3.y
    r0.w = ((source[14].zzzz)*(r0.wwww)+(r3.yyyy)).w;
    // 44: mul r0.w, r0.w, cb0[14].x
    r0.w = ((r0.wwww)*(source[14].xxxx)).w;
    // 45: mul r0.w, r0.w, l(0.000314)
    r0.w = ((r0.wwww)*(float4(0.000314,0.000314,0.000314,0.000314))).w;
    // 46: mad r0.w, cb0[14].y, cb0[22].w, r0.w
    r0.w = ((source[14].yyyy)*(source[22].wwww)+(r0.wwww)).w;
    // 47: add r3.xyz, r0.wwww, l(0.000000, 0.330000, 0.660000, 0.000000)
    r3.xyz = ((r0.wwww)+(float4(0.000000,0.330000,0.660000,0.000000))).xyz;
    // 48: mul r3.xyz, r3.xyzx, l(6.283185, 6.283185, 6.283185, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(6.283185,6.283185,6.283185,0.000000))).xyz;
    // 49: sincos null, r3.xyz, r3.xyzx
    r3.xyz = (cos(r3.xyzx)).xyz;
    // 50: mad r3.xyz, r3.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 51: mad r1.xyz, cb0[14].wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((source[14].wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 52: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 54: mad r1.xyz, cb0[22].yyyy, r3.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 55: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 57: mad r1.xyz, cb0[22].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 58: mad r3.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 59: mad r4.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 60: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 61: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 62: mad r1.xyz, r0.xyzx, r1.xyzx, -r0.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 64: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 65: dp3 r0.w, cb0[5].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((source[5].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 66: add r1.xyz, r0.wwww, -cb0[5].xyzx
    r1.xyz = ((r0.wwww)+(-(source[5].xyzx))).xyz;
    // 67: mad r1.xyz, cb0[22].yyyy, r1.xyzx, cb0[5].xyzx
    r1.xyz = ((source[22].yyyy)*(r1.xyzx)+(source[5].xyzx)).xyz;
    // 68: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 69: add r4.xyz, -r1.xyzx, r0.wwww
    r4.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 70: mad r1.xyz, cb0[22].zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 71: mul r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.xyzx)).xyz;
    // 72: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 74: mov_sat r1.xyz, r1.xyzx
    r1.xyz = (saturate(r1.xyzx)).xyz;
    // 75: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 76: add r0.w, r1.z, r0.w
    r0.w = ((r1.zzzz)+(r0.wwww)).w;
    // 77: add_sat r0.w, r1.w, r0.w
    r0.w = (saturate((r1.wwww)+(r0.wwww))).w;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 79: dp3 r3.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 80: add r3.xyz, -r1.xyzx, r3.xxxx
    r3.xyz = ((-(r1.xyzx))+(r3.xxxx)).xyz;
    // 81: mad r1.xyz, cb0[22].yyyy, r3.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 82: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: dp3 r3.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 84: add r3.xyz, -r1.xyzx, r3.xxxx
    r3.xyz = ((-(r1.xyzx))+(r3.xxxx)).xyz;
    // 85: mad r1.xyz, cb0[22].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 86: dp3 r3.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 87: add r3.xyz, -r1.xyzx, r3.xxxx
    r3.xyz = ((-(r1.xyzx))+(r3.xxxx)).xyz;
    // 88: mul r3.xyz, r3.xyzx, cb0[23].yyyy
    r3.xyz = ((r3.xyzx)*(source[23].yyyy)).xyz;
    // 89: mad r1.xyz, r0.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 90: max r3.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 91: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 92: mul r3.xyz, r3.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 93: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 94: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 96: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 97: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 98: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mad r3.x, -r0.w, r0.w, l(1.000000)
    r3.x = ((-(r0.wwww))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: max r3.x, r3.x, l(0.001000)
    r3.x = (max(r3.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 101: div r3.x, cb0[24].z, r3.x
    r3.x = ((source[24].zzzz)/(r3.xxxx)).x;
    // 102: sample_b_indexable(texture2d)(float,float,float,float) r3.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r3.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 103: mad r3.yz, r3.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((r3.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 104: dp2 r3.w, r3.yzyy, r3.yzyy
    r3.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 105: mul r4.xy, r3.yzyy, cb0[22].xxxx
    r4.xy = ((r3.yzyy)*(source[22].xxxx)).xy;
    // 106: add r3.y, -r3.w, l(1.000000)
    r3.y = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 107: max r3.y, r3.y, l(0.000000)
    r3.y = (max(r3.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 108: sqrt r3.y, r3.y
    r3.y = (sqrt(r3.yyyy)).y;
    // 109: add r4.z, r3.y, l(0.000010)
    r4.z = ((r3.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 110: dp3 r3.y, r4.xyzx, r4.xyzx
    r3.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 111: sqrt r3.y, r3.y
    r3.y = (sqrt(r3.yyyy)).y;
    // 112: div r3.yzw, r4.xxyz, r3.yyyy
    r3.yzw = ((r4.xxyz)/(r3.yyyy)).yzw;
    // 113: dp3 r4.x, v5.xyzx, v5.xyzx
    r4.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 114: rsq r4.x, r4.x
    r4.x = (rsqrt(r4.xxxx)).x;
    // 115: mul r4.yzw, r4.xxxx, v5.xxyz
    r4.yzw = ((r4.xxxx)*(v5.xxyz)).yzw;
    // 116: mad r5.xyz, v5.xyzx, r4.xxxx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((v5.xyzx)*(r4.xxxx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 117: dp3 r4.x, r3.yzwy, r4.yzwy
    r4.x = (dot((r3.yzwy).xyz,(r4.yzwy).xyz).xxxx).x;
    // 118: mul_sat r5.w, r4.x, cb0[23].z
    r5.w = (saturate((r4.xxxx)*(source[23].zzzz))).w;
    // 119: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: mul_sat r6.x, r4.w, cb0[23].z
    r6.x = (saturate((r4.wwww)*(source[23].zzzz))).x;
    // 121: add r6.x, -r6.x, l(1.000000)
    r6.x = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 122: add_sat r6.x, r6.x, -cb0[23].w
    r6.x = (saturate((r6.xxxx)+(-(source[23].wwww)))).x;
    // 123: log r6.y, r6.x
    r6.y = (log2(r6.xxxx)).y;
    // 124: lt r6.x, r6.x, l(0.000001)
    r6.x = (asfloat((uint4)((r6.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 125: mul r6.y, r6.y, cb0[24].x
    r6.y = ((r6.yyyy)*(source[24].xxxx)).y;
    // 126: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 127: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 128: movc r5.w, r6.x, l(0), r5.w
    r5.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 129: mul r3.x, r3.x, r5.w
    r3.x = ((r3.xxxx)*(r5.wwww)).x;
    // 130: mad r1.xyz, r3.xxxx, r1.xyzx, -r0.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 131: add r3.x, -r0.w, l(1.000000)
    r3.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mul r3.x, r3.x, cb0[24].w
    r3.x = ((r3.xxxx)*(source[24].wwww)).x;
    // 133: mad r1.xyz, r3.xxxx, r1.xyzx, r0.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 134: dp3 r3.x, r5.xyzx, r5.xyzx
    r3.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 135: sqrt r6.x, r3.x
    r6.x = (sqrt(r3.xxxx)).x;
    // 136: div r5.xyz, r5.xyzx, r6.xxxx
    r5.xyz = ((r5.xyzx)/(r6.xxxx)).xyz;
    // 137: dp3 r4.y, r5.xyzx, r4.yzwy
    r4.y = (dot((r5.xyzx).xyz,(r4.yzwy).xyz).xxxx).y;
    // 138: add r4.xz, -|r4.xxwx|, l(1.000000, 0.000000, 1.000000, 0.000000)
    r4.xz = ((-(abs(r4.xxwx)))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 139: mul r4.x, r4.x, r4.z
    r4.x = ((r4.xxxx)*(r4.zzzz)).x;
    // 140: add r4.y, -r4.y, l(1.000000)
    r4.y = ((-(r4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 141: mul r4.z, |r4.y|, |r4.y|
    r4.z = ((abs(r4.yyyy))*(abs(r4.yyyy))).z;
    // 142: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 143: mul r4.z, r4.z, |r4.y|
    r4.z = ((r4.zzzz)*(abs(r4.yyyy))).z;
    // 144: lt r4.y, |r4.y|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 145: movc r4.y, r4.y, l(0), r4.z
    r4.y = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.zzzz)).y;
    // 146: add r4.z, r4.y, l(-0.027778)
    r4.z = ((r4.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 147: mad r4.y, r4.y, r4.z, l(0.027778)
    r4.y = ((r4.yyyy)*(r4.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 148: div_sat r3.x, r4.y, r3.x
    r3.x = (saturate((r4.yyyy)/(r3.xxxx))).x;
    // 149: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 150: log r4.y, |r1.w|
    r4.y = (log2(abs(r1.wwww))).y;
    // 151: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 152: mul r4.y, r4.y, cb0[25].x
    r4.y = ((r4.yyyy)*(source[25].xxxx)).y;
    // 153: exp r4.y, r4.y
    r4.y = (exp2(r4.yyyy)).y;
    // 154: min r4.y, r4.y, l(1.000000)
    r4.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 155: mul r3.x, r3.x, r4.y
    r3.x = ((r3.xxxx)*(r4.yyyy)).x;
    // 156: movc r1.w, r1.w, l(0), r3.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 157: mad r4.yzw, r1.wwww, r1.xxyz, -r0.xxyz
    r4.yzw = ((r1.wwww)*(r1.xxyz)+(-(r0.xxyz))).yzw;
    // 158: mad r0.xyz, r0.wwww, r4.yzwy, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.yzwy)+(r0.xyzx)).xyz;
    // 159: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: mul r0.w, r0.w, cb0[22].w
    r0.w = ((r0.wwww)*(source[22].wwww)).w;
    // 161: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 162: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 163: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: mul r1.w, cb0[4].z, l(1.500000)
    r1.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 165: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 166: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 167: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 168: mul r5.x, r1.w, l(0.125000)
    r5.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 169: mul r6.y, cb0[4].y, cb0[18].y
    r6.y = ((source[4].yyyy)*(source[18].yyyy)).y;
    // 170: mov r5.y, v4.y
    r5.y = (v4.yyyy).y;
    // 171: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 172: add r4.yz, r5.xxyx, r6.xxyx
    r4.yz = ((r5.xxyx)+(r6.xxyx)).yz;
    // 173: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 174: add r3.x, -r1.w, cb0[4].x
    r3.x = ((-(r1.wwww))+(source[4].xxxx)).x;
    // 175: mul r6.z, r3.x, l(0.125000)
    r6.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 176: add r4.yz, r4.yyzy, r6.zzwz
    r4.yz = ((r4.yyzy)+(r6.zzwz)).yz;
    // 177: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r4.yzyy, t5.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 178: mul r4.yzw, r0.wwww, r6.xxyz
    r4.yzw = ((r0.wwww)*(r6.xxyz)).yzw;
    // 179: mul r0.w, r1.w, r6.w
    r0.w = ((r1.wwww)*(r6.wwww)).w;
    // 180: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: mad r4.yzw, r4.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r0.xxyz
    r4.yzw = ((r4.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r0.xxyz))).yzw;
    // 182: mad r0.xyz, r0.wwww, r4.yzwy, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.yzwy)+(r0.xyzx)).xyz;
    // 183: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 184: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 185: mad r2.xy, cb0[19].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[19].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 186: mul r0.w, cb0[19].y, cb0[22].w
    r0.w = ((source[19].yyyy)*(source[22].wwww)).w;
    // 187: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 188: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 189: mul r5.y, r0.w, l(0.020000)
    r5.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 190: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 192: mul r2.z, cb0[19].x, l(0.001000)
    r2.z = ((source[19].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 193: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 194: mad r2.xy, r2.zzzz, r2.xyxx, r5.xyxx
    r2.xy = ((r2.zzzz)*(r2.xyxx)+(r5.xyxx)).xy;
    // 195: dp2 r5.y, cb0[21].xyxx, r2.xyxx
    r5.y = (dot((source[21].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 196: dp2 r2.x, cb0[20].xyxx, r2.xyxx
    r2.x = (dot((source[20].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 197: frc r2.x, r2.x
    r2.x = (frac(r2.xxxx)).x;
    // 198: mul r5.x, r2.x, l(0.125000)
    r5.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 199: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r5.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 200: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 201: mul r2.w, r2.w, l(0.900000)
    r2.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 202: mad r2.xyz, r2.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 203: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 204: mad r4.yzw, cb0[19].zzzz, r2.xxyz, -r0.xxyz
    r4.yzw = ((source[19].zzzz)*(r2.xxyz)+(-(r0.xxyz))).yzw;
    // 205: mul r2.xyz, r2.xyzx, cb0[19].zzzz
    r2.xyz = ((r2.xyzx)*(source[19].zzzz)).xyz;
    // 206: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 207: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 208: mad r0.xyz, r0.wwww, r4.yzwy, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.yzwy)+(r0.xyzx)).xyz;
    // 209: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 210: mad r2.xyz, r5.wwww, cb0[16].xyzx, -cb0[16].xyzx
    r2.xyz = ((r5.wwww)*(source[16].xyzx)+(-(source[16].xyzx))).xyz;
    // 211: mad r2.xyz, cb0[16].wwww, r2.xyzx, cb0[16].xyzx
    r2.xyz = ((source[16].wwww)*(r2.xyzx)+(source[16].xyzx)).xyz;
    // 212: mad r2.xyz, r5.wwww, cb0[15].xyzx, r2.xyzx
    r2.xyz = ((r5.wwww)*(source[15].xyzx)+(r2.xyzx)).xyz;
    // 213: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 214: log r0.w, |r4.x|
    r0.w = (log2(abs(r4.xxxx))).w;
    // 215: lt r1.w, |r4.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 216: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 217: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 218: mul r2.xyz, r0.wwww, cb0[17].xyzx
    r2.xyz = ((r0.wwww)*(source[17].xyzx)).xyz;
    // 219: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 220: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 221: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 222: dp3 r0.w, r3.yzwy, r3.yzwy
    r0.w = (dot((r3.yzwy).xyz,(r3.yzwy).xyz).xxxx).w;
    // 223: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 224: mul r2.xyz, r0.wwww, r3.yzwy
    r2.xyz = ((r0.wwww)*(r3.yzwy)).xyz;
    // 225: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 226: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 227: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 228: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 229: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 230: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 231: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 232: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 233: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 234: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 235: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 236: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 237: mad o0.xyz, r0.xyzx, cb0[28].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[28].xyzx)+(r1.xyzx)).xyz;
    // 238: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 239: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 240: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 241: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 242: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 243: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 244: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 245: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 246: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 247: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 248: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 249: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 250: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 251: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 252: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 253: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 254: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 255: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 256: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 257: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 258: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 259: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 260: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 261: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 262: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 263: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 264: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 265: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 266: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 267: ret
    return output;
}

// source.character.mokoko-av036-908.v1 / source program ef3ed8accecee5408d9ec5722c094cca
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase908(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[19]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[26].w=(g_SourceCharacterTime.xxxx).x;
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

// source.character.mokoko-av036-909.v1 / source program 90354559e1a5114e8d542c4ab375935a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase909(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[21]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[23]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[24]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[25].w=(g_SourceCharacterTime.xxxx).x;
    source[28].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[28].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
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
    // 7: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 8: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 9: mad r0.xyz, cb0[25].yyyy, r1.xyzx, r0.yzwy
    r0.xyz = ((source[25].yyyy)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 10: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 11: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 12: mad r0.xyz, cb0[25].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[25].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 13: add r1.xy, v4.xyxx, cb0[14].xyxx
    r1.xy = ((v4.xyxx)+(source[14].xyxx)).xy;
    // 14: mul r2.xyz, v7.yyyy, cb1[1].xywx
    r2.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 15: mad r2.xyz, cb1[0].xywx, v7.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v7.xxxx)+(r2.xyzx)).xyz;
    // 16: mad r2.xyz, cb1[2].xywx, v7.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v7.zzzz)+(r2.xyzx)).xyz;
    // 17: mad r2.xyz, cb1[3].xywx, v7.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v7.wwww)+(r2.xyzx)).xyz;
    // 18: div r1.zw, r2.xxxy, r2.zzzz
    r1.zw = ((r2.xxxy)/(r2.zzzz)).zw;
    // 19: mad r1.zw, r1.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r1.zw = ((r1.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 20: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 21: mad r1.xy, cb0[14].zzzz, r1.xyxx, r1.zwzz
    r1.xy = ((source[14].zzzz)*(r1.xyxx)+(r1.zwzz)).xy;
    // 22: mul r1.xy, r1.xyxx, cb0[13].xyxx
    r1.xy = ((r1.xyxx)*(source[13].xyxx)).xy;
    // 23: mad r1.xy, cb0[25].wwww, cb0[13].zwzz, r1.xyxx
    r1.xy = ((source[25].wwww)*(source[13].zwzz)+(r1.xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.xzwy, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).w;
    // 25: mul r2.xyz, r0.wwww, cb0[15].xyzx
    r2.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 26: mul r2.xyz, r2.xyzx, cb0[15].wwww
    r2.xyz = ((r2.xyzx)*(source[15].wwww)).xyz;
    // 27: add r1.xy, v4.xyxx, cb0[11].xyxx
    r1.xy = ((v4.xyxx)+(source[11].xyxx)).xy;
    // 28: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 29: mad r1.xy, cb0[11].zzzz, r1.xyxx, r1.zwzz
    r1.xy = ((source[11].zzzz)*(r1.xyxx)+(r1.zwzz)).xy;
    // 30: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 31: mad r1.xy, cb0[25].wwww, cb0[10].zwzz, r1.xyxx
    r1.xy = ((source[25].wwww)*(source[10].zwzz)+(r1.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 33: mul r1.xyz, r0.wwww, cb0[12].xyzx
    r1.xyz = ((r0.wwww)*(source[12].xyzx)).xyz;
    // 34: mad r3.xyz, cb0[12].wwww, r1.xyzx, r2.xyzx
    r3.xyz = ((source[12].wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 36: mad r1.xyz, r1.xyzx, r2.xyzx, -r3.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // 37: mad r1.xyz, cb0[26].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[26].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyz, -r1.xyzx, cb0[16].xyzx
    r2.xyz = ((-(r1.xyzx))+(source[16].xyzx)).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 40: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 42: mul r3.xy, r3.xyxx, cb0[25].xxxx
    r3.xy = ((r3.xyxx)*(source[25].xxxx)).xy;
    // 43: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 46: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 47: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r4.xyz, r4.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r3.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r3.xyzx)).xyz;
    // 49: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 50: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 51: div r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 52: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r5.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r4.xyzx)).xyz;
    // 54: mul r6.xyz, r5.yyyy, cb0[3].xyzx
    r6.xyz = ((r5.yyyy)*(source[3].xyzx)).xyz;
    // 55: mad r5.xyw, cb0[2].xyxz, r5.xxxx, r6.xyxz
    r5.xyw = ((source[2].xyxz)*(r5.xxxx)+(r6.xyxz)).xyw;
    // 56: mad r5.xyz, cb0[4].xyzx, r5.zzzz, r5.xywx
    r5.xyz = ((source[4].xyzx)*(r5.zzzz)+(r5.xywx)).xyz;
    // 57: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 58: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 59: mul r6.xyz, r0.wwww, v5.xyzx
    r6.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 60: mad r7.xyz, v5.xyzx, r0.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((v5.xyzx)*(r0.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 62: mul r4.xy, r4.xyxx, r0.wwww
    r4.xy = ((r4.xyxx)*(r0.wwww)).xy;
    // 63: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r6.xyxx
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r6.xyxx))).xy;
    // 64: mad r4.zw, cb0[25].wwww, cb0[17].zzzw, v4.xxxy
    r4.zw = ((source[25].wwww)*(source[17].zzzw)+(v4.xxxy)).zw;
    // 65: mad r4.xy, r4.zwzz, cb0[17].xyxx, r4.xyxx
    r4.xy = ((r4.zwzz)*(source[17].xyxx)+(r4.xyxx)).xy;
    // 66: add r4.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r4.xyxx, t2.xywz, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).w;
    // 68: mul_sat r0.w, r0.w, cb0[16].w
    r0.w = (saturate((r0.wwww)*(source[16].wwww))).w;
    // 69: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 70: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 72: mad r1.xyz, cb0[25].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[25].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 73: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 75: mad r1.xyz, cb0[25].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[25].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 76: mad r2.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mad r4.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 79: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 80: mul r4.xyz, r0.xyzx, r1.xyzx
    r4.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 81: mad r1.xyz, r0.xyzx, r1.xyzx, -r0.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 83: mul r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 84: mul r5.xyz, r0.xyzx, r0.wwww
    r5.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 85: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 86: mul r1.xyz, r5.xyzx, cb0[28].xxxx
    r1.xyz = ((r5.xyzx)*(source[28].xxxx)).xyz;
    // 87: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 88: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 89: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 90: dp3 r1.w, r3.xyzx, r6.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 91: mul_sat r2.w, r1.w, cb0[26].z
    r2.w = (saturate((r1.wwww)*(source[26].zzzz))).w;
    // 92: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mul_sat r3.w, r6.z, cb0[26].z
    r3.w = (saturate((r6.zzzz)*(source[26].zzzz))).w;
    // 95: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: add_sat r3.w, r3.w, -cb0[26].w
    r3.w = (saturate((r3.wwww)+(-(source[26].wwww)))).w;
    // 97: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 98: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 99: mul r4.w, r4.w, cb0[27].x
    r4.w = ((r4.wwww)*(source[27].xxxx)).w;
    // 100: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 101: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 102: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 103: mad r5.xyz, r2.wwww, cb0[19].xyzx, -cb0[19].xyzx
    r5.xyz = ((r2.wwww)*(source[19].xyzx)+(-(source[19].xyzx))).xyz;
    // 104: mad r5.xyz, cb0[19].wwww, r5.xyzx, cb0[19].xyzx
    r5.xyz = ((source[19].wwww)*(r5.xyzx)+(source[19].xyzx)).xyz;
    // 105: mad r5.xyz, r2.wwww, cb0[18].xyzx, r5.xyzx
    r5.xyz = ((r2.wwww)*(source[18].xyzx)+(r5.xyzx)).xyz;
    // 106: mad r1.xyz, r4.xyzx, r1.xyzx, -r5.xyzx
    r1.xyz = ((r4.xyzx)*(r1.xyzx)+(-(r5.xyzx))).xyz;
    // 107: mad r1.xyz, r0.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 108: dp3 r0.w, cb0[7].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((source[7].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r4.xyz, r0.wwww, -cb0[7].xyzx
    r4.xyz = ((r0.wwww)+(-(source[7].xyzx))).xyz;
    // 110: mad r4.xyz, cb0[25].yyyy, r4.xyzx, cb0[7].xyzx
    r4.xyz = ((source[25].yyyy)*(r4.xyzx)+(source[7].xyzx)).xyz;
    // 111: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 112: add r5.xyz, -r4.xyzx, r0.wwww
    r5.xyz = ((-(r4.xyzx))+(r0.wwww)).xyz;
    // 113: mad r4.xyz, cb0[25].zzzz, r5.xyzx, r4.xyzx
    r4.xyz = ((source[25].zzzz)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 114: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 115: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 116: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 117: mov_sat r4.xyz, r4.xyzx
    r4.xyz = (saturate(r4.xyzx)).xyz;
    // 118: add r0.w, r4.y, r4.x
    r0.w = ((r4.yyyy)+(r4.xxxx)).w;
    // 119: add r0.w, r4.z, r0.w
    r0.w = ((r4.zzzz)+(r0.wwww)).w;
    // 120: add_sat r0.w, r4.w, r0.w
    r0.w = (saturate((r4.wwww)+(r0.wwww))).w;
    // 121: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 122: dp3 r2.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 123: add r2.xyz, -r4.xyzx, r2.xxxx
    r2.xyz = ((-(r4.xyzx))+(r2.xxxx)).xyz;
    // 124: mad r2.xyz, cb0[25].yyyy, r2.xyzx, r4.xyzx
    r2.xyz = ((source[25].yyyy)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 125: add r3.w, -r4.w, l(1.000000)
    r3.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: dp3 r4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 127: add r4.xyz, -r2.xyzx, r4.xxxx
    r4.xyz = ((-(r2.xyzx))+(r4.xxxx)).xyz;
    // 128: mad r2.xyz, cb0[25].zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((source[25].zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 129: dp3 r4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 130: add r4.xyz, -r2.xyzx, r4.xxxx
    r4.xyz = ((-(r2.xyzx))+(r4.xxxx)).xyz;
    // 131: mul r4.xyz, r4.xyzx, cb0[26].yyyy
    r4.xyz = ((r4.xyzx)*(source[26].yyyy)).xyz;
    // 132: mad r2.xyz, r0.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 133: max r4.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 134: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 135: mul r4.xyz, r4.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 136: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 137: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 138: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 139: mul r0.w, r0.w, cb0[27].y
    r0.w = ((r0.wwww)*(source[27].yyyy)).w;
    // 140: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 141: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mad r4.x, -r0.w, r0.w, l(1.000000)
    r4.x = ((-(r0.wwww))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: max r4.x, r4.x, l(0.001000)
    r4.x = (max(r4.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 144: div r4.x, cb0[27].z, r4.x
    r4.x = ((source[27].zzzz)/(r4.xxxx)).x;
    // 145: mul r2.w, r2.w, r4.x
    r2.w = ((r2.wwww)*(r4.xxxx)).w;
    // 146: mad r2.xyz, r2.wwww, r2.xyzx, -r0.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 147: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: mul r2.w, r2.w, cb0[27].w
    r2.w = ((r2.wwww)*(source[27].wwww)).w;
    // 149: mad r2.xyz, r2.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 150: frc r2.w, cb0[6].x
    r2.w = (frac(source[6].xxxx)).w;
    // 151: add r4.x, -r2.w, l(1.000000)
    r4.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 152: mad r1.xyz, r4.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r4.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 153: add r4.x, -|r6.z|, l(1.000000)
    r4.x = ((-(abs(r6.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 154: mul r1.w, r1.w, r4.x
    r1.w = ((r1.wwww)*(r4.xxxx)).w;
    // 155: log r4.x, |r1.w|
    r4.x = (log2(abs(r1.wwww))).x;
    // 156: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 157: mul r4.x, r4.x, l(1.500000)
    r4.x = ((r4.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 158: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 159: mul r4.xyz, r4.xxxx, cb0[20].xyzx
    r4.xyz = ((r4.xxxx)*(source[20].xyzx)).xyz;
    // 160: movc r4.xyz, r1.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 161: add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // 162: add r1.xyz, r1.xyzx, cb0[5].xyzx
    r1.xyz = ((r1.xyzx)+(source[5].xyzx)).xyz;
    // 163: dp3 r1.w, r7.xyzx, r7.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 164: sqrt r4.x, r1.w
    r4.x = (sqrt(r1.wwww)).x;
    // 165: div r4.xyz, r7.xyzx, r4.xxxx
    r4.xyz = ((r7.xyzx)/(r4.xxxx)).xyz;
    // 166: dp3 r4.x, r4.xyzx, r6.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 167: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 168: mul r4.y, |r4.x|, |r4.x|
    r4.y = ((abs(r4.xxxx))*(abs(r4.xxxx))).y;
    // 169: mul r4.y, r4.y, r4.y
    r4.y = ((r4.yyyy)*(r4.yyyy)).y;
    // 170: mul r4.y, r4.y, |r4.x|
    r4.y = ((r4.yyyy)*(abs(r4.xxxx))).y;
    // 171: lt r4.x, |r4.x|, l(0.000001)
    r4.x = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 172: movc r4.x, r4.x, l(0), r4.y
    r4.x = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.yyyy)).x;
    // 173: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 174: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 175: div_sat r1.w, r4.x, r1.w
    r1.w = (saturate((r4.xxxx)/(r1.wwww))).w;
    // 176: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: log r4.x, |r3.w|
    r4.x = (log2(abs(r3.wwww))).x;
    // 178: lt r3.w, |r3.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 179: mul r4.x, r4.x, cb0[28].y
    r4.x = ((r4.xxxx)*(source[28].yyyy)).x;
    // 180: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 181: min r4.x, r4.x, l(1.000000)
    r4.x = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 182: mul r1.w, r1.w, r4.x
    r1.w = ((r1.wwww)*(r4.xxxx)).w;
    // 183: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 184: mad r2.xyz, r1.wwww, r2.xyzx, -r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 185: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 186: add r0.w, -cb0[6].w, l(1.000000)
    r0.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 187: mul r0.w, r0.w, cb0[25].w
    r0.w = ((r0.wwww)*(source[25].wwww)).w;
    // 188: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 189: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 190: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mul r1.w, cb0[6].z, l(1.500000)
    r1.w = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 192: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 193: mad r0.w, r0.w, l(0.500000), cb0[6].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 194: add r1.w, -r2.w, cb0[6].x
    r1.w = ((-(r2.wwww))+(source[6].xxxx)).w;
    // 195: mul r4.z, r1.w, l(0.125000)
    r4.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 196: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 197: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 198: mul r4.y, cb0[6].y, cb0[21].y
    r4.y = ((source[6].yyyy)*(source[21].yyyy)).y;
    // 199: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 200: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 201: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 202: add r2.xy, r2.xyxx, r4.zwzz
    r2.xy = ((r2.xyxx)+(r4.zwzz)).xy;
    // 203: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 204: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 205: mul r0.w, r2.w, r4.w
    r0.w = ((r2.wwww)*(r4.wwww)).w;
    // 206: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 207: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 208: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 209: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 210: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 211: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 212: mad r2.xy, cb0[22].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[22].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 213: mul r0.w, cb0[22].y, cb0[25].w
    r0.w = ((source[22].yyyy)*(source[25].wwww)).w;
    // 214: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 215: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 216: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 217: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 219: mul r1.w, cb0[22].x, l(0.001000)
    r1.w = ((source[22].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 220: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 221: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 222: dp2 r1.w, cb0[23].xyxx, r2.xyxx
    r1.w = (dot((source[23].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 223: dp2 r2.y, cb0[24].xyxx, r2.xyxx
    r2.y = (dot((source[24].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 224: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 225: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 226: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 227: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 228: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 229: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 230: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 231: mad r4.xyz, cb0[22].zzzz, r2.xyzx, -r0.xyzx
    r4.xyz = ((source[22].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 232: mul r2.xyz, r2.xyzx, cb0[22].zzzz
    r2.xyz = ((r2.xyzx)*(source[22].zzzz)).xyz;
    // 233: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 234: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 235: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 236: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 237: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 238: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 239: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 240: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 241: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 242: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 243: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 244: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 245: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 246: mul r3.yzw, r3.yyyy, cb0[30].xxyz
    r3.yzw = ((r3.yyyy)*(source[30].xxyz)).yzw;
    // 247: mad r3.xyz, r3.xxxx, cb0[29].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[29].xyzx)+(r3.yzwy)).xyz;
    // 248: mul r3.xyz, r3.xyzx, cb0[31].wwww
    r3.xyz = ((r3.xyzx)*(source[31].wwww)).xyz;
    // 249: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 251: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 252: mad o0.xyz, r0.xyzx, cb0[31].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[31].xyzx)+(r1.xyzx)).xyz;
    // 253: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 254: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 255: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 256: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 257: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 258: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 259: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 260: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 261: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 262: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 263: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 264: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 265: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 266: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 267: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 268: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 269: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 270: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 271: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 272: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 273: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 274: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 275: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 276: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 277: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 278: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 279: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 280: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 281: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 282: ret
    return output;
}

// source.character.mokoko-av036-910.v1 / source program 51b8426a9aa240479ef9ef2dc78e6a72
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase910(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].w=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[16].xyzw
    r1.xyzw = ((r0.xyzw)*(source[16].xyzw)).xyzw;
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
    // 15: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 16: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 17: mad r0.xyz, cb0[17].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[17].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 18: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 19: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 20: mad r0.xyz, cb0[17].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 21: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 22: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 23: mad r2.xyz, -cb0[4].wwww, cb0[4].xyzx, r0.wwww
    r2.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r0.wwww)).xyz;
    // 24: mad r1.xyz, cb0[17].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 25: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 27: mad r1.xyz, cb0[17].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 28: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 29: mad r3.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 30: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 31: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 32: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 33: mul r1.xyz, v7.yyyy, cb1[1].xywx
    r1.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 34: mad r1.xyz, cb1[0].xywx, v7.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v7.xxxx)+(r1.xyzx)).xyz;
    // 35: mad r1.xyz, cb1[2].xywx, v7.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v7.zzzz)+(r1.xyzx)).xyz;
    // 36: mad r1.xyz, cb1[3].xywx, v7.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v7.wwww)+(r1.xyzx)).xyz;
    // 37: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 38: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 39: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 40: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 41: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 42: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 43: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 44: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 45: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 46: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 47: rcp r1.x, |r0.w|
    r1.x = (1.0/(abs(r0.wwww))).x;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 49: add r1.y, -r3.w, l(1.000000)
    r1.y = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 50: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 51: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 52: mul r1.z, r1.z, cb0[18].x
    r1.z = ((r1.zzzz)*(source[18].xxxx)).z;
    // 53: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 54: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 55: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 56: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 57: mul r1.z, r1.z, cb0[18].y
    r1.z = ((r1.zzzz)*(source[18].yyyy)).z;
    // 58: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 59: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 60: add r0.w, |r0.w|, r1.x
    r0.w = ((abs(r0.wwww))+(r1.xxxx)).w;
    // 61: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 63: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 64: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 65: mul r4.xy, r1.xzxx, cb0[17].xxxx
    r4.xy = ((r1.xzxx)*(source[17].xxxx)).xy;
    // 66: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 67: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 68: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 69: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 70: add r1.xzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 71: mad r1.xzw, cb0[17].wwww, r1.xxzw, r4.xxyz
    r1.xzw = ((source[17].wwww)*(r1.xxzw)+(r4.xxyz)).xzw;
    // 72: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 73: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 74: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 75: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 76: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 77: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 78: dp3 r6.x, r5.xyzx, r1.xzwx
    r6.x = (dot((r5.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 79: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 80: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 81: mul r7.xyz, r2.wwww, v1.xyzx
    r7.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 82: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 83: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 84: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 85: dp3 r6.y, r8.xyzx, r1.xzwx
    r6.y = (dot((r8.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 86: dp3 r6.z, r7.xyzx, r1.xzwx
    r6.z = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 87: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 88: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 89: mul r9.xyz, r1.xxxx, v5.xyzx
    r9.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 90: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 91: dp3 r10.y, r8.xyzx, r9.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 92: dp3 r10.x, r5.xyzx, r9.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 93: dp3 r10.z, r7.xyzx, r9.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 94: dp3 r2.w, r6.xyzx, r10.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 95: mul r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 96: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 97: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 98: dp2 r2.w, r6.ywyy, r6.ywyy
    r2.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 99: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 100: div r6.xy, r6.ywyy, r2.wwww
    r6.xy = ((r6.ywyy)/(r2.wwww)).xy;
    // 101: mad r2.w, -r6.z, l(0.250000), l(0.250000)
    r2.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 102: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 104: mad r6.xy, r2.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 105: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s3, r0.w
    r6.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.wwww).x)).xyzw).xyz;
    // 106: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 107: rcp r0.w, cb0[18].z
    r0.w = (1.0/(source[18].zzzz)).w;
    // 108: mul r11.xyz, r10.xyzx, r0.wwww
    r11.xyz = ((r10.xyzx)*(r0.wwww)).xyz;
    // 109: mul r10.xyz, r10.xyzx, cb0[18].zzzz
    r10.xyz = ((r10.xyzx)*(source[18].zzzz)).xyz;
    // 110: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 111: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 112: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 113: mad r10.xyz, r10.xyzx, cb0[18].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[18].zzzz)+(r11.xyzx)).xyz;
    // 114: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 115: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 116: add r0.w, cb0[18].z, l(1.000000)
    r0.w = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 118: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r6.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r6.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 120: mad r6.xyz, r3.wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 121: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 122: mul r6.xyz, r6.xyzx, cb0[18].wwww
    r6.xyz = ((r6.xyzx)*(source[18].wwww)).xyz;
    // 123: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 124: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 125: div r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 126: dp3 r0.w, r4.xyzx, r9.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 127: mul_sat r2.w, r0.w, cb0[19].y
    r2.w = (saturate((r0.wwww)*(source[19].yyyy))).w;
    // 128: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul_sat r3.w, r9.z, cb0[19].y
    r3.w = (saturate((r9.zzzz)*(source[19].yyyy))).w;
    // 131: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: add_sat r3.w, r3.w, -cb0[19].z
    r3.w = (saturate((r3.wwww)+(-(source[19].zzzz)))).w;
    // 133: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 134: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 135: mul r4.w, r4.w, cb0[19].w
    r4.w = ((r4.wwww)*(source[19].wwww)).w;
    // 136: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 137: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 138: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 139: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 141: mad r3.xyz, cb0[17].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[17].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 142: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 143: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 144: mad r3.xyz, cb0[17].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[17].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 145: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 146: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 147: mul r10.xyz, r10.xyzx, cb0[19].xxxx
    r10.xyz = ((r10.xyzx)*(source[19].xxxx)).xyz;
    // 148: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 149: add r3.w, r11.y, r11.x
    r3.w = ((r11.yyyy)+(r11.xxxx)).w;
    // 150: add r3.w, r11.z, r3.w
    r3.w = ((r11.zzzz)+(r3.wwww)).w;
    // 151: add_sat r3.w, r11.w, r3.w
    r3.w = (saturate((r11.wwww)+(r3.wwww))).w;
    // 152: mad r3.xyz, r3.wwww, r10.xyzx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 153: max r10.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 154: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 155: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 156: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 157: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 158: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 159: mul r3.w, r3.w, cb0[20].x
    r3.w = ((r3.wwww)*(source[20].xxxx)).w;
    // 160: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 161: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 164: div r4.w, cb0[20].y, r4.w
    r4.w = ((source[20].yyyy)/(r4.wwww)).w;
    // 165: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 166: mul r10.xyz, r6.xyzx, r4.wwww
    r10.xyz = ((r6.xyzx)*(r4.wwww)).xyz;
    // 167: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 168: mad r3.xyz, r3.xyzx, r10.xyzx, -r6.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 169: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: mul r4.w, r4.w, cb0[20].z
    r4.w = ((r4.wwww)*(source[20].zzzz)).w;
    // 171: mad r3.xyz, r4.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r4.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 172: dp3 r4.w, r1.xzwx, r1.xzwx
    r4.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 173: sqrt r5.w, r4.w
    r5.w = (sqrt(r4.wwww)).w;
    // 174: div r1.xzw, r1.xxzw, r5.wwww
    r1.xzw = ((r1.xxzw)/(r5.wwww)).xzw;
    // 175: dp3 r1.x, r1.xzwx, r9.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 176: add r1.z, -|r9.z|, l(1.000000)
    r1.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 177: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 178: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 179: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 180: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 181: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 182: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 183: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 184: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 185: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 186: div_sat r1.x, r1.x, r4.w
    r1.x = (saturate((r1.xxxx)/(r4.wwww))).x;
    // 187: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 188: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 189: mad r1.xyz, r1.xxxx, r3.xyzx, -r0.xyzx
    r1.xyz = ((r1.xxxx)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 190: mad r0.xyz, r3.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 191: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 192: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 193: mad r0.xyz, cb0[17].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 194: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 195: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 196: mad r0.xyz, cb0[17].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 197: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 198: add r1.x, -cb0[3].w, l(1.000000)
    r1.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 199: mul r1.x, r1.x, cb0[20].w
    r1.x = ((r1.xxxx)*(source[20].wwww)).x;
    // 200: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 201: sincos r1.x, null, r1.x
    r1.x = (sin(r1.xxxx)).x;
    // 202: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 203: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 204: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 205: mad r1.x, r1.x, l(0.500000), cb0[3].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 206: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 207: mul r6.x, r1.y, l(0.125000)
    r6.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 208: mul r9.y, cb0[3].y, cb0[12].y
    r9.y = ((source[3].yyyy)*(source[12].yyyy)).y;
    // 209: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 210: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 211: add r1.yz, r6.xxyx, r9.xxyx
    r1.yz = ((r6.xxyx)+(r9.xxyx)).yz;
    // 212: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 213: add r3.w, -r1.w, cb0[3].x
    r3.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 214: mul r9.z, r3.w, l(0.125000)
    r9.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 215: add r1.yz, r1.yyzy, r9.zzwz
    r1.yz = ((r1.yyzy)+(r9.zzwz)).yz;
    // 216: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.yzyy, t6.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 217: mul r1.xyz, r1.xxxx, r6.xyzx
    r1.xyz = ((r1.xxxx)*(r6.xyzx)).xyz;
    // 218: mul r3.w, r1.w, r6.w
    r3.w = ((r1.wwww)*(r6.wwww)).w;
    // 219: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 221: mad r0.xyz, r3.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 222: add r6.xyzw, v7.yzxy, cb0[0].yzxy
    r6.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 223: add r6.xyzw, r6.xyzw, -cb0[1].yzxy
    r6.xyzw = ((r6.xyzw)+(-(source[1].yzxy))).xyzw;
    // 224: add r1.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 225: add r1.xy, -r6.zwzz, r1.xyxx
    r1.xy = ((-(r6.zwzz))+(r1.xyxx)).xy;
    // 226: mad r1.xy, cb0[13].wwww, r1.xyxx, r6.zwzz
    r1.xy = ((source[13].wwww)*(r1.xyxx)+(r6.zwzz)).xy;
    // 227: mul r1.z, cb0[13].y, cb0[20].w
    r1.z = ((source[13].yyyy)*(source[20].wwww)).z;
    // 228: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 229: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 230: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 231: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 232: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 233: mul r3.w, cb0[13].x, l(0.001000)
    r3.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 234: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 235: mad r1.xy, r3.wwww, r1.xyxx, r6.xyxx
    r1.xy = ((r3.wwww)*(r1.xyxx)+(r6.xyxx)).xy;
    // 236: dp2 r3.w, cb0[14].xyxx, r1.xyxx
    r3.w = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 237: dp2 r1.y, cb0[15].xyxx, r1.xyxx
    r1.y = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 238: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 239: mul r1.x, r3.w, l(0.125000)
    r1.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 240: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.xyxx, t6.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 241: mad r6.xyz, r6.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 242: mul r1.x, r6.w, l(0.900000)
    r1.x = ((r6.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 243: mad r6.xyz, r1.xxxx, r6.xyzx, r0.xyzx
    r6.xyz = ((r1.xxxx)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 244: mul_sat r1.xyz, r1.zzzz, r6.xyzx
    r1.xyz = (saturate((r1.zzzz)*(r6.xyzx))).xyz;
    // 245: mad r6.xyz, cb0[13].zzzz, r1.xyzx, -r0.xyzx
    r6.xyz = ((source[13].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 246: mul r1.xyz, r1.xyzx, cb0[13].zzzz
    r1.xyz = ((r1.xyzx)*(source[13].zzzz)).xyz;
    // 247: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 248: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 249: mad r0.xyz, r1.xxxx, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 250: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 251: add r1.x, r2.w, -r11.z
    r1.x = ((r2.wwww)+(-(r11.zzzz))).x;
    // 252: mad r6.xyz, r2.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r6.xyz = ((r2.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 253: mad r6.xyz, cb0[10].wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((source[10].wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 254: mad r1.x, cb0[9].w, r1.x, r11.z
    r1.x = ((source[9].wwww)*(r1.xxxx)+(r11.zzzz)).x;
    // 255: mad r1.xyz, r1.xxxx, cb0[9].xyzx, r6.xyzx
    r1.xyz = ((r1.xxxx)*(source[9].xyzx)+(r6.xyzx)).xyz;
    // 256: mul r6.xyz, r3.xyzx, r1.wwww
    r6.xyz = ((r3.xyzx)*(r1.wwww)).xyz;
    // 257: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: mad r3.xyz, -r1.wwww, r3.xyzx, r2.wwww
    r3.xyz = ((-(r1.wwww))*(r3.xyzx)+(r2.wwww)).xyz;
    // 259: mad r3.xyz, cb0[17].yyyy, r3.xyzx, r6.xyzx
    r3.xyz = ((source[17].yyyy)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 260: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 261: add r6.xyz, -r3.xyzx, r1.wwww
    r6.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 262: mad r3.xyz, cb0[17].zzzz, r6.xyzx, r3.xyzx
    r3.xyz = ((source[17].zzzz)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 263: mad r1.xyz, r3.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 264: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 265: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 266: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 267: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 268: mul r2.xyz, r1.wwww, cb0[11].xyzx
    r2.xyz = ((r1.wwww)*(source[11].xyzx)).xyz;
    // 269: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 270: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 271: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 272: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 273: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 274: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 275: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 276: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 277: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 278: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 279: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 280: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 281: mul r3.yzw, r3.yyyy, cb0[22].xxyz
    r3.yzw = ((r3.yyyy)*(source[22].xxyz)).yzw;
    // 282: mad r3.xyz, r3.xxxx, cb0[21].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[21].xyzx)+(r3.yzwy)).xyz;
    // 283: mul r3.xyz, r3.xyzx, cb0[23].wwww
    r3.xyz = ((r3.xyzx)*(source[23].wwww)).xyz;
    // 284: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 285: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 286: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 287: mad o0.xyz, r0.xyzx, cb0[23].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 288: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 289: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 290: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 291: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 292: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 293: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 294: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 295: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 296: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 297: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 298: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 299: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 300: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 301: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 302: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 303: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 304: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 305: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 306: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 307: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 308: ret
    return output;
}

// source.character.mokoko-av036-911.v1 / source program 1197647575df5c4e84030f14b1aad34b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase911(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 17: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 18: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r0.w, r5.w, cb0[1].w
    r0.w = ((r5.wwww)*(source[1].wwww)).w;
    // 23: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 24: mul r6.xyz, r6.xyzx, cb0[16].xxxx
    r6.xyz = ((r6.xyzx)*(source[16].xxxx)).xyz;
    // 25: mad r6.xyz, r5.yyyy, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r5.yyyy)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 26: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 28: mad r6.xyz, cb0[16].yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((source[16].yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 29: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 31: mad r6.xyz, cb0[16].zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((source[16].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 32: mad r7.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: mad r8.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 34: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 35: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 36: mul r8.xyz, r5.xxxx, r8.xyzx
    r8.xyz = ((r5.xxxx)*(r8.xyzx)).xyz;
    // 37: mul r1.w, cb0[11].z, l(1.500000)
    r1.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 38: add r2.w, -cb0[11].w, l(1.000000)
    r2.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r2.w, r2.w, cb0[19].y
    r2.w = ((r2.wwww)*(source[19].yyyy)).w;
    // 40: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 41: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 42: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 44: mad r1.w, r1.w, l(0.500000), cb0[11].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 45: frc r2.w, cb0[11].x
    r2.w = (frac(source[11].xxxx)).w;
    // 46: add r3.w, -r2.w, cb0[11].x
    r3.w = ((-(r2.wwww))+(source[11].xxxx)).w;
    // 47: mul r9.z, r3.w, l(0.125000)
    r9.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 48: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 49: mul r9.y, cb0[11].y, cb0[12].y
    r9.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 50: mul r10.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r10.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 51: frc r3.w, r10.x
    r3.w = (frac(r10.xxxx)).w;
    // 52: mul r10.y, r3.w, l(0.125000)
    r10.y = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 53: add r5.xy, r9.xyxx, r10.yzyy
    r5.xy = ((r9.xyxx)+(r10.yzyy)).xy;
    // 54: add r5.xy, r5.xyxx, r9.zwzz
    r5.xy = ((r5.xyxx)+(r9.zwzz)).xy;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 56: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 57: mul r1.w, r2.w, r9.w
    r1.w = ((r2.wwww)*(r9.wwww)).w;
    // 58: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 59: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 60: mul r1.w, cb0[13].y, cb0[19].y
    r1.w = ((source[13].yyyy)*(source[19].yyyy)).w;
    // 61: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 62: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 63: mul r5.y, r1.w, l(0.020000)
    r5.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 64: add r9.xyzw, r1.yzxy, -cb0[1].yzxy
    r9.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 65: add r9.xy, -r9.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r9.xy = ((-(r9.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 66: add r9.xy, -r9.zwzz, r9.xyxx
    r9.xy = ((-(r9.zwzz))+(r9.xyxx)).xy;
    // 67: mad r9.xy, cb0[13].wwww, r9.xyxx, r9.zwzz
    r9.xy = ((source[13].wwww)*(r9.xyxx)+(r9.zwzz)).xy;
    // 68: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 69: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 70: mad r5.xy, r2.wwww, r9.xyxx, r5.xyxx
    r5.xy = ((r2.wwww)*(r9.xyxx)+(r5.xyxx)).xy;
    // 71: dp2 r2.w, cb0[14].xyxx, r5.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 72: dp2 r5.y, cb0[15].xyxx, r5.xyxx
    r5.y = (dot((source[15].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 73: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 74: mul r5.x, r2.w, l(0.125000)
    r5.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 76: mul r2.w, r9.w, l(0.900000)
    r2.w = ((r9.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 77: mad r9.xyz, r9.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r8.xyzx))).xyz;
    // 78: mad r9.xyz, r2.wwww, r9.xyzx, r8.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 79: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 81: mul_sat r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = (saturate((r9.xyzx)*(r1.wwww))).xyz;
    // 82: mul r10.xyz, r9.xyzx, cb0[13].zzzz
    r10.xyz = ((r9.xyzx)*(source[13].zzzz)).xyz;
    // 83: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 84: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 85: mad r9.xyz, cb0[13].zzzz, r9.xyzx, -r8.xyzx
    r9.xyz = ((source[13].zzzz)*(r9.xyzx)+(-(r8.xyzx))).xyz;
    // 86: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 87: mad r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = ((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 88: mad r6.xyz, r6.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r6.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 89: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 90: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 91: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 92: mul r5.xyz, r5.zzzz, r6.xyzx
    r5.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 93: mov_sat r1.w, r2.z
    r1.w = (saturate(r2.zzzz)).w;
    // 94: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 95: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 96: mul r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 97: mul r5.xyz, r5.xyzx, cb0[16].wwww
    r5.xyz = ((r5.xyzx)*(source[16].wwww)).xyz;
    // 98: add r1.xyz, -r1.xyzx, cb0[0].yzwy
    r1.xyz = ((-(r1.xyzx))+(source[0].yzwy)).xyz;
    // 99: mul r6.xyz, r2.zzzz, r1.xyzx
    r6.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 100: mad r1.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 101: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 102: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 103: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 104: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 105: dp3 r1.y, r0.xyzx, r3.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 106: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 107: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 108: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 109: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 110: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: mad r1.y, cb0[17].z, l(4.500000), l(0.500000)
    r1.y = ((source[17].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 112: mul r1.y, r1.y, cb0[17].w
    r1.y = ((r1.yyyy)*(source[17].wwww)).y;
    // 113: mul r1.y, r1.y, l(0.050000)
    r1.y = ((r1.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 114: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 115: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 116: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 117: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 118: mul r1.x, r1.x, cb0[18].x
    r1.x = ((r1.xxxx)*(source[18].xxxx)).x;
    // 119: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 120: dp3 r1.y, r3.xyzx, r2.xyzx
    r1.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 121: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 122: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add r2.x, -|r1.y|, l(1.000000)
    r2.x = ((-(abs(r1.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 124: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 125: mad r2.xyw, r1.wwww, cb0[9].xyxz, -cb0[9].xyxz
    r2.xyw = ((r1.wwww)*(source[9].xyxz)+(-(source[9].xyxz))).xyw;
    // 126: mad r2.xyw, cb0[9].wwww, r2.xyxw, cb0[9].xyxz
    r2.xyw = ((source[9].wwww)*(r2.xyxw)+(source[9].xyxz)).xyw;
    // 127: mad r2.xyw, r1.zzzz, cb0[8].xyxz, r2.xyxw
    r2.xyw = ((r1.zzzz)*(source[8].xyxz)+(r2.xyxw)).xyw;
    // 128: mul_sat r1.y, r1.y, cb0[18].y
    r1.y = (saturate((r1.yyyy)*(source[18].yyyy))).y;
    // 129: mul_sat r1.z, r2.z, cb0[18].y
    r1.z = (saturate((r2.zzzz)*(source[18].yyyy))).z;
    // 130: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 131: add_sat r1.z, r1.z, -cb0[18].z
    r1.z = (saturate((r1.zzzz)+(-(source[18].zzzz)))).z;
    // 132: lt r1.w, r1.z, l(0.000001)
    r1.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 133: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 134: mul r1.z, r1.z, cb0[18].w
    r1.z = ((r1.zzzz)*(source[18].wwww)).z;
    // 135: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 136: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 137: mul r1.y, r1.y, cb0[10].w
    r1.y = ((r1.yyyy)*(source[10].wwww)).y;
    // 138: mul r3.xyz, r1.yyyy, cb0[10].xyzx
    r3.xyz = ((r1.yyyy)*(source[10].xyzx)).xyz;
    // 139: movc r1.yzw, r1.wwww, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 140: add r1.yzw, r1.yyzw, r2.xxyw
    r1.yzw = ((r1.yyzw)+(r2.xxyw)).yzw;
    // 141: mad r1.xyz, r1.xxxx, r5.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(r1.yzwy)).xyz;
    // 142: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 143: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 144: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 145: mul r2.xyz, r1.wwww, v7.xyzx
    r2.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 146: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 147: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 148: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 149: mul r2.yzw, r2.yyyy, cb0[21].xxyz
    r2.yzw = ((r2.yyyy)*(source[21].xxyz)).yzw;
    // 150: mad r2.xyz, r2.xxxx, cb0[20].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[20].xyzx)+(r2.yzwy)).xyz;
    // 151: mul r2.xyz, r2.xyzx, cb0[22].wwww
    r2.xyz = ((r2.xyzx)*(source[22].wwww)).xyz;
    // 152: mul r3.xyz, r8.xyzx, r2.xyzx
    r3.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 153: mad r1.xyz, r2.xyzx, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 154: mad r1.xyz, r8.xyzx, cb0[22].xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(source[22].xyzx)+(r1.xyzx)).xyz;
    // 155: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 157: eq r2.x, cb0[23].x, l(0.000000)
    r2.x = (asfloat((uint4)((source[23].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 158: not r2.y, r2.x
    r2.y = (asfloat(~asuint(r2.xxxx))).y;
    // 159: lt r2.z, r0.w, r1.w
    r2.z = (asfloat((uint4)((r0.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 160: and r2.y, r2.z, r2.y
    r2.y = (asfloat(asuint(r2.zzzz) & asuint(r2.yyyy))).y;
    // 161: discard_nz r2.y
    if ((asuint(r2.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 162: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 163: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 164: mul r2.yzw, r2.yyyy, v0.xxyz
    r2.yzw = ((r2.yyyy)*(v0.xxyz)).yzw;
    // 165: mul r5.xyz, r0.zxyz, r2.zwyz
    r5.xyz = ((r0.zxyz)*(r2.zwyz)).xyz;
    // 166: mad r5.xyz, r0.yzxy, r2.wyzw, -r5.xyzx
    r5.xyz = ((r0.yzxy)*(r2.wyzw)+(-(r5.xyzx))).xyz;
    // 167: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 168: movc r3.w, v9.x, l(1.000000), l(-1.000000)
    r3.w = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 169: mul r3.w, r3.w, cb0[0].x
    r3.w = ((r3.wwww)*(source[0].xxxx)).w;
    // 170: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 171: ge r1.w, r0.w, r1.w
    r1.w = (asfloat((uint4)((r0.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 172: mad r3.w, r5.w, cb0[1].w, l(-0.900000)
    r3.w = ((r5.wwww)*(source[1].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 173: mul_sat r3.w, r3.w, l(9.999998)
    r3.w = (saturate((r3.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 174: mad r4.w, r3.w, l(-2.000000), l(3.000000)
    r4.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 175: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 176: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 177: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 178: movc r1.w, r1.w, r3.w, r0.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r3.wwww) : (r0.wwww)).w;
    // 179: movc o0.w, r2.x, r1.w, r0.w
    output.targets[0].w = ((asuint(r2.xxxx) != 0u) ? (r1.wwww) : (r0.wwww)).w;
    // 180: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 181: dp3 r1.x, r2.yzwy, r4.xyzx
    r1.x = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).x;
    // 182: dp3 r1.y, r5.xyzx, r4.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 183: dp3 r1.z, r0.xyzx, r4.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 184: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 185: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 186: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 187: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 188: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 189: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 190: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 191: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 192: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 193: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 194: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 195: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 197: mov o3.xyz, r8.xyzx
    output.targets[3].xyz = (r8.xyzx).xyz;
    // 198: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 199: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 200: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 201: ret
    return output;
}

// source.character.mokoko-av036-912.v1 / source program b84386481c5a494790f47702c98ff7dc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase912(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].w=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 7: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 8: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 9: mad r0.xyz, cb0[16].yyyy, r1.xyzx, r0.yzwy
    r0.xyz = ((source[16].yyyy)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 10: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 11: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 12: mad r0.xyz, cb0[16].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 13: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 14: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: mad r2.xyz, -cb0[4].wwww, cb0[4].xyzx, r0.wwww
    r2.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r0.wwww)).xyz;
    // 16: mad r1.xyz, cb0[16].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 17: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 18: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 19: mad r1.xyz, cb0[16].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 20: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 21: mad r3.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 22: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 23: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 24: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: mul r1.xyz, v7.yyyy, cb1[1].xywx
    r1.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 26: mad r1.xyz, cb1[0].xywx, v7.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v7.xxxx)+(r1.xyzx)).xyz;
    // 27: mad r1.xyz, cb1[2].xywx, v7.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v7.zzzz)+(r1.xyzx)).xyz;
    // 28: mad r1.xyz, cb1[3].xywx, v7.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v7.wwww)+(r1.xyzx)).xyz;
    // 29: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 30: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 31: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 32: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 33: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 34: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 35: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 36: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 37: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 38: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 39: rcp r1.x, |r0.w|
    r1.x = (1.0/(abs(r0.wwww))).x;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: add r1.y, -r3.w, l(1.000000)
    r1.y = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 42: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 43: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 44: mul r1.z, r1.z, cb0[17].x
    r1.z = ((r1.zzzz)*(source[17].xxxx)).z;
    // 45: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 46: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 47: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 48: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 49: mul r1.z, r1.z, cb0[17].y
    r1.z = ((r1.zzzz)*(source[17].yyyy)).z;
    // 50: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 51: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 52: add r0.w, |r0.w|, r1.x
    r0.w = ((abs(r0.wwww))+(r1.xxxx)).w;
    // 53: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 55: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 56: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 57: mul r4.xy, r1.xzxx, cb0[16].xxxx
    r4.xy = ((r1.xzxx)*(source[16].xxxx)).xy;
    // 58: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 60: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 61: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: add r1.xzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 63: mad r1.xzw, cb0[16].wwww, r1.xxzw, r4.xxyz
    r1.xzw = ((source[16].wwww)*(r1.xxzw)+(r4.xxyz)).xzw;
    // 64: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 65: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 66: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 67: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 68: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 69: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 70: dp3 r6.x, r5.xyzx, r1.xzwx
    r6.x = (dot((r5.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 71: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 72: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 73: mul r7.xyz, r2.wwww, v1.xyzx
    r7.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 74: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 75: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 76: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 77: dp3 r6.y, r8.xyzx, r1.xzwx
    r6.y = (dot((r8.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 78: dp3 r6.z, r7.xyzx, r1.xzwx
    r6.z = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 79: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 80: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 81: mul r9.xyz, r1.xxxx, v5.xyzx
    r9.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 82: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 83: dp3 r10.y, r8.xyzx, r9.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 84: dp3 r10.x, r5.xyzx, r9.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 85: dp3 r10.z, r7.xyzx, r9.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 86: dp3 r2.w, r6.xyzx, r10.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 87: mul r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 88: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 89: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 90: dp2 r2.w, r6.ywyy, r6.ywyy
    r2.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 91: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 92: div r6.xy, r6.ywyy, r2.wwww
    r6.xy = ((r6.ywyy)/(r2.wwww)).xy;
    // 93: mad r2.w, -r6.z, l(0.250000), l(0.250000)
    r2.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 94: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 96: mad r6.xy, r2.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 97: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t3.xyzw, s3, r0.w
    r6.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.wwww).x)).xyzw).xyz;
    // 98: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 99: rcp r0.w, cb0[17].z
    r0.w = (1.0/(source[17].zzzz)).w;
    // 100: mul r11.xyz, r10.xyzx, r0.wwww
    r11.xyz = ((r10.xyzx)*(r0.wwww)).xyz;
    // 101: mul r10.xyz, r10.xyzx, cb0[17].zzzz
    r10.xyz = ((r10.xyzx)*(source[17].zzzz)).xyz;
    // 102: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 103: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 104: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 105: mad r10.xyz, r10.xyzx, cb0[17].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[17].zzzz)+(r11.xyzx)).xyz;
    // 106: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 107: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 108: add r0.w, cb0[17].z, l(1.000000)
    r0.w = ((source[17].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 110: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 111: add r6.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r6.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 112: mad r6.xyz, r3.wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 113: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 114: mul r6.xyz, r6.xyzx, cb0[17].wwww
    r6.xyz = ((r6.xyzx)*(source[17].wwww)).xyz;
    // 115: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 116: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 117: div r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 118: dp3 r0.w, r4.xyzx, r9.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 119: mul_sat r2.w, r0.w, cb0[18].y
    r2.w = (saturate((r0.wwww)*(source[18].yyyy))).w;
    // 120: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul_sat r3.w, r9.z, cb0[18].y
    r3.w = (saturate((r9.zzzz)*(source[18].yyyy))).w;
    // 123: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: add_sat r3.w, r3.w, -cb0[18].z
    r3.w = (saturate((r3.wwww)+(-(source[18].zzzz)))).w;
    // 125: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 126: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 127: mul r4.w, r4.w, cb0[18].w
    r4.w = ((r4.wwww)*(source[18].wwww)).w;
    // 128: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 129: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 130: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 131: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 133: mad r3.xyz, cb0[16].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 134: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 135: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 136: mad r3.xyz, cb0[16].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 137: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 138: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 139: mul r10.xyz, r10.xyzx, cb0[18].xxxx
    r10.xyz = ((r10.xyzx)*(source[18].xxxx)).xyz;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 141: add r3.w, r11.y, r11.x
    r3.w = ((r11.yyyy)+(r11.xxxx)).w;
    // 142: add r3.w, r11.z, r3.w
    r3.w = ((r11.zzzz)+(r3.wwww)).w;
    // 143: add_sat r3.w, r11.w, r3.w
    r3.w = (saturate((r11.wwww)+(r3.wwww))).w;
    // 144: mad r3.xyz, r3.wwww, r10.xyzx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 145: max r10.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 146: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 147: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 148: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 149: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 151: mul r3.w, r3.w, cb0[19].x
    r3.w = ((r3.wwww)*(source[19].xxxx)).w;
    // 152: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 153: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 156: div r4.w, cb0[19].y, r4.w
    r4.w = ((source[19].yyyy)/(r4.wwww)).w;
    // 157: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 158: mul r10.xyz, r6.xyzx, r4.wwww
    r10.xyz = ((r6.xyzx)*(r4.wwww)).xyz;
    // 159: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 160: mad r3.xyz, r3.xyzx, r10.xyzx, -r6.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 161: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: mul r4.w, r4.w, cb0[19].z
    r4.w = ((r4.wwww)*(source[19].zzzz)).w;
    // 163: mad r3.xyz, r4.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r4.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 164: dp3 r4.w, r1.xzwx, r1.xzwx
    r4.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 165: sqrt r5.w, r4.w
    r5.w = (sqrt(r4.wwww)).w;
    // 166: div r1.xzw, r1.xxzw, r5.wwww
    r1.xzw = ((r1.xxzw)/(r5.wwww)).xzw;
    // 167: dp3 r1.x, r1.xzwx, r9.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 168: add r1.z, -|r9.z|, l(1.000000)
    r1.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 169: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 170: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 171: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 172: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 173: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 174: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 175: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 176: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 177: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 178: div_sat r1.x, r1.x, r4.w
    r1.x = (saturate((r1.xxxx)/(r4.wwww))).x;
    // 179: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 181: mad r1.xyz, r1.xxxx, r3.xyzx, -r0.xyzx
    r1.xyz = ((r1.xxxx)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 182: mad r0.xyz, r3.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 183: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 184: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 185: mad r0.xyz, cb0[16].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[16].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 186: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 187: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 188: mad r0.xyz, cb0[16].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 189: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 190: add r1.x, -cb0[3].w, l(1.000000)
    r1.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 191: mul r1.x, r1.x, cb0[19].w
    r1.x = ((r1.xxxx)*(source[19].wwww)).x;
    // 192: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 193: sincos r1.x, null, r1.x
    r1.x = (sin(r1.xxxx)).x;
    // 194: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 195: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 196: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 197: mad r1.x, r1.x, l(0.500000), cb0[3].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 198: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 199: mul r6.x, r1.y, l(0.125000)
    r6.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 200: mul r9.y, cb0[3].y, cb0[12].y
    r9.y = ((source[3].yyyy)*(source[12].yyyy)).y;
    // 201: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 202: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 203: add r1.yz, r6.xxyx, r9.xxyx
    r1.yz = ((r6.xxyx)+(r9.xxyx)).yz;
    // 204: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 205: add r3.w, -r1.w, cb0[3].x
    r3.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 206: mul r9.z, r3.w, l(0.125000)
    r9.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 207: add r1.yz, r1.yyzy, r9.zzwz
    r1.yz = ((r1.yyzy)+(r9.zzwz)).yz;
    // 208: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.yzyy, t5.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 209: mul r1.xyz, r1.xxxx, r6.xyzx
    r1.xyz = ((r1.xxxx)*(r6.xyzx)).xyz;
    // 210: mul r3.w, r1.w, r6.w
    r3.w = ((r1.wwww)*(r6.wwww)).w;
    // 211: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 212: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 213: mad r0.xyz, r3.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 214: add r6.xyzw, v7.yzxy, cb0[0].yzxy
    r6.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 215: add r6.xyzw, r6.xyzw, -cb0[1].yzxy
    r6.xyzw = ((r6.xyzw)+(-(source[1].yzxy))).xyzw;
    // 216: add r1.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 217: add r1.xy, -r6.zwzz, r1.xyxx
    r1.xy = ((-(r6.zwzz))+(r1.xyxx)).xy;
    // 218: mad r1.xy, cb0[13].wwww, r1.xyxx, r6.zwzz
    r1.xy = ((source[13].wwww)*(r1.xyxx)+(r6.zwzz)).xy;
    // 219: mul r1.z, cb0[13].y, cb0[19].w
    r1.z = ((source[13].yyyy)*(source[19].wwww)).z;
    // 220: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 221: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 222: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 223: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 224: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 225: mul r3.w, cb0[13].x, l(0.001000)
    r3.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 226: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 227: mad r1.xy, r3.wwww, r1.xyxx, r6.xyxx
    r1.xy = ((r3.wwww)*(r1.xyxx)+(r6.xyxx)).xy;
    // 228: dp2 r3.w, cb0[14].xyxx, r1.xyxx
    r3.w = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 229: dp2 r1.y, cb0[15].xyxx, r1.xyxx
    r1.y = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 230: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 231: mul r1.x, r3.w, l(0.125000)
    r1.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 232: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 233: mad r6.xyz, r6.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 234: mul r1.x, r6.w, l(0.900000)
    r1.x = ((r6.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 235: mad r6.xyz, r1.xxxx, r6.xyzx, r0.xyzx
    r6.xyz = ((r1.xxxx)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 236: mul_sat r1.xyz, r1.zzzz, r6.xyzx
    r1.xyz = (saturate((r1.zzzz)*(r6.xyzx))).xyz;
    // 237: mad r6.xyz, cb0[13].zzzz, r1.xyzx, -r0.xyzx
    r6.xyz = ((source[13].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 238: mul r1.xyz, r1.xyzx, cb0[13].zzzz
    r1.xyz = ((r1.xyzx)*(source[13].zzzz)).xyz;
    // 239: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 240: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 241: mad r0.xyz, r1.xxxx, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 242: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 243: add r1.x, r2.w, -r11.z
    r1.x = ((r2.wwww)+(-(r11.zzzz))).x;
    // 244: mad r6.xyz, r2.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r6.xyz = ((r2.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 245: mad r6.xyz, cb0[10].wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((source[10].wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 246: mad r1.x, cb0[9].w, r1.x, r11.z
    r1.x = ((source[9].wwww)*(r1.xxxx)+(r11.zzzz)).x;
    // 247: mad r1.xyz, r1.xxxx, cb0[9].xyzx, r6.xyzx
    r1.xyz = ((r1.xxxx)*(source[9].xyzx)+(r6.xyzx)).xyz;
    // 248: mul r6.xyz, r3.xyzx, r1.wwww
    r6.xyz = ((r3.xyzx)*(r1.wwww)).xyz;
    // 249: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 250: mad r3.xyz, -r1.wwww, r3.xyzx, r2.wwww
    r3.xyz = ((-(r1.wwww))*(r3.xyzx)+(r2.wwww)).xyz;
    // 251: mad r3.xyz, cb0[16].yyyy, r3.xyzx, r6.xyzx
    r3.xyz = ((source[16].yyyy)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 252: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 253: add r6.xyz, -r3.xyzx, r1.wwww
    r6.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 254: mad r3.xyz, cb0[16].zzzz, r6.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 255: mad r1.xyz, r3.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 256: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 257: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 258: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 259: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 260: mul r2.xyz, r1.wwww, cb0[11].xyzx
    r2.xyz = ((r1.wwww)*(source[11].xyzx)).xyz;
    // 261: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 262: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 263: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 264: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 265: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 266: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 267: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 268: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 269: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 270: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 271: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 272: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 273: mul r3.yzw, r3.yyyy, cb0[21].xxyz
    r3.yzw = ((r3.yyyy)*(source[21].xxyz)).yzw;
    // 274: mad r3.xyz, r3.xxxx, cb0[20].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[20].xyzx)+(r3.yzwy)).xyz;
    // 275: mul r3.xyz, r3.xyzx, cb0[22].wwww
    r3.xyz = ((r3.xyzx)*(source[22].wwww)).xyz;
    // 276: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 277: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 278: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 279: mad o0.xyz, r0.xyzx, cb0[22].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)+(r1.xyzx)).xyz;
    // 280: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 281: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 282: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 283: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 284: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 285: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 286: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 287: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 288: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 289: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 290: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 291: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 292: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 293: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 294: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 295: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 296: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 297: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 298: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 299: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 300: ret
    return output;
}

// source.character.mokoko-av036-913.v1 / source program f0b3a577d431bf47bed0179fe9f2161d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase913(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].x=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 4: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 5: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 6: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 7: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 8: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 9: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 10: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 11: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 12: mul r0.z, cb0[1].z, cb0[15].x
    r0.z = ((source[1].zzzz)*(source[15].xxxx)).z;
    // 13: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 14: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 15: add r0.zw, v4.xxxy, -cb0[1].xxxy
    r0.zw = ((v4.xxxy)+(-(source[1].xxxy))).zw;
    // 16: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 17: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 18: dp2 r1.y, r0.wzww, r3.yzyy
    r1.y = (dot((r0.wzww).xy,(r3.yzyy).xy).xxxx).y;
    // 19: dp2 r1.x, r0.wzww, r3.xyxx
    r1.x = (dot((r0.wzww).xy,(r3.xyxx).xy).xxxx).x;
    // 20: add r0.zw, r1.xxxy, cb0[1].xxxy
    r0.zw = ((r1.xxxy)+(source[1].xxxy)).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r0.zwzz, t1.zxyw, s1, l(0.000000)
    r1.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.x, r1.x, -r2.z
    r1.x = ((r1.xxxx)+(-(r2.zzzz))).x;
    // 24: mad r1.x, r2.y, r1.x, r2.z
    r1.x = ((r2.yyyy)*(r1.xxxx)+(r2.zzzz)).x;
    // 25: add r1.y, -cb0[15].z, cb0[15].y
    r1.y = ((-(source[15].zzzz))+(source[15].yyyy)).y;
    // 26: mad r1.y, r2.x, r1.y, cb0[15].z
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[15].zzzz)).y;
    // 27: add r1.z, -r1.y, cb0[15].w
    r1.z = ((-(r1.yyyy))+(source[15].wwww)).z;
    // 28: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 29: add r1.z, -r1.y, cb0[16].x
    r1.z = ((-(r1.yyyy))+(source[16].xxxx)).z;
    // 30: mad r1.y, r1.x, r1.z, r1.y
    r1.y = ((r1.xxxx)*(r1.zzzz)+(r1.yyyy)).y;
    // 31: add r1.z, -r1.y, cb0[16].y
    r1.z = ((-(r1.yyyy))+(source[16].yyyy)).z;
    // 32: mad r1.y, r2.w, r1.z, r1.y
    r1.y = ((r2.wwww)*(r1.zzzz)+(r1.yyyy)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: add r3.xyzw, r3.wxyz, -r5.wxyz
    r3.xyzw = ((r3.wxyz)+(-(r5.wxyz))).xyzw;
    // 37: mad r3.xyzw, r2.yyyy, r3.xyzw, r5.wxyz
    r3.xyzw = ((r2.yyyy)*(r3.xyzw)+(r5.wxyz)).xyzw;
    // 38: add r0.z, -r3.x, l(1.000000)
    r0.z = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 39: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 40: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 41: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 42: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 43: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 45: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 46: add r1.y, -r0.w, cb0[17].w
    r1.y = ((-(r0.wwww))+(source[17].wwww)).y;
    // 47: mad r0.w, r2.w, r1.y, r0.w
    r0.w = ((r2.wwww)*(r1.yyyy)+(r0.wwww)).w;
    // 48: add r1.y, cb0[16].w, -cb0[17].x
    r1.y = ((source[16].wwww)+(-(source[17].xxxx))).y;
    // 49: mad r1.y, r2.x, r1.y, cb0[17].x
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[17].xxxx)).y;
    // 50: add r1.z, -r1.y, cb0[17].y
    r1.z = ((-(r1.yyyy))+(source[17].yyyy)).z;
    // 51: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 52: add r1.z, -r1.y, cb0[17].z
    r1.z = ((-(r1.yyyy))+(source[17].zzzz)).z;
    // 53: mad r1.y, r1.x, r1.z, r1.y
    r1.y = ((r1.xxxx)*(r1.zzzz)+(r1.yyyy)).y;
    // 54: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 55: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 56: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 57: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 58: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 60: mad r6.xyzw, r0.ywyw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r6.xyzw = ((r0.ywyw)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 61: dp2 r0.y, r6.zwzz, r6.zwzz
    r0.y = (dot((r6.zwzz).xy,(r6.zwzz).xy).xxxx).y;
    // 62: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 63: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 64: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 65: add r7.z, r0.y, l(0.000010)
    r7.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 66: mul r7.xy, r6.xyxx, cb0[14].xxxx
    r7.xy = ((r6.xyxx)*(source[14].xxxx)).xy;
    // 67: mad r6.xy, cb0[14].wwww, r6.zwzz, -r7.xyxx
    r6.xy = ((source[14].wwww)*(r6.zwzz)+(-(r7.xyxx))).xy;
    // 68: mov r6.z, l(0)
    r6.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 69: mad r1.yzw, r2.wwww, r6.xxyz, r7.xxyz
    r1.yzw = ((r2.wwww)*(r6.xxyz)+(r7.xxyz)).yzw;
    // 70: add r5.yzw, -r1.yyzw, l(0.000000, 0.000000, 0.000000, 1.000000)
    r5.yzw = ((-(r1.yyzw))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 71: mad r5.yzw, cb0[16].zzzz, r5.yyzw, r1.yyzw
    r5.yzw = ((source[16].zzzz)*(r5.yyzw)+(r1.yyzw)).yzw;
    // 72: dp3 r0.y, r5.yzwy, r5.yzwy
    r0.y = (dot((r5.yzwy).xyz,(r5.yzwy).xyz).xxxx).y;
    // 73: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 74: div r5.yzw, r5.yyzw, r0.yyyy
    r5.yzw = ((r5.yyzw)/(r0.yyyy)).yzw;
    // 75: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 76: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 77: mul r6.xyz, r0.yyyy, v1.xyzx
    r6.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 78: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 79: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 80: mul r7.xyz, r0.yyyy, v0.xyzx
    r7.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 81: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 82: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 83: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 84: dp3 r9.y, r8.xyzx, r5.yzwy
    r9.y = (dot((r8.xyzx).xyz,(r5.yzwy).xyz).xxxx).y;
    // 85: dp3 r9.x, r7.xyzx, r5.yzwy
    r9.x = (dot((r7.xyzx).xyz,(r5.yzwy).xyz).xxxx).x;
    // 86: dp3 r9.z, r6.xyzx, r5.yzwy
    r9.z = (dot((r6.xyzx).xyz,(r5.yzwy).xyz).xxxx).z;
    // 87: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 88: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 89: mul r5.yzw, r0.yyyy, v5.xxyz
    r5.yzw = ((r0.yyyy)*(v5.xxyz)).yzw;
    // 90: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 91: dp3 r11.y, r8.xyzx, r5.yzwy
    r11.y = (dot((r8.xyzx).xyz,(r5.yzwy).xyz).xxxx).y;
    // 92: dp3 r11.x, r7.xyzx, r5.yzwy
    r11.x = (dot((r7.xyzx).xyz,(r5.yzwy).xyz).xxxx).x;
    // 93: dp3 r11.z, r6.xyzx, r5.yzwy
    r11.z = (dot((r6.xyzx).xyz,(r5.yzwy).xyz).xxxx).z;
    // 94: dp3 r0.y, r9.xyzx, r11.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 95: mul r9.xyz, r9.xyzx, r0.yyyy
    r9.xyz = ((r9.xyzx)*(r0.yyyy)).xyz;
    // 96: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 97: mov r9.w, -r9.x
    r9.w = (-(r9.xxxx)).w;
    // 98: dp2 r0.y, r9.ywyy, r9.ywyy
    r0.y = (dot((r9.ywyy).xy,(r9.ywyy).xy).xxxx).y;
    // 99: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 100: div r0.yw, r9.yyyw, r0.yyyy
    r0.yw = ((r9.yyyw)/(r0.yyyy)).yw;
    // 101: mad r2.z, -r9.z, l(0.250000), l(0.250000)
    r2.z = ((-(r9.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 102: add r3.x, r9.z, l(1.000000)
    r3.x = ((r9.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: mul r3.x, r3.x, l(0.500000)
    r3.x = ((r3.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 104: mad r0.yw, r2.zzzz, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r2.zzzz)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 105: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t4.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 106: log r9.xyz, r0.xywx
    r9.xyz = (log2(r0.xywx)).xyz;
    // 107: rcp r2.z, cb0[18].x
    r2.z = (1.0/(source[18].xxxx)).z;
    // 108: mul r11.xyz, r9.xyzx, r2.zzzz
    r11.xyz = ((r9.xyzx)*(r2.zzzz)).xyz;
    // 109: mul r9.xyz, r9.xyzx, cb0[18].xxxx
    r9.xyz = ((r9.xyzx)*(source[18].xxxx)).xyz;
    // 110: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 111: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 112: mul r11.xyz, r2.zzzz, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r11.xyzx)).xyz;
    // 113: mad r9.xyz, r9.xyzx, cb0[18].xxxx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(source[18].xxxx)+(r11.xyzx)).xyz;
    // 114: add r0.xyw, r0.xyxw, r9.xyxz
    r0.xyw = ((r0.xyxw)+(r9.xyxz)).xyw;
    // 115: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 116: add r2.z, cb0[18].x, l(1.000000)
    r2.z = ((source[18].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 117: mul r0.xyw, r0.xyxw, r2.zzzz
    r0.xyw = ((r0.xyxw)*(r2.zzzz)).xyw;
    // 118: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 119: add r9.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r9.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 120: mad r9.xyz, r3.xxxx, r9.xyzx, cb0[8].xyzx
    r9.xyz = ((r3.xxxx)*(r9.xyzx)+(source[8].xyzx)).xyz;
    // 121: mul r0.xyw, r0.xxxx, r9.xyxz
    r0.xyw = ((r0.xxxx)*(r9.xyxz)).xyw;
    // 122: mul r0.xyw, r0.xyxw, cb0[18].yyyy
    r0.xyw = ((r0.xyxw)*(source[18].yyyy)).xyw;
    // 123: add r2.z, r2.y, r2.x
    r2.z = ((r2.yyyy)+(r2.xxxx)).z;
    // 124: add r2.z, r1.x, r2.z
    r2.z = ((r1.xxxx)+(r2.zzzz)).z;
    // 125: add_sat r2.z, r2.w, r2.z
    r2.z = (saturate((r2.wwww)+(r2.zzzz))).z;
    // 126: dp3 r3.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 127: add r9.xyz, -r3.yzwy, r3.xxxx
    r9.xyz = ((-(r3.yzwy))+(r3.xxxx)).xyz;
    // 128: mul r9.xyz, r9.xyzx, cb0[18].zzzz
    r9.xyz = ((r9.xyzx)*(source[18].zzzz)).xyz;
    // 129: mad r3.xyz, r2.zzzz, r9.xyzx, r3.yzwy
    r3.xyz = ((r2.zzzz)*(r9.xyzx)+(r3.yzwy)).xyz;
    // 130: add r9.xyz, -r3.xyzx, r5.xxxx
    r9.xyz = ((-(r3.xyzx))+(r5.xxxx)).xyz;
    // 131: mad r3.xyz, r2.wwww, r9.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 132: max r9.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 133: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 134: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 135: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 136: dp3 r2.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 137: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 138: add r3.w, cb0[18].w, -cb0[19].x
    r3.w = ((source[18].wwww)+(-(source[19].xxxx))).w;
    // 139: mad r3.w, r2.w, r3.w, cb0[19].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[19].xxxx)).w;
    // 140: mul r2.z, r2.z, r3.w
    r2.z = ((r2.zzzz)*(r3.wwww)).z;
    // 141: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 142: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 143: mad r3.w, -r2.z, r2.z, l(1.000000)
    r3.w = ((-(r2.zzzz))*(r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: max r3.w, r3.w, l(0.001000)
    r3.w = (max(r3.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 145: div r3.w, cb0[19].y, r3.w
    r3.w = ((source[19].yyyy)/(r3.wwww)).w;
    // 146: dp3 r4.w, r1.yzwy, r1.yzwy
    r4.w = (dot((r1.yzwy).xyz,(r1.yzwy).xyz).xxxx).w;
    // 147: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 148: div r1.yzw, r1.yyzw, r4.wwww
    r1.yzw = ((r1.yyzw)/(r4.wwww)).yzw;
    // 149: dp3 r4.w, r1.yzwy, r5.yzwy
    r4.w = (dot((r1.yzwy).xyz,(r5.yzwy).xyz).xxxx).w;
    // 150: mov_sat r5.x, r4.w
    r5.x = (saturate(r4.wwww)).x;
    // 151: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: add r5.x, -r5.x, l(1.000000)
    r5.x = ((-(r5.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 153: mov_sat r6.w, r5.w
    r6.w = (saturate(r5.wwww)).w;
    // 154: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: mul r5.x, r5.x, r6.w
    r5.x = ((r5.xxxx)*(r6.wwww)).x;
    // 156: mul r3.w, r3.w, r5.x
    r3.w = ((r3.wwww)*(r5.xxxx)).w;
    // 157: mul r9.xyz, r0.xywx, r3.wwww
    r9.xyz = ((r0.xywx)*(r3.wwww)).xyz;
    // 158: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 159: mad r12.xyz, cb0[4].wwww, cb0[4].xyzx, -r11.xyzx
    r12.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r11.xyzx))).xyz;
    // 160: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 161: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 162: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 163: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 164: mad r11.xyz, r1.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r1.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 165: add r12.xyz, -r11.xyzx, cb0[7].xyzx
    r12.xyz = ((-(r11.xyzx))+(source[7].xyzx)).xyz;
    // 166: mad r11.xyz, r2.wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 167: sample_b_indexable(texture2d)(float,float,float,float) r12.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r12.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 168: add r4.xyz, r4.xyzx, -r12.xyzx
    r4.xyz = ((r4.xyzx)+(-(r12.xyzx))).xyz;
    // 169: mad r2.xyw, r2.yyyy, r4.xyxz, r12.xyxz
    r2.xyw = ((r2.yyyy)*(r4.xyxz)+(r12.xyxz)).xyw;
    // 170: mul r2.xyw, r2.xyxw, r11.xyxz
    r2.xyw = ((r2.xyxw)*(r11.xyxz)).xyw;
    // 171: mul r0.xyw, r0.xyxw, r2.xyxw
    r0.xyw = ((r0.xyxw)*(r2.xyxw)).xyw;
    // 172: mad r3.xyz, r3.xyzx, r9.xyzx, -r0.xywx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)+(-(r0.xywx))).xyz;
    // 173: add r1.x, -r2.z, l(1.000000)
    r1.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 174: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 175: mad r0.xyw, r1.xxxx, r3.xyxz, r0.xyxw
    r0.xyw = ((r1.xxxx)*(r3.xyxz)+(r0.xyxw)).xyw;
    // 176: dp3 r1.x, r10.xyzx, r10.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 177: sqrt r3.x, r1.x
    r3.x = (sqrt(r1.xxxx)).x;
    // 178: div r3.xyz, r10.xyzx, r3.xxxx
    r3.xyz = ((r10.xyzx)/(r3.xxxx)).xyz;
    // 179: dp3 r3.x, r3.xyzx, r5.yzwy
    r3.x = (dot((r3.xyzx).xyz,(r5.yzwy).xyz).xxxx).x;
    // 180: add r3.y, -|r5.w|, l(1.000000)
    r3.y = ((-(abs(r5.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 181: mul r3.y, r4.w, r3.y
    r3.y = ((r4.wwww)*(r3.yyyy)).y;
    // 182: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mul r3.z, |r3.x|, |r3.x|
    r3.z = ((abs(r3.xxxx))*(abs(r3.xxxx))).z;
    // 184: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 185: mul r3.z, r3.z, |r3.x|
    r3.z = ((r3.zzzz)*(abs(r3.xxxx))).z;
    // 186: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 187: movc r3.x, r3.x, l(0), r3.z
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).x;
    // 188: add r3.z, r3.x, l(-0.027778)
    r3.z = ((r3.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 189: mad r3.x, r3.x, r3.z, l(0.027778)
    r3.x = ((r3.xxxx)*(r3.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 190: div_sat r1.x, r3.x, r1.x
    r1.x = (saturate((r3.xxxx)/(r1.xxxx))).x;
    // 191: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 192: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 193: mad r3.xzw, r0.zzzz, r0.xxyw, -r2.xxyw
    r3.xzw = ((r0.zzzz)*(r0.xxyw)+(-(r2.xxyw))).xzw;
    // 194: mad r2.xyz, r2.zzzz, r3.xzwx, r2.xywx
    r2.xyz = ((r2.zzzz)*(r3.xzwx)+(r2.xywx)).xyz;
    // 195: add r0.z, -cb0[2].w, l(1.000000)
    r0.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 196: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 197: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 198: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 199: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 200: mul r1.x, cb0[2].z, l(1.500000)
    r1.x = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 201: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 202: mad r0.z, r0.z, l(0.500000), cb0[2].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 203: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 204: mul r4.x, r1.x, l(0.125000)
    r4.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 205: mul r9.y, cb0[2].y, cb0[13].y
    r9.y = ((source[2].yyyy)*(source[13].yyyy)).y;
    // 206: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 207: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 208: add r3.xz, r4.xxyx, r9.xxyx
    r3.xz = ((r4.xxyx)+(r9.xxyx)).xz;
    // 209: frc r1.x, cb0[2].x
    r1.x = (frac(source[2].xxxx)).x;
    // 210: add r2.w, -r1.x, cb0[2].x
    r2.w = ((-(r1.xxxx))+(source[2].xxxx)).w;
    // 211: mul r9.z, r2.w, l(0.125000)
    r9.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 212: add r3.xz, r3.xxzx, r9.zzwz
    r3.xz = ((r3.xxzx)+(r9.zzwz)).xz;
    // 213: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xzxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 214: mul r3.xzw, r0.zzzz, r4.xxyz
    r3.xzw = ((r0.zzzz)*(r4.xxyz)).xzw;
    // 215: mul r0.z, r1.x, r4.w
    r0.z = ((r1.xxxx)*(r4.wwww)).z;
    // 216: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 217: mad r3.xzw, r3.xxzw, l(2.000000, 0.000000, 2.000000, 2.000000), -r2.xxyz
    r3.xzw = ((r3.xxzw)*(float4(2.000000,0.000000,2.000000,2.000000))+(-(r2.xxyz))).xzw;
    // 218: mad r2.xyz, r0.zzzz, r3.xzwx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r3.xzwx)+(r2.xyzx)).xyz;
    // 219: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 220: mad r3.xzw, r5.xxxx, cb0[11].xxyz, -cb0[11].xxyz
    r3.xzw = ((r5.xxxx)*(source[11].xxyz)+(-(source[11].xxyz))).xzw;
    // 221: mul r0.z, r5.x, cb0[10].w
    r0.z = ((r5.xxxx)*(source[10].wwww)).z;
    // 222: mad r3.xzw, cb0[11].wwww, r3.xxzw, cb0[11].xxyz
    r3.xzw = ((source[11].wwww)*(r3.xxzw)+(source[11].xxyz)).xzw;
    // 223: mad r3.xzw, r0.zzzz, cb0[10].xxyz, r3.xxzw
    r3.xzw = ((r0.zzzz)*(source[10].xxyz)+(r3.xxzw)).xzw;
    // 224: mad r0.xyz, r1.xxxx, r0.xywx, r3.xzwx
    r0.xyz = ((r1.xxxx)*(r0.xywx)+(r3.xzwx)).xyz;
    // 225: log r0.w, |r3.y|
    r0.w = (log2(abs(r3.yyyy))).w;
    // 226: lt r1.x, |r3.y|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 227: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 228: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 229: mul r3.xyz, r0.wwww, cb0[12].xyzx
    r3.xyz = ((r0.wwww)*(source[12].xyzx)).xyz;
    // 230: movc r3.xyz, r1.xxxx, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 231: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 232: add r0.xyz, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 233: dp3 r0.w, r1.yzwy, r1.yzwy
    r0.w = (dot((r1.yzwy).xyz,(r1.yzwy).xyz).xxxx).w;
    // 234: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 235: mul r1.xyz, r0.wwww, r1.yzwy
    r1.xyz = ((r0.wwww)*(r1.yzwy)).xyz;
    // 236: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 237: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 238: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 239: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 240: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 241: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 242: mul r3.yzw, r3.yyyy, cb0[21].xxyz
    r3.yzw = ((r3.yyyy)*(source[21].xxyz)).yzw;
    // 243: mad r3.xyz, r3.xxxx, cb0[20].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[20].xyzx)+(r3.yzwy)).xyz;
    // 244: mul r3.xyz, r3.xyzx, cb0[22].wwww
    r3.xyz = ((r3.xyzx)*(source[22].wwww)).xyz;
    // 245: mad r0.xyz, r3.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 246: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 247: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 248: mad o0.xyz, r2.xyzx, cb0[22].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[22].xyzx)+(r0.xyzx)).xyz;
    // 249: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 250: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 251: dp3 r0.x, r7.xyzx, r1.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 252: dp3 r0.z, r6.xyzx, r1.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 253: dp3 r0.y, r8.xyzx, r1.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 254: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 255: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 256: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 257: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 258: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 259: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 260: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 261: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 262: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 263: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 264: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 265: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 266: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 267: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 268: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 269: ret
    return output;
}

// source.character.mokoko-av036-914.v1 / source program ebe62e3e9e16c1499f30b7abcd438f01
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase914(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[25].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[25].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 16: add r0.z, -cb0[21].y, cb0[21].x
    r0.z = ((-(source[21].yyyy))+(source[21].xxxx)).z;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mad r0.z, r1.x, r0.z, cb0[21].y
    r0.z = ((r1.xxxx)*(r0.zzzz)+(source[21].yyyy)).z;
    // 19: add r0.w, -r0.z, cb0[21].z
    r0.w = ((-(r0.zzzz))+(source[21].zzzz)).w;
    // 20: mad r0.z, r1.y, r0.w, r0.z
    r0.z = ((r1.yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 21: add r0.w, -r0.z, cb0[21].w
    r0.w = ((-(r0.zzzz))+(source[21].wwww)).w;
    // 22: mad r0.z, r1.z, r0.w, r0.z
    r0.z = ((r1.zzzz)*(r0.wwww)+(r0.zzzz)).z;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
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
    // 32: add r2.w, -cb0[20].y, cb0[20].x
    r2.w = ((-(source[20].yyyy))+(source[20].xxxx)).w;
    // 33: mad r2.w, r1.x, r2.w, cb0[20].y
    r2.w = ((r1.xxxx)*(r2.wwww)+(source[20].yyyy)).w;
    // 34: add r3.x, -r2.w, cb0[20].z
    r3.x = ((-(r2.wwww))+(source[20].zzzz)).x;
    // 35: mad r2.w, r1.y, r3.x, r2.w
    r2.w = ((r1.yyyy)*(r3.xxxx)+(r2.wwww)).w;
    // 36: add r3.x, -r2.w, cb0[20].w
    r3.x = ((-(r2.wwww))+(source[20].wwww)).x;
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
    // 46: mul r3.xy, r0.ywyy, cb0[19].xxxx
    r3.xy = ((r0.ywyy)*(source[19].xxxx)).xy;
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
    // 52: mad r4.xyz, cb0[19].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[19].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
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
    // 86: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t4.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 87: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 88: rcp r2.w, cb0[22].x
    r2.w = (1.0/(source[22].xxxx)).w;
    // 89: mul r10.xyz, r6.xyzx, r2.wwww
    r10.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 90: mul r6.xyz, r6.xyzx, cb0[22].xxxx
    r6.xyz = ((r6.xyzx)*(source[22].xxxx)).xyz;
    // 91: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 92: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 93: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 94: mad r6.xyz, r6.xyzx, cb0[22].xxxx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[22].xxxx)+(r10.xyzx)).xyz;
    // 95: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 96: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 97: add r2.w, cb0[22].x, l(1.000000)
    r2.w = ((source[22].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r0.xyw, r0.xyxw, r2.wwww
    r0.xyw = ((r0.xyxw)*(r2.wwww)).xyw;
    // 99: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 100: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 101: mad r6.xyz, r3.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 102: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 103: mul r0.xyw, r0.xyxw, cb0[22].yyyy
    r0.xyw = ((r0.xyxw)*(source[22].yyyy)).xyw;
    // 104: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r6.xyz, -r2.xyzx, r2.wwww
    r6.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 106: mad r2.xyz, cb0[19].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 107: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 108: add r6.xyz, -r2.xyzx, r2.wwww
    r6.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 109: mad r2.xyz, cb0[19].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 110: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 111: add r6.xyz, -r2.xyzx, r2.wwww
    r6.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 112: mul r6.xyz, r6.xyzx, cb0[22].zzzz
    r6.xyz = ((r6.xyzx)*(source[22].zzzz)).xyz;
    // 113: add r2.w, r1.y, r1.x
    r2.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 114: add r2.w, r1.z, r2.w
    r2.w = ((r1.zzzz)+(r2.wwww)).w;
    // 115: add_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)+(r2.wwww))).w;
    // 116: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 117: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 118: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 119: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 120: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 121: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 123: mul r1.w, r1.w, cb0[23].z
    r1.w = ((r1.wwww)*(source[23].zzzz)).w;
    // 124: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 125: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 128: div r2.w, cb0[23].w, r2.w
    r2.w = ((source[23].wwww)/(r2.wwww)).w;
    // 129: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 130: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 131: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 132: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 133: mul_sat r4.w, r3.w, cb0[22].w
    r4.w = (saturate((r3.wwww)*(source[22].wwww))).w;
    // 134: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mul_sat r5.w, r4.z, cb0[22].w
    r5.w = (saturate((r4.zzzz)*(source[22].wwww))).w;
    // 137: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: add_sat r5.w, r5.w, -cb0[23].x
    r5.w = (saturate((r5.wwww)+(-(source[23].xxxx)))).w;
    // 139: log r6.x, r5.w
    r6.x = (log2(r5.wwww)).x;
    // 140: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 141: mul r6.x, r6.x, cb0[23].y
    r6.x = ((r6.xxxx)*(source[23].yyyy)).x;
    // 142: exp r6.x, r6.x
    r6.x = (exp2(r6.xxxx)).x;
    // 143: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 144: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 145: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 146: mul r6.xyz, r0.xywx, r2.wwww
    r6.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 147: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 148: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r10.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r10.xyzx))).xyz;
    // 149: mad r10.xyz, r1.xxxx, r11.xyzx, r10.xyzx
    r10.xyz = ((r1.xxxx)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 150: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, -r10.xyzx
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r10.xyzx))).xyz;
    // 151: mad r10.xyz, r1.yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((r1.yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 152: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r10.xyzx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r10.xyzx))).xyz;
    // 153: mad r1.xyz, r1.zzzz, r11.xyzx, r10.xyzx
    r1.xyz = ((r1.zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 154: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r10.xyz, -r1.xyzx, r2.wwww
    r10.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 156: mad r1.xyz, cb0[19].yyyy, r10.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 157: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 158: add r10.xyz, -r1.xyzx, r2.wwww
    r10.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 159: mad r1.xyz, cb0[19].zzzz, r10.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 160: mad r10.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: mad r11.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 162: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 163: mul r1.xyz, r1.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 164: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 165: dp3 r2.w, r11.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 166: add r12.xyz, -r11.yzwy, r2.wwww
    r12.xyz = ((-(r11.yzwy))+(r2.wwww)).xyz;
    // 167: mad r11.yzw, cb0[19].yyyy, r12.xxyz, r11.yyzw
    r11.yzw = ((source[19].yyyy)*(r12.xxyz)+(r11.yyzw)).yzw;
    // 168: mov_sat r11.x, r11.x
    r11.x = (saturate(r11.xxxx)).x;
    // 169: mul_sat r2.w, r11.x, cb0[25].z
    r2.w = (saturate((r11.xxxx)*(source[25].zzzz))).w;
    // 170: mul o0.w, r2.w, cb0[1].w
    output.targets[0].w = ((r2.wwww)*(source[1].wwww)).w;
    // 171: dp3 r2.w, r11.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r12.xyz, -r11.yzwy, r2.wwww
    r12.xyz = ((-(r11.yzwy))+(r2.wwww)).xyz;
    // 173: mad r11.xyz, cb0[19].zzzz, r12.xyzx, r11.yzwy
    r11.xyz = ((source[19].zzzz)*(r12.xyzx)+(r11.yzwy)).xyz;
    // 174: mul r1.xyz, r1.xyzx, r11.xyzx
    r1.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 175: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 176: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 177: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: mul r2.w, r2.w, cb0[24].x
    r2.w = ((r2.wwww)*(source[24].xxxx)).w;
    // 179: mad r0.xyw, r2.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 180: dp3 r2.x, r9.xyzx, r9.xyzx
    r2.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 181: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 182: div r2.yzw, r9.xxyz, r2.yyyy
    r2.yzw = ((r9.xxyz)/(r2.yyyy)).yzw;
    // 183: dp3 r2.y, r2.yzwy, r4.xyzx
    r2.y = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).y;
    // 184: add r2.z, -|r4.z|, l(1.000000)
    r2.z = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 185: mul r2.z, r3.w, r2.z
    r2.z = ((r3.wwww)*(r2.zzzz)).z;
    // 186: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 187: mul r2.w, |r2.y|, |r2.y|
    r2.w = ((abs(r2.yyyy))*(abs(r2.yyyy))).w;
    // 188: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 189: mul r2.w, r2.w, |r2.y|
    r2.w = ((r2.wwww)*(abs(r2.yyyy))).w;
    // 190: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 191: movc r2.y, r2.y, l(0), r2.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 192: add r2.w, r2.y, l(-0.027778)
    r2.w = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 193: mad r2.y, r2.y, r2.w, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 194: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 195: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 196: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 197: mad r2.xyw, r0.zzzz, r0.xyxw, -r1.xyxz
    r2.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r1.xyxz))).xyw;
    // 198: mad r1.xyz, r1.wwww, r2.xywx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xywx)+(r1.xyzx)).xyz;
    // 199: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 200: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 201: mad r1.xyz, cb0[19].yyyy, r2.xywx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r2.xywx)+(r1.xyzx)).xyz;
    // 202: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 203: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 204: mad r1.xyz, cb0[19].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 205: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 206: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: mul r0.z, r0.z, cb0[24].y
    r0.z = ((r0.zzzz)*(source[24].yyyy)).z;
    // 208: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 209: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 210: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 211: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 212: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 213: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 214: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 215: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 216: mul r6.y, cb0[3].y, cb0[15].y
    r6.y = ((source[3].yyyy)*(source[15].yyyy)).y;
    // 217: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 218: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 219: add r2.xy, r2.xyxx, r6.xyxx
    r2.xy = ((r2.xyxx)+(r6.xyxx)).xy;
    // 220: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 221: add r2.w, -r1.w, cb0[3].x
    r2.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 222: mul r6.z, r2.w, l(0.125000)
    r6.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 223: add r2.xy, r2.xyxx, r6.zwzz
    r2.xy = ((r2.xyxx)+(r6.zwzz)).xy;
    // 224: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 225: mul r2.xyw, r0.zzzz, r6.xyxz
    r2.xyw = ((r0.zzzz)*(r6.xyxz)).xyw;
    // 226: mul r0.z, r1.w, r6.w
    r0.z = ((r1.wwww)*(r6.wwww)).z;
    // 227: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 228: mad r2.xyw, r2.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxz
    r2.xyw = ((r2.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxz))).xyw;
    // 229: mad r1.xyz, r0.zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 230: add r6.xyzw, v8.yzxy, cb0[0].yzxy
    r6.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 231: add r6.xyzw, r6.xyzw, -cb0[1].yzxy
    r6.xyzw = ((r6.xyzw)+(-(source[1].yzxy))).xyzw;
    // 232: add r2.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 233: add r2.xy, -r6.zwzz, r2.xyxx
    r2.xy = ((-(r6.zwzz))+(r2.xyxx)).xy;
    // 234: mad r2.xy, cb0[16].wwww, r2.xyxx, r6.zwzz
    r2.xy = ((source[16].wwww)*(r2.xyxx)+(r6.zwzz)).xy;
    // 235: mul r0.z, cb0[16].y, cb0[24].y
    r0.z = ((source[16].yyyy)*(source[24].yyyy)).z;
    // 236: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 237: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 238: mul r4.y, r0.z, l(0.020000)
    r4.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 239: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 240: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 241: mul r2.w, cb0[16].x, l(0.001000)
    r2.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 242: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 243: mad r2.xy, r2.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 244: dp2 r2.w, cb0[17].xyxx, r2.xyxx
    r2.w = (dot((source[17].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 245: dp2 r2.y, cb0[18].xyxx, r2.xyxx
    r2.y = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 246: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 247: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 248: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 249: mad r2.xyw, r6.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r6.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 250: mul r3.w, r6.w, l(0.900000)
    r3.w = ((r6.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 251: mad r2.xyw, r3.wwww, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.wwww)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 252: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 253: mad r4.xyz, cb0[16].zzzz, r2.xywx, -r1.xyzx
    r4.xyz = ((source[16].zzzz)*(r2.xywx)+(-(r1.xyzx))).xyz;
    // 254: mul r2.xyw, r2.xyxw, cb0[16].zzzz
    r2.xyw = ((r2.xyxw)*(source[16].zzzz)).xyw;
    // 255: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 256: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 257: mad r1.xyz, r0.zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 258: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 259: mul r2.xyw, r0.xyxw, r1.wwww
    r2.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 260: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 261: mad r0.xyz, -r1.wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(r1.wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 262: mad r0.xyz, cb0[19].yyyy, r0.xyzx, r2.xywx
    r0.xyz = ((source[19].yyyy)*(r0.xyzx)+(r2.xywx)).xyz;
    // 263: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 264: add r2.xyw, -r0.xyxz, r0.wwww
    r2.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 265: mad r0.xyz, cb0[19].zzzz, r2.xywx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r2.xywx)+(r0.xyzx)).xyz;
    // 266: mad r2.xyw, r4.wwww, cb0[13].xyxz, -cb0[13].xyxz
    r2.xyw = ((r4.wwww)*(source[13].xyxz)+(-(source[13].xyxz))).xyw;
    // 267: mul r0.w, r4.w, cb0[12].w
    r0.w = ((r4.wwww)*(source[12].wwww)).w;
    // 268: mad r2.xyw, cb0[13].wwww, r2.xyxw, cb0[13].xyxz
    r2.xyw = ((source[13].wwww)*(r2.xyxw)+(source[13].xyxz)).xyw;
    // 269: mad r2.xyw, r0.wwww, cb0[12].xyxz, r2.xyxw
    r2.xyw = ((r0.wwww)*(source[12].xyxz)+(r2.xyxw)).xyw;
    // 270: mad r0.xyz, r0.xyzx, r10.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)+(r2.xywx)).xyz;
    // 271: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 272: lt r1.w, |r2.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 273: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 274: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 275: mul r2.xyz, r0.wwww, cb0[14].xyzx
    r2.xyz = ((r0.wwww)*(source[14].xyzx)).xyz;
    // 276: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 277: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 278: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 279: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 280: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 281: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 282: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 283: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 284: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 285: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 286: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 287: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 288: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 289: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 290: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 291: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 292: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 293: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 294: mad r0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 295: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 296: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
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

// source.character.mokoko-av036-915.v1 / source program 6258f3c9d3378142a00ea8580cb964b9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase915(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].w=(g_SourceCharacterTime.xxxx).x;
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
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 73: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t4.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
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
    // 87: add r5.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r5.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 88: mad r5.xyz, r2.wwww, r5.xyzx, cb0[9].xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(source[9].xyzx)).xyz;
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
    // 116: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 136: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 138: mad r10.xyz, cb0[18].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[18].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 139: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: add r11.xyz, -r10.xyzx, r4.wwww
    r11.xyz = ((-(r10.xyzx))+(r4.wwww)).xyz;
    // 141: mad r10.xyz, cb0[18].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[18].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 142: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 143: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 144: mad r11.xyz, r9.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r9.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 145: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 146: mad r9.xyw, r9.yyyy, r12.xyxz, r11.xyxz
    r9.xyw = ((r9.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 147: dp3 r4.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 148: add r11.xyz, -r9.xywx, r4.wwww
    r11.xyz = ((-(r9.xywx))+(r4.wwww)).xyz;
    // 149: mad r9.xyw, cb0[18].yyyy, r11.xyxz, r9.xyxw
    r9.xyw = ((source[18].yyyy)*(r11.xyxz)+(r9.xyxw)).xyw;
    // 150: dp3 r4.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 151: add r11.xyz, -r9.xywx, r4.wwww
    r11.xyz = ((-(r9.xywx))+(r4.wwww)).xyz;
    // 152: mad r9.xyw, cb0[18].zzzz, r11.xyxz, r9.xyxw
    r9.xyw = ((source[18].zzzz)*(r11.xyxz)+(r9.xyxw)).xyw;
    // 153: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 154: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 156: mul r9.xyw, r9.xyxw, r11.xyxz
    r9.xyw = ((r9.xyxw)*(r11.xyxz)).xyw;
    // 157: mul r9.xyw, r10.xyxz, r9.xyxw
    r9.xyw = ((r10.xyxz)*(r9.xyxw)).xyw;
    // 158: mul r0.xyw, r0.xyxw, r9.xyxw
    r0.xyw = ((r0.xyxw)*(r9.xyxw)).xyw;
    // 159: mad r1.xyz, r1.xyzx, r5.xyzx, -r0.xywx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(-(r0.xywx))).xyz;
    // 160: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul r4.w, r4.w, cb0[21].z
    r4.w = ((r4.wwww)*(source[21].zzzz)).w;
    // 162: mad r0.xyw, r4.wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 163: dp3 r1.x, r8.xyzx, r8.xyzx
    r1.x = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 164: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 165: div r5.xyz, r8.xyzx, r1.yyyy
    r5.xyz = ((r8.xyzx)/(r1.yyyy)).xyz;
    // 166: dp3 r1.y, r5.xyzx, r3.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 167: add r1.z, -|r3.z|, l(1.000000)
    r1.z = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 168: mul r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)*(r1.zzzz)).z;
    // 169: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 170: mul r1.w, |r1.y|, |r1.y|
    r1.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 171: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 172: mul r1.w, r1.w, |r1.y|
    r1.w = ((r1.wwww)*(abs(r1.yyyy))).w;
    // 173: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 174: movc r1.y, r1.y, l(0), r1.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 175: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 176: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 177: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 178: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 179: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 180: mad r1.xyw, r0.zzzz, r0.xyxw, -r9.xyxw
    r1.xyw = ((r0.zzzz)*(r0.xyxw)+(-(r9.xyxw))).xyw;
    // 181: mad r1.xyw, r3.wwww, r1.xyxw, r9.xyxw
    r1.xyw = ((r3.wwww)*(r1.xyxw)+(r9.xyxw)).xyw;
    // 182: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r3.xyz, -r1.xywx, r0.zzzz
    r3.xyz = ((-(r1.xywx))+(r0.zzzz)).xyz;
    // 184: mad r1.xyw, cb0[18].yyyy, r3.xyxz, r1.xyxw
    r1.xyw = ((source[18].yyyy)*(r3.xyxz)+(r1.xyxw)).xyw;
    // 185: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 186: add r3.xyz, -r1.xywx, r0.zzzz
    r3.xyz = ((-(r1.xywx))+(r0.zzzz)).xyz;
    // 187: mad r1.xyw, cb0[18].zzzz, r3.xyxz, r1.xyxw
    r1.xyw = ((source[18].zzzz)*(r3.xyxz)+(r1.xyxw)).xyw;
    // 188: mul r1.xyw, r11.xyxz, r1.xyxw
    r1.xyw = ((r11.xyxz)*(r1.xyxw)).xyw;
    // 189: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 190: mul r0.z, r0.z, cb0[21].w
    r0.z = ((r0.zzzz)*(source[21].wwww)).z;
    // 191: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 192: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 193: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 194: mul r3.x, cb0[3].z, l(1.500000)
    r3.x = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 195: mul r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)*(r3.xxxx)).z;
    // 196: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 197: frc r3.x, v4.x
    r3.x = (frac(v4.xxxx)).x;
    // 198: mul r3.x, r3.x, l(0.125000)
    r3.x = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 199: mul r5.y, cb0[3].y, cb0[14].y
    r5.y = ((source[3].yyyy)*(source[14].yyyy)).y;
    // 200: mov r3.y, v4.y
    r3.y = (v4.yyyy).y;
    // 201: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 202: add r3.xy, r3.xyxx, r5.xyxx
    r3.xy = ((r3.xyxx)+(r5.xyxx)).xy;
    // 203: frc r3.z, cb0[3].x
    r3.z = (frac(source[3].xxxx)).z;
    // 204: add r3.w, -r3.z, cb0[3].x
    r3.w = ((-(r3.zzzz))+(source[3].xxxx)).w;
    // 205: mul r5.z, r3.w, l(0.125000)
    r5.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 206: add r3.xy, r3.xyxx, r5.zwzz
    r3.xy = ((r3.xyxx)+(r5.zwzz)).xy;
    // 207: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r3.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 208: mul r3.xyw, r0.zzzz, r5.xyxz
    r3.xyw = ((r0.zzzz)*(r5.xyxz)).xyw;
    // 209: mul r0.z, r3.z, r5.w
    r0.z = ((r3.zzzz)*(r5.wwww)).z;
    // 210: add r3.z, -r3.z, l(1.000000)
    r3.z = ((-(r3.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 211: mad r3.xyw, r3.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxw
    r3.xyw = ((r3.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxw))).xyw;
    // 212: mad r1.xyw, r0.zzzz, r3.xyxw, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r3.xyxw)+(r1.xyxw)).xyw;
    // 213: add r5.xyzw, v7.yzxy, cb0[0].yzxy
    r5.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 214: add r5.xyzw, r5.xyzw, -cb0[1].yzxy
    r5.xyzw = ((r5.xyzw)+(-(source[1].yzxy))).xyzw;
    // 215: add r3.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 216: add r3.xy, -r5.zwzz, r3.xyxx
    r3.xy = ((-(r5.zwzz))+(r3.xyxx)).xy;
    // 217: mad r3.xy, cb0[15].wwww, r3.xyxx, r5.zwzz
    r3.xy = ((source[15].wwww)*(r3.xyxx)+(r5.zwzz)).xy;
    // 218: mul r0.z, cb0[15].y, cb0[21].w
    r0.z = ((source[15].yyyy)*(source[21].wwww)).z;
    // 219: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 220: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 221: mul r5.y, r0.z, l(0.020000)
    r5.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 222: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 223: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 224: mul r3.w, cb0[15].x, l(0.001000)
    r3.w = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 225: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 226: mad r3.xy, r3.wwww, r3.xyxx, r5.xyxx
    r3.xy = ((r3.wwww)*(r3.xyxx)+(r5.xyxx)).xy;
    // 227: dp2 r3.w, cb0[16].xyxx, r3.xyxx
    r3.w = (dot((source[16].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 228: dp2 r3.y, cb0[17].xyxx, r3.xyxx
    r3.y = (dot((source[17].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 229: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 230: mul r3.x, r3.w, l(0.125000)
    r3.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 231: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r3.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 232: mad r3.xyw, r5.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxw
    r3.xyw = ((r5.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxw))).xyw;
    // 233: mul r4.w, r5.w, l(0.900000)
    r4.w = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 234: mad r3.xyw, r4.wwww, r3.xyxw, r1.xyxw
    r3.xyw = ((r4.wwww)*(r3.xyxw)+(r1.xyxw)).xyw;
    // 235: mul_sat r3.xyw, r0.zzzz, r3.xyxw
    r3.xyw = (saturate((r0.zzzz)*(r3.xyxw))).xyw;
    // 236: mad r5.xyz, cb0[15].zzzz, r3.xywx, -r1.xywx
    r5.xyz = ((source[15].zzzz)*(r3.xywx)+(-(r1.xywx))).xyz;
    // 237: mul r3.xyw, r3.xyxw, cb0[15].zzzz
    r3.xyw = ((r3.xyxw)*(source[15].zzzz)).xyw;
    // 238: dp3 r0.z, r3.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 239: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 240: mad r1.xyw, r0.zzzz, r5.xyxz, r1.xyxw
    r1.xyw = ((r0.zzzz)*(r5.xyxz)+(r1.xyxw)).xyw;
    // 241: mad r1.xyw, r1.xyxw, cb2[3].wwww, cb2[3].xyxz
    r1.xyw = ((r1.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 242: add r0.z, r2.w, -r9.z
    r0.z = ((r2.wwww)+(-(r9.zzzz))).z;
    // 243: mad r3.xyw, r2.wwww, cb0[12].xyxz, -cb0[12].xyxz
    r3.xyw = ((r2.wwww)*(source[12].xyxz)+(-(source[12].xyxz))).xyw;
    // 244: mad r3.xyw, cb0[12].wwww, r3.xyxw, cb0[12].xyxz
    r3.xyw = ((source[12].wwww)*(r3.xyxw)+(source[12].xyxz)).xyw;
    // 245: mad r0.z, cb0[11].w, r0.z, r9.z
    r0.z = ((source[11].wwww)*(r0.zzzz)+(r9.zzzz)).z;
    // 246: mad r3.xyw, r0.zzzz, cb0[11].xyxz, r3.xyxw
    r3.xyw = ((r0.zzzz)*(source[11].xyxz)+(r3.xyxw)).xyw;
    // 247: mul r5.xyz, r0.xywx, r3.zzzz
    r5.xyz = ((r0.xywx)*(r3.zzzz)).xyz;
    // 248: dp3 r0.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 249: mad r0.xyz, -r3.zzzz, r0.xywx, r0.zzzz
    r0.xyz = ((-(r3.zzzz))*(r0.xywx)+(r0.zzzz)).xyz;
    // 250: mad r0.xyz, cb0[18].yyyy, r0.xyzx, r5.xyzx
    r0.xyz = ((source[18].yyyy)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 251: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 252: add r5.xyz, -r0.xyzx, r0.wwww
    r5.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 253: mad r0.xyz, cb0[18].zzzz, r5.xyzx, r0.xyzx
    r0.xyz = ((source[18].zzzz)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 254: mad r0.xyz, r0.xyzx, r11.xyzx, r3.xywx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r3.xywx)).xyz;
    // 255: log r0.w, |r1.z|
    r0.w = (log2(abs(r1.zzzz))).w;
    // 256: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 257: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 258: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 259: mul r3.xyz, r0.wwww, cb0[13].xyzx
    r3.xyz = ((r0.wwww)*(source[13].xyzx)).xyz;
    // 260: movc r3.xyz, r1.zzzz, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 261: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 262: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 263: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 264: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 265: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 266: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 267: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 268: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 269: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 270: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 271: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 272: mul r3.yzw, r3.yyyy, cb0[23].xxyz
    r3.yzw = ((r3.yyyy)*(source[23].xxyz)).yzw;
    // 273: mad r3.xyz, r3.xxxx, cb0[22].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[22].xyzx)+(r3.yzwy)).xyz;
    // 274: mul r3.xyz, r3.xyzx, cb0[24].wwww
    r3.xyz = ((r3.xyzx)*(source[24].wwww)).xyz;
    // 275: mad r0.xyz, r3.xyzx, r1.xywx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xywx)+(r0.xyzx)).xyz;
    // 276: mul r3.xyz, r1.xywx, r3.xyzx
    r3.xyz = ((r1.xywx)*(r3.xyzx)).xyz;
    // 277: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 278: mad o0.xyz, r1.xywx, cb0[24].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xywx)*(source[24].xyzx)+(r0.xyzx)).xyz;
    // 279: mov o3.xyz, r1.xywx
    output.targets[3].xyz = (r1.xywx).xyz;
    // 280: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 281: dp3 r0.x, r4.xyzx, r2.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 282: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 283: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 284: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 285: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 286: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 287: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 288: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 289: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 290: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 291: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 292: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 293: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 294: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 296: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 297: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 298: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 299: ret
    return output;
}

// source.character.static-map-native-1100.v1 / source program 247fd3b3462a9548b081b5174f7a2f8a
#else // SOURCE_CHARACTER_BASE_DISPATCH_CASES
    case 901u: return SourceCharacterBase901(input);
    case 902u: return SourceCharacterBase902(input);
    case 903u: return SourceCharacterBase903(input);
    case 904u: return SourceCharacterBase904(input);
    case 905u: return SourceCharacterBase905(input);
    case 906u: return SourceCharacterBase906(input);
    case 907u: return SourceCharacterBase907(input);
    case 908u: return SourceCharacterBase908(input);
    case 909u: return SourceCharacterBase909(input);
    case 910u: return SourceCharacterBase910(input);
    case 911u: return SourceCharacterBase911(input);
    case 912u: return SourceCharacterBase912(input);
    case 913u: return SourceCharacterBase913(input);
    case 914u: return SourceCharacterBase914(input);
    case 915u: return SourceCharacterBase915(input);
#endif // SOURCE_CHARACTER_BASE_DISPATCH_CASES
