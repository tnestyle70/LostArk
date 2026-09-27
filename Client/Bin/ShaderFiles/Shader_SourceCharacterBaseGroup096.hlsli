#ifndef SOURCE_CHARACTER_BASE_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase96(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
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
    // 7: add r0.x, -cb0[8].w, l(1.000000)
    r0.x = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
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
    // 17: mul r2.y, cb0[8].y, cb0[16].y
    r2.y = ((source[8].yyyy)*(source[16].yyyy)).y;
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
    // 30: mad r2.xyz, cb0[20].xxxx, r2.xyzx, r0.yzwy
    r2.xyz = ((source[20].xxxx)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[20].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 43: mul r1.z, r6.y, cb0[18].y
    r1.z = ((r6.yyyy)*(source[18].yyyy)).z;
    // 44: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 45: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: movc r1.z, r5.y, l(0), r1.z
    r1.z = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 47: mad r3.xyz, r1.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
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
    // 54: mad r4.xyz, r1.zzzz, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 57: mad r3.xyz, r7.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r7.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 58: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
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
    // 67: mul r4.xyz, cb0[7].xyzx, cb0[7].wwww
    r4.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
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
    // 79: mad r4.xyz, cb0[20].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 80: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 81: dp3 r1.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 82: add r3.xyz, -r4.xyzx, r1.zzzz
    r3.xyz = ((-(r4.xyzx))+(r1.zzzz)).xyz;
    // 83: mad r3.xyz, cb0[20].yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((source[20].yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 84: mad r4.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mad r9.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
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
    // 91: mad r2.xyz, cb0[20].xxxx, r2.xyzx, r9.xyzx
    r2.xyz = ((source[20].xxxx)*(r2.xyzx)+(r9.xyzx)).xyz;
    // 92: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 93: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 94: mad r2.xyz, cb0[20].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 95: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 96: mad r1.xyz, r1.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 97: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 98: mul r0.x, r6.x, cb0[21].y
    r0.x = ((r6.xxxx)*(source[21].yyyy)).x;
    // 99: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 100: movc r0.x, r5.x, l(0), r0.x
    r0.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 101: add_sat r0.x, r0.x, cb0[21].z
    r0.x = (saturate((r0.xxxx)+(source[21].zzzz))).x;
    // 102: add r1.w, -r0.x, l(1.000000)
    r1.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r2.xyz, r1.wwww, cb0[15].xyzx
    r2.xyz = ((r1.wwww)*(source[15].xyzx)).xyz;
    // 104: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 105: mad r1.xyz, -r2.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r2.xyzx))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 106: mad r1.xyz, r0.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 107: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
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
    // 117: mov_sat r1.w, cb0[22].y
    r1.w = (saturate(source[22].yyyy)).w;
    // 118: mad r3.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r3.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 119: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 120: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 121: mad r3.xyz, r8.wwww, r3.xyzx, r2.wwww
    r3.xyz = ((r8.wwww)*(r3.xyzx)+(r2.wwww)).xyz;
    // 122: add r1.w, -cb0[23].y, cb0[23].x
    r1.w = ((-(source[23].yyyy))+(source[23].xxxx)).w;
    // 123: mad r1.w, r7.x, r1.w, cb0[23].y
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[23].yyyy)).w;
    // 124: add r2.w, -r1.w, cb0[23].w
    r2.w = ((-(r1.wwww))+(source[23].wwww)).w;
    // 125: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 126: add r2.w, -r1.w, cb0[24].y
    r2.w = ((-(r1.wwww))+(source[24].yyyy)).w;
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
    // 132: max r1.w, r1.w, cb0[1].x
    r1.w = (max(r1.wwww,source[1].xxxx)).w;
    // 133: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 135: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 136: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 137: mul r5.xy, r5.xyxx, cb0[18].xxxx
    r5.xy = ((r5.xyxx)*(source[18].xxxx)).xy;
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
    // 171: mul r1.w, r1.w, cb0[2].y
    r1.w = ((r1.wwww)*(source[2].yyyy)).w;
    // 172: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 173: mad_sat r1.w, r1.w, cb0[2].w, cb0[2].z
    r1.w = (saturate((r1.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 174: mul r1.w, r1.w, cb0[24].z
    r1.w = ((r1.wwww)*(source[24].zzzz)).w;
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
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r11.yyyy)).w;
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
    // 205: dp2 r14.z, r16.xyxx, cb0[26].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 206: mul r8.xz, cb0[26].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[26].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 207: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 208: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 209: dp2 r17.z, r15.xyxx, cb0[26].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 210: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 211: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 212: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 213: dp4 r13.x, cb0[27].xyzw, r14.xyzw
    r13.x = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 214: dp4 r13.y, cb0[28].xyzw, r14.xyzw
    r13.y = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 215: dp4 r13.z, cb0[29].xyzw, r14.xyzw
    r13.z = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 216: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 217: dp4 r18.x, cb0[30].xyzw, r15.xyzw
    r18.x = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 218: dp4 r18.y, cb0[31].xyzw, r15.xyzw
    r18.y = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 219: dp4 r18.z, cb0[32].xyzw, r15.xyzw
    r18.z = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 220: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 221: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 222: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 223: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 224: mad r13.xyz, cb0[33].xyzx, r5.wwww, r13.xyzx
    r13.xyz = ((source[33].xyzx)*(r5.wwww)+(r13.xyzx)).xyz;
    // 225: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 226: mul r13.xyz, r13.xyzx, cb0[25].xyzx
    r13.xyz = ((r13.xyzx)*(source[25].xyzx)).xyz;
    // 227: mul r13.xyz, r13.xyzx, cb0[26].zzzz
    r13.xyz = ((r13.xyzx)*(source[26].zzzz)).xyz;
    // 228: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[25].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[25].wwww)).xyz;
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
    // 256: mul r8.xyz, r8.xyzx, cb0[25].xyzx
    r8.xyz = ((r8.xyzx)*(source[25].xyzx)).xyz;
    // 257: mul r8.xyz, r8.xyzx, cb0[26].zzzz
    r8.xyz = ((r8.xyzx)*(source[26].zzzz)).xyz;
    // 258: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[25].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[25].wwww)).xyz;
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
    // 290: mul r10.yzw, r10.yyyy, cb0[36].xxyz
    r10.yzw = ((r10.yyyy)*(source[36].xxyz)).yzw;
    // 291: mad r10.xyz, r10.xxxx, cb0[35].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[35].xyzx)+(r10.yzwy)).xyz;
    // 292: mul r10.xyz, r10.xyzx, cb0[37].wwww
    r10.xyz = ((r10.xyzx)*(source[37].wwww)).xyz;
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
    // 299: mul r3.xyz, r6.yyyy, cb0[36].xyzx
    r3.xyz = ((r6.yyyy)*(source[36].xyzx)).xyz;
    // 300: mad r3.xyz, cb0[35].xyzx, r6.xxxx, r3.xyzx
    r3.xyz = ((source[35].xyzx)*(r6.xxxx)+(r3.xyzx)).xyz;
    // 301: mul r3.xyz, r3.xyzx, cb0[37].wwww
    r3.xyz = ((r3.xyzx)*(source[37].wwww)).xyz;
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
    // 309: mul_sat r2.w, r0.x, cb0[20].z
    r2.w = (saturate((r0.xxxx)*(source[20].zzzz))).w;
    // 310: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 311: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 312: mul_sat r3.x, r7.z, cb0[20].z
    r3.x = (saturate((r7.zzzz)*(source[20].zzzz))).x;
    // 313: add r3.y, -|r7.z|, l(1.000000)
    r3.y = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 314: mul r0.x, r0.x, r3.y
    r0.x = ((r0.xxxx)*(r3.yyyy)).x;
    // 315: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 316: add_sat r3.x, r3.x, -cb0[20].w
    r3.x = (saturate((r3.xxxx)+(-(source[20].wwww)))).x;
    // 317: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 318: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 319: mul r3.y, r3.y, cb0[21].x
    r3.y = ((r3.yyyy)*(source[21].xxxx)).y;
    // 320: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 321: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 322: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 323: mad r3.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r3.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 324: mul r2.w, r2.w, cb0[12].w
    r2.w = ((r2.wwww)*(source[12].wwww)).w;
    // 325: mad r3.xyz, cb0[13].wwww, r3.xyzx, cb0[13].xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(source[13].xyzx)).xyz;
    // 326: mad r3.xyz, r2.wwww, cb0[12].xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 327: add r2.w, cb0[0].y, cb0[0].x
    r2.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 328: add r2.w, r2.w, cb0[0].z
    r2.w = ((r2.wwww)+(source[0].zzzz)).w;
    // 329: add r4.w, -r2.w, l(1000.000000)
    r4.w = ((-(r2.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 330: mad r2.w, cb0[19].w, r4.w, r2.w
    r2.w = ((source[19].wwww)*(r4.wwww)+(r2.wwww)).w;
    // 331: mul r2.w, r2.w, l(0.010000)
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 332: mad r2.w, cb0[19].y, cb0[19].z, r2.w
    r2.w = ((source[19].yyyy)*(source[19].zzzz)+(r2.wwww)).w;
    // 333: mul r4.w, r2.w, l(3.524534)
    r4.w = ((r2.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 334: sincos null, r4.w, r4.w
    r4.w = (cos(r4.wwww)).w;
    // 335: add r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)+(r4.wwww)).w;
    // 336: mul r2.w, r2.w, l(1.328987)
    r2.w = ((r2.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 337: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 338: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 339: mad r2.w, r2.w, l(0.500000), cb0[19].x
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[19].xxxx)).w;
    // 340: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 341: mul r7.xyz, cb0[9].xyzx, cb0[18].wwww
    r7.xyz = ((source[9].xyzx)*(source[18].wwww)).xyz;
    // 342: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 343: mul r7.xyz, r2.wwww, r5.xyzx
    r7.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 344: dp3 r4.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 345: mad r5.xyz, -r2.wwww, r5.xyzx, r4.wwww
    r5.xyz = ((-(r2.wwww))*(r5.xyzx)+(r4.wwww)).xyz;
    // 346: mad r5.xyz, cb0[20].xxxx, r5.xyzx, r7.xyzx
    r5.xyz = ((source[20].xxxx)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 347: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 348: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 349: mad r5.xyz, cb0[20].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[20].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 350: mad r3.xyz, r5.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 351: log r2.w, |r0.x|
    r2.w = (log2(abs(r0.xxxx))).w;
    // 352: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 353: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 354: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 355: mul r4.xyz, r2.wwww, cb0[14].xyzx
    r4.xyz = ((r2.wwww)*(source[14].xyzx)).xyz;
    // 356: movc r4.xyz, r0.xxxx, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 357: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 358: mad r0.xyz, cb0[18].zzzz, r0.yzwy, r3.xyzx
    r0.xyz = ((source[18].zzzz)*(r0.yzwy)+(r3.xyzx)).xyz;
    // 359: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 360: mul r3.xyz, r6.wwww, cb0[36].xyzx
    r3.xyz = ((r6.wwww)*(source[36].xyzx)).xyz;
    // 361: mad r3.xyz, r6.zzzz, cb0[35].xyzx, r3.xyzx
    r3.xyz = ((r6.zzzz)*(source[35].xyzx)+(r3.xyzx)).xyz;
    // 362: mul r3.xyz, r3.xyzx, cb0[37].wwww
    r3.xyz = ((r3.xyzx)*(source[37].wwww)).xyz;
    // 363: mul_sat r4.xyz, cb0[17].xyzx, cb0[17].wwww
    r4.xyz = (saturate((source[17].xyzx)*(source[17].wwww))).xyz;
    // 364: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 365: mul r4.xyz, r4.xyzx, cb0[24].zzzz
    r4.xyz = ((r4.xyzx)*(source[24].zzzz)).xyz;
    // 366: dp3_sat o5.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 367: mul r4.xyz, r3.wwww, r5.xyzx
    r4.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 368: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 369: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 370: mad r0.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 371: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 372: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 373: mad o0.xyz, r1.xyzx, cb0[37].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[37].xyzx)+(r0.xyzx)).xyz;
    // 374: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 375: dp3 r0.x, r16.xyzx, r16.xyzx
    r0.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 376: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 377: mul r0.xyz, r0.xxxx, r16.xyzx
    r0.xyz = ((r0.xxxx)*(r16.xyzx)).xyz;
    // 378: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 379: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 380: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 381: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 382: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 383: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 384: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 385: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 386: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 387: ftou r0.x, cb0[34].z
    r0.x = (asfloat((uint4)(source[34].zzzz))).x;
    // 388: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 389: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 390: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 391: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 392: ret
    return output;
}

// source.character.realpbr-wing-ddk.v1 / source program 6134b047d9e0084e8c8c35a5dc689df5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase97(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18].z=(g_SourceCharacterTime.xxxx).x;
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
    // 9: mul r2.y, cb0[5].y, cb0[15].y
    r2.y = ((source[5].yyyy)*(source[15].yyyy)).y;
    // 10: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 11: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 12: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 13: frc r0.x, cb0[5].x
    r0.x = (frac(source[5].xxxx)).x;
    // 14: add r1.z, -r0.x, cb0[5].x
    r1.z = ((-(r0.xxxx))+(source[5].xxxx)).z;
    // 15: mul r2.z, r1.z, l(0.125000)
    r2.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 16: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t3.xyzw, s6, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 19: add r1.w, -cb0[5].w, l(1.000000)
    r1.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 21: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 22: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 23: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r2.x, cb0[5].z, l(1.500000)
    r2.x = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 25: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 26: mad r1.w, r1.w, l(0.500000), cb0[5].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 27: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 28: dp3 r1.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r0.yzwy, r1.wwww
    r2.xyz = ((-(r0.yzwy))+(r1.wwww)).xyz;
    // 30: mad r2.xyz, cb0[20].yyyy, r2.xyzx, r0.yzwy
    r2.xyz = ((source[20].yyyy)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 33: mad r2.xyz, cb0[20].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 43: mul r1.w, r6.y, cb0[17].y
    r1.w = ((r6.yyyy)*(source[17].yyyy)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 49: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 50: mad r4.xyz, cb0[20].yyyy, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 51: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 52: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r3.xyz, -r4.xyzx, r2.wwww
    r3.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 54: mad r3.xyz, cb0[20].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[20].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 55: mad r4.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 56: mad r7.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 57: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 58: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 59: mul r7.xyz, r2.xyzx, r3.xyzx
    r7.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 60: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: mad r2.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 62: mad r2.xyz, cb0[20].yyyy, r2.xyzx, r7.xyzx
    r2.xyz = ((source[20].yyyy)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 63: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 64: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 65: mad r2.xyz, cb0[20].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 66: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 67: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 68: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 69: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 70: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 71: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 72: max r0.x, r0.x, cb0[21].z
    r0.x = (max(r0.xxxx,source[21].zzzz)).x;
    // 73: min r0.x, r0.x, cb0[21].y
    r0.x = (min(r0.xxxx,source[21].yyyy)).x;
    // 74: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: mad r0.x, r1.w, r2.x, r0.x
    r0.x = ((r1.wwww)*(r2.xxxx)+(r0.xxxx)).x;
    // 76: mul_sat r2.w, r1.w, cb2[3].w
    r2.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 77: add r1.w, r0.x, l(-1.000000)
    r1.w = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 78: mad r1.w, cb0[22].x, r1.w, l(1.000000)
    r1.w = ((source[22].xxxx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: mul r3.xyz, r1.xyzx, r1.wwww
    r3.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 80: mul r2.x, r6.x, cb0[20].w
    r2.x = ((r6.xxxx)*(source[20].wwww)).x;
    // 81: mul r2.y, r6.z, cb0[22].z
    r2.y = ((r6.zzzz)*(source[22].zzzz)).y;
    // 82: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 83: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 84: movc r2.y, r5.z, l(0), r2.y
    r2.y = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 85: max r2.y, r2.y, cb0[1].x
    r2.y = (max(r2.yyyy,source[1].xxxx)).y;
    // 86: min r2.z, r2.y, l(1.000000)
    r2.z = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 87: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 88: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 89: add_sat r2.x, r2.x, cb0[21].x
    r2.x = (saturate((r2.xxxx)+(source[21].xxxx))).x;
    // 90: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 91: mul r5.xyz, r2.yyyy, cb0[14].xyzx
    r5.xyz = ((r2.yyyy)*(source[14].xyzx)).xyz;
    // 92: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 93: mad r1.xyz, r1.wwww, r1.xyzx, -r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 94: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 95: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 96: add r3.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 98: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 99: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 100: mad r5.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 101: mad r6.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r6.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 102: mad r5.xyz, r0.xxxx, r5.xyzx, r6.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 103: mad r3.xyz, r5.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 104: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 105: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 106: mov_sat r1.w, cb0[22].y
    r1.w = (saturate(source[22].yyyy)).w;
    // 107: mad r5.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r5.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 108: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 109: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 110: mad r5.xyz, r2.wwww, r5.xyzx, r2.xxxx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r2.xxxx)).xyz;
    // 111: mul_sat r1.w, r5.y, l(50.000000)
    r1.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 113: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 114: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 115: mul r6.xy, r2.xyxx, cb0[17].xxxx
    r6.xy = ((r2.xyxx)*(source[17].xxxx)).xy;
    // 116: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 118: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 119: add r6.z, r2.x, l(0.000010)
    r6.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 120: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 121: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 122: div r6.xyz, r6.xyzx, r2.xxxx
    r6.xyz = ((r6.xyzx)/(r2.xxxx)).xyz;
    // 123: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 124: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 125: mul r7.xyz, r2.xxxx, r6.xyzx
    r7.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 126: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 127: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 128: mul r8.xyz, r2.xxxx, v5.xyzx
    r8.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 129: dp3 r2.x, r7.xyzx, r8.xyzx
    r2.x = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 130: deriv_rtx_coarse r9.x, r2.x
    r9.x = (ddx_coarse(r2.xxxx)).x;
    // 131: deriv_rty_coarse r9.y, r2.x
    r9.y = (ddy_coarse(r2.xxxx)).y;
    // 132: dp2 r2.y, r9.xyxx, r9.xyxx
    r2.y = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).y;
    // 133: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 134: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 135: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 136: min r9.y, r2.y, l(1.000000)
    r9.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: add r2.y, -r9.y, l(1.000000)
    r2.y = ((-(r9.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 138: max r10.xyz, r5.xyzx, r2.yyyy
    r10.xyz = (max(r5.xyzx,r2.yyyy)).xyz;
    // 139: add r10.xyz, -r5.xyzx, r10.xyzx
    r10.xyz = ((-(r5.xyzx))+(r10.xyzx)).xyz;
    // 140: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 141: mul r11.xyz, r2.xxxx, r7.xyzx
    r11.xyz = ((r2.xxxx)*(r7.xyzx)).xyz;
    // 142: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 143: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 147: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 148: mul r2.x, r2.x, cb0[2].y
    r2.x = ((r2.xxxx)*(source[2].yyyy)).x;
    // 149: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 150: mad_sat r2.x, r2.x, cb0[2].w, cb0[2].z
    r2.x = (saturate((r2.xxxx)*(source[2].wwww)+(source[2].zzzz))).x;
    // 151: mul r2.x, r2.x, cb0[22].w
    r2.x = ((r2.xxxx)*(source[22].wwww)).x;
    // 152: add_sat r9.x, -r1.w, r2.y
    r9.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 153: sample_indexable(texture2d)(float,float,float,float) r2.yz, r9.xyxx, t7.zxyw, s8
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 154: add r1.w, r0.x, r9.x
    r1.w = ((r0.xxxx)+(r9.xxxx)).w;
    // 155: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 156: mul r9.xzw, r2.zzzz, r5.xxyz
    r9.xzw = ((r2.zzzz)*(r5.xxyz)).xzw;
    // 157: mad r9.xzw, r10.xxyz, r2.yyyy, r9.xxzw
    r9.xzw = ((r10.xxyz)*(r2.yyyy)+(r9.xxzw)).xzw;
    // 158: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.zzzz)).y;
    // 159: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 160: mad r10.xyz, r5.xyzx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((r5.xyzx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: dp3 r2.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 162: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 163: mad r12.xyz, -r9.xzwx, r10.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xzwx))*(r10.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mul r9.xzw, r9.xxzw, r10.xxyz
    r9.xzw = ((r9.xxzw)*(r10.xxyz)).xzw;
    // 165: mul r10.xyz, r1.xyzx, r12.xyzx
    r10.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 166: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: mul r10.xyz, r2.yyyy, r10.xyzx
    r10.xyz = ((r2.yyyy)*(r10.xyzx)).xyz;
    // 168: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 169: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 170: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 171: mul r13.xyz, r3.wwww, v1.xyzx
    r13.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 172: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 173: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 174: mul r14.xyz, r3.wwww, v0.xyzx
    r14.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 175: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 176: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 177: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 178: dp3 r16.y, r15.xyzx, r7.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 179: dp3 r15.y, r15.xyzx, r11.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 180: dp3 r16.x, r14.xyzx, r7.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 181: dp3 r15.x, r14.xyzx, r11.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 182: dp2 r14.z, r16.xyxx, cb0[24].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 183: mul r15.zw, cb0[24].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r15.zw = ((source[24].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 184: dp2 r14.x, r16.xyxx, r15.zwzz
    r14.x = (dot((r16.xyxx).xy,(r15.zwzz).xy).xxxx).x;
    // 185: dp2 r17.x, r15.xyxx, r15.zwzz
    r17.x = (dot((r15.xyxx).xy,(r15.zwzz).xy).xxxx).x;
    // 186: dp2 r17.z, r15.xyxx, cb0[24].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 187: dp3 r14.y, r13.xyzx, r7.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 188: dp3 r17.y, r13.xyzx, r11.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 189: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 190: dp4 r13.x, cb0[25].xyzw, r14.xyzw
    r13.x = (dot((source[25].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 191: dp4 r13.y, cb0[26].xyzw, r14.xyzw
    r13.y = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 192: dp4 r13.z, cb0[27].xyzw, r14.xyzw
    r13.z = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 193: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 194: dp4 r18.x, cb0[28].xyzw, r15.xyzw
    r18.x = (dot((source[28].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 195: dp4 r18.y, cb0[29].xyzw, r15.xyzw
    r18.y = (dot((source[29].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 196: dp4 r18.z, cb0[30].xyzw, r15.xyzw
    r18.z = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 197: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 198: mul r3.w, r14.y, r14.y
    r3.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 199: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 200: mad r3.w, r14.x, r14.x, -r3.w
    r3.w = ((r14.xxxx)*(r14.xxxx)+(-(r3.wwww))).w;
    // 201: mad r13.xyz, cb0[31].xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((source[31].xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 202: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 203: mul r13.xyz, r13.xyzx, cb0[23].xyzx
    r13.xyz = ((r13.xyzx)*(source[23].xyzx)).xyz;
    // 204: mul r13.xyz, r13.xyzx, cb0[24].zzzz
    r13.xyz = ((r13.xyzx)*(source[24].zzzz)).xyz;
    // 205: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[23].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[23].wwww)).xyz;
    // 206: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 207: add r13.xyz, -r3.wwww, r13.xyzx
    r13.xyz = ((-(r3.wwww))+(r13.xyzx)).xyz;
    // 208: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 209: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 210: mad r4.w, r9.y, l(2.000000), l(2.000000)
    r4.w = ((r9.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 211: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 212: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 213: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 214: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 215: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 216: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 217: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 218: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 219: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 220: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 221: mul r10.xyz, r10.xyzx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r13.xyzx)).xyz;
    // 222: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 223: mul r10.xyz, r3.xyzx, r10.xyzx
    r10.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 224: mul r3.w, r9.y, l(5.000000)
    r3.w = ((r9.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 225: mul r5.w, r9.y, r9.y
    r5.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 226: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 227: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 228: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 229: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 230: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 231: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t8.xyzw, s7, r3.w
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r17.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 232: mul r13.xyz, r13.xyzx, r13.wwww
    r13.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 233: mul r13.xyz, r13.xyzx, cb0[23].xyzx
    r13.xyz = ((r13.xyzx)*(source[23].xyzx)).xyz;
    // 234: mul r13.xyz, r13.xyzx, cb0[24].zzzz
    r13.xyz = ((r13.xyzx)*(source[24].zzzz)).xyz;
    // 235: mad r13.xyz, r13.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[23].wwww
    r13.xyz = ((r13.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[23].wwww)).xyz;
    // 236: dp3 r1.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 237: add r13.xyz, -r1.wwww, r13.xyzx
    r13.xyz = ((-(r1.wwww))+(r13.xyzx)).xyz;
    // 238: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 239: dp3 r1.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 240: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 241: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 242: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 243: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 244: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 245: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 246: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 247: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 248: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 249: mul r13.xyz, r1.wwww, r13.xyzx
    r13.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 250: mul r14.xyz, r9.xzwx, r13.xyzx
    r14.xyz = ((r9.xzwx)*(r13.xyzx)).xyz;
    // 251: mad r1.w, r0.x, r5.x, r5.y
    r1.w = ((r0.xxxx)*(r5.xxxx)+(r5.yyyy)).w;
    // 252: mad r1.w, r1.w, r0.x, r5.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r5.zzzz)).w;
    // 253: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 254: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 255: mad r5.xyz, r14.xyzx, r0.xxxx, r10.xyzx
    r5.xyz = ((r14.xyzx)*(r0.xxxx)+(r10.xyzx)).xyz;
    // 256: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 257: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 258: mul r10.xyz, r1.wwww, v6.xyzx
    r10.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 259: dp3 r1.w, r10.xyzx, r7.xyzx
    r1.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 260: dp3 r2.z, -r10.xyzx, r7.xyzx
    r2.z = (dot((-(r10.xyzx)).xyz,(r7.xyzx).xyz).xxxx).z;
    // 261: dp3 r3.w, r10.xyzx, r11.xyzx
    r3.w = (dot((r10.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 262: mad r7.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 263: mad r7.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r7.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 264: mul r7.xyzw, r7.xyzw, r7.xyzw
    r7.xyzw = ((r7.xyzw)*(r7.xyzw)).xyzw;
    // 265: mad r10.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 266: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 267: mul r10.yzw, r10.yyyy, cb0[34].xxyz
    r10.yzw = ((r10.yyyy)*(source[34].xxyz)).yzw;
    // 268: mad r10.xyz, r10.xxxx, cb0[33].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[33].xyzx)+(r10.yzwy)).xyz;
    // 269: mul r10.xyz, r10.xyzx, cb0[35].wwww
    r10.xyz = ((r10.xyzx)*(source[35].wwww)).xyz;
    // 270: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 271: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 272: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 273: mul r3.xyz, r12.xyzx, r3.xyzx
    r3.xyz = ((r12.xyzx)*(r3.xyzx)).xyz;
    // 274: mad r3.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 275: mad r3.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 276: mul r5.xyz, r7.yyyy, cb0[34].xyzx
    r5.xyz = ((r7.yyyy)*(source[34].xyzx)).xyz;
    // 277: mad r5.xyz, cb0[33].xyzx, r7.xxxx, r5.xyzx
    r5.xyz = ((source[33].xyzx)*(r7.xxxx)+(r5.xyzx)).xyz;
    // 278: mul r5.xyz, r5.xyzx, cb0[35].wwww
    r5.xyz = ((r5.xyzx)*(source[35].wwww)).xyz;
    // 279: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 280: mul r5.xyz, r13.xyzx, r5.xyzx
    r5.xyz = ((r13.xyzx)*(r5.xyzx)).xyz;
    // 281: mul r5.xyz, r5.xyzx, r9.xzwx
    r5.xyz = ((r5.xyzx)*(r9.xzwx)).xyz;
    // 282: mad r3.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 283: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 284: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 285: dp3 r0.x, r6.xyzx, r8.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 286: mul_sat r1.w, r0.x, cb0[19].z
    r1.w = (saturate((r0.xxxx)*(source[19].zzzz))).w;
    // 287: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 288: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 289: mul_sat r2.z, r8.z, cb0[19].z
    r2.z = (saturate((r8.zzzz)*(source[19].zzzz))).z;
    // 290: add r2.w, -|r8.z|, l(1.000000)
    r2.w = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 291: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 292: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 293: add_sat r2.z, r2.z, -cb0[19].w
    r2.z = (saturate((r2.zzzz)+(-(source[19].wwww)))).z;
    // 294: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 295: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 296: mul r2.w, r2.w, cb0[20].x
    r2.w = ((r2.wwww)*(source[20].xxxx)).w;
    // 297: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 298: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 299: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 300: sample_b_indexable(texture2d)(float,float,float,float) r2.z, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r2.z = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).z;
    // 301: add r2.w, r1.w, -r2.z
    r2.w = ((r1.wwww)+(-(r2.zzzz))).w;
    // 302: mad r2.z, cb0[11].w, r2.w, r2.z
    r2.z = ((source[11].wwww)*(r2.wwww)+(r2.zzzz)).z;
    // 303: mad r5.xyz, r1.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r5.xyz = ((r1.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 304: mad r5.xyz, cb0[12].wwww, r5.xyzx, cb0[12].xyzx
    r5.xyz = ((source[12].wwww)*(r5.xyzx)+(source[12].xyzx)).xyz;
    // 305: mad r5.xyz, r2.zzzz, cb0[11].xyzx, r5.xyzx
    r5.xyz = ((r2.zzzz)*(source[11].xyzx)+(r5.xyzx)).xyz;
    // 306: mul r2.zw, v4.xxxy, cb0[7].zzzz
    r2.zw = ((v4.xxxy)*(source[7].zzzz)).zw;
    // 307: mul r6.xy, cb0[7].xyxx, cb0[18].zzzz
    r6.xy = ((source[7].xyxx)*(source[18].zzzz)).xy;
    // 308: mad r2.zw, r6.xxxy, l(0.000000, 0.000000, -0.500000, 0.500000), r2.zzzw
    r2.zw = ((r6.xxxy)*(float4(0.000000,0.000000,-0.500000,0.500000))+(r2.zzzw)).zw;
    // 309: mad r6.xy, cb0[7].zzzz, v4.xyxx, r6.xyxx
    r6.xy = ((source[7].zzzz)*(v4.xyxx)+(r6.xyxx)).xy;
    // 310: sample_b_indexable(texture2d)(float,float,float,float) r2.z, r2.zwzz, t5.yzxw, s4, l(0.000000)
    r2.z = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // 311: mad r2.zw, r2.zzzz, cb0[19].xxxx, r6.xxxy
    r2.zw = ((r2.zzzz)*(source[19].xxxx)+(r6.xxxy)).zw;
    // 312: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.zwzz, t5.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 313: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 314: mul r6.xyz, r6.xyzx, r8.wwww
    r6.xyz = ((r6.xyzx)*(r8.wwww)).xyz;
    // 315: mul r9.xyz, cb0[8].xyzx, cb0[19].yyyy
    r9.xyz = ((source[8].xyzx)*(source[19].yyyy)).xyz;
    // 316: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 317: mad r9.xyz, r1.wwww, r6.xyzx, -r6.xyzx
    r9.xyz = ((r1.wwww)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 318: mad r6.xyz, cb0[8].wwww, r9.xyzx, r6.xyzx
    r6.xyz = ((source[8].wwww)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 319: add r1.w, cb0[0].y, cb0[0].x
    r1.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 320: add r1.w, r1.w, cb0[0].z
    r1.w = ((r1.wwww)+(source[0].zzzz)).w;
    // 321: add r2.z, -r1.w, l(1000.000000)
    r2.z = ((-(r1.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).z;
    // 322: mad r1.w, cb0[18].w, r2.z, r1.w
    r1.w = ((source[18].wwww)*(r2.zzzz)+(r1.wwww)).w;
    // 323: mul r1.w, r1.w, l(0.010000)
    r1.w = ((r1.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 324: mad r1.w, cb0[18].y, cb0[18].z, r1.w
    r1.w = ((source[18].yyyy)*(source[18].zzzz)+(r1.wwww)).w;
    // 325: mul r2.z, r1.w, l(3.524534)
    r2.z = ((r1.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).z;
    // 326: sincos null, r2.z, r2.z
    r2.z = (cos(r2.zzzz)).z;
    // 327: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 328: mul r1.w, r1.w, l(1.328987)
    r1.w = ((r1.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 329: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 330: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 331: mad r1.w, r1.w, l(0.500000), cb0[18].x
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[18].xxxx)).w;
    // 332: mul r9.xyz, cb0[6].xyzx, cb0[17].wwww
    r9.xyz = ((source[6].xyzx)*(source[17].wwww)).xyz;
    // 333: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 334: mad r6.xyz, r1.wwww, r8.xyzx, r6.xyzx
    r6.xyz = ((r1.wwww)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 335: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 336: add r8.xyz, -r6.xyzx, r1.wwww
    r8.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 337: mad r6.xyz, cb0[20].yyyy, r8.xyzx, r6.xyzx
    r6.xyz = ((source[20].yyyy)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 338: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 339: add r8.xyz, -r6.xyzx, r1.wwww
    r8.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 340: mad r6.xyz, cb0[20].zzzz, r8.xyzx, r6.xyzx
    r6.xyz = ((source[20].zzzz)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 341: mad r4.xyz, r6.xyzx, r4.xyzx, r5.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 342: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 343: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 344: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 345: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 346: mul r5.xyz, r1.wwww, cb0[13].xyzx
    r5.xyz = ((r1.wwww)*(source[13].xyzx)).xyz;
    // 347: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 348: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 349: mad r0.xyz, cb0[17].zzzz, r0.yzwy, r4.xyzx
    r0.xyz = ((source[17].zzzz)*(r0.yzwy)+(r4.xyzx)).xyz;
    // 350: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 351: mul r4.xyz, r7.wwww, cb0[34].xyzx
    r4.xyz = ((r7.wwww)*(source[34].xyzx)).xyz;
    // 352: mad r4.xyz, r7.zzzz, cb0[33].xyzx, r4.xyzx
    r4.xyz = ((r7.zzzz)*(source[33].xyzx)+(r4.xyzx)).xyz;
    // 353: mul r4.xyz, r4.xyzx, cb0[35].wwww
    r4.xyz = ((r4.xyzx)*(source[35].wwww)).xyz;
    // 354: mul_sat r5.xyz, cb0[16].xyzx, cb0[16].wwww
    r5.xyz = (saturate((source[16].xyzx)*(source[16].wwww))).xyz;
    // 355: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 356: mul r5.xyz, r5.xyzx, cb0[22].wwww
    r5.xyz = ((r5.xyzx)*(source[22].wwww)).xyz;
    // 357: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 358: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 359: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 360: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 361: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 362: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 363: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 364: mad o0.xyz, r1.xyzx, cb0[35].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[35].xyzx)+(r0.xyzx)).xyz;
    // 365: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 366: dp3 r0.x, r16.xyzx, r16.xyzx
    r0.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 367: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 368: mul r0.xyz, r0.xxxx, r16.xyzx
    r0.xyz = ((r0.xxxx)*(r16.xyzx)).xyz;
    // 369: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 370: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 371: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 372: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 373: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 374: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 375: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 376: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 377: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 378: ftou r0.x, cb0[32].z
    r0.x = (asfloat((uint4)(source[32].zzzz))).x;
    // 379: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 380: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 381: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 382: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 383: ret
    return output;
}

// source.character.realpbr-wing-ddk-membrane.v1 / source program 6a904bb0c49378438ff67ad401f49bee
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase98(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].z=(g_SourceCharacterTime.xxxx).x;
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
    // 7: add r0.x, -cb0[4].w, l(1.000000)
    r0.x = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul r0.x, r0.x, cb0[15].z
    r0.x = ((r0.xxxx)*(source[15].zzzz)).x;
    // 9: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 10: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 11: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r1.x, cb0[4].z, l(1.500000)
    r1.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 13: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.500000), cb0[4].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).x;
    // 15: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 16: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[4].y, cb0[11].y
    r2.y = ((source[4].yyyy)*(source[11].yyyy)).y;
    // 18: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 21: frc r1.z, cb0[4].x
    r1.z = (frac(source[4].xxxx)).z;
    // 22: add r1.w, -r1.z, cb0[4].x
    r1.w = ((-(r1.zzzz))+(source[4].xxxx)).w;
    // 23: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 24: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r1.xyw, r0.xxxx, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r2.xyxz)).xyw;
    // 27: mul r0.x, r1.z, r2.w
    r0.x = ((r1.zzzz)*(r2.wwww)).x;
    // 28: dp3 r1.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 29: add r2.xyz, -r0.yzwy, r1.zzzz
    r2.xyz = ((-(r0.yzwy))+(r1.zzzz)).xyz;
    // 30: mad r2.xyz, cb0[13].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[13].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[14].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[14].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 43: mul r1.z, r6.y, cb0[13].y
    r1.z = ((r6.yyyy)*(source[13].yyyy)).z;
    // 44: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 45: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: movc r1.z, r5.y, l(0), r1.z
    r1.z = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 47: mad r3.xyz, r1.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 49: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 50: mad r4.xyz, cb0[13].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[13].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 51: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 52: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r3.xyz, -r4.xyzx, r2.wwww
    r3.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 54: mad r3.xyz, cb0[14].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[14].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 55: mad r4.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 56: mad r7.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 57: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 58: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 59: mul r7.xyz, r2.xyzx, r3.xyzx
    r7.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 60: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: mad r2.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 62: mad r2.xyz, cb0[13].wwww, r2.xyzx, r7.xyzx
    r2.xyz = ((source[13].wwww)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 63: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 64: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 65: mad r2.xyz, cb0[14].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[14].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 66: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 67: mad r1.xyw, r1.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r2.xyxz
    r1.xyw = ((r1.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r2.xyxz))).xyw;
    // 68: mad r1.xyw, r0.xxxx, r1.xyxw, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 69: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 70: add r0.x, r1.w, r0.x
    r0.x = ((r1.wwww)+(r0.xxxx)).x;
    // 71: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 72: max r0.x, r0.x, cb0[16].x
    r0.x = (max(r0.xxxx,source[16].xxxx)).x;
    // 73: min r0.x, r0.x, cb0[15].w
    r0.x = (min(r0.xxxx,source[15].wwww)).x;
    // 74: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: mad r0.x, r1.z, r2.x, r0.x
    r0.x = ((r1.zzzz)*(r2.xxxx)+(r0.xxxx)).x;
    // 76: mul_sat r2.w, r1.z, cb2[3].w
    r2.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 77: add r1.z, r0.x, l(-1.000000)
    r1.z = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 78: mad r1.z, cb0[16].z, r1.z, l(1.000000)
    r1.z = ((source[16].zzzz)*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 79: mul r3.xyz, r1.xywx, r1.zzzz
    r3.xyz = ((r1.xywx)*(r1.zzzz)).xyz;
    // 80: mul r2.x, r6.x, cb0[15].x
    r2.x = ((r6.xxxx)*(source[15].xxxx)).x;
    // 81: mul r2.y, r6.z, cb0[17].x
    r2.y = ((r6.zzzz)*(source[17].xxxx)).y;
    // 82: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 83: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 84: movc r2.y, r5.z, l(0), r2.y
    r2.y = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 85: max r2.y, r2.y, cb0[0].x
    r2.y = (max(r2.yyyy,source[0].xxxx)).y;
    // 86: min r2.z, r2.y, l(1.000000)
    r2.z = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 87: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 88: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 89: add_sat r2.x, r2.x, cb0[15].y
    r2.x = (saturate((r2.xxxx)+(source[15].yyyy))).x;
    // 90: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 91: mul r4.xyz, r2.yyyy, cb0[10].xyzx
    r4.xyz = ((r2.yyyy)*(source[10].xyzx)).xyz;
    // 92: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 93: mad r1.xyz, r1.zzzz, r1.xywx, -r3.xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(-(r3.xyzx))).xyz;
    // 94: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 95: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 96: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 98: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 99: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 100: mad r4.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 101: mad r5.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 102: mad r4.xyz, r0.xxxx, r4.xyzx, r5.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 103: mad r3.xyz, r4.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 104: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 105: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 106: mov_sat r1.w, cb0[16].w
    r1.w = (saturate(source[16].wwww)).w;
    // 107: mad r4.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r4.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 108: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 109: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 110: mad r4.xyz, r2.wwww, r4.xyzx, r2.xxxx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xxxx)).xyz;
    // 111: mul_sat r1.w, r4.y, l(50.000000)
    r1.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 113: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 114: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 115: mul r5.xy, r2.xyxx, cb0[13].xxxx
    r5.xy = ((r2.xyxx)*(source[13].xxxx)).xy;
    // 116: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 118: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 119: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 120: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 121: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 122: div r5.xyz, r5.xyzx, r2.xxxx
    r5.xyz = ((r5.xyzx)/(r2.xxxx)).xyz;
    // 123: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 124: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 125: mul r6.xyz, r2.xxxx, r5.xyzx
    r6.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 126: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 127: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 128: mul r7.xyz, r2.xxxx, v5.xyzx
    r7.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 129: dp3 r2.x, r6.xyzx, r7.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 130: deriv_rtx_coarse r8.x, r2.x
    r8.x = (ddx_coarse(r2.xxxx)).x;
    // 131: deriv_rty_coarse r8.y, r2.x
    r8.y = (ddy_coarse(r2.xxxx)).y;
    // 132: dp2 r2.y, r8.xyxx, r8.xyxx
    r2.y = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).y;
    // 133: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 134: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 135: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 136: min r8.y, r2.y, l(1.000000)
    r8.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: add r2.y, -r8.y, l(1.000000)
    r2.y = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 138: max r9.xyz, r4.xyzx, r2.yyyy
    r9.xyz = (max(r4.xyzx,r2.yyyy)).xyz;
    // 139: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 140: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 141: mul r10.xyz, r2.xxxx, r6.xyzx
    r10.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 142: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 143: add r1.w, r10.z, l(1.000000)
    r1.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 147: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 148: mul r2.x, r2.x, cb0[1].y
    r2.x = ((r2.xxxx)*(source[1].yyyy)).x;
    // 149: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 150: mad_sat r2.x, r2.x, cb0[1].w, cb0[1].z
    r2.x = (saturate((r2.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 151: mul r2.x, r2.x, cb0[17].y
    r2.x = ((r2.xxxx)*(source[17].yyyy)).x;
    // 152: add_sat r8.x, -r1.w, r2.y
    r8.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 153: sample_indexable(texture2d)(float,float,float,float) r2.yz, r8.xyxx, t5.zxyw, s6
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 154: add r1.w, r0.x, r8.x
    r1.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 155: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 156: mul r8.xzw, r2.zzzz, r4.xxyz
    r8.xzw = ((r2.zzzz)*(r4.xxyz)).xzw;
    // 157: mad r8.xzw, r9.xxyz, r2.yyyy, r8.xxzw
    r8.xzw = ((r9.xxyz)*(r2.yyyy)+(r8.xxzw)).xzw;
    // 158: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.zzzz)).y;
    // 159: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 160: mad r9.xyz, r4.xyzx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((r4.xyzx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: dp3 r2.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 162: mad r4.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 163: mad r11.xyz, -r8.xzwx, r9.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r8.xzwx))*(r9.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mul r8.xzw, r8.xxzw, r9.xxyz
    r8.xzw = ((r8.xxzw)*(r9.xxyz)).xzw;
    // 165: mul r9.xyz, r1.xyzx, r11.xyzx
    r9.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 166: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: mul r9.xyz, r2.yyyy, r9.xyzx
    r9.xyz = ((r2.yyyy)*(r9.xyzx)).xyz;
    // 168: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 169: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 170: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 171: mul r12.xyz, r3.wwww, v1.xyzx
    r12.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 172: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 173: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 174: mul r13.xyz, r3.wwww, v0.xyzx
    r13.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 175: mul r14.xyz, r12.zxyz, r13.yzxy
    r14.xyz = ((r12.zxyz)*(r13.yzxy)).xyz;
    // 176: mad r14.xyz, r12.yzxy, r13.zxyz, -r14.xyzx
    r14.xyz = ((r12.yzxy)*(r13.zxyz)+(-(r14.xyzx))).xyz;
    // 177: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 178: dp3 r15.y, r14.xyzx, r6.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 179: dp3 r14.y, r14.xyzx, r10.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 180: dp3 r15.x, r13.xyzx, r6.xyzx
    r15.x = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 181: dp3 r14.x, r13.xyzx, r10.xyzx
    r14.x = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 182: dp2 r13.z, r15.xyxx, cb0[19].xyxx
    r13.z = (dot((r15.xyxx).xy,(source[19].xyxx).xy).xxxx).z;
    // 183: mul r14.zw, cb0[19].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r14.zw = ((source[19].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 184: dp2 r13.x, r15.xyxx, r14.zwzz
    r13.x = (dot((r15.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 185: dp2 r16.x, r14.xyxx, r14.zwzz
    r16.x = (dot((r14.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 186: dp2 r16.z, r14.xyxx, cb0[19].xyxx
    r16.z = (dot((r14.xyxx).xy,(source[19].xyxx).xy).xxxx).z;
    // 187: dp3 r13.y, r12.xyzx, r6.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 188: dp3 r16.y, r12.xyzx, r10.xyzx
    r16.y = (dot((r12.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 189: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 190: dp4 r12.x, cb0[20].xyzw, r13.xyzw
    r12.x = (dot((source[20].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 191: dp4 r12.y, cb0[21].xyzw, r13.xyzw
    r12.y = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 192: dp4 r12.z, cb0[22].xyzw, r13.xyzw
    r12.z = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 193: mul r14.xyzw, r13.yzzx, r13.xyzz
    r14.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 194: dp4 r17.x, cb0[23].xyzw, r14.xyzw
    r17.x = (dot((source[23].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 195: dp4 r17.y, cb0[24].xyzw, r14.xyzw
    r17.y = (dot((source[24].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 196: dp4 r17.z, cb0[25].xyzw, r14.xyzw
    r17.z = (dot((source[25].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 197: add r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)+(r17.xyzx)).xyz;
    // 198: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 199: mov r15.z, r13.y
    r15.z = (r13.yyyy).z;
    // 200: mad r3.w, r13.x, r13.x, -r3.w
    r3.w = ((r13.xxxx)*(r13.xxxx)+(-(r3.wwww))).w;
    // 201: mad r12.xyz, cb0[26].xyzx, r3.wwww, r12.xyzx
    r12.xyz = ((source[26].xyzx)*(r3.wwww)+(r12.xyzx)).xyz;
    // 202: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 203: mul r12.xyz, r12.xyzx, cb0[18].xyzx
    r12.xyz = ((r12.xyzx)*(source[18].xyzx)).xyz;
    // 204: mul r12.xyz, r12.xyzx, cb0[19].zzzz
    r12.xyz = ((r12.xyzx)*(source[19].zzzz)).xyz;
    // 205: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[18].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[18].wwww)).xyz;
    // 206: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 207: add r12.xyz, -r3.wwww, r12.xyzx
    r12.xyz = ((-(r3.wwww))+(r12.xyzx)).xyz;
    // 208: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 209: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 210: mad r4.w, r8.y, l(2.000000), l(2.000000)
    r4.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 211: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 212: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 213: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 214: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 215: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 216: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 217: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 218: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 219: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 220: mul r12.xyz, r3.wwww, r12.xyzx
    r12.xyz = ((r3.wwww)*(r12.xyzx)).xyz;
    // 221: mul r9.xyz, r9.xyzx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r12.xyzx)).xyz;
    // 222: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 223: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 224: mul r3.w, r8.y, l(5.000000)
    r3.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 225: mul r5.w, r8.y, r8.y
    r5.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 226: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 227: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 228: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 229: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 230: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 231: sample_l_indexable(texturecube)(float,float,float,float) r12.xyzw, r16.xyzx, t6.xyzw, s5, r3.w
    r12.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r16.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 232: mul r12.xyz, r12.xyzx, r12.wwww
    r12.xyz = ((r12.xyzx)*(r12.wwww)).xyz;
    // 233: mul r12.xyz, r12.xyzx, cb0[18].xyzx
    r12.xyz = ((r12.xyzx)*(source[18].xyzx)).xyz;
    // 234: mul r12.xyz, r12.xyzx, cb0[19].zzzz
    r12.xyz = ((r12.xyzx)*(source[19].zzzz)).xyz;
    // 235: mad r12.xyz, r12.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[18].wwww
    r12.xyz = ((r12.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[18].wwww)).xyz;
    // 236: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 237: add r12.xyz, -r1.wwww, r12.xyzx
    r12.xyz = ((-(r1.wwww))+(r12.xyzx)).xyz;
    // 238: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 239: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 240: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 241: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 242: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 243: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 244: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 245: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 246: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 247: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 248: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 249: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 250: mul r13.xyz, r8.xzwx, r12.xyzx
    r13.xyz = ((r8.xzwx)*(r12.xyzx)).xyz;
    // 251: mad r1.w, r0.x, r4.x, r4.y
    r1.w = ((r0.xxxx)*(r4.xxxx)+(r4.yyyy)).w;
    // 252: mad r1.w, r1.w, r0.x, r4.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r4.zzzz)).w;
    // 253: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 254: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 255: mad r4.xyz, r13.xyzx, r0.xxxx, r9.xyzx
    r4.xyz = ((r13.xyzx)*(r0.xxxx)+(r9.xyzx)).xyz;
    // 256: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 257: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 258: mul r9.xyz, r1.wwww, v6.xyzx
    r9.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 259: dp3 r1.w, r9.xyzx, r6.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 260: dp3 r2.z, -r9.xyzx, r6.xyzx
    r2.z = (dot((-(r9.xyzx)).xyz,(r6.xyzx).xyz).xxxx).z;
    // 261: dp3 r3.w, r9.xyzx, r10.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 262: mad r6.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 263: mad r6.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 264: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 265: mad r9.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 266: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 267: mul r9.yzw, r9.yyyy, cb0[29].xxyz
    r9.yzw = ((r9.yyyy)*(source[29].xxyz)).yzw;
    // 268: mad r9.xyz, r9.xxxx, cb0[28].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[28].xyzx)+(r9.yzwy)).xyz;
    // 269: mul r9.xyz, r9.xyzx, cb0[30].wwww
    r9.xyz = ((r9.xyzx)*(source[30].wwww)).xyz;
    // 270: mul r9.xyz, r1.xyzx, r9.xyzx
    r9.xyz = ((r1.xyzx)*(r9.xyzx)).xyz;
    // 271: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 272: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 273: mul r3.xyz, r11.xyzx, r3.xyzx
    r3.xyz = ((r11.xyzx)*(r3.xyzx)).xyz;
    // 274: mad r3.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 275: mad r3.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 276: mul r4.xyz, r6.yyyy, cb0[29].xyzx
    r4.xyz = ((r6.yyyy)*(source[29].xyzx)).xyz;
    // 277: mad r4.xyz, cb0[28].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[28].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 278: mul r4.xyz, r4.xyzx, cb0[30].wwww
    r4.xyz = ((r4.xyzx)*(source[30].wwww)).xyz;
    // 279: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 280: mul r4.xyz, r12.xyzx, r4.xyzx
    r4.xyz = ((r12.xyzx)*(r4.xyzx)).xyz;
    // 281: mul r4.xyz, r4.xyzx, r8.xzwx
    r4.xyz = ((r4.xyzx)*(r8.xzwx)).xyz;
    // 282: mad r3.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 283: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 284: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 285: dp3 r0.x, r5.xyzx, r7.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 286: mul_sat r1.w, r0.x, cb0[14].y
    r1.w = (saturate((r0.xxxx)*(source[14].yyyy))).w;
    // 287: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 288: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 289: mul_sat r2.z, r7.z, cb0[14].y
    r2.z = (saturate((r7.zzzz)*(source[14].yyyy))).z;
    // 290: add r2.w, -|r7.z|, l(1.000000)
    r2.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 291: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 292: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 293: add_sat r2.z, r2.z, -cb0[14].z
    r2.z = (saturate((r2.zzzz)+(-(source[14].zzzz)))).z;
    // 294: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 295: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 296: mul r2.w, r2.w, cb0[14].w
    r2.w = ((r2.wwww)*(source[14].wwww)).w;
    // 297: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 298: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 299: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 300: sample_b_indexable(texture2d)(float,float,float,float) r2.z, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.z = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).z;
    // 301: add r2.w, r1.w, -r2.z
    r2.w = ((r1.wwww)+(-(r2.zzzz))).w;
    // 302: mad r4.xyz, r1.wwww, cb0[8].xyzx, -cb0[8].xyzx
    r4.xyz = ((r1.wwww)*(source[8].xyzx)+(-(source[8].xyzx))).xyz;
    // 303: mad r4.xyz, cb0[8].wwww, r4.xyzx, cb0[8].xyzx
    r4.xyz = ((source[8].wwww)*(r4.xyzx)+(source[8].xyzx)).xyz;
    // 304: mad r1.w, cb0[7].w, r2.w, r2.z
    r1.w = ((source[7].wwww)*(r2.wwww)+(r2.zzzz)).w;
    // 305: mad r4.xyz, r1.wwww, cb0[7].xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(source[7].xyzx)+(r4.xyzx)).xyz;
    // 306: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 307: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 308: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 309: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 310: mul r5.xyz, r1.wwww, cb0[9].xyzx
    r5.xyz = ((r1.wwww)*(source[9].xyzx)).xyz;
    // 311: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 312: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 313: mad r0.xyz, cb0[13].zzzz, r0.yzwy, r4.xyzx
    r0.xyz = ((source[13].zzzz)*(r0.yzwy)+(r4.xyzx)).xyz;
    // 314: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 315: mul r4.xyz, r6.wwww, cb0[29].xyzx
    r4.xyz = ((r6.wwww)*(source[29].xyzx)).xyz;
    // 316: mad r4.xyz, r6.zzzz, cb0[28].xyzx, r4.xyzx
    r4.xyz = ((r6.zzzz)*(source[28].xyzx)+(r4.xyzx)).xyz;
    // 317: mul r4.xyz, r4.xyzx, cb0[30].wwww
    r4.xyz = ((r4.xyzx)*(source[30].wwww)).xyz;
    // 318: mul_sat r5.xyz, cb0[12].xyzx, cb0[12].wwww
    r5.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 319: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 320: mul r5.xyz, r5.xyzx, cb0[17].yyyy
    r5.xyz = ((r5.xyzx)*(source[17].yyyy)).xyz;
    // 321: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 322: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 323: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 324: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 325: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 326: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 327: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 328: mad o0.xyz, r1.xyzx, cb0[30].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[30].xyzx)+(r0.xyzx)).xyz;
    // 329: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 330: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 331: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 332: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 333: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 334: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 335: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 336: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 337: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 338: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 339: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 340: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 341: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 342: ftou r0.x, cb0[27].z
    r0.x = (asfloat((uint4)(source[27].zzzz))).x;
    // 343: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 344: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 345: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 346: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 347: ret
    return output;
}

// source.character.hair-ddk.v1 / source program dea8fd54f818c441b66600ac13b9ee61
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase99(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[0].x=1.0; source[1].w=1.0;
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

// source.vehicle.ancient-sea-realpbr.v1 / source program 96b15a1c8fa2034b92050219fbc9df22
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase100(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
    source[20].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
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
    // 8: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
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
    // 30: mad r2.xyz, cb0[17].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[17].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[18].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[18].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
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
    // 79: mad r4.xyz, cb0[17].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[17].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 80: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 81: dp3 r1.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 82: add r3.xyz, -r4.xyzx, r1.zzzz
    r3.xyz = ((-(r4.xyzx))+(r1.zzzz)).xyz;
    // 83: mad r3.xyz, cb0[18].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[18].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 84: mad r4.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mad r9.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 86: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 87: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 88: mul r9.xyz, r2.xyzx, r3.xyzx
    r9.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 89: dp3 r1.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 90: mad r10.xyz, -r3.xyzx, r2.xyzx, r1.zzzz
    r10.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r1.zzzz)).xyz;
    // 91: mad r2.xyz, r3.xyzx, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = ((r3.xyzx)*(r2.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 92: mad r3.xyz, cb0[17].wwww, r10.xyzx, r9.xyzx
    r3.xyz = ((source[17].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 93: dp3 r1.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 94: add r9.xyz, -r3.xyzx, r1.zzzz
    r9.xyz = ((-(r3.xyzx))+(r1.zzzz)).xyz;
    // 95: mad r3.xyz, cb0[18].xxxx, r9.xyzx, r3.xyzx
    r3.xyz = ((source[18].xxxx)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 96: mul r3.xyz, r4.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 97: mad r1.xyz, r1.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r1.xyz = ((r1.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 98: mad r1.xyz, r0.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 99: mul r0.x, r6.x, cb0[20].y
    r0.x = ((r6.xxxx)*(source[20].yyyy)).x;
    // 100: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 101: movc r0.x, r5.x, l(0), r0.x
    r0.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 102: add_sat r0.x, r0.x, cb0[20].z
    r0.x = (saturate((r0.xxxx)+(source[20].zzzz))).x;
    // 103: add r1.w, -r0.x, l(1.000000)
    r1.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 104: mul r3.xyz, r1.wwww, cb0[14].xyzx
    r3.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 105: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 106: mad r1.xyz, -r3.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r3.xyzx))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 107: mad r1.xyz, r0.xxxx, r1.xyzx, r4.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 108: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 109: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 110: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 111: mad r3.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 112: mad r4.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 113: mad r3.xyz, r0.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 114: mad r4.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 115: mad r3.xyz, r3.xyzx, r0.xxxx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // 116: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 117: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 118: mov_sat r1.w, cb0[21].y
    r1.w = (saturate(source[21].yyyy)).w;
    // 119: mad r4.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r4.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 120: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 121: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 122: mad r4.xyz, r8.wwww, r4.xyzx, r2.wwww
    r4.xyz = ((r8.wwww)*(r4.xyzx)+(r2.wwww)).xyz;
    // 123: add r1.w, -cb0[22].y, cb0[22].x
    r1.w = ((-(source[22].yyyy))+(source[22].xxxx)).w;
    // 124: mad r1.w, r7.x, r1.w, cb0[22].y
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[22].yyyy)).w;
    // 125: add r2.w, -r1.w, cb0[22].w
    r2.w = ((-(r1.wwww))+(source[22].wwww)).w;
    // 126: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 127: add r2.w, -r1.w, cb0[23].y
    r2.w = ((-(r1.wwww))+(source[23].yyyy)).w;
    // 128: mad r1.w, r7.z, r2.w, r1.w
    r1.w = ((r7.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 129: mul r1.w, r6.z, r1.w
    r1.w = ((r6.zzzz)*(r1.wwww)).w;
    // 130: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 131: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: movc r1.w, r5.z, l(0), r1.w
    r1.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 133: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 134: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 136: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 137: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 138: mul r5.xy, r5.xyxx, cb0[17].xxxx
    r5.xy = ((r5.xyxx)*(source[17].xxxx)).xy;
    // 139: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 141: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 142: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 143: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 144: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 145: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 146: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 147: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 148: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 149: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 150: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 151: mul r7.xyz, r1.wwww, v5.xyzx
    r7.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 152: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 153: deriv_rtx_coarse r8.x, r1.w
    r8.x = (ddx_coarse(r1.wwww)).x;
    // 154: deriv_rty_coarse r8.y, r1.w
    r8.y = (ddy_coarse(r1.wwww)).y;
    // 155: dp2 r2.w, r8.xyxx, r8.xyxx
    r2.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 156: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 157: mad r2.w, r2.w, l(0.300000), r8.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz)).w;
    // 158: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 159: min r8.y, r2.w, l(1.000000)
    r8.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 160: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: max r9.xyz, r4.xyzx, r2.wwww
    r9.xyz = (max(r4.xyzx,r2.wwww)).xyz;
    // 162: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 163: mul_sat r2.w, r4.y, l(50.000000)
    r2.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 164: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 165: mul r10.xyz, r1.wwww, r6.xyzx
    r10.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 166: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 167: add r2.w, r10.z, l(1.000000)
    r2.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 171: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 172: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 173: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 174: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 175: mul r1.w, r1.w, cb0[23].z
    r1.w = ((r1.wwww)*(source[23].zzzz)).w;
    // 176: add_sat r8.x, -r2.w, r3.w
    r8.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 177: sample_indexable(texture2d)(float,float,float,float) r11.xy, r8.xyxx, t5.xyzw, s6
    r11.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 178: add r2.w, r0.x, r8.x
    r2.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 179: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 180: mul r12.xyz, r4.xyzx, r11.yyyy
    r12.xyz = ((r4.xyzx)*(r11.yyyy)).xyz;
    // 181: mad r9.xyz, r9.xyzx, r11.xxxx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xxxx)+(r12.xyzx)).xyz;
    // 182: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r11.yyyy)).w;
    // 183: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 184: mad r11.xyz, r4.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r4.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: dp3 r3.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: mad r4.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 187: mad r12.xyz, -r9.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 189: mul r11.xyz, r1.xyzx, r12.xyzx
    r11.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 190: add r3.w, -r8.w, l(1.000000)
    r3.w = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mul r11.xyz, r3.wwww, r11.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)).xyz;
    // 192: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 193: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 194: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 195: mul r13.xyz, r5.wwww, v1.xyzx
    r13.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 196: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 197: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 198: mul r14.xyz, r5.wwww, v0.xyzx
    r14.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 199: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 200: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 201: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 202: dp3 r16.y, r15.xyzx, r6.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 203: dp3 r15.y, r15.xyzx, r10.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 204: dp3 r16.x, r14.xyzx, r6.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 205: dp3 r15.x, r14.xyzx, r10.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 206: dp2 r14.z, r16.xyxx, cb0[25].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 207: mul r8.xz, cb0[25].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[25].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 208: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 209: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 210: dp2 r17.z, r15.xyxx, cb0[25].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 211: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 212: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 213: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 214: dp4 r13.x, cb0[26].xyzw, r14.xyzw
    r13.x = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 215: dp4 r13.y, cb0[27].xyzw, r14.xyzw
    r13.y = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 216: dp4 r13.z, cb0[28].xyzw, r14.xyzw
    r13.z = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 217: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 218: dp4 r18.x, cb0[29].xyzw, r15.xyzw
    r18.x = (dot((source[29].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 219: dp4 r18.y, cb0[30].xyzw, r15.xyzw
    r18.y = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 220: dp4 r18.z, cb0[31].xyzw, r15.xyzw
    r18.z = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 221: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 222: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 223: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 224: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 225: mad r13.xyz, cb0[32].xyzx, r5.wwww, r13.xyzx
    r13.xyz = ((source[32].xyzx)*(r5.wwww)+(r13.xyzx)).xyz;
    // 226: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 227: mul r13.xyz, r13.xyzx, cb0[24].xyzx
    r13.xyz = ((r13.xyzx)*(source[24].xyzx)).xyz;
    // 228: mul r13.xyz, r13.xyzx, cb0[25].zzzz
    r13.xyz = ((r13.xyzx)*(source[25].zzzz)).xyz;
    // 229: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[24].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[24].wwww)).xyz;
    // 230: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 231: add r13.xyz, -r5.wwww, r13.xyzx
    r13.xyz = ((-(r5.wwww))+(r13.xyzx)).xyz;
    // 232: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 233: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
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
    // 244: mul r13.xyz, r5.wwww, r13.xyzx
    r13.xyz = ((r5.wwww)*(r13.xyzx)).xyz;
    // 245: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 246: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 247: mul r11.xyz, r3.xyzx, r11.xyzx
    r11.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 248: mul r5.w, r8.y, l(5.000000)
    r5.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 249: mul r7.w, r8.y, r8.y
    r7.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 250: mul r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)*(r7.wwww)).w;
    // 251: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 252: add r2.w, r0.x, r2.w
    r2.w = ((r0.xxxx)+(r2.wwww)).w;
    // 253: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 254: add_sat r0.x, r2.w, l(-1.000000)
    r0.x = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 255: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t6.xyzw, s5, r5.w
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r17.xyzx).xyz, (r5.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 256: mul r8.xyz, r13.xyzx, r13.wwww
    r8.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 257: mul r8.xyz, r8.xyzx, cb0[24].xyzx
    r8.xyz = ((r8.xyzx)*(source[24].xyzx)).xyz;
    // 258: mul r8.xyz, r8.xyzx, cb0[25].zzzz
    r8.xyz = ((r8.xyzx)*(source[25].zzzz)).xyz;
    // 259: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[24].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[24].wwww)).xyz;
    // 260: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 261: add r8.xyz, -r2.wwww, r8.xyzx
    r8.xyz = ((-(r2.wwww))+(r8.xyzx)).xyz;
    // 262: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 263: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 264: div r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)/(r6.wwww)).w;
    // 265: mad r2.w, r4.w, l(5.000000), r2.w
    r2.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 266: add_sat r2.w, r8.w, r2.w
    r2.w = (saturate((r8.wwww)+(r2.wwww))).w;
    // 267: mad r4.w, r2.w, l(-2.000000), l(3.000000)
    r4.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 268: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 269: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 270: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 271: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 272: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 273: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 274: mul r13.xyz, r8.xyzx, r9.xyzx
    r13.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 275: mad r2.w, r0.x, r4.x, r4.y
    r2.w = ((r0.xxxx)*(r4.xxxx)+(r4.yyyy)).w;
    // 276: mad r2.w, r2.w, r0.x, r4.z
    r2.w = ((r2.wwww)*(r0.xxxx)+(r4.zzzz)).w;
    // 277: mul r2.w, r0.x, r2.w
    r2.w = ((r0.xxxx)*(r2.wwww)).w;
    // 278: max r0.x, r0.x, r2.w
    r0.x = (max(r0.xxxx,r2.wwww)).x;
    // 279: mad r4.xyz, r13.xyzx, r0.xxxx, r11.xyzx
    r4.xyz = ((r13.xyzx)*(r0.xxxx)+(r11.xyzx)).xyz;
    // 280: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 281: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 282: mul r11.xyz, r2.wwww, v6.xyzx
    r11.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 283: dp3 r2.w, r11.xyzx, r6.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 284: dp3 r4.w, -r11.xyzx, r6.xyzx
    r4.w = (dot((-(r11.xyzx)).xyz,(r6.xyzx).xyz).xxxx).w;
    // 285: dp3 r5.w, r11.xyzx, r10.xyzx
    r5.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 286: mad r6.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 287: mad r6.zw, r4.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r4.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 288: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 289: mad r10.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 290: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 291: mul r10.yzw, r10.yyyy, cb0[35].xxyz
    r10.yzw = ((r10.yyyy)*(source[35].xxyz)).yzw;
    // 292: mad r10.xyz, r10.xxxx, cb0[34].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[34].xyzx)+(r10.yzwy)).xyz;
    // 293: mul r10.xyz, r10.xyzx, cb0[36].wwww
    r10.xyz = ((r10.xyzx)*(source[36].wwww)).xyz;
    // 294: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 295: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 296: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 297: mul r3.xyz, r12.xyzx, r3.xyzx
    r3.xyz = ((r12.xyzx)*(r3.xyzx)).xyz;
    // 298: mad r3.xyz, -r3.xyzx, r8.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r8.wwww)+(r3.xyzx)).xyz;
    // 299: mad r3.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 300: mul r4.xyz, r6.yyyy, cb0[35].xyzx
    r4.xyz = ((r6.yyyy)*(source[35].xyzx)).xyz;
    // 301: mad r4.xyz, cb0[34].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[34].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 302: mul r4.xyz, r4.xyzx, cb0[36].wwww
    r4.xyz = ((r4.xyzx)*(source[36].wwww)).xyz;
    // 303: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 304: mul r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)*(r4.xyzx)).xyz;
    // 305: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 306: mad r3.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 307: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 308: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 309: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 310: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 311: div r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 312: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 313: add r4.xyz, -r2.xyzx, r0.xxxx
    r4.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 314: add r2.xyz, r2.xyzx, -r4.xyzx
    r2.xyz = ((r2.xyzx)+(-(r4.xyzx))).xyz;
    // 315: dp3 r0.x, r5.xyzx, r7.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 316: mul_sat r2.w, r0.x, cb0[18].y
    r2.w = (saturate((r0.xxxx)*(source[18].yyyy))).w;
    // 317: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 318: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 319: mul_sat r4.x, r7.z, cb0[18].y
    r4.x = (saturate((r7.zzzz)*(source[18].yyyy))).x;
    // 320: add r4.y, -|r7.z|, l(1.000000)
    r4.y = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 321: mul r0.x, r0.x, r4.y
    r0.x = ((r0.xxxx)*(r4.yyyy)).x;
    // 322: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 323: add_sat r4.x, r4.x, -cb0[18].z
    r4.x = (saturate((r4.xxxx)+(-(source[18].zzzz)))).x;
    // 324: log r4.y, r4.x
    r4.y = (log2(r4.xxxx)).y;
    // 325: lt r4.x, r4.x, l(0.000001)
    r4.x = (asfloat((uint4)((r4.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 326: mul r4.y, r4.y, cb0[18].w
    r4.y = ((r4.yyyy)*(source[18].wwww)).y;
    // 327: exp r4.y, r4.y
    r4.y = (exp2(r4.yyyy)).y;
    // 328: mul r2.w, r2.w, r4.y
    r2.w = ((r2.wwww)*(r4.yyyy)).w;
    // 329: movc r2.w, r4.x, l(0), r2.w
    r2.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 330: mul r4.xyz, cb0[12].xyzx, cb0[19].yyyy
    r4.xyz = ((source[12].xyzx)*(source[19].yyyy)).xyz;
    // 331: mul r4.xyz, r4.xyzx, cb0[20].xxxx
    r4.xyz = ((r4.xyzx)*(source[20].xxxx)).xyz;
    // 332: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 333: mad r5.xyz, r2.wwww, cb0[11].xyzx, -cb0[11].xyzx
    r5.xyz = ((r2.wwww)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 334: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 335: mad r2.w, cb0[10].w, r2.w, l(1.000000)
    r2.w = ((source[10].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 336: mad r5.xyz, cb0[11].wwww, r5.xyzx, cb0[11].xyzx
    r5.xyz = ((source[11].wwww)*(r5.xyzx)+(source[11].xyzx)).xyz;
    // 337: mad r2.xyz, r2.xyzx, r4.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 338: mad r2.xyz, r2.wwww, cb0[10].xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(source[10].xyzx)+(r2.xyzx)).xyz;
    // 339: log r2.w, |r0.x|
    r2.w = (log2(abs(r0.xxxx))).w;
    // 340: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 341: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 342: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 343: mul r4.xyz, r2.wwww, cb0[13].xyzx
    r4.xyz = ((r2.wwww)*(source[13].xyzx)).xyz;
    // 344: movc r4.xyz, r0.xxxx, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 345: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 346: mad r0.xyz, cb0[17].zzzz, r0.yzwy, r2.xyzx
    r0.xyz = ((source[17].zzzz)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 347: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 348: mul r2.xyz, r6.wwww, cb0[35].xyzx
    r2.xyz = ((r6.wwww)*(source[35].xyzx)).xyz;
    // 349: mad r2.xyz, r6.zzzz, cb0[34].xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(source[34].xyzx)+(r2.xyzx)).xyz;
    // 350: mul r2.xyz, r2.xyzx, cb0[36].wwww
    r2.xyz = ((r2.xyzx)*(source[36].wwww)).xyz;
    // 351: mul_sat r4.xyz, cb0[16].xyzx, cb0[16].wwww
    r4.xyz = (saturate((source[16].xyzx)*(source[16].wwww))).xyz;
    // 352: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 353: mul r4.xyz, r4.xyzx, cb0[23].zzzz
    r4.xyz = ((r4.xyzx)*(source[23].zzzz)).xyz;
    // 354: dp3_sat o5.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 355: mul r4.xyz, r3.wwww, r5.xyzx
    r4.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
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
    // 361: mad o0.xyz, r1.xyzx, cb0[36].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[36].xyzx)+(r0.xyzx)).xyz;
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
    // 375: ftou r0.x, cb0[33].z
    r0.x = (asfloat((uint4)(source[33].zzzz))).x;
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

// source.character.guardianknight-sk-ddk-drg-00-body-mi-dead.v1 / source program 824358517f0ef0448e57d8ac93e923da
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase101(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[25].w=(g_SourceCharacterTime.xxxx).x;
    source[26].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[26].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[31] = g_SourceCharacterEnvironmentColor;
        source[32] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[27].wwww
    r0.xy = ((v4.xyxx)*(source[27].wwww)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[28].x
    r0.x = ((r0.xxxx)+(-(source[28].xxxx))).x;
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
    // 11: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 13: mad r0.xyz, cb0[24].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[24].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[25].xxxx, r2.xyzx, r0.xyzx
    r0.xyz = ((source[25].xxxx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 17: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 18: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 19: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 20: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 21: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 22: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 24: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 25: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 26: mul r0.w, r5.y, cb0[19].y
    r0.w = ((r5.yyyy)*(source[19].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: movc r0.w, r4.y, l(0), r0.w
    r0.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 31: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 32: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 33: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 34: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 35: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 36: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 37: mad r3.xyz, r0.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mad r2.xyz, r6.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r6.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 41: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 42: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 43: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 47: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 48: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 49: mad r2.xyz, r6.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 50: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 51: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 52: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 53: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 54: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 55: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 56: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 57: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 58: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 59: mad r2.xyz, r6.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 60: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 62: mad r3.xyz, cb0[24].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[24].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[25].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[25].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 67: mad r3.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 71: mul r8.xyz, r0.xyzx, r2.xyzx
    r8.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 72: dp3 r0.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r9.xyz, -r2.xyzx, r0.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 74: mad r0.xyz, r2.xyzx, r0.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 75: mad r2.xyz, cb0[24].wwww, r9.xyzx, r8.xyzx
    r2.xyz = ((source[24].wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r8.xyz, -r2.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[25].xxxx, r8.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[25].w
    r0.w = ((r0.wwww)*(source[25].wwww)).w;
    // 82: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 83: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 84: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r1.w, cb0[7].z, l(1.500000)
    r1.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 87: mad r0.w, r0.w, l(0.500000), cb0[7].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 88: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 89: mul r7.x, r1.w, l(0.125000)
    r7.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 90: mul r8.y, cb0[7].y, cb0[17].y
    r8.y = ((source[7].yyyy)*(source[17].yyyy)).y;
    // 91: mov r7.y, v4.y
    r7.y = (v4.yyyy).y;
    // 92: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 93: add r4.yw, r7.xxxy, r8.xxxy
    r4.yw = ((r7.xxxy)+(r8.xxxy)).yw;
    // 94: frc r1.w, cb0[7].x
    r1.w = (frac(source[7].xxxx)).w;
    // 95: add r2.w, -r1.w, cb0[7].x
    r2.w = ((-(r1.wwww))+(source[7].xxxx)).w;
    // 96: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 97: add r4.yw, r4.yyyw, r8.zzzw
    r4.yw = ((r4.yyyw)+(r8.zzzw)).yw;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r4.ywyy, t5.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 99: mul r8.xyz, r0.wwww, r8.xyzx
    r8.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 100: mul r0.w, r1.w, r8.w
    r0.w = ((r1.wwww)*(r8.wwww)).w;
    // 101: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 103: mad r2.xyz, r0.wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 104: mul r0.w, r5.x, cb0[26].z
    r0.w = ((r5.xxxx)*(source[26].zzzz)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[26].w
    r0.w = (saturate((r0.wwww)+(source[26].wwww))).w;
    // 108: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r4.xyw, r2.wwww, cb0[16].xyxz
    r4.xyw = ((r2.wwww)*(source[16].xyxz)).xyw;
    // 110: mul r5.xyw, r2.xyxz, r4.xyxw
    r5.xyw = ((r2.xyxz)*(r4.xyxw)).xyw;
    // 111: mad r2.xyz, -r4.xywx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r4.xywx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 112: mad r2.xyz, r0.wwww, r2.xyzx, r5.xywx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xywx)).xyz;
    // 113: add r4.xyw, -cb0[2].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r4.xyw = ((-(source[2].xyxz))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 114: mul r2.xyz, r2.xyzx, r4.xywx
    r2.xyz = ((r2.xyzx)*(r4.xywx)).xyz;
    // 115: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 116: mad r4.xyw, r2.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r4.xyw = ((r2.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 117: mad r5.xyw, r2.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r5.xyw = ((r2.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 118: mad r4.xyw, r0.wwww, r4.xyxw, r5.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)+(r5.xyxw)).xyw;
    // 119: mad r5.xyw, r2.xyxz, l(2.755200, 2.755200, 0.000000, 2.755200), l(0.690300, 0.690300, 0.000000, 0.690300)
    r5.xyw = ((r2.xyxz)*(float4(2.755200,2.755200,0.000000,2.755200))+(float4(0.690300,0.690300,0.000000,0.690300))).xyw;
    // 120: mad r4.xyw, r4.xyxw, r0.wwww, r5.xyxw
    r4.xyw = ((r4.xyxw)*(r0.wwww)+(r5.xyxw)).xyw;
    // 121: mul r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)).xyw;
    // 122: max r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = (max(r0.wwww,r4.xyxw)).xyw;
    // 123: mov_sat r2.w, cb0[27].z
    r2.w = (saturate(source[27].zzzz)).w;
    // 124: mad r5.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r5.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 125: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 126: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 127: mad r5.xyw, r7.wwww, r5.xyxw, r3.wwww
    r5.xyw = ((r7.wwww)*(r5.xyxw)+(r3.wwww)).xyw;
    // 128: add r2.w, cb0[28].w, -cb0[29].x
    r2.w = ((source[28].wwww)+(-(source[29].xxxx))).w;
    // 129: mad r2.w, r6.x, r2.w, cb0[29].x
    r2.w = ((r6.xxxx)*(r2.wwww)+(source[29].xxxx)).w;
    // 130: add r3.w, -r2.w, cb0[29].z
    r3.w = ((-(r2.wwww))+(source[29].zzzz)).w;
    // 131: mad r2.w, r6.y, r3.w, r2.w
    r2.w = ((r6.yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 132: add r3.w, -r2.w, cb0[30].x
    r3.w = ((-(r2.wwww))+(source[30].xxxx)).w;
    // 133: mad r2.w, r6.z, r3.w, r2.w
    r2.w = ((r6.zzzz)*(r3.wwww)+(r2.wwww)).w;
    // 134: mul r2.w, r5.z, r2.w
    r2.w = ((r5.zzzz)*(r2.wwww)).w;
    // 135: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 136: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: movc r2.w, r4.z, l(0), r2.w
    r2.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 138: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 139: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 141: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 142: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 143: mul r6.xy, r6.xyxx, cb0[19].xxxx
    r6.xy = ((r6.xyxx)*(source[19].xxxx)).xy;
    // 144: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 147: add r6.z, r2.w, l(0.000010)
    r6.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 148: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 149: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 150: div r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)/(r2.wwww)).xyz;
    // 151: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 152: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 153: mul r8.xyz, r2.wwww, r6.xyzx
    r8.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 154: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 156: mul r9.xyz, r2.wwww, v5.xyzx
    r9.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r2.w, r8.xyzx, r9.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 158: deriv_rtx_coarse r7.x, r2.w
    r7.x = (ddx_coarse(r2.wwww)).x;
    // 159: deriv_rty_coarse r7.y, r2.w
    r7.y = (ddy_coarse(r2.wwww)).y;
    // 160: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 161: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 162: mad r3.w, r3.w, l(0.300000), r7.z
    r3.w = ((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).w;
    // 163: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 164: min r7.y, r3.w, l(1.000000)
    r7.y = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r10.xyz, r5.xywx, r3.wwww
    r10.xyz = (max(r5.xywx,r3.wwww)).xyz;
    // 167: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 168: mul_sat r3.w, r5.y, l(50.000000)
    r3.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 170: mul r11.xyz, r2.wwww, r8.xyzx
    r11.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 171: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 172: add r3.w, r11.z, l(1.000000)
    r3.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r4.z, r2.w, l(1.000000)
    r4.z = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mov_sat r2.w, r2.w
    r2.w = (saturate(r2.wwww)).w;
    // 176: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 177: mul r2.w, r2.w, cb0[1].y
    r2.w = ((r2.wwww)*(source[1].yyyy)).w;
    // 178: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 179: mad_sat r2.w, r2.w, cb0[1].w, cb0[1].z
    r2.w = (saturate((r2.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 180: mul r2.w, r2.w, cb0[30].y
    r2.w = ((r2.wwww)*(source[30].yyyy)).w;
    // 181: add_sat r7.x, -r3.w, r4.z
    r7.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t6.xyzw, s7
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 183: add r3.w, r0.w, r7.x
    r3.w = ((r0.wwww)+(r7.xxxx)).w;
    // 184: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 185: mul r13.xyz, r5.xywx, r12.yyyy
    r13.xyz = ((r5.xywx)*(r12.yyyy)).xyz;
    // 186: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 187: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.yyyy)).z;
    // 188: add r4.z, r4.z, l(-1.000000)
    r4.z = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 189: mad r12.xyz, r5.xywx, r4.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r4.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 190: dp3 r4.z, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.z = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: mad r5.xyz, r4.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r4.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 194: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 195: add r4.z, -r7.w, l(1.000000)
    r4.z = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 196: mul r12.xyz, r4.zzzz, r12.xyzx
    r12.xyz = ((r4.zzzz)*(r12.xyzx)).xyz;
    // 197: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: mad r6.w, r7.y, l(2.000000), l(2.000000)
    r6.w = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 199: dp3 r7.x, v1.xyzx, v1.xyzx
    r7.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 200: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 201: mul r14.xyz, r7.xxxx, v1.xyzx
    r14.xyz = ((r7.xxxx)*(v1.xyzx)).xyz;
    // 202: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 203: dp3 r7.x, v0.xyzx, v0.xyzx
    r7.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 204: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 205: mul r16.xyz, r7.xxxx, v0.xyzx
    r16.xyz = ((r7.xxxx)*(v0.xyzx)).xyz;
    // 206: mul r17.xyz, r14.zxyz, r16.yzxy
    r17.xyz = ((r14.zxyz)*(r16.yzxy)).xyz;
    // 207: mad r17.xyz, r14.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r14.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 208: dp3 r14.y, r14.xyzx, r11.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 209: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 210: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 211: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 212: dp2 r15.z, r18.xyxx, cb0[32].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[32].xyxx).xy).xxxx).z;
    // 213: mul r7.xz, cb0[32].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[32].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 214: dp2 r15.x, r18.xyxx, r7.xzxx
    r15.x = (dot((r18.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 215: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 216: dp4 r19.x, cb0[33].xyzw, r15.xyzw
    r19.x = (dot((source[33].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 217: dp4 r19.y, cb0[34].xyzw, r15.xyzw
    r19.y = (dot((source[34].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 218: dp4 r19.z, cb0[35].xyzw, r15.xyzw
    r19.z = (dot((source[35].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 219: mul r20.xyzw, r15.yzzx, r15.xyzz
    r20.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 220: dp4 r21.x, cb0[36].xyzw, r20.xyzw
    r21.x = (dot((source[36].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).x;
    // 221: dp4 r21.y, cb0[37].xyzw, r20.xyzw
    r21.y = (dot((source[37].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).y;
    // 222: dp4 r21.z, cb0[38].xyzw, r20.xyzw
    r21.z = (dot((source[38].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).z;
    // 223: add r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)+(r21.xyzx)).xyz;
    // 224: mul r8.w, r15.y, r15.y
    r8.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 225: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 226: mad r8.w, r15.x, r15.x, -r8.w
    r8.w = ((r15.xxxx)*(r15.xxxx)+(-(r8.wwww))).w;
    // 227: mad r15.xyz, cb0[39].xyzx, r8.wwww, r19.xyzx
    r15.xyz = ((source[39].xyzx)*(r8.wwww)+(r19.xyzx)).xyz;
    // 228: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 229: mul r15.xyz, r15.xyzx, cb0[31].xyzx
    r15.xyz = ((r15.xyzx)*(source[31].xyzx)).xyz;
    // 230: mul r15.xyz, r15.xyzx, cb0[32].zzzz
    r15.xyz = ((r15.xyzx)*(source[32].zzzz)).xyz;
    // 231: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[31].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[31].wwww)).xyz;
    // 232: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: add r15.xyz, -r8.wwww, r15.xyzx
    r15.xyz = ((-(r8.wwww))+(r15.xyzx)).xyz;
    // 234: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r8.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r8.wwww)).xyz;
    // 235: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: div r8.w, r8.w, r6.w
    r8.w = ((r8.wwww)/(r6.wwww)).w;
    // 237: mad r8.w, r5.w, l(5.000000), r8.w
    r8.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r8.wwww)).w;
    // 238: add_sat r8.w, r7.w, r8.w
    r8.w = (saturate((r7.wwww)+(r8.wwww))).w;
    // 239: mad r9.w, r8.w, l(-2.000000), l(3.000000)
    r9.w = ((r8.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 240: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 241: mul r8.w, r8.w, r9.w
    r8.w = ((r8.wwww)*(r9.wwww)).w;
    // 242: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 243: mul r8.w, r8.w, l(1.500000)
    r8.w = ((r8.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 244: exp r8.w, r8.w
    r8.w = (exp2(r8.wwww)).w;
    // 245: mul r15.xyz, r8.wwww, r15.xyzx
    r15.xyz = ((r8.wwww)*(r15.xyzx)).xyz;
    // 246: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 247: mul r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r15.xyzx)).xyz;
    // 248: mul r12.xyz, r4.xywx, r12.xyzx
    r12.xyz = ((r4.xywx)*(r12.xyzx)).xyz;
    // 249: mul r8.w, r7.y, l(5.000000)
    r8.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 250: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 251: mul r3.w, r3.w, r7.y
    r3.w = ((r3.wwww)*(r7.yyyy)).w;
    // 252: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 253: add r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)+(r3.wwww)).w;
    // 254: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 255: add_sat r0.w, r3.w, l(-1.000000)
    r0.w = (saturate((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 256: dp3 r15.x, r16.xyzx, r11.xyzx
    r15.x = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 257: dp3 r15.y, r17.xyzx, r11.xyzx
    r15.y = (dot((r17.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 258: dp2 r14.x, r15.xyxx, r7.xzxx
    r14.x = (dot((r15.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 259: dp2 r14.z, r15.xyxx, cb0[32].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[32].xyxx).xy).xxxx).z;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t7.xyzw, s6, r8.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r8.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 261: mul r7.xyz, r14.xyzx, r14.wwww
    r7.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 262: mul r7.xyz, r7.xyzx, cb0[31].xyzx
    r7.xyz = ((r7.xyzx)*(source[31].xyzx)).xyz;
    // 263: mul r7.xyz, r7.xyzx, cb0[32].zzzz
    r7.xyz = ((r7.xyzx)*(source[32].zzzz)).xyz;
    // 264: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[31].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[31].wwww)).xyz;
    // 265: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r7.xyz, -r3.wwww, r7.xyzx
    r7.xyz = ((-(r3.wwww))+(r7.xyzx)).xyz;
    // 267: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 268: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)/(r6.wwww)).w;
    // 270: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 271: add_sat r3.w, r7.w, r3.w
    r3.w = (saturate((r7.wwww)+(r3.wwww))).w;
    // 272: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 274: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 275: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 276: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 278: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 279: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 280: mad r3.w, r0.w, r5.x, r5.y
    r3.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 281: mad r3.w, r3.w, r0.w, r5.z
    r3.w = ((r3.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 282: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 283: max r0.w, r0.w, r3.w
    r0.w = (max(r0.wwww,r3.wwww)).w;
    // 284: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 285: dp3 r3.w, v6.xyzx, v6.xyzx
    r3.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 287: mul r12.xyz, r3.wwww, v6.xyzx
    r12.xyz = ((r3.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r3.w, r12.xyzx, r8.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 289: dp3 r5.w, -r12.xyzx, r8.xyzx
    r5.w = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).w;
    // 290: dp3 r6.w, r12.xyzx, r11.xyzx
    r6.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 291: mad r8.xy, r6.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r6.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 292: mad r8.zw, r5.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r5.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 293: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 294: mad r11.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 296: mul r11.yzw, r11.yyyy, cb0[42].xxyz
    r11.yzw = ((r11.yyyy)*(source[42].xxyz)).yzw;
    // 297: mad r11.xyz, r11.xxxx, cb0[41].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[41].xyzx)+(r11.yzwy)).xyz;
    // 298: mul r11.xyz, r11.xyzx, cb0[43].wwww
    r11.xyz = ((r11.xyzx)*(source[43].wwww)).xyz;
    // 299: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 300: mul r4.xyw, r4.xyxw, r11.xyxz
    r4.xyw = ((r4.xyxw)*(r11.xyxz)).xyw;
    // 301: mul r4.xyw, r4.xyxw, l(0.600000, 0.600000, 0.000000, 0.600000)
    r4.xyw = ((r4.xyxw)*(float4(0.600000,0.600000,0.000000,0.600000))).xyw;
    // 302: mul r4.xyw, r13.xyxz, r4.xyxw
    r4.xyw = ((r13.xyxz)*(r4.xyxw)).xyw;
    // 303: mad r4.xyw, -r4.xyxw, r7.wwww, r4.xyxw
    r4.xyw = ((-(r4.xyxw))*(r7.wwww)+(r4.xyxw)).xyw;
    // 304: mad r4.xyw, r5.xyxz, l(0.400000, 0.400000, 0.000000, 0.400000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.400000,0.400000,0.000000,0.400000))+(r4.xyxw)).xyw;
    // 305: mul r5.xyz, r8.yyyy, cb0[42].xyzx
    r5.xyz = ((r8.yyyy)*(source[42].xyzx)).xyz;
    // 306: mad r5.xyz, cb0[41].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[41].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, cb0[43].wwww
    r5.xyz = ((r5.xyzx)*(source[43].wwww)).xyz;
    // 308: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 309: mul r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 310: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 311: mad r4.xyw, r5.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r4.xyxw)).xyw;
    // 312: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 313: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 314: mul r0.w, r17.z, cb0[20].w
    r0.w = ((r17.zzzz)*(source[20].wwww)).w;
    // 315: mul r3.w, r17.z, cb0[21].x
    r3.w = ((r17.zzzz)*(source[21].xxxx)).w;
    // 316: mad r5.x, r16.z, cb0[20].w, -r3.w
    r5.x = ((r16.zzzz)*(source[20].wwww)+(-(r3.wwww))).x;
    // 317: mad r5.y, r16.z, cb0[21].x, r0.w
    r5.y = ((r16.zzzz)*(source[21].xxxx)+(r0.wwww)).y;
    // 318: max r0.w, |r5.x|, |r5.y|
    r0.w = (max(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 319: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 320: min r3.w, |r5.x|, |r5.y|
    r3.w = (min(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 321: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 322: mul r3.w, r0.w, r0.w
    r3.w = ((r0.wwww)*(r0.wwww)).w;
    // 323: mad r5.z, r3.w, l(0.020835), l(-0.085133)
    r5.z = ((r3.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 324: mad r5.z, r3.w, r5.z, l(0.180141)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 325: mad r5.z, r3.w, r5.z, l(-0.330299)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 326: mad r3.w, r3.w, r5.z, l(0.999866)
    r3.w = ((r3.wwww)*(r5.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 327: mul r5.z, r0.w, r3.w
    r5.z = ((r0.wwww)*(r3.wwww)).z;
    // 328: mad r5.z, r5.z, l(-2.000000), l(1.570796)
    r5.z = ((r5.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 329: lt r5.w, |r5.x|, |r5.y|
    r5.w = (asfloat((uint4)((abs(r5.xxxx))<(abs(r5.yyyy))) * 0xffffffffu)).w;
    // 330: and r5.z, r5.w, r5.z
    r5.z = (asfloat(asuint(r5.wwww) & asuint(r5.zzzz))).z;
    // 331: mad r0.w, r0.w, r3.w, r5.z
    r0.w = ((r0.wwww)*(r3.wwww)+(r5.zzzz)).w;
    // 332: lt r3.w, r5.x, -r5.x
    r3.w = (asfloat((uint4)((r5.xxxx)<(-(r5.xxxx))) * 0xffffffffu)).w;
    // 333: and r3.w, r3.w, l(0xc0490fdb)
    r3.w = (asfloat(asuint(r3.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 334: add r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)+(r3.wwww)).w;
    // 335: min r3.w, r5.x, r5.y
    r3.w = (min(r5.xxxx,r5.yyyy)).w;
    // 336: lt r3.w, r3.w, -r3.w
    r3.w = (asfloat((uint4)((r3.wwww)<(-(r3.wwww))) * 0xffffffffu)).w;
    // 337: max r5.z, r5.x, r5.y
    r5.z = (max(r5.xxxx,r5.yyyy)).z;
    // 338: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 339: add r5.x, r5.y, r5.x
    r5.x = ((r5.yyyy)+(r5.xxxx)).x;
    // 340: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 341: ge r5.y, r5.z, -r5.z
    r5.y = (asfloat((uint4)((r5.zzzz)>=(-(r5.zzzz))) * 0xffffffffu)).y;
    // 342: and r3.w, r3.w, r5.y
    r3.w = (asfloat(asuint(r3.wwww) & asuint(r5.yyyy))).w;
    // 343: movc r0.w, r3.w, -r0.w, r0.w
    r0.w = ((asuint(r3.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 344: add r3.w, r0.w, l(6.283185)
    r3.w = ((r0.wwww)+(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 345: mul r3.w, r3.w, l(0.159155)
    r3.w = ((r3.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 346: mul r5.y, r0.w, l(0.159155)
    r5.y = ((r0.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).y;
    // 347: ge r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 348: movc r0.w, r0.w, r5.y, r3.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (r5.yyyy) : (r3.wwww)).w;
    // 349: add r3.w, -r0.w, -cb0[21].w
    r3.w = ((-(r0.wwww))+(-(source[21].wwww))).w;
    // 350: add r0.w, r0.w, -cb0[21].w
    r0.w = ((r0.wwww)+(-(source[21].wwww))).w;
    // 351: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 352: add r5.y, -cb0[21].w, l(1.000000)
    r5.y = ((-(source[21].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 353: div r5.y, l(1.000000, 1.000000, 1.000000, 1.000000), r5.y
    r5.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r5.yyyy)).y;
    // 354: mul_sat r3.w, r3.w, r5.y
    r3.w = (saturate((r3.wwww)*(r5.yyyy))).w;
    // 355: mul_sat r0.w, r0.w, r5.y
    r0.w = (saturate((r0.wwww)*(r5.yyyy))).w;
    // 356: mad r5.y, r3.w, l(-2.000000), l(3.000000)
    r5.y = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 357: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 358: mul r3.w, r3.w, r5.y
    r3.w = ((r3.wwww)*(r5.yyyy)).w;
    // 359: mad r5.y, r0.w, l(-2.000000), l(3.000000)
    r5.y = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 360: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 361: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 362: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 363: log r3.w, r5.x
    r3.w = (log2(r5.xxxx)).w;
    // 364: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 365: mul r3.w, r3.w, l(5.082000)
    r3.w = ((r3.wwww)*(float4(5.082000,5.082000,5.082000,5.082000))).w;
    // 366: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 367: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 368: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 369: dp3 r3.w, r6.xyzx, r9.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 370: mul_sat r5.x, r3.w, cb0[22].x
    r5.x = (saturate((r3.wwww)*(source[22].xxxx))).x;
    // 371: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 372: mul_sat r5.y, r9.z, cb0[22].x
    r5.y = (saturate((r9.zzzz)*(source[22].xxxx))).y;
    // 373: add r5.z, -|r9.z|, l(1.000000)
    r5.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 374: mul r3.w, r3.w, r5.z
    r3.w = ((r3.wwww)*(r5.zzzz)).w;
    // 375: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 376: add_sat r5.y, r5.y, -cb0[22].y
    r5.y = (saturate((r5.yyyy)+(-(source[22].yyyy)))).y;
    // 377: log r5.z, r5.y
    r5.z = (log2(r5.yyyy)).z;
    // 378: lt r5.y, r5.y, l(0.000001)
    r5.y = (asfloat((uint4)((r5.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 379: mul r5.z, r5.z, cb0[22].z
    r5.z = ((r5.zzzz)*(source[22].zzzz)).z;
    // 380: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 381: mul r5.x, r5.z, r5.x
    r5.x = ((r5.zzzz)*(r5.xxxx)).x;
    // 382: movc r5.x, r5.y, l(0), r5.x
    r5.x = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).x;
    // 383: mul r5.y, r5.x, cb0[22].w
    r5.y = ((r5.xxxx)*(source[22].wwww)).y;
    // 384: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 385: mul r0.w, r0.w, cb0[22].w
    r0.w = ((r0.wwww)*(source[22].wwww)).w;
    // 386: lt r5.y, |r0.w|, l(0.000001)
    r5.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 387: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 388: mul r0.w, r0.w, cb0[23].x
    r0.w = ((r0.wwww)*(source[23].xxxx)).w;
    // 389: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 390: mul r6.xyz, cb0[8].xyzx, cb0[8].wwww
    r6.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 391: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 392: add r5.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r5.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 393: dp2 r0.w, cb0[9].xyxx, r5.zwzz
    r0.w = (dot((source[9].xyxx).xy,(r5.zwzz).xy).xxxx).w;
    // 394: add r0.w, r0.w, cb0[24].z
    r0.w = ((r0.wwww)+(source[24].zzzz)).w;
    // 395: add_sat r0.w, r0.w, l(-0.500000)
    r0.w = (saturate((r0.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).w;
    // 396: mul r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)*(r0.wwww)).xyz;
    // 397: movc r5.yzw, r5.yyyy, l(0,0,0,0), r6.xxyz
    r5.yzw = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxyz)).yzw;
    // 398: mul r6.xyz, r1.wwww, r5.yzwy
    r6.xyz = ((r1.wwww)*(r5.yzwy)).xyz;
    // 399: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 400: mad r5.yzw, -r1.wwww, r5.yyzw, r0.wwww
    r5.yzw = ((-(r1.wwww))*(r5.yyzw)+(r0.wwww)).yzw;
    // 401: mad r5.yzw, cb0[24].wwww, r5.yyzw, r6.xxyz
    r5.yzw = ((source[24].wwww)*(r5.yyzw)+(r6.xxyz)).yzw;
    // 402: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 403: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 404: mad r5.yzw, cb0[25].xxxx, r6.xxyz, r5.yyzw
    r5.yzw = ((source[25].xxxx)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 405: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 406: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 407: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 408: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 409: add r6.xyz, -r0.xyzx, r0.wwww
    r6.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 410: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 411: mul r6.xyz, cb0[14].xyzx, cb0[25].zzzz
    r6.xyz = ((source[14].xyzx)*(source[25].zzzz)).xyz;
    // 412: mul r6.xyz, r6.xyzx, cb0[26].yyyy
    r6.xyz = ((r6.xyzx)*(source[26].yyyy)).xyz;
    // 413: mul r6.xyz, r5.xxxx, r6.xyzx
    r6.xyz = ((r5.xxxx)*(r6.xyzx)).xyz;
    // 414: mad r7.xyz, r5.xxxx, cb0[13].xyzx, -cb0[13].xyzx
    r7.xyz = ((r5.xxxx)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 415: add r0.w, r5.x, l(-1.000000)
    r0.w = ((r5.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 416: mad r0.w, cb0[12].w, r0.w, l(1.000000)
    r0.w = ((source[12].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 417: mad r7.xyz, cb0[13].wwww, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((source[13].wwww)*(r7.xyzx)+(source[13].xyzx)).xyz;
    // 418: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 419: mad r0.xyz, r0.wwww, cb0[12].xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(source[12].xyzx)+(r0.xyzx)).xyz;
    // 420: mad r0.xyz, r5.yzwy, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.yzwy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 421: log r0.w, |r3.w|
    r0.w = (log2(abs(r3.wwww))).w;
    // 422: lt r1.w, |r3.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 423: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 424: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 425: mul r3.xyz, r0.wwww, cb0[15].xyzx
    r3.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 426: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 427: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 428: mad r0.xyz, cb0[19].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 429: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 430: mul r1.xyz, r8.wwww, cb0[42].xyzx
    r1.xyz = ((r8.wwww)*(source[42].xyzx)).xyz;
    // 431: mad r1.xyz, r8.zzzz, cb0[41].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[41].xyzx)+(r1.xyzx)).xyz;
    // 432: mul r1.xyz, r1.xyzx, cb0[43].wwww
    r1.xyz = ((r1.xyzx)*(source[43].wwww)).xyz;
    // 433: mul_sat r3.xyz, cb0[18].xyzx, cb0[18].wwww
    r3.xyz = (saturate((source[18].xyzx)*(source[18].wwww))).xyz;
    // 434: mul r5.xyz, r2.wwww, r3.xyzx
    r5.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 435: mul r3.xyz, r3.xyzx, cb0[30].yyyy
    r3.xyz = ((r3.xyzx)*(source[30].yyyy)).xyz;
    // 436: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 437: mul r3.xyz, r4.zzzz, r5.xyzx
    r3.xyz = ((r4.zzzz)*(r5.xyzx)).xyz;
    // 438: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 439: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 440: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 441: add r0.xyz, r4.xywx, r0.xyzx
    r0.xyz = ((r4.xywx)+(r0.xyzx)).xyz;
    // 442: dp3 o4.y, r4.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 443: mad o0.xyz, r2.xyzx, cb0[43].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[43].xyzx)+(r0.xyzx)).xyz;
    // 444: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 445: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 446: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 447: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 448: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 449: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 450: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 451: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 452: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 453: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 454: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 455: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 456: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 457: ftou r0.x, cb0[40].z
    r0.x = (asfloat((uint4)(source[40].zzzz))).x;
    // 458: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 459: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 460: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 461: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 462: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drg-00-head-mi-dead.v1 / source program fcf2ae320f020d48b6a207cedc870d36
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase102(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[21]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[29].z=(g_SourceCharacterTime.xxxx).x;
    source[31].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[32].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[32].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[32].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[32].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[37] = g_SourceCharacterEnvironmentColor;
        source[38] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[34].yyyy
    r0.xy = ((v4.xyxx)*(source[34].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[34].z
    r0.x = ((r0.xxxx)+(-(source[34].zzzz))).x;
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
    // 11: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 13: mad r0.xyz, cb0[30].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[30].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[30].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[30].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 17: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 18: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 19: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 20: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 21: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 22: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 24: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 25: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 26: mul r0.w, r5.y, cb0[23].y
    r0.w = ((r5.yyyy)*(source[23].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: movc r0.w, r4.y, l(0), r0.w
    r0.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 31: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 32: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 33: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 34: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 35: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 36: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 37: mad r3.xyz, r0.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mad r2.xyz, r6.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r6.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 41: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 42: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 43: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 47: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 48: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 49: mad r2.xyz, r6.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 50: mul r3.xyz, cb0[7].xyzx, cb0[7].wwww
    r3.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 51: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 52: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 53: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 54: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 55: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 56: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 57: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 58: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 59: mad r2.xyz, r6.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 60: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 62: mad r3.xyz, cb0[30].zzzz, r3.xyzx, r2.xyzx
    r3.xyz = ((source[30].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[30].wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((source[30].wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 67: mad r3.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 71: mul r8.xyz, r0.xyzx, r2.xyzx
    r8.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 72: dp3 r0.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r9.xyz, -r2.xyzx, r0.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 74: mad r0.xyz, r2.xyzx, r0.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 75: mad r2.xyz, cb0[30].zzzz, r9.xyzx, r8.xyzx
    r2.xyz = ((source[30].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r8.xyz, -r2.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[30].wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((source[30].wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[8].w, l(1.000000)
    r0.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[29].z
    r0.w = ((r0.wwww)*(source[29].zzzz)).w;
    // 82: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 83: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 84: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r1.w, cb0[8].z, l(1.500000)
    r1.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 87: mad r0.w, r0.w, l(0.500000), cb0[8].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 88: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 89: mul r7.x, r1.w, l(0.125000)
    r7.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 90: mul r8.y, cb0[8].y, cb0[21].y
    r8.y = ((source[8].yyyy)*(source[21].yyyy)).y;
    // 91: mov r7.y, v4.y
    r7.y = (v4.yyyy).y;
    // 92: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 93: add r4.yw, r7.xxxy, r8.xxxy
    r4.yw = ((r7.xxxy)+(r8.xxxy)).yw;
    // 94: frc r1.w, cb0[8].x
    r1.w = (frac(source[8].xxxx)).w;
    // 95: add r2.w, -r1.w, cb0[8].x
    r2.w = ((-(r1.wwww))+(source[8].xxxx)).w;
    // 96: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 97: add r4.yw, r4.yyyw, r8.zzzw
    r4.yw = ((r4.yyyw)+(r8.zzzw)).yw;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r4.ywyy, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 99: mul r8.xyz, r0.wwww, r8.xyzx
    r8.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 100: mul r0.w, r1.w, r8.w
    r0.w = ((r1.wwww)*(r8.wwww)).w;
    // 101: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 103: mad r2.xyz, r0.wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 104: mul r0.w, r5.x, cb0[33].x
    r0.w = ((r5.xxxx)*(source[33].xxxx)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[33].y
    r0.w = (saturate((r0.wwww)+(source[33].yyyy))).w;
    // 108: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r4.xyw, r2.wwww, cb0[20].xyxz
    r4.xyw = ((r2.wwww)*(source[20].xyxz)).xyw;
    // 110: mul r5.xyw, r2.xyxz, r4.xyxw
    r5.xyw = ((r2.xyxz)*(r4.xyxw)).xyw;
    // 111: mad r2.xyz, -r4.xywx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r4.xywx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 112: mad r2.xyz, r0.wwww, r2.xyzx, r5.xywx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xywx)).xyz;
    // 113: add r4.xyw, -cb0[3].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r4.xyw = ((-(source[3].xyxz))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 114: mul r2.xyz, r2.xyzx, r4.xywx
    r2.xyz = ((r2.xyzx)*(r4.xywx)).xyz;
    // 115: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 116: mad r4.xyw, r2.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r4.xyw = ((r2.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 117: mad r5.xyw, r2.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r5.xyw = ((r2.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 118: mad r4.xyw, r0.wwww, r4.xyxw, r5.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)+(r5.xyxw)).xyw;
    // 119: mad r5.xyw, r2.xyxz, l(2.755200, 2.755200, 0.000000, 2.755200), l(0.690300, 0.690300, 0.000000, 0.690300)
    r5.xyw = ((r2.xyxz)*(float4(2.755200,2.755200,0.000000,2.755200))+(float4(0.690300,0.690300,0.000000,0.690300))).xyw;
    // 120: mad r4.xyw, r4.xyxw, r0.wwww, r5.xyxw
    r4.xyw = ((r4.xyxw)*(r0.wwww)+(r5.xyxw)).xyw;
    // 121: mul r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)).xyw;
    // 122: max r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = (max(r0.wwww,r4.xyxw)).xyw;
    // 123: mov_sat r2.w, cb0[34].x
    r2.w = (saturate(source[34].xxxx)).w;
    // 124: mad r5.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r5.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 125: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 126: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 127: mad r5.xyw, r7.wwww, r5.xyxw, r3.wwww
    r5.xyw = ((r7.wwww)*(r5.xyxw)+(r3.wwww)).xyw;
    // 128: add r2.w, -cb0[35].z, cb0[35].y
    r2.w = ((-(source[35].zzzz))+(source[35].yyyy)).w;
    // 129: mad r2.w, r6.x, r2.w, cb0[35].z
    r2.w = ((r6.xxxx)*(r2.wwww)+(source[35].zzzz)).w;
    // 130: add r3.w, -r2.w, cb0[36].x
    r3.w = ((-(r2.wwww))+(source[36].xxxx)).w;
    // 131: mad r2.w, r6.y, r3.w, r2.w
    r2.w = ((r6.yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 132: add r3.w, -r2.w, cb0[36].z
    r3.w = ((-(r2.wwww))+(source[36].zzzz)).w;
    // 133: mad r2.w, r6.z, r3.w, r2.w
    r2.w = ((r6.zzzz)*(r3.wwww)+(r2.wwww)).w;
    // 134: mul r2.w, r5.z, r2.w
    r2.w = ((r5.zzzz)*(r2.wwww)).w;
    // 135: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 136: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: movc r2.w, r4.z, l(0), r2.w
    r2.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 138: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 139: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 141: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 142: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 143: mul r6.xy, r6.xyxx, cb0[23].xxxx
    r6.xy = ((r6.xyxx)*(source[23].xxxx)).xy;
    // 144: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 147: add r6.z, r2.w, l(0.000010)
    r6.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 148: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 149: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 150: div r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)/(r2.wwww)).xyz;
    // 151: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 152: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 153: mul r8.xyz, r2.wwww, r6.xyzx
    r8.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 154: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 156: mul r9.xyz, r2.wwww, v5.xyzx
    r9.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r2.w, r8.xyzx, r9.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 158: deriv_rtx_coarse r7.x, r2.w
    r7.x = (ddx_coarse(r2.wwww)).x;
    // 159: deriv_rty_coarse r7.y, r2.w
    r7.y = (ddy_coarse(r2.wwww)).y;
    // 160: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 161: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 162: mad r3.w, r3.w, l(0.300000), r7.z
    r3.w = ((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).w;
    // 163: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 164: min r7.y, r3.w, l(1.000000)
    r7.y = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r10.xyz, r5.xywx, r3.wwww
    r10.xyz = (max(r5.xywx,r3.wwww)).xyz;
    // 167: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 168: mul_sat r3.w, r5.y, l(50.000000)
    r3.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 170: mul r11.xyz, r2.wwww, r8.xyzx
    r11.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 171: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 172: add r3.w, r11.z, l(1.000000)
    r3.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r4.z, r2.w, l(1.000000)
    r4.z = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mov_sat r2.w, r2.w
    r2.w = (saturate(r2.wwww)).w;
    // 176: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 177: mul r2.w, r2.w, cb0[2].y
    r2.w = ((r2.wwww)*(source[2].yyyy)).w;
    // 178: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 179: mad_sat r2.w, r2.w, cb0[2].w, cb0[2].z
    r2.w = (saturate((r2.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 180: mul r2.w, r2.w, cb0[36].w
    r2.w = ((r2.wwww)*(source[36].wwww)).w;
    // 181: add_sat r7.x, -r3.w, r4.z
    r7.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t8.xyzw, s9
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 183: add r3.w, r0.w, r7.x
    r3.w = ((r0.wwww)+(r7.xxxx)).w;
    // 184: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 185: mul r13.xyz, r5.xywx, r12.yyyy
    r13.xyz = ((r5.xywx)*(r12.yyyy)).xyz;
    // 186: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 187: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.yyyy)).z;
    // 188: add r4.z, r4.z, l(-1.000000)
    r4.z = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 189: mad r12.xyz, r5.xywx, r4.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r4.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 190: dp3 r4.z, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.z = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: mad r5.xyz, r4.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r4.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 194: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 195: add r4.z, -r7.w, l(1.000000)
    r4.z = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 196: mul r12.xyz, r4.zzzz, r12.xyzx
    r12.xyz = ((r4.zzzz)*(r12.xyzx)).xyz;
    // 197: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: mad r6.w, r7.y, l(2.000000), l(2.000000)
    r6.w = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 199: dp3 r7.x, v1.xyzx, v1.xyzx
    r7.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 200: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 201: mul r14.xyz, r7.xxxx, v1.xyzx
    r14.xyz = ((r7.xxxx)*(v1.xyzx)).xyz;
    // 202: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 203: dp3 r7.x, v0.xyzx, v0.xyzx
    r7.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 204: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 205: mul r16.xyz, r7.xxxx, v0.xyzx
    r16.xyz = ((r7.xxxx)*(v0.xyzx)).xyz;
    // 206: mul r17.xyz, r14.zxyz, r16.yzxy
    r17.xyz = ((r14.zxyz)*(r16.yzxy)).xyz;
    // 207: mad r17.xyz, r14.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r14.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 208: dp3 r14.y, r14.xyzx, r11.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 209: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 210: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 211: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 212: dp2 r15.z, r18.xyxx, cb0[38].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[38].xyxx).xy).xxxx).z;
    // 213: mul r7.xz, cb0[38].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[38].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 214: dp2 r15.x, r18.xyxx, r7.xzxx
    r15.x = (dot((r18.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 215: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 216: dp4 r19.x, cb0[39].xyzw, r15.xyzw
    r19.x = (dot((source[39].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 217: dp4 r19.y, cb0[40].xyzw, r15.xyzw
    r19.y = (dot((source[40].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 218: dp4 r19.z, cb0[41].xyzw, r15.xyzw
    r19.z = (dot((source[41].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 219: mul r20.xyzw, r15.yzzx, r15.xyzz
    r20.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 220: dp4 r21.x, cb0[42].xyzw, r20.xyzw
    r21.x = (dot((source[42].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).x;
    // 221: dp4 r21.y, cb0[43].xyzw, r20.xyzw
    r21.y = (dot((source[43].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).y;
    // 222: dp4 r21.z, cb0[44].xyzw, r20.xyzw
    r21.z = (dot((source[44].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).z;
    // 223: add r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)+(r21.xyzx)).xyz;
    // 224: mul r8.w, r15.y, r15.y
    r8.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 225: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 226: mad r8.w, r15.x, r15.x, -r8.w
    r8.w = ((r15.xxxx)*(r15.xxxx)+(-(r8.wwww))).w;
    // 227: mad r15.xyz, cb0[45].xyzx, r8.wwww, r19.xyzx
    r15.xyz = ((source[45].xyzx)*(r8.wwww)+(r19.xyzx)).xyz;
    // 228: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 229: mul r15.xyz, r15.xyzx, cb0[37].xyzx
    r15.xyz = ((r15.xyzx)*(source[37].xyzx)).xyz;
    // 230: mul r15.xyz, r15.xyzx, cb0[38].zzzz
    r15.xyz = ((r15.xyzx)*(source[38].zzzz)).xyz;
    // 231: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[37].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[37].wwww)).xyz;
    // 232: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: add r15.xyz, -r8.wwww, r15.xyzx
    r15.xyz = ((-(r8.wwww))+(r15.xyzx)).xyz;
    // 234: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r8.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r8.wwww)).xyz;
    // 235: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: div r8.w, r8.w, r6.w
    r8.w = ((r8.wwww)/(r6.wwww)).w;
    // 237: mad r8.w, r5.w, l(5.000000), r8.w
    r8.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r8.wwww)).w;
    // 238: add_sat r8.w, r7.w, r8.w
    r8.w = (saturate((r7.wwww)+(r8.wwww))).w;
    // 239: mad r9.w, r8.w, l(-2.000000), l(3.000000)
    r9.w = ((r8.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 240: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 241: mul r8.w, r8.w, r9.w
    r8.w = ((r8.wwww)*(r9.wwww)).w;
    // 242: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 243: mul r8.w, r8.w, l(1.500000)
    r8.w = ((r8.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 244: exp r8.w, r8.w
    r8.w = (exp2(r8.wwww)).w;
    // 245: mul r15.xyz, r8.wwww, r15.xyzx
    r15.xyz = ((r8.wwww)*(r15.xyzx)).xyz;
    // 246: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 247: mul r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r15.xyzx)).xyz;
    // 248: mul r12.xyz, r4.xywx, r12.xyzx
    r12.xyz = ((r4.xywx)*(r12.xyzx)).xyz;
    // 249: mul r8.w, r7.y, l(5.000000)
    r8.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 250: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 251: mul r3.w, r3.w, r7.y
    r3.w = ((r3.wwww)*(r7.yyyy)).w;
    // 252: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 253: add r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)+(r3.wwww)).w;
    // 254: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 255: add_sat r0.w, r3.w, l(-1.000000)
    r0.w = (saturate((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 256: dp3 r15.x, r16.xyzx, r11.xyzx
    r15.x = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 257: dp3 r15.y, r17.xyzx, r11.xyzx
    r15.y = (dot((r17.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 258: dp2 r14.x, r15.xyxx, r7.xzxx
    r14.x = (dot((r15.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 259: dp2 r14.z, r15.xyxx, cb0[38].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[38].xyxx).xy).xxxx).z;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t9.xyzw, s8, r8.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r8.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 261: mul r7.xyz, r14.xyzx, r14.wwww
    r7.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 262: mul r7.xyz, r7.xyzx, cb0[37].xyzx
    r7.xyz = ((r7.xyzx)*(source[37].xyzx)).xyz;
    // 263: mul r7.xyz, r7.xyzx, cb0[38].zzzz
    r7.xyz = ((r7.xyzx)*(source[38].zzzz)).xyz;
    // 264: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[37].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[37].wwww)).xyz;
    // 265: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r7.xyz, -r3.wwww, r7.xyzx
    r7.xyz = ((-(r3.wwww))+(r7.xyzx)).xyz;
    // 267: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 268: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)/(r6.wwww)).w;
    // 270: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 271: add_sat r3.w, r7.w, r3.w
    r3.w = (saturate((r7.wwww)+(r3.wwww))).w;
    // 272: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 274: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 275: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 276: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 278: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 279: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 280: mad r3.w, r0.w, r5.x, r5.y
    r3.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 281: mad r3.w, r3.w, r0.w, r5.z
    r3.w = ((r3.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 282: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 283: max r0.w, r0.w, r3.w
    r0.w = (max(r0.wwww,r3.wwww)).w;
    // 284: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 285: dp3 r3.w, v6.xyzx, v6.xyzx
    r3.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 287: mul r12.xyz, r3.wwww, v6.xyzx
    r12.xyz = ((r3.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r3.w, r12.xyzx, r8.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 289: dp3 r5.w, -r12.xyzx, r8.xyzx
    r5.w = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).w;
    // 290: dp3 r6.w, r12.xyzx, r11.xyzx
    r6.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 291: mad r8.xy, r6.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r6.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 292: mad r8.zw, r5.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r5.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 293: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 294: mad r11.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 296: mul r11.yzw, r11.yyyy, cb0[48].xxyz
    r11.yzw = ((r11.yyyy)*(source[48].xxyz)).yzw;
    // 297: mad r11.xyz, r11.xxxx, cb0[47].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[47].xyzx)+(r11.yzwy)).xyz;
    // 298: mul r11.xyz, r11.xyzx, cb0[49].wwww
    r11.xyz = ((r11.xyzx)*(source[49].wwww)).xyz;
    // 299: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 300: mul r4.xyw, r4.xyxw, r11.xyxz
    r4.xyw = ((r4.xyxw)*(r11.xyxz)).xyw;
    // 301: mul r4.xyw, r4.xyxw, l(0.600000, 0.600000, 0.000000, 0.600000)
    r4.xyw = ((r4.xyxw)*(float4(0.600000,0.600000,0.000000,0.600000))).xyw;
    // 302: mul r4.xyw, r13.xyxz, r4.xyxw
    r4.xyw = ((r13.xyxz)*(r4.xyxw)).xyw;
    // 303: mad r4.xyw, -r4.xyxw, r7.wwww, r4.xyxw
    r4.xyw = ((-(r4.xyxw))*(r7.wwww)+(r4.xyxw)).xyw;
    // 304: mad r4.xyw, r5.xyxz, l(0.400000, 0.400000, 0.000000, 0.400000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.400000,0.400000,0.000000,0.400000))+(r4.xyxw)).xyw;
    // 305: mul r5.xyz, r8.yyyy, cb0[48].xyzx
    r5.xyz = ((r8.yyyy)*(source[48].xyzx)).xyz;
    // 306: mad r5.xyz, cb0[47].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[47].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, cb0[49].wwww
    r5.xyz = ((r5.xyzx)*(source[49].wwww)).xyz;
    // 308: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 309: mul r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 310: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 311: mad r4.xyw, r5.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r4.xyxw)).xyw;
    // 312: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 313: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 314: mul r0.w, r17.z, cb0[24].w
    r0.w = ((r17.zzzz)*(source[24].wwww)).w;
    // 315: mul r3.w, r17.z, cb0[25].x
    r3.w = ((r17.zzzz)*(source[25].xxxx)).w;
    // 316: mad r5.x, r16.z, cb0[24].w, -r3.w
    r5.x = ((r16.zzzz)*(source[24].wwww)+(-(r3.wwww))).x;
    // 317: mad r5.y, r16.z, cb0[25].x, r0.w
    r5.y = ((r16.zzzz)*(source[25].xxxx)+(r0.wwww)).y;
    // 318: max r0.w, |r5.x|, |r5.y|
    r0.w = (max(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 319: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 320: min r3.w, |r5.x|, |r5.y|
    r3.w = (min(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 321: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 322: mul r3.w, r0.w, r0.w
    r3.w = ((r0.wwww)*(r0.wwww)).w;
    // 323: mad r5.z, r3.w, l(0.020835), l(-0.085133)
    r5.z = ((r3.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 324: mad r5.z, r3.w, r5.z, l(0.180141)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 325: mad r5.z, r3.w, r5.z, l(-0.330299)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 326: mad r3.w, r3.w, r5.z, l(0.999866)
    r3.w = ((r3.wwww)*(r5.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 327: mul r5.z, r0.w, r3.w
    r5.z = ((r0.wwww)*(r3.wwww)).z;
    // 328: mad r5.z, r5.z, l(-2.000000), l(1.570796)
    r5.z = ((r5.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 329: lt r5.w, |r5.x|, |r5.y|
    r5.w = (asfloat((uint4)((abs(r5.xxxx))<(abs(r5.yyyy))) * 0xffffffffu)).w;
    // 330: and r5.z, r5.w, r5.z
    r5.z = (asfloat(asuint(r5.wwww) & asuint(r5.zzzz))).z;
    // 331: mad r0.w, r0.w, r3.w, r5.z
    r0.w = ((r0.wwww)*(r3.wwww)+(r5.zzzz)).w;
    // 332: lt r3.w, r5.x, -r5.x
    r3.w = (asfloat((uint4)((r5.xxxx)<(-(r5.xxxx))) * 0xffffffffu)).w;
    // 333: and r3.w, r3.w, l(0xc0490fdb)
    r3.w = (asfloat(asuint(r3.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 334: add r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)+(r3.wwww)).w;
    // 335: min r3.w, r5.x, r5.y
    r3.w = (min(r5.xxxx,r5.yyyy)).w;
    // 336: lt r3.w, r3.w, -r3.w
    r3.w = (asfloat((uint4)((r3.wwww)<(-(r3.wwww))) * 0xffffffffu)).w;
    // 337: max r5.z, r5.x, r5.y
    r5.z = (max(r5.xxxx,r5.yyyy)).z;
    // 338: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 339: add r5.x, r5.y, r5.x
    r5.x = ((r5.yyyy)+(r5.xxxx)).x;
    // 340: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 341: ge r5.y, r5.z, -r5.z
    r5.y = (asfloat((uint4)((r5.zzzz)>=(-(r5.zzzz))) * 0xffffffffu)).y;
    // 342: and r3.w, r3.w, r5.y
    r3.w = (asfloat(asuint(r3.wwww) & asuint(r5.yyyy))).w;
    // 343: movc r0.w, r3.w, -r0.w, r0.w
    r0.w = ((asuint(r3.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 344: add r3.w, r0.w, l(6.283185)
    r3.w = ((r0.wwww)+(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 345: mul r3.w, r3.w, l(0.159155)
    r3.w = ((r3.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 346: mul r5.y, r0.w, l(0.159155)
    r5.y = ((r0.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).y;
    // 347: ge r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 348: movc r0.w, r0.w, r5.y, r3.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (r5.yyyy) : (r3.wwww)).w;
    // 349: add r3.w, -r0.w, -cb0[25].w
    r3.w = ((-(r0.wwww))+(-(source[25].wwww))).w;
    // 350: add r0.w, r0.w, -cb0[25].w
    r0.w = ((r0.wwww)+(-(source[25].wwww))).w;
    // 351: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 352: add r5.y, -cb0[25].w, l(1.000000)
    r5.y = ((-(source[25].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 353: div r5.y, l(1.000000, 1.000000, 1.000000, 1.000000), r5.y
    r5.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r5.yyyy)).y;
    // 354: mul_sat r3.w, r3.w, r5.y
    r3.w = (saturate((r3.wwww)*(r5.yyyy))).w;
    // 355: mul_sat r0.w, r0.w, r5.y
    r0.w = (saturate((r0.wwww)*(r5.yyyy))).w;
    // 356: mad r5.y, r3.w, l(-2.000000), l(3.000000)
    r5.y = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 357: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 358: mul r3.w, r3.w, r5.y
    r3.w = ((r3.wwww)*(r5.yyyy)).w;
    // 359: mad r5.y, r0.w, l(-2.000000), l(3.000000)
    r5.y = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 360: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 361: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 362: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 363: log r3.w, r5.x
    r3.w = (log2(r5.xxxx)).w;
    // 364: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 365: mul r3.w, r3.w, l(5.082000)
    r3.w = ((r3.wwww)*(float4(5.082000,5.082000,5.082000,5.082000))).w;
    // 366: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 367: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 368: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 369: dp3 r3.w, r6.xyzx, r9.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 370: mul_sat r5.x, r3.w, cb0[26].x
    r5.x = (saturate((r3.wwww)*(source[26].xxxx))).x;
    // 371: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 372: mul_sat r5.y, r9.z, cb0[26].x
    r5.y = (saturate((r9.zzzz)*(source[26].xxxx))).y;
    // 373: add r5.z, -|r9.z|, l(1.000000)
    r5.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 374: mul r3.w, r3.w, r5.z
    r3.w = ((r3.wwww)*(r5.zzzz)).w;
    // 375: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 376: add_sat r5.y, r5.y, -cb0[26].y
    r5.y = (saturate((r5.yyyy)+(-(source[26].yyyy)))).y;
    // 377: log r5.z, r5.y
    r5.z = (log2(r5.yyyy)).z;
    // 378: lt r5.y, r5.y, l(0.000001)
    r5.y = (asfloat((uint4)((r5.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 379: mul r5.z, r5.z, cb0[26].z
    r5.z = ((r5.zzzz)*(source[26].zzzz)).z;
    // 380: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 381: mul r5.x, r5.z, r5.x
    r5.x = ((r5.zzzz)*(r5.xxxx)).x;
    // 382: movc r5.x, r5.y, l(0), r5.x
    r5.x = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).x;
    // 383: mul r5.y, r5.x, cb0[26].w
    r5.y = ((r5.xxxx)*(source[26].wwww)).y;
    // 384: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 385: mul r0.w, r0.w, cb0[26].w
    r0.w = ((r0.wwww)*(source[26].wwww)).w;
    // 386: log r5.y, |r0.w|
    r5.y = (log2(abs(r0.wwww))).y;
    // 387: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 388: mul r5.y, r5.y, cb0[27].x
    r5.y = ((r5.yyyy)*(source[27].xxxx)).y;
    // 389: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 390: mul r6.xyz, cb0[9].xyzx, cb0[9].wwww
    r6.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 391: mul r5.yzw, r5.yyyy, r6.xxyz
    r5.yzw = ((r5.yyyy)*(r6.xxyz)).yzw;
    // 392: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 393: dp2 r6.x, cb0[10].xyxx, r6.xyxx
    r6.x = (dot((source[10].xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 394: add r6.x, r6.x, cb0[28].z
    r6.x = ((r6.xxxx)+(source[28].zzzz)).x;
    // 395: add_sat r6.x, r6.x, l(-0.500000)
    r6.x = (saturate((r6.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).x;
    // 396: mul r5.yzw, r5.yyzw, r6.xxxx
    r5.yzw = ((r5.yyzw)*(r6.xxxx)).yzw;
    // 397: movc r5.yzw, r0.wwww, l(0,0,0,0), r5.yyzw
    r5.yzw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.yyzw)).yzw;
    // 398: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 399: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 400: add r6.x, -r0.w, l(1000.000000)
    r6.x = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 401: mad r0.w, cb0[29].w, r6.x, r0.w
    r0.w = ((source[29].wwww)*(r6.xxxx)+(r0.wwww)).w;
    // 402: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 403: mad r0.w, cb0[29].y, cb0[29].z, r0.w
    r0.w = ((source[29].yyyy)*(source[29].zzzz)+(r0.wwww)).w;
    // 404: mul r6.x, r0.w, l(3.524534)
    r6.x = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 405: sincos null, r6.x, r6.x
    r6.x = (cos(r6.xxxx)).x;
    // 406: add r0.w, r0.w, r6.x
    r0.w = ((r0.wwww)+(r6.xxxx)).w;
    // 407: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 408: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 409: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 410: mad r0.w, r0.w, l(0.500000), cb0[29].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[29].xxxx)).w;
    // 411: mul r6.xyz, cb0[11].xyzx, cb0[28].wwww
    r6.xyz = ((source[11].xyzx)*(source[28].wwww)).xyz;
    // 412: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 413: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 414: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 415: mad r5.yzw, r1.wwww, r5.yyzw, r6.xxyz
    r5.yzw = ((r1.wwww)*(r5.yyzw)+(r6.xxyz)).yzw;
    // 416: mul r6.xy, v4.xyxx, cb0[12].zzzz
    r6.xy = ((v4.xyxx)*(source[12].zzzz)).xy;
    // 417: mul r6.zw, cb0[12].xxxy, cb0[29].zzzz
    r6.zw = ((source[12].xxxy)*(source[29].zzzz)).zw;
    // 418: mad r6.xy, r6.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r6.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 419: mad r6.zw, cb0[12].zzzz, v4.xxxy, r6.zzzw
    r6.zw = ((source[12].zzzz)*(v4.xxxy)+(r6.zzzw)).zw;
    // 420: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r6.xyxx, t7.yzwx, s5, l(0.000000)
    r0.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 421: mad r6.xy, r0.wwww, cb0[30].xxxx, r6.zwzz
    r6.xy = ((r0.wwww)*(source[30].xxxx)+(r6.zwzz)).xy;
    // 422: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t7.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 423: mul r6.xyz, r6.xyzx, r7.wwww
    r6.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 424: mul r7.xyz, cb0[13].xyzx, cb0[30].yyyy
    r7.xyz = ((source[13].xyzx)*(source[30].yyyy)).xyz;
    // 425: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 426: mad r7.xyz, r5.xxxx, r6.xyzx, -r6.xyzx
    r7.xyz = ((r5.xxxx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 427: mad r6.xyz, cb0[13].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[13].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 428: add r5.yzw, r5.yyzw, r6.xxyz
    r5.yzw = ((r5.yyzw)+(r6.xxyz)).yzw;
    // 429: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 430: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 431: mad r5.yzw, cb0[30].zzzz, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].zzzz)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 432: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 433: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 434: mad r5.yzw, cb0[30].wwww, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].wwww)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 435: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 436: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 437: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 438: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 439: add r6.xyz, -r0.xyzx, r0.wwww
    r6.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 440: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 441: mul r6.xyz, cb0[18].xyzx, cb0[31].yyyy
    r6.xyz = ((source[18].xyzx)*(source[31].yyyy)).xyz;
    // 442: mul r6.xyz, r6.xyzx, cb0[32].wwww
    r6.xyz = ((r6.xyzx)*(source[32].wwww)).xyz;
    // 443: mul r6.xyz, r5.xxxx, r6.xyzx
    r6.xyz = ((r5.xxxx)*(r6.xyzx)).xyz;
    // 444: mad r7.xyz, r5.xxxx, cb0[17].xyzx, -cb0[17].xyzx
    r7.xyz = ((r5.xxxx)*(source[17].xyzx)+(-(source[17].xyzx))).xyz;
    // 445: add r0.w, r5.x, l(-1.000000)
    r0.w = ((r5.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 446: mad r0.w, cb0[16].w, r0.w, l(1.000000)
    r0.w = ((source[16].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 447: mad r7.xyz, cb0[17].wwww, r7.xyzx, cb0[17].xyzx
    r7.xyz = ((source[17].wwww)*(r7.xyzx)+(source[17].xyzx)).xyz;
    // 448: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 449: mad r0.xyz, r0.wwww, cb0[16].xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 450: mad r0.xyz, r5.yzwy, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.yzwy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 451: log r0.w, |r3.w|
    r0.w = (log2(abs(r3.wwww))).w;
    // 452: lt r1.w, |r3.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 453: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 454: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 455: mul r3.xyz, r0.wwww, cb0[19].xyzx
    r3.xyz = ((r0.wwww)*(source[19].xyzx)).xyz;
    // 456: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 457: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 458: mad r0.xyz, cb0[23].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 459: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 460: mul r1.xyz, r8.wwww, cb0[48].xyzx
    r1.xyz = ((r8.wwww)*(source[48].xyzx)).xyz;
    // 461: mad r1.xyz, r8.zzzz, cb0[47].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[47].xyzx)+(r1.xyzx)).xyz;
    // 462: mul r1.xyz, r1.xyzx, cb0[49].wwww
    r1.xyz = ((r1.xyzx)*(source[49].wwww)).xyz;
    // 463: mul_sat r3.xyz, cb0[22].xyzx, cb0[22].wwww
    r3.xyz = (saturate((source[22].xyzx)*(source[22].wwww))).xyz;
    // 464: mul r5.xyz, r2.wwww, r3.xyzx
    r5.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 465: mul r3.xyz, r3.xyzx, cb0[36].wwww
    r3.xyz = ((r3.xyzx)*(source[36].wwww)).xyz;
    // 466: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 467: mul r3.xyz, r4.zzzz, r5.xyzx
    r3.xyz = ((r4.zzzz)*(r5.xyzx)).xyz;
    // 468: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 469: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 470: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 471: add r0.xyz, r4.xywx, r0.xyzx
    r0.xyz = ((r4.xywx)+(r0.xyzx)).xyz;
    // 472: dp3 o4.y, r4.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 473: mad o0.xyz, r2.xyzx, cb0[49].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[49].xyzx)+(r0.xyzx)).xyz;
    // 474: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 475: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 476: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 477: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 478: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 479: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 480: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 481: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 482: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 483: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 484: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 485: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 486: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 487: ftou r0.x, cb0[46].z
    r0.x = (asfloat((uint4)(source[46].zzzz))).x;
    // 488: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 489: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 490: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 491: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 492: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drg-00-neck-mi-dead.v1 / source program fcf2ae320f020d48b6a207cedc870d36
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase103(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[21]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[29].z=(g_SourceCharacterTime.xxxx).x;
    source[31].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[32].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[32].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[32].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[32].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[37] = g_SourceCharacterEnvironmentColor;
        source[38] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[34].yyyy
    r0.xy = ((v4.xyxx)*(source[34].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[34].z
    r0.x = ((r0.xxxx)+(-(source[34].zzzz))).x;
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
    // 11: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 13: mad r0.xyz, cb0[30].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[30].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[30].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[30].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 17: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 18: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 19: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 20: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 21: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 22: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 24: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 25: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 26: mul r0.w, r5.y, cb0[23].y
    r0.w = ((r5.yyyy)*(source[23].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: movc r0.w, r4.y, l(0), r0.w
    r0.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 31: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 32: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 33: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 34: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 35: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 36: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 37: mad r3.xyz, r0.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mad r2.xyz, r6.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r6.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 41: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 42: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 43: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 47: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 48: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 49: mad r2.xyz, r6.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 50: mul r3.xyz, cb0[7].xyzx, cb0[7].wwww
    r3.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 51: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 52: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 53: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 54: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 55: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 56: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 57: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 58: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 59: mad r2.xyz, r6.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 60: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 62: mad r3.xyz, cb0[30].zzzz, r3.xyzx, r2.xyzx
    r3.xyz = ((source[30].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[30].wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((source[30].wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 67: mad r3.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 71: mul r8.xyz, r0.xyzx, r2.xyzx
    r8.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 72: dp3 r0.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r9.xyz, -r2.xyzx, r0.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 74: mad r0.xyz, r2.xyzx, r0.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 75: mad r2.xyz, cb0[30].zzzz, r9.xyzx, r8.xyzx
    r2.xyz = ((source[30].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r8.xyz, -r2.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[30].wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((source[30].wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[8].w, l(1.000000)
    r0.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[29].z
    r0.w = ((r0.wwww)*(source[29].zzzz)).w;
    // 82: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 83: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 84: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r1.w, cb0[8].z, l(1.500000)
    r1.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 87: mad r0.w, r0.w, l(0.500000), cb0[8].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 88: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 89: mul r7.x, r1.w, l(0.125000)
    r7.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 90: mul r8.y, cb0[8].y, cb0[21].y
    r8.y = ((source[8].yyyy)*(source[21].yyyy)).y;
    // 91: mov r7.y, v4.y
    r7.y = (v4.yyyy).y;
    // 92: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 93: add r4.yw, r7.xxxy, r8.xxxy
    r4.yw = ((r7.xxxy)+(r8.xxxy)).yw;
    // 94: frc r1.w, cb0[8].x
    r1.w = (frac(source[8].xxxx)).w;
    // 95: add r2.w, -r1.w, cb0[8].x
    r2.w = ((-(r1.wwww))+(source[8].xxxx)).w;
    // 96: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 97: add r4.yw, r4.yyyw, r8.zzzw
    r4.yw = ((r4.yyyw)+(r8.zzzw)).yw;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r4.ywyy, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 99: mul r8.xyz, r0.wwww, r8.xyzx
    r8.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 100: mul r0.w, r1.w, r8.w
    r0.w = ((r1.wwww)*(r8.wwww)).w;
    // 101: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 103: mad r2.xyz, r0.wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 104: mul r0.w, r5.x, cb0[33].x
    r0.w = ((r5.xxxx)*(source[33].xxxx)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[33].y
    r0.w = (saturate((r0.wwww)+(source[33].yyyy))).w;
    // 108: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r4.xyw, r2.wwww, cb0[20].xyxz
    r4.xyw = ((r2.wwww)*(source[20].xyxz)).xyw;
    // 110: mul r5.xyw, r2.xyxz, r4.xyxw
    r5.xyw = ((r2.xyxz)*(r4.xyxw)).xyw;
    // 111: mad r2.xyz, -r4.xywx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r4.xywx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 112: mad r2.xyz, r0.wwww, r2.xyzx, r5.xywx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xywx)).xyz;
    // 113: add r4.xyw, -cb0[3].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r4.xyw = ((-(source[3].xyxz))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 114: mul r2.xyz, r2.xyzx, r4.xywx
    r2.xyz = ((r2.xyzx)*(r4.xywx)).xyz;
    // 115: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 116: mad r4.xyw, r2.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r4.xyw = ((r2.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 117: mad r5.xyw, r2.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r5.xyw = ((r2.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 118: mad r4.xyw, r0.wwww, r4.xyxw, r5.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)+(r5.xyxw)).xyw;
    // 119: mad r5.xyw, r2.xyxz, l(2.755200, 2.755200, 0.000000, 2.755200), l(0.690300, 0.690300, 0.000000, 0.690300)
    r5.xyw = ((r2.xyxz)*(float4(2.755200,2.755200,0.000000,2.755200))+(float4(0.690300,0.690300,0.000000,0.690300))).xyw;
    // 120: mad r4.xyw, r4.xyxw, r0.wwww, r5.xyxw
    r4.xyw = ((r4.xyxw)*(r0.wwww)+(r5.xyxw)).xyw;
    // 121: mul r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)).xyw;
    // 122: max r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = (max(r0.wwww,r4.xyxw)).xyw;
    // 123: mov_sat r2.w, cb0[34].x
    r2.w = (saturate(source[34].xxxx)).w;
    // 124: mad r5.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r5.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 125: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 126: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 127: mad r5.xyw, r7.wwww, r5.xyxw, r3.wwww
    r5.xyw = ((r7.wwww)*(r5.xyxw)+(r3.wwww)).xyw;
    // 128: add r2.w, -cb0[35].z, cb0[35].y
    r2.w = ((-(source[35].zzzz))+(source[35].yyyy)).w;
    // 129: mad r2.w, r6.x, r2.w, cb0[35].z
    r2.w = ((r6.xxxx)*(r2.wwww)+(source[35].zzzz)).w;
    // 130: add r3.w, -r2.w, cb0[36].x
    r3.w = ((-(r2.wwww))+(source[36].xxxx)).w;
    // 131: mad r2.w, r6.y, r3.w, r2.w
    r2.w = ((r6.yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 132: add r3.w, -r2.w, cb0[36].z
    r3.w = ((-(r2.wwww))+(source[36].zzzz)).w;
    // 133: mad r2.w, r6.z, r3.w, r2.w
    r2.w = ((r6.zzzz)*(r3.wwww)+(r2.wwww)).w;
    // 134: mul r2.w, r5.z, r2.w
    r2.w = ((r5.zzzz)*(r2.wwww)).w;
    // 135: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 136: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: movc r2.w, r4.z, l(0), r2.w
    r2.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 138: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 139: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 141: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 142: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 143: mul r6.xy, r6.xyxx, cb0[23].xxxx
    r6.xy = ((r6.xyxx)*(source[23].xxxx)).xy;
    // 144: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 147: add r6.z, r2.w, l(0.000010)
    r6.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 148: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 149: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 150: div r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)/(r2.wwww)).xyz;
    // 151: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 152: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 153: mul r8.xyz, r2.wwww, r6.xyzx
    r8.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 154: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 156: mul r9.xyz, r2.wwww, v5.xyzx
    r9.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r2.w, r8.xyzx, r9.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 158: deriv_rtx_coarse r7.x, r2.w
    r7.x = (ddx_coarse(r2.wwww)).x;
    // 159: deriv_rty_coarse r7.y, r2.w
    r7.y = (ddy_coarse(r2.wwww)).y;
    // 160: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 161: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 162: mad r3.w, r3.w, l(0.300000), r7.z
    r3.w = ((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).w;
    // 163: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 164: min r7.y, r3.w, l(1.000000)
    r7.y = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r10.xyz, r5.xywx, r3.wwww
    r10.xyz = (max(r5.xywx,r3.wwww)).xyz;
    // 167: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 168: mul_sat r3.w, r5.y, l(50.000000)
    r3.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 170: mul r11.xyz, r2.wwww, r8.xyzx
    r11.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 171: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 172: add r3.w, r11.z, l(1.000000)
    r3.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r4.z, r2.w, l(1.000000)
    r4.z = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mov_sat r2.w, r2.w
    r2.w = (saturate(r2.wwww)).w;
    // 176: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 177: mul r2.w, r2.w, cb0[2].y
    r2.w = ((r2.wwww)*(source[2].yyyy)).w;
    // 178: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 179: mad_sat r2.w, r2.w, cb0[2].w, cb0[2].z
    r2.w = (saturate((r2.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 180: mul r2.w, r2.w, cb0[36].w
    r2.w = ((r2.wwww)*(source[36].wwww)).w;
    // 181: add_sat r7.x, -r3.w, r4.z
    r7.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t8.xyzw, s9
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 183: add r3.w, r0.w, r7.x
    r3.w = ((r0.wwww)+(r7.xxxx)).w;
    // 184: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 185: mul r13.xyz, r5.xywx, r12.yyyy
    r13.xyz = ((r5.xywx)*(r12.yyyy)).xyz;
    // 186: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 187: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.yyyy)).z;
    // 188: add r4.z, r4.z, l(-1.000000)
    r4.z = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 189: mad r12.xyz, r5.xywx, r4.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r4.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 190: dp3 r4.z, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.z = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: mad r5.xyz, r4.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r4.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 194: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 195: add r4.z, -r7.w, l(1.000000)
    r4.z = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 196: mul r12.xyz, r4.zzzz, r12.xyzx
    r12.xyz = ((r4.zzzz)*(r12.xyzx)).xyz;
    // 197: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: mad r6.w, r7.y, l(2.000000), l(2.000000)
    r6.w = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 199: dp3 r7.x, v1.xyzx, v1.xyzx
    r7.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 200: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 201: mul r14.xyz, r7.xxxx, v1.xyzx
    r14.xyz = ((r7.xxxx)*(v1.xyzx)).xyz;
    // 202: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 203: dp3 r7.x, v0.xyzx, v0.xyzx
    r7.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 204: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 205: mul r16.xyz, r7.xxxx, v0.xyzx
    r16.xyz = ((r7.xxxx)*(v0.xyzx)).xyz;
    // 206: mul r17.xyz, r14.zxyz, r16.yzxy
    r17.xyz = ((r14.zxyz)*(r16.yzxy)).xyz;
    // 207: mad r17.xyz, r14.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r14.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 208: dp3 r14.y, r14.xyzx, r11.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 209: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 210: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 211: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 212: dp2 r15.z, r18.xyxx, cb0[38].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[38].xyxx).xy).xxxx).z;
    // 213: mul r7.xz, cb0[38].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[38].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 214: dp2 r15.x, r18.xyxx, r7.xzxx
    r15.x = (dot((r18.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 215: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 216: dp4 r19.x, cb0[39].xyzw, r15.xyzw
    r19.x = (dot((source[39].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 217: dp4 r19.y, cb0[40].xyzw, r15.xyzw
    r19.y = (dot((source[40].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 218: dp4 r19.z, cb0[41].xyzw, r15.xyzw
    r19.z = (dot((source[41].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 219: mul r20.xyzw, r15.yzzx, r15.xyzz
    r20.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 220: dp4 r21.x, cb0[42].xyzw, r20.xyzw
    r21.x = (dot((source[42].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).x;
    // 221: dp4 r21.y, cb0[43].xyzw, r20.xyzw
    r21.y = (dot((source[43].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).y;
    // 222: dp4 r21.z, cb0[44].xyzw, r20.xyzw
    r21.z = (dot((source[44].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).z;
    // 223: add r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)+(r21.xyzx)).xyz;
    // 224: mul r8.w, r15.y, r15.y
    r8.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 225: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 226: mad r8.w, r15.x, r15.x, -r8.w
    r8.w = ((r15.xxxx)*(r15.xxxx)+(-(r8.wwww))).w;
    // 227: mad r15.xyz, cb0[45].xyzx, r8.wwww, r19.xyzx
    r15.xyz = ((source[45].xyzx)*(r8.wwww)+(r19.xyzx)).xyz;
    // 228: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 229: mul r15.xyz, r15.xyzx, cb0[37].xyzx
    r15.xyz = ((r15.xyzx)*(source[37].xyzx)).xyz;
    // 230: mul r15.xyz, r15.xyzx, cb0[38].zzzz
    r15.xyz = ((r15.xyzx)*(source[38].zzzz)).xyz;
    // 231: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[37].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[37].wwww)).xyz;
    // 232: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: add r15.xyz, -r8.wwww, r15.xyzx
    r15.xyz = ((-(r8.wwww))+(r15.xyzx)).xyz;
    // 234: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r8.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r8.wwww)).xyz;
    // 235: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: div r8.w, r8.w, r6.w
    r8.w = ((r8.wwww)/(r6.wwww)).w;
    // 237: mad r8.w, r5.w, l(5.000000), r8.w
    r8.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r8.wwww)).w;
    // 238: add_sat r8.w, r7.w, r8.w
    r8.w = (saturate((r7.wwww)+(r8.wwww))).w;
    // 239: mad r9.w, r8.w, l(-2.000000), l(3.000000)
    r9.w = ((r8.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 240: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 241: mul r8.w, r8.w, r9.w
    r8.w = ((r8.wwww)*(r9.wwww)).w;
    // 242: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 243: mul r8.w, r8.w, l(1.500000)
    r8.w = ((r8.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 244: exp r8.w, r8.w
    r8.w = (exp2(r8.wwww)).w;
    // 245: mul r15.xyz, r8.wwww, r15.xyzx
    r15.xyz = ((r8.wwww)*(r15.xyzx)).xyz;
    // 246: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 247: mul r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r15.xyzx)).xyz;
    // 248: mul r12.xyz, r4.xywx, r12.xyzx
    r12.xyz = ((r4.xywx)*(r12.xyzx)).xyz;
    // 249: mul r8.w, r7.y, l(5.000000)
    r8.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 250: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 251: mul r3.w, r3.w, r7.y
    r3.w = ((r3.wwww)*(r7.yyyy)).w;
    // 252: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 253: add r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)+(r3.wwww)).w;
    // 254: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 255: add_sat r0.w, r3.w, l(-1.000000)
    r0.w = (saturate((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 256: dp3 r15.x, r16.xyzx, r11.xyzx
    r15.x = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 257: dp3 r15.y, r17.xyzx, r11.xyzx
    r15.y = (dot((r17.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 258: dp2 r14.x, r15.xyxx, r7.xzxx
    r14.x = (dot((r15.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 259: dp2 r14.z, r15.xyxx, cb0[38].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[38].xyxx).xy).xxxx).z;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t9.xyzw, s8, r8.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r8.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 261: mul r7.xyz, r14.xyzx, r14.wwww
    r7.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 262: mul r7.xyz, r7.xyzx, cb0[37].xyzx
    r7.xyz = ((r7.xyzx)*(source[37].xyzx)).xyz;
    // 263: mul r7.xyz, r7.xyzx, cb0[38].zzzz
    r7.xyz = ((r7.xyzx)*(source[38].zzzz)).xyz;
    // 264: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[37].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[37].wwww)).xyz;
    // 265: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r7.xyz, -r3.wwww, r7.xyzx
    r7.xyz = ((-(r3.wwww))+(r7.xyzx)).xyz;
    // 267: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 268: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)/(r6.wwww)).w;
    // 270: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 271: add_sat r3.w, r7.w, r3.w
    r3.w = (saturate((r7.wwww)+(r3.wwww))).w;
    // 272: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 274: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 275: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 276: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 278: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 279: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 280: mad r3.w, r0.w, r5.x, r5.y
    r3.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 281: mad r3.w, r3.w, r0.w, r5.z
    r3.w = ((r3.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 282: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 283: max r0.w, r0.w, r3.w
    r0.w = (max(r0.wwww,r3.wwww)).w;
    // 284: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 285: dp3 r3.w, v6.xyzx, v6.xyzx
    r3.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 287: mul r12.xyz, r3.wwww, v6.xyzx
    r12.xyz = ((r3.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r3.w, r12.xyzx, r8.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 289: dp3 r5.w, -r12.xyzx, r8.xyzx
    r5.w = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).w;
    // 290: dp3 r6.w, r12.xyzx, r11.xyzx
    r6.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 291: mad r8.xy, r6.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r6.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 292: mad r8.zw, r5.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r5.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 293: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 294: mad r11.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 296: mul r11.yzw, r11.yyyy, cb0[48].xxyz
    r11.yzw = ((r11.yyyy)*(source[48].xxyz)).yzw;
    // 297: mad r11.xyz, r11.xxxx, cb0[47].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[47].xyzx)+(r11.yzwy)).xyz;
    // 298: mul r11.xyz, r11.xyzx, cb0[49].wwww
    r11.xyz = ((r11.xyzx)*(source[49].wwww)).xyz;
    // 299: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 300: mul r4.xyw, r4.xyxw, r11.xyxz
    r4.xyw = ((r4.xyxw)*(r11.xyxz)).xyw;
    // 301: mul r4.xyw, r4.xyxw, l(0.600000, 0.600000, 0.000000, 0.600000)
    r4.xyw = ((r4.xyxw)*(float4(0.600000,0.600000,0.000000,0.600000))).xyw;
    // 302: mul r4.xyw, r13.xyxz, r4.xyxw
    r4.xyw = ((r13.xyxz)*(r4.xyxw)).xyw;
    // 303: mad r4.xyw, -r4.xyxw, r7.wwww, r4.xyxw
    r4.xyw = ((-(r4.xyxw))*(r7.wwww)+(r4.xyxw)).xyw;
    // 304: mad r4.xyw, r5.xyxz, l(0.400000, 0.400000, 0.000000, 0.400000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.400000,0.400000,0.000000,0.400000))+(r4.xyxw)).xyw;
    // 305: mul r5.xyz, r8.yyyy, cb0[48].xyzx
    r5.xyz = ((r8.yyyy)*(source[48].xyzx)).xyz;
    // 306: mad r5.xyz, cb0[47].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[47].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, cb0[49].wwww
    r5.xyz = ((r5.xyzx)*(source[49].wwww)).xyz;
    // 308: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 309: mul r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 310: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 311: mad r4.xyw, r5.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r4.xyxw)).xyw;
    // 312: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 313: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 314: mul r0.w, r17.z, cb0[24].w
    r0.w = ((r17.zzzz)*(source[24].wwww)).w;
    // 315: mul r3.w, r17.z, cb0[25].x
    r3.w = ((r17.zzzz)*(source[25].xxxx)).w;
    // 316: mad r5.x, r16.z, cb0[24].w, -r3.w
    r5.x = ((r16.zzzz)*(source[24].wwww)+(-(r3.wwww))).x;
    // 317: mad r5.y, r16.z, cb0[25].x, r0.w
    r5.y = ((r16.zzzz)*(source[25].xxxx)+(r0.wwww)).y;
    // 318: max r0.w, |r5.x|, |r5.y|
    r0.w = (max(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 319: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 320: min r3.w, |r5.x|, |r5.y|
    r3.w = (min(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 321: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 322: mul r3.w, r0.w, r0.w
    r3.w = ((r0.wwww)*(r0.wwww)).w;
    // 323: mad r5.z, r3.w, l(0.020835), l(-0.085133)
    r5.z = ((r3.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 324: mad r5.z, r3.w, r5.z, l(0.180141)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 325: mad r5.z, r3.w, r5.z, l(-0.330299)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 326: mad r3.w, r3.w, r5.z, l(0.999866)
    r3.w = ((r3.wwww)*(r5.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 327: mul r5.z, r0.w, r3.w
    r5.z = ((r0.wwww)*(r3.wwww)).z;
    // 328: mad r5.z, r5.z, l(-2.000000), l(1.570796)
    r5.z = ((r5.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 329: lt r5.w, |r5.x|, |r5.y|
    r5.w = (asfloat((uint4)((abs(r5.xxxx))<(abs(r5.yyyy))) * 0xffffffffu)).w;
    // 330: and r5.z, r5.w, r5.z
    r5.z = (asfloat(asuint(r5.wwww) & asuint(r5.zzzz))).z;
    // 331: mad r0.w, r0.w, r3.w, r5.z
    r0.w = ((r0.wwww)*(r3.wwww)+(r5.zzzz)).w;
    // 332: lt r3.w, r5.x, -r5.x
    r3.w = (asfloat((uint4)((r5.xxxx)<(-(r5.xxxx))) * 0xffffffffu)).w;
    // 333: and r3.w, r3.w, l(0xc0490fdb)
    r3.w = (asfloat(asuint(r3.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 334: add r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)+(r3.wwww)).w;
    // 335: min r3.w, r5.x, r5.y
    r3.w = (min(r5.xxxx,r5.yyyy)).w;
    // 336: lt r3.w, r3.w, -r3.w
    r3.w = (asfloat((uint4)((r3.wwww)<(-(r3.wwww))) * 0xffffffffu)).w;
    // 337: max r5.z, r5.x, r5.y
    r5.z = (max(r5.xxxx,r5.yyyy)).z;
    // 338: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 339: add r5.x, r5.y, r5.x
    r5.x = ((r5.yyyy)+(r5.xxxx)).x;
    // 340: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 341: ge r5.y, r5.z, -r5.z
    r5.y = (asfloat((uint4)((r5.zzzz)>=(-(r5.zzzz))) * 0xffffffffu)).y;
    // 342: and r3.w, r3.w, r5.y
    r3.w = (asfloat(asuint(r3.wwww) & asuint(r5.yyyy))).w;
    // 343: movc r0.w, r3.w, -r0.w, r0.w
    r0.w = ((asuint(r3.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 344: add r3.w, r0.w, l(6.283185)
    r3.w = ((r0.wwww)+(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 345: mul r3.w, r3.w, l(0.159155)
    r3.w = ((r3.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 346: mul r5.y, r0.w, l(0.159155)
    r5.y = ((r0.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).y;
    // 347: ge r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 348: movc r0.w, r0.w, r5.y, r3.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (r5.yyyy) : (r3.wwww)).w;
    // 349: add r3.w, -r0.w, -cb0[25].w
    r3.w = ((-(r0.wwww))+(-(source[25].wwww))).w;
    // 350: add r0.w, r0.w, -cb0[25].w
    r0.w = ((r0.wwww)+(-(source[25].wwww))).w;
    // 351: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 352: add r5.y, -cb0[25].w, l(1.000000)
    r5.y = ((-(source[25].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 353: div r5.y, l(1.000000, 1.000000, 1.000000, 1.000000), r5.y
    r5.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r5.yyyy)).y;
    // 354: mul_sat r3.w, r3.w, r5.y
    r3.w = (saturate((r3.wwww)*(r5.yyyy))).w;
    // 355: mul_sat r0.w, r0.w, r5.y
    r0.w = (saturate((r0.wwww)*(r5.yyyy))).w;
    // 356: mad r5.y, r3.w, l(-2.000000), l(3.000000)
    r5.y = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 357: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 358: mul r3.w, r3.w, r5.y
    r3.w = ((r3.wwww)*(r5.yyyy)).w;
    // 359: mad r5.y, r0.w, l(-2.000000), l(3.000000)
    r5.y = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 360: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 361: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 362: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 363: log r3.w, r5.x
    r3.w = (log2(r5.xxxx)).w;
    // 364: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 365: mul r3.w, r3.w, l(5.082000)
    r3.w = ((r3.wwww)*(float4(5.082000,5.082000,5.082000,5.082000))).w;
    // 366: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 367: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 368: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 369: dp3 r3.w, r6.xyzx, r9.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 370: mul_sat r5.x, r3.w, cb0[26].x
    r5.x = (saturate((r3.wwww)*(source[26].xxxx))).x;
    // 371: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 372: mul_sat r5.y, r9.z, cb0[26].x
    r5.y = (saturate((r9.zzzz)*(source[26].xxxx))).y;
    // 373: add r5.z, -|r9.z|, l(1.000000)
    r5.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 374: mul r3.w, r3.w, r5.z
    r3.w = ((r3.wwww)*(r5.zzzz)).w;
    // 375: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 376: add_sat r5.y, r5.y, -cb0[26].y
    r5.y = (saturate((r5.yyyy)+(-(source[26].yyyy)))).y;
    // 377: log r5.z, r5.y
    r5.z = (log2(r5.yyyy)).z;
    // 378: lt r5.y, r5.y, l(0.000001)
    r5.y = (asfloat((uint4)((r5.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 379: mul r5.z, r5.z, cb0[26].z
    r5.z = ((r5.zzzz)*(source[26].zzzz)).z;
    // 380: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 381: mul r5.x, r5.z, r5.x
    r5.x = ((r5.zzzz)*(r5.xxxx)).x;
    // 382: movc r5.x, r5.y, l(0), r5.x
    r5.x = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).x;
    // 383: mul r5.y, r5.x, cb0[26].w
    r5.y = ((r5.xxxx)*(source[26].wwww)).y;
    // 384: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 385: mul r0.w, r0.w, cb0[26].w
    r0.w = ((r0.wwww)*(source[26].wwww)).w;
    // 386: log r5.y, |r0.w|
    r5.y = (log2(abs(r0.wwww))).y;
    // 387: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 388: mul r5.y, r5.y, cb0[27].x
    r5.y = ((r5.yyyy)*(source[27].xxxx)).y;
    // 389: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 390: mul r6.xyz, cb0[9].xyzx, cb0[9].wwww
    r6.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 391: mul r5.yzw, r5.yyyy, r6.xxyz
    r5.yzw = ((r5.yyyy)*(r6.xxyz)).yzw;
    // 392: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 393: dp2 r6.x, cb0[10].xyxx, r6.xyxx
    r6.x = (dot((source[10].xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 394: add r6.x, r6.x, cb0[28].z
    r6.x = ((r6.xxxx)+(source[28].zzzz)).x;
    // 395: add_sat r6.x, r6.x, l(-0.500000)
    r6.x = (saturate((r6.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).x;
    // 396: mul r5.yzw, r5.yyzw, r6.xxxx
    r5.yzw = ((r5.yyzw)*(r6.xxxx)).yzw;
    // 397: movc r5.yzw, r0.wwww, l(0,0,0,0), r5.yyzw
    r5.yzw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.yyzw)).yzw;
    // 398: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 399: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 400: add r6.x, -r0.w, l(1000.000000)
    r6.x = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 401: mad r0.w, cb0[29].w, r6.x, r0.w
    r0.w = ((source[29].wwww)*(r6.xxxx)+(r0.wwww)).w;
    // 402: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 403: mad r0.w, cb0[29].y, cb0[29].z, r0.w
    r0.w = ((source[29].yyyy)*(source[29].zzzz)+(r0.wwww)).w;
    // 404: mul r6.x, r0.w, l(3.524534)
    r6.x = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 405: sincos null, r6.x, r6.x
    r6.x = (cos(r6.xxxx)).x;
    // 406: add r0.w, r0.w, r6.x
    r0.w = ((r0.wwww)+(r6.xxxx)).w;
    // 407: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 408: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 409: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 410: mad r0.w, r0.w, l(0.500000), cb0[29].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[29].xxxx)).w;
    // 411: mul r6.xyz, cb0[11].xyzx, cb0[28].wwww
    r6.xyz = ((source[11].xyzx)*(source[28].wwww)).xyz;
    // 412: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 413: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 414: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 415: mad r5.yzw, r1.wwww, r5.yyzw, r6.xxyz
    r5.yzw = ((r1.wwww)*(r5.yyzw)+(r6.xxyz)).yzw;
    // 416: mul r6.xy, v4.xyxx, cb0[12].zzzz
    r6.xy = ((v4.xyxx)*(source[12].zzzz)).xy;
    // 417: mul r6.zw, cb0[12].xxxy, cb0[29].zzzz
    r6.zw = ((source[12].xxxy)*(source[29].zzzz)).zw;
    // 418: mad r6.xy, r6.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r6.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 419: mad r6.zw, cb0[12].zzzz, v4.xxxy, r6.zzzw
    r6.zw = ((source[12].zzzz)*(v4.xxxy)+(r6.zzzw)).zw;
    // 420: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r6.xyxx, t7.yzwx, s5, l(0.000000)
    r0.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 421: mad r6.xy, r0.wwww, cb0[30].xxxx, r6.zwzz
    r6.xy = ((r0.wwww)*(source[30].xxxx)+(r6.zwzz)).xy;
    // 422: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t7.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 423: mul r6.xyz, r6.xyzx, r7.wwww
    r6.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 424: mul r7.xyz, cb0[13].xyzx, cb0[30].yyyy
    r7.xyz = ((source[13].xyzx)*(source[30].yyyy)).xyz;
    // 425: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 426: mad r7.xyz, r5.xxxx, r6.xyzx, -r6.xyzx
    r7.xyz = ((r5.xxxx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 427: mad r6.xyz, cb0[13].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[13].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 428: add r5.yzw, r5.yyzw, r6.xxyz
    r5.yzw = ((r5.yyzw)+(r6.xxyz)).yzw;
    // 429: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 430: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 431: mad r5.yzw, cb0[30].zzzz, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].zzzz)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 432: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 433: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 434: mad r5.yzw, cb0[30].wwww, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].wwww)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 435: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 436: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 437: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 438: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 439: add r6.xyz, -r0.xyzx, r0.wwww
    r6.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 440: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 441: mul r6.xyz, cb0[18].xyzx, cb0[31].yyyy
    r6.xyz = ((source[18].xyzx)*(source[31].yyyy)).xyz;
    // 442: mul r6.xyz, r6.xyzx, cb0[32].wwww
    r6.xyz = ((r6.xyzx)*(source[32].wwww)).xyz;
    // 443: mul r6.xyz, r5.xxxx, r6.xyzx
    r6.xyz = ((r5.xxxx)*(r6.xyzx)).xyz;
    // 444: mad r7.xyz, r5.xxxx, cb0[17].xyzx, -cb0[17].xyzx
    r7.xyz = ((r5.xxxx)*(source[17].xyzx)+(-(source[17].xyzx))).xyz;
    // 445: add r0.w, r5.x, l(-1.000000)
    r0.w = ((r5.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 446: mad r0.w, cb0[16].w, r0.w, l(1.000000)
    r0.w = ((source[16].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 447: mad r7.xyz, cb0[17].wwww, r7.xyzx, cb0[17].xyzx
    r7.xyz = ((source[17].wwww)*(r7.xyzx)+(source[17].xyzx)).xyz;
    // 448: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 449: mad r0.xyz, r0.wwww, cb0[16].xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 450: mad r0.xyz, r5.yzwy, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.yzwy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 451: log r0.w, |r3.w|
    r0.w = (log2(abs(r3.wwww))).w;
    // 452: lt r1.w, |r3.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 453: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 454: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 455: mul r3.xyz, r0.wwww, cb0[19].xyzx
    r3.xyz = ((r0.wwww)*(source[19].xyzx)).xyz;
    // 456: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 457: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 458: mad r0.xyz, cb0[23].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 459: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 460: mul r1.xyz, r8.wwww, cb0[48].xyzx
    r1.xyz = ((r8.wwww)*(source[48].xyzx)).xyz;
    // 461: mad r1.xyz, r8.zzzz, cb0[47].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[47].xyzx)+(r1.xyzx)).xyz;
    // 462: mul r1.xyz, r1.xyzx, cb0[49].wwww
    r1.xyz = ((r1.xyzx)*(source[49].wwww)).xyz;
    // 463: mul_sat r3.xyz, cb0[22].xyzx, cb0[22].wwww
    r3.xyz = (saturate((source[22].xyzx)*(source[22].wwww))).xyz;
    // 464: mul r5.xyz, r2.wwww, r3.xyzx
    r5.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 465: mul r3.xyz, r3.xyzx, cb0[36].wwww
    r3.xyz = ((r3.xyzx)*(source[36].wwww)).xyz;
    // 466: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 467: mul r3.xyz, r4.zzzz, r5.xyzx
    r3.xyz = ((r4.zzzz)*(r5.xyzx)).xyz;
    // 468: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 469: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 470: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 471: add r0.xyz, r4.xywx, r0.xyzx
    r0.xyz = ((r4.xywx)+(r0.xyzx)).xyz;
    // 472: dp3 o4.y, r4.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 473: mad o0.xyz, r2.xyzx, cb0[49].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[49].xyzx)+(r0.xyzx)).xyz;
    // 474: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 475: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 476: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 477: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 478: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 479: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 480: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 481: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 482: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 483: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 484: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 485: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 486: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 487: ftou r0.x, cb0[46].z
    r0.x = (asfloat((uint4)(source[46].zzzz))).x;
    // 488: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 489: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 490: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 491: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 492: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drg-00-wing1-mi-dead.v1 / source program 824358517f0ef0448e57d8ac93e923da
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase104(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[25].w=(g_SourceCharacterTime.xxxx).x;
    source[26].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[26].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[31] = g_SourceCharacterEnvironmentColor;
        source[32] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[27].wwww
    r0.xy = ((v4.xyxx)*(source[27].wwww)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[28].x
    r0.x = ((r0.xxxx)+(-(source[28].xxxx))).x;
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
    // 11: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 13: mad r0.xyz, cb0[24].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[24].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[25].xxxx, r2.xyzx, r0.xyzx
    r0.xyz = ((source[25].xxxx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 17: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 18: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 19: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 20: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 21: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 22: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 24: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 25: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 26: mul r0.w, r5.y, cb0[19].y
    r0.w = ((r5.yyyy)*(source[19].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: movc r0.w, r4.y, l(0), r0.w
    r0.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 31: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 32: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 33: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 34: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 35: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 36: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 37: mad r3.xyz, r0.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mad r2.xyz, r6.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r6.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 41: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 42: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 43: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 47: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 48: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 49: mad r2.xyz, r6.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 50: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 51: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 52: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 53: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 54: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 55: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 56: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 57: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 58: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 59: mad r2.xyz, r6.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 60: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 62: mad r3.xyz, cb0[24].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[24].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[25].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[25].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 67: mad r3.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 71: mul r8.xyz, r0.xyzx, r2.xyzx
    r8.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 72: dp3 r0.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r9.xyz, -r2.xyzx, r0.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 74: mad r0.xyz, r2.xyzx, r0.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 75: mad r2.xyz, cb0[24].wwww, r9.xyzx, r8.xyzx
    r2.xyz = ((source[24].wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r8.xyz, -r2.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[25].xxxx, r8.xyzx, r2.xyzx
    r2.xyz = ((source[25].xxxx)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[25].w
    r0.w = ((r0.wwww)*(source[25].wwww)).w;
    // 82: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 83: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 84: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r1.w, cb0[7].z, l(1.500000)
    r1.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 87: mad r0.w, r0.w, l(0.500000), cb0[7].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 88: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 89: mul r7.x, r1.w, l(0.125000)
    r7.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 90: mul r8.y, cb0[7].y, cb0[17].y
    r8.y = ((source[7].yyyy)*(source[17].yyyy)).y;
    // 91: mov r7.y, v4.y
    r7.y = (v4.yyyy).y;
    // 92: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 93: add r4.yw, r7.xxxy, r8.xxxy
    r4.yw = ((r7.xxxy)+(r8.xxxy)).yw;
    // 94: frc r1.w, cb0[7].x
    r1.w = (frac(source[7].xxxx)).w;
    // 95: add r2.w, -r1.w, cb0[7].x
    r2.w = ((-(r1.wwww))+(source[7].xxxx)).w;
    // 96: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 97: add r4.yw, r4.yyyw, r8.zzzw
    r4.yw = ((r4.yyyw)+(r8.zzzw)).yw;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r4.ywyy, t5.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 99: mul r8.xyz, r0.wwww, r8.xyzx
    r8.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 100: mul r0.w, r1.w, r8.w
    r0.w = ((r1.wwww)*(r8.wwww)).w;
    // 101: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 103: mad r2.xyz, r0.wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 104: mul r0.w, r5.x, cb0[26].z
    r0.w = ((r5.xxxx)*(source[26].zzzz)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[26].w
    r0.w = (saturate((r0.wwww)+(source[26].wwww))).w;
    // 108: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r4.xyw, r2.wwww, cb0[16].xyxz
    r4.xyw = ((r2.wwww)*(source[16].xyxz)).xyw;
    // 110: mul r5.xyw, r2.xyxz, r4.xyxw
    r5.xyw = ((r2.xyxz)*(r4.xyxw)).xyw;
    // 111: mad r2.xyz, -r4.xywx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r4.xywx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 112: mad r2.xyz, r0.wwww, r2.xyzx, r5.xywx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xywx)).xyz;
    // 113: add r4.xyw, -cb0[2].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r4.xyw = ((-(source[2].xyxz))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 114: mul r2.xyz, r2.xyzx, r4.xywx
    r2.xyz = ((r2.xyzx)*(r4.xywx)).xyz;
    // 115: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 116: mad r4.xyw, r2.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r4.xyw = ((r2.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 117: mad r5.xyw, r2.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r5.xyw = ((r2.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 118: mad r4.xyw, r0.wwww, r4.xyxw, r5.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)+(r5.xyxw)).xyw;
    // 119: mad r5.xyw, r2.xyxz, l(2.755200, 2.755200, 0.000000, 2.755200), l(0.690300, 0.690300, 0.000000, 0.690300)
    r5.xyw = ((r2.xyxz)*(float4(2.755200,2.755200,0.000000,2.755200))+(float4(0.690300,0.690300,0.000000,0.690300))).xyw;
    // 120: mad r4.xyw, r4.xyxw, r0.wwww, r5.xyxw
    r4.xyw = ((r4.xyxw)*(r0.wwww)+(r5.xyxw)).xyw;
    // 121: mul r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)).xyw;
    // 122: max r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = (max(r0.wwww,r4.xyxw)).xyw;
    // 123: mov_sat r2.w, cb0[27].z
    r2.w = (saturate(source[27].zzzz)).w;
    // 124: mad r5.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r5.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 125: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 126: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 127: mad r5.xyw, r7.wwww, r5.xyxw, r3.wwww
    r5.xyw = ((r7.wwww)*(r5.xyxw)+(r3.wwww)).xyw;
    // 128: add r2.w, cb0[28].w, -cb0[29].x
    r2.w = ((source[28].wwww)+(-(source[29].xxxx))).w;
    // 129: mad r2.w, r6.x, r2.w, cb0[29].x
    r2.w = ((r6.xxxx)*(r2.wwww)+(source[29].xxxx)).w;
    // 130: add r3.w, -r2.w, cb0[29].z
    r3.w = ((-(r2.wwww))+(source[29].zzzz)).w;
    // 131: mad r2.w, r6.y, r3.w, r2.w
    r2.w = ((r6.yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 132: add r3.w, -r2.w, cb0[30].x
    r3.w = ((-(r2.wwww))+(source[30].xxxx)).w;
    // 133: mad r2.w, r6.z, r3.w, r2.w
    r2.w = ((r6.zzzz)*(r3.wwww)+(r2.wwww)).w;
    // 134: mul r2.w, r5.z, r2.w
    r2.w = ((r5.zzzz)*(r2.wwww)).w;
    // 135: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 136: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: movc r2.w, r4.z, l(0), r2.w
    r2.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 138: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 139: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 141: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 142: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 143: mul r6.xy, r6.xyxx, cb0[19].xxxx
    r6.xy = ((r6.xyxx)*(source[19].xxxx)).xy;
    // 144: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 147: add r6.z, r2.w, l(0.000010)
    r6.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 148: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 149: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 150: div r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)/(r2.wwww)).xyz;
    // 151: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 152: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 153: mul r8.xyz, r2.wwww, r6.xyzx
    r8.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 154: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 156: mul r9.xyz, r2.wwww, v5.xyzx
    r9.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r2.w, r8.xyzx, r9.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 158: deriv_rtx_coarse r7.x, r2.w
    r7.x = (ddx_coarse(r2.wwww)).x;
    // 159: deriv_rty_coarse r7.y, r2.w
    r7.y = (ddy_coarse(r2.wwww)).y;
    // 160: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 161: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 162: mad r3.w, r3.w, l(0.300000), r7.z
    r3.w = ((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).w;
    // 163: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 164: min r7.y, r3.w, l(1.000000)
    r7.y = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r10.xyz, r5.xywx, r3.wwww
    r10.xyz = (max(r5.xywx,r3.wwww)).xyz;
    // 167: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 168: mul_sat r3.w, r5.y, l(50.000000)
    r3.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 170: mul r11.xyz, r2.wwww, r8.xyzx
    r11.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 171: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 172: add r3.w, r11.z, l(1.000000)
    r3.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r4.z, r2.w, l(1.000000)
    r4.z = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mov_sat r2.w, r2.w
    r2.w = (saturate(r2.wwww)).w;
    // 176: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 177: mul r2.w, r2.w, cb0[1].y
    r2.w = ((r2.wwww)*(source[1].yyyy)).w;
    // 178: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 179: mad_sat r2.w, r2.w, cb0[1].w, cb0[1].z
    r2.w = (saturate((r2.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 180: mul r2.w, r2.w, cb0[30].y
    r2.w = ((r2.wwww)*(source[30].yyyy)).w;
    // 181: add_sat r7.x, -r3.w, r4.z
    r7.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t6.xyzw, s7
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 183: add r3.w, r0.w, r7.x
    r3.w = ((r0.wwww)+(r7.xxxx)).w;
    // 184: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 185: mul r13.xyz, r5.xywx, r12.yyyy
    r13.xyz = ((r5.xywx)*(r12.yyyy)).xyz;
    // 186: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 187: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.yyyy)).z;
    // 188: add r4.z, r4.z, l(-1.000000)
    r4.z = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 189: mad r12.xyz, r5.xywx, r4.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r4.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 190: dp3 r4.z, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.z = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: mad r5.xyz, r4.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r4.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 194: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 195: add r4.z, -r7.w, l(1.000000)
    r4.z = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 196: mul r12.xyz, r4.zzzz, r12.xyzx
    r12.xyz = ((r4.zzzz)*(r12.xyzx)).xyz;
    // 197: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: mad r6.w, r7.y, l(2.000000), l(2.000000)
    r6.w = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 199: dp3 r7.x, v1.xyzx, v1.xyzx
    r7.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 200: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 201: mul r14.xyz, r7.xxxx, v1.xyzx
    r14.xyz = ((r7.xxxx)*(v1.xyzx)).xyz;
    // 202: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 203: dp3 r7.x, v0.xyzx, v0.xyzx
    r7.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 204: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 205: mul r16.xyz, r7.xxxx, v0.xyzx
    r16.xyz = ((r7.xxxx)*(v0.xyzx)).xyz;
    // 206: mul r17.xyz, r14.zxyz, r16.yzxy
    r17.xyz = ((r14.zxyz)*(r16.yzxy)).xyz;
    // 207: mad r17.xyz, r14.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r14.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 208: dp3 r14.y, r14.xyzx, r11.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 209: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 210: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 211: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 212: dp2 r15.z, r18.xyxx, cb0[32].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[32].xyxx).xy).xxxx).z;
    // 213: mul r7.xz, cb0[32].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[32].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 214: dp2 r15.x, r18.xyxx, r7.xzxx
    r15.x = (dot((r18.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 215: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 216: dp4 r19.x, cb0[33].xyzw, r15.xyzw
    r19.x = (dot((source[33].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 217: dp4 r19.y, cb0[34].xyzw, r15.xyzw
    r19.y = (dot((source[34].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 218: dp4 r19.z, cb0[35].xyzw, r15.xyzw
    r19.z = (dot((source[35].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 219: mul r20.xyzw, r15.yzzx, r15.xyzz
    r20.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 220: dp4 r21.x, cb0[36].xyzw, r20.xyzw
    r21.x = (dot((source[36].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).x;
    // 221: dp4 r21.y, cb0[37].xyzw, r20.xyzw
    r21.y = (dot((source[37].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).y;
    // 222: dp4 r21.z, cb0[38].xyzw, r20.xyzw
    r21.z = (dot((source[38].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).z;
    // 223: add r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)+(r21.xyzx)).xyz;
    // 224: mul r8.w, r15.y, r15.y
    r8.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 225: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 226: mad r8.w, r15.x, r15.x, -r8.w
    r8.w = ((r15.xxxx)*(r15.xxxx)+(-(r8.wwww))).w;
    // 227: mad r15.xyz, cb0[39].xyzx, r8.wwww, r19.xyzx
    r15.xyz = ((source[39].xyzx)*(r8.wwww)+(r19.xyzx)).xyz;
    // 228: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 229: mul r15.xyz, r15.xyzx, cb0[31].xyzx
    r15.xyz = ((r15.xyzx)*(source[31].xyzx)).xyz;
    // 230: mul r15.xyz, r15.xyzx, cb0[32].zzzz
    r15.xyz = ((r15.xyzx)*(source[32].zzzz)).xyz;
    // 231: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[31].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[31].wwww)).xyz;
    // 232: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: add r15.xyz, -r8.wwww, r15.xyzx
    r15.xyz = ((-(r8.wwww))+(r15.xyzx)).xyz;
    // 234: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r8.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r8.wwww)).xyz;
    // 235: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: div r8.w, r8.w, r6.w
    r8.w = ((r8.wwww)/(r6.wwww)).w;
    // 237: mad r8.w, r5.w, l(5.000000), r8.w
    r8.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r8.wwww)).w;
    // 238: add_sat r8.w, r7.w, r8.w
    r8.w = (saturate((r7.wwww)+(r8.wwww))).w;
    // 239: mad r9.w, r8.w, l(-2.000000), l(3.000000)
    r9.w = ((r8.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 240: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 241: mul r8.w, r8.w, r9.w
    r8.w = ((r8.wwww)*(r9.wwww)).w;
    // 242: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 243: mul r8.w, r8.w, l(1.500000)
    r8.w = ((r8.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 244: exp r8.w, r8.w
    r8.w = (exp2(r8.wwww)).w;
    // 245: mul r15.xyz, r8.wwww, r15.xyzx
    r15.xyz = ((r8.wwww)*(r15.xyzx)).xyz;
    // 246: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 247: mul r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r15.xyzx)).xyz;
    // 248: mul r12.xyz, r4.xywx, r12.xyzx
    r12.xyz = ((r4.xywx)*(r12.xyzx)).xyz;
    // 249: mul r8.w, r7.y, l(5.000000)
    r8.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 250: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 251: mul r3.w, r3.w, r7.y
    r3.w = ((r3.wwww)*(r7.yyyy)).w;
    // 252: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 253: add r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)+(r3.wwww)).w;
    // 254: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 255: add_sat r0.w, r3.w, l(-1.000000)
    r0.w = (saturate((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 256: dp3 r15.x, r16.xyzx, r11.xyzx
    r15.x = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 257: dp3 r15.y, r17.xyzx, r11.xyzx
    r15.y = (dot((r17.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 258: dp2 r14.x, r15.xyxx, r7.xzxx
    r14.x = (dot((r15.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 259: dp2 r14.z, r15.xyxx, cb0[32].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[32].xyxx).xy).xxxx).z;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t7.xyzw, s6, r8.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r8.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 261: mul r7.xyz, r14.xyzx, r14.wwww
    r7.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 262: mul r7.xyz, r7.xyzx, cb0[31].xyzx
    r7.xyz = ((r7.xyzx)*(source[31].xyzx)).xyz;
    // 263: mul r7.xyz, r7.xyzx, cb0[32].zzzz
    r7.xyz = ((r7.xyzx)*(source[32].zzzz)).xyz;
    // 264: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[31].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[31].wwww)).xyz;
    // 265: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r7.xyz, -r3.wwww, r7.xyzx
    r7.xyz = ((-(r3.wwww))+(r7.xyzx)).xyz;
    // 267: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 268: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)/(r6.wwww)).w;
    // 270: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 271: add_sat r3.w, r7.w, r3.w
    r3.w = (saturate((r7.wwww)+(r3.wwww))).w;
    // 272: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 274: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 275: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 276: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 278: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 279: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 280: mad r3.w, r0.w, r5.x, r5.y
    r3.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 281: mad r3.w, r3.w, r0.w, r5.z
    r3.w = ((r3.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 282: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 283: max r0.w, r0.w, r3.w
    r0.w = (max(r0.wwww,r3.wwww)).w;
    // 284: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 285: dp3 r3.w, v6.xyzx, v6.xyzx
    r3.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 287: mul r12.xyz, r3.wwww, v6.xyzx
    r12.xyz = ((r3.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r3.w, r12.xyzx, r8.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 289: dp3 r5.w, -r12.xyzx, r8.xyzx
    r5.w = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).w;
    // 290: dp3 r6.w, r12.xyzx, r11.xyzx
    r6.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 291: mad r8.xy, r6.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r6.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 292: mad r8.zw, r5.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r5.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 293: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 294: mad r11.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 296: mul r11.yzw, r11.yyyy, cb0[42].xxyz
    r11.yzw = ((r11.yyyy)*(source[42].xxyz)).yzw;
    // 297: mad r11.xyz, r11.xxxx, cb0[41].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[41].xyzx)+(r11.yzwy)).xyz;
    // 298: mul r11.xyz, r11.xyzx, cb0[43].wwww
    r11.xyz = ((r11.xyzx)*(source[43].wwww)).xyz;
    // 299: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 300: mul r4.xyw, r4.xyxw, r11.xyxz
    r4.xyw = ((r4.xyxw)*(r11.xyxz)).xyw;
    // 301: mul r4.xyw, r4.xyxw, l(0.600000, 0.600000, 0.000000, 0.600000)
    r4.xyw = ((r4.xyxw)*(float4(0.600000,0.600000,0.000000,0.600000))).xyw;
    // 302: mul r4.xyw, r13.xyxz, r4.xyxw
    r4.xyw = ((r13.xyxz)*(r4.xyxw)).xyw;
    // 303: mad r4.xyw, -r4.xyxw, r7.wwww, r4.xyxw
    r4.xyw = ((-(r4.xyxw))*(r7.wwww)+(r4.xyxw)).xyw;
    // 304: mad r4.xyw, r5.xyxz, l(0.400000, 0.400000, 0.000000, 0.400000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.400000,0.400000,0.000000,0.400000))+(r4.xyxw)).xyw;
    // 305: mul r5.xyz, r8.yyyy, cb0[42].xyzx
    r5.xyz = ((r8.yyyy)*(source[42].xyzx)).xyz;
    // 306: mad r5.xyz, cb0[41].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[41].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, cb0[43].wwww
    r5.xyz = ((r5.xyzx)*(source[43].wwww)).xyz;
    // 308: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 309: mul r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 310: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 311: mad r4.xyw, r5.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r4.xyxw)).xyw;
    // 312: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 313: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 314: mul r0.w, r17.z, cb0[20].w
    r0.w = ((r17.zzzz)*(source[20].wwww)).w;
    // 315: mul r3.w, r17.z, cb0[21].x
    r3.w = ((r17.zzzz)*(source[21].xxxx)).w;
    // 316: mad r5.x, r16.z, cb0[20].w, -r3.w
    r5.x = ((r16.zzzz)*(source[20].wwww)+(-(r3.wwww))).x;
    // 317: mad r5.y, r16.z, cb0[21].x, r0.w
    r5.y = ((r16.zzzz)*(source[21].xxxx)+(r0.wwww)).y;
    // 318: max r0.w, |r5.x|, |r5.y|
    r0.w = (max(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 319: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 320: min r3.w, |r5.x|, |r5.y|
    r3.w = (min(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 321: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 322: mul r3.w, r0.w, r0.w
    r3.w = ((r0.wwww)*(r0.wwww)).w;
    // 323: mad r5.z, r3.w, l(0.020835), l(-0.085133)
    r5.z = ((r3.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 324: mad r5.z, r3.w, r5.z, l(0.180141)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 325: mad r5.z, r3.w, r5.z, l(-0.330299)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 326: mad r3.w, r3.w, r5.z, l(0.999866)
    r3.w = ((r3.wwww)*(r5.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 327: mul r5.z, r0.w, r3.w
    r5.z = ((r0.wwww)*(r3.wwww)).z;
    // 328: mad r5.z, r5.z, l(-2.000000), l(1.570796)
    r5.z = ((r5.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 329: lt r5.w, |r5.x|, |r5.y|
    r5.w = (asfloat((uint4)((abs(r5.xxxx))<(abs(r5.yyyy))) * 0xffffffffu)).w;
    // 330: and r5.z, r5.w, r5.z
    r5.z = (asfloat(asuint(r5.wwww) & asuint(r5.zzzz))).z;
    // 331: mad r0.w, r0.w, r3.w, r5.z
    r0.w = ((r0.wwww)*(r3.wwww)+(r5.zzzz)).w;
    // 332: lt r3.w, r5.x, -r5.x
    r3.w = (asfloat((uint4)((r5.xxxx)<(-(r5.xxxx))) * 0xffffffffu)).w;
    // 333: and r3.w, r3.w, l(0xc0490fdb)
    r3.w = (asfloat(asuint(r3.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 334: add r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)+(r3.wwww)).w;
    // 335: min r3.w, r5.x, r5.y
    r3.w = (min(r5.xxxx,r5.yyyy)).w;
    // 336: lt r3.w, r3.w, -r3.w
    r3.w = (asfloat((uint4)((r3.wwww)<(-(r3.wwww))) * 0xffffffffu)).w;
    // 337: max r5.z, r5.x, r5.y
    r5.z = (max(r5.xxxx,r5.yyyy)).z;
    // 338: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 339: add r5.x, r5.y, r5.x
    r5.x = ((r5.yyyy)+(r5.xxxx)).x;
    // 340: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 341: ge r5.y, r5.z, -r5.z
    r5.y = (asfloat((uint4)((r5.zzzz)>=(-(r5.zzzz))) * 0xffffffffu)).y;
    // 342: and r3.w, r3.w, r5.y
    r3.w = (asfloat(asuint(r3.wwww) & asuint(r5.yyyy))).w;
    // 343: movc r0.w, r3.w, -r0.w, r0.w
    r0.w = ((asuint(r3.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 344: add r3.w, r0.w, l(6.283185)
    r3.w = ((r0.wwww)+(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 345: mul r3.w, r3.w, l(0.159155)
    r3.w = ((r3.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 346: mul r5.y, r0.w, l(0.159155)
    r5.y = ((r0.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).y;
    // 347: ge r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 348: movc r0.w, r0.w, r5.y, r3.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (r5.yyyy) : (r3.wwww)).w;
    // 349: add r3.w, -r0.w, -cb0[21].w
    r3.w = ((-(r0.wwww))+(-(source[21].wwww))).w;
    // 350: add r0.w, r0.w, -cb0[21].w
    r0.w = ((r0.wwww)+(-(source[21].wwww))).w;
    // 351: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 352: add r5.y, -cb0[21].w, l(1.000000)
    r5.y = ((-(source[21].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 353: div r5.y, l(1.000000, 1.000000, 1.000000, 1.000000), r5.y
    r5.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r5.yyyy)).y;
    // 354: mul_sat r3.w, r3.w, r5.y
    r3.w = (saturate((r3.wwww)*(r5.yyyy))).w;
    // 355: mul_sat r0.w, r0.w, r5.y
    r0.w = (saturate((r0.wwww)*(r5.yyyy))).w;
    // 356: mad r5.y, r3.w, l(-2.000000), l(3.000000)
    r5.y = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 357: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 358: mul r3.w, r3.w, r5.y
    r3.w = ((r3.wwww)*(r5.yyyy)).w;
    // 359: mad r5.y, r0.w, l(-2.000000), l(3.000000)
    r5.y = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 360: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 361: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 362: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 363: log r3.w, r5.x
    r3.w = (log2(r5.xxxx)).w;
    // 364: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 365: mul r3.w, r3.w, l(5.082000)
    r3.w = ((r3.wwww)*(float4(5.082000,5.082000,5.082000,5.082000))).w;
    // 366: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 367: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 368: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 369: dp3 r3.w, r6.xyzx, r9.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 370: mul_sat r5.x, r3.w, cb0[22].x
    r5.x = (saturate((r3.wwww)*(source[22].xxxx))).x;
    // 371: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 372: mul_sat r5.y, r9.z, cb0[22].x
    r5.y = (saturate((r9.zzzz)*(source[22].xxxx))).y;
    // 373: add r5.z, -|r9.z|, l(1.000000)
    r5.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 374: mul r3.w, r3.w, r5.z
    r3.w = ((r3.wwww)*(r5.zzzz)).w;
    // 375: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 376: add_sat r5.y, r5.y, -cb0[22].y
    r5.y = (saturate((r5.yyyy)+(-(source[22].yyyy)))).y;
    // 377: log r5.z, r5.y
    r5.z = (log2(r5.yyyy)).z;
    // 378: lt r5.y, r5.y, l(0.000001)
    r5.y = (asfloat((uint4)((r5.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 379: mul r5.z, r5.z, cb0[22].z
    r5.z = ((r5.zzzz)*(source[22].zzzz)).z;
    // 380: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 381: mul r5.x, r5.z, r5.x
    r5.x = ((r5.zzzz)*(r5.xxxx)).x;
    // 382: movc r5.x, r5.y, l(0), r5.x
    r5.x = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).x;
    // 383: mul r5.y, r5.x, cb0[22].w
    r5.y = ((r5.xxxx)*(source[22].wwww)).y;
    // 384: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 385: mul r0.w, r0.w, cb0[22].w
    r0.w = ((r0.wwww)*(source[22].wwww)).w;
    // 386: lt r5.y, |r0.w|, l(0.000001)
    r5.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 387: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 388: mul r0.w, r0.w, cb0[23].x
    r0.w = ((r0.wwww)*(source[23].xxxx)).w;
    // 389: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 390: mul r6.xyz, cb0[8].xyzx, cb0[8].wwww
    r6.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 391: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 392: add r5.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r5.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 393: dp2 r0.w, cb0[9].xyxx, r5.zwzz
    r0.w = (dot((source[9].xyxx).xy,(r5.zwzz).xy).xxxx).w;
    // 394: add r0.w, r0.w, cb0[24].z
    r0.w = ((r0.wwww)+(source[24].zzzz)).w;
    // 395: add_sat r0.w, r0.w, l(-0.500000)
    r0.w = (saturate((r0.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).w;
    // 396: mul r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)*(r0.wwww)).xyz;
    // 397: movc r5.yzw, r5.yyyy, l(0,0,0,0), r6.xxyz
    r5.yzw = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxyz)).yzw;
    // 398: mul r6.xyz, r1.wwww, r5.yzwy
    r6.xyz = ((r1.wwww)*(r5.yzwy)).xyz;
    // 399: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 400: mad r5.yzw, -r1.wwww, r5.yyzw, r0.wwww
    r5.yzw = ((-(r1.wwww))*(r5.yyzw)+(r0.wwww)).yzw;
    // 401: mad r5.yzw, cb0[24].wwww, r5.yyzw, r6.xxyz
    r5.yzw = ((source[24].wwww)*(r5.yyzw)+(r6.xxyz)).yzw;
    // 402: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 403: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 404: mad r5.yzw, cb0[25].xxxx, r6.xxyz, r5.yyzw
    r5.yzw = ((source[25].xxxx)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 405: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 406: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 407: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 408: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 409: add r6.xyz, -r0.xyzx, r0.wwww
    r6.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 410: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 411: mul r6.xyz, cb0[14].xyzx, cb0[25].zzzz
    r6.xyz = ((source[14].xyzx)*(source[25].zzzz)).xyz;
    // 412: mul r6.xyz, r6.xyzx, cb0[26].yyyy
    r6.xyz = ((r6.xyzx)*(source[26].yyyy)).xyz;
    // 413: mul r6.xyz, r5.xxxx, r6.xyzx
    r6.xyz = ((r5.xxxx)*(r6.xyzx)).xyz;
    // 414: mad r7.xyz, r5.xxxx, cb0[13].xyzx, -cb0[13].xyzx
    r7.xyz = ((r5.xxxx)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 415: add r0.w, r5.x, l(-1.000000)
    r0.w = ((r5.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 416: mad r0.w, cb0[12].w, r0.w, l(1.000000)
    r0.w = ((source[12].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 417: mad r7.xyz, cb0[13].wwww, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((source[13].wwww)*(r7.xyzx)+(source[13].xyzx)).xyz;
    // 418: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 419: mad r0.xyz, r0.wwww, cb0[12].xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(source[12].xyzx)+(r0.xyzx)).xyz;
    // 420: mad r0.xyz, r5.yzwy, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.yzwy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 421: log r0.w, |r3.w|
    r0.w = (log2(abs(r3.wwww))).w;
    // 422: lt r1.w, |r3.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 423: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 424: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 425: mul r3.xyz, r0.wwww, cb0[15].xyzx
    r3.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 426: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 427: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 428: mad r0.xyz, cb0[19].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 429: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 430: mul r1.xyz, r8.wwww, cb0[42].xyzx
    r1.xyz = ((r8.wwww)*(source[42].xyzx)).xyz;
    // 431: mad r1.xyz, r8.zzzz, cb0[41].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[41].xyzx)+(r1.xyzx)).xyz;
    // 432: mul r1.xyz, r1.xyzx, cb0[43].wwww
    r1.xyz = ((r1.xyzx)*(source[43].wwww)).xyz;
    // 433: mul_sat r3.xyz, cb0[18].xyzx, cb0[18].wwww
    r3.xyz = (saturate((source[18].xyzx)*(source[18].wwww))).xyz;
    // 434: mul r5.xyz, r2.wwww, r3.xyzx
    r5.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 435: mul r3.xyz, r3.xyzx, cb0[30].yyyy
    r3.xyz = ((r3.xyzx)*(source[30].yyyy)).xyz;
    // 436: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 437: mul r3.xyz, r4.zzzz, r5.xyzx
    r3.xyz = ((r4.zzzz)*(r5.xyzx)).xyz;
    // 438: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 439: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 440: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 441: add r0.xyz, r4.xywx, r0.xyzx
    r0.xyz = ((r4.xywx)+(r0.xyzx)).xyz;
    // 442: dp3 o4.y, r4.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 443: mad o0.xyz, r2.xyzx, cb0[43].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[43].xyzx)+(r0.xyzx)).xyz;
    // 444: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 445: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 446: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 447: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 448: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 449: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 450: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 451: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 452: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 453: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 454: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 455: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 456: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 457: ftou r0.x, cb0[40].z
    r0.x = (asfloat((uint4)(source[40].zzzz))).x;
    // 458: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 459: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 460: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 461: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 462: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drg-00-wing-mi-dead.v1 / source program fcf2ae320f020d48b6a207cedc870d36
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase105(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[21]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[29].z=(g_SourceCharacterTime.xxxx).x;
    source[31].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[32].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[32].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[32].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[32].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[37] = g_SourceCharacterEnvironmentColor;
        source[38] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[34].yyyy
    r0.xy = ((v4.xyxx)*(source[34].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[34].z
    r0.x = ((r0.xxxx)+(-(source[34].zzzz))).x;
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
    // 11: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 13: mad r0.xyz, cb0[30].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[30].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[30].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[30].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 17: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 18: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 19: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 20: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 21: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 22: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 24: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 25: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 26: mul r0.w, r5.y, cb0[23].y
    r0.w = ((r5.yyyy)*(source[23].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: movc r0.w, r4.y, l(0), r0.w
    r0.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 31: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 32: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 33: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 34: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 35: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 36: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 37: mad r3.xyz, r0.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mad r2.xyz, r6.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r6.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 41: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 42: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 43: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 47: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 48: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 49: mad r2.xyz, r6.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 50: mul r3.xyz, cb0[7].xyzx, cb0[7].wwww
    r3.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 51: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 52: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 53: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 54: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 55: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 56: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 57: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 58: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 59: mad r2.xyz, r6.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 60: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 62: mad r3.xyz, cb0[30].zzzz, r3.xyzx, r2.xyzx
    r3.xyz = ((source[30].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[30].wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((source[30].wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 67: mad r3.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 71: mul r8.xyz, r0.xyzx, r2.xyzx
    r8.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 72: dp3 r0.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r9.xyz, -r2.xyzx, r0.xyzx, r0.wwww
    r9.xyz = ((-(r2.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 74: mad r0.xyz, r2.xyzx, r0.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 75: mad r2.xyz, cb0[30].zzzz, r9.xyzx, r8.xyzx
    r2.xyz = ((source[30].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r8.xyz, -r2.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[30].wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((source[30].wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[8].w, l(1.000000)
    r0.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[29].z
    r0.w = ((r0.wwww)*(source[29].zzzz)).w;
    // 82: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 83: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 84: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r1.w, cb0[8].z, l(1.500000)
    r1.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 87: mad r0.w, r0.w, l(0.500000), cb0[8].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 88: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 89: mul r7.x, r1.w, l(0.125000)
    r7.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 90: mul r8.y, cb0[8].y, cb0[21].y
    r8.y = ((source[8].yyyy)*(source[21].yyyy)).y;
    // 91: mov r7.y, v4.y
    r7.y = (v4.yyyy).y;
    // 92: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 93: add r4.yw, r7.xxxy, r8.xxxy
    r4.yw = ((r7.xxxy)+(r8.xxxy)).yw;
    // 94: frc r1.w, cb0[8].x
    r1.w = (frac(source[8].xxxx)).w;
    // 95: add r2.w, -r1.w, cb0[8].x
    r2.w = ((-(r1.wwww))+(source[8].xxxx)).w;
    // 96: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 97: add r4.yw, r4.yyyw, r8.zzzw
    r4.yw = ((r4.yyyw)+(r8.zzzw)).yw;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r4.ywyy, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 99: mul r8.xyz, r0.wwww, r8.xyzx
    r8.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 100: mul r0.w, r1.w, r8.w
    r0.w = ((r1.wwww)*(r8.wwww)).w;
    // 101: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 103: mad r2.xyz, r0.wwww, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 104: mul r0.w, r5.x, cb0[33].x
    r0.w = ((r5.xxxx)*(source[33].xxxx)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[33].y
    r0.w = (saturate((r0.wwww)+(source[33].yyyy))).w;
    // 108: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mul r4.xyw, r2.wwww, cb0[20].xyxz
    r4.xyw = ((r2.wwww)*(source[20].xyxz)).xyw;
    // 110: mul r5.xyw, r2.xyxz, r4.xyxw
    r5.xyw = ((r2.xyxz)*(r4.xyxw)).xyw;
    // 111: mad r2.xyz, -r4.xywx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r4.xywx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 112: mad r2.xyz, r0.wwww, r2.xyzx, r5.xywx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xywx)).xyz;
    // 113: add r4.xyw, -cb0[3].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r4.xyw = ((-(source[3].xyxz))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 114: mul r2.xyz, r2.xyzx, r4.xywx
    r2.xyz = ((r2.xyzx)*(r4.xywx)).xyz;
    // 115: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 116: mad r4.xyw, r2.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r4.xyw = ((r2.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 117: mad r5.xyw, r2.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r5.xyw = ((r2.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 118: mad r4.xyw, r0.wwww, r4.xyxw, r5.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)+(r5.xyxw)).xyw;
    // 119: mad r5.xyw, r2.xyxz, l(2.755200, 2.755200, 0.000000, 2.755200), l(0.690300, 0.690300, 0.000000, 0.690300)
    r5.xyw = ((r2.xyxz)*(float4(2.755200,2.755200,0.000000,2.755200))+(float4(0.690300,0.690300,0.000000,0.690300))).xyw;
    // 120: mad r4.xyw, r4.xyxw, r0.wwww, r5.xyxw
    r4.xyw = ((r4.xyxw)*(r0.wwww)+(r5.xyxw)).xyw;
    // 121: mul r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = ((r0.wwww)*(r4.xyxw)).xyw;
    // 122: max r4.xyw, r0.wwww, r4.xyxw
    r4.xyw = (max(r0.wwww,r4.xyxw)).xyw;
    // 123: mov_sat r2.w, cb0[34].x
    r2.w = (saturate(source[34].xxxx)).w;
    // 124: mad r5.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r5.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 125: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 126: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 127: mad r5.xyw, r7.wwww, r5.xyxw, r3.wwww
    r5.xyw = ((r7.wwww)*(r5.xyxw)+(r3.wwww)).xyw;
    // 128: add r2.w, -cb0[35].z, cb0[35].y
    r2.w = ((-(source[35].zzzz))+(source[35].yyyy)).w;
    // 129: mad r2.w, r6.x, r2.w, cb0[35].z
    r2.w = ((r6.xxxx)*(r2.wwww)+(source[35].zzzz)).w;
    // 130: add r3.w, -r2.w, cb0[36].x
    r3.w = ((-(r2.wwww))+(source[36].xxxx)).w;
    // 131: mad r2.w, r6.y, r3.w, r2.w
    r2.w = ((r6.yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 132: add r3.w, -r2.w, cb0[36].z
    r3.w = ((-(r2.wwww))+(source[36].zzzz)).w;
    // 133: mad r2.w, r6.z, r3.w, r2.w
    r2.w = ((r6.zzzz)*(r3.wwww)+(r2.wwww)).w;
    // 134: mul r2.w, r5.z, r2.w
    r2.w = ((r5.zzzz)*(r2.wwww)).w;
    // 135: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 136: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: movc r2.w, r4.z, l(0), r2.w
    r2.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 138: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 139: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 141: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 142: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 143: mul r6.xy, r6.xyxx, cb0[23].xxxx
    r6.xy = ((r6.xyxx)*(source[23].xxxx)).xy;
    // 144: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 147: add r6.z, r2.w, l(0.000010)
    r6.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 148: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 149: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 150: div r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)/(r2.wwww)).xyz;
    // 151: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 152: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 153: mul r8.xyz, r2.wwww, r6.xyzx
    r8.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 154: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 156: mul r9.xyz, r2.wwww, v5.xyzx
    r9.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r2.w, r8.xyzx, r9.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 158: deriv_rtx_coarse r7.x, r2.w
    r7.x = (ddx_coarse(r2.wwww)).x;
    // 159: deriv_rty_coarse r7.y, r2.w
    r7.y = (ddy_coarse(r2.wwww)).y;
    // 160: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 161: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 162: mad r3.w, r3.w, l(0.300000), r7.z
    r3.w = ((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).w;
    // 163: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 164: min r7.y, r3.w, l(1.000000)
    r7.y = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r10.xyz, r5.xywx, r3.wwww
    r10.xyz = (max(r5.xywx,r3.wwww)).xyz;
    // 167: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 168: mul_sat r3.w, r5.y, l(50.000000)
    r3.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 170: mul r11.xyz, r2.wwww, r8.xyzx
    r11.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 171: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 172: add r3.w, r11.z, l(1.000000)
    r3.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r4.z, r2.w, l(1.000000)
    r4.z = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mov_sat r2.w, r2.w
    r2.w = (saturate(r2.wwww)).w;
    // 176: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 177: mul r2.w, r2.w, cb0[2].y
    r2.w = ((r2.wwww)*(source[2].yyyy)).w;
    // 178: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 179: mad_sat r2.w, r2.w, cb0[2].w, cb0[2].z
    r2.w = (saturate((r2.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 180: mul r2.w, r2.w, cb0[36].w
    r2.w = ((r2.wwww)*(source[36].wwww)).w;
    // 181: add_sat r7.x, -r3.w, r4.z
    r7.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t8.xyzw, s9
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 183: add r3.w, r0.w, r7.x
    r3.w = ((r0.wwww)+(r7.xxxx)).w;
    // 184: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 185: mul r13.xyz, r5.xywx, r12.yyyy
    r13.xyz = ((r5.xywx)*(r12.yyyy)).xyz;
    // 186: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 187: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.yyyy)).z;
    // 188: add r4.z, r4.z, l(-1.000000)
    r4.z = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 189: mad r12.xyz, r5.xywx, r4.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r4.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 190: dp3 r4.z, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.z = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: mad r5.xyz, r4.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r4.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 194: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 195: add r4.z, -r7.w, l(1.000000)
    r4.z = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 196: mul r12.xyz, r4.zzzz, r12.xyzx
    r12.xyz = ((r4.zzzz)*(r12.xyzx)).xyz;
    // 197: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: mad r6.w, r7.y, l(2.000000), l(2.000000)
    r6.w = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 199: dp3 r7.x, v1.xyzx, v1.xyzx
    r7.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 200: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 201: mul r14.xyz, r7.xxxx, v1.xyzx
    r14.xyz = ((r7.xxxx)*(v1.xyzx)).xyz;
    // 202: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 203: dp3 r7.x, v0.xyzx, v0.xyzx
    r7.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 204: rsq r7.x, r7.x
    r7.x = (rsqrt(r7.xxxx)).x;
    // 205: mul r16.xyz, r7.xxxx, v0.xyzx
    r16.xyz = ((r7.xxxx)*(v0.xyzx)).xyz;
    // 206: mul r17.xyz, r14.zxyz, r16.yzxy
    r17.xyz = ((r14.zxyz)*(r16.yzxy)).xyz;
    // 207: mad r17.xyz, r14.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r14.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 208: dp3 r14.y, r14.xyzx, r11.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 209: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 210: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 211: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 212: dp2 r15.z, r18.xyxx, cb0[38].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[38].xyxx).xy).xxxx).z;
    // 213: mul r7.xz, cb0[38].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[38].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 214: dp2 r15.x, r18.xyxx, r7.xzxx
    r15.x = (dot((r18.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 215: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 216: dp4 r19.x, cb0[39].xyzw, r15.xyzw
    r19.x = (dot((source[39].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 217: dp4 r19.y, cb0[40].xyzw, r15.xyzw
    r19.y = (dot((source[40].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 218: dp4 r19.z, cb0[41].xyzw, r15.xyzw
    r19.z = (dot((source[41].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 219: mul r20.xyzw, r15.yzzx, r15.xyzz
    r20.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 220: dp4 r21.x, cb0[42].xyzw, r20.xyzw
    r21.x = (dot((source[42].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).x;
    // 221: dp4 r21.y, cb0[43].xyzw, r20.xyzw
    r21.y = (dot((source[43].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).y;
    // 222: dp4 r21.z, cb0[44].xyzw, r20.xyzw
    r21.z = (dot((source[44].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).z;
    // 223: add r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)+(r21.xyzx)).xyz;
    // 224: mul r8.w, r15.y, r15.y
    r8.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 225: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 226: mad r8.w, r15.x, r15.x, -r8.w
    r8.w = ((r15.xxxx)*(r15.xxxx)+(-(r8.wwww))).w;
    // 227: mad r15.xyz, cb0[45].xyzx, r8.wwww, r19.xyzx
    r15.xyz = ((source[45].xyzx)*(r8.wwww)+(r19.xyzx)).xyz;
    // 228: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 229: mul r15.xyz, r15.xyzx, cb0[37].xyzx
    r15.xyz = ((r15.xyzx)*(source[37].xyzx)).xyz;
    // 230: mul r15.xyz, r15.xyzx, cb0[38].zzzz
    r15.xyz = ((r15.xyzx)*(source[38].zzzz)).xyz;
    // 231: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[37].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[37].wwww)).xyz;
    // 232: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: add r15.xyz, -r8.wwww, r15.xyzx
    r15.xyz = ((-(r8.wwww))+(r15.xyzx)).xyz;
    // 234: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r8.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r8.wwww)).xyz;
    // 235: dp3 r8.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: div r8.w, r8.w, r6.w
    r8.w = ((r8.wwww)/(r6.wwww)).w;
    // 237: mad r8.w, r5.w, l(5.000000), r8.w
    r8.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r8.wwww)).w;
    // 238: add_sat r8.w, r7.w, r8.w
    r8.w = (saturate((r7.wwww)+(r8.wwww))).w;
    // 239: mad r9.w, r8.w, l(-2.000000), l(3.000000)
    r9.w = ((r8.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 240: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 241: mul r8.w, r8.w, r9.w
    r8.w = ((r8.wwww)*(r9.wwww)).w;
    // 242: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 243: mul r8.w, r8.w, l(1.500000)
    r8.w = ((r8.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 244: exp r8.w, r8.w
    r8.w = (exp2(r8.wwww)).w;
    // 245: mul r15.xyz, r8.wwww, r15.xyzx
    r15.xyz = ((r8.wwww)*(r15.xyzx)).xyz;
    // 246: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 247: mul r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r15.xyzx)).xyz;
    // 248: mul r12.xyz, r4.xywx, r12.xyzx
    r12.xyz = ((r4.xywx)*(r12.xyzx)).xyz;
    // 249: mul r8.w, r7.y, l(5.000000)
    r8.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 250: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 251: mul r3.w, r3.w, r7.y
    r3.w = ((r3.wwww)*(r7.yyyy)).w;
    // 252: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 253: add r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)+(r3.wwww)).w;
    // 254: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 255: add_sat r0.w, r3.w, l(-1.000000)
    r0.w = (saturate((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 256: dp3 r15.x, r16.xyzx, r11.xyzx
    r15.x = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 257: dp3 r15.y, r17.xyzx, r11.xyzx
    r15.y = (dot((r17.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 258: dp2 r14.x, r15.xyxx, r7.xzxx
    r14.x = (dot((r15.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 259: dp2 r14.z, r15.xyxx, cb0[38].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[38].xyxx).xy).xxxx).z;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t9.xyzw, s8, r8.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r8.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 261: mul r7.xyz, r14.xyzx, r14.wwww
    r7.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 262: mul r7.xyz, r7.xyzx, cb0[37].xyzx
    r7.xyz = ((r7.xyzx)*(source[37].xyzx)).xyz;
    // 263: mul r7.xyz, r7.xyzx, cb0[38].zzzz
    r7.xyz = ((r7.xyzx)*(source[38].zzzz)).xyz;
    // 264: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[37].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[37].wwww)).xyz;
    // 265: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r7.xyz, -r3.wwww, r7.xyzx
    r7.xyz = ((-(r3.wwww))+(r7.xyzx)).xyz;
    // 267: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 268: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)/(r6.wwww)).w;
    // 270: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 271: add_sat r3.w, r7.w, r3.w
    r3.w = (saturate((r7.wwww)+(r3.wwww))).w;
    // 272: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 274: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 275: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 276: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 278: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 279: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 280: mad r3.w, r0.w, r5.x, r5.y
    r3.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 281: mad r3.w, r3.w, r0.w, r5.z
    r3.w = ((r3.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 282: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 283: max r0.w, r0.w, r3.w
    r0.w = (max(r0.wwww,r3.wwww)).w;
    // 284: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 285: dp3 r3.w, v6.xyzx, v6.xyzx
    r3.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 287: mul r12.xyz, r3.wwww, v6.xyzx
    r12.xyz = ((r3.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r3.w, r12.xyzx, r8.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 289: dp3 r5.w, -r12.xyzx, r8.xyzx
    r5.w = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).w;
    // 290: dp3 r6.w, r12.xyzx, r11.xyzx
    r6.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 291: mad r8.xy, r6.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r6.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 292: mad r8.zw, r5.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r5.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 293: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 294: mad r11.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 296: mul r11.yzw, r11.yyyy, cb0[48].xxyz
    r11.yzw = ((r11.yyyy)*(source[48].xxyz)).yzw;
    // 297: mad r11.xyz, r11.xxxx, cb0[47].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[47].xyzx)+(r11.yzwy)).xyz;
    // 298: mul r11.xyz, r11.xyzx, cb0[49].wwww
    r11.xyz = ((r11.xyzx)*(source[49].wwww)).xyz;
    // 299: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 300: mul r4.xyw, r4.xyxw, r11.xyxz
    r4.xyw = ((r4.xyxw)*(r11.xyxz)).xyw;
    // 301: mul r4.xyw, r4.xyxw, l(0.600000, 0.600000, 0.000000, 0.600000)
    r4.xyw = ((r4.xyxw)*(float4(0.600000,0.600000,0.000000,0.600000))).xyw;
    // 302: mul r4.xyw, r13.xyxz, r4.xyxw
    r4.xyw = ((r13.xyxz)*(r4.xyxw)).xyw;
    // 303: mad r4.xyw, -r4.xyxw, r7.wwww, r4.xyxw
    r4.xyw = ((-(r4.xyxw))*(r7.wwww)+(r4.xyxw)).xyw;
    // 304: mad r4.xyw, r5.xyxz, l(0.400000, 0.400000, 0.000000, 0.400000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.400000,0.400000,0.000000,0.400000))+(r4.xyxw)).xyw;
    // 305: mul r5.xyz, r8.yyyy, cb0[48].xyzx
    r5.xyz = ((r8.yyyy)*(source[48].xyzx)).xyz;
    // 306: mad r5.xyz, cb0[47].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[47].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, cb0[49].wwww
    r5.xyz = ((r5.xyzx)*(source[49].wwww)).xyz;
    // 308: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 309: mul r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 310: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 311: mad r4.xyw, r5.xyxz, l(0.600000, 0.600000, 0.000000, 0.600000), r4.xyxw
    r4.xyw = ((r5.xyxz)*(float4(0.600000,0.600000,0.000000,0.600000))+(r4.xyxw)).xyw;
    // 312: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 313: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 314: mul r0.w, r17.z, cb0[24].w
    r0.w = ((r17.zzzz)*(source[24].wwww)).w;
    // 315: mul r3.w, r17.z, cb0[25].x
    r3.w = ((r17.zzzz)*(source[25].xxxx)).w;
    // 316: mad r5.x, r16.z, cb0[24].w, -r3.w
    r5.x = ((r16.zzzz)*(source[24].wwww)+(-(r3.wwww))).x;
    // 317: mad r5.y, r16.z, cb0[25].x, r0.w
    r5.y = ((r16.zzzz)*(source[25].xxxx)+(r0.wwww)).y;
    // 318: max r0.w, |r5.x|, |r5.y|
    r0.w = (max(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 319: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 320: min r3.w, |r5.x|, |r5.y|
    r3.w = (min(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 321: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 322: mul r3.w, r0.w, r0.w
    r3.w = ((r0.wwww)*(r0.wwww)).w;
    // 323: mad r5.z, r3.w, l(0.020835), l(-0.085133)
    r5.z = ((r3.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 324: mad r5.z, r3.w, r5.z, l(0.180141)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 325: mad r5.z, r3.w, r5.z, l(-0.330299)
    r5.z = ((r3.wwww)*(r5.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 326: mad r3.w, r3.w, r5.z, l(0.999866)
    r3.w = ((r3.wwww)*(r5.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 327: mul r5.z, r0.w, r3.w
    r5.z = ((r0.wwww)*(r3.wwww)).z;
    // 328: mad r5.z, r5.z, l(-2.000000), l(1.570796)
    r5.z = ((r5.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 329: lt r5.w, |r5.x|, |r5.y|
    r5.w = (asfloat((uint4)((abs(r5.xxxx))<(abs(r5.yyyy))) * 0xffffffffu)).w;
    // 330: and r5.z, r5.w, r5.z
    r5.z = (asfloat(asuint(r5.wwww) & asuint(r5.zzzz))).z;
    // 331: mad r0.w, r0.w, r3.w, r5.z
    r0.w = ((r0.wwww)*(r3.wwww)+(r5.zzzz)).w;
    // 332: lt r3.w, r5.x, -r5.x
    r3.w = (asfloat((uint4)((r5.xxxx)<(-(r5.xxxx))) * 0xffffffffu)).w;
    // 333: and r3.w, r3.w, l(0xc0490fdb)
    r3.w = (asfloat(asuint(r3.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 334: add r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)+(r3.wwww)).w;
    // 335: min r3.w, r5.x, r5.y
    r3.w = (min(r5.xxxx,r5.yyyy)).w;
    // 336: lt r3.w, r3.w, -r3.w
    r3.w = (asfloat((uint4)((r3.wwww)<(-(r3.wwww))) * 0xffffffffu)).w;
    // 337: max r5.z, r5.x, r5.y
    r5.z = (max(r5.xxxx,r5.yyyy)).z;
    // 338: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 339: add r5.x, r5.y, r5.x
    r5.x = ((r5.yyyy)+(r5.xxxx)).x;
    // 340: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 341: ge r5.y, r5.z, -r5.z
    r5.y = (asfloat((uint4)((r5.zzzz)>=(-(r5.zzzz))) * 0xffffffffu)).y;
    // 342: and r3.w, r3.w, r5.y
    r3.w = (asfloat(asuint(r3.wwww) & asuint(r5.yyyy))).w;
    // 343: movc r0.w, r3.w, -r0.w, r0.w
    r0.w = ((asuint(r3.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 344: add r3.w, r0.w, l(6.283185)
    r3.w = ((r0.wwww)+(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 345: mul r3.w, r3.w, l(0.159155)
    r3.w = ((r3.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 346: mul r5.y, r0.w, l(0.159155)
    r5.y = ((r0.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).y;
    // 347: ge r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 348: movc r0.w, r0.w, r5.y, r3.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (r5.yyyy) : (r3.wwww)).w;
    // 349: add r3.w, -r0.w, -cb0[25].w
    r3.w = ((-(r0.wwww))+(-(source[25].wwww))).w;
    // 350: add r0.w, r0.w, -cb0[25].w
    r0.w = ((r0.wwww)+(-(source[25].wwww))).w;
    // 351: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 352: add r5.y, -cb0[25].w, l(1.000000)
    r5.y = ((-(source[25].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 353: div r5.y, l(1.000000, 1.000000, 1.000000, 1.000000), r5.y
    r5.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r5.yyyy)).y;
    // 354: mul_sat r3.w, r3.w, r5.y
    r3.w = (saturate((r3.wwww)*(r5.yyyy))).w;
    // 355: mul_sat r0.w, r0.w, r5.y
    r0.w = (saturate((r0.wwww)*(r5.yyyy))).w;
    // 356: mad r5.y, r3.w, l(-2.000000), l(3.000000)
    r5.y = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 357: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 358: mul r3.w, r3.w, r5.y
    r3.w = ((r3.wwww)*(r5.yyyy)).w;
    // 359: mad r5.y, r0.w, l(-2.000000), l(3.000000)
    r5.y = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 360: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 361: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 362: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 363: log r3.w, r5.x
    r3.w = (log2(r5.xxxx)).w;
    // 364: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 365: mul r3.w, r3.w, l(5.082000)
    r3.w = ((r3.wwww)*(float4(5.082000,5.082000,5.082000,5.082000))).w;
    // 366: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 367: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 368: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 369: dp3 r3.w, r6.xyzx, r9.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 370: mul_sat r5.x, r3.w, cb0[26].x
    r5.x = (saturate((r3.wwww)*(source[26].xxxx))).x;
    // 371: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 372: mul_sat r5.y, r9.z, cb0[26].x
    r5.y = (saturate((r9.zzzz)*(source[26].xxxx))).y;
    // 373: add r5.z, -|r9.z|, l(1.000000)
    r5.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 374: mul r3.w, r3.w, r5.z
    r3.w = ((r3.wwww)*(r5.zzzz)).w;
    // 375: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 376: add_sat r5.y, r5.y, -cb0[26].y
    r5.y = (saturate((r5.yyyy)+(-(source[26].yyyy)))).y;
    // 377: log r5.z, r5.y
    r5.z = (log2(r5.yyyy)).z;
    // 378: lt r5.y, r5.y, l(0.000001)
    r5.y = (asfloat((uint4)((r5.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 379: mul r5.z, r5.z, cb0[26].z
    r5.z = ((r5.zzzz)*(source[26].zzzz)).z;
    // 380: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 381: mul r5.x, r5.z, r5.x
    r5.x = ((r5.zzzz)*(r5.xxxx)).x;
    // 382: movc r5.x, r5.y, l(0), r5.x
    r5.x = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).x;
    // 383: mul r5.y, r5.x, cb0[26].w
    r5.y = ((r5.xxxx)*(source[26].wwww)).y;
    // 384: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 385: mul r0.w, r0.w, cb0[26].w
    r0.w = ((r0.wwww)*(source[26].wwww)).w;
    // 386: log r5.y, |r0.w|
    r5.y = (log2(abs(r0.wwww))).y;
    // 387: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 388: mul r5.y, r5.y, cb0[27].x
    r5.y = ((r5.yyyy)*(source[27].xxxx)).y;
    // 389: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 390: mul r6.xyz, cb0[9].xyzx, cb0[9].wwww
    r6.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 391: mul r5.yzw, r5.yyyy, r6.xxyz
    r5.yzw = ((r5.yyyy)*(r6.xxyz)).yzw;
    // 392: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 393: dp2 r6.x, cb0[10].xyxx, r6.xyxx
    r6.x = (dot((source[10].xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 394: add r6.x, r6.x, cb0[28].z
    r6.x = ((r6.xxxx)+(source[28].zzzz)).x;
    // 395: add_sat r6.x, r6.x, l(-0.500000)
    r6.x = (saturate((r6.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).x;
    // 396: mul r5.yzw, r5.yyzw, r6.xxxx
    r5.yzw = ((r5.yyzw)*(r6.xxxx)).yzw;
    // 397: movc r5.yzw, r0.wwww, l(0,0,0,0), r5.yyzw
    r5.yzw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.yyzw)).yzw;
    // 398: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 399: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 400: add r6.x, -r0.w, l(1000.000000)
    r6.x = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 401: mad r0.w, cb0[29].w, r6.x, r0.w
    r0.w = ((source[29].wwww)*(r6.xxxx)+(r0.wwww)).w;
    // 402: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 403: mad r0.w, cb0[29].y, cb0[29].z, r0.w
    r0.w = ((source[29].yyyy)*(source[29].zzzz)+(r0.wwww)).w;
    // 404: mul r6.x, r0.w, l(3.524534)
    r6.x = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 405: sincos null, r6.x, r6.x
    r6.x = (cos(r6.xxxx)).x;
    // 406: add r0.w, r0.w, r6.x
    r0.w = ((r0.wwww)+(r6.xxxx)).w;
    // 407: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 408: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 409: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 410: mad r0.w, r0.w, l(0.500000), cb0[29].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[29].xxxx)).w;
    // 411: mul r6.xyz, cb0[11].xyzx, cb0[28].wwww
    r6.xyz = ((source[11].xyzx)*(source[28].wwww)).xyz;
    // 412: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 413: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 414: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 415: mad r5.yzw, r1.wwww, r5.yyzw, r6.xxyz
    r5.yzw = ((r1.wwww)*(r5.yyzw)+(r6.xxyz)).yzw;
    // 416: mul r6.xy, v4.xyxx, cb0[12].zzzz
    r6.xy = ((v4.xyxx)*(source[12].zzzz)).xy;
    // 417: mul r6.zw, cb0[12].xxxy, cb0[29].zzzz
    r6.zw = ((source[12].xxxy)*(source[29].zzzz)).zw;
    // 418: mad r6.xy, r6.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r6.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 419: mad r6.zw, cb0[12].zzzz, v4.xxxy, r6.zzzw
    r6.zw = ((source[12].zzzz)*(v4.xxxy)+(r6.zzzw)).zw;
    // 420: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r6.xyxx, t7.yzwx, s5, l(0.000000)
    r0.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 421: mad r6.xy, r0.wwww, cb0[30].xxxx, r6.zwzz
    r6.xy = ((r0.wwww)*(source[30].xxxx)+(r6.zwzz)).xy;
    // 422: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t7.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 423: mul r6.xyz, r6.xyzx, r7.wwww
    r6.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 424: mul r7.xyz, cb0[13].xyzx, cb0[30].yyyy
    r7.xyz = ((source[13].xyzx)*(source[30].yyyy)).xyz;
    // 425: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 426: mad r7.xyz, r5.xxxx, r6.xyzx, -r6.xyzx
    r7.xyz = ((r5.xxxx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 427: mad r6.xyz, cb0[13].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[13].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 428: add r5.yzw, r5.yyzw, r6.xxyz
    r5.yzw = ((r5.yyzw)+(r6.xxyz)).yzw;
    // 429: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 430: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 431: mad r5.yzw, cb0[30].zzzz, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].zzzz)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 432: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 433: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 434: mad r5.yzw, cb0[30].wwww, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].wwww)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 435: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 436: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 437: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 438: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 439: add r6.xyz, -r0.xyzx, r0.wwww
    r6.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 440: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 441: mul r6.xyz, cb0[18].xyzx, cb0[31].yyyy
    r6.xyz = ((source[18].xyzx)*(source[31].yyyy)).xyz;
    // 442: mul r6.xyz, r6.xyzx, cb0[32].wwww
    r6.xyz = ((r6.xyzx)*(source[32].wwww)).xyz;
    // 443: mul r6.xyz, r5.xxxx, r6.xyzx
    r6.xyz = ((r5.xxxx)*(r6.xyzx)).xyz;
    // 444: mad r7.xyz, r5.xxxx, cb0[17].xyzx, -cb0[17].xyzx
    r7.xyz = ((r5.xxxx)*(source[17].xyzx)+(-(source[17].xyzx))).xyz;
    // 445: add r0.w, r5.x, l(-1.000000)
    r0.w = ((r5.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 446: mad r0.w, cb0[16].w, r0.w, l(1.000000)
    r0.w = ((source[16].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 447: mad r7.xyz, cb0[17].wwww, r7.xyzx, cb0[17].xyzx
    r7.xyz = ((source[17].wwww)*(r7.xyzx)+(source[17].xyzx)).xyz;
    // 448: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 449: mad r0.xyz, r0.wwww, cb0[16].xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 450: mad r0.xyz, r5.yzwy, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.yzwy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 451: log r0.w, |r3.w|
    r0.w = (log2(abs(r3.wwww))).w;
    // 452: lt r1.w, |r3.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 453: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 454: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 455: mul r3.xyz, r0.wwww, cb0[19].xyzx
    r3.xyz = ((r0.wwww)*(source[19].xyzx)).xyz;
    // 456: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 457: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 458: mad r0.xyz, cb0[23].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 459: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 460: mul r1.xyz, r8.wwww, cb0[48].xyzx
    r1.xyz = ((r8.wwww)*(source[48].xyzx)).xyz;
    // 461: mad r1.xyz, r8.zzzz, cb0[47].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[47].xyzx)+(r1.xyzx)).xyz;
    // 462: mul r1.xyz, r1.xyzx, cb0[49].wwww
    r1.xyz = ((r1.xyzx)*(source[49].wwww)).xyz;
    // 463: mul_sat r3.xyz, cb0[22].xyzx, cb0[22].wwww
    r3.xyz = (saturate((source[22].xyzx)*(source[22].wwww))).xyz;
    // 464: mul r5.xyz, r2.wwww, r3.xyzx
    r5.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 465: mul r3.xyz, r3.xyzx, cb0[36].wwww
    r3.xyz = ((r3.xyzx)*(source[36].wwww)).xyz;
    // 466: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 467: mul r3.xyz, r4.zzzz, r5.xyzx
    r3.xyz = ((r4.zzzz)*(r5.xyzx)).xyz;
    // 468: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 469: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 470: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 471: add r0.xyz, r4.xywx, r0.xyzx
    r0.xyz = ((r4.xywx)+(r0.xyzx)).xyz;
    // 472: dp3 o4.y, r4.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 473: mad o0.xyz, r2.xyzx, cb0[49].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[49].xyzx)+(r0.xyzx)).xyz;
    // 474: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 475: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 476: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 477: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 478: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 479: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 480: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 481: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 482: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 483: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 484: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 485: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 486: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 487: ftou r0.x, cb0[46].z
    r0.x = (asfloat((uint4)(source[46].zzzz))).x;
    // 488: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 489: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 490: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 491: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 492: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drr-00-body-mi.v1 / source program fe38b50e0f8656448805e1fc88f67c83
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase106(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[25].x=(g_SourceCharacterTime.xxxx).x;
    source[27].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[30] = g_SourceCharacterEnvironmentColor;
        source[31] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0, r22=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[28].zzzz
    r0.xy = ((v4.xyxx)*(source[28].zzzz)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s6, l(0.000000)
    r0.x = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[28].w
    r0.x = ((r0.xxxx)+(-(source[28].wwww))).x;
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
    // 11: add r0.x, -cb0[4].w, l(1.000000)
    r0.x = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r0.x, r0.x, cb0[25].x
    r0.x = ((r0.xxxx)*(source[25].xxxx)).x;
    // 13: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 14: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 15: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 16: mul r0.y, cb0[4].z, l(1.500000)
    r0.y = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 17: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 18: mad r0.x, r0.x, l(0.500000), cb0[4].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).x;
    // 19: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 20: mul r2.x, r0.y, l(0.125000)
    r2.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 21: mul r3.y, cb0[4].y, cb0[17].y
    r3.y = ((source[4].yyyy)*(source[17].yyyy)).y;
    // 22: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 23: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 24: add r0.yz, r2.xxyx, r3.xxyx
    r0.yz = ((r2.xxyx)+(r3.xxyx)).yz;
    // 25: frc r0.w, cb0[4].x
    r0.w = (frac(source[4].xxxx)).w;
    // 26: add r1.w, -r0.w, cb0[4].x
    r1.w = ((-(r0.wwww))+(source[4].xxxx)).w;
    // 27: mul r3.z, r1.w, l(0.125000)
    r3.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 28: add r0.yz, r0.yyzy, r3.zzwz
    r0.yz = ((r0.yyzy)+(r3.zzwz)).yz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t4.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 31: mul r1.w, r0.w, r2.w
    r1.w = ((r0.wwww)*(r2.wwww)).w;
    // 32: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 34: add r2.xyz, -r1.xyzx, r2.xxxx
    r2.xyz = ((-(r1.xyzx))+(r2.xxxx)).xyz;
    // 35: mad r2.xyz, cb0[25].wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((source[25].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 36: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 38: mad r2.xyz, cb0[26].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[26].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 39: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 40: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 41: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 42: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 43: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 44: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 46: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 47: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 48: mul r2.w, r6.y, cb0[19].y
    r2.w = ((r6.yyyy)*(source[19].yyyy)).w;
    // 49: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 50: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: movc r2.w, r5.y, l(0), r2.w
    r2.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 52: mad r3.xyz, r2.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 53: mul_sat r4.w, r2.w, cb2[3].w
    r4.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 54: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 55: add r7.xyz, -r3.xyzx, r2.wwww
    r7.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 56: mad r7.xyz, cb0[25].wwww, r7.xyzx, r3.xyzx
    r7.xyz = ((source[25].wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 57: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 58: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: add r3.xyz, -r7.xyzx, r2.wwww
    r3.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 60: mad r3.xyz, cb0[26].xxxx, r3.xyzx, r7.xyzx
    r3.xyz = ((source[26].xxxx)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 61: mad r7.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mad r8.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 63: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 64: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 65: mul r8.xyz, r2.xyzx, r3.xyzx
    r8.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 66: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 67: mad r9.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r9.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 68: mad r2.xyz, r3.xyzx, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = ((r3.xyzx)*(r2.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 69: mad r3.xyz, cb0[25].wwww, r9.xyzx, r8.xyzx
    r3.xyz = ((source[25].wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 70: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: add r8.xyz, -r3.xyzx, r2.wwww
    r8.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 72: mad r3.xyz, cb0[26].xxxx, r8.xyzx, r3.xyzx
    r3.xyz = ((source[26].xxxx)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 73: mul r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 74: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 75: mad r0.xyz, r1.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 76: mul r1.w, r6.x, cb0[27].y
    r1.w = ((r6.xxxx)*(source[27].yyyy)).w;
    // 77: mul r2.w, r6.z, cb0[29].x
    r2.w = ((r6.zzzz)*(source[29].xxxx)).w;
    // 78: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 79: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: movc r2.w, r5.z, l(0), r2.w
    r2.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 81: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 82: min r4.z, r2.w, l(1.000000)
    r4.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 83: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 84: movc r1.w, r5.x, l(0), r1.w
    r1.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 85: add_sat r1.w, r1.w, cb0[27].z
    r1.w = (saturate((r1.wwww)+(source[27].zzzz))).w;
    // 86: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 87: mul r3.xyz, r2.wwww, cb0[16].xyzx
    r3.xyz = ((r2.wwww)*(source[16].xyzx)).xyz;
    // 88: mul r5.xyz, r0.xyzx, r3.xyzx
    r5.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 89: mad r0.xyz, -r3.xyzx, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r3.xyzx))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 90: mad r0.xyz, r1.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 91: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 93: mad_sat r3.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 94: mad r0.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r0.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 95: mad r5.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 96: mad r0.xyz, r1.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 97: mad r5.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r5.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 98: mad r0.xyz, r0.xyzx, r1.wwww, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r1.wwww)+(r5.xyzx)).xyz;
    // 99: mul r0.xyz, r1.wwww, r0.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)).xyz;
    // 100: max r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 101: mov_sat r3.w, cb0[28].y
    r3.w = (saturate(source[28].yyyy)).w;
    // 102: mad r5.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r5.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 103: mul r2.w, r3.w, l(0.080000)
    r2.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 104: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 105: mad r5.xyz, r4.wwww, r5.xyzx, r2.wwww
    r5.xyz = ((r4.wwww)*(r5.xyzx)+(r2.wwww)).xyz;
    // 106: mul_sat r2.w, r5.y, l(50.000000)
    r2.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 107: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 108: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 109: dp2 r3.w, r4.xyxx, r4.xyxx
    r3.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 110: mul r6.xy, r4.xyxx, cb0[19].xxxx
    r6.xy = ((r4.xyxx)*(source[19].xxxx)).xy;
    // 111: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 113: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 114: add r6.z, r3.w, l(0.000010)
    r6.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 115: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 116: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 117: div r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)/(r3.wwww)).xyz;
    // 118: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 119: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 120: mul r8.xyz, r3.wwww, r6.xyzx
    r8.xyz = ((r3.wwww)*(r6.xyzx)).xyz;
    // 121: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 122: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 123: mul r9.xyz, r3.wwww, v5.xyzx
    r9.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 124: dp3 r3.w, r8.xyzx, r9.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 125: deriv_rtx_coarse r4.x, r3.w
    r4.x = (ddx_coarse(r3.wwww)).x;
    // 126: deriv_rty_coarse r4.y, r3.w
    r4.y = (ddy_coarse(r3.wwww)).y;
    // 127: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 128: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 129: mad r4.x, r4.x, l(0.300000), r4.z
    r4.x = ((r4.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz)).x;
    // 130: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 131: min r4.y, r4.x, l(1.000000)
    r4.y = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 132: add r4.z, -r4.y, l(1.000000)
    r4.z = ((-(r4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 133: max r10.xyz, r5.xyzx, r4.zzzz
    r10.xyz = (max(r5.xyzx,r4.zzzz)).xyz;
    // 134: add r10.xyz, -r5.xyzx, r10.xyzx
    r10.xyz = ((-(r5.xyzx))+(r10.xyzx)).xyz;
    // 135: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 136: mul r11.xyz, r3.wwww, r8.xyzx
    r11.xyz = ((r3.wwww)*(r8.xyzx)).xyz;
    // 137: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 138: add r2.w, r11.z, l(1.000000)
    r2.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: add r4.z, r3.w, l(1.000000)
    r4.z = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 141: mov_sat r3.w, r3.w
    r3.w = (saturate(r3.wwww)).w;
    // 142: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 143: mul r3.w, r3.w, cb0[1].y
    r3.w = ((r3.wwww)*(source[1].yyyy)).w;
    // 144: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 145: mad_sat r3.w, r3.w, cb0[1].w, cb0[1].z
    r3.w = (saturate((r3.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 146: mul r3.w, r3.w, cb0[29].y
    r3.w = ((r3.wwww)*(source[29].yyyy)).w;
    // 147: add_sat r4.x, -r2.w, r4.z
    r4.x = (saturate((-(r2.wwww))+(r4.zzzz))).x;
    // 148: sample_indexable(texture2d)(float,float,float,float) r12.xy, r4.xyxx, t7.xyzw, s8
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 149: add r2.w, r1.w, r4.x
    r2.w = ((r1.wwww)+(r4.xxxx)).w;
    // 150: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 151: mul r13.xyz, r5.xyzx, r12.yyyy
    r13.xyz = ((r5.xyzx)*(r12.yyyy)).xyz;
    // 152: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 153: div r4.x, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.yyyy)).x;
    // 154: add r4.x, r4.x, l(-1.000000)
    r4.x = ((r4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 155: mad r12.xyz, r5.xyzx, r4.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xyzx)*(r4.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: dp3 r4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 157: mad r5.xyz, r4.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r4.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 158: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 160: mul r12.xyz, r3.xyzx, r13.xyzx
    r12.xyz = ((r3.xyzx)*(r13.xyzx)).xyz;
    // 161: add r4.x, -r4.w, l(1.000000)
    r4.x = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 162: mul r12.xyz, r4.xxxx, r12.xyzx
    r12.xyz = ((r4.xxxx)*(r12.xyzx)).xyz;
    // 163: dp3 r4.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 164: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 165: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 166: mul r14.xyz, r5.wwww, v1.xyzx
    r14.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 167: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 168: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 169: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 170: mul r16.xyz, r5.wwww, v0.xyzx
    r16.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 171: mul r17.xyz, r14.zxyz, r16.yzxy
    r17.xyz = ((r14.zxyz)*(r16.yzxy)).xyz;
    // 172: mad r17.xyz, r14.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r14.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 173: dp3 r14.y, r14.xyzx, r11.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 174: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 175: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 176: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 177: dp2 r15.z, r18.xyxx, cb0[31].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[31].xyxx).xy).xxxx).z;
    // 178: mul r19.xy, cb0[31].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r19.xy = ((source[31].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 179: dp2 r15.x, r18.xyxx, r19.xyxx
    r15.x = (dot((r18.xyxx).xy,(r19.xyxx).xy).xxxx).x;
    // 180: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 181: dp4 r20.x, cb0[32].xyzw, r15.xyzw
    r20.x = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 182: dp4 r20.y, cb0[33].xyzw, r15.xyzw
    r20.y = (dot((source[33].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 183: dp4 r20.z, cb0[34].xyzw, r15.xyzw
    r20.z = (dot((source[34].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 184: mul r21.xyzw, r15.yzzx, r15.xyzz
    r21.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 185: dp4 r22.x, cb0[35].xyzw, r21.xyzw
    r22.x = (dot((source[35].xyzw).xyzw,(r21.xyzw).xyzw).xxxx).x;
    // 186: dp4 r22.y, cb0[36].xyzw, r21.xyzw
    r22.y = (dot((source[36].xyzw).xyzw,(r21.xyzw).xyzw).xxxx).y;
    // 187: dp4 r22.z, cb0[37].xyzw, r21.xyzw
    r22.z = (dot((source[37].xyzw).xyzw,(r21.xyzw).xyzw).xxxx).z;
    // 188: add r20.xyz, r20.xyzx, r22.xyzx
    r20.xyz = ((r20.xyzx)+(r22.xyzx)).xyz;
    // 189: mul r5.w, r15.y, r15.y
    r5.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 190: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 191: mad r5.w, r15.x, r15.x, -r5.w
    r5.w = ((r15.xxxx)*(r15.xxxx)+(-(r5.wwww))).w;
    // 192: mad r15.xyz, cb0[38].xyzx, r5.wwww, r20.xyzx
    r15.xyz = ((source[38].xyzx)*(r5.wwww)+(r20.xyzx)).xyz;
    // 193: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 194: mul r15.xyz, r15.xyzx, cb0[30].xyzx
    r15.xyz = ((r15.xyzx)*(source[30].xyzx)).xyz;
    // 195: mul r15.xyz, r15.xyzx, cb0[31].zzzz
    r15.xyz = ((r15.xyzx)*(source[31].zzzz)).xyz;
    // 196: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[30].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[30].wwww)).xyz;
    // 197: dp3 r5.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: add r15.xyz, -r5.wwww, r15.xyzx
    r15.xyz = ((-(r5.wwww))+(r15.xyzx)).xyz;
    // 199: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 200: dp3 r5.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 201: mad r6.w, r4.y, l(2.000000), l(2.000000)
    r6.w = ((r4.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 202: div r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)/(r6.wwww)).w;
    // 203: mad r5.w, r4.z, l(5.000000), r5.w
    r5.w = ((r4.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 204: add_sat r5.w, r4.w, r5.w
    r5.w = (saturate((r4.wwww)+(r5.wwww))).w;
    // 205: mad r7.w, r5.w, l(-2.000000), l(3.000000)
    r7.w = ((r5.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 206: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 207: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 208: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 209: mul r5.w, r5.w, l(1.500000)
    r5.w = ((r5.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 210: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 211: mul r15.xyz, r5.wwww, r15.xyzx
    r15.xyz = ((r5.wwww)*(r15.xyzx)).xyz;
    // 212: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 213: mul r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r15.xyzx)).xyz;
    // 214: mul r12.xyz, r0.xyzx, r12.xyzx
    r12.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 215: mul r5.w, r4.y, l(5.000000)
    r5.w = ((r4.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 216: mul r4.y, r4.y, r4.y
    r4.y = ((r4.yyyy)*(r4.yyyy)).y;
    // 217: mul r2.w, r2.w, r4.y
    r2.w = ((r2.wwww)*(r4.yyyy)).w;
    // 218: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 219: add r2.w, r1.w, r2.w
    r2.w = ((r1.wwww)+(r2.wwww)).w;
    // 220: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 221: add_sat r1.w, r2.w, l(-1.000000)
    r1.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 222: dp3 r15.x, r16.xyzx, r11.xyzx
    r15.x = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 223: dp3 r15.y, r17.xyzx, r11.xyzx
    r15.y = (dot((r17.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 224: dp2 r14.x, r15.xyxx, r19.xyxx
    r14.x = (dot((r15.xyxx).xy,(r19.xyxx).xy).xxxx).x;
    // 225: dp2 r14.z, r15.xyxx, cb0[31].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[31].xyxx).xy).xxxx).z;
    // 226: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t8.xyzw, s7, r5.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r5.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 227: mul r14.xyz, r14.xyzx, r14.wwww
    r14.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 228: mul r14.xyz, r14.xyzx, cb0[30].xyzx
    r14.xyz = ((r14.xyzx)*(source[30].xyzx)).xyz;
    // 229: mul r14.xyz, r14.xyzx, cb0[31].zzzz
    r14.xyz = ((r14.xyzx)*(source[31].zzzz)).xyz;
    // 230: mad r14.xyz, r14.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[30].wwww
    r14.xyz = ((r14.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[30].wwww)).xyz;
    // 231: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 232: add r14.xyz, -r2.wwww, r14.xyzx
    r14.xyz = ((-(r2.wwww))+(r14.xyzx)).xyz;
    // 233: mad r14.xyz, r14.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r14.xyz = ((r14.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 234: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 235: div r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)/(r6.wwww)).w;
    // 236: mad r2.w, r4.z, l(5.000000), r2.w
    r2.w = ((r4.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 237: add_sat r2.w, r4.w, r2.w
    r2.w = (saturate((r4.wwww)+(r2.wwww))).w;
    // 238: mad r4.y, r2.w, l(-2.000000), l(3.000000)
    r4.y = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 239: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 240: mul r2.w, r2.w, r4.y
    r2.w = ((r2.wwww)*(r4.yyyy)).w;
    // 241: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 242: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 243: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 244: mul r14.xyz, r2.wwww, r14.xyzx
    r14.xyz = ((r2.wwww)*(r14.xyzx)).xyz;
    // 245: mul r15.xyz, r10.xyzx, r14.xyzx
    r15.xyz = ((r10.xyzx)*(r14.xyzx)).xyz;
    // 246: mad r2.w, r1.w, r5.x, r5.y
    r2.w = ((r1.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 247: mad r2.w, r2.w, r1.w, r5.z
    r2.w = ((r2.wwww)*(r1.wwww)+(r5.zzzz)).w;
    // 248: mul r2.w, r1.w, r2.w
    r2.w = ((r1.wwww)*(r2.wwww)).w;
    // 249: max r1.w, r1.w, r2.w
    r1.w = (max(r1.wwww,r2.wwww)).w;
    // 250: mad r5.xyz, r15.xyzx, r1.wwww, r12.xyzx
    r5.xyz = ((r15.xyzx)*(r1.wwww)+(r12.xyzx)).xyz;
    // 251: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 252: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 253: mul r12.xyz, r2.wwww, v6.xyzx
    r12.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 254: dp3 r2.w, r12.xyzx, r8.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 255: dp3 r4.y, -r12.xyzx, r8.xyzx
    r4.y = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).y;
    // 256: dp3 r4.z, r12.xyzx, r11.xyzx
    r4.z = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).z;
    // 257: mad r8.xy, r4.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r4.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 258: mad r4.yz, r4.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r4.yz = ((r4.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 259: mul r4.yz, r4.yyzy, r4.yyzy
    r4.yz = ((r4.yyzy)*(r4.yyzy)).yz;
    // 260: mad r8.zw, r2.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r2.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 261: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 262: mul r11.xyz, r8.wwww, cb0[41].xyzx
    r11.xyz = ((r8.wwww)*(source[41].xyzx)).xyz;
    // 263: mad r11.xyz, r8.zzzz, cb0[40].xyzx, r11.xyzx
    r11.xyz = ((r8.zzzz)*(source[40].xyzx)+(r11.xyzx)).xyz;
    // 264: mul r11.xyz, r11.xyzx, cb0[42].wwww
    r11.xyz = ((r11.xyzx)*(source[42].wwww)).xyz;
    // 265: mul r11.xyz, r3.xyzx, r11.xyzx
    r11.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 266: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 267: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 268: mul r0.xyz, r13.xyzx, r0.xyzx
    r0.xyz = ((r13.xyzx)*(r0.xyzx)).xyz;
    // 269: mad r0.xyz, -r0.xyzx, r4.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r4.wwww)+(r0.xyzx)).xyz;
    // 270: mad r0.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 271: mul r5.xyz, r8.yyyy, cb0[41].xyzx
    r5.xyz = ((r8.yyyy)*(source[41].xyzx)).xyz;
    // 272: mad r5.xyz, cb0[40].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[40].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 273: mul r5.xyz, r5.xyzx, cb0[42].wwww
    r5.xyz = ((r5.xyzx)*(source[42].wwww)).xyz;
    // 274: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 275: mul r5.xyz, r14.xyzx, r5.xyzx
    r5.xyz = ((r14.xyzx)*(r5.xyzx)).xyz;
    // 276: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 277: mad r0.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 278: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 279: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 280: mul r1.w, r17.z, cb0[20].w
    r1.w = ((r17.zzzz)*(source[20].wwww)).w;
    // 281: mul r2.w, r17.z, cb0[21].x
    r2.w = ((r17.zzzz)*(source[21].xxxx)).w;
    // 282: mad r5.x, r16.z, cb0[20].w, -r2.w
    r5.x = ((r16.zzzz)*(source[20].wwww)+(-(r2.wwww))).x;
    // 283: mad r5.y, r16.z, cb0[21].x, r1.w
    r5.y = ((r16.zzzz)*(source[21].xxxx)+(r1.wwww)).y;
    // 284: max r1.w, |r5.x|, |r5.y|
    r1.w = (max(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 285: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 286: min r2.w, |r5.x|, |r5.y|
    r2.w = (min(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 287: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 288: mul r2.w, r1.w, r1.w
    r2.w = ((r1.wwww)*(r1.wwww)).w;
    // 289: mad r4.w, r2.w, l(0.020835), l(-0.085133)
    r4.w = ((r2.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).w;
    // 290: mad r4.w, r2.w, r4.w, l(0.180141)
    r4.w = ((r2.wwww)*(r4.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 291: mad r4.w, r2.w, r4.w, l(-0.330299)
    r4.w = ((r2.wwww)*(r4.wwww)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).w;
    // 292: mad r2.w, r2.w, r4.w, l(0.999866)
    r2.w = ((r2.wwww)*(r4.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 293: mul r4.w, r1.w, r2.w
    r4.w = ((r1.wwww)*(r2.wwww)).w;
    // 294: mad r4.w, r4.w, l(-2.000000), l(1.570796)
    r4.w = ((r4.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).w;
    // 295: lt r5.z, |r5.x|, |r5.y|
    r5.z = (asfloat((uint4)((abs(r5.xxxx))<(abs(r5.yyyy))) * 0xffffffffu)).z;
    // 296: and r4.w, r4.w, r5.z
    r4.w = (asfloat(asuint(r4.wwww) & asuint(r5.zzzz))).w;
    // 297: mad r1.w, r1.w, r2.w, r4.w
    r1.w = ((r1.wwww)*(r2.wwww)+(r4.wwww)).w;
    // 298: lt r2.w, r5.x, -r5.x
    r2.w = (asfloat((uint4)((r5.xxxx)<(-(r5.xxxx))) * 0xffffffffu)).w;
    // 299: and r2.w, r2.w, l(0xc0490fdb)
    r2.w = (asfloat(asuint(r2.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 300: add r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)+(r2.wwww)).w;
    // 301: min r2.w, r5.x, r5.y
    r2.w = (min(r5.xxxx,r5.yyyy)).w;
    // 302: lt r2.w, r2.w, -r2.w
    r2.w = (asfloat((uint4)((r2.wwww)<(-(r2.wwww))) * 0xffffffffu)).w;
    // 303: max r4.w, r5.x, r5.y
    r4.w = (max(r5.xxxx,r5.yyyy)).w;
    // 304: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 305: add r5.x, r5.y, r5.x
    r5.x = ((r5.yyyy)+(r5.xxxx)).x;
    // 306: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 307: ge r4.w, r4.w, -r4.w
    r4.w = (asfloat((uint4)((r4.wwww)>=(-(r4.wwww))) * 0xffffffffu)).w;
    // 308: and r2.w, r2.w, r4.w
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r4.wwww))).w;
    // 309: movc r1.w, r2.w, -r1.w, r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (-(r1.wwww)) : (r1.wwww)).w;
    // 310: add r2.w, r1.w, l(6.283185)
    r2.w = ((r1.wwww)+(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 311: mul r2.w, r2.w, l(0.159155)
    r2.w = ((r2.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 312: mul r4.w, r1.w, l(0.159155)
    r4.w = ((r1.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 313: ge r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 314: movc r1.w, r1.w, r4.w, r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r4.wwww) : (r2.wwww)).w;
    // 315: add r2.w, -r1.w, -cb0[21].w
    r2.w = ((-(r1.wwww))+(-(source[21].wwww))).w;
    // 316: add r1.w, r1.w, -cb0[21].w
    r1.w = ((r1.wwww)+(-(source[21].wwww))).w;
    // 317: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 318: add r4.w, -cb0[21].w, l(1.000000)
    r4.w = ((-(source[21].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 319: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r4.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r4.wwww)).w;
    // 320: mul_sat r2.w, r2.w, r4.w
    r2.w = (saturate((r2.wwww)*(r4.wwww))).w;
    // 321: mul_sat r1.w, r1.w, r4.w
    r1.w = (saturate((r1.wwww)*(r4.wwww))).w;
    // 322: mad r4.w, r2.w, l(-2.000000), l(3.000000)
    r4.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 323: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 324: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 325: mad r4.w, r1.w, l(-2.000000), l(3.000000)
    r4.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 326: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 327: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 328: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 329: log r2.w, r5.x
    r2.w = (log2(r5.xxxx)).w;
    // 330: lt r4.w, r5.x, l(0.000001)
    r4.w = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 331: mul r2.w, r2.w, l(5.082000)
    r2.w = ((r2.wwww)*(float4(5.082000,5.082000,5.082000,5.082000))).w;
    // 332: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 333: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 334: movc r1.w, r4.w, l(0), r1.w
    r1.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 335: dp3 r2.w, r6.xyzx, r9.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 336: mul_sat r4.w, r2.w, cb0[22].x
    r4.w = (saturate((r2.wwww)*(source[22].xxxx))).w;
    // 337: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 338: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 339: mul_sat r5.x, r9.z, cb0[22].x
    r5.x = (saturate((r9.zzzz)*(source[22].xxxx))).x;
    // 340: add r5.y, -|r9.z|, l(1.000000)
    r5.y = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 341: mul r2.w, r2.w, r5.y
    r2.w = ((r2.wwww)*(r5.yyyy)).w;
    // 342: add r5.x, -r5.x, l(1.000000)
    r5.x = ((-(r5.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 343: add_sat r5.x, r5.x, -cb0[22].y
    r5.x = (saturate((r5.xxxx)+(-(source[22].yyyy)))).x;
    // 344: log r5.y, r5.x
    r5.y = (log2(r5.xxxx)).y;
    // 345: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 346: mul r5.y, r5.y, cb0[22].z
    r5.y = ((r5.yyyy)*(source[22].zzzz)).y;
    // 347: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 348: mul r4.w, r4.w, r5.y
    r4.w = ((r4.wwww)*(r5.yyyy)).w;
    // 349: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 350: mul r5.x, r4.w, cb0[22].w
    r5.x = ((r4.wwww)*(source[22].wwww)).x;
    // 351: mul r1.w, r1.w, r5.x
    r1.w = ((r1.wwww)*(r5.xxxx)).w;
    // 352: mul r1.w, r1.w, cb0[22].w
    r1.w = ((r1.wwww)*(source[22].wwww)).w;
    // 353: log r5.x, |r1.w|
    r5.x = (log2(abs(r1.wwww))).x;
    // 354: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 355: mul r5.x, r5.x, cb0[23].x
    r5.x = ((r5.xxxx)*(source[23].xxxx)).x;
    // 356: exp r5.x, r5.x
    r5.x = (exp2(r5.xxxx)).x;
    // 357: mul r5.yzw, cb0[5].xxyz, cb0[5].wwww
    r5.yzw = ((source[5].xxyz)*(source[5].wwww)).yzw;
    // 358: mul r5.xyz, r5.yzwy, r5.xxxx
    r5.xyz = ((r5.yzwy)*(r5.xxxx)).xyz;
    // 359: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 360: dp2 r5.w, cb0[6].xyxx, r6.xyxx
    r5.w = (dot((source[6].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 361: add r5.w, r5.w, cb0[24].z
    r5.w = ((r5.wwww)+(source[24].zzzz)).w;
    // 362: add_sat r5.w, r5.w, l(-0.500000)
    r5.w = (saturate((r5.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).w;
    // 363: mul r5.xyz, r5.xyzx, r5.wwww
    r5.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 364: movc r5.xyz, r1.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 365: mul r6.xyz, cb0[7].xyzx, cb0[24].wwww
    r6.xyz = ((source[7].xyzx)*(source[24].wwww)).xyz;
    // 366: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t5.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 367: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 368: mad r5.xyz, r0.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 369: mul r6.xy, v4.xyxx, cb0[8].zzzz
    r6.xy = ((v4.xyxx)*(source[8].zzzz)).xy;
    // 370: mul r6.zw, cb0[8].xxxy, cb0[25].xxxx
    r6.zw = ((source[8].xxxy)*(source[25].xxxx)).zw;
    // 371: mad r6.xy, r6.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r6.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 372: mad r6.zw, cb0[8].zzzz, v4.xxxy, r6.zzzw
    r6.zw = ((source[8].zzzz)*(v4.xxxy)+(r6.zzzw)).zw;
    // 373: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r6.xyxx, t6.yzwx, s4, l(0.000000)
    r0.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 374: mad r6.xy, r0.wwww, cb0[25].yyyy, r6.zwzz
    r6.xy = ((r0.wwww)*(source[25].yyyy)+(r6.zwzz)).xy;
    // 375: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t6.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 376: mul r6.xyz, r6.xyzx, r8.wwww
    r6.xyz = ((r6.xyzx)*(r8.wwww)).xyz;
    // 377: mul r8.xyz, cb0[9].xyzx, cb0[25].zzzz
    r8.xyz = ((source[9].xyzx)*(source[25].zzzz)).xyz;
    // 378: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 379: mad r8.xyz, r4.wwww, r6.xyzx, -r6.xyzx
    r8.xyz = ((r4.wwww)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 380: mad r6.xyz, cb0[9].wwww, r8.xyzx, r6.xyzx
    r6.xyz = ((source[9].wwww)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 381: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 382: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 383: add r6.xyz, -r5.xyzx, r0.wwww
    r6.xyz = ((-(r5.xyzx))+(r0.wwww)).xyz;
    // 384: mad r5.xyz, cb0[25].wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((source[25].wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 385: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 386: add r6.xyz, -r5.xyzx, r0.wwww
    r6.xyz = ((-(r5.xyzx))+(r0.wwww)).xyz;
    // 387: mad r5.xyz, cb0[26].xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((source[26].xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 388: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 389: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 390: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 391: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 392: add r6.xyz, -r2.xyzx, r0.wwww
    r6.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 393: add r2.xyz, r2.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)+(-(r6.xyzx))).xyz;
    // 394: mul r6.xyz, cb0[14].xyzx, cb0[26].zzzz
    r6.xyz = ((source[14].xyzx)*(source[26].zzzz)).xyz;
    // 395: mul r6.xyz, r6.xyzx, cb0[27].xxxx
    r6.xyz = ((r6.xyzx)*(source[27].xxxx)).xyz;
    // 396: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 397: mad r8.xyz, r4.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r8.xyz = ((r4.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 398: add r0.w, r4.w, l(-1.000000)
    r0.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 399: mad r0.w, cb0[12].w, r0.w, l(1.000000)
    r0.w = ((source[12].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 400: mad r8.xyz, cb0[13].wwww, r8.xyzx, cb0[13].xyzx
    r8.xyz = ((source[13].wwww)*(r8.xyzx)+(source[13].xyzx)).xyz;
    // 401: mad r2.xyz, r2.xyzx, r6.xyzx, r8.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 402: mad r2.xyz, r0.wwww, cb0[12].xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(source[12].xyzx)+(r2.xyzx)).xyz;
    // 403: mad r2.xyz, r5.xyzx, r7.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 404: log r0.w, |r2.w|
    r0.w = (log2(abs(r2.wwww))).w;
    // 405: lt r1.w, |r2.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 406: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 407: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 408: mul r5.xyz, r0.wwww, cb0[15].xyzx
    r5.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 409: movc r5.xyz, r1.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 410: add r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 411: mad r1.xyz, cb0[19].zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((source[19].zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 412: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 413: mul r2.xyz, r4.zzzz, cb0[41].xyzx
    r2.xyz = ((r4.zzzz)*(source[41].xyzx)).xyz;
    // 414: mad r2.xyz, r4.yyyy, cb0[40].xyzx, r2.xyzx
    r2.xyz = ((r4.yyyy)*(source[40].xyzx)+(r2.xyzx)).xyz;
    // 415: mul r2.xyz, r2.xyzx, cb0[42].wwww
    r2.xyz = ((r2.xyzx)*(source[42].wwww)).xyz;
    // 416: mul_sat r4.yzw, cb0[18].xxyz, cb0[18].wwww
    r4.yzw = (saturate((source[18].xxyz)*(source[18].wwww))).yzw;
    // 417: mul r5.xyz, r3.wwww, r4.yzwy
    r5.xyz = ((r3.wwww)*(r4.yzwy)).xyz;
    // 418: mul r4.yzw, r4.yyzw, cb0[29].yyyy
    r4.yzw = ((r4.yyzw)*(source[29].yyyy)).yzw;
    // 419: dp3_sat o5.x, r4.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r4.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 420: mul r4.xyz, r4.xxxx, r5.xyzx
    r4.xyz = ((r4.xxxx)*(r5.xyzx)).xyz;
    // 421: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 422: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 423: mad r1.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 424: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 425: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 426: mad o0.xyz, r3.xyzx, cb0[42].xyzx, r1.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[42].xyzx)+(r1.xyzx)).xyz;
    // 427: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 428: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 429: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 430: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 431: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 432: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 433: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 434: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 435: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 436: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 437: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 438: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 439: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 440: ftou r0.x, cb0[39].z
    r0.x = (asfloat((uint4)(source[39].zzzz))).x;
    // 441: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 442: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 443: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 444: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 445: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drr-00-head-mi.v1 / source program 14c753dc7e67da47b5fd1f7628119996
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase107(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[21]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[29].z=(g_SourceCharacterTime.xxxx).x;
    source[31].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[32].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[32].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[32].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[32].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[36] = g_SourceCharacterEnvironmentColor;
        source[37] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[34].yyyy
    r0.xy = ((v4.xyxx)*(source[34].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[34].z
    r0.x = ((r0.xxxx)+(-(source[34].zzzz))).x;
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
    // 11: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 12: add r0.xyz, -r1.xyzx, r0.xxxx
    r0.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 13: mad r0.xyz, cb0[30].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[30].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[30].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[30].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 17: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 18: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 19: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 20: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 21: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 22: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 24: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 25: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 26: mul r0.w, r5.y, cb0[23].y
    r0.w = ((r5.yyyy)*(source[23].yyyy)).w;
    // 27: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 28: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: movc r0.w, r4.y, l(0), r0.w
    r0.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 30: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 31: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 32: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 33: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 34: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 35: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 36: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 37: mad r3.xyz, r0.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 38: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mad r2.xyz, r6.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r6.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 41: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 42: max r7.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 43: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 47: mad r3.xyz, r0.wwww, r7.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 48: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 49: mad r2.xyz, r6.yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 50: mul r3.xyz, cb0[7].xyzx, cb0[7].wwww
    r3.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 51: max r6.xyw, r3.xyxz, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r3.xyxz,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 52: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 53: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 54: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 55: add r6.xyw, -r3.xyxz, r6.xyxw
    r6.xyw = ((-(r3.xyxz))+(r6.xyxw)).xyw;
    // 56: mad r3.xyz, r0.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 57: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 58: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 59: mad r2.xyz, r6.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r6.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 60: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 62: mad r3.xyz, cb0[30].zzzz, r3.xyzx, r2.xyzx
    r3.xyz = ((source[30].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[30].wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((source[30].wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 67: mad r3.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r6.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 70: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 71: mul r6.xyz, r0.xyzx, r2.xyzx
    r6.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 72: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r8.xyz, -r2.xyzx, r0.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))*(r0.xyzx)+(r0.wwww)).xyz;
    // 74: mad r0.xyz, r2.xyzx, r0.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 75: mad r2.xyz, cb0[30].zzzz, r8.xyzx, r6.xyzx
    r2.xyz = ((source[30].zzzz)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r6.xyz, -r2.xyzx, r0.wwww
    r6.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[30].wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((source[30].wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[8].w, l(1.000000)
    r0.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[29].z
    r0.w = ((r0.wwww)*(source[29].zzzz)).w;
    // 82: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 83: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 84: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r1.w, cb0[8].z, l(1.500000)
    r1.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 87: mad r0.w, r0.w, l(0.500000), cb0[8].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 88: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 89: mul r6.x, r1.w, l(0.125000)
    r6.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 90: mul r8.y, cb0[8].y, cb0[21].y
    r8.y = ((source[8].yyyy)*(source[21].yyyy)).y;
    // 91: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 92: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 93: add r4.yw, r6.xxxy, r8.xxxy
    r4.yw = ((r6.xxxy)+(r8.xxxy)).yw;
    // 94: frc r1.w, cb0[8].x
    r1.w = (frac(source[8].xxxx)).w;
    // 95: add r2.w, -r1.w, cb0[8].x
    r2.w = ((-(r1.wwww))+(source[8].xxxx)).w;
    // 96: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 97: add r4.yw, r4.yyyw, r8.zzzw
    r4.yw = ((r4.yyyw)+(r8.zzzw)).yw;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r4.ywyy, t5.xyzw, s6, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 99: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 100: mul r0.w, r1.w, r6.w
    r0.w = ((r1.wwww)*(r6.wwww)).w;
    // 101: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 103: mad r2.xyz, r0.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 104: mul r0.w, r5.x, cb0[33].x
    r0.w = ((r5.xxxx)*(source[33].xxxx)).w;
    // 105: mul r2.w, r5.z, cb0[34].w
    r2.w = ((r5.zzzz)*(source[34].wwww)).w;
    // 106: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 107: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: movc r2.w, r4.z, l(0), r2.w
    r2.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 109: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 110: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 111: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 112: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 113: add_sat r0.w, r0.w, cb0[33].y
    r0.w = (saturate((r0.wwww)+(source[33].yyyy))).w;
    // 114: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: mul r4.xyz, r2.wwww, cb0[20].xyzx
    r4.xyz = ((r2.wwww)*(source[20].xyzx)).xyz;
    // 116: mul r5.xyz, r2.xyzx, r4.xyzx
    r5.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 117: mad r2.xyz, -r4.xyzx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r4.xyzx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 118: mad r2.xyz, r0.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 119: add r4.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 120: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 121: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 122: mad r4.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 123: mad r5.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 124: mad r4.xyz, r0.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 125: mad r5.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 126: mad r4.xyz, r4.xyzx, r0.wwww, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r0.wwww)+(r5.xyzx)).xyz;
    // 127: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 128: max r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = (max(r0.wwww,r4.xyzx)).xyz;
    // 129: mov_sat r2.w, cb0[34].x
    r2.w = (saturate(source[34].xxxx)).w;
    // 130: mad r5.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r5.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 131: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 132: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 133: mad r5.xyz, r7.wwww, r5.xyzx, r3.wwww
    r5.xyz = ((r7.wwww)*(r5.xyzx)+(r3.wwww)).xyz;
    // 134: mul_sat r2.w, r5.y, l(50.000000)
    r2.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 136: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 137: dp2 r3.w, r6.xyxx, r6.xyxx
    r3.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 138: mul r6.xy, r6.xyxx, cb0[23].xxxx
    r6.xy = ((r6.xyxx)*(source[23].xxxx)).xy;
    // 139: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 141: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 142: add r6.z, r3.w, l(0.000010)
    r6.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 143: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 144: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 145: div r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)/(r3.wwww)).xyz;
    // 146: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 147: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 148: mul r8.xyz, r3.wwww, r6.xyzx
    r8.xyz = ((r3.wwww)*(r6.xyzx)).xyz;
    // 149: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 150: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 151: mul r9.xyz, r3.wwww, v5.xyzx
    r9.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 152: dp3 r3.w, r8.xyzx, r9.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 153: deriv_rtx_coarse r7.x, r3.w
    r7.x = (ddx_coarse(r3.wwww)).x;
    // 154: deriv_rty_coarse r7.y, r3.w
    r7.y = (ddy_coarse(r3.wwww)).y;
    // 155: dp2 r4.w, r7.xyxx, r7.xyxx
    r4.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 156: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 157: mad r4.w, r4.w, l(0.300000), r7.z
    r4.w = ((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).w;
    // 158: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 159: min r7.y, r4.w, l(1.000000)
    r7.y = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 160: add r4.w, -r7.y, l(1.000000)
    r4.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: max r10.xyz, r5.xyzx, r4.wwww
    r10.xyz = (max(r5.xyzx,r4.wwww)).xyz;
    // 162: add r10.xyz, -r5.xyzx, r10.xyzx
    r10.xyz = ((-(r5.xyzx))+(r10.xyzx)).xyz;
    // 163: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 164: mul r11.xyz, r3.wwww, r8.xyzx
    r11.xyz = ((r3.wwww)*(r8.xyzx)).xyz;
    // 165: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 166: add r2.w, r11.z, l(1.000000)
    r2.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: add r4.w, r3.w, l(1.000000)
    r4.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mov_sat r3.w, r3.w
    r3.w = (saturate(r3.wwww)).w;
    // 170: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 171: mul r3.w, r3.w, cb0[2].y
    r3.w = ((r3.wwww)*(source[2].yyyy)).w;
    // 172: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 173: mad_sat r3.w, r3.w, cb0[2].w, cb0[2].z
    r3.w = (saturate((r3.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 174: mul r3.w, r3.w, cb0[35].x
    r3.w = ((r3.wwww)*(source[35].xxxx)).w;
    // 175: add_sat r7.x, -r2.w, r4.w
    r7.x = (saturate((-(r2.wwww))+(r4.wwww))).x;
    // 176: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t8.xyzw, s9
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 177: add r2.w, r0.w, r7.x
    r2.w = ((r0.wwww)+(r7.xxxx)).w;
    // 178: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 179: mul r13.xyz, r5.xyzx, r12.yyyy
    r13.xyz = ((r5.xyzx)*(r12.yyyy)).xyz;
    // 180: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 181: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r12.yyyy)).w;
    // 182: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 183: mad r12.xyz, r5.xyzx, r4.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xyzx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: dp3 r4.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 185: mad r5.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 186: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 188: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 189: add r4.w, -r7.w, l(1.000000)
    r4.w = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: mul r12.xyz, r4.wwww, r12.xyzx
    r12.xyz = ((r4.wwww)*(r12.xyzx)).xyz;
    // 191: dp3 r5.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: dp3 r6.w, v1.xyzx, v1.xyzx
    r6.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 193: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 194: mul r14.xyz, r6.wwww, v1.xyzx
    r14.xyz = ((r6.wwww)*(v1.xyzx)).xyz;
    // 195: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 196: dp3 r6.w, v0.xyzx, v0.xyzx
    r6.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 197: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 198: mul r16.xyz, r6.wwww, v0.xyzx
    r16.xyz = ((r6.wwww)*(v0.xyzx)).xyz;
    // 199: mul r17.xyz, r14.zxyz, r16.yzxy
    r17.xyz = ((r14.zxyz)*(r16.yzxy)).xyz;
    // 200: mad r17.xyz, r14.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r14.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 201: dp3 r14.y, r14.xyzx, r11.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 202: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 203: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 204: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 205: dp2 r15.z, r18.xyxx, cb0[37].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[37].xyxx).xy).xxxx).z;
    // 206: mul r7.xz, cb0[37].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[37].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 207: dp2 r15.x, r18.xyxx, r7.xzxx
    r15.x = (dot((r18.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 208: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 209: dp4 r19.x, cb0[38].xyzw, r15.xyzw
    r19.x = (dot((source[38].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 210: dp4 r19.y, cb0[39].xyzw, r15.xyzw
    r19.y = (dot((source[39].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 211: dp4 r19.z, cb0[40].xyzw, r15.xyzw
    r19.z = (dot((source[40].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 212: mul r20.xyzw, r15.yzzx, r15.xyzz
    r20.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 213: dp4 r21.x, cb0[41].xyzw, r20.xyzw
    r21.x = (dot((source[41].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).x;
    // 214: dp4 r21.y, cb0[42].xyzw, r20.xyzw
    r21.y = (dot((source[42].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).y;
    // 215: dp4 r21.z, cb0[43].xyzw, r20.xyzw
    r21.z = (dot((source[43].xyzw).xyzw,(r20.xyzw).xyzw).xxxx).z;
    // 216: add r19.xyz, r19.xyzx, r21.xyzx
    r19.xyz = ((r19.xyzx)+(r21.xyzx)).xyz;
    // 217: mul r6.w, r15.y, r15.y
    r6.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 218: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 219: mad r6.w, r15.x, r15.x, -r6.w
    r6.w = ((r15.xxxx)*(r15.xxxx)+(-(r6.wwww))).w;
    // 220: mad r15.xyz, cb0[44].xyzx, r6.wwww, r19.xyzx
    r15.xyz = ((source[44].xyzx)*(r6.wwww)+(r19.xyzx)).xyz;
    // 221: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 222: mul r15.xyz, r15.xyzx, cb0[36].xyzx
    r15.xyz = ((r15.xyzx)*(source[36].xyzx)).xyz;
    // 223: mul r15.xyz, r15.xyzx, cb0[37].zzzz
    r15.xyz = ((r15.xyzx)*(source[37].zzzz)).xyz;
    // 224: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[36].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[36].wwww)).xyz;
    // 225: dp3 r6.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 226: add r15.xyz, -r6.wwww, r15.xyzx
    r15.xyz = ((-(r6.wwww))+(r15.xyzx)).xyz;
    // 227: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r6.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r6.wwww)).xyz;
    // 228: dp3 r6.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: mad r8.w, r7.y, l(2.000000), l(2.000000)
    r8.w = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 230: div r6.w, r6.w, r8.w
    r6.w = ((r6.wwww)/(r8.wwww)).w;
    // 231: mad r6.w, r5.w, l(5.000000), r6.w
    r6.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r6.wwww)).w;
    // 232: add_sat r6.w, r7.w, r6.w
    r6.w = (saturate((r7.wwww)+(r6.wwww))).w;
    // 233: mad r9.w, r6.w, l(-2.000000), l(3.000000)
    r9.w = ((r6.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 234: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 235: mul r6.w, r6.w, r9.w
    r6.w = ((r6.wwww)*(r9.wwww)).w;
    // 236: log r6.w, r6.w
    r6.w = (log2(r6.wwww)).w;
    // 237: mul r6.w, r6.w, l(1.500000)
    r6.w = ((r6.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 238: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 239: mul r15.xyz, r6.wwww, r15.xyzx
    r15.xyz = ((r6.wwww)*(r15.xyzx)).xyz;
    // 240: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 241: mul r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r15.xyzx)).xyz;
    // 242: mul r12.xyz, r4.xyzx, r12.xyzx
    r12.xyz = ((r4.xyzx)*(r12.xyzx)).xyz;
    // 243: mul r6.w, r7.y, l(5.000000)
    r6.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 244: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 245: mul r2.w, r2.w, r7.y
    r2.w = ((r2.wwww)*(r7.yyyy)).w;
    // 246: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 247: add r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)+(r2.wwww)).w;
    // 248: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 249: add_sat r0.w, r2.w, l(-1.000000)
    r0.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 250: dp3 r15.x, r16.xyzx, r11.xyzx
    r15.x = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 251: dp3 r15.y, r17.xyzx, r11.xyzx
    r15.y = (dot((r17.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 252: dp2 r14.x, r15.xyxx, r7.xzxx
    r14.x = (dot((r15.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 253: dp2 r14.z, r15.xyxx, cb0[37].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[37].xyxx).xy).xxxx).z;
    // 254: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t9.xyzw, s8, r6.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r6.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 255: mul r7.xyz, r14.xyzx, r14.wwww
    r7.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 256: mul r7.xyz, r7.xyzx, cb0[36].xyzx
    r7.xyz = ((r7.xyzx)*(source[36].xyzx)).xyz;
    // 257: mul r7.xyz, r7.xyzx, cb0[37].zzzz
    r7.xyz = ((r7.xyzx)*(source[37].zzzz)).xyz;
    // 258: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[36].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[36].wwww)).xyz;
    // 259: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r7.xyz, -r2.wwww, r7.xyzx
    r7.xyz = ((-(r2.wwww))+(r7.xyzx)).xyz;
    // 261: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 262: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: div r2.w, r2.w, r8.w
    r2.w = ((r2.wwww)/(r8.wwww)).w;
    // 264: mad r2.w, r5.w, l(5.000000), r2.w
    r2.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 265: add_sat r2.w, r7.w, r2.w
    r2.w = (saturate((r7.wwww)+(r2.wwww))).w;
    // 266: mad r5.w, r2.w, l(-2.000000), l(3.000000)
    r5.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 267: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 268: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 269: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 270: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 271: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 272: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 273: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 274: mad r2.w, r0.w, r5.x, r5.y
    r2.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 275: mad r2.w, r2.w, r0.w, r5.z
    r2.w = ((r2.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 276: mul r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)*(r2.wwww)).w;
    // 277: max r0.w, r0.w, r2.w
    r0.w = (max(r0.wwww,r2.wwww)).w;
    // 278: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 279: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 280: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 281: mul r12.xyz, r2.wwww, v6.xyzx
    r12.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 282: dp3 r2.w, r12.xyzx, r8.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 283: dp3 r5.w, -r12.xyzx, r8.xyzx
    r5.w = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).w;
    // 284: dp3 r6.w, r12.xyzx, r11.xyzx
    r6.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 285: mad r8.xy, r6.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r6.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 286: mad r8.zw, r5.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r5.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 287: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 288: mad r11.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 289: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 290: mul r11.yzw, r11.yyyy, cb0[47].xxyz
    r11.yzw = ((r11.yyyy)*(source[47].xxyz)).yzw;
    // 291: mad r11.xyz, r11.xxxx, cb0[46].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[46].xyzx)+(r11.yzwy)).xyz;
    // 292: mul r11.xyz, r11.xyzx, cb0[48].wwww
    r11.xyz = ((r11.xyzx)*(source[48].wwww)).xyz;
    // 293: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 294: mul r4.xyz, r4.xyzx, r11.xyzx
    r4.xyz = ((r4.xyzx)*(r11.xyzx)).xyz;
    // 295: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 296: mul r4.xyz, r13.xyzx, r4.xyzx
    r4.xyz = ((r13.xyzx)*(r4.xyzx)).xyz;
    // 297: mad r4.xyz, -r4.xyzx, r7.wwww, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(r7.wwww)+(r4.xyzx)).xyz;
    // 298: mad r4.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r4.xyzx
    r4.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r4.xyzx)).xyz;
    // 299: mul r5.xyz, r8.yyyy, cb0[47].xyzx
    r5.xyz = ((r8.yyyy)*(source[47].xyzx)).xyz;
    // 300: mad r5.xyz, cb0[46].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[46].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 301: mul r5.xyz, r5.xyzx, cb0[48].wwww
    r5.xyz = ((r5.xyzx)*(source[48].wwww)).xyz;
    // 302: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 303: mul r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 304: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 305: mad r4.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r4.xyzx
    r4.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r4.xyzx)).xyz;
    // 306: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 307: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 308: mul r0.w, r17.z, cb0[24].w
    r0.w = ((r17.zzzz)*(source[24].wwww)).w;
    // 309: mul r2.w, r17.z, cb0[25].x
    r2.w = ((r17.zzzz)*(source[25].xxxx)).w;
    // 310: mad r5.x, r16.z, cb0[24].w, -r2.w
    r5.x = ((r16.zzzz)*(source[24].wwww)+(-(r2.wwww))).x;
    // 311: mad r5.y, r16.z, cb0[25].x, r0.w
    r5.y = ((r16.zzzz)*(source[25].xxxx)+(r0.wwww)).y;
    // 312: max r0.w, |r5.x|, |r5.y|
    r0.w = (max(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 313: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 314: min r2.w, |r5.x|, |r5.y|
    r2.w = (min(abs(r5.xxxx),abs(r5.yyyy))).w;
    // 315: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 316: mul r2.w, r0.w, r0.w
    r2.w = ((r0.wwww)*(r0.wwww)).w;
    // 317: mad r5.z, r2.w, l(0.020835), l(-0.085133)
    r5.z = ((r2.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).z;
    // 318: mad r5.z, r2.w, r5.z, l(0.180141)
    r5.z = ((r2.wwww)*(r5.zzzz)+(float4(0.180141,0.180141,0.180141,0.180141))).z;
    // 319: mad r5.z, r2.w, r5.z, l(-0.330299)
    r5.z = ((r2.wwww)*(r5.zzzz)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).z;
    // 320: mad r2.w, r2.w, r5.z, l(0.999866)
    r2.w = ((r2.wwww)*(r5.zzzz)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 321: mul r5.z, r0.w, r2.w
    r5.z = ((r0.wwww)*(r2.wwww)).z;
    // 322: mad r5.z, r5.z, l(-2.000000), l(1.570796)
    r5.z = ((r5.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).z;
    // 323: lt r5.w, |r5.x|, |r5.y|
    r5.w = (asfloat((uint4)((abs(r5.xxxx))<(abs(r5.yyyy))) * 0xffffffffu)).w;
    // 324: and r5.z, r5.w, r5.z
    r5.z = (asfloat(asuint(r5.wwww) & asuint(r5.zzzz))).z;
    // 325: mad r0.w, r0.w, r2.w, r5.z
    r0.w = ((r0.wwww)*(r2.wwww)+(r5.zzzz)).w;
    // 326: lt r2.w, r5.x, -r5.x
    r2.w = (asfloat((uint4)((r5.xxxx)<(-(r5.xxxx))) * 0xffffffffu)).w;
    // 327: and r2.w, r2.w, l(0xc0490fdb)
    r2.w = (asfloat(asuint(r2.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 328: add r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)+(r2.wwww)).w;
    // 329: min r2.w, r5.x, r5.y
    r2.w = (min(r5.xxxx,r5.yyyy)).w;
    // 330: lt r2.w, r2.w, -r2.w
    r2.w = (asfloat((uint4)((r2.wwww)<(-(r2.wwww))) * 0xffffffffu)).w;
    // 331: max r5.z, r5.x, r5.y
    r5.z = (max(r5.xxxx,r5.yyyy)).z;
    // 332: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 333: add r5.x, r5.y, r5.x
    r5.x = ((r5.yyyy)+(r5.xxxx)).x;
    // 334: sqrt r5.x, r5.x
    r5.x = (sqrt(r5.xxxx)).x;
    // 335: ge r5.y, r5.z, -r5.z
    r5.y = (asfloat((uint4)((r5.zzzz)>=(-(r5.zzzz))) * 0xffffffffu)).y;
    // 336: and r2.w, r2.w, r5.y
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r5.yyyy))).w;
    // 337: movc r0.w, r2.w, -r0.w, r0.w
    r0.w = ((asuint(r2.wwww) != 0u) ? (-(r0.wwww)) : (r0.wwww)).w;
    // 338: add r2.w, r0.w, l(6.283185)
    r2.w = ((r0.wwww)+(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 339: mul r2.w, r2.w, l(0.159155)
    r2.w = ((r2.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).w;
    // 340: mul r5.y, r0.w, l(0.159155)
    r5.y = ((r0.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))).y;
    // 341: ge r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 342: movc r0.w, r0.w, r5.y, r2.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (r5.yyyy) : (r2.wwww)).w;
    // 343: add r2.w, -r0.w, -cb0[25].w
    r2.w = ((-(r0.wwww))+(-(source[25].wwww))).w;
    // 344: add r0.w, r0.w, -cb0[25].w
    r0.w = ((r0.wwww)+(-(source[25].wwww))).w;
    // 345: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 346: add r5.y, -cb0[25].w, l(1.000000)
    r5.y = ((-(source[25].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 347: div r5.y, l(1.000000, 1.000000, 1.000000, 1.000000), r5.y
    r5.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r5.yyyy)).y;
    // 348: mul_sat r2.w, r2.w, r5.y
    r2.w = (saturate((r2.wwww)*(r5.yyyy))).w;
    // 349: mul_sat r0.w, r0.w, r5.y
    r0.w = (saturate((r0.wwww)*(r5.yyyy))).w;
    // 350: mad r5.y, r2.w, l(-2.000000), l(3.000000)
    r5.y = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 351: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 352: mul r2.w, r2.w, r5.y
    r2.w = ((r2.wwww)*(r5.yyyy)).w;
    // 353: mad r5.y, r0.w, l(-2.000000), l(3.000000)
    r5.y = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 354: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 355: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 356: mul r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)*(r0.wwww)).w;
    // 357: log r2.w, r5.x
    r2.w = (log2(r5.xxxx)).w;
    // 358: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 359: mul r2.w, r2.w, l(5.082000)
    r2.w = ((r2.wwww)*(float4(5.082000,5.082000,5.082000,5.082000))).w;
    // 360: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 361: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 362: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 363: dp3 r2.w, r6.xyzx, r9.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 364: mul_sat r5.x, r2.w, cb0[26].x
    r5.x = (saturate((r2.wwww)*(source[26].xxxx))).x;
    // 365: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 366: mul_sat r5.y, r9.z, cb0[26].x
    r5.y = (saturate((r9.zzzz)*(source[26].xxxx))).y;
    // 367: add r5.z, -|r9.z|, l(1.000000)
    r5.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 368: mul r2.w, r2.w, r5.z
    r2.w = ((r2.wwww)*(r5.zzzz)).w;
    // 369: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 370: add_sat r5.y, r5.y, -cb0[26].y
    r5.y = (saturate((r5.yyyy)+(-(source[26].yyyy)))).y;
    // 371: log r5.z, r5.y
    r5.z = (log2(r5.yyyy)).z;
    // 372: lt r5.y, r5.y, l(0.000001)
    r5.y = (asfloat((uint4)((r5.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 373: mul r5.z, r5.z, cb0[26].z
    r5.z = ((r5.zzzz)*(source[26].zzzz)).z;
    // 374: exp r5.z, r5.z
    r5.z = (exp2(r5.zzzz)).z;
    // 375: mul r5.x, r5.z, r5.x
    r5.x = ((r5.zzzz)*(r5.xxxx)).x;
    // 376: movc r5.x, r5.y, l(0), r5.x
    r5.x = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).x;
    // 377: mul r5.y, r5.x, cb0[26].w
    r5.y = ((r5.xxxx)*(source[26].wwww)).y;
    // 378: mul r0.w, r0.w, r5.y
    r0.w = ((r0.wwww)*(r5.yyyy)).w;
    // 379: mul r0.w, r0.w, cb0[26].w
    r0.w = ((r0.wwww)*(source[26].wwww)).w;
    // 380: log r5.y, |r0.w|
    r5.y = (log2(abs(r0.wwww))).y;
    // 381: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 382: mul r5.y, r5.y, cb0[27].x
    r5.y = ((r5.yyyy)*(source[27].xxxx)).y;
    // 383: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 384: mul r6.xyz, cb0[9].xyzx, cb0[9].wwww
    r6.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 385: mul r5.yzw, r5.yyyy, r6.xxyz
    r5.yzw = ((r5.yyyy)*(r6.xxyz)).yzw;
    // 386: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 387: dp2 r6.x, cb0[10].xyxx, r6.xyxx
    r6.x = (dot((source[10].xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 388: add r6.x, r6.x, cb0[28].z
    r6.x = ((r6.xxxx)+(source[28].zzzz)).x;
    // 389: add_sat r6.x, r6.x, l(-0.500000)
    r6.x = (saturate((r6.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).x;
    // 390: mul r5.yzw, r5.yyzw, r6.xxxx
    r5.yzw = ((r5.yyzw)*(r6.xxxx)).yzw;
    // 391: movc r5.yzw, r0.wwww, l(0,0,0,0), r5.yyzw
    r5.yzw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.yyzw)).yzw;
    // 392: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 393: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 394: add r6.x, -r0.w, l(1000.000000)
    r6.x = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 395: mad r0.w, cb0[29].w, r6.x, r0.w
    r0.w = ((source[29].wwww)*(r6.xxxx)+(r0.wwww)).w;
    // 396: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 397: mad r0.w, cb0[29].y, cb0[29].z, r0.w
    r0.w = ((source[29].yyyy)*(source[29].zzzz)+(r0.wwww)).w;
    // 398: mul r6.x, r0.w, l(3.524534)
    r6.x = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 399: sincos null, r6.x, r6.x
    r6.x = (cos(r6.xxxx)).x;
    // 400: add r0.w, r0.w, r6.x
    r0.w = ((r0.wwww)+(r6.xxxx)).w;
    // 401: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 402: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 403: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 404: mad r0.w, r0.w, l(0.500000), cb0[29].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[29].xxxx)).w;
    // 405: mul r6.xyz, cb0[11].xyzx, cb0[28].wwww
    r6.xyz = ((source[11].xyzx)*(source[28].wwww)).xyz;
    // 406: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 407: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 408: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 409: mad r5.yzw, r1.wwww, r5.yyzw, r6.xxyz
    r5.yzw = ((r1.wwww)*(r5.yyzw)+(r6.xxyz)).yzw;
    // 410: mul r6.xy, v4.xyxx, cb0[12].zzzz
    r6.xy = ((v4.xyxx)*(source[12].zzzz)).xy;
    // 411: mul r6.zw, cb0[12].xxxy, cb0[29].zzzz
    r6.zw = ((source[12].xxxy)*(source[29].zzzz)).zw;
    // 412: mad r6.xy, r6.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r6.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 413: mad r6.zw, cb0[12].zzzz, v4.xxxy, r6.zzzw
    r6.zw = ((source[12].zzzz)*(v4.xxxy)+(r6.zzzw)).zw;
    // 414: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r6.xyxx, t7.yzwx, s5, l(0.000000)
    r0.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 415: mad r6.xy, r0.wwww, cb0[30].xxxx, r6.zwzz
    r6.xy = ((r0.wwww)*(source[30].xxxx)+(r6.zwzz)).xy;
    // 416: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t7.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 417: mul r6.xyz, r6.xyzx, r7.wwww
    r6.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 418: mul r7.xyz, cb0[13].xyzx, cb0[30].yyyy
    r7.xyz = ((source[13].xyzx)*(source[30].yyyy)).xyz;
    // 419: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 420: mad r7.xyz, r5.xxxx, r6.xyzx, -r6.xyzx
    r7.xyz = ((r5.xxxx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 421: mad r6.xyz, cb0[13].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[13].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 422: add r5.yzw, r5.yyzw, r6.xxyz
    r5.yzw = ((r5.yyzw)+(r6.xxyz)).yzw;
    // 423: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 424: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 425: mad r5.yzw, cb0[30].zzzz, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].zzzz)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 426: dp3 r0.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 427: add r6.xyz, -r5.yzwy, r0.wwww
    r6.xyz = ((-(r5.yzwy))+(r0.wwww)).xyz;
    // 428: mad r5.yzw, cb0[30].wwww, r6.xxyz, r5.yyzw
    r5.yzw = ((source[30].wwww)*(r6.xxyz)+(r5.yyzw)).yzw;
    // 429: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 430: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 431: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 432: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 433: add r6.xyz, -r0.xyzx, r0.wwww
    r6.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 434: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 435: mul r6.xyz, cb0[18].xyzx, cb0[31].yyyy
    r6.xyz = ((source[18].xyzx)*(source[31].yyyy)).xyz;
    // 436: mul r6.xyz, r6.xyzx, cb0[32].wwww
    r6.xyz = ((r6.xyzx)*(source[32].wwww)).xyz;
    // 437: mul r6.xyz, r5.xxxx, r6.xyzx
    r6.xyz = ((r5.xxxx)*(r6.xyzx)).xyz;
    // 438: mad r7.xyz, r5.xxxx, cb0[17].xyzx, -cb0[17].xyzx
    r7.xyz = ((r5.xxxx)*(source[17].xyzx)+(-(source[17].xyzx))).xyz;
    // 439: add r0.w, r5.x, l(-1.000000)
    r0.w = ((r5.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 440: mad r0.w, cb0[16].w, r0.w, l(1.000000)
    r0.w = ((source[16].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 441: mad r7.xyz, cb0[17].wwww, r7.xyzx, cb0[17].xyzx
    r7.xyz = ((source[17].wwww)*(r7.xyzx)+(source[17].xyzx)).xyz;
    // 442: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 443: mad r0.xyz, r0.wwww, cb0[16].xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 444: mad r0.xyz, r5.yzwy, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.yzwy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 445: log r0.w, |r2.w|
    r0.w = (log2(abs(r2.wwww))).w;
    // 446: lt r1.w, |r2.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 447: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 448: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 449: mul r3.xyz, r0.wwww, cb0[19].xyzx
    r3.xyz = ((r0.wwww)*(source[19].xyzx)).xyz;
    // 450: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 451: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 452: mad r0.xyz, cb0[23].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 453: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 454: mul r1.xyz, r8.wwww, cb0[47].xyzx
    r1.xyz = ((r8.wwww)*(source[47].xyzx)).xyz;
    // 455: mad r1.xyz, r8.zzzz, cb0[46].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[46].xyzx)+(r1.xyzx)).xyz;
    // 456: mul r1.xyz, r1.xyzx, cb0[48].wwww
    r1.xyz = ((r1.xyzx)*(source[48].wwww)).xyz;
    // 457: mul_sat r3.xyz, cb0[22].xyzx, cb0[22].wwww
    r3.xyz = (saturate((source[22].xyzx)*(source[22].wwww))).xyz;
    // 458: mul r5.xyz, r3.xyzx, r3.wwww
    r5.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 459: mul r3.xyz, r3.xyzx, cb0[35].xxxx
    r3.xyz = ((r3.xyzx)*(source[35].xxxx)).xyz;
    // 460: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 461: mul r3.xyz, r4.wwww, r5.xyzx
    r3.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 462: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 463: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 464: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 465: add r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)+(r0.xyzx)).xyz;
    // 466: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 467: mad o0.xyz, r2.xyzx, cb0[48].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[48].xyzx)+(r0.xyzx)).xyz;
    // 468: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 469: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 470: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 471: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 472: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 473: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 474: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 475: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 476: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 477: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 478: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 479: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 480: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 481: ftou r0.x, cb0[45].z
    r0.x = (asfloat((uint4)(source[45].zzzz))).x;
    // 482: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 483: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 484: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 485: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 486: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drr-00-neck-mi.v1 / source program 548bfcb45fdddb4b91812819ce2a60fc
#else // SOURCE_CHARACTER_BASE_DISPATCH_CASES
    case 96u: return SourceCharacterBase96(input);
    case 97u: return SourceCharacterBase97(input);
    case 98u: return SourceCharacterBase98(input);
    case 99u: return SourceCharacterBase99(input);
    case 100u: return SourceCharacterBase100(input);
    case 101u: return SourceCharacterBase101(input);
    case 102u: return SourceCharacterBase102(input);
    case 103u: return SourceCharacterBase103(input);
    case 104u: return SourceCharacterBase104(input);
    case 105u: return SourceCharacterBase105(input);
    case 106u: return SourceCharacterBase106(input);
    case 107u: return SourceCharacterBase107(input);
#endif // SOURCE_CHARACTER_BASE_DISPATCH_CASES
