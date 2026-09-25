SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase235(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[21]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[26].z=(g_SourceCharacterTime.xxxx).x;
    source[29].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[29].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[30].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[30].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[30].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    // Original engine primitive opacity/environment rows.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[35]=g_SourceCharacterEnvironmentColor; source[36]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[32].xxxx
    r0.xy = ((v4.xyxx)*(source[32].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[32].y
    r0.x = ((r0.xxxx)+(-(source[32].yyyy))).x;
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
    // 13: mad r0.xyz, cb0[28].yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((source[28].yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[28].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[28].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
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
    // 62: mad r3.xyz, cb0[28].yyyy, r3.xyzx, r2.xyzx
    r3.xyz = ((source[28].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[28].zzzz, r2.xyzx, r3.xyzx
    r2.xyz = ((source[28].zzzz)*(r2.xyzx)+(r3.xyzx)).xyz;
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
    // 75: mad r2.xyz, cb0[28].yyyy, r9.xyzx, r8.xyzx
    r2.xyz = ((source[28].yyyy)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r8.xyz, -r2.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[28].zzzz, r8.xyzx, r2.xyzx
    r2.xyz = ((source[28].zzzz)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[8].w, l(1.000000)
    r0.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[26].z
    r0.w = ((r0.wwww)*(source[26].zzzz)).w;
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
    // 104: mul r0.w, r5.x, cb0[30].w
    r0.w = ((r5.xxxx)*(source[30].wwww)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[31].x
    r0.w = (saturate((r0.wwww)+(source[31].xxxx))).w;
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
    // 123: mov_sat r2.w, cb0[31].w
    r2.w = (saturate(source[31].wwww)).w;
    // 124: mad r5.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r5.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 125: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 126: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 127: mad r5.xyw, r7.wwww, r5.xyxw, r3.wwww
    r5.xyw = ((r7.wwww)*(r5.xyxw)+(r3.wwww)).xyw;
    // 128: mul_sat r2.w, r5.y, l(50.000000)
    r2.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 129: add r3.w, -cb0[33].y, cb0[33].x
    r3.w = ((-(source[33].yyyy))+(source[33].xxxx)).w;
    // 130: mad r3.w, r6.x, r3.w, cb0[33].y
    r3.w = ((r6.xxxx)*(r3.wwww)+(source[33].yyyy)).w;
    // 131: add r6.x, -r3.w, cb0[33].w
    r6.x = ((-(r3.wwww))+(source[33].wwww)).x;
    // 132: mad r3.w, r6.y, r6.x, r3.w
    r3.w = ((r6.yyyy)*(r6.xxxx)+(r3.wwww)).w;
    // 133: add r6.x, -r3.w, cb0[34].y
    r6.x = ((-(r3.wwww))+(source[34].yyyy)).x;
    // 134: mad r3.w, r6.z, r6.x, r3.w
    r3.w = ((r6.zzzz)*(r6.xxxx)+(r3.wwww)).w;
    // 135: mul r3.w, r5.z, r3.w
    r3.w = ((r5.zzzz)*(r3.wwww)).w;
    // 136: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 137: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: movc r3.w, r4.z, l(0), r3.w
    r3.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 139: max r3.w, r3.w, cb0[1].x
    r3.w = (max(r3.wwww,source[1].xxxx)).w;
    // 140: min r7.z, r3.w, l(1.000000)
    r7.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 141: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 142: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 143: dp2 r3.w, r6.xyxx, r6.xyxx
    r3.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 144: mul r6.xy, r6.xyxx, cb0[23].xxxx
    r6.xy = ((r6.xyxx)*(source[23].xxxx)).xy;
    // 145: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 147: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 148: add r6.z, r3.w, l(0.000010)
    r6.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 149: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 150: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 151: div r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)/(r3.wwww)).xyz;
    // 152: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 153: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 154: mul r8.xyz, r3.wwww, r6.xyzx
    r8.xyz = ((r3.wwww)*(r6.xyzx)).xyz;
    // 155: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 156: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 157: mul r9.xyz, r3.wwww, v5.xyzx
    r9.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 158: dp3 r3.w, r8.xyzx, r9.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 159: deriv_rtx_coarse r7.x, r3.w
    r7.x = (ddx_coarse(r3.wwww)).x;
    // 160: deriv_rty_coarse r7.y, r3.w
    r7.y = (ddy_coarse(r3.wwww)).y;
    // 161: dp2 r4.z, r7.xyxx, r7.xyxx
    r4.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 162: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 163: mad r4.z, r4.z, l(0.300000), r7.z
    r4.z = ((r4.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).z;
    // 164: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 165: min r7.y, r4.z, l(1.000000)
    r7.y = (min(r4.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 166: add r4.z, -r7.y, l(1.000000)
    r4.z = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 167: max r10.xyz, r5.xywx, r4.zzzz
    r10.xyz = (max(r5.xywx,r4.zzzz)).xyz;
    // 168: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 169: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 170: mul r11.xyz, r3.wwww, r8.xyzx
    r11.xyz = ((r3.wwww)*(r8.xyzx)).xyz;
    // 171: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 172: add r2.w, r11.z, l(1.000000)
    r2.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r4.z, r3.w, l(1.000000)
    r4.z = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mov_sat r3.w, r3.w
    r3.w = (saturate(r3.wwww)).w;
    // 176: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 177: mul r3.w, r3.w, cb0[2].y
    r3.w = ((r3.wwww)*(source[2].yyyy)).w;
    // 178: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 179: mad_sat r3.w, r3.w, cb0[2].w, cb0[2].z
    r3.w = (saturate((r3.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 180: mul r3.w, r3.w, cb0[34].z
    r3.w = ((r3.wwww)*(source[34].zzzz)).w;
    // 181: add_sat r7.x, -r2.w, r4.z
    r7.x = (saturate((-(r2.wwww))+(r4.zzzz))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t8.xyzw, s9
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 183: add r2.w, r0.w, r7.x
    r2.w = ((r0.wwww)+(r7.xxxx)).w;
    // 184: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 185: mul r13.xyz, r5.xywx, r12.yyyy
    r13.xyz = ((r5.xywx)*(r12.yyyy)).xyz;
    // 186: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 187: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.z = r12.y != 0.f ? 1.f / r12.y : 0.f;
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
    // 198: dp3 r6.w, v1.xyzx, v1.xyzx
    r6.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 199: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 200: mul r14.xyz, r6.wwww, v1.xyzx
    r14.xyz = ((r6.wwww)*(v1.xyzx)).xyz;
    // 201: dp3 r6.w, v0.xyzx, v0.xyzx
    r6.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 202: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 203: mul r15.xyz, r6.wwww, v0.xyzx
    r15.xyz = ((r6.wwww)*(v0.xyzx)).xyz;
    // 204: mul r16.xyz, r14.zxyz, r15.yzxy
    r16.xyz = ((r14.zxyz)*(r15.yzxy)).xyz;
    // 205: mad r16.xyz, r14.yzxy, r15.zxyz, -r16.xyzx
    r16.xyz = ((r14.yzxy)*(r15.zxyz)+(-(r16.xyzx))).xyz;
    // 206: mul r16.xyz, r16.xyzx, v1.wwww
    r16.xyz = ((r16.xyzx)*(v1.wwww)).xyz;
    // 207: dp3 r17.y, r16.xyzx, r8.xyzx
    r17.y = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 208: dp3 r16.y, r16.xyzx, r11.xyzx
    r16.y = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 209: dp3 r17.x, r15.xyzx, r8.xyzx
    r17.x = (dot((r15.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 210: dp3 r16.x, r15.xyzx, r11.xyzx
    r16.x = (dot((r15.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 211: dp2 r15.z, r17.xyxx, cb0[36].xyxx
    r15.z = (dot((r17.xyxx).xy,(source[36].xyxx).xy).xxxx).z;
    // 212: mul r7.xz, cb0[36].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[36].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 213: dp2 r15.x, r17.xyxx, r7.xzxx
    r15.x = (dot((r17.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 214: dp2 r18.x, r16.xyxx, r7.xzxx
    r18.x = (dot((r16.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 215: dp2 r18.z, r16.xyxx, cb0[36].xyxx
    r18.z = (dot((r16.xyxx).xy,(source[36].xyxx).xy).xxxx).z;
    // 216: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 217: dp3 r18.y, r14.xyzx, r11.xyzx
    r18.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 218: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 219: dp4 r14.x, cb0[37].xyzw, r15.xyzw
    r14.x = (dot((source[37].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 220: dp4 r14.y, cb0[38].xyzw, r15.xyzw
    r14.y = (dot((source[38].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 221: dp4 r14.z, cb0[39].xyzw, r15.xyzw
    r14.z = (dot((source[39].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 222: mul r16.xyzw, r15.yzzx, r15.xyzz
    r16.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 223: dp4 r19.x, cb0[40].xyzw, r16.xyzw
    r19.x = (dot((source[40].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 224: dp4 r19.y, cb0[41].xyzw, r16.xyzw
    r19.y = (dot((source[41].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 225: dp4 r19.z, cb0[42].xyzw, r16.xyzw
    r19.z = (dot((source[42].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 226: add r14.xyz, r14.xyzx, r19.xyzx
    r14.xyz = ((r14.xyzx)+(r19.xyzx)).xyz;
    // 227: mul r6.w, r15.y, r15.y
    r6.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 228: mov r17.z, r15.y
    r17.z = (r15.yyyy).z;
    // 229: mad r6.w, r15.x, r15.x, -r6.w
    r6.w = ((r15.xxxx)*(r15.xxxx)+(-(r6.wwww))).w;
    // 230: mad r14.xyz, cb0[43].xyzx, r6.wwww, r14.xyzx
    r14.xyz = ((source[43].xyzx)*(r6.wwww)+(r14.xyzx)).xyz;
    // 231: max r14.xyz, r14.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r14.xyz = (max(r14.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 232: mul r14.xyz, r14.xyzx, cb0[35].xyzx
    r14.xyz = ((r14.xyzx)*(source[35].xyzx)).xyz;
    // 233: mul r14.xyz, r14.xyzx, cb0[36].zzzz
    r14.xyz = ((r14.xyzx)*(source[36].zzzz)).xyz;
    // 234: mad r14.xyz, r14.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[35].wwww
    r14.xyz = ((r14.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[35].wwww)).xyz;
    // 235: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: add r14.xyz, -r6.wwww, r14.xyzx
    r14.xyz = ((-(r6.wwww))+(r14.xyzx)).xyz;
    // 237: mad r14.xyz, r14.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r6.wwww
    r14.xyz = ((r14.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r6.wwww)).xyz;
    // 238: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 239: mad r7.x, r7.y, l(2.000000), l(2.000000)
    r7.x = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).x;
    // 240: div r6.w, r6.w, r7.x
    r6.w = ((r6.wwww)/(r7.xxxx)).w;
    // 241: mad r6.w, r5.w, l(5.000000), r6.w
    r6.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r6.wwww)).w;
    // 242: add_sat r6.w, r7.w, r6.w
    r6.w = (saturate((r7.wwww)+(r6.wwww))).w;
    // 243: mad r7.z, r6.w, l(-2.000000), l(3.000000)
    r7.z = ((r6.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 244: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 245: mul r6.w, r6.w, r7.z
    r6.w = ((r6.wwww)*(r7.zzzz)).w;
    // 246: log r6.w, r6.w
    r6.w = (log2(r6.wwww)).w;
    // 247: mul r6.w, r6.w, l(1.500000)
    r6.w = ((r6.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 248: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 249: mul r14.xyz, r6.wwww, r14.xyzx
    r14.xyz = ((r6.wwww)*(r14.xyzx)).xyz;
    // 250: mul r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)*(r14.xyzx)).xyz;
    // 251: mul r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)*(r14.xyzx)).xyz;
    // 252: mul r12.xyz, r4.xywx, r12.xyzx
    r12.xyz = ((r4.xywx)*(r12.xyzx)).xyz;
    // 253: mul r6.w, r7.y, l(5.000000)
    r6.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 254: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 255: mul r2.w, r2.w, r7.y
    r2.w = ((r2.wwww)*(r7.yyyy)).w;
    // 256: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 257: add r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)+(r2.wwww)).w;
    // 258: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 259: add_sat r0.w, r2.w, l(-1.000000)
    r0.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r18.xyzx, t9.xyzw, s8, r6.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r18.xyzx).xyz, (r6.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 261: mul r14.xyz, r14.xyzx, r14.wwww
    r14.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 262: mul r14.xyz, r14.xyzx, cb0[35].xyzx
    r14.xyz = ((r14.xyzx)*(source[35].xyzx)).xyz;
    // 263: mul r14.xyz, r14.xyzx, cb0[36].zzzz
    r14.xyz = ((r14.xyzx)*(source[36].zzzz)).xyz;
    // 264: mad r14.xyz, r14.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[35].wwww
    r14.xyz = ((r14.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[35].wwww)).xyz;
    // 265: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r14.xyz, -r2.wwww, r14.xyzx
    r14.xyz = ((-(r2.wwww))+(r14.xyzx)).xyz;
    // 267: mad r14.xyz, r14.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r14.xyz = ((r14.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 268: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r2.w, r2.w, r7.x
    r2.w = ((r2.wwww)/(r7.xxxx)).w;
    // 270: mad r2.w, r5.w, l(5.000000), r2.w
    r2.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 271: add_sat r2.w, r7.w, r2.w
    r2.w = (saturate((r7.wwww)+(r2.wwww))).w;
    // 272: mad r5.w, r2.w, l(-2.000000), l(3.000000)
    r5.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 274: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 275: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 276: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 278: mul r7.xyz, r2.wwww, r14.xyzx
    r7.xyz = ((r2.wwww)*(r14.xyzx)).xyz;
    // 279: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 280: mad r2.w, r0.w, r5.x, r5.y
    r2.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 281: mad r2.w, r2.w, r0.w, r5.z
    r2.w = ((r2.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 282: mul r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)*(r2.wwww)).w;
    // 283: max r0.w, r0.w, r2.w
    r0.w = (max(r0.wwww,r2.wwww)).w;
    // 284: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 285: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 287: mul r12.xyz, r2.wwww, v6.xyzx
    r12.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r2.w, r12.xyzx, r8.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
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
    // 294: mad r11.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 296: mul r11.yzw, r11.yyyy, cb0[46].xxyz
    r11.yzw = ((r11.yyyy)*(source[46].xxyz)).yzw;
    // 297: mad r11.xyz, r11.xxxx, cb0[45].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[45].xyzx)+(r11.yzwy)).xyz;
    // 298: mul r11.xyz, r11.xyzx, cb0[47].wwww
    r11.xyz = ((r11.xyzx)*(source[47].wwww)).xyz;
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
    // 305: mul r5.xyz, r8.yyyy, cb0[46].xyzx
    r5.xyz = ((r8.yyyy)*(source[46].xyzx)).xyz;
    // 306: mad r5.xyz, cb0[45].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[45].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, cb0[47].wwww
    r5.xyz = ((r5.xyzx)*(source[47].wwww)).xyz;
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
    // 314: dp3 r0.w, r6.xyzx, r9.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 315: mul_sat r2.w, r0.w, cb0[23].w
    r2.w = (saturate((r0.wwww)*(source[23].wwww))).w;
    // 316: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 317: mul_sat r5.x, r9.z, cb0[23].w
    r5.x = (saturate((r9.zzzz)*(source[23].wwww))).x;
    // 318: add r5.x, -r5.x, l(1.000000)
    r5.x = ((-(r5.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 319: log r5.y, r5.x
    r5.y = (log2(r5.xxxx)).y;
    // 320: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 321: mul r5.y, r5.y, cb0[24].x
    r5.y = ((r5.yyyy)*(source[24].xxxx)).y;
    // 322: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 323: mul r2.w, r2.w, r5.y
    r2.w = ((r2.wwww)*(r5.yyyy)).w;
    // 324: mul r5.yzw, cb0[9].xxyz, cb0[9].wwww
    r5.yzw = ((source[9].xxyz)*(source[9].wwww)).yzw;
    // 325: mul r5.yzw, r2.wwww, r5.yyzw
    r5.yzw = ((r2.wwww)*(r5.yyzw)).yzw;
    // 326: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 327: dp2 r2.w, cb0[10].xyxx, r6.xyxx
    r2.w = (dot((source[10].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 328: add r2.w, r2.w, cb0[25].z
    r2.w = ((r2.wwww)+(source[25].zzzz)).w;
    // 329: add_sat r2.w, r2.w, l(-0.500000)
    r2.w = (saturate((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).w;
    // 330: mul r5.yzw, r5.yyzw, r2.wwww
    r5.yzw = ((r5.yyzw)*(r2.wwww)).yzw;
    // 331: movc r5.xyz, r5.xxxx, l(0,0,0,0), r5.yzwy
    r5.xyz = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.yzwy)).xyz;
    // 332: add r2.w, cb0[0].y, cb0[0].x
    r2.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 333: add r2.w, r2.w, cb0[0].z
    r2.w = ((r2.wwww)+(source[0].zzzz)).w;
    // 334: add r5.w, -r2.w, l(1000.000000)
    r5.w = ((-(r2.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 335: mad r2.w, cb0[26].w, r5.w, r2.w
    r2.w = ((source[26].wwww)*(r5.wwww)+(r2.wwww)).w;
    // 336: mul r2.w, r2.w, l(0.010000)
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 337: mad r2.w, cb0[26].y, cb0[26].z, r2.w
    r2.w = ((source[26].yyyy)*(source[26].zzzz)+(r2.wwww)).w;
    // 338: mul r5.w, r2.w, l(3.524534)
    r5.w = ((r2.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 339: sincos null, r5.w, r5.w
    r5.w = (cos(r5.wwww)).w;
    // 340: add r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)+(r5.wwww)).w;
    // 341: mul r2.w, r2.w, l(1.328987)
    r2.w = ((r2.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 342: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 343: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 344: mad r2.w, r2.w, l(0.500000), cb0[26].x
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[26].xxxx)).w;
    // 345: mul r6.xyz, cb0[11].xyzx, cb0[25].wwww
    r6.xyz = ((source[11].xyzx)*(source[25].wwww)).xyz;
    // 346: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 347: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 348: mul r6.xyz, r2.wwww, r6.xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 349: mad r5.xyz, r1.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 350: mul r6.xy, v4.xyxx, cb0[12].zzzz
    r6.xy = ((v4.xyxx)*(source[12].zzzz)).xy;
    // 351: mul r6.zw, cb0[12].xxxy, cb0[26].zzzz
    r6.zw = ((source[12].xxxy)*(source[26].zzzz)).zw;
    // 352: mad r6.xy, r6.zwzz, l(-0.500000, 0.500000, 0.000000, 0.000000), r6.xyxx
    r6.xy = ((r6.zwzz)*(float4(-0.500000,0.500000,0.000000,0.000000))+(r6.xyxx)).xy;
    // 353: mad r6.zw, cb0[12].zzzz, v4.xxxy, r6.zzzw
    r6.zw = ((source[12].zzzz)*(v4.xxxy)+(r6.zzzw)).zw;
    // 354: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r6.xyxx, t7.yzwx, s5, l(0.000000)
    r1.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 355: mad r6.xy, r1.wwww, cb0[27].xxxx, r6.zwzz
    r6.xy = ((r1.wwww)*(source[27].xxxx)+(r6.zwzz)).xy;
    // 356: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t7.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 357: mul r6.xyz, r6.xyzx, r7.wwww
    r6.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 358: mul r7.xyz, cb0[13].xyzx, cb0[27].yyyy
    r7.xyz = ((source[13].xyzx)*(source[27].yyyy)).xyz;
    // 359: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 360: mul_sat r1.w, r0.w, cb0[27].z
    r1.w = (saturate((r0.wwww)*(source[27].zzzz))).w;
    // 361: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 362: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 363: mul_sat r2.w, r9.z, cb0[27].z
    r2.w = (saturate((r9.zzzz)*(source[27].zzzz))).w;
    // 364: add r5.w, -|r9.z|, l(1.000000)
    r5.w = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 365: mul r0.w, r0.w, r5.w
    r0.w = ((r0.wwww)*(r5.wwww)).w;
    // 366: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 367: add_sat r2.w, r2.w, -cb0[27].w
    r2.w = (saturate((r2.wwww)+(-(source[27].wwww)))).w;
    // 368: log r5.w, r2.w
    r5.w = (log2(r2.wwww)).w;
    // 369: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 370: mul r5.w, r5.w, cb0[28].x
    r5.w = ((r5.wwww)*(source[28].xxxx)).w;
    // 371: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 372: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 373: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 374: mad r7.xyz, r1.wwww, r6.xyzx, -r6.xyzx
    r7.xyz = ((r1.wwww)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 375: mad r6.xyz, cb0[13].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[13].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 376: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 377: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 378: add r6.xyz, -r5.xyzx, r2.wwww
    r6.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 379: mad r5.xyz, cb0[28].yyyy, r6.xyzx, r5.xyzx
    r5.xyz = ((source[28].yyyy)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 380: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 381: add r6.xyz, -r5.xyzx, r2.wwww
    r6.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 382: mad r5.xyz, cb0[28].zzzz, r6.xyzx, r5.xyzx
    r5.xyz = ((source[28].zzzz)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 383: dp3 r2.w, r0.xyzx, r0.xyzx
    r2.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 384: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 385: div r0.xyz, r0.xyzx, r2.wwww
    r0.xyz = ((r0.xyzx)/(r2.wwww)).xyz;
    // 386: dp3 r2.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 387: add r6.xyz, -r0.xyzx, r2.wwww
    r6.xyz = ((-(r0.xyzx))+(r2.wwww)).xyz;
    // 388: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 389: mul r6.xyz, cb0[18].xyzx, cb0[29].xxxx
    r6.xyz = ((source[18].xyzx)*(source[29].xxxx)).xyz;
    // 390: mul r6.xyz, r6.xyzx, cb0[30].zzzz
    r6.xyz = ((r6.xyzx)*(source[30].zzzz)).xyz;
    // 391: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 392: mad r7.xyz, r1.wwww, cb0[17].xyzx, -cb0[17].xyzx
    r7.xyz = ((r1.wwww)*(source[17].xyzx)+(-(source[17].xyzx))).xyz;
    // 393: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 394: mad r1.w, cb0[16].w, r1.w, l(1.000000)
    r1.w = ((source[16].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 395: mad r7.xyz, cb0[17].wwww, r7.xyzx, cb0[17].xyzx
    r7.xyz = ((source[17].wwww)*(r7.xyzx)+(source[17].xyzx)).xyz;
    // 396: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 397: mad r0.xyz, r1.wwww, cb0[16].xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 398: mad r0.xyz, r5.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 399: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 400: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 401: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 402: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 403: mul r3.xyz, r1.wwww, cb0[19].xyzx
    r3.xyz = ((r1.wwww)*(source[19].xyzx)).xyz;
    // 404: movc r3.xyz, r0.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 405: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 406: mad r0.xyz, cb0[23].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 407: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 408: mul r1.xyz, r8.wwww, cb0[46].xyzx
    r1.xyz = ((r8.wwww)*(source[46].xyzx)).xyz;
    // 409: mad r1.xyz, r8.zzzz, cb0[45].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[45].xyzx)+(r1.xyzx)).xyz;
    // 410: mul r1.xyz, r1.xyzx, cb0[47].wwww
    r1.xyz = ((r1.xyzx)*(source[47].wwww)).xyz;
    // 411: mul_sat r3.xyz, cb0[22].xyzx, cb0[22].wwww
    r3.xyz = (saturate((source[22].xyzx)*(source[22].wwww))).xyz;
    // 412: mul r5.xyz, r3.xyzx, r3.wwww
    r5.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 413: mul r3.xyz, r3.xyzx, cb0[34].zzzz
    r3.xyz = ((r3.xyzx)*(source[34].zzzz)).xyz;
    // 414: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 415: mul r3.xyz, r4.zzzz, r5.xyzx
    r3.xyz = ((r4.zzzz)*(r5.xyzx)).xyz;
    // 416: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 417: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 418: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 419: add r0.xyz, r4.xywx, r0.xyzx
    r0.xyz = ((r4.xywx)+(r0.xyzx)).xyz;
    // 420: dp3 o4.y, r4.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 421: mad o0.xyz, r2.xyzx, cb0[47].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[47].xyzx)+(r0.xyzx)).xyz;
    // 422: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 423: dp3 r0.x, r17.xyzx, r17.xyzx
    r0.x = (dot((r17.xyzx).xyz,(r17.xyzx).xyz).xxxx).x;
    // 424: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 425: mul r0.xyz, r0.xxxx, r17.xyzx
    r0.xyz = ((r0.xxxx)*(r17.xyzx)).xyz;
    // 426: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 427: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 428: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 429: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 430: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 431: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 432: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 433: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 434: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 435: ftou r0.x, cb0[44].z
    r0.x = (asfloat((uint4)(source[44].zzzz))).x;
    // 436: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 437: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 438: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 439: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 440: ret
    return output;
}

// source.character.selection-native-236.v1 / source program a7fc8aaf0c738d4ea3e2c64f0de1f724
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase236(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[23].z=(g_SourceCharacterTime.xxxx).x;
    source[24].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    // Original engine primitive opacity/environment rows.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[29]=g_SourceCharacterEnvironmentColor; source[30]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[25].zzzz
    r0.xy = ((v4.xyxx)*(source[25].zzzz)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[25].w
    r0.x = ((r0.xxxx)+(-(source[25].wwww))).x;
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
    // 13: mad r0.xyz, cb0[21].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[21].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 15: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 16: mad r0.xyz, cb0[22].xxxx, r2.xyzx, r0.xyzx
    r0.xyz = ((source[22].xxxx)*(r2.xyzx)+(r0.xyzx)).xyz;
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
    // 62: mad r3.xyz, cb0[21].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[21].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 64: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r2.xyz, -r3.xyzx, r0.wwww
    r2.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 66: mad r2.xyz, cb0[22].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[22].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
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
    // 75: mad r2.xyz, cb0[21].wwww, r9.xyzx, r8.xyzx
    r2.xyz = ((source[21].wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r8.xyz, -r2.xyzx, r0.wwww
    r8.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[22].xxxx, r8.xyzx, r2.xyzx
    r2.xyz = ((source[22].xxxx)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, cb0[23].z
    r0.w = ((r0.wwww)*(source[23].zzzz)).w;
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
    // 104: mul r0.w, r5.x, cb0[24].y
    r0.w = ((r5.xxxx)*(source[24].yyyy)).w;
    // 105: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 106: movc r0.w, r4.x, l(0), r0.w
    r0.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 107: add_sat r0.w, r0.w, cb0[24].z
    r0.w = (saturate((r0.wwww)+(source[24].zzzz))).w;
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
    // 123: mov_sat r2.w, cb0[25].y
    r2.w = (saturate(source[25].yyyy)).w;
    // 124: mad r5.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r5.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 125: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 126: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 127: mad r5.xyw, r7.wwww, r5.xyxw, r3.wwww
    r5.xyw = ((r7.wwww)*(r5.xyxw)+(r3.wwww)).xyw;
    // 128: mul_sat r2.w, r5.y, l(50.000000)
    r2.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 129: add r3.w, -cb0[26].w, cb0[26].z
    r3.w = ((-(source[26].wwww))+(source[26].zzzz)).w;
    // 130: mad r3.w, r6.x, r3.w, cb0[26].w
    r3.w = ((r6.xxxx)*(r3.wwww)+(source[26].wwww)).w;
    // 131: add r6.x, -r3.w, cb0[27].y
    r6.x = ((-(r3.wwww))+(source[27].yyyy)).x;
    // 132: mad r3.w, r6.y, r6.x, r3.w
    r3.w = ((r6.yyyy)*(r6.xxxx)+(r3.wwww)).w;
    // 133: add r6.x, -r3.w, cb0[27].w
    r6.x = ((-(r3.wwww))+(source[27].wwww)).x;
    // 134: mad r3.w, r6.z, r6.x, r3.w
    r3.w = ((r6.zzzz)*(r6.xxxx)+(r3.wwww)).w;
    // 135: mul r3.w, r5.z, r3.w
    r3.w = ((r5.zzzz)*(r3.wwww)).w;
    // 136: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 137: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: movc r3.w, r4.z, l(0), r3.w
    r3.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 139: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 140: min r7.z, r3.w, l(1.000000)
    r7.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 141: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 142: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 143: dp2 r3.w, r6.xyxx, r6.xyxx
    r3.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 144: mul r6.xy, r6.xyxx, cb0[19].xxxx
    r6.xy = ((r6.xyxx)*(source[19].xxxx)).xy;
    // 145: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 147: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 148: add r6.z, r3.w, l(0.000010)
    r6.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 149: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 150: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 151: div r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)/(r3.wwww)).xyz;
    // 152: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 153: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 154: mul r8.xyz, r3.wwww, r6.xyzx
    r8.xyz = ((r3.wwww)*(r6.xyzx)).xyz;
    // 155: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 156: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 157: mul r9.xyz, r3.wwww, v5.xyzx
    r9.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 158: dp3 r3.w, r8.xyzx, r9.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 159: deriv_rtx_coarse r7.x, r3.w
    r7.x = (ddx_coarse(r3.wwww)).x;
    // 160: deriv_rty_coarse r7.y, r3.w
    r7.y = (ddy_coarse(r3.wwww)).y;
    // 161: dp2 r4.z, r7.xyxx, r7.xyxx
    r4.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 162: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 163: mad r4.z, r4.z, l(0.300000), r7.z
    r4.z = ((r4.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz)).z;
    // 164: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 165: min r7.y, r4.z, l(1.000000)
    r7.y = (min(r4.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 166: add r4.z, -r7.y, l(1.000000)
    r4.z = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 167: max r10.xyz, r5.xywx, r4.zzzz
    r10.xyz = (max(r5.xywx,r4.zzzz)).xyz;
    // 168: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 169: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 170: mul r11.xyz, r3.wwww, r8.xyzx
    r11.xyz = ((r3.wwww)*(r8.xyzx)).xyz;
    // 171: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 172: add r2.w, r11.z, l(1.000000)
    r2.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add r4.z, r3.w, l(1.000000)
    r4.z = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mov_sat r3.w, r3.w
    r3.w = (saturate(r3.wwww)).w;
    // 176: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 177: mul r3.w, r3.w, cb0[1].y
    r3.w = ((r3.wwww)*(source[1].yyyy)).w;
    // 178: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 179: mad_sat r3.w, r3.w, cb0[1].w, cb0[1].z
    r3.w = (saturate((r3.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 180: mul r3.w, r3.w, cb0[28].x
    r3.w = ((r3.wwww)*(source[28].xxxx)).w;
    // 181: add_sat r7.x, -r2.w, r4.z
    r7.x = (saturate((-(r2.wwww))+(r4.zzzz))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t6.xyzw, s7
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 183: add r2.w, r0.w, r7.x
    r2.w = ((r0.wwww)+(r7.xxxx)).w;
    // 184: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 185: mul r13.xyz, r5.xywx, r12.yyyy
    r13.xyz = ((r5.xywx)*(r12.yyyy)).xyz;
    // 186: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 187: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r4.z = r12.y != 0.f ? 1.f / r12.y : 0.f;
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
    // 198: dp3 r6.w, v1.xyzx, v1.xyzx
    r6.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 199: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 200: mul r14.xyz, r6.wwww, v1.xyzx
    r14.xyz = ((r6.wwww)*(v1.xyzx)).xyz;
    // 201: dp3 r6.w, v0.xyzx, v0.xyzx
    r6.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 202: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 203: mul r15.xyz, r6.wwww, v0.xyzx
    r15.xyz = ((r6.wwww)*(v0.xyzx)).xyz;
    // 204: mul r16.xyz, r14.zxyz, r15.yzxy
    r16.xyz = ((r14.zxyz)*(r15.yzxy)).xyz;
    // 205: mad r16.xyz, r14.yzxy, r15.zxyz, -r16.xyzx
    r16.xyz = ((r14.yzxy)*(r15.zxyz)+(-(r16.xyzx))).xyz;
    // 206: mul r16.xyz, r16.xyzx, v1.wwww
    r16.xyz = ((r16.xyzx)*(v1.wwww)).xyz;
    // 207: dp3 r17.y, r16.xyzx, r8.xyzx
    r17.y = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 208: dp3 r16.y, r16.xyzx, r11.xyzx
    r16.y = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 209: dp3 r17.x, r15.xyzx, r8.xyzx
    r17.x = (dot((r15.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 210: dp3 r16.x, r15.xyzx, r11.xyzx
    r16.x = (dot((r15.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 211: dp2 r15.z, r17.xyxx, cb0[30].xyxx
    r15.z = (dot((r17.xyxx).xy,(source[30].xyxx).xy).xxxx).z;
    // 212: mul r7.xz, cb0[30].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r7.xz = ((source[30].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 213: dp2 r15.x, r17.xyxx, r7.xzxx
    r15.x = (dot((r17.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 214: dp2 r18.x, r16.xyxx, r7.xzxx
    r18.x = (dot((r16.xyxx).xy,(r7.xzxx).xy).xxxx).x;
    // 215: dp2 r18.z, r16.xyxx, cb0[30].xyxx
    r18.z = (dot((r16.xyxx).xy,(source[30].xyxx).xy).xxxx).z;
    // 216: dp3 r15.y, r14.xyzx, r8.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 217: dp3 r18.y, r14.xyzx, r11.xyzx
    r18.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 218: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 219: dp4 r14.x, cb0[31].xyzw, r15.xyzw
    r14.x = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 220: dp4 r14.y, cb0[32].xyzw, r15.xyzw
    r14.y = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 221: dp4 r14.z, cb0[33].xyzw, r15.xyzw
    r14.z = (dot((source[33].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 222: mul r16.xyzw, r15.yzzx, r15.xyzz
    r16.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 223: dp4 r19.x, cb0[34].xyzw, r16.xyzw
    r19.x = (dot((source[34].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 224: dp4 r19.y, cb0[35].xyzw, r16.xyzw
    r19.y = (dot((source[35].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 225: dp4 r19.z, cb0[36].xyzw, r16.xyzw
    r19.z = (dot((source[36].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 226: add r14.xyz, r14.xyzx, r19.xyzx
    r14.xyz = ((r14.xyzx)+(r19.xyzx)).xyz;
    // 227: mul r6.w, r15.y, r15.y
    r6.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 228: mov r17.z, r15.y
    r17.z = (r15.yyyy).z;
    // 229: mad r6.w, r15.x, r15.x, -r6.w
    r6.w = ((r15.xxxx)*(r15.xxxx)+(-(r6.wwww))).w;
    // 230: mad r14.xyz, cb0[37].xyzx, r6.wwww, r14.xyzx
    r14.xyz = ((source[37].xyzx)*(r6.wwww)+(r14.xyzx)).xyz;
    // 231: max r14.xyz, r14.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r14.xyz = (max(r14.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 232: mul r14.xyz, r14.xyzx, cb0[29].xyzx
    r14.xyz = ((r14.xyzx)*(source[29].xyzx)).xyz;
    // 233: mul r14.xyz, r14.xyzx, cb0[30].zzzz
    r14.xyz = ((r14.xyzx)*(source[30].zzzz)).xyz;
    // 234: mad r14.xyz, r14.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[29].wwww
    r14.xyz = ((r14.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[29].wwww)).xyz;
    // 235: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: add r14.xyz, -r6.wwww, r14.xyzx
    r14.xyz = ((-(r6.wwww))+(r14.xyzx)).xyz;
    // 237: mad r14.xyz, r14.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r6.wwww
    r14.xyz = ((r14.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r6.wwww)).xyz;
    // 238: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 239: mad r7.x, r7.y, l(2.000000), l(2.000000)
    r7.x = ((r7.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).x;
    // 240: div r6.w, r6.w, r7.x
    r6.w = ((r6.wwww)/(r7.xxxx)).w;
    // 241: mad r6.w, r5.w, l(5.000000), r6.w
    r6.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r6.wwww)).w;
    // 242: add_sat r6.w, r7.w, r6.w
    r6.w = (saturate((r7.wwww)+(r6.wwww))).w;
    // 243: mad r7.z, r6.w, l(-2.000000), l(3.000000)
    r7.z = ((r6.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 244: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 245: mul r6.w, r6.w, r7.z
    r6.w = ((r6.wwww)*(r7.zzzz)).w;
    // 246: log r6.w, r6.w
    r6.w = (log2(r6.wwww)).w;
    // 247: mul r6.w, r6.w, l(1.500000)
    r6.w = ((r6.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 248: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 249: mul r14.xyz, r6.wwww, r14.xyzx
    r14.xyz = ((r6.wwww)*(r14.xyzx)).xyz;
    // 250: mul r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)*(r14.xyzx)).xyz;
    // 251: mul r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)*(r14.xyzx)).xyz;
    // 252: mul r12.xyz, r4.xywx, r12.xyzx
    r12.xyz = ((r4.xywx)*(r12.xyzx)).xyz;
    // 253: mul r6.w, r7.y, l(5.000000)
    r6.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 254: mul r7.y, r7.y, r7.y
    r7.y = ((r7.yyyy)*(r7.yyyy)).y;
    // 255: mul r2.w, r2.w, r7.y
    r2.w = ((r2.wwww)*(r7.yyyy)).w;
    // 256: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 257: add r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)+(r2.wwww)).w;
    // 258: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 259: add_sat r0.w, r2.w, l(-1.000000)
    r0.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 260: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r18.xyzx, t7.xyzw, s6, r6.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r18.xyzx).xyz, (r6.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 261: mul r14.xyz, r14.xyzx, r14.wwww
    r14.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 262: mul r14.xyz, r14.xyzx, cb0[29].xyzx
    r14.xyz = ((r14.xyzx)*(source[29].xyzx)).xyz;
    // 263: mul r14.xyz, r14.xyzx, cb0[30].zzzz
    r14.xyz = ((r14.xyzx)*(source[30].zzzz)).xyz;
    // 264: mad r14.xyz, r14.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[29].wwww
    r14.xyz = ((r14.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[29].wwww)).xyz;
    // 265: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r14.xyz, -r2.wwww, r14.xyzx
    r14.xyz = ((-(r2.wwww))+(r14.xyzx)).xyz;
    // 267: mad r14.xyz, r14.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r14.xyz = ((r14.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 268: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: div r2.w, r2.w, r7.x
    r2.w = ((r2.wwww)/(r7.xxxx)).w;
    // 270: mad r2.w, r5.w, l(5.000000), r2.w
    r2.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 271: add_sat r2.w, r7.w, r2.w
    r2.w = (saturate((r7.wwww)+(r2.wwww))).w;
    // 272: mad r5.w, r2.w, l(-2.000000), l(3.000000)
    r5.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 273: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 274: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 275: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 276: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 277: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 278: mul r7.xyz, r2.wwww, r14.xyzx
    r7.xyz = ((r2.wwww)*(r14.xyzx)).xyz;
    // 279: mul r14.xyz, r7.xyzx, r10.xyzx
    r14.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 280: mad r2.w, r0.w, r5.x, r5.y
    r2.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 281: mad r2.w, r2.w, r0.w, r5.z
    r2.w = ((r2.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 282: mul r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)*(r2.wwww)).w;
    // 283: max r0.w, r0.w, r2.w
    r0.w = (max(r0.wwww,r2.wwww)).w;
    // 284: mad r5.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 285: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 286: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 287: mul r12.xyz, r2.wwww, v6.xyzx
    r12.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 288: dp3 r2.w, r12.xyzx, r8.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
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
    // 294: mad r11.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 295: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 296: mul r11.yzw, r11.yyyy, cb0[40].xxyz
    r11.yzw = ((r11.yyyy)*(source[40].xxyz)).yzw;
    // 297: mad r11.xyz, r11.xxxx, cb0[39].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[39].xyzx)+(r11.yzwy)).xyz;
    // 298: mul r11.xyz, r11.xyzx, cb0[41].wwww
    r11.xyz = ((r11.xyzx)*(source[41].wwww)).xyz;
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
    // 305: mul r5.xyz, r8.yyyy, cb0[40].xyzx
    r5.xyz = ((r8.yyyy)*(source[40].xyzx)).xyz;
    // 306: mad r5.xyz, cb0[39].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[39].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 307: mul r5.xyz, r5.xyzx, cb0[41].wwww
    r5.xyz = ((r5.xyzx)*(source[41].wwww)).xyz;
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
    // 314: dp3 r0.w, r6.xyzx, r9.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 315: mul_sat r2.w, r0.w, cb0[19].w
    r2.w = (saturate((r0.wwww)*(source[19].wwww))).w;
    // 316: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 317: mul_sat r5.x, r9.z, cb0[19].w
    r5.x = (saturate((r9.zzzz)*(source[19].wwww))).x;
    // 318: add r5.x, -r5.x, l(1.000000)
    r5.x = ((-(r5.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 319: log r5.y, r5.x
    r5.y = (log2(r5.xxxx)).y;
    // 320: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 321: mul r5.y, r5.y, cb0[20].x
    r5.y = ((r5.yyyy)*(source[20].xxxx)).y;
    // 322: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 323: mul r2.w, r2.w, r5.y
    r2.w = ((r2.wwww)*(r5.yyyy)).w;
    // 324: mul r5.yzw, cb0[8].xxyz, cb0[8].wwww
    r5.yzw = ((source[8].xxyz)*(source[8].wwww)).yzw;
    // 325: mul r5.yzw, r2.wwww, r5.yyzw
    r5.yzw = ((r2.wwww)*(r5.yyzw)).yzw;
    // 326: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 327: dp2 r2.w, cb0[9].xyxx, r6.xyxx
    r2.w = (dot((source[9].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 328: add r2.w, r2.w, cb0[21].z
    r2.w = ((r2.wwww)+(source[21].zzzz)).w;
    // 329: add_sat r2.w, r2.w, l(-0.500000)
    r2.w = (saturate((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).w;
    // 330: mul r5.yzw, r5.yyzw, r2.wwww
    r5.yzw = ((r5.yyzw)*(r2.wwww)).yzw;
    // 331: movc r5.xyz, r5.xxxx, l(0,0,0,0), r5.yzwy
    r5.xyz = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.yzwy)).xyz;
    // 332: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 333: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 334: mad r5.xyz, -r1.wwww, r5.xyzx, r2.wwww
    r5.xyz = ((-(r1.wwww))*(r5.xyzx)+(r2.wwww)).xyz;
    // 335: mad r5.xyz, cb0[21].wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((source[21].wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 336: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 337: add r6.xyz, -r5.xyzx, r1.wwww
    r6.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 338: mad r5.xyz, cb0[22].xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((source[22].xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 339: dp3 r1.w, r0.xyzx, r0.xyzx
    r1.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 340: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 341: div r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)/(r1.wwww)).xyz;
    // 342: dp3 r1.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 343: add r6.xyz, -r0.xyzx, r1.wwww
    r6.xyz = ((-(r0.xyzx))+(r1.wwww)).xyz;
    // 344: add r0.xyz, r0.xyzx, -r6.xyzx
    r0.xyz = ((r0.xyzx)+(-(r6.xyzx))).xyz;
    // 345: mul_sat r1.w, r0.w, cb0[22].y
    r1.w = (saturate((r0.wwww)*(source[22].yyyy))).w;
    // 346: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 347: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 348: mul_sat r2.w, r9.z, cb0[22].y
    r2.w = (saturate((r9.zzzz)*(source[22].yyyy))).w;
    // 349: add r5.w, -|r9.z|, l(1.000000)
    r5.w = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 350: mul r0.w, r0.w, r5.w
    r0.w = ((r0.wwww)*(r5.wwww)).w;
    // 351: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 352: add_sat r2.w, r2.w, -cb0[22].z
    r2.w = (saturate((r2.wwww)+(-(source[22].zzzz)))).w;
    // 353: log r5.w, r2.w
    r5.w = (log2(r2.wwww)).w;
    // 354: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 355: mul r5.w, r5.w, cb0[22].w
    r5.w = ((r5.wwww)*(source[22].wwww)).w;
    // 356: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 357: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 358: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 359: mul r6.xyz, cb0[14].xyzx, cb0[23].yyyy
    r6.xyz = ((source[14].xyzx)*(source[23].yyyy)).xyz;
    // 360: mul r6.xyz, r6.xyzx, cb0[24].xxxx
    r6.xyz = ((r6.xyzx)*(source[24].xxxx)).xyz;
    // 361: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 362: mad r7.xyz, r1.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r7.xyz = ((r1.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 363: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 364: mad r1.w, cb0[12].w, r1.w, l(1.000000)
    r1.w = ((source[12].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 365: mad r7.xyz, cb0[13].wwww, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((source[13].wwww)*(r7.xyzx)+(source[13].xyzx)).xyz;
    // 366: mad r0.xyz, r0.xyzx, r6.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 367: mad r0.xyz, r1.wwww, cb0[12].xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(source[12].xyzx)+(r0.xyzx)).xyz;
    // 368: mad r0.xyz, r5.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 369: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 370: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 371: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 372: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 373: mul r3.xyz, r1.wwww, cb0[15].xyzx
    r3.xyz = ((r1.wwww)*(source[15].xyzx)).xyz;
    // 374: movc r3.xyz, r0.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 375: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 376: mad r0.xyz, cb0[19].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 377: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 378: mul r1.xyz, r8.wwww, cb0[40].xyzx
    r1.xyz = ((r8.wwww)*(source[40].xyzx)).xyz;
    // 379: mad r1.xyz, r8.zzzz, cb0[39].xyzx, r1.xyzx
    r1.xyz = ((r8.zzzz)*(source[39].xyzx)+(r1.xyzx)).xyz;
    // 380: mul r1.xyz, r1.xyzx, cb0[41].wwww
    r1.xyz = ((r1.xyzx)*(source[41].wwww)).xyz;
    // 381: mul_sat r3.xyz, cb0[18].xyzx, cb0[18].wwww
    r3.xyz = (saturate((source[18].xyzx)*(source[18].wwww))).xyz;
    // 382: mul r5.xyz, r3.xyzx, r3.wwww
    r5.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 383: mul r3.xyz, r3.xyzx, cb0[28].xxxx
    r3.xyz = ((r3.xyzx)*(source[28].xxxx)).xyz;
    // 384: dp3_sat o5.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 385: mul r3.xyz, r4.zzzz, r5.xyzx
    r3.xyz = ((r4.zzzz)*(r5.xyzx)).xyz;
    // 386: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 387: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 388: mad r0.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 389: add r0.xyz, r4.xywx, r0.xyzx
    r0.xyz = ((r4.xywx)+(r0.xyzx)).xyz;
    // 390: dp3 o4.y, r4.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 391: mad o0.xyz, r2.xyzx, cb0[41].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[41].xyzx)+(r0.xyzx)).xyz;
    // 392: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 393: dp3 r0.x, r17.xyzx, r17.xyzx
    r0.x = (dot((r17.xyzx).xyz,(r17.xyzx).xyz).xxxx).x;
    // 394: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 395: mul r0.xyz, r0.xxxx, r17.xyzx
    r0.xyz = ((r0.xxxx)*(r17.xyzx)).xyz;
    // 396: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 397: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 398: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 399: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 400: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 401: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 402: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 403: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 404: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 405: ftou r0.x, cb0[38].z
    r0.x = (asfloat((uint4)(source[38].zzzz))).x;
    // 406: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 407: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 408: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 409: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 410: ret
    return output;
}

// source.character.static-map-native-214.v1 / source program 4b65c1956d0f25469fa644129f4284ec
