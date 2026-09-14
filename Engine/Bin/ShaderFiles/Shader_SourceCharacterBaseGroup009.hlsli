SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase9(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[26] = g_SourceCharacterEnvironmentColor;
        source[27] = g_SourceCharacterEnvironmentRotation;
    }
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].z=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[17].xyzw
    r1.xyzw = ((r0.xyzw)*(source[17].xyzw)).xyzw;
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
    // 15: add r0.x, -cb0[8].w, l(1.000000)
    r0.x = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 16: mul r0.x, r0.x, cb0[20].z
    r0.x = ((r0.xxxx)*(source[20].zzzz)).x;
    // 17: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 18: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 19: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: mul r0.y, cb0[8].z, l(1.500000)
    r0.y = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 21: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 22: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 23: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 24: mul r2.x, r0.y, l(0.125000)
    r2.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 25: mul r3.y, cb0[8].y, cb0[16].y
    r3.y = ((source[8].yyyy)*(source[16].yyyy)).y;
    // 26: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 27: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 28: add r0.yz, r2.xxyx, r3.xxyx
    r0.yz = ((r2.xxyx)+(r3.xxyx)).yz;
    // 29: frc r0.w, cb0[8].x
    r0.w = (frac(source[8].xxxx)).w;
    // 30: add r1.w, -r0.w, cb0[8].x
    r1.w = ((-(r0.wwww))+(source[8].xxxx)).w;
    // 31: mul r3.z, r1.w, l(0.125000)
    r3.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 32: add r0.yz, r0.yyzy, r3.zzwz
    r0.yz = ((r0.yyzy)+(r3.zzwz)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 35: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 36: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 38: mad r2.xyz, cb0[21].xxxx, r2.xyzx, r1.xyzx
    r2.xyz = ((source[21].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 39: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 41: mad r2.xyz, cb0[21].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 42: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
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
    // 51: mul r1.w, r6.y, cb0[19].y
    r1.w = ((r6.yyyy)*(source[19].yyyy)).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 55: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 56: mul r4.xyz, cb0[4].xyzx, cb0[4].wwww
    r4.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
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
    // 63: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 65: mad r3.xyz, r7.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r7.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 66: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 67: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 68: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 69: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 70: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 71: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 72: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 73: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 74: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 75: mul r4.xyz, cb0[7].xyzx, cb0[7].wwww
    r4.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 76: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 77: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 78: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 79: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 80: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 81: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 82: mul_sat r8.w, r1.w, cb2[3].w
    r8.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 83: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 84: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 85: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 86: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 87: mad r4.xyz, cb0[21].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[21].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 88: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 89: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r3.xyz, -r4.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 91: mad r3.xyz, cb0[21].yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((source[21].yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 92: mad r4.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 93: mad r9.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 94: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 95: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 96: mul r9.xyz, r2.xyzx, r3.xyzx
    r9.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 97: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: mad r2.xyz, -r3.xyzx, r2.xyzx, r1.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 99: mad r2.xyz, cb0[21].xxxx, r2.xyzx, r9.xyzx
    r2.xyz = ((source[21].xxxx)*(r2.xyzx)+(r9.xyzx)).xyz;
    // 100: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 102: mad r2.xyz, cb0[21].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 103: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 104: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 105: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 106: mul r0.w, r6.x, cb0[22].y
    r0.w = ((r6.xxxx)*(source[22].yyyy)).w;
    // 107: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 108: movc r0.w, r5.x, l(0), r0.w
    r0.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 109: add_sat r0.w, r0.w, cb0[22].z
    r0.w = (saturate((r0.wwww)+(source[22].zzzz))).w;
    // 110: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: mul r2.xyz, r1.wwww, cb0[15].xyzx
    r2.xyz = ((r1.wwww)*(source[15].xyzx)).xyz;
    // 112: mul r3.xyz, r0.xyzx, r2.xyzx
    r3.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 113: mad r0.xyz, -r2.xyzx, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r2.xyzx))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 114: mad r0.xyz, r0.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 115: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 117: mad_sat r2.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 118: mad r0.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r0.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 119: mad r3.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 120: mad r0.xyz, r0.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 121: mad r3.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 122: mad r0.xyz, r0.xyzx, r0.wwww, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 123: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 124: max r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (max(r0.xyzx,r0.wwww)).xyz;
    // 125: mov_sat r2.w, cb0[23].y
    r2.w = (saturate(source[23].yyyy)).w;
    // 126: mad r3.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r3.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 127: mul r1.w, r2.w, l(0.080000)
    r1.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 128: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 129: mad r3.xyz, r8.wwww, r3.xyzx, r1.wwww
    r3.xyz = ((r8.wwww)*(r3.xyzx)+(r1.wwww)).xyz;
    // 130: add r1.w, -cb0[24].y, cb0[24].x
    r1.w = ((-(source[24].yyyy))+(source[24].xxxx)).w;
    // 131: mad r1.w, r7.x, r1.w, cb0[24].y
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[24].yyyy)).w;
    // 132: add r2.w, -r1.w, cb0[24].w
    r2.w = ((-(r1.wwww))+(source[24].wwww)).w;
    // 133: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 134: add r2.w, -r1.w, cb0[25].y
    r2.w = ((-(r1.wwww))+(source[25].yyyy)).w;
    // 135: mad r1.w, r7.z, r2.w, r1.w
    r1.w = ((r7.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 136: mul r1.w, r6.z, r1.w
    r1.w = ((r6.zzzz)*(r1.wwww)).w;
    // 137: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 138: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: movc r1.w, r5.z, l(0), r1.w
    r1.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 140: max r1.w, r1.w, cb0[1].x
    r1.w = (max(r1.wwww,source[1].xxxx)).w;
    // 141: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 142: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 143: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 144: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 145: mul r5.xy, r5.xyxx, cb0[19].xxxx
    r5.xy = ((r5.xyxx)*(source[19].xxxx)).xy;
    // 146: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 148: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 149: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 150: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 151: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 152: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 153: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 154: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 155: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 156: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 157: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 158: mul r7.xyz, r1.wwww, v5.xyzx
    r7.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 159: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 160: deriv_rtx_coarse r8.x, r1.w
    r8.x = (ddx_coarse(r1.wwww)).x;
    // 161: deriv_rty_coarse r8.y, r1.w
    r8.y = (ddy_coarse(r1.wwww)).y;
    // 162: dp2 r2.w, r8.xyxx, r8.xyxx
    r2.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 163: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 164: mad r2.w, r2.w, l(0.300000), r8.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz)).w;
    // 165: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 166: min r8.y, r2.w, l(1.000000)
    r8.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: max r9.xyz, r3.xyzx, r2.wwww
    r9.xyz = (max(r3.xyzx,r2.wwww)).xyz;
    // 169: add r9.xyz, -r3.xyzx, r9.xyzx
    r9.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 170: mul_sat r2.w, r3.y, l(50.000000)
    r2.w = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 171: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 172: mul r10.xyz, r1.wwww, r6.xyzx
    r10.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 173: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 174: add r2.w, r10.z, l(1.000000)
    r2.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 178: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 179: mul r1.w, r1.w, cb0[2].y
    r1.w = ((r1.wwww)*(source[2].yyyy)).w;
    // 180: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 181: mad_sat r1.w, r1.w, cb0[2].w, cb0[2].z
    r1.w = (saturate((r1.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 182: mul r1.w, r1.w, cb0[25].z
    r1.w = ((r1.wwww)*(source[25].zzzz)).w;
    // 183: add_sat r8.x, -r2.w, r3.w
    r8.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 184: sample_indexable(texture2d)(float,float,float,float) r11.xy, r8.xyxx, t7.xyzw, s8
    r11.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 185: add r2.w, r0.w, r8.x
    r2.w = ((r0.wwww)+(r8.xxxx)).w;
    // 186: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 187: mul r12.xyz, r3.xyzx, r11.yyyy
    r12.xyz = ((r3.xyzx)*(r11.yyyy)).xyz;
    // 188: mad r9.xyz, r9.xyzx, r11.xxxx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xxxx)+(r12.xyzx)).xyz;
    // 189: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r11.yyyy)).w;
    // 190: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 191: mad r11.xyz, r3.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r3.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: dp3 r3.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 193: mad r3.xyz, r3.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r3.xyz = ((r3.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 194: mad r12.xyz, -r9.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 195: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 196: mul r11.xyz, r2.xyzx, r12.xyzx
    r11.xyz = ((r2.xyzx)*(r12.xyzx)).xyz;
    // 197: add r3.w, -r8.w, l(1.000000)
    r3.w = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 198: mul r11.xyz, r3.wwww, r11.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)).xyz;
    // 199: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 200: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 201: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 202: mul r13.xyz, r5.wwww, v1.xyzx
    r13.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 203: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 204: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 205: mul r14.xyz, r5.wwww, v0.xyzx
    r14.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 206: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 207: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 208: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 209: dp3 r16.y, r15.xyzx, r6.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 210: dp3 r15.y, r15.xyzx, r10.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 211: dp3 r16.x, r14.xyzx, r6.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 212: dp3 r15.x, r14.xyzx, r10.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 213: dp2 r14.z, r16.xyxx, cb0[27].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 214: mul r8.xz, cb0[27].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[27].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 215: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 216: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 217: dp2 r17.z, r15.xyxx, cb0[27].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 218: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 219: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 220: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 221: dp4 r13.x, cb0[28].xyzw, r14.xyzw
    r13.x = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 222: dp4 r13.y, cb0[29].xyzw, r14.xyzw
    r13.y = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 223: dp4 r13.z, cb0[30].xyzw, r14.xyzw
    r13.z = (dot((source[30].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 224: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 225: dp4 r18.x, cb0[31].xyzw, r15.xyzw
    r18.x = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 226: dp4 r18.y, cb0[32].xyzw, r15.xyzw
    r18.y = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 227: dp4 r18.z, cb0[33].xyzw, r15.xyzw
    r18.z = (dot((source[33].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 228: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 229: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 230: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 231: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 232: mad r13.xyz, cb0[34].xyzx, r5.wwww, r13.xyzx
    r13.xyz = ((source[34].xyzx)*(r5.wwww)+(r13.xyzx)).xyz;
    // 233: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 234: mul r13.xyz, r13.xyzx, cb0[26].xyzx
    r13.xyz = ((r13.xyzx)*(source[26].xyzx)).xyz;
    // 235: mul r13.xyz, r13.xyzx, cb0[27].zzzz
    r13.xyz = ((r13.xyzx)*(source[27].zzzz)).xyz;
    // 236: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[26].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[26].wwww)).xyz;
    // 237: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 238: add r13.xyz, -r5.wwww, r13.xyzx
    r13.xyz = ((-(r5.wwww))+(r13.xyzx)).xyz;
    // 239: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 240: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 241: mad r6.w, r8.y, l(2.000000), l(2.000000)
    r6.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 242: div r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)/(r6.wwww)).w;
    // 243: mad r5.w, r4.w, l(5.000000), r5.w
    r5.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 244: add_sat r5.w, r8.w, r5.w
    r5.w = (saturate((r8.wwww)+(r5.wwww))).w;
    // 245: mad r7.w, r5.w, l(-2.000000), l(3.000000)
    r7.w = ((r5.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 246: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 247: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 248: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 249: mul r5.w, r5.w, l(1.500000)
    r5.w = ((r5.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 250: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 251: mul r13.xyz, r5.wwww, r13.xyzx
    r13.xyz = ((r5.wwww)*(r13.xyzx)).xyz;
    // 252: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 253: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 254: mul r11.xyz, r0.xyzx, r11.xyzx
    r11.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 255: mul r5.w, r8.y, l(5.000000)
    r5.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 256: mul r7.w, r8.y, r8.y
    r7.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 257: mul r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)*(r7.wwww)).w;
    // 258: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 259: add r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)+(r2.wwww)).w;
    // 260: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 261: add_sat r0.w, r2.w, l(-1.000000)
    r0.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 262: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t8.xyzw, s7, r5.w
    r13.xyzw = g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, r17.xyz, r5.w) : 0.f;
    // 263: mul r8.xyz, r13.xyzx, r13.wwww
    r8.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 264: mul r8.xyz, r8.xyzx, cb0[26].xyzx
    r8.xyz = ((r8.xyzx)*(source[26].xyzx)).xyz;
    // 265: mul r8.xyz, r8.xyzx, cb0[27].zzzz
    r8.xyz = ((r8.xyzx)*(source[27].zzzz)).xyz;
    // 266: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[26].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[26].wwww)).xyz;
    // 267: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 268: add r8.xyz, -r2.wwww, r8.xyzx
    r8.xyz = ((-(r2.wwww))+(r8.xyzx)).xyz;
    // 269: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 270: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 271: div r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)/(r6.wwww)).w;
    // 272: mad r2.w, r4.w, l(5.000000), r2.w
    r2.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 273: add_sat r2.w, r8.w, r2.w
    r2.w = (saturate((r8.wwww)+(r2.wwww))).w;
    // 274: mad r4.w, r2.w, l(-2.000000), l(3.000000)
    r4.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 275: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 276: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 277: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 278: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 279: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 280: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 281: mul r13.xyz, r8.xyzx, r9.xyzx
    r13.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 282: mad r2.w, r0.w, r3.x, r3.y
    r2.w = ((r0.wwww)*(r3.xxxx)+(r3.yyyy)).w;
    // 283: mad r2.w, r2.w, r0.w, r3.z
    r2.w = ((r2.wwww)*(r0.wwww)+(r3.zzzz)).w;
    // 284: mul r2.w, r0.w, r2.w
    r2.w = ((r0.wwww)*(r2.wwww)).w;
    // 285: max r0.w, r0.w, r2.w
    r0.w = (max(r0.wwww,r2.wwww)).w;
    // 286: mad r3.xyz, r13.xyzx, r0.wwww, r11.xyzx
    r3.xyz = ((r13.xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 287: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 288: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 289: mul r11.xyz, r2.wwww, v6.xyzx
    r11.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 290: dp3 r2.w, r11.xyzx, r6.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 291: dp3 r4.w, -r11.xyzx, r6.xyzx
    r4.w = (dot((-(r11.xyzx)).xyz,(r6.xyzx).xyz).xxxx).w;
    // 292: dp3 r5.w, r11.xyzx, r10.xyzx
    r5.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 293: mad r6.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 294: mad r6.zw, r4.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r4.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 295: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 296: mad r10.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 297: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 298: mul r10.yzw, r10.yyyy, cb0[37].xxyz
    r10.yzw = ((r10.yyyy)*(source[37].xxyz)).yzw;
    // 299: mad r10.xyz, r10.xxxx, cb0[36].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[36].xyzx)+(r10.yzwy)).xyz;
    // 300: mul r10.xyz, r10.xyzx, cb0[38].wwww
    r10.xyz = ((r10.xyzx)*(source[38].wwww)).xyz;
    // 301: mul r10.xyz, r2.xyzx, r10.xyzx
    r10.xyz = ((r2.xyzx)*(r10.xyzx)).xyz;
    // 302: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 303: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 304: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 305: mad r0.xyz, -r0.xyzx, r8.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r8.wwww)+(r0.xyzx)).xyz;
    // 306: mad r0.xyz, r3.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 307: mul r3.xyz, r6.yyyy, cb0[37].xyzx
    r3.xyz = ((r6.yyyy)*(source[37].xyzx)).xyz;
    // 308: mad r3.xyz, cb0[36].xyzx, r6.xxxx, r3.xyzx
    r3.xyz = ((source[36].xyzx)*(r6.xxxx)+(r3.xyzx)).xyz;
    // 309: mul r3.xyz, r3.xyzx, cb0[38].wwww
    r3.xyz = ((r3.xyzx)*(source[38].wwww)).xyz;
    // 310: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 311: mul r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)*(r3.xyzx)).xyz;
    // 312: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 313: mad r0.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 314: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 315: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 316: dp3 r0.w, r5.xyzx, r7.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 317: mul_sat r2.w, r0.w, cb0[21].z
    r2.w = (saturate((r0.wwww)*(source[21].zzzz))).w;
    // 318: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 319: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 320: mul_sat r3.x, r7.z, cb0[21].z
    r3.x = (saturate((r7.zzzz)*(source[21].zzzz))).x;
    // 321: add r3.y, -|r7.z|, l(1.000000)
    r3.y = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 322: mul r0.w, r0.w, r3.y
    r0.w = ((r0.wwww)*(r3.yyyy)).w;
    // 323: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 324: add_sat r3.x, r3.x, -cb0[21].w
    r3.x = (saturate((r3.xxxx)+(-(source[21].wwww)))).x;
    // 325: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 326: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 327: mul r3.y, r3.y, cb0[22].x
    r3.y = ((r3.yyyy)*(source[22].xxxx)).y;
    // 328: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 329: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 330: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 331: mad r3.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r3.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 332: mul r2.w, r2.w, cb0[12].w
    r2.w = ((r2.wwww)*(source[12].wwww)).w;
    // 333: mad r3.xyz, cb0[13].wwww, r3.xyzx, cb0[13].xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(source[13].xyzx)).xyz;
    // 334: mad r3.xyz, r2.wwww, cb0[12].xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 335: add r2.w, cb0[0].y, cb0[0].x
    r2.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 336: add r2.w, r2.w, cb0[0].z
    r2.w = ((r2.wwww)+(source[0].zzzz)).w;
    // 337: add r4.w, -r2.w, l(1000.000000)
    r4.w = ((-(r2.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 338: mad r2.w, cb0[20].w, r4.w, r2.w
    r2.w = ((source[20].wwww)*(r4.wwww)+(r2.wwww)).w;
    // 339: mul r2.w, r2.w, l(0.010000)
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 340: mad r2.w, cb0[20].y, cb0[20].z, r2.w
    r2.w = ((source[20].yyyy)*(source[20].zzzz)+(r2.wwww)).w;
    // 341: mul r4.w, r2.w, l(3.524534)
    r4.w = ((r2.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 342: sincos null, r4.w, r4.w
    r4.w = (cos(r4.wwww)).w;
    // 343: add r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)+(r4.wwww)).w;
    // 344: mul r2.w, r2.w, l(1.328987)
    r2.w = ((r2.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 345: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 346: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 347: mad r2.w, r2.w, l(0.500000), cb0[20].x
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[20].xxxx)).w;
    // 348: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 349: mul r7.xyz, cb0[9].xyzx, cb0[19].wwww
    r7.xyz = ((source[9].xyzx)*(source[19].wwww)).xyz;
    // 350: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 351: mul r7.xyz, r2.wwww, r5.xyzx
    r7.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 352: dp3 r4.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 353: mad r5.xyz, -r2.wwww, r5.xyzx, r4.wwww
    r5.xyz = ((-(r2.wwww))*(r5.xyzx)+(r4.wwww)).xyz;
    // 354: mad r5.xyz, cb0[21].xxxx, r5.xyzx, r7.xyzx
    r5.xyz = ((source[21].xxxx)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 355: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 356: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 357: mad r5.xyz, cb0[21].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[21].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 358: mad r3.xyz, r5.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 359: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 360: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 361: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 362: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 363: mul r4.xyz, r2.wwww, cb0[14].xyzx
    r4.xyz = ((r2.wwww)*(source[14].xyzx)).xyz;
    // 364: movc r4.xyz, r0.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 365: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 366: mad r1.xyz, cb0[19].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[19].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 367: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 368: mul r3.xyz, r6.wwww, cb0[37].xyzx
    r3.xyz = ((r6.wwww)*(source[37].xyzx)).xyz;
    // 369: mad r3.xyz, r6.zzzz, cb0[36].xyzx, r3.xyzx
    r3.xyz = ((r6.zzzz)*(source[36].xyzx)+(r3.xyzx)).xyz;
    // 370: mul r3.xyz, r3.xyzx, cb0[38].wwww
    r3.xyz = ((r3.xyzx)*(source[38].wwww)).xyz;
    // 371: mul_sat r4.xyz, cb0[18].xyzx, cb0[18].wwww
    r4.xyz = (saturate((source[18].xyzx)*(source[18].wwww))).xyz;
    // 372: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 373: mul r4.xyz, r4.xyzx, cb0[25].zzzz
    r4.xyz = ((r4.xyzx)*(source[25].zzzz)).xyz;
    // 374: dp3_sat o5.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 375: mul r4.xyz, r3.wwww, r5.xyzx
    r4.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 376: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 377: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 378: mad r1.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 379: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 380: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 381: mad o0.xyz, r2.xyzx, cb0[38].xyzx, r1.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[38].xyzx)+(r1.xyzx)).xyz;
    // 382: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 383: dp3 r0.x, r16.xyzx, r16.xyzx
    r0.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 384: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 385: mul r0.xyz, r0.xxxx, r16.xyzx
    r0.xyz = ((r0.xxxx)*(r16.xyzx)).xyz;
    // 386: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 387: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 388: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 389: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 390: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 391: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 392: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 393: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 394: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 395: ftou r0.x, cb0[35].z
    r0.x = (asfloat((uint4)(source[35].zzzz))).x;
    // 396: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 397: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 398: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 399: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 400: ret
    return output;
}

// source.character.classic-parts-lower.v1 / source program 0d0eb237a166574d8753e1fa05a5ad5f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase10(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 25: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r1.z, r1.z, cb0[19].x
    r1.z = ((r1.zzzz)*(source[19].xxxx)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 30: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 31: mul r1.z, r1.z, cb0[19].y
    r1.z = ((r1.zzzz)*(source[19].yyyy)).z;
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
    // 39: mul r3.xy, r1.xzxx, cb0[18].xxxx
    r3.xy = ((r1.xzxx)*(source[18].xxxx)).xy;
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
    // 45: mad r1.xzw, cb0[18].wwww, r1.xxzw, r3.xxyz
    r1.xzw = ((source[18].wwww)*(r1.xxzw)+(r3.xxyz)).xzw;
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
    // 79: sample_l_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t4.xyzw, s4, r0.x
    r5.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r5.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 80: log r9.xyz, r5.xyzx
    r9.xyz = (log2(r5.xyzx)).xyz;
    // 81: rcp r0.x, cb0[19].z
    r0.x = (1.0/(source[19].zzzz)).x;
    // 82: mul r10.xyz, r9.xyzx, r0.xxxx
    r10.xyz = ((r9.xyzx)*(r0.xxxx)).xyz;
    // 83: mul r9.xyz, r9.xyzx, cb0[19].zzzz
    r9.xyz = ((r9.xyzx)*(source[19].zzzz)).xyz;
    // 84: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 85: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 86: mul r10.xyz, r0.xxxx, r10.xyzx
    r10.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 87: mad r9.xyz, r9.xyzx, cb0[19].zzzz, r10.xyzx
    r9.xyz = ((r9.xyzx)*(source[19].zzzz)+(r10.xyzx)).xyz;
    // 88: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 89: mul r5.xyz, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 90: add r0.x, cb0[19].z, l(1.000000)
    r0.x = ((source[19].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 92: dp3 r0.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r5.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r5.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 94: mad r5.xyz, r3.wwww, r5.xyzx, cb0[9].xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(source[9].xyzx)).xyz;
    // 95: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 96: mul r5.xyz, r5.xyzx, cb0[19].wwww
    r5.xyz = ((r5.xyzx)*(source[19].wwww)).xyz;
    // 97: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 98: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 99: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 100: dp3 r0.x, r3.xyzx, r8.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 101: mul_sat r2.w, r0.x, cb0[20].y
    r2.w = (saturate((r0.xxxx)*(source[20].yyyy))).w;
    // 102: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 104: mul_sat r3.w, r8.z, cb0[20].y
    r3.w = (saturate((r8.zzzz)*(source[20].yyyy))).w;
    // 105: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: add_sat r3.w, r3.w, -cb0[20].z
    r3.w = (saturate((r3.wwww)+(-(source[20].zzzz)))).w;
    // 107: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 108: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 109: mul r4.w, r4.w, cb0[20].w
    r4.w = ((r4.wwww)*(source[20].wwww)).w;
    // 110: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 111: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 112: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 113: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r9.xyz, -r2.xyzx, r3.wwww
    r9.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 115: mad r2.xyz, cb0[18].yyyy, r9.xyzx, r2.xyzx
    r2.xyz = ((source[18].yyyy)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 116: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 117: add r9.xyz, -r2.xyzx, r3.wwww
    r9.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 118: mad r2.xyz, cb0[18].zzzz, r9.xyzx, r2.xyzx
    r2.xyz = ((source[18].zzzz)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 119: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 120: add r9.xyz, -r2.xyzx, r3.wwww
    r9.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 121: mul r9.xyz, r9.xyzx, cb0[20].xxxx
    r9.xyz = ((r9.xyzx)*(source[20].xxxx)).xyz;
    // 122: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 123: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 124: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 125: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 126: mad r2.xyz, r3.wwww, r9.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 127: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 128: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 129: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 130: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 131: dp3 r3.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 133: mul r3.w, r3.w, cb0[21].x
    r3.w = ((r3.wwww)*(source[21].xxxx)).w;
    // 134: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 135: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 138: div r4.w, cb0[21].y, r4.w
    r4.w = ((source[21].yyyy)/(r4.wwww)).w;
    // 139: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 140: mul r9.xyz, r5.xyzx, r4.wwww
    r9.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 141: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 143: mad r0.yzw, cb0[18].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[18].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 144: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 146: mad r0.yzw, cb0[18].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[18].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 147: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 148: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 149: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 150: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 151: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 152: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 154: mad r10.xyw, cb0[18].yyyy, r11.xyxz, r10.xyxw
    r10.xyw = ((source[18].yyyy)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 155: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 157: mad r10.xyw, cb0[18].zzzz, r11.xyxz, r10.xyxw
    r10.xyw = ((source[18].zzzz)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 158: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 161: mul r10.xyw, r10.xyxw, r11.xyxz
    r10.xyw = ((r10.xyxw)*(r11.xyxz)).xyw;
    // 162: mul r0.yzw, r0.yyzw, r10.xxyw
    r0.yzw = ((r0.yyzw)*(r10.xxyw)).yzw;
    // 163: mul r5.xyz, r5.xyzx, r0.yzwy
    r5.xyz = ((r5.xyzx)*(r0.yzwy)).xyz;
    // 164: mad r2.xyz, r2.xyzx, r9.xyzx, -r5.xyzx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 165: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: mul r4.w, r4.w, cb0[21].z
    r4.w = ((r4.wwww)*(source[21].zzzz)).w;
    // 167: mad r2.xyz, r4.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r4.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 168: dp3 r4.w, r1.xzwx, r1.xzwx
    r4.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 169: sqrt r5.x, r4.w
    r5.x = (sqrt(r4.wwww)).x;
    // 170: div r1.xzw, r1.xxzw, r5.xxxx
    r1.xzw = ((r1.xxzw)/(r5.xxxx)).xzw;
    // 171: dp3 r1.x, r1.xzwx, r8.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 172: add r1.z, -|r8.z|, l(1.000000)
    r1.z = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 173: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 174: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 175: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 176: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 177: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 178: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 179: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 180: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 181: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 182: div_sat r1.x, r1.x, r4.w
    r1.x = (saturate((r1.xxxx)/(r4.wwww))).x;
    // 183: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 184: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 185: mad r1.xyz, r1.xxxx, r2.xyzx, -r0.yzwy
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r0.yzwy))).xyz;
    // 186: mad r0.yzw, r3.wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 187: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 188: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 189: mad r0.yzw, cb0[18].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[18].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 190: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 191: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 192: mad r0.yzw, cb0[18].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[18].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 193: mul r0.yzw, r11.xxyz, r0.yyzw
    r0.yzw = ((r11.xxyz)*(r0.yyzw)).yzw;
    // 194: add r1.x, -cb0[3].w, l(1.000000)
    r1.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 195: mul r1.x, r1.x, cb0[21].w
    r1.x = ((r1.xxxx)*(source[21].wwww)).x;
    // 196: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 197: sincos r1.x, null, r1.x
    r1.x = (sin(r1.xxxx)).x;
    // 198: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 199: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 200: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 201: mad r1.x, r1.x, l(0.500000), cb0[3].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 202: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 203: mul r5.x, r1.y, l(0.125000)
    r5.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 204: mul r8.y, cb0[3].y, cb0[14].y
    r8.y = ((source[3].yyyy)*(source[14].yyyy)).y;
    // 205: mov r5.y, v4.y
    r5.y = (v4.yyyy).y;
    // 206: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 207: add r1.yz, r5.xxyx, r8.xxyx
    r1.yz = ((r5.xxyx)+(r8.xxyx)).yz;
    // 208: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 209: add r3.w, -r1.w, cb0[3].x
    r3.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 210: mul r8.z, r3.w, l(0.125000)
    r8.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 211: add r1.yz, r1.yyzy, r8.zzwz
    r1.yz = ((r1.yyzy)+(r8.zzwz)).yz;
    // 212: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r1.yzyy, t5.xyzw, s5, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 213: mul r1.xyz, r1.xxxx, r5.xyzx
    r1.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 214: mul r3.w, r1.w, r5.w
    r3.w = ((r1.wwww)*(r5.wwww)).w;
    // 215: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 216: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.yzwy
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.yzwy))).xyz;
    // 217: mad r0.yzw, r3.wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 218: add r5.xyzw, v7.yzxy, cb0[0].yzxy
    r5.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 219: add r5.xyzw, r5.xyzw, -cb0[1].yzxy
    r5.xyzw = ((r5.xyzw)+(-(source[1].yzxy))).xyzw;
    // 220: add r1.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 221: add r1.xy, -r5.zwzz, r1.xyxx
    r1.xy = ((-(r5.zwzz))+(r1.xyxx)).xy;
    // 222: mad r1.xy, cb0[15].wwww, r1.xyxx, r5.zwzz
    r1.xy = ((source[15].wwww)*(r1.xyxx)+(r5.zwzz)).xy;
    // 223: mul r1.z, cb0[15].y, cb0[21].w
    r1.z = ((source[15].yyyy)*(source[21].wwww)).z;
    // 224: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 225: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 226: mul r5.y, r1.z, l(0.020000)
    r5.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 227: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 228: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 229: mul r3.w, cb0[15].x, l(0.001000)
    r3.w = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 230: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 231: mad r1.xy, r3.wwww, r1.xyxx, r5.xyxx
    r1.xy = ((r3.wwww)*(r1.xyxx)+(r5.xyxx)).xy;
    // 232: dp2 r3.w, cb0[16].xyxx, r1.xyxx
    r3.w = (dot((source[16].xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 233: dp2 r1.y, cb0[17].xyxx, r1.xyxx
    r1.y = (dot((source[17].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 234: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 235: mul r1.x, r3.w, l(0.125000)
    r1.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 236: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 237: mad r5.xyz, r5.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.yzwy
    r5.xyz = ((r5.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.yzwy))).xyz;
    // 238: mul r1.x, r5.w, l(0.900000)
    r1.x = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 239: mad r5.xyz, r1.xxxx, r5.xyzx, r0.yzwy
    r5.xyz = ((r1.xxxx)*(r5.xyzx)+(r0.yzwy)).xyz;
    // 240: mul_sat r1.xyz, r1.zzzz, r5.xyzx
    r1.xyz = (saturate((r1.zzzz)*(r5.xyzx))).xyz;
    // 241: mad r5.xyz, cb0[15].zzzz, r1.xyzx, -r0.yzwy
    r5.xyz = ((source[15].zzzz)*(r1.xyzx)+(-(r0.yzwy))).xyz;
    // 242: mul r1.xyz, r1.xyzx, cb0[15].zzzz
    r1.xyz = ((r1.xyzx)*(source[15].zzzz)).xyz;
    // 243: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 244: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 245: mad r0.yzw, r1.xxxx, r5.xxyz, r0.yyzw
    r0.yzw = ((r1.xxxx)*(r5.xxyz)+(r0.yyzw)).yzw;
    // 246: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 247: add r1.x, r2.w, -r10.z
    r1.x = ((r2.wwww)+(-(r10.zzzz))).x;
    // 248: mad r5.xyz, r2.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r5.xyz = ((r2.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 249: mad r5.xyz, cb0[12].wwww, r5.xyzx, cb0[12].xyzx
    r5.xyz = ((source[12].wwww)*(r5.xyzx)+(source[12].xyzx)).xyz;
    // 250: mad r1.x, cb0[11].w, r1.x, r10.z
    r1.x = ((source[11].wwww)*(r1.xxxx)+(r10.zzzz)).x;
    // 251: mad r1.xyz, r1.xxxx, cb0[11].xyzx, r5.xyzx
    r1.xyz = ((r1.xxxx)*(source[11].xyzx)+(r5.xyzx)).xyz;
    // 252: mul r5.xyz, r2.xyzx, r1.wwww
    r5.xyz = ((r2.xyzx)*(r1.wwww)).xyz;
    // 253: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 254: mad r2.xyz, -r1.wwww, r2.xyzx, r2.wwww
    r2.xyz = ((-(r1.wwww))*(r2.xyzx)+(r2.wwww)).xyz;
    // 255: mad r2.xyz, cb0[18].yyyy, r2.xyzx, r5.xyzx
    r2.xyz = ((source[18].yyyy)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 256: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 257: add r5.xyz, -r2.xyzx, r1.wwww
    r5.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 258: mad r2.xyz, cb0[18].zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((source[18].zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 259: mad r1.xyz, r2.xyzx, r11.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 260: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 261: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 262: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 263: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 264: mul r2.xyz, r1.wwww, cb0[13].xyzx
    r2.xyz = ((r1.wwww)*(source[13].xyzx)).xyz;
    // 265: movc r2.xyz, r0.xxxx, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 266: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 267: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 268: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 269: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 270: mul r2.xyz, r0.xxxx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 271: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 272: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 273: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 274: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 275: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 276: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 277: mul r3.yzw, r3.yyyy, cb0[23].xxyz
    r3.yzw = ((r3.yyyy)*(source[23].xxyz)).yzw;
    // 278: mad r3.xyz, r3.xxxx, cb0[22].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[22].xyzx)+(r3.yzwy)).xyz;
    // 279: mul r3.xyz, r3.xyzx, cb0[24].wwww
    r3.xyz = ((r3.xyzx)*(source[24].wwww)).xyz;
    // 280: mad r1.xyz, r3.xyzx, r0.yzwy, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 281: mul r3.xyz, r0.yzwy, r3.xyzx
    r3.xyz = ((r0.yzwy)*(r3.xyzx)).xyz;
    // 282: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 283: mad o0.xyz, r0.yzwy, cb0[24].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[24].xyzx)+(r1.xyzx)).xyz;
    // 284: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 285: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 286: dp3 r0.x, r4.xyzx, r2.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 287: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 288: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 289: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 290: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 291: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 292: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 293: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 294: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 295: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 296: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 297: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 298: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 299: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 300: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 301: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 302: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 303: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 304: ret
    return output;
}

// source.character.classic-parts-upper.v1 / source program 4c19dc6bca8b0240b4f4337cf9888556
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase11(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 26: add r1.w, -cb0[20].y, cb0[20].x
    r1.w = ((-(source[20].yyyy))+(source[20].xxxx)).w;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 28: mad r1.w, r3.w, r1.w, cb0[20].y
    r1.w = ((r3.wwww)*(r1.wwww)+(source[20].yyyy)).w;
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
    // 34: add r1.w, -r1.z, cb0[21].y
    r1.w = ((-(r1.zzzz))+(source[21].yyyy)).w;
    // 35: mad r1.z, r3.w, r1.w, r1.z
    r1.z = ((r3.wwww)*(r1.wwww)+(r1.zzzz)).z;
    // 36: mul r1.z, r1.z, cb0[21].z
    r1.z = ((r1.zzzz)*(source[21].zzzz)).z;
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
    // 48: mul r5.xy, r4.xyxx, cb0[19].xxxx
    r5.xy = ((r4.xyxx)*(source[19].xxxx)).xy;
    // 49: mad r4.xy, cb0[19].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[19].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 50: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 51: mad r1.xzw, r3.wwww, r4.xxyz, r5.xxyz
    r1.xzw = ((r3.wwww)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 52: add r4.xyz, -r1.xzwx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xzwx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[21].xxxx, r4.xyzx, r1.xzwx
    r4.xyz = ((source[21].xxxx)*(r4.xyzx)+(r1.xzwx)).xyz;
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
    // 89: rcp r0.x, cb0[21].w
    r0.x = (1.0/(source[21].wwww)).x;
    // 90: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 91: mul r10.xyz, r10.xyzx, cb0[21].wwww
    r10.xyz = ((r10.xyzx)*(source[21].wwww)).xyz;
    // 92: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 93: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 94: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 95: mad r10.xyz, r10.xyzx, cb0[21].wwww, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[21].wwww)+(r11.xyzx)).xyz;
    // 96: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 97: mul r8.xyz, r8.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 98: add r0.x, cb0[21].w, l(1.000000)
    r0.x = ((source[21].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 100: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r8.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r8.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 102: mad r8.xyz, r4.wwww, r8.xyzx, cb0[10].xyzx
    r8.xyz = ((r4.wwww)*(r8.xyzx)+(source[10].xyzx)).xyz;
    // 103: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 104: mul r8.xyz, r8.xyzx, cb0[22].xxxx
    r8.xyz = ((r8.xyzx)*(source[22].xxxx)).xyz;
    // 105: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 106: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 107: mad r2.yzw, cb0[20].zzzz, r10.xxyz, r2.xxyz
    r2.yzw = ((source[20].zzzz)*(r10.xxyz)+(r2.xxyz)).yzw;
    // 108: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 110: mad r2.yzw, cb0[20].wwww, r10.xxyz, r2.yyzw
    r2.yzw = ((source[20].wwww)*(r10.xxyz)+(r2.yyzw)).yzw;
    // 111: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 113: mul r10.xyz, r10.xyzx, cb0[22].yyyy
    r10.xyz = ((r10.xyzx)*(source[22].yyyy)).xyz;
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
    // 126: add r2.w, -cb0[23].z, cb0[23].y
    r2.w = ((-(source[23].zzzz))+(source[23].yyyy)).w;
    // 127: mad r2.w, r3.w, r2.w, cb0[23].z
    r2.w = ((r3.wwww)*(r2.wwww)+(source[23].zzzz)).w;
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
    // 133: div r2.w, cb0[23].w, r2.w
    r2.w = ((source[23].wwww)/(r2.wwww)).w;
    // 134: dp3 r4.w, r1.xzwx, r1.xzwx
    r4.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 135: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 136: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 137: dp3 r4.w, r1.xzwx, r4.xyzx
    r4.w = (dot((r1.xzwx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 138: mul_sat r5.w, r4.w, cb0[22].z
    r5.w = (saturate((r4.wwww)*(source[22].zzzz))).w;
    // 139: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mul_sat r6.w, r4.z, cb0[22].z
    r6.w = (saturate((r4.zzzz)*(source[22].zzzz))).w;
    // 142: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: add_sat r6.w, r6.w, -cb0[22].w
    r6.w = (saturate((r6.wwww)+(-(source[22].wwww)))).w;
    // 144: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 145: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 146: mul r7.w, r7.w, cb0[23].x
    r7.w = ((r7.wwww)*(source[23].xxxx)).w;
    // 147: exp r7.w, r7.w
    r7.w = (exp2(r7.wwww)).w;
    // 148: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 149: movc r5.w, r6.w, l(0), r5.w
    r5.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 150: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 151: mul r10.xyz, r8.xyzx, r2.wwww
    r10.xyz = ((r8.xyzx)*(r2.wwww)).xyz;
    // 152: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 154: mad r0.yzw, cb0[20].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[20].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 155: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 157: mad r0.yzw, cb0[20].wwww, r11.xxyz, r0.yyzw
    r0.yzw = ((source[20].wwww)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 158: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 159: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 160: mad r11.xyz, r3.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 161: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 162: mad r11.xyz, r3.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 163: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 165: mad r11.xyz, cb0[20].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[20].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 166: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 167: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 168: mad r11.xyz, cb0[20].wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((source[20].wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 169: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 171: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 172: mul r13.xyz, r11.xyzx, r12.xyzx
    r13.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 173: mad r11.xyz, -r11.xyzx, r12.xyzx, cb0[9].xyzx
    r11.xyz = ((-(r11.xyzx))*(r12.xyzx)+(source[9].xyzx)).xyz;
    // 174: mad r3.xyw, r3.wwww, r11.xyxz, r13.xyxz
    r3.xyw = ((r3.wwww)*(r11.xyxz)+(r13.xyxz)).xyw;
    // 175: mul r0.yzw, r0.yyzw, r3.xxyw
    r0.yzw = ((r0.yyzw)*(r3.xxyw)).yzw;
    // 176: mul r3.xyw, r8.xyxz, r0.yzyw
    r3.xyw = ((r8.xyxz)*(r0.yzyw)).xyw;
    // 177: mad r2.xyz, r2.xyzx, r10.xyzx, -r3.xywx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r3.xywx))).xyz;
    // 178: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: mul r2.w, r2.w, cb0[24].x
    r2.w = ((r2.wwww)*(source[24].xxxx)).w;
    // 180: mad r2.xyz, r2.wwww, r2.xyzx, r3.xywx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r3.xywx)).xyz;
    // 181: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 182: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 183: div r3.xyw, r9.xyxz, r3.xxxx
    r3.xyw = ((r9.xyxz)/(r3.xxxx)).xyw;
    // 184: dp3 r3.x, r3.xywx, r4.xyzx
    r3.x = (dot((r3.xywx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 185: add r3.y, -|r4.z|, l(1.000000)
    r3.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: mul r3.y, r4.w, r3.y
    r3.y = ((r4.wwww)*(r3.yyyy)).y;
    // 187: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 188: mul r3.w, |r3.x|, |r3.x|
    r3.w = ((abs(r3.xxxx))*(abs(r3.xxxx))).w;
    // 189: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 190: mul r3.w, r3.w, |r3.x|
    r3.w = ((r3.wwww)*(abs(r3.xxxx))).w;
    // 191: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 192: movc r3.x, r3.x, l(0), r3.w
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).x;
    // 193: add r3.w, r3.x, l(-0.027778)
    r3.w = ((r3.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 194: mad r3.x, r3.x, r3.w, l(0.027778)
    r3.x = ((r3.xxxx)*(r3.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 195: div_sat r2.w, r3.x, r2.w
    r2.w = (saturate((r3.xxxx)/(r2.wwww))).w;
    // 196: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 197: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 198: mad r4.xyz, r1.yyyy, r2.xyzx, -r0.yzwy
    r4.xyz = ((r1.yyyy)*(r2.xyzx)+(-(r0.yzwy))).xyz;
    // 199: mad r0.xyz, r0.xxxx, r4.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r4.xyzx)+(r0.yzwy)).xyz;
    // 200: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 201: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 202: mad r0.xyz, cb0[20].zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 203: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 204: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 205: mad r0.xyz, cb0[20].wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((source[20].wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 206: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 207: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 208: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 209: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 210: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 211: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 212: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 213: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 214: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 215: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 216: mul r4.x, r1.y, l(0.125000)
    r4.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 217: mul r8.y, cb0[3].y, cb0[15].y
    r8.y = ((source[3].yyyy)*(source[15].yyyy)).y;
    // 218: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 219: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 220: add r3.xw, r4.xxxy, r8.xxxy
    r3.xw = ((r4.xxxy)+(r8.xxxy)).xw;
    // 221: frc r1.y, cb0[3].x
    r1.y = (frac(source[3].xxxx)).y;
    // 222: add r2.w, -r1.y, cb0[3].x
    r2.w = ((-(r1.yyyy))+(source[3].xxxx)).w;
    // 223: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 224: add r3.xw, r3.xxxw, r8.zzzw
    r3.xw = ((r3.xxxw)+(r8.zzzw)).xw;
    // 225: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xwxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 226: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 227: mul r0.w, r1.y, r4.w
    r0.w = ((r1.yyyy)*(r4.wwww)).w;
    // 228: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 229: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 230: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 231: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 232: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 233: add r3.xw, -r4.xxxy, l(1.000000, 0.000000, 0.000000, 1.000000)
    r3.xw = ((-(r4.xxxy))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // 234: add r3.xw, -r4.zzzw, r3.xxxw
    r3.xw = ((-(r4.zzzw))+(r3.xxxw)).xw;
    // 235: mad r3.xw, cb0[16].wwww, r3.xxxw, r4.zzzw
    r3.xw = ((source[16].wwww)*(r3.xxxw)+(r4.zzzw)).xw;
    // 236: mul r0.w, cb0[16].y, cb0[24].y
    r0.w = ((source[16].yyyy)*(source[24].yyyy)).w;
    // 237: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 238: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 239: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 240: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 241: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 242: mul r2.w, cb0[16].x, l(0.001000)
    r2.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 243: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 244: mad r3.xw, r2.wwww, r3.xxxw, r4.xxxy
    r3.xw = ((r2.wwww)*(r3.xxxw)+(r4.xxxy)).xw;
    // 245: dp2 r2.w, cb0[17].xyxx, r3.xwxx
    r2.w = (dot((source[17].xyxx).xy,(r3.xwxx).xy).xxxx).w;
    // 246: dp2 r4.y, cb0[18].xyxx, r3.xwxx
    r4.y = (dot((source[18].xyxx).xy,(r3.xwxx).xy).xxxx).y;
    // 247: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 248: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 249: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 250: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 251: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 252: mad r4.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 253: mul_sat r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = (saturate((r0.wwww)*(r4.xyzx))).xyz;
    // 254: mad r8.xyz, cb0[16].zzzz, r4.xyzx, -r0.xyzx
    r8.xyz = ((source[16].zzzz)*(r4.xyzx)+(-(r0.xyzx))).xyz;
    // 255: mul r4.xyz, r4.xyzx, cb0[16].zzzz
    r4.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 256: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 257: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 258: mad r0.xyz, r0.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 259: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 260: add r0.w, -r3.z, r5.w
    r0.w = ((-(r3.zzzz))+(r5.wwww)).w;
    // 261: mad r4.xyz, r5.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r4.xyz = ((r5.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 262: mad r4.xyz, cb0[13].wwww, r4.xyzx, cb0[13].xyzx
    r4.xyz = ((source[13].wwww)*(r4.xyzx)+(source[13].xyzx)).xyz;
    // 263: mad r0.w, cb0[12].w, r0.w, r3.z
    r0.w = ((source[12].wwww)*(r0.wwww)+(r3.zzzz)).w;
    // 264: mad r3.xzw, r0.wwww, cb0[12].xxyz, r4.xxyz
    r3.xzw = ((r0.wwww)*(source[12].xxyz)+(r4.xxyz)).xzw;
    // 265: mul r4.xyz, r2.xyzx, r1.yyyy
    r4.xyz = ((r2.xyzx)*(r1.yyyy)).xyz;
    // 266: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 267: mad r2.xyz, -r1.yyyy, r2.xyzx, r0.wwww
    r2.xyz = ((-(r1.yyyy))*(r2.xyzx)+(r0.wwww)).xyz;
    // 268: mad r2.xyz, cb0[20].zzzz, r2.xyzx, r4.xyzx
    r2.xyz = ((source[20].zzzz)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 269: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 270: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 271: mad r2.xyz, cb0[20].wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((source[20].wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 272: mad r2.xyz, r2.xyzx, r12.xyzx, r3.xzwx
    r2.xyz = ((r2.xyzx)*(r12.xyzx)+(r3.xzwx)).xyz;
    // 273: log r0.w, |r3.y|
    r0.w = (log2(abs(r3.yyyy))).w;
    // 274: lt r1.y, |r3.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 275: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 276: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 277: mul r3.xyz, r0.wwww, cb0[14].xyzx
    r3.xyz = ((r0.wwww)*(source[14].xyzx)).xyz;
    // 278: movc r3.xyz, r1.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 279: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 280: add r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)+(source[2].xyzx)).xyz;
    // 281: dp3 r0.w, r1.xzwx, r1.xzwx
    r0.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 282: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 283: mul r1.xyz, r0.wwww, r1.xzwx
    r1.xyz = ((r0.wwww)*(r1.xzwx)).xyz;
    // 284: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 285: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 286: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 287: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 288: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 289: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 290: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 291: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 292: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 293: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 294: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 295: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 296: mad o0.xyz, r0.xyzx, cb0[27].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[27].xyzx)+(r2.xyzx)).xyz;
    // 297: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 298: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 299: dp3 r0.x, r6.xyzx, r1.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 300: dp3 r0.z, r5.xyzx, r1.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 301: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 302: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 303: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 304: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 305: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 306: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 307: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 308: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 309: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 310: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 311: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 312: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 313: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 314: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 315: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 316: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 317: ret
    return output;
}

// source.character.classic-head-legacy.v1 / source program 2c4d9e7c741f82438b640deecaa7ac52
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase12(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[13].x=(g_SourceCharacterTime.xxxx).x;
    source[13].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[13].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[13].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0;
    // 1: add r0.x, -cb0[7].w, l(1.000000)
    r0.x = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: mul r0.x, r0.x, cb0[13].x
    r0.x = ((r0.xxxx)*(source[13].xxxx)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: mul r0.y, cb0[7].z, l(1.500000)
    r0.y = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 7: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 8: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 9: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 10: mul r1.x, r0.y, l(0.125000)
    r1.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 11: mul r2.y, cb0[7].y, cb0[8].y
    r2.y = ((source[7].yyyy)*(source[8].yyyy)).y;
    // 12: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 13: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 14: add r0.yz, r1.xxyx, r2.xxyx
    r0.yz = ((r1.xxyx)+(r2.xxyx)).yz;
    // 15: frc r0.w, cb0[7].x
    r0.w = (frac(source[7].xxxx)).w;
    // 16: add r1.x, -r0.w, cb0[7].x
    r1.x = ((-(r0.wwww))+(source[7].xxxx)).x;
    // 17: mul r2.z, r1.x, l(0.125000)
    r2.z = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 18: add r0.yz, r0.yyzy, r2.zzwz
    r0.yz = ((r0.yyzy)+(r2.zzwz)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 20: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 21: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 22: mad r1.xy, -cb0[6].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[6].zwzz
    r1.xy = ((-(source[6].xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(source[6].zwzz)).xy;
    // 23: div r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)/(source[6].xyxx)).xy;
    // 24: div r1.zw, l(0.000000, 0.000000, 1024.000000, 1024.000000), cb0[6].xxxy
    r1.zw = ((float4(0.000000,0.000000,1024.000000,1024.000000))/(source[6].xxxy)).zw;
    // 25: mad r1.xy, v4.xyxx, r1.zwzz, -r1.xyxx
    r1.xy = ((v4.xyxx)*(r1.zwzz)+(-(r1.xyxx))).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 28: mad r1.xyz, -r2.xyzx, cb0[5].xyzx, r1.xyzx
    r1.xyz = ((-(r2.xyzx))*(source[5].xyzx)+(r1.xyzx)).xyz;
    // 29: mul r3.xyz, r2.xyzx, cb0[5].xyzx
    r3.xyz = ((r2.xyzx)*(source[5].xyzx)).xyz;
    // 30: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 31: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t3.xzwy, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).w;
    // 33: mad r1.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 34: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 35: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 36: add r1.xyzw, v7.yzxy, cb0[0].yzxy
    r1.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 37: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 38: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 39: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 40: mad r1.xy, cb0[9].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[9].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 41: mul r0.w, cb0[9].y, cb0[13].x
    r0.w = ((source[9].yyyy)*(source[13].xxxx)).w;
    // 42: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 43: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 44: mul r2.y, r0.w, l(0.020000)
    r2.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 45: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 47: mul r1.z, cb0[9].x, l(0.001000)
    r1.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 48: mov r2.x, l(0)
    r2.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 49: mad r1.xy, r1.zzzz, r1.xyxx, r2.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r2.xyxx)).xy;
    // 50: dp2 r1.z, cb0[10].xyxx, r1.xyxx
    r1.z = (dot((source[10].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 51: dp2 r1.y, cb0[11].xyxx, r1.xyxx
    r1.y = (dot((source[11].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 52: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 53: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 56: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 57: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 58: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 59: mad r2.xyz, cb0[9].zzzz, r1.xyzx, -r0.xyzx
    r2.xyz = ((source[9].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 60: mul r1.xyz, r1.xyzx, cb0[9].zzzz
    r1.xyz = ((r1.xyzx)*(source[9].zzzz)).xyz;
    // 61: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 63: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 64: mul r0.xyz, r0.xyzx, cb0[12].xyzx
    r0.xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 65: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 67: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 68: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 69: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 70: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 71: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 72: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 73: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 74: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 75: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 76: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 77: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 78: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 79: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 80: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: add r2.x, -|r0.w|, l(1.000000)
    r2.x = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 82: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 84: mad r2.xyz, r1.wwww, cb0[4].xyzx, -cb0[4].xyzx
    r2.xyz = ((r1.wwww)*(source[4].xyzx)+(-(source[4].xyzx))).xyz;
    // 85: mad r2.xyz, cb0[4].wwww, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((source[4].wwww)*(r2.xyzx)+(source[4].xyzx)).xyz;
    // 86: mad r2.xyz, r0.wwww, cb0[3].xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(source[3].xyzx)+(r2.xyzx)).xyz;
    // 87: add r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)+(source[2].xyzx)).xyz;
    // 88: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 91: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 92: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 93: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 94: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 95: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 97: mul r3.yzw, r3.yyyy, cb0[15].xxyz
    r3.yzw = ((r3.yyyy)*(source[15].xxyz)).yzw;
    // 98: mad r3.xyz, r3.xxxx, cb0[14].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[14].xyzx)+(r3.yzwy)).xyz;
    // 99: mul r3.xyz, r3.xyzx, cb0[16].wwww
    r3.xyz = ((r3.xyzx)*(source[16].wwww)).xyz;
    // 100: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 101: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 102: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 103: mad o0.xyz, r0.xyzx, cb0[16].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[16].xyzx)+(r2.xyzx)).xyz;
    // 104: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 105: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 106: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 107: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 108: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 109: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 110: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 111: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 112: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 113: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 114: dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 115: dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 116: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 117: dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 118: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 119: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 120: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 121: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 122: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 123: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 124: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 125: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 126: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 127: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 128: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 129: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 130: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 131: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 132: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 133: ret
    return output;
}

// source.character.classic-armor-emissive.v1 / source program e2ef05d6a3dc5b4bb17bdc3a5872c88f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase13(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].x=(g_SourceCharacterTime.xxxx).x;
    source[24].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[24].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[24].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 34: mul r0.w, r0.w, cb0[21].x
    r0.w = ((r0.wwww)*(source[21].xxxx)).w;
    // 35: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 36: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 38: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 39: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
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
    // 47: mul r3.xy, r0.ywyy, cb0[20].xxxx
    r3.xy = ((r0.ywyy)*(source[20].xxxx)).xy;
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
    // 53: mad r4.xyz, cb0[20].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
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
    // 89: rcp r1.w, cb0[21].z
    r1.w = (1.0/(source[21].zzzz)).w;
    // 90: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 91: mul r6.xyz, r6.xyzx, cb0[21].zzzz
    r6.xyz = ((r6.xyzx)*(source[21].zzzz)).xyz;
    // 92: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 93: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 94: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 95: mad r6.xyz, r6.xyzx, cb0[21].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[21].zzzz)+(r10.xyzx)).xyz;
    // 96: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 97: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 98: add r1.w, cb0[21].z, l(1.000000)
    r1.w = ((source[21].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 100: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r6.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r6.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 102: mad r6.xyz, r2.wwww, r6.xyzx, cb0[9].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[9].xyzx)).xyz;
    // 103: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 104: mul r0.xyw, r0.xyxw, cb0[21].wwww
    r0.xyw = ((r0.xyxw)*(source[21].wwww)).xyw;
    // 105: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 106: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 107: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 108: dp3 r1.w, r3.xyzx, r4.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 109: mul_sat r2.w, r1.w, cb0[22].y
    r2.w = (saturate((r1.wwww)*(source[22].yyyy))).w;
    // 110: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul_sat r3.w, r4.z, cb0[22].y
    r3.w = (saturate((r4.zzzz)*(source[22].yyyy))).w;
    // 113: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: add_sat r3.w, r3.w, -cb0[22].z
    r3.w = (saturate((r3.wwww)+(-(source[22].zzzz)))).w;
    // 115: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 116: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 117: mul r4.w, r4.w, cb0[22].w
    r4.w = ((r4.wwww)*(source[22].wwww)).w;
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
    // 123: mad r2.xyz, cb0[20].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 124: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 126: mad r2.xyz, cb0[20].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 127: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 129: mul r6.xyz, r6.xyzx, cb0[22].xxxx
    r6.xyz = ((r6.xyzx)*(source[22].xxxx)).xyz;
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
    // 141: mul r3.w, r3.w, cb0[23].x
    r3.w = ((r3.wwww)*(source[23].xxxx)).w;
    // 142: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 143: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: div r4.w, cb0[23].y, r4.w
    r4.w = ((source[23].yyyy)/(r4.wwww)).w;
    // 147: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 148: mul r6.xyz, r0.xywx, r4.wwww
    r6.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 149: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 151: mad r1.xyz, cb0[20].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 152: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 154: mad r1.xyz, cb0[20].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
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
    // 160: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 162: mad r10.xyw, cb0[20].yyyy, r11.xyxz, r10.xyxw
    r10.xyw = ((source[20].yyyy)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 163: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 165: mad r10.xyw, cb0[20].zzzz, r11.xyxz, r10.xyxw
    r10.xyw = ((source[20].zzzz)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 166: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 169: mul r10.xyw, r10.xyxw, r11.xyxz
    r10.xyw = ((r10.xyxw)*(r11.xyxz)).xyw;
    // 170: mul r1.xyz, r1.xyzx, r10.xywx
    r1.xyz = ((r1.xyzx)*(r10.xywx)).xyz;
    // 171: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 172: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 173: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: mul r4.w, r4.w, cb0[23].z
    r4.w = ((r4.wwww)*(source[23].zzzz)).w;
    // 175: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 176: dp3 r2.x, r9.xyzx, r9.xyzx
    r2.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 177: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 178: div r6.xyz, r9.xyzx, r2.yyyy
    r6.xyz = ((r9.xyzx)/(r2.yyyy)).xyz;
    // 179: dp3 r2.y, r6.xyzx, r4.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 180: add r2.z, -|r4.z|, l(1.000000)
    r2.z = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 181: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 182: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 183: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 184: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 185: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 186: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 187: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 188: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 189: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 190: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 191: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 192: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 193: mad r2.xyz, r0.zzzz, r0.xywx, -r1.xyzx
    r2.xyz = ((r0.zzzz)*(r0.xywx)+(-(r1.xyzx))).xyz;
    // 194: mad r1.xyz, r3.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 195: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r1.xyz, cb0[20].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 198: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 199: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 200: mad r1.xyz, cb0[20].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 201: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 202: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 203: mul r0.z, r0.z, cb0[24].x
    r0.z = ((r0.zzzz)*(source[24].xxxx)).z;
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
    // 216: frc r2.z, cb0[3].x
    r2.z = (frac(source[3].xxxx)).z;
    // 217: add r3.w, -r2.z, cb0[3].x
    r3.w = ((-(r2.zzzz))+(source[3].xxxx)).w;
    // 218: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 219: add r2.xy, r2.xyxx, r4.zwzz
    r2.xy = ((r2.xyxx)+(r4.zwzz)).xy;
    // 220: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 221: mul r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = ((r0.zzzz)*(r4.xyzx)).xyz;
    // 222: mul r0.z, r2.z, r4.w
    r0.z = ((r2.zzzz)*(r4.wwww)).z;
    // 223: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 225: mad r1.xyz, r0.zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 226: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 227: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 228: add r2.yz, -r4.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r2.yz = ((-(r4.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 229: add r2.yz, -r4.zzwz, r2.yyzy
    r2.yz = ((-(r4.zzwz))+(r2.yyzy)).yz;
    // 230: mad r2.yz, cb0[16].wwww, r2.yyzy, r4.zzwz
    r2.yz = ((source[16].wwww)*(r2.yyzy)+(r4.zzwz)).yz;
    // 231: mul r0.z, cb0[16].y, cb0[24].x
    r0.z = ((source[16].yyyy)*(source[24].xxxx)).z;
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
    // 239: mad r2.yz, r3.wwww, r2.yyzy, r4.xxyx
    r2.yz = ((r3.wwww)*(r2.yyzy)+(r4.xxyx)).yz;
    // 240: dp2 r3.w, cb0[17].xyxx, r2.yzyy
    r3.w = (dot((source[17].xyxx).xy,(r2.yzyy).xy).xxxx).w;
    // 241: dp2 r4.y, cb0[18].xyxx, r2.yzyy
    r4.y = (dot((source[18].xyxx).xy,(r2.yzyy).xy).xxxx).y;
    // 242: frc r2.y, r3.w
    r2.y = (frac(r3.wwww)).y;
    // 243: mul r4.x, r2.y, l(0.125000)
    r4.x = ((r2.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 244: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t6.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 245: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 246: mul r2.y, r4.w, l(0.900000)
    r2.y = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 247: mad r4.xyz, r2.yyyy, r4.xyzx, r1.xyzx
    r4.xyz = ((r2.yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 248: mul_sat r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = (saturate((r0.zzzz)*(r4.xyzx))).xyz;
    // 249: mad r6.xyz, cb0[16].zzzz, r4.xyzx, -r1.xyzx
    r6.xyz = ((source[16].zzzz)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 250: mul r4.xyz, r4.xyzx, cb0[16].zzzz
    r4.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 251: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 252: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 253: mad r1.xyz, r0.zzzz, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 254: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 255: add r0.z, r2.w, -r10.z
    r0.z = ((r2.wwww)+(-(r10.zzzz))).z;
    // 256: mad r2.yzw, r2.wwww, cb0[13].xxyz, -cb0[13].xxyz
    r2.yzw = ((r2.wwww)*(source[13].xxyz)+(-(source[13].xxyz))).yzw;
    // 257: mad r2.yzw, cb0[13].wwww, r2.yyzw, cb0[13].xxyz
    r2.yzw = ((source[13].wwww)*(r2.yyzw)+(source[13].xxyz)).yzw;
    // 258: mad r0.z, cb0[12].w, r0.z, r10.z
    r0.z = ((source[12].wwww)*(r0.zzzz)+(r10.zzzz)).z;
    // 259: mad r2.yzw, r0.zzzz, cb0[12].xxyz, r2.yyzw
    r2.yzw = ((r0.zzzz)*(source[12].xxyz)+(r2.yyzw)).yzw;
    // 260: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t7.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 261: mul r6.xyz, cb0[11].xyzx, cb0[23].wwww
    r6.xyz = ((source[11].xyzx)*(source[23].wwww)).xyz;
    // 262: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 263: mad r0.xyz, r2.xxxx, r0.xywx, r4.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xywx)+(r4.xyzx)).xyz;
    // 264: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 265: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 266: mad r0.xyz, cb0[20].yyyy, r4.xyzx, r0.xyzx
    r0.xyz = ((source[20].yyyy)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 267: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 268: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 269: mad r0.xyz, cb0[20].zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 270: mad r0.xyz, r0.xyzx, r11.xyzx, r2.yzwy
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 271: log r0.w, |r1.w|
    r0.w = (log2(abs(r1.wwww))).w;
    // 272: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
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
    // 288: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 289: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 290: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 291: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 292: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 293: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 294: mad o0.xyz, r1.xyzx, cb0[27].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[27].xyzx)+(r0.xyzx)).xyz;
    // 295: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
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

// source.character.classic-weapon-emissive.v1 / source program 1edd92ae9fb0e24088fc588b174bbb2b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase14(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[19]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[25].z=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 155: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 156: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 157: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 158: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 159: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 160: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 162: mad r10.xyw, cb0[21].yyyy, r11.xyxz, r10.xyxw
    r10.xyw = ((source[21].yyyy)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 163: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 165: mad r10.xyw, cb0[21].zzzz, r11.xyxz, r10.xyxw
    r10.xyw = ((source[21].zzzz)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 166: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 169: mul r10.xyw, r10.xyxw, r11.xyxz
    r10.xyw = ((r10.xyxw)*(r11.xyxz)).xyw;
    // 170: mul r1.xyz, r1.xyzx, r10.xywx
    r1.xyz = ((r1.xyzx)*(r10.xywx)).xyz;
    // 171: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 172: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 173: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: mul r4.w, r4.w, cb0[24].z
    r4.w = ((r4.wwww)*(source[24].zzzz)).w;
    // 175: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 176: dp3 r2.x, r9.xyzx, r9.xyzx
    r2.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 177: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 178: div r6.xyz, r9.xyzx, r2.yyyy
    r6.xyz = ((r9.xyzx)/(r2.yyyy)).xyz;
    // 179: dp3 r2.y, r6.xyzx, r4.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 180: add r2.z, -|r4.z|, l(1.000000)
    r2.z = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 181: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 182: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 183: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 184: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 185: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 186: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 187: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 188: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 189: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 190: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 191: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 192: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 193: mad r2.xyz, r0.zzzz, r0.xywx, -r1.xyzx
    r2.xyz = ((r0.zzzz)*(r0.xywx)+(-(r1.xyzx))).xyz;
    // 194: mad r1.xyz, r3.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 195: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r1.xyz, cb0[21].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[21].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 198: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 199: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 200: mad r1.xyz, cb0[21].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[21].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 201: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 202: add r0.z, -cb0[4].w, l(1.000000)
    r0.z = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 203: mul r0.z, r0.z, cb0[25].z
    r0.z = ((r0.zzzz)*(source[25].zzzz)).z;
    // 204: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 205: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 206: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: mul r2.x, cb0[4].z, l(1.500000)
    r2.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 208: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 209: mad r0.z, r0.z, l(0.500000), cb0[4].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).z;
    // 210: frc r2.x, v4.x
    r2.x = (frac(v4.xxxx)).x;
    // 211: mul r2.x, r2.x, l(0.125000)
    r2.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 212: mul r4.y, cb0[4].y, cb0[16].y
    r4.y = ((source[4].yyyy)*(source[16].yyyy)).y;
    // 213: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 214: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 215: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 216: frc r2.z, cb0[4].x
    r2.z = (frac(source[4].xxxx)).z;
    // 217: add r3.w, -r2.z, cb0[4].x
    r3.w = ((-(r2.zzzz))+(source[4].xxxx)).w;
    // 218: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 219: add r2.xy, r2.xyxx, r4.zwzz
    r2.xy = ((r2.xyxx)+(r4.zwzz)).xy;
    // 220: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 221: mul r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = ((r0.zzzz)*(r4.xyzx)).xyz;
    // 222: mul r0.z, r2.z, r4.w
    r0.z = ((r2.zzzz)*(r4.wwww)).z;
    // 223: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 225: mad r1.xyz, r0.zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 226: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 227: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 228: add r2.yz, -r4.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r2.yz = ((-(r4.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 229: add r2.yz, -r4.zzwz, r2.yyzy
    r2.yz = ((-(r4.zzwz))+(r2.yyzy)).yz;
    // 230: mad r2.yz, cb0[17].wwww, r2.yyzy, r4.zzwz
    r2.yz = ((source[17].wwww)*(r2.yyzy)+(r4.zzwz)).yz;
    // 231: mul r0.z, cb0[17].y, cb0[25].z
    r0.z = ((source[17].yyyy)*(source[25].zzzz)).z;
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
    // 237: mul r3.w, cb0[17].x, l(0.001000)
    r3.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 238: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 239: mad r2.yz, r3.wwww, r2.yyzy, r4.xxyx
    r2.yz = ((r3.wwww)*(r2.yyzy)+(r4.xxyx)).yz;
    // 240: dp2 r3.w, cb0[18].xyxx, r2.yzyy
    r3.w = (dot((source[18].xyxx).xy,(r2.yzyy).xy).xxxx).w;
    // 241: dp2 r4.y, cb0[19].xyxx, r2.yzyy
    r4.y = (dot((source[19].xyxx).xy,(r2.yzyy).xy).xxxx).y;
    // 242: frc r2.y, r3.w
    r2.y = (frac(r3.wwww)).y;
    // 243: mul r4.x, r2.y, l(0.125000)
    r4.x = ((r2.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 244: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t6.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 245: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 246: mul r2.y, r4.w, l(0.900000)
    r2.y = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 247: mad r4.xyz, r2.yyyy, r4.xyzx, r1.xyzx
    r4.xyz = ((r2.yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 248: mul_sat r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = (saturate((r0.zzzz)*(r4.xyzx))).xyz;
    // 249: mad r6.xyz, cb0[17].zzzz, r4.xyzx, -r1.xyzx
    r6.xyz = ((source[17].zzzz)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 250: mul r4.xyz, r4.xyzx, cb0[17].zzzz
    r4.xyz = ((r4.xyzx)*(source[17].zzzz)).xyz;
    // 251: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 252: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 253: mad r1.xyz, r0.zzzz, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 254: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 255: add r0.z, r2.w, -r10.z
    r0.z = ((r2.wwww)+(-(r10.zzzz))).z;
    // 256: mad r2.yzw, r2.wwww, cb0[14].xxyz, -cb0[14].xxyz
    r2.yzw = ((r2.wwww)*(source[14].xxyz)+(-(source[14].xxyz))).yzw;
    // 257: mad r2.yzw, cb0[14].wwww, r2.yyzw, cb0[14].xxyz
    r2.yzw = ((source[14].wwww)*(r2.yyzw)+(source[14].xxyz)).yzw;
    // 258: mad r0.z, cb0[13].w, r0.z, r10.z
    r0.z = ((source[13].wwww)*(r0.zzzz)+(r10.zzzz)).z;
    // 259: mad r2.yzw, r0.zzzz, cb0[13].xxyz, r2.yyzw
    r2.yzw = ((r0.zzzz)*(source[13].xxyz)+(r2.yyzw)).yzw;
    // 260: add r0.z, cb0[2].y, cb0[2].x
    r0.z = ((source[2].yyyy)+(source[2].xxxx)).z;
    // 261: add r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)+(source[2].zzzz)).z;
    // 262: add r3.w, -r0.z, l(1000.000000)
    r3.w = ((-(r0.zzzz))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 263: mad r0.z, cb0[25].w, r3.w, r0.z
    r0.z = ((source[25].wwww)*(r3.wwww)+(r0.zzzz)).z;
    // 264: mul r0.z, r0.z, l(0.010000)
    r0.z = ((r0.zzzz)*(float4(0.010000,0.010000,0.010000,0.010000))).z;
    // 265: mad r0.z, cb0[25].y, cb0[25].z, r0.z
    r0.z = ((source[25].yyyy)*(source[25].zzzz)+(r0.zzzz)).z;
    // 266: mul r3.w, r0.z, l(3.524534)
    r3.w = ((r0.zzzz)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 267: sincos null, r3.w, r3.w
    r3.w = (cos(r3.wwww)).w;
    // 268: add r0.z, r0.z, r3.w
    r0.z = ((r0.zzzz)+(r3.wwww)).z;
    // 269: mul r0.z, r0.z, l(1.328987)
    r0.z = ((r0.zzzz)*(float4(1.328987,1.328987,1.328987,1.328987))).z;
    // 270: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 271: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 272: mad r0.z, r0.z, l(0.500000), cb0[25].x
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[25].xxxx)).z;
    // 273: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t7.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 274: mul r6.xyz, cb0[12].xyzx, cb0[24].wwww
    r6.xyz = ((source[12].xyzx)*(source[24].wwww)).xyz;
    // 275: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 276: mul r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = ((r0.zzzz)*(r4.xyzx)).xyz;
    // 277: mad r0.xyz, r2.xxxx, r0.xywx, r4.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xywx)+(r4.xyzx)).xyz;
    // 278: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 279: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 280: mad r0.xyz, cb0[21].yyyy, r4.xyzx, r0.xyzx
    r0.xyz = ((source[21].yyyy)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 281: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 282: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 283: mad r0.xyz, cb0[21].zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((source[21].zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 284: mad r0.xyz, r0.xyzx, r11.xyzx, r2.yzwy
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 285: log r0.w, |r1.w|
    r0.w = (log2(abs(r1.wwww))).w;
    // 286: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 287: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 288: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 289: mul r2.xyz, r0.wwww, cb0[15].xyzx
    r2.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 290: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 291: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 292: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 293: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 294: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 295: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 296: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 297: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 298: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 299: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 300: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 301: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 302: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 303: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 304: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 305: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 306: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 307: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 308: mad o0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 309: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 310: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 311: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 312: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 313: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 314: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 315: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 316: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 317: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 318: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 319: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 320: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 321: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 322: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 323: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 324: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 325: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 326: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 327: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 328: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 329: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase15(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[19]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[27].w=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s6, l(0.000000)
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
    // 30: add r0.z, -cb0[22].y, cb0[22].x
    r0.z = ((-(source[22].yyyy))+(source[22].xxxx)).z;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mad r0.z, r2.x, r0.z, cb0[22].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[22].yyyy)).z;
    // 33: add r0.w, -r0.z, cb0[22].z
    r0.w = ((-(r0.zzzz))+(source[22].zzzz)).w;
    // 34: mad r0.z, r2.y, r0.w, r0.z
    r0.z = ((r2.yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 35: add r0.w, -r0.z, cb0[22].w
    r0.w = ((-(r0.zzzz))+(source[22].wwww)).w;
    // 36: mad r0.z, r2.z, r0.w, r0.z
    r0.z = ((r2.zzzz)*(r0.wwww)+(r0.zzzz)).z;
    // 37: add r0.w, -r0.z, cb0[23].x
    r0.w = ((-(r0.zzzz))+(source[23].xxxx)).w;
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
    // 48: add r1.w, -r0.w, cb0[25].x
    r1.w = ((-(r0.wwww))+(source[25].xxxx)).w;
    // 49: mad r0.w, r2.w, r1.w, r0.w
    r0.w = ((r2.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 50: add r1.w, -cb0[24].y, cb0[24].x
    r1.w = ((-(source[24].yyyy))+(source[24].xxxx)).w;
    // 51: mad r1.w, r2.x, r1.w, cb0[24].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[24].yyyy)).w;
    // 52: add r3.w, -r1.w, cb0[24].z
    r3.w = ((-(r1.wwww))+(source[24].zzzz)).w;
    // 53: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 54: add r3.w, -r1.w, cb0[24].w
    r3.w = ((-(r1.wwww))+(source[24].wwww)).w;
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
    // 68: mul r5.xy, r4.xyxx, cb0[21].xxxx
    r5.xy = ((r4.xyxx)*(source[21].xxxx)).xy;
    // 69: mad r4.xy, cb0[21].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[21].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 70: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 71: mad r4.xyz, r2.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 72: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: mad r5.xyz, cb0[23].wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((source[23].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
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
    // 109: rcp r1.w, cb0[25].y
    r1.w = (1.0/(source[25].yyyy)).w;
    // 110: mul r11.xyz, r9.xyzx, r1.wwww
    r11.xyz = ((r9.xyzx)*(r1.wwww)).xyz;
    // 111: mul r9.xyz, r9.xyzx, cb0[25].yyyy
    r9.xyz = ((r9.xyzx)*(source[25].yyyy)).xyz;
    // 112: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 113: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 114: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 115: mad r9.xyz, r9.xyzx, cb0[25].yyyy, r11.xyzx
    r9.xyz = ((r9.xyzx)*(source[25].yyyy)+(r11.xyzx)).xyz;
    // 116: add r0.xyw, r0.xyxw, r9.xyxz
    r0.xyw = ((r0.xyxw)+(r9.xyxz)).xyw;
    // 117: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 118: add r1.w, cb0[25].y, l(1.000000)
    r1.w = ((source[25].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 120: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r9.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r9.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 122: mad r9.xyz, r3.wwww, r9.xyzx, cb0[11].xyzx
    r9.xyz = ((r3.wwww)*(r9.xyzx)+(source[11].xyzx)).xyz;
    // 123: mul r0.xyw, r0.xxxx, r9.xyxz
    r0.xyw = ((r0.xxxx)*(r9.xyxz)).xyw;
    // 124: mul r0.xyw, r0.xyxw, cb0[25].zzzz
    r0.xyw = ((r0.xyxw)*(source[25].zzzz)).xyw;
    // 125: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r9.xyz, -r3.xyzx, r1.wwww
    r9.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 127: mad r3.yzw, cb0[23].yyyy, r9.xxyz, r3.xxyz
    r3.yzw = ((source[23].yyyy)*(r9.xxyz)+(r3.xxyz)).yzw;
    // 128: dp3 r1.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: add r9.xyz, -r3.yzwy, r1.wwww
    r9.xyz = ((-(r3.yzwy))+(r1.wwww)).xyz;
    // 130: mad r3.yzw, cb0[23].zzzz, r9.xxyz, r3.yyzw
    r3.yzw = ((source[23].zzzz)*(r9.xxyz)+(r3.yyzw)).yzw;
    // 131: dp3 r1.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: add r9.xyz, -r3.yzwy, r1.wwww
    r9.xyz = ((-(r3.yzwy))+(r1.wwww)).xyz;
    // 133: mul r9.xyz, r9.xyzx, cb0[25].wwww
    r9.xyz = ((r9.xyzx)*(source[25].wwww)).xyz;
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
    // 146: add r3.w, cb0[26].w, -cb0[27].x
    r3.w = ((source[26].wwww)+(-(source[27].xxxx))).w;
    // 147: mad r3.w, r2.w, r3.w, cb0[27].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[27].xxxx)).w;
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
    // 153: div r3.w, cb0[27].y, r3.w
    r3.w = ((source[27].yyyy)/(r3.wwww)).w;
    // 154: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 155: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 156: div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // 157: dp3 r4.w, r4.xyzx, r5.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 158: mul_sat r5.w, r4.w, cb0[26].x
    r5.w = (saturate((r4.wwww)*(source[26].xxxx))).w;
    // 159: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul_sat r6.w, r5.z, cb0[26].x
    r6.w = (saturate((r5.zzzz)*(source[26].xxxx))).w;
    // 162: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: add_sat r6.w, r6.w, -cb0[26].y
    r6.w = (saturate((r6.wwww)+(-(source[26].yyyy)))).w;
    // 164: log r7.w, r6.w
    r7.w = (log2(r6.wwww)).w;
    // 165: lt r6.w, r6.w, l(0.000001)
    r6.w = (asfloat((uint4)((r6.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 166: mul r7.w, r7.w, cb0[26].z
    r7.w = ((r7.wwww)*(source[26].zzzz)).w;
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
    // 174: mad r1.xyz, cb0[23].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[23].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 175: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: add r11.xyz, -r1.xyzx, r3.wwww
    r11.xyz = ((-(r1.xyzx))+(r3.wwww)).xyz;
    // 177: mad r1.xyz, cb0[23].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[23].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 178: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 179: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 180: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 181: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 182: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 183: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 184: mad r2.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r2.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 185: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 187: mad r2.xyz, cb0[23].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[23].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 188: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: add r11.xyz, -r2.xyzx, r3.wwww
    r11.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 190: mad r2.xyz, cb0[23].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[23].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 191: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 194: mul r12.xyz, r2.xyzx, r11.xyzx
    r12.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 195: mad r2.xyz, -r2.xyzx, r11.xyzx, cb0[10].xyzx
    r2.xyz = ((-(r2.xyzx))*(r11.xyzx)+(source[10].xyzx)).xyz;
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
    // 201: mul r2.w, r2.w, cb0[27].z
    r2.w = ((r2.wwww)*(source[27].zzzz)).w;
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
    // 224: mad r1.xyz, cb0[23].yyyy, r2.xywx, r1.xyzx
    r1.xyz = ((source[23].yyyy)*(r2.xywx)+(r1.xyzx)).xyz;
    // 225: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 226: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 227: mad r1.xyz, cb0[23].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[23].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 228: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 229: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 230: mul r0.z, r0.z, cb0[27].w
    r0.z = ((r0.zzzz)*(source[27].wwww)).z;
    // 231: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 232: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 233: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 234: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 235: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 236: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 237: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 238: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 239: mul r3.y, cb0[3].y, cb0[16].y
    r3.y = ((source[3].yyyy)*(source[16].yyyy)).y;
    // 240: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 241: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 242: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 243: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 244: add r2.w, -r1.w, cb0[3].x
    r2.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 245: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 246: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 247: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 257: mad r2.xy, cb0[17].wwww, r2.xyxx, r3.zwzz
    r2.xy = ((source[17].wwww)*(r2.xyxx)+(r3.zwzz)).xy;
    // 258: mul r0.z, cb0[17].y, cb0[27].w
    r0.z = ((source[17].yyyy)*(source[27].wwww)).z;
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
    // 264: mul r2.w, cb0[17].x, l(0.001000)
    r2.w = ((source[17].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 265: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 266: mad r2.xy, r2.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 267: dp2 r2.w, cb0[18].xyxx, r2.xyxx
    r2.w = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 268: dp2 r2.y, cb0[19].xyxx, r2.xyxx
    r2.y = (dot((source[19].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 269: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 270: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 271: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 272: mad r2.xyw, r3.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r3.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 273: mul r3.x, r3.w, l(0.900000)
    r3.x = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 274: mad r2.xyw, r3.xxxx, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.xxxx)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 275: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 276: mad r3.xyz, cb0[17].zzzz, r2.xywx, -r1.xyzx
    r3.xyz = ((source[17].zzzz)*(r2.xywx)+(-(r1.xyzx))).xyz;
    // 277: mul r2.xyw, r2.xyxw, cb0[17].zzzz
    r2.xyw = ((r2.xyxw)*(source[17].zzzz)).xyw;
    // 278: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 279: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 280: mad r1.xyz, r0.zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 281: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 282: mul r2.xyw, r0.xyxw, r1.wwww
    r2.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 283: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 284: mad r0.xyz, -r1.wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(r1.wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 285: mad r0.xyz, cb0[23].yyyy, r0.xyzx, r2.xywx
    r0.xyz = ((source[23].yyyy)*(r0.xyzx)+(r2.xywx)).xyz;
    // 286: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 287: add r2.xyw, -r0.xyxz, r0.wwww
    r2.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 288: mad r0.xyz, cb0[23].zzzz, r2.xywx, r0.xyzx
    r0.xyz = ((source[23].zzzz)*(r2.xywx)+(r0.xyzx)).xyz;
    // 289: mad r2.xyw, r5.wwww, cb0[14].xyxz, -cb0[14].xyxz
    r2.xyw = ((r5.wwww)*(source[14].xyxz)+(-(source[14].xyxz))).xyw;
    // 290: mul r0.w, r5.w, cb0[13].w
    r0.w = ((r5.wwww)*(source[13].wwww)).w;
    // 291: mad r2.xyw, cb0[14].wwww, r2.xyxw, cb0[14].xyxz
    r2.xyw = ((source[14].wwww)*(r2.xyxw)+(source[14].xyxz)).xyw;
    // 292: mad r2.xyw, r0.wwww, cb0[13].xyxz, r2.xyxw
    r2.xyw = ((r0.wwww)*(source[13].xyxz)+(r2.xyxw)).xyw;
    // 293: mad r0.xyz, r0.xyzx, r11.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.xywx)).xyz;
    // 294: log r0.w, |r2.z|
    r0.w = (log2(abs(r2.zzzz))).w;
    // 295: lt r1.w, |r2.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 296: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 297: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 298: mul r2.xyz, r0.wwww, cb0[15].xyzx
    r2.xyz = ((r0.wwww)*(source[15].xyzx)).xyz;
    // 299: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 300: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 301: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 302: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 303: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 304: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 305: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 306: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 307: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 308: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 309: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 310: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 311: mul r3.yzw, r3.yyyy, cb0[29].xxyz
    r3.yzw = ((r3.yyyy)*(source[29].xxyz)).yzw;
    // 312: mad r3.xyz, r3.xxxx, cb0[28].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[28].xyzx)+(r3.yzwy)).xyz;
    // 313: mul r3.xyz, r3.xyzx, cb0[30].wwww
    r3.xyz = ((r3.xyzx)*(source[30].wwww)).xyz;
    // 314: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 315: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 316: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 317: mad o0.xyz, r1.xyzx, cb0[30].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[30].xyzx)+(r0.xyzx)).xyz;
    // 318: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 319: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 320: dp3 r0.x, r7.xyzx, r2.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 321: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 322: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 323: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 324: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 325: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 326: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 327: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 328: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 329: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 330: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 331: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 332: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 333: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 334: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 335: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 336: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 337: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 338: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase16(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
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
    // 30: add r0.z, -cb0[22].y, cb0[22].x
    r0.z = ((-(source[22].yyyy))+(source[22].xxxx)).z;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mad r0.z, r2.x, r0.z, cb0[22].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[22].yyyy)).z;
    // 33: add r0.w, -r0.z, cb0[22].z
    r0.w = ((-(r0.zzzz))+(source[22].zzzz)).w;
    // 34: mad r0.z, r2.y, r0.w, r0.z
    r0.z = ((r2.yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 35: add r0.w, -r0.z, cb0[22].w
    r0.w = ((-(r0.zzzz))+(source[22].wwww)).w;
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
    // 46: add r1.w, -cb0[21].y, cb0[21].x
    r1.w = ((-(source[21].yyyy))+(source[21].xxxx)).w;
    // 47: mad r1.w, r2.x, r1.w, cb0[21].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[21].yyyy)).w;
    // 48: add r3.w, -r1.w, cb0[21].z
    r3.w = ((-(r1.wwww))+(source[21].zzzz)).w;
    // 49: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 50: add r3.w, -r1.w, cb0[21].w
    r3.w = ((-(r1.wwww))+(source[21].wwww)).w;
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
    // 60: mul r4.xy, r0.ywyy, cb0[20].xxxx
    r4.xy = ((r0.ywyy)*(source[20].xxxx)).xy;
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
    // 66: mad r5.xyz, cb0[20].wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((source[20].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
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
    // 102: rcp r1.w, cb0[23].x
    r1.w = (1.0/(source[23].xxxx)).w;
    // 103: mul r11.xyz, r7.xyzx, r1.wwww
    r11.xyz = ((r7.xyzx)*(r1.wwww)).xyz;
    // 104: mul r7.xyz, r7.xyzx, cb0[23].xxxx
    r7.xyz = ((r7.xyzx)*(source[23].xxxx)).xyz;
    // 105: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 106: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 107: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 108: mad r7.xyz, r7.xyzx, cb0[23].xxxx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(source[23].xxxx)+(r11.xyzx)).xyz;
    // 109: add r0.xyw, r0.xyxw, r7.xyxz
    r0.xyw = ((r0.xyxw)+(r7.xyxz)).xyw;
    // 110: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 111: add r1.w, cb0[23].x, l(1.000000)
    r1.w = ((source[23].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 113: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r7.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r7.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 115: mad r7.xyz, r3.wwww, r7.xyzx, cb0[10].xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)+(source[10].xyzx)).xyz;
    // 116: mul r0.xyw, r0.xxxx, r7.xyxz
    r0.xyw = ((r0.xxxx)*(r7.xyxz)).xyw;
    // 117: mul r0.xyw, r0.xyxw, cb0[23].yyyy
    r0.xyw = ((r0.xyxw)*(source[23].yyyy)).xyw;
    // 118: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r7.xyz, -r3.xyzx, r1.wwww
    r7.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 120: mad r3.xyz, cb0[20].yyyy, r7.xyzx, r3.xyzx
    r3.xyz = ((source[20].yyyy)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 121: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r7.xyz, -r3.xyzx, r1.wwww
    r7.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 123: mad r3.xyz, cb0[20].zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((source[20].zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 124: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r7.xyz, -r3.xyzx, r1.wwww
    r7.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 126: mul r7.xyz, r7.xyzx, cb0[23].zzzz
    r7.xyz = ((r7.xyzx)*(source[23].zzzz)).xyz;
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
    // 137: mul r1.w, r1.w, cb0[24].z
    r1.w = ((r1.wwww)*(source[24].zzzz)).w;
    // 138: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 139: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: mad r2.w, -r1.w, r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 142: div r2.w, cb0[24].w, r2.w
    r2.w = ((source[24].wwww)/(r2.wwww)).w;
    // 143: dp3 r3.w, r4.xyzx, r4.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 144: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 145: div r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)/(r3.wwww)).xyz;
    // 146: dp3 r3.w, r4.xyzx, r5.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 147: mul_sat r4.w, r3.w, cb0[23].w
    r4.w = (saturate((r3.wwww)*(source[23].wwww))).w;
    // 148: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mul_sat r5.w, r5.z, cb0[23].w
    r5.w = (saturate((r5.zzzz)*(source[23].wwww))).w;
    // 151: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: add_sat r5.w, r5.w, -cb0[24].x
    r5.w = (saturate((r5.wwww)+(-(source[24].xxxx)))).w;
    // 153: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 154: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 155: mul r6.w, r6.w, cb0[24].y
    r6.w = ((r6.wwww)*(source[24].yyyy)).w;
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
    // 163: mad r1.xyz, cb0[20].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 164: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 165: add r11.xyz, -r1.xyzx, r2.wwww
    r11.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 166: mad r1.xyz, cb0[20].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 167: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 168: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 169: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 170: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 171: mad r2.xyw, r2.yyyy, r12.xyxz, r11.xyxz
    r2.xyw = ((r2.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 172: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r2.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r2.xywx))).xyz;
    // 173: mad r2.xyz, r2.zzzz, r11.xyzx, r2.xywx
    r2.xyz = ((r2.zzzz)*(r11.xyzx)+(r2.xywx)).xyz;
    // 174: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 176: mad r2.xyz, cb0[20].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 177: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 179: mad r2.xyz, cb0[20].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[20].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 180: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
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
    // 188: mul r2.w, r2.w, cb0[25].x
    r2.w = ((r2.wwww)*(source[25].xxxx)).w;
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
    // 211: mad r1.xyz, cb0[20].yyyy, r2.xywx, r1.xyzx
    r1.xyz = ((source[20].yyyy)*(r2.xywx)+(r1.xyzx)).xyz;
    // 212: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 213: add r2.xyw, -r1.xyxz, r0.zzzz
    r2.xyw = ((-(r1.xyxz))+(r0.zzzz)).xyw;
    // 214: mad r1.xyz, cb0[20].zzzz, r2.xywx, r1.xyzx
    r1.xyz = ((source[20].zzzz)*(r2.xywx)+(r1.xyzx)).xyz;
    // 215: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 216: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 217: mul r0.z, r0.z, cb0[25].y
    r0.z = ((r0.zzzz)*(source[25].yyyy)).z;
    // 218: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 219: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 220: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 221: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 222: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 223: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 224: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 225: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 226: mul r3.y, cb0[3].y, cb0[15].y
    r3.y = ((source[3].yyyy)*(source[15].yyyy)).y;
    // 227: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 228: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 229: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 230: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 231: add r2.w, -r1.w, cb0[3].x
    r2.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 232: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 233: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 234: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 244: mad r2.xy, cb0[16].wwww, r2.xyxx, r3.zwzz
    r2.xy = ((source[16].wwww)*(r2.xyxx)+(r3.zwzz)).xy;
    // 245: mul r0.z, cb0[16].y, cb0[25].y
    r0.z = ((source[16].yyyy)*(source[25].yyyy)).z;
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
    // 251: mul r2.w, cb0[16].x, l(0.001000)
    r2.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 252: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 253: mad r2.xy, r2.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r2.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 254: dp2 r2.w, cb0[17].xyxx, r2.xyxx
    r2.w = (dot((source[17].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 255: dp2 r2.y, cb0[18].xyxx, r2.xyxx
    r2.y = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 256: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 257: mul r2.x, r2.w, l(0.125000)
    r2.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 258: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 259: mad r2.xyw, r3.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r2.xyw = ((r3.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 260: mul r3.x, r3.w, l(0.900000)
    r3.x = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 261: mad r2.xyw, r3.xxxx, r2.xyxw, r1.xyxz
    r2.xyw = ((r3.xxxx)*(r2.xyxw)+(r1.xyxz)).xyw;
    // 262: mul_sat r2.xyw, r0.zzzz, r2.xyxw
    r2.xyw = (saturate((r0.zzzz)*(r2.xyxw))).xyw;
    // 263: mad r3.xyz, cb0[16].zzzz, r2.xywx, -r1.xyzx
    r3.xyz = ((source[16].zzzz)*(r2.xywx)+(-(r1.xyzx))).xyz;
    // 264: mul r2.xyw, r2.xyxw, cb0[16].zzzz
    r2.xyw = ((r2.xyxw)*(source[16].zzzz)).xyw;
    // 265: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 266: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 267: mad r1.xyz, r0.zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 268: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 269: mul r2.xyw, r0.xyxw, r1.wwww
    r2.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 270: dp3 r0.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 271: mad r0.xyz, -r1.wwww, r0.xywx, r0.zzzz
    r0.xyz = ((-(r1.wwww))*(r0.xywx)+(r0.zzzz)).xyz;
    // 272: mad r0.xyz, cb0[20].yyyy, r0.xyzx, r2.xywx
    r0.xyz = ((source[20].yyyy)*(r0.xyzx)+(r2.xywx)).xyz;
    // 273: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: add r2.xyw, -r0.xyxz, r0.wwww
    r2.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 275: mad r0.xyz, cb0[20].zzzz, r2.xywx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r2.xywx)+(r0.xyzx)).xyz;
    // 276: mad r2.xyw, r4.wwww, cb0[13].xyxz, -cb0[13].xyxz
    r2.xyw = ((r4.wwww)*(source[13].xyxz)+(-(source[13].xyxz))).xyw;
    // 277: mul r0.w, r4.w, cb0[12].w
    r0.w = ((r4.wwww)*(source[12].wwww)).w;
    // 278: mad r2.xyw, cb0[13].wwww, r2.xyxw, cb0[13].xyxz
    r2.xyw = ((source[13].wwww)*(r2.xyxw)+(source[13].xyxz)).xyw;
    // 279: mad r2.xyw, r0.wwww, cb0[12].xyxz, r2.xyxw
    r2.xyw = ((r0.wwww)*(source[12].xyxz)+(r2.xyxw)).xyw;
    // 280: mad r0.xyz, r0.xyzx, r11.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.xywx)).xyz;
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
    // 307: dp3 r0.x, r6.xyzx, r2.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 308: dp3 r0.z, r8.xyzx, r2.xyzx
    r0.z = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 309: dp3 r0.y, r9.xyzx, r2.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
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

