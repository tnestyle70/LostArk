#ifndef SOURCE_CHARACTER_BASE_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase108(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[24].z=(g_SourceCharacterTime.xxxx).x;
    source[26].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[26].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[29] = g_SourceCharacterEnvironmentColor;
        source[30] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0, r21=0.0, r22=0.0;
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
    // 11: add r0.x, -cb0[5].w, l(1.000000)
    r0.x = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r0.x, r0.x, cb0[24].z
    r0.x = ((r0.xxxx)*(source[24].zzzz)).x;
    // 13: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 14: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 15: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 16: mul r0.y, cb0[5].z, l(1.500000)
    r0.y = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 17: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 18: mad r0.x, r0.x, l(0.500000), cb0[5].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).x;
    // 19: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 20: mul r2.x, r0.y, l(0.125000)
    r2.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 21: mul r3.y, cb0[5].y, cb0[16].y
    r3.y = ((source[5].yyyy)*(source[16].yyyy)).y;
    // 22: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 23: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 24: add r0.yz, r2.xxyx, r3.xxyx
    r0.yz = ((r2.xxyx)+(r3.xxyx)).yz;
    // 25: frc r0.w, cb0[5].x
    r0.w = (frac(source[5].xxxx)).w;
    // 26: add r1.w, -r0.w, cb0[5].x
    r1.w = ((-(r0.wwww))+(source[5].xxxx)).w;
    // 27: mul r3.z, r1.w, l(0.125000)
    r3.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 28: add r0.yz, r0.yyzy, r3.zzwz
    r0.yz = ((r0.yyzy)+(r3.zzwz)).yz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 35: mad r2.xyz, cb0[25].xxxx, r2.xyzx, r1.xyzx
    r2.xyz = ((source[25].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 36: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 38: mad r2.xyz, cb0[25].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[25].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 39: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
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
    // 48: mul r2.w, r6.y, cb0[18].y
    r2.w = ((r6.yyyy)*(source[18].yyyy)).w;
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
    // 56: mad r7.xyz, cb0[25].xxxx, r7.xyzx, r3.xyzx
    r7.xyz = ((source[25].xxxx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 57: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 58: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: add r3.xyz, -r7.xyzx, r2.wwww
    r3.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 60: mad r3.xyz, cb0[25].yyyy, r3.xyzx, r7.xyzx
    r3.xyz = ((source[25].yyyy)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 61: mad r7.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mad r8.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
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
    // 69: mad r3.xyz, cb0[25].xxxx, r9.xyzx, r8.xyzx
    r3.xyz = ((source[25].xxxx)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 70: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: add r8.xyz, -r3.xyzx, r2.wwww
    r8.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 72: mad r3.xyz, cb0[25].yyyy, r8.xyzx, r3.xyzx
    r3.xyz = ((source[25].yyyy)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 73: mul r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 74: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 75: mad r0.xyz, r1.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 76: mul r1.w, r6.x, cb0[26].z
    r1.w = ((r6.xxxx)*(source[26].zzzz)).w;
    // 77: mul r2.w, r6.z, cb0[28].y
    r2.w = ((r6.zzzz)*(source[28].yyyy)).w;
    // 78: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 79: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: movc r2.w, r5.z, l(0), r2.w
    r2.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 81: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 82: min r4.z, r2.w, l(1.000000)
    r4.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 83: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 84: movc r1.w, r5.x, l(0), r1.w
    r1.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 85: add_sat r1.w, r1.w, cb0[26].w
    r1.w = (saturate((r1.wwww)+(source[26].wwww))).w;
    // 86: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 87: mul r3.xyz, r2.wwww, cb0[15].xyzx
    r3.xyz = ((r2.wwww)*(source[15].xyzx)).xyz;
    // 88: mul r5.xyz, r0.xyzx, r3.xyzx
    r5.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 89: mad r0.xyz, -r3.xyzx, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r3.xyzx))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 90: mad r0.xyz, r1.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 91: add r3.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
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
    // 101: mov_sat r3.w, cb0[27].z
    r3.w = (saturate(source[27].zzzz)).w;
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
    // 110: mul r6.xy, r4.xyxx, cb0[18].xxxx
    r6.xy = ((r4.xyxx)*(source[18].xxxx)).xy;
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
    // 143: mul r3.w, r3.w, cb0[2].y
    r3.w = ((r3.wwww)*(source[2].yyyy)).w;
    // 144: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 145: mad_sat r3.w, r3.w, cb0[2].w, cb0[2].z
    r3.w = (saturate((r3.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 146: mul r3.w, r3.w, cb0[28].z
    r3.w = ((r3.wwww)*(source[28].zzzz)).w;
    // 147: add_sat r4.x, -r2.w, r4.z
    r4.x = (saturate((-(r2.wwww))+(r4.zzzz))).x;
    // 148: sample_indexable(texture2d)(float,float,float,float) r12.xy, r4.xyxx, t6.xyzw, s7
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
    // 177: dp2 r15.z, r18.xyxx, cb0[30].xyxx
    r15.z = (dot((r18.xyxx).xy,(source[30].xyxx).xy).xxxx).z;
    // 178: mul r19.xy, cb0[30].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r19.xy = ((source[30].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 179: dp2 r15.x, r18.xyxx, r19.xyxx
    r15.x = (dot((r18.xyxx).xy,(r19.xyxx).xy).xxxx).x;
    // 180: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 181: dp4 r20.x, cb0[31].xyzw, r15.xyzw
    r20.x = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 182: dp4 r20.y, cb0[32].xyzw, r15.xyzw
    r20.y = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 183: dp4 r20.z, cb0[33].xyzw, r15.xyzw
    r20.z = (dot((source[33].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 184: mul r21.xyzw, r15.yzzx, r15.xyzz
    r21.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 185: dp4 r22.x, cb0[34].xyzw, r21.xyzw
    r22.x = (dot((source[34].xyzw).xyzw,(r21.xyzw).xyzw).xxxx).x;
    // 186: dp4 r22.y, cb0[35].xyzw, r21.xyzw
    r22.y = (dot((source[35].xyzw).xyzw,(r21.xyzw).xyzw).xxxx).y;
    // 187: dp4 r22.z, cb0[36].xyzw, r21.xyzw
    r22.z = (dot((source[36].xyzw).xyzw,(r21.xyzw).xyzw).xxxx).z;
    // 188: add r20.xyz, r20.xyzx, r22.xyzx
    r20.xyz = ((r20.xyzx)+(r22.xyzx)).xyz;
    // 189: mul r5.w, r15.y, r15.y
    r5.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 190: mov r18.z, r15.y
    r18.z = (r15.yyyy).z;
    // 191: mad r5.w, r15.x, r15.x, -r5.w
    r5.w = ((r15.xxxx)*(r15.xxxx)+(-(r5.wwww))).w;
    // 192: mad r15.xyz, cb0[37].xyzx, r5.wwww, r20.xyzx
    r15.xyz = ((source[37].xyzx)*(r5.wwww)+(r20.xyzx)).xyz;
    // 193: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 194: mul r15.xyz, r15.xyzx, cb0[29].xyzx
    r15.xyz = ((r15.xyzx)*(source[29].xyzx)).xyz;
    // 195: mul r15.xyz, r15.xyzx, cb0[30].zzzz
    r15.xyz = ((r15.xyzx)*(source[30].zzzz)).xyz;
    // 196: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[29].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[29].wwww)).xyz;
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
    // 225: dp2 r14.z, r15.xyxx, cb0[30].xyxx
    r14.z = (dot((r15.xyxx).xy,(source[30].xyxx).xy).xxxx).z;
    // 226: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r14.xyzx, t7.xyzw, s6, r5.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r14.xyzx).xyz, (r5.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 227: mul r14.xyz, r14.xyzx, r14.wwww
    r14.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 228: mul r14.xyz, r14.xyzx, cb0[29].xyzx
    r14.xyz = ((r14.xyzx)*(source[29].xyzx)).xyz;
    // 229: mul r14.xyz, r14.xyzx, cb0[30].zzzz
    r14.xyz = ((r14.xyzx)*(source[30].zzzz)).xyz;
    // 230: mad r14.xyz, r14.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[29].wwww
    r14.xyz = ((r14.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[29].wwww)).xyz;
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
    // 262: mul r11.xyz, r8.wwww, cb0[40].xyzx
    r11.xyz = ((r8.wwww)*(source[40].xyzx)).xyz;
    // 263: mad r11.xyz, r8.zzzz, cb0[39].xyzx, r11.xyzx
    r11.xyz = ((r8.zzzz)*(source[39].xyzx)+(r11.xyzx)).xyz;
    // 264: mul r11.xyz, r11.xyzx, cb0[41].wwww
    r11.xyz = ((r11.xyzx)*(source[41].wwww)).xyz;
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
    // 271: mul r5.xyz, r8.yyyy, cb0[40].xyzx
    r5.xyz = ((r8.yyyy)*(source[40].xyzx)).xyz;
    // 272: mad r5.xyz, cb0[39].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[39].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 273: mul r5.xyz, r5.xyzx, cb0[41].wwww
    r5.xyz = ((r5.xyzx)*(source[41].wwww)).xyz;
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
    // 280: mul r1.w, r17.z, cb0[19].w
    r1.w = ((r17.zzzz)*(source[19].wwww)).w;
    // 281: mul r2.w, r17.z, cb0[20].x
    r2.w = ((r17.zzzz)*(source[20].xxxx)).w;
    // 282: mad r5.x, r16.z, cb0[19].w, -r2.w
    r5.x = ((r16.zzzz)*(source[19].wwww)+(-(r2.wwww))).x;
    // 283: mad r5.y, r16.z, cb0[20].x, r1.w
    r5.y = ((r16.zzzz)*(source[20].xxxx)+(r1.wwww)).y;
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
    // 315: add r2.w, -r1.w, -cb0[20].w
    r2.w = ((-(r1.wwww))+(-(source[20].wwww))).w;
    // 316: add r1.w, r1.w, -cb0[20].w
    r1.w = ((r1.wwww)+(-(source[20].wwww))).w;
    // 317: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 318: add r4.w, -cb0[20].w, l(1.000000)
    r4.w = ((-(source[20].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 336: mul_sat r4.w, r2.w, cb0[21].x
    r4.w = (saturate((r2.wwww)*(source[21].xxxx))).w;
    // 337: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 338: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 339: mul_sat r5.x, r9.z, cb0[21].x
    r5.x = (saturate((r9.zzzz)*(source[21].xxxx))).x;
    // 340: add r5.y, -|r9.z|, l(1.000000)
    r5.y = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 341: mul r2.w, r2.w, r5.y
    r2.w = ((r2.wwww)*(r5.yyyy)).w;
    // 342: add r5.x, -r5.x, l(1.000000)
    r5.x = ((-(r5.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 343: add_sat r5.x, r5.x, -cb0[21].y
    r5.x = (saturate((r5.xxxx)+(-(source[21].yyyy)))).x;
    // 344: log r5.y, r5.x
    r5.y = (log2(r5.xxxx)).y;
    // 345: lt r5.x, r5.x, l(0.000001)
    r5.x = (asfloat((uint4)((r5.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 346: mul r5.y, r5.y, cb0[21].z
    r5.y = ((r5.yyyy)*(source[21].zzzz)).y;
    // 347: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 348: mul r4.w, r4.w, r5.y
    r4.w = ((r4.wwww)*(r5.yyyy)).w;
    // 349: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 350: mul r5.x, r4.w, cb0[21].w
    r5.x = ((r4.wwww)*(source[21].wwww)).x;
    // 351: mul r1.w, r1.w, r5.x
    r1.w = ((r1.wwww)*(r5.xxxx)).w;
    // 352: mul r1.w, r1.w, cb0[21].w
    r1.w = ((r1.wwww)*(source[21].wwww)).w;
    // 353: lt r5.x, |r1.w|, l(0.000001)
    r5.x = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 354: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 355: mul r1.w, r1.w, cb0[22].x
    r1.w = ((r1.wwww)*(source[22].xxxx)).w;
    // 356: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 357: mul r5.yzw, cb0[6].xxyz, cb0[6].wwww
    r5.yzw = ((source[6].xxyz)*(source[6].wwww)).yzw;
    // 358: mul r5.yzw, r1.wwww, r5.yyzw
    r5.yzw = ((r1.wwww)*(r5.yyzw)).yzw;
    // 359: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 360: dp2 r1.w, cb0[7].xyxx, r6.xyxx
    r1.w = (dot((source[7].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 361: add r1.w, r1.w, cb0[23].z
    r1.w = ((r1.wwww)+(source[23].zzzz)).w;
    // 362: add_sat r1.w, r1.w, l(-0.500000)
    r1.w = (saturate((r1.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).w;
    // 363: mul r5.yzw, r5.yyzw, r1.wwww
    r5.yzw = ((r5.yyzw)*(r1.wwww)).yzw;
    // 364: movc r5.xyz, r5.xxxx, l(0,0,0,0), r5.yzwy
    r5.xyz = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.yzwy)).xyz;
    // 365: add r1.w, cb0[0].y, cb0[0].x
    r1.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 366: add r1.w, r1.w, cb0[0].z
    r1.w = ((r1.wwww)+(source[0].zzzz)).w;
    // 367: add r5.w, -r1.w, l(1000.000000)
    r5.w = ((-(r1.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 368: mad r1.w, cb0[24].w, r5.w, r1.w
    r1.w = ((source[24].wwww)*(r5.wwww)+(r1.wwww)).w;
    // 369: mul r1.w, r1.w, l(0.010000)
    r1.w = ((r1.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 370: mad r1.w, cb0[24].y, cb0[24].z, r1.w
    r1.w = ((source[24].yyyy)*(source[24].zzzz)+(r1.wwww)).w;
    // 371: mul r5.w, r1.w, l(3.524534)
    r5.w = ((r1.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 372: sincos null, r5.w, r5.w
    r5.w = (cos(r5.wwww)).w;
    // 373: add r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)+(r5.wwww)).w;
    // 374: mul r1.w, r1.w, l(1.328987)
    r1.w = ((r1.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 375: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 376: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 377: mad r1.w, r1.w, l(0.500000), cb0[24].x
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[24].xxxx)).w;
    // 378: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t5.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 379: mul r8.xyz, cb0[8].xyzx, cb0[23].wwww
    r8.xyz = ((source[8].xyzx)*(source[23].wwww)).xyz;
    // 380: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 381: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 382: mad r5.xyz, r0.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 383: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 384: add r6.xyz, -r5.xyzx, r0.wwww
    r6.xyz = ((-(r5.xyzx))+(r0.wwww)).xyz;
    // 385: mad r5.xyz, cb0[25].xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((source[25].xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 386: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 387: add r6.xyz, -r5.xyzx, r0.wwww
    r6.xyz = ((-(r5.xyzx))+(r0.wwww)).xyz;
    // 388: mad r5.xyz, cb0[25].yyyy, r6.xyzx, r5.xyzx
    r5.xyz = ((source[25].yyyy)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 389: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 390: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 391: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 392: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 393: add r6.xyz, -r2.xyzx, r0.wwww
    r6.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 394: add r2.xyz, r2.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)+(-(r6.xyzx))).xyz;
    // 395: mul r6.xyz, cb0[13].xyzx, cb0[25].wwww
    r6.xyz = ((source[13].xyzx)*(source[25].wwww)).xyz;
    // 396: mul r6.xyz, r6.xyzx, cb0[26].yyyy
    r6.xyz = ((r6.xyzx)*(source[26].yyyy)).xyz;
    // 397: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 398: mad r8.xyz, r4.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r8.xyz = ((r4.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 399: add r0.w, r4.w, l(-1.000000)
    r0.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 400: mad r0.w, cb0[11].w, r0.w, l(1.000000)
    r0.w = ((source[11].wwww)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 401: mad r8.xyz, cb0[12].wwww, r8.xyzx, cb0[12].xyzx
    r8.xyz = ((source[12].wwww)*(r8.xyzx)+(source[12].xyzx)).xyz;
    // 402: mad r2.xyz, r2.xyzx, r6.xyzx, r8.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 403: mad r2.xyz, r0.wwww, cb0[11].xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(source[11].xyzx)+(r2.xyzx)).xyz;
    // 404: mad r2.xyz, r5.xyzx, r7.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 405: log r0.w, |r2.w|
    r0.w = (log2(abs(r2.wwww))).w;
    // 406: lt r1.w, |r2.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 407: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 408: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 409: mul r5.xyz, r0.wwww, cb0[14].xyzx
    r5.xyz = ((r0.wwww)*(source[14].xyzx)).xyz;
    // 410: movc r5.xyz, r1.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 411: add r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 412: mad r1.xyz, cb0[18].zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((source[18].zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 413: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 414: mul r2.xyz, r4.zzzz, cb0[40].xyzx
    r2.xyz = ((r4.zzzz)*(source[40].xyzx)).xyz;
    // 415: mad r2.xyz, r4.yyyy, cb0[39].xyzx, r2.xyzx
    r2.xyz = ((r4.yyyy)*(source[39].xyzx)+(r2.xyzx)).xyz;
    // 416: mul r2.xyz, r2.xyzx, cb0[41].wwww
    r2.xyz = ((r2.xyzx)*(source[41].wwww)).xyz;
    // 417: mul_sat r4.yzw, cb0[17].xxyz, cb0[17].wwww
    r4.yzw = (saturate((source[17].xxyz)*(source[17].wwww))).yzw;
    // 418: mul r5.xyz, r3.wwww, r4.yzwy
    r5.xyz = ((r3.wwww)*(r4.yzwy)).xyz;
    // 419: mul r4.yzw, r4.yyzw, cb0[28].zzzz
    r4.yzw = ((r4.yyzw)*(source[28].zzzz)).yzw;
    // 420: dp3_sat o5.x, r4.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r4.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 421: mul r4.xyz, r4.xxxx, r5.xyzx
    r4.xyz = ((r4.xxxx)*(r5.xyzx)).xyz;
    // 422: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 423: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 424: mad r1.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 425: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 426: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 427: mad o0.xyz, r3.xyzx, cb0[41].xyzx, r1.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[41].xyzx)+(r1.xyzx)).xyz;
    // 428: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 429: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 430: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 431: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 432: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 433: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 434: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 435: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 436: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 437: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 438: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 439: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 440: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 441: ftou r0.x, cb0[38].z
    r0.x = (asfloat((uint4)(source[38].zzzz))).x;
    // 442: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 443: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 444: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 445: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 446: ret
    return output;
}

// source.character.monster-fd6df5a0ab9a.v1 / source program d29e975c521d17458747d3450bbf5ed5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase109(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].w=(g_SourceCharacterTime.xxxx).x;
    source[20].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[20].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
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
    // 7: add r1.xyzw, -cb0[9].xyzw, cb0[10].xyzw
    r1.xyzw = ((-(source[9].xyzw))+(source[10].xyzw)).xyzw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 9: mad r1.xyzw, r2.xxxx, r1.xyzw, cb0[9].xyzw
    r1.xyzw = ((r2.xxxx)*(r1.xyzw)+(source[9].xyzw)).xyzw;
    // 10: add r3.xyzw, -r1.xyzw, cb0[11].xyzw
    r3.xyzw = ((-(r1.xyzw))+(source[11].xyzw)).xyzw;
    // 11: mad r1.xyzw, r2.yyyy, r3.xyzw, r1.xyzw
    r1.xyzw = ((r2.yyyy)*(r3.xyzw)+(r1.xyzw)).xyzw;
    // 12: add r3.xyzw, -r1.xyzw, cb0[12].xyzw
    r3.xyzw = ((-(r1.xyzw))+(source[12].xyzw)).xyzw;
    // 13: mad r1.xyzw, r2.zzzz, r3.xyzw, r1.xyzw
    r1.xyzw = ((r2.zzzz)*(r3.xyzw)+(r1.xyzw)).xyzw;
    // 14: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 15: mul r1.xyz, r1.xyzx, cb0[20].wwww
    r1.xyz = ((r1.xyzx)*(source[20].wwww)).xyz;
    // 16: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 17: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 18: mad r0.xyz, -r0.yzwy, r1.xyzx, r0.xxxx
    r0.xyz = ((-(r0.yzwy))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 19: mad r0.xyz, cb0[18].xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((source[18].xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 20: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 22: mad r0.xyz, cb0[18].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[18].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 23: mad r1.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 24: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 25: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 26: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 27: add r0.w, -cb0[13].w, l(1.000000)
    r0.w = ((-(source[13].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: mul r0.w, r0.w, cb0[19].w
    r0.w = ((r0.wwww)*(source[19].wwww)).w;
    // 29: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 30: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 31: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: mul r1.x, cb0[13].z, l(1.500000)
    r1.x = ((source[13].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 33: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 34: mad r0.w, r0.w, l(0.500000), cb0[13].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[13].zzzz)).w;
    // 35: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 36: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 37: mul r2.y, cb0[13].y, cb0[14].y
    r2.y = ((source[13].yyyy)*(source[14].yyyy)).y;
    // 38: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 39: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 40: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 41: frc r1.z, cb0[13].x
    r1.z = (frac(source[13].xxxx)).z;
    // 42: add r1.w, -r1.z, cb0[13].x
    r1.w = ((-(r1.zzzz))+(source[13].xxxx)).w;
    // 43: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 44: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r1.xyw, r0.wwww, r2.xyxz
    r1.xyw = ((r0.wwww)*(r2.xyxz)).xyw;
    // 47: mul r0.w, r1.z, r2.w
    r0.w = ((r1.zzzz)*(r2.wwww)).w;
    // 48: mad r1.xyz, r1.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 49: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 50: add r1.xyzw, v7.yzxy, cb0[0].yzxy
    r1.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 51: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 52: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 53: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 54: mad r1.xy, cb0[15].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[15].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 55: mul r0.w, cb0[15].y, cb0[19].w
    r0.w = ((source[15].yyyy)*(source[19].wwww)).w;
    // 56: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 57: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 58: mul r2.y, r0.w, l(0.020000)
    r2.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 59: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 61: mul r1.z, cb0[15].x, l(0.001000)
    r1.z = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 62: mov r2.x, l(0)
    r2.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 63: mad r1.xy, r1.zzzz, r1.xyxx, r2.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r2.xyxx)).xy;
    // 64: dp2 r1.z, cb0[16].xyxx, r1.xyxx
    r1.z = (dot((source[16].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 65: dp2 r1.y, cb0[17].xyxx, r1.xyxx
    r1.y = (dot((source[17].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 66: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 67: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 69: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 70: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 71: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 72: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 73: mad r2.xyz, cb0[15].zzzz, r1.xyzx, -r0.xyzx
    r2.xyz = ((source[15].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 74: mul r1.xyz, r1.xyzx, cb0[15].zzzz
    r1.xyz = ((r1.xyzx)*(source[15].zzzz)).xyz;
    // 75: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 76: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 77: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 78: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 79: mul r1.xyz, cb0[8].xyzx, cb0[19].zzzz
    r1.xyz = ((source[8].xyzx)*(source[19].zzzz)).xyz;
    // 80: mul r1.xyz, r1.xyzx, cb0[20].yyyy
    r1.xyz = ((r1.xyzx)*(source[20].yyyy)).xyz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 82: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 83: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 84: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 86: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 87: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 89: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 90: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 91: mul r3.xyz, r0.wwww, r2.xyzx
    r3.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 92: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 93: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 96: dp3 r0.w, r2.xyzx, r4.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 97: add r1.w, -|r4.z|, l(1.000000)
    r1.w = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 100: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 101: mul r1.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 102: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 103: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 104: mul r1.xyz, r1.xyzx, cb0[20].zzzz
    r1.xyz = ((r1.xyzx)*(source[20].zzzz)).xyz;
    // 105: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 106: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 107: mad_sat r1.w, r0.w, cb0[18].z, -cb0[18].w
    r1.w = (saturate((r0.wwww)*(source[18].zzzz)+(-(source[18].wwww)))).w;
    // 108: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 109: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 110: mul r2.x, r2.x, cb0[19].x
    r2.x = ((r2.xxxx)*(source[19].xxxx)).x;
    // 111: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 112: movc r1.w, r1.w, l(0), r2.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 113: mad r2.xyz, r1.wwww, cb0[7].xyzx, -cb0[7].xyzx
    r2.xyz = ((r1.wwww)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 114: mad r4.xyz, r1.wwww, cb0[6].xyzx, -cb0[6].xyzx
    r4.xyz = ((r1.wwww)*(source[6].xyzx)+(-(source[6].xyzx))).xyz;
    // 115: mad r4.xyz, cb0[6].wwww, r4.xyzx, cb0[6].xyzx
    r4.xyz = ((source[6].wwww)*(r4.xyzx)+(source[6].xyzx)).xyz;
    // 116: mad r2.xyz, cb0[7].wwww, r2.xyzx, cb0[7].xyzx
    r2.xyz = ((source[7].wwww)*(r2.xyzx)+(source[7].xyzx)).xyz;
    // 117: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 118: mad r1.xyz, cb0[19].yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((source[19].yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 119: add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // 120: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 121: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 122: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 123: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 124: mul r2.xyz, r1.wwww, cb0[3].xyzx
    r2.xyz = ((r1.wwww)*(source[3].xyzx)).xyz;
    // 125: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 126: mad r1.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xyzx)).xyz;
    // 127: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 128: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 129: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 130: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 131: dp3 r0.w, r2.xyzx, r3.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 132: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 133: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 134: mul r2.yzw, r2.yyyy, cb0[22].xxyz
    r2.yzw = ((r2.yyyy)*(source[22].xxyz)).yzw;
    // 135: mad r2.xyz, r2.xxxx, cb0[21].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[21].xyzx)+(r2.yzwy)).xyz;
    // 136: mul r2.xyz, r2.xyzx, cb0[23].wwww
    r2.xyz = ((r2.xyzx)*(source[23].wwww)).xyz;
    // 137: mad r1.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 138: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 139: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 140: mad o0.xyz, r0.xyzx, cb0[23].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 141: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 142: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 143: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 144: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 145: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 146: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 147: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 148: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 149: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 150: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 151: dp3 r0.z, r0.xyzx, r3.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 152: dp3 r0.x, r1.xyzx, r3.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 153: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 154: dp3 r0.y, r1.xyzx, r3.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 155: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 156: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 157: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 158: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 159: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 160: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 161: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 162: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 163: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 164: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 165: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 166: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 167: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 168: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 169: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 170: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drr-01-head-st-mi-fx-dead.v1 / source program e56633f592e0154eb0794435cf3c2717
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase110(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19].x=(g_SourceCharacterTime.xxxx).x;
    source[19].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[19].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[20].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[20].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[20].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[24] = g_SourceCharacterEnvironmentColor;
        source[25] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
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
    // 15: mul r5.xy, r4.xyxx, cb0[16].xxxx
    r5.xy = ((r4.xyxx)*(source[16].xxxx)).xy;
    // 16: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 17: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 19: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 20: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 22: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 23: div r4.xyz, r5.xyzx, r0.wwww
    r4.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 24: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 27: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 28: mul r6.xyz, r0.wwww, r5.xyzx
    r6.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 29: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r8.xy, v4.xyxx, cb0[22].xxxx
    r8.xy = ((v4.xyxx)*(source[22].xxxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r8.xyxx, t2.yzwx, s4, l(0.000000)
    r1.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 33: add r1.w, r1.w, -cb0[22].y
    r1.w = ((r1.wwww)+(-(source[22].yyyy))).w;
    // 34: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 35: mul_sat r1.w, r1.w, r7.w
    r1.w = (saturate((r1.wwww)*(r7.wwww))).w;
    // 36: mul_sat r1.w, r1.w, cb0[22].z
    r1.w = (saturate((r1.wwww)*(source[22].zzzz))).w;
    // 37: mul r2.w, r1.w, cb0[2].x
    r2.w = ((r1.wwww)*(source[2].xxxx)).w;
    // 38: add r8.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: lt r10.xyz, |r9.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (asfloat((uint4)((abs(r9.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 41: log r9.xyz, |r9.xzyx|
    r9.xyz = (log2(abs(r9.xzyx))).xyz;
    // 42: mul r3.w, r9.x, cb0[20].w
    r3.w = ((r9.xxxx)*(source[20].wwww)).w;
    // 43: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 44: movc r3.w, r10.x, l(0), r3.w
    r3.w = ((asuint(r10.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 45: add_sat r3.w, r3.w, cb0[21].x
    r3.w = (saturate((r3.wwww)+(source[21].xxxx))).w;
    // 46: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mul r11.xyz, r4.wwww, cb0[13].xyzx
    r11.xyz = ((r4.wwww)*(source[13].xyzx)).xyz;
    // 48: add r4.w, r3.w, l(-1.000000)
    r4.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 49: mad r4.w, cb0[21].z, r4.w, l(1.000000)
    r4.w = ((source[21].zzzz)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mul r12.xyz, cb0[4].xyzx, cb0[4].wwww
    r12.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 51: max r13.xyz, r12.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r13.xyz = (max(r12.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 52: min r13.xyz, r13.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r13.xyz = (min(r13.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: max r12.xyz, r12.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 54: min r12.xyz, r12.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r12.xyz = (min(r12.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 55: mul r5.w, r9.y, cb0[16].y
    r5.w = ((r9.yyyy)*(source[16].yyyy)).w;
    // 56: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 57: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: movc r5.w, r10.y, l(0), r5.w
    r5.w = ((asuint(r10.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 59: add r9.xyw, -r13.xyxz, r12.xyxz
    r9.xyw = ((-(r13.xyxz))+(r12.xyxz)).xyw;
    // 60: mad r9.xyw, r5.wwww, r9.xyxw, r13.xyxz
    r9.xyw = ((r5.wwww)*(r9.xyxw)+(r13.xyxz)).xyw;
    // 61: dp3 r6.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: add r10.xyw, -r9.xyxw, r6.wwww
    r10.xyw = ((-(r9.xyxw))+(r6.wwww)).xyw;
    // 63: mad r10.xyw, cb0[17].yyyy, r10.xyxw, r9.xyxw
    r10.xyw = ((source[17].yyyy)*(r10.xyxw)+(r9.xyxw)).xyw;
    // 64: dp3 r6.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r12.xyz, -r10.xywx, r6.wwww
    r12.xyz = ((-(r10.xywx))+(r6.wwww)).xyz;
    // 66: mad r10.xyw, cb0[17].zzzz, r12.xyxz, r10.xyxw
    r10.xyw = ((source[17].zzzz)*(r12.xyxz)+(r10.xyxw)).xyw;
    // 67: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 70: mul r10.xyw, r10.xyxw, r12.xyxz
    r10.xyw = ((r10.xyxw)*(r12.xyxz)).xyw;
    // 71: dp3 r6.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 72: add r13.xyz, -r7.xyzx, r6.wwww
    r13.xyz = ((-(r7.xyzx))+(r6.wwww)).xyz;
    // 73: mad r13.xyz, cb0[17].yyyy, r13.xyzx, r7.xyzx
    r13.xyz = ((source[17].yyyy)*(r13.xyzx)+(r7.xyzx)).xyz;
    // 74: dp3 r6.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 75: add r14.xyz, -r13.xyzx, r6.wwww
    r14.xyz = ((-(r13.xyzx))+(r6.wwww)).xyz;
    // 76: mad r13.xyz, cb0[17].zzzz, r14.xyzx, r13.xyzx
    r13.xyz = ((source[17].zzzz)*(r14.xyzx)+(r13.xyzx)).xyz;
    // 77: mul r14.xyz, r10.xywx, r13.xyzx
    r14.xyz = ((r10.xywx)*(r13.xyzx)).xyz;
    // 78: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 79: mad r15.xyz, -r10.xywx, r13.xyzx, r6.wwww
    r15.xyz = ((-(r10.xywx))*(r13.xyzx)+(r6.wwww)).xyz;
    // 80: mad r14.xyz, cb0[17].yyyy, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].yyyy)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 81: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r15.xyz, -r14.xyzx, r6.wwww
    r15.xyz = ((-(r14.xyzx))+(r6.wwww)).xyz;
    // 83: mad r14.xyz, cb0[17].zzzz, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 84: mul r14.xyz, r12.xyzx, r14.xyzx
    r14.xyz = ((r12.xyzx)*(r14.xyzx)).xyz;
    // 85: mul r6.w, cb0[5].z, l(1.500000)
    r6.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: add r7.w, -cb0[5].w, l(1.000000)
    r7.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 87: mul r7.w, r7.w, cb0[19].x
    r7.w = ((r7.wwww)*(source[19].xxxx)).w;
    // 88: mul r7.w, r7.w, l(6.283185)
    r7.w = ((r7.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 89: sincos r7.w, null, r7.w
    r7.w = (sin(r7.wwww)).w;
    // 90: add r7.w, r7.w, l(1.000000)
    r7.w = ((r7.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: mul r6.w, r6.w, r7.w
    r6.w = ((r6.wwww)*(r7.wwww)).w;
    // 92: mad r6.w, r6.w, l(0.500000), cb0[5].z
    r6.w = ((r6.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 93: frc r7.w, cb0[5].x
    r7.w = (frac(source[5].xxxx)).w;
    // 94: add r8.w, -r7.w, cb0[5].x
    r8.w = ((-(r7.wwww))+(source[5].xxxx)).w;
    // 95: mul r15.z, r8.w, l(0.125000)
    r15.z = ((r8.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 96: mov r15.xw, l(0,0,0,0)
    r15.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 97: mul r15.y, cb0[5].y, cb0[14].y
    r15.y = ((source[5].yyyy)*(source[14].yyyy)).y;
    // 98: frc r8.w, v4.x
    r8.w = (frac(v4.xxxx)).w;
    // 99: mul r16.x, r8.w, l(0.125000)
    r16.x = ((r8.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 100: mov r16.y, v4.y
    r16.y = (v4.yyyy).y;
    // 101: add r15.xy, r15.xyxx, r16.xyxx
    r15.xy = ((r15.xyxx)+(r16.xyxx)).xy;
    // 102: add r15.xy, r15.xyxx, r15.zwzz
    r15.xy = ((r15.xyxx)+(r15.zwzz)).xy;
    // 103: sample_b_indexable(texture2d)(float,float,float,float) r15.xyzw, r15.xyxx, t4.xyzw, s3, l(0.000000)
    r15.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r15.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 104: mul r15.xyz, r6.wwww, r15.xyzx
    r15.xyz = ((r6.wwww)*(r15.xyzx)).xyz;
    // 105: mul r6.w, r7.w, r15.w
    r6.w = ((r7.wwww)*(r15.wwww)).w;
    // 106: mad r15.xyz, r15.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r15.xyz = ((r15.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 107: mad r14.xyz, r6.wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((r6.wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 108: mul r15.xyz, r4.wwww, r14.xyzx
    r15.xyz = ((r4.wwww)*(r14.xyzx)).xyz;
    // 109: mul r11.xyz, r11.xyzx, r15.xyzx
    r11.xyz = ((r11.xyzx)*(r15.xyzx)).xyz;
    // 110: mad r14.xyz, r4.wwww, r14.xyzx, -r11.xyzx
    r14.xyz = ((r4.wwww)*(r14.xyzx)+(-(r11.xyzx))).xyz;
    // 111: mad r11.xyz, r3.wwww, r14.xyzx, r11.xyzx
    r11.xyz = ((r3.wwww)*(r14.xyzx)+(r11.xyzx)).xyz;
    // 112: mul r8.xyz, r8.xyzx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 113: mad_sat r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = (saturate((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 114: mul r7.xyz, r7.xyzx, r9.xywx
    r7.xyz = ((r7.xyzx)*(r9.xywx)).xyz;
    // 115: add r4.w, -r7.w, l(1.000000)
    r4.w = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: add r6.w, -|r3.z|, l(1.000000)
    r6.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: dp3 r3.x, r4.xyzx, r3.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 118: add r3.y, -|r3.x|, l(1.000000)
    r3.y = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 119: mul r3.y, r3.y, r6.w
    r3.y = ((r3.yyyy)*(r6.wwww)).y;
    // 120: mul r4.x, r3.y, cb0[16].w
    r4.x = ((r3.yyyy)*(source[16].wwww)).x;
    // 121: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 122: log r4.x, |r4.x|
    r4.x = (log2(abs(r4.xxxx))).x;
    // 123: mul r4.x, r4.x, cb0[17].x
    r4.x = ((r4.xxxx)*(source[17].xxxx)).x;
    // 124: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 125: mul r9.xyw, cb0[6].xyxz, cb0[6].wwww
    r9.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 126: mul r9.xyw, r4.xxxx, r9.xyxw
    r9.xyw = ((r4.xxxx)*(r9.xyxw)).xyw;
    // 127: movc r4.xyz, r4.yyyy, l(0,0,0,0), r9.xywx
    r4.xyz = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xywx)).xyz;
    // 128: mul r9.xyw, r4.xyxz, r4.wwww
    r9.xyw = ((r4.xyxz)*(r4.wwww)).xyw;
    // 129: dp3 r6.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: mad r4.xyz, -r4.wwww, r4.xyzx, r6.wwww
    r4.xyz = ((-(r4.wwww))*(r4.xyzx)+(r6.wwww)).xyz;
    // 131: mad r4.xyz, cb0[17].yyyy, r4.xyzx, r9.xywx
    r4.xyz = ((source[17].yyyy)*(r4.xyzx)+(r9.xywx)).xyz;
    // 132: dp3 r4.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 133: add r9.xyw, -r4.xyxz, r4.wwww
    r9.xyw = ((-(r4.xyxz))+(r4.wwww)).xyw;
    // 134: mad r4.xyz, cb0[17].zzzz, r9.xywx, r4.xyzx
    r4.xyz = ((source[17].zzzz)*(r9.xywx)+(r4.xyzx)).xyz;
    // 135: mul_sat r3.xz, r3.xxzx, cb0[17].wwww
    r3.xz = (saturate((r3.xxzx)*(source[17].wwww))).xz;
    // 136: add r3.xz, -r3.xxzx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r3.xz = ((-(r3.xxzx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 137: add_sat r3.z, r3.z, -cb0[18].x
    r3.z = (saturate((r3.zzzz)+(-(source[18].xxxx)))).z;
    // 138: lt r4.w, r3.z, l(0.000001)
    r4.w = (asfloat((uint4)((r3.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: log r3.z, r3.z
    r3.z = (log2(r3.zzzz)).z;
    // 140: mul r3.z, r3.z, cb0[18].y
    r3.z = ((r3.zzzz)*(source[18].yyyy)).z;
    // 141: exp r3.z, r3.z
    r3.z = (exp2(r3.zzzz)).z;
    // 142: mul r3.x, r3.z, r3.x
    r3.x = ((r3.zzzz)*(r3.xxxx)).x;
    // 143: movc r3.x, r4.w, l(0), r3.x
    r3.x = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 144: add r3.z, r3.x, l(-1.000000)
    r3.z = ((r3.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 145: mad r3.z, cb0[9].w, r3.z, l(1.000000)
    r3.z = ((source[9].wwww)*(r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: mad r9.xyw, r3.xxxx, cb0[10].xyxz, -cb0[10].xyxz
    r9.xyw = ((r3.xxxx)*(source[10].xyxz)+(-(source[10].xyxz))).xyw;
    // 147: mad r9.xyw, cb0[10].wwww, r9.xyxw, cb0[10].xyxz
    r9.xyw = ((source[10].wwww)*(r9.xyxw)+(source[10].xyxz)).xyw;
    // 148: mad r10.xyw, r10.xyxw, r13.xyxz, l(0.010000, 0.010000, 0.000000, 0.010000)
    r10.xyw = ((r10.xyxw)*(r13.xyxz)+(float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 149: dp3 r4.w, r10.xywx, r10.xywx
    r4.w = (dot((r10.xywx).xyz,(r10.xywx).xyz).xxxx).w;
    // 150: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 151: div r10.xyw, r10.xyxw, r4.wwww
    r10.xyw = ((r10.xyxw)/(r4.wwww)).xyw;
    // 152: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 154: add r10.xyw, r10.xyxw, -r11.xyxz
    r10.xyw = ((r10.xyxw)+(-(r11.xyxz))).xyw;
    // 155: mul r11.xyz, cb0[11].xyzx, cb0[18].wwww
    r11.xyz = ((source[11].xyzx)*(source[18].wwww)).xyz;
    // 156: mul r11.xyz, r11.xyzx, cb0[20].zzzz
    r11.xyz = ((r11.xyzx)*(source[20].zzzz)).xyz;
    // 157: mul r11.xyz, r3.xxxx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r11.xyzx)).xyz;
    // 158: mad r9.xyw, r10.xyxw, r11.xyxz, r9.xyxw
    r9.xyw = ((r10.xyxw)*(r11.xyxz)+(r9.xyxw)).xyw;
    // 159: mad r9.xyw, r3.zzzz, cb0[9].xyxz, r9.xyxw
    r9.xyw = ((r3.zzzz)*(source[9].xyxz)+(r9.xyxw)).xyw;
    // 160: mad r4.xyz, r4.xyzx, r12.xyzx, r9.xywx
    r4.xyz = ((r4.xyzx)*(r12.xyzx)+(r9.xywx)).xyz;
    // 161: lt r3.x, |r3.y|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 162: log r3.y, |r3.y|
    r3.y = (log2(abs(r3.yyyy))).y;
    // 163: mul r3.y, r3.y, l(1.500000)
    r3.y = ((r3.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 164: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 165: mul r9.xyw, r3.yyyy, cb0[12].xyxz
    r9.xyw = ((r3.yyyy)*(source[12].xyxz)).xyw;
    // 166: movc r3.xyz, r3.xxxx, l(0,0,0,0), r9.xywx
    r3.xyz = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xywx)).xyz;
    // 167: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 168: mad r3.xyz, cb0[16].zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 169: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 170: mul r4.x, r9.z, cb0[22].w
    r4.x = ((r9.zzzz)*(source[22].wwww)).x;
    // 171: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 172: min r4.x, r4.x, l(1.000000)
    r4.x = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 173: movc r4.x, r10.z, l(0), r4.x
    r4.x = ((asuint(r10.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 174: max r4.x, r4.x, cb0[0].x
    r4.x = (max(r4.xxxx,source[0].xxxx)).x;
    // 175: min r4.z, r4.x, l(1.000000)
    r4.z = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 176: mul_sat r4.w, r5.w, cb2[3].w
    r4.w = (saturate((r5.wwww)*(passValues[3].wwww))).w;
    // 177: mov_sat r8.w, cb0[21].w
    r8.w = (saturate(source[21].wwww)).w;
    // 178: mul_sat r7.xyz, cb0[15].xyzx, cb0[15].wwww
    r7.xyz = (saturate((source[15].xyzx)*(source[15].wwww))).xyz;
    // 179: mov_sat r4.x, r0.w
    r4.x = (saturate(r0.wwww)).x;
    // 180: log r4.x, r4.x
    r4.x = (log2(r4.xxxx)).x;
    // 181: mul r4.x, r4.x, cb0[1].y
    r4.x = ((r4.xxxx)*(source[1].yyyy)).x;
    // 182: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 183: mad_sat r4.x, r4.x, cb0[1].w, cb0[1].z
    r4.x = (saturate((r4.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 184: mul r4.x, r4.x, cb0[23].x
    r4.x = ((r4.xxxx)*(source[23].xxxx)).x;
    // 185: mul r9.xyz, r7.xyzx, r4.xxxx
    r9.xyz = ((r7.xyzx)*(r4.xxxx)).xyz;
    // 186: add r4.x, -r4.w, l(1.000000)
    r4.x = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: mul r9.xyz, r4.xxxx, r9.xyzx
    r9.xyz = ((r4.xxxx)*(r9.xyzx)).xyz;
    // 188: dp3 r4.y, v7.xyzx, v7.xyzx
    r4.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 189: rsq r4.y, r4.y
    r4.y = (rsqrt(r4.yyyy)).y;
    // 190: mul r10.xyz, r4.yyyy, v7.xyzx
    r10.xyz = ((r4.yyyy)*(v7.xyzx)).xyz;
    // 191: dp3 r4.y, r10.xyzx, r5.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 192: mad r11.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 193: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 194: mul r11.yzw, r11.yyyy, cb0[35].xxyz
    r11.yzw = ((r11.yyyy)*(source[35].xxyz)).yzw;
    // 195: mad r11.xyz, r11.xxxx, cb0[34].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[34].xyzx)+(r11.yzwy)).xyz;
    // 196: mul r11.xyz, r11.xyzx, cb0[36].wwww
    r11.xyz = ((r11.xyzx)*(source[36].wwww)).xyz;
    // 197: mul r11.xyz, r8.xyzx, r11.xyzx
    r11.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 198: dp3 r4.y, r10.xyzx, r6.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 199: mad r12.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 200: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 201: mul r12.yzw, r12.yyyy, cb0[35].xxyz
    r12.yzw = ((r12.yyyy)*(source[35].xxyz)).yzw;
    // 202: mad r12.xyz, cb0[34].xyzx, r12.xxxx, r12.yzwy
    r12.xyz = ((source[34].xyzx)*(r12.xxxx)+(r12.yzwy)).xyz;
    // 203: mul r12.xyz, r12.xyzx, cb0[36].wwww
    r12.xyz = ((r12.xyzx)*(source[36].wwww)).xyz;
    // 204: dp3 r4.y, -r10.xyzx, r5.xyzx
    r4.y = (dot((-(r10.xyzx)).xyz,(r5.xyzx).xyz).xxxx).y;
    // 205: mad r10.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 206: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 207: mul r10.yzw, r10.yyyy, cb0[35].xxyz
    r10.yzw = ((r10.yyyy)*(source[35].xxyz)).yzw;
    // 208: mad r10.xyz, r10.xxxx, cb0[34].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[34].xyzx)+(r10.yzwy)).xyz;
    // 209: mul r10.xyz, r10.xyzx, cb0[36].wwww
    r10.xyz = ((r10.xyzx)*(source[36].wwww)).xyz;
    // 210: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 211: mul r9.xyz, r8.xyzx, r9.xyzx
    r9.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 212: mul r4.y, r8.w, l(0.080000)
    r4.y = ((r8.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 213: mad r10.xyz, -r8.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r8.xyzx
    r10.xyz = ((-(r8.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r8.xyzx)).xyz;
    // 214: mad r10.xyz, r4.wwww, r10.xyzx, r4.yyyy
    r10.xyz = ((r4.wwww)*(r10.xyzx)+(r4.yyyy)).xyz;
    // 215: deriv_rtx_coarse r13.x, r0.w
    r13.x = (ddx_coarse(r0.wwww)).x;
    // 216: deriv_rty_coarse r13.y, r0.w
    r13.y = (ddy_coarse(r0.wwww)).y;
    // 217: dp2 r4.y, r13.xyxx, r13.xyxx
    r4.y = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).y;
    // 218: sqrt r4.y, r4.y
    r4.y = (sqrt(r4.yyyy)).y;
    // 219: mad r4.y, r4.y, l(0.300000), r4.z
    r4.y = ((r4.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz)).y;
    // 220: min r13.y, r4.y, l(1.000000)
    r13.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 221: add r4.y, r6.z, l(1.000000)
    r4.y = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 222: min r4.y, r4.y, l(1.000000)
    r4.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 223: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 224: add_sat r13.x, -r4.y, r0.w
    r13.x = (saturate((-(r4.yyyy))+(r0.wwww))).x;
    // 225: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 226: add r0.w, -r13.y, l(1.000000)
    r0.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 227: max r14.xyz, r10.xyzx, r0.wwww
    r14.xyz = (max(r10.xyzx,r0.wwww)).xyz;
    // 228: add r14.xyz, -r10.xyzx, r14.xyzx
    r14.xyz = ((-(r10.xyzx))+(r14.xyzx)).xyz;
    // 229: mul_sat r0.w, r10.y, l(50.000000)
    r0.w = (saturate((r10.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 230: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 231: mul r15.xyz, r10.xyzx, r13.wwww
    r15.xyz = ((r10.xyzx)*(r13.wwww)).xyz;
    // 232: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 233: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r0.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 234: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 235: mad r15.xyz, r10.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r10.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 236: mul r16.xyz, r14.xyzx, r15.xyzx
    r16.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 237: dp3 r17.x, r1.xyzx, r6.xyzx
    r17.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 238: dp3 r17.y, r2.xyzx, r6.xyzx
    r17.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 239: dp3 r6.y, r0.xyzx, r6.xyzx
    r6.y = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 240: mul r0.w, r13.y, l(5.000000)
    r0.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 241: mul r13.zw, cb0[25].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r13.zw = ((source[25].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 242: dp2 r6.x, r17.xyxx, r13.zwzz
    r6.x = (dot((r17.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 243: dp2 r6.z, r17.xyxx, cb0[25].xyxx
    r6.z = (dot((r17.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 244: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r6.xyzx, t6.xyzw, s5, r0.w
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r0.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 245: mul r6.xyz, r6.xyzx, r6.wwww
    r6.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 246: mul r6.xyz, r6.xyzx, cb0[24].xyzx
    r6.xyz = ((r6.xyzx)*(source[24].xyzx)).xyz;
    // 247: mul r6.xyz, r6.xyzx, cb0[25].zzzz
    r6.xyz = ((r6.xyzx)*(source[25].zzzz)).xyz;
    // 248: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[24].wwww)).xyz;
    // 249: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 250: add r6.xyz, -r0.wwww, r6.xyzx
    r6.xyz = ((-(r0.wwww))+(r6.xyzx)).xyz;
    // 251: mad r6.xyz, r6.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r0.wwww
    r6.xyz = ((r6.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r0.wwww)).xyz;
    // 252: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 253: mad r4.y, r13.y, l(2.000000), l(2.000000)
    r4.y = ((r13.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).y;
    // 254: div r0.w, r0.w, r4.y
    r0.w = ((r0.wwww)/(r4.yyyy)).w;
    // 255: dp3 r5.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 256: mad r0.w, r5.w, l(5.000000), r0.w
    r0.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.wwww)).w;
    // 257: add_sat r0.w, r4.w, r0.w
    r0.w = (saturate((r4.wwww)+(r0.wwww))).w;
    // 258: mad r6.w, r0.w, l(-2.000000), l(3.000000)
    r6.w = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 259: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 260: mul r0.w, r0.w, r6.w
    r0.w = ((r0.wwww)*(r6.wwww)).w;
    // 261: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 262: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 263: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 264: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 265: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 266: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 267: dp3 r0.y, r0.xyzx, r5.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 268: dp2 r0.x, r1.xyxx, r13.zwzz
    r0.x = (dot((r1.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 269: dp2 r0.z, r1.xyxx, cb0[25].xyxx
    r0.z = (dot((r1.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 270: mov r0.w, l(1.000000)
    r0.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 271: dp4 r2.x, cb0[26].xyzw, r0.xyzw
    r2.x = (dot((source[26].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 272: dp4 r2.y, cb0[27].xyzw, r0.xyzw
    r2.y = (dot((source[27].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).y;
    // 273: dp4 r2.z, cb0[28].xyzw, r0.xyzw
    r2.z = (dot((source[28].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).z;
    // 274: mul r17.xyzw, r0.yzzx, r0.xyzz
    r17.xyzw = ((r0.yzzx)*(r0.xyzz)).xyzw;
    // 275: dp4 r5.x, cb0[29].xyzw, r17.xyzw
    r5.x = (dot((source[29].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).x;
    // 276: dp4 r5.y, cb0[30].xyzw, r17.xyzw
    r5.y = (dot((source[30].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).y;
    // 277: dp4 r5.z, cb0[31].xyzw, r17.xyzw
    r5.z = (dot((source[31].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).z;
    // 278: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 279: mad r0.x, r0.x, r0.x, -r0.z
    r0.x = ((r0.xxxx)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 280: add r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 281: mad r0.xzw, cb0[32].xxyz, r0.xxxx, r2.xxyz
    r0.xzw = ((source[32].xxyz)*(r0.xxxx)+(r2.xxyz)).xzw;
    // 282: max r0.xzw, r0.xxzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xzw = (max(r0.xxzw,float4(0.000000,0.000000,0.000000,0.000000))).xzw;
    // 283: mul r0.xzw, r0.xxzw, cb0[24].xxyz
    r0.xzw = ((r0.xxzw)*(source[24].xxyz)).xzw;
    // 284: mul r0.xzw, r0.xxzw, cb0[25].zzzz
    r0.xzw = ((r0.xxzw)*(source[25].zzzz)).xzw;
    // 285: mad r0.xzw, r0.xxzw, l(3.141593, 0.000000, 3.141593, 3.141593), cb0[24].wwww
    r0.xzw = ((r0.xxzw)*(float4(3.141593,0.000000,3.141593,3.141593))+(source[24].wwww)).xzw;
    // 286: dp3 r2.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 287: add r0.xzw, r0.xxzw, -r2.xxxx
    r0.xzw = ((r0.xxzw)+(-(r2.xxxx))).xzw;
    // 288: mad r0.xzw, r0.xxzw, l(0.800000, 0.000000, 0.800000, 0.800000), r2.xxxx
    r0.xzw = ((r0.xxzw)*(float4(0.800000,0.000000,0.800000,0.800000))+(r2.xxxx)).xzw;
    // 289: dp3 r2.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 290: div r2.x, r2.x, r4.y
    r2.x = ((r2.xxxx)/(r4.yyyy)).x;
    // 291: mad r2.x, r5.w, l(5.000000), r2.x
    r2.x = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.xxxx)).x;
    // 292: add_sat r2.x, r4.w, r2.x
    r2.x = (saturate((r4.wwww)+(r2.xxxx))).x;
    // 293: mad r2.y, r2.x, l(-2.000000), l(3.000000)
    r2.y = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 294: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 295: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 296: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 297: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 298: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 299: mul r0.xzw, r0.xxzw, r2.xxxx
    r0.xzw = ((r0.xxzw)*(r2.xxxx)).xzw;
    // 300: mul r2.x, r13.y, r13.y
    r2.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 301: dp3 r2.y, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 302: add r2.z, r3.w, r13.x
    r2.z = ((r3.wwww)+(r13.xxxx)).z;
    // 303: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 304: mul r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)*(r2.xxxx)).x;
    // 305: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 306: add r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)+(r2.xxxx)).x;
    // 307: add_sat r2.x, r2.x, l(-1.000000)
    r2.x = (saturate((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 308: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 309: mad r2.y, r2.x, r5.x, r5.y
    r2.y = ((r2.xxxx)*(r5.xxxx)+(r5.yyyy)).y;
    // 310: mad r2.y, r2.y, r2.x, r5.z
    r2.y = ((r2.yyyy)*(r2.xxxx)+(r5.zzzz)).y;
    // 311: mul r2.y, r2.x, r2.y
    r2.y = ((r2.xxxx)*(r2.yyyy)).y;
    // 312: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 313: mad r5.xyz, r8.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r8.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 314: mad r10.xyz, r8.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r8.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 315: mad r13.xyz, r8.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r8.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 316: mad r5.xyz, r3.wwww, r5.xyzx, r10.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(r10.xyzx)).xyz;
    // 317: mad r5.xyz, r5.xyzx, r3.wwww, r13.xyzx
    r5.xyz = ((r5.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 318: mul r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 319: max r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = (max(r3.wwww,r5.xyzx)).xyz;
    // 320: mul r10.xyz, r2.xxxx, r12.xyzx
    r10.xyz = ((r2.xxxx)*(r12.xyzx)).xyz;
    // 321: mul r11.xyz, r5.xyzx, r11.xyzx
    r11.xyz = ((r5.xyzx)*(r11.xyzx)).xyz;
    // 322: mul r10.xyz, r6.xyzx, r10.xyzx
    r10.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 323: mul r10.xyz, r10.xyzx, r16.xyzx
    r10.xyz = ((r10.xyzx)*(r16.xyzx)).xyz;
    // 324: mul r12.xyz, r10.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r12.xyz = ((r10.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 325: mad r13.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 326: mul r11.xyz, r11.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 327: mul r14.xyz, r0.xzwx, r13.xyzx
    r14.xyz = ((r0.xzwx)*(r13.xyzx)).xyz;
    // 328: mul r11.xyz, r11.xyzx, r14.xyzx
    r11.xyz = ((r11.xyzx)*(r14.xyzx)).xyz;
    // 329: mad r11.xyz, -r11.xyzx, r4.wwww, r11.xyzx
    r11.xyz = ((-(r11.xyzx))*(r4.wwww)+(r11.xyzx)).xyz;
    // 330: mul r6.xyz, r6.xyzx, r16.xyzx
    r6.xyz = ((r6.xyzx)*(r16.xyzx)).xyz;
    // 331: mul r13.xyz, r8.xyzx, r13.xyzx
    r13.xyz = ((r8.xyzx)*(r13.xyzx)).xyz;
    // 332: mul r13.xyz, r4.xxxx, r13.xyzx
    r13.xyz = ((r4.xxxx)*(r13.xyzx)).xyz;
    // 333: mul r0.xzw, r0.xxzw, r13.xxyz
    r0.xzw = ((r0.xxzw)*(r13.xxyz)).xzw;
    // 334: mul r0.xzw, r5.xxyz, r0.xxzw
    r0.xzw = ((r5.xxyz)*(r0.xxzw)).xzw;
    // 335: mad r0.xzw, r6.xxyz, r2.xxxx, r0.xxzw
    r0.xzw = ((r6.xxyz)*(r2.xxxx)+(r0.xxzw)).xzw;
    // 336: mad r0.xzw, r0.xxzw, l(0.400000, 0.000000, 0.400000, 0.400000), r11.xxyz
    r0.xzw = ((r0.xxzw)*(float4(0.400000,0.000000,0.400000,0.400000))+(r11.xxyz)).xzw;
    // 337: mad r2.xyz, r9.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r2.xyz = ((r9.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 338: mad r0.xzw, r10.xxyz, l(0.600000, 0.000000, 0.600000, 0.600000), r0.xxzw
    r0.xzw = ((r10.xxyz)*(float4(0.600000,0.000000,0.600000,0.600000))+(r0.xxzw)).xzw;
    // 339: add r2.xyz, r0.xzwx, r2.xyzx
    r2.xyz = ((r0.xzwx)+(r2.xyzx)).xyz;
    // 340: mad r2.xyz, r8.xyzx, cb0[36].xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)*(source[36].xyzx)+(r2.xyzx)).xyz;
    // 341: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 342: mad r3.x, r3.x, l(-0.250000), l(0.400000)
    r3.x = ((r3.xxxx)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).x;
    // 343: eq r3.y, cb0[37].x, l(0.000000)
    r3.y = (asfloat((uint4)((source[37].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 344: not r3.z, r3.y
    r3.z = (asfloat(~asuint(r3.yyyy))).z;
    // 345: lt r4.x, r2.w, r3.x
    r4.x = (asfloat((uint4)((r2.wwww)<(r3.xxxx)) * 0xffffffffu)).x;
    // 346: and r3.z, r3.z, r4.x
    r3.z = (asfloat(asuint(r3.zzzz) & asuint(r4.xxxx))).z;
    // 347: discard_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 348: ge r3.x, r2.w, r3.x
    r3.x = (asfloat((uint4)((r2.wwww)>=(r3.xxxx)) * 0xffffffffu)).x;
    // 349: mad r1.w, r1.w, cb0[2].x, l(-0.900000)
    r1.w = ((r1.wwww)*(source[2].xxxx)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 350: mul_sat r1.w, r1.w, l(9.999998)
    r1.w = (saturate((r1.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 351: mad r3.z, r1.w, l(-2.000000), l(3.000000)
    r3.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 352: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 353: mul r1.w, r1.w, r3.z
    r1.w = ((r1.wwww)*(r3.zzzz)).w;
    // 354: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 355: movc r1.w, r3.x, r1.w, r2.w
    r1.w = ((asuint(r3.xxxx) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 356: movc o0.w, r3.y, r1.w, r2.w
    output.targets[0].w = ((asuint(r3.yyyy) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 357: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 358: mov r1.z, r0.y
    r1.z = (r0.yyyy).z;
    // 359: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 360: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 361: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 362: dp3 r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r0.y = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).y;
    // 363: div r1.xy, r1.xyxx, r0.yyyy
    r1.xy = ((r1.xyxx)/(r0.yyyy)).xy;
    // 364: ge r0.y, l(0.000000), r1.z
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).y;
    // 365: ge r1.zw, r1.xxxy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.zw = (asfloat((uint4)((r1.xxxy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).zw;
    // 366: movc r1.zw, r1.zzzw, l(0,0,1.000000,1.000000), l(0,0,-1.000000,-1.000000)
    r1.zw = ((asuint(r1.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),1.000000,1.000000)) : (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000))).zw;
    // 367: mad r1.zw, -|r1.yyyx|, r1.zzzw, r1.zzzw
    r1.zw = ((-(abs(r1.yyyx)))*(r1.zzzw)+(r1.zzzw)).zw;
    // 368: movc r1.xy, r0.yyyy, r1.zwzz, r1.xyxx
    r1.xy = ((asuint(r0.yyyy) != 0u) ? (r1.zwzz) : (r1.xyxx)).xy;
    // 369: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 370: dp3 o4.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 371: dp3 o4.y, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 372: mul r0.xyz, r7.xyzx, cb0[23].xxxx
    r0.xyz = ((r7.xyzx)*(source[23].xxxx)).xyz;
    // 373: ftou r0.w, cb0[33].z
    r0.w = (asfloat((uint4)(source[33].zzzz))).w;
    // 374: and r0.w, r0.w, l(31)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(31u,31u,31u,31u))).w;
    // 375: utof r0.w, r0.w
    r0.w = ((float4)(asuint(r0.wwww))).w;
    // 376: mul o5.w, r0.w, l(0.003922)
    output.targets[5].w = ((r0.wwww)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 377: dp3_sat o5.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 378: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 379: mov o3.xyzw, r8.xyzw
    output.targets[3].xyzw = (r8.xyzw).xyzw;
    // 380: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 381: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 382: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 383: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drr-01-neck-st-mi-fx-dead.v1 / source program e56633f592e0154eb0794435cf3c2717
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase111(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19].x=(g_SourceCharacterTime.xxxx).x;
    source[19].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[19].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[20].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[20].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[20].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[24] = g_SourceCharacterEnvironmentColor;
        source[25] = g_SourceCharacterEnvironmentRotation;
    }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
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
    // 15: mul r5.xy, r4.xyxx, cb0[16].xxxx
    r5.xy = ((r4.xyxx)*(source[16].xxxx)).xy;
    // 16: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 17: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 19: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 20: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 22: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 23: div r4.xyz, r5.xyzx, r0.wwww
    r4.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 24: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 27: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 28: mul r6.xyz, r0.wwww, r5.xyzx
    r6.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 29: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r8.xy, v4.xyxx, cb0[22].xxxx
    r8.xy = ((v4.xyxx)*(source[22].xxxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r8.xyxx, t2.yzwx, s4, l(0.000000)
    r1.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 33: add r1.w, r1.w, -cb0[22].y
    r1.w = ((r1.wwww)+(-(source[22].yyyy))).w;
    // 34: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 35: mul_sat r1.w, r1.w, r7.w
    r1.w = (saturate((r1.wwww)*(r7.wwww))).w;
    // 36: mul_sat r1.w, r1.w, cb0[22].z
    r1.w = (saturate((r1.wwww)*(source[22].zzzz))).w;
    // 37: mul r2.w, r1.w, cb0[2].x
    r2.w = ((r1.wwww)*(source[2].xxxx)).w;
    // 38: add r8.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: lt r10.xyz, |r9.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (asfloat((uint4)((abs(r9.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 41: log r9.xyz, |r9.xzyx|
    r9.xyz = (log2(abs(r9.xzyx))).xyz;
    // 42: mul r3.w, r9.x, cb0[20].w
    r3.w = ((r9.xxxx)*(source[20].wwww)).w;
    // 43: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 44: movc r3.w, r10.x, l(0), r3.w
    r3.w = ((asuint(r10.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 45: add_sat r3.w, r3.w, cb0[21].x
    r3.w = (saturate((r3.wwww)+(source[21].xxxx))).w;
    // 46: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mul r11.xyz, r4.wwww, cb0[13].xyzx
    r11.xyz = ((r4.wwww)*(source[13].xyzx)).xyz;
    // 48: add r4.w, r3.w, l(-1.000000)
    r4.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 49: mad r4.w, cb0[21].z, r4.w, l(1.000000)
    r4.w = ((source[21].zzzz)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mul r12.xyz, cb0[4].xyzx, cb0[4].wwww
    r12.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 51: max r13.xyz, r12.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r13.xyz = (max(r12.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 52: min r13.xyz, r13.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r13.xyz = (min(r13.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: max r12.xyz, r12.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 54: min r12.xyz, r12.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r12.xyz = (min(r12.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 55: mul r5.w, r9.y, cb0[16].y
    r5.w = ((r9.yyyy)*(source[16].yyyy)).w;
    // 56: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 57: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: movc r5.w, r10.y, l(0), r5.w
    r5.w = ((asuint(r10.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).w;
    // 59: add r9.xyw, -r13.xyxz, r12.xyxz
    r9.xyw = ((-(r13.xyxz))+(r12.xyxz)).xyw;
    // 60: mad r9.xyw, r5.wwww, r9.xyxw, r13.xyxz
    r9.xyw = ((r5.wwww)*(r9.xyxw)+(r13.xyxz)).xyw;
    // 61: dp3 r6.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: add r10.xyw, -r9.xyxw, r6.wwww
    r10.xyw = ((-(r9.xyxw))+(r6.wwww)).xyw;
    // 63: mad r10.xyw, cb0[17].yyyy, r10.xyxw, r9.xyxw
    r10.xyw = ((source[17].yyyy)*(r10.xyxw)+(r9.xyxw)).xyw;
    // 64: dp3 r6.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r12.xyz, -r10.xywx, r6.wwww
    r12.xyz = ((-(r10.xywx))+(r6.wwww)).xyz;
    // 66: mad r10.xyw, cb0[17].zzzz, r12.xyxz, r10.xyxw
    r10.xyw = ((source[17].zzzz)*(r12.xyxz)+(r10.xyxw)).xyw;
    // 67: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 70: mul r10.xyw, r10.xyxw, r12.xyxz
    r10.xyw = ((r10.xyxw)*(r12.xyxz)).xyw;
    // 71: dp3 r6.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 72: add r13.xyz, -r7.xyzx, r6.wwww
    r13.xyz = ((-(r7.xyzx))+(r6.wwww)).xyz;
    // 73: mad r13.xyz, cb0[17].yyyy, r13.xyzx, r7.xyzx
    r13.xyz = ((source[17].yyyy)*(r13.xyzx)+(r7.xyzx)).xyz;
    // 74: dp3 r6.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 75: add r14.xyz, -r13.xyzx, r6.wwww
    r14.xyz = ((-(r13.xyzx))+(r6.wwww)).xyz;
    // 76: mad r13.xyz, cb0[17].zzzz, r14.xyzx, r13.xyzx
    r13.xyz = ((source[17].zzzz)*(r14.xyzx)+(r13.xyzx)).xyz;
    // 77: mul r14.xyz, r10.xywx, r13.xyzx
    r14.xyz = ((r10.xywx)*(r13.xyzx)).xyz;
    // 78: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 79: mad r15.xyz, -r10.xywx, r13.xyzx, r6.wwww
    r15.xyz = ((-(r10.xywx))*(r13.xyzx)+(r6.wwww)).xyz;
    // 80: mad r14.xyz, cb0[17].yyyy, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].yyyy)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 81: dp3 r6.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r15.xyz, -r14.xyzx, r6.wwww
    r15.xyz = ((-(r14.xyzx))+(r6.wwww)).xyz;
    // 83: mad r14.xyz, cb0[17].zzzz, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 84: mul r14.xyz, r12.xyzx, r14.xyzx
    r14.xyz = ((r12.xyzx)*(r14.xyzx)).xyz;
    // 85: mul r6.w, cb0[5].z, l(1.500000)
    r6.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 86: add r7.w, -cb0[5].w, l(1.000000)
    r7.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 87: mul r7.w, r7.w, cb0[19].x
    r7.w = ((r7.wwww)*(source[19].xxxx)).w;
    // 88: mul r7.w, r7.w, l(6.283185)
    r7.w = ((r7.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 89: sincos r7.w, null, r7.w
    r7.w = (sin(r7.wwww)).w;
    // 90: add r7.w, r7.w, l(1.000000)
    r7.w = ((r7.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: mul r6.w, r6.w, r7.w
    r6.w = ((r6.wwww)*(r7.wwww)).w;
    // 92: mad r6.w, r6.w, l(0.500000), cb0[5].z
    r6.w = ((r6.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 93: frc r7.w, cb0[5].x
    r7.w = (frac(source[5].xxxx)).w;
    // 94: add r8.w, -r7.w, cb0[5].x
    r8.w = ((-(r7.wwww))+(source[5].xxxx)).w;
    // 95: mul r15.z, r8.w, l(0.125000)
    r15.z = ((r8.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 96: mov r15.xw, l(0,0,0,0)
    r15.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 97: mul r15.y, cb0[5].y, cb0[14].y
    r15.y = ((source[5].yyyy)*(source[14].yyyy)).y;
    // 98: frc r8.w, v4.x
    r8.w = (frac(v4.xxxx)).w;
    // 99: mul r16.x, r8.w, l(0.125000)
    r16.x = ((r8.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 100: mov r16.y, v4.y
    r16.y = (v4.yyyy).y;
    // 101: add r15.xy, r15.xyxx, r16.xyxx
    r15.xy = ((r15.xyxx)+(r16.xyxx)).xy;
    // 102: add r15.xy, r15.xyxx, r15.zwzz
    r15.xy = ((r15.xyxx)+(r15.zwzz)).xy;
    // 103: sample_b_indexable(texture2d)(float,float,float,float) r15.xyzw, r15.xyxx, t4.xyzw, s3, l(0.000000)
    r15.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r15.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 104: mul r15.xyz, r6.wwww, r15.xyzx
    r15.xyz = ((r6.wwww)*(r15.xyzx)).xyz;
    // 105: mul r6.w, r7.w, r15.w
    r6.w = ((r7.wwww)*(r15.wwww)).w;
    // 106: mad r15.xyz, r15.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r15.xyz = ((r15.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 107: mad r14.xyz, r6.wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((r6.wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 108: mul r15.xyz, r4.wwww, r14.xyzx
    r15.xyz = ((r4.wwww)*(r14.xyzx)).xyz;
    // 109: mul r11.xyz, r11.xyzx, r15.xyzx
    r11.xyz = ((r11.xyzx)*(r15.xyzx)).xyz;
    // 110: mad r14.xyz, r4.wwww, r14.xyzx, -r11.xyzx
    r14.xyz = ((r4.wwww)*(r14.xyzx)+(-(r11.xyzx))).xyz;
    // 111: mad r11.xyz, r3.wwww, r14.xyzx, r11.xyzx
    r11.xyz = ((r3.wwww)*(r14.xyzx)+(r11.xyzx)).xyz;
    // 112: mul r8.xyz, r8.xyzx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 113: mad_sat r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = (saturate((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 114: mul r7.xyz, r7.xyzx, r9.xywx
    r7.xyz = ((r7.xyzx)*(r9.xywx)).xyz;
    // 115: add r4.w, -r7.w, l(1.000000)
    r4.w = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: add r6.w, -|r3.z|, l(1.000000)
    r6.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: dp3 r3.x, r4.xyzx, r3.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 118: add r3.y, -|r3.x|, l(1.000000)
    r3.y = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 119: mul r3.y, r3.y, r6.w
    r3.y = ((r3.yyyy)*(r6.wwww)).y;
    // 120: mul r4.x, r3.y, cb0[16].w
    r4.x = ((r3.yyyy)*(source[16].wwww)).x;
    // 121: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 122: log r4.x, |r4.x|
    r4.x = (log2(abs(r4.xxxx))).x;
    // 123: mul r4.x, r4.x, cb0[17].x
    r4.x = ((r4.xxxx)*(source[17].xxxx)).x;
    // 124: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 125: mul r9.xyw, cb0[6].xyxz, cb0[6].wwww
    r9.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 126: mul r9.xyw, r4.xxxx, r9.xyxw
    r9.xyw = ((r4.xxxx)*(r9.xyxw)).xyw;
    // 127: movc r4.xyz, r4.yyyy, l(0,0,0,0), r9.xywx
    r4.xyz = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xywx)).xyz;
    // 128: mul r9.xyw, r4.xyxz, r4.wwww
    r9.xyw = ((r4.xyxz)*(r4.wwww)).xyw;
    // 129: dp3 r6.w, r9.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r9.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: mad r4.xyz, -r4.wwww, r4.xyzx, r6.wwww
    r4.xyz = ((-(r4.wwww))*(r4.xyzx)+(r6.wwww)).xyz;
    // 131: mad r4.xyz, cb0[17].yyyy, r4.xyzx, r9.xywx
    r4.xyz = ((source[17].yyyy)*(r4.xyzx)+(r9.xywx)).xyz;
    // 132: dp3 r4.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 133: add r9.xyw, -r4.xyxz, r4.wwww
    r9.xyw = ((-(r4.xyxz))+(r4.wwww)).xyw;
    // 134: mad r4.xyz, cb0[17].zzzz, r9.xywx, r4.xyzx
    r4.xyz = ((source[17].zzzz)*(r9.xywx)+(r4.xyzx)).xyz;
    // 135: mul_sat r3.xz, r3.xxzx, cb0[17].wwww
    r3.xz = (saturate((r3.xxzx)*(source[17].wwww))).xz;
    // 136: add r3.xz, -r3.xxzx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r3.xz = ((-(r3.xxzx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 137: add_sat r3.z, r3.z, -cb0[18].x
    r3.z = (saturate((r3.zzzz)+(-(source[18].xxxx)))).z;
    // 138: lt r4.w, r3.z, l(0.000001)
    r4.w = (asfloat((uint4)((r3.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: log r3.z, r3.z
    r3.z = (log2(r3.zzzz)).z;
    // 140: mul r3.z, r3.z, cb0[18].y
    r3.z = ((r3.zzzz)*(source[18].yyyy)).z;
    // 141: exp r3.z, r3.z
    r3.z = (exp2(r3.zzzz)).z;
    // 142: mul r3.x, r3.z, r3.x
    r3.x = ((r3.zzzz)*(r3.xxxx)).x;
    // 143: movc r3.x, r4.w, l(0), r3.x
    r3.x = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 144: add r3.z, r3.x, l(-1.000000)
    r3.z = ((r3.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 145: mad r3.z, cb0[9].w, r3.z, l(1.000000)
    r3.z = ((source[9].wwww)*(r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: mad r9.xyw, r3.xxxx, cb0[10].xyxz, -cb0[10].xyxz
    r9.xyw = ((r3.xxxx)*(source[10].xyxz)+(-(source[10].xyxz))).xyw;
    // 147: mad r9.xyw, cb0[10].wwww, r9.xyxw, cb0[10].xyxz
    r9.xyw = ((source[10].wwww)*(r9.xyxw)+(source[10].xyxz)).xyw;
    // 148: mad r10.xyw, r10.xyxw, r13.xyxz, l(0.010000, 0.010000, 0.000000, 0.010000)
    r10.xyw = ((r10.xyxw)*(r13.xyxz)+(float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 149: dp3 r4.w, r10.xywx, r10.xywx
    r4.w = (dot((r10.xywx).xyz,(r10.xywx).xyz).xxxx).w;
    // 150: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 151: div r10.xyw, r10.xyxw, r4.wwww
    r10.xyw = ((r10.xyxw)/(r4.wwww)).xyw;
    // 152: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 154: add r10.xyw, r10.xyxw, -r11.xyxz
    r10.xyw = ((r10.xyxw)+(-(r11.xyxz))).xyw;
    // 155: mul r11.xyz, cb0[11].xyzx, cb0[18].wwww
    r11.xyz = ((source[11].xyzx)*(source[18].wwww)).xyz;
    // 156: mul r11.xyz, r11.xyzx, cb0[20].zzzz
    r11.xyz = ((r11.xyzx)*(source[20].zzzz)).xyz;
    // 157: mul r11.xyz, r3.xxxx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r11.xyzx)).xyz;
    // 158: mad r9.xyw, r10.xyxw, r11.xyxz, r9.xyxw
    r9.xyw = ((r10.xyxw)*(r11.xyxz)+(r9.xyxw)).xyw;
    // 159: mad r9.xyw, r3.zzzz, cb0[9].xyxz, r9.xyxw
    r9.xyw = ((r3.zzzz)*(source[9].xyxz)+(r9.xyxw)).xyw;
    // 160: mad r4.xyz, r4.xyzx, r12.xyzx, r9.xywx
    r4.xyz = ((r4.xyzx)*(r12.xyzx)+(r9.xywx)).xyz;
    // 161: lt r3.x, |r3.y|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 162: log r3.y, |r3.y|
    r3.y = (log2(abs(r3.yyyy))).y;
    // 163: mul r3.y, r3.y, l(1.500000)
    r3.y = ((r3.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 164: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 165: mul r9.xyw, r3.yyyy, cb0[12].xyxz
    r9.xyw = ((r3.yyyy)*(source[12].xyxz)).xyw;
    // 166: movc r3.xyz, r3.xxxx, l(0,0,0,0), r9.xywx
    r3.xyz = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xywx)).xyz;
    // 167: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 168: mad r3.xyz, cb0[16].zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 169: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 170: mul r4.x, r9.z, cb0[22].w
    r4.x = ((r9.zzzz)*(source[22].wwww)).x;
    // 171: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 172: min r4.x, r4.x, l(1.000000)
    r4.x = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 173: movc r4.x, r10.z, l(0), r4.x
    r4.x = ((asuint(r10.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 174: max r4.x, r4.x, cb0[0].x
    r4.x = (max(r4.xxxx,source[0].xxxx)).x;
    // 175: min r4.z, r4.x, l(1.000000)
    r4.z = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 176: mul_sat r4.w, r5.w, cb2[3].w
    r4.w = (saturate((r5.wwww)*(passValues[3].wwww))).w;
    // 177: mov_sat r8.w, cb0[21].w
    r8.w = (saturate(source[21].wwww)).w;
    // 178: mul_sat r7.xyz, cb0[15].xyzx, cb0[15].wwww
    r7.xyz = (saturate((source[15].xyzx)*(source[15].wwww))).xyz;
    // 179: mov_sat r4.x, r0.w
    r4.x = (saturate(r0.wwww)).x;
    // 180: log r4.x, r4.x
    r4.x = (log2(r4.xxxx)).x;
    // 181: mul r4.x, r4.x, cb0[1].y
    r4.x = ((r4.xxxx)*(source[1].yyyy)).x;
    // 182: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 183: mad_sat r4.x, r4.x, cb0[1].w, cb0[1].z
    r4.x = (saturate((r4.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 184: mul r4.x, r4.x, cb0[23].x
    r4.x = ((r4.xxxx)*(source[23].xxxx)).x;
    // 185: mul r9.xyz, r7.xyzx, r4.xxxx
    r9.xyz = ((r7.xyzx)*(r4.xxxx)).xyz;
    // 186: add r4.x, -r4.w, l(1.000000)
    r4.x = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: mul r9.xyz, r4.xxxx, r9.xyzx
    r9.xyz = ((r4.xxxx)*(r9.xyzx)).xyz;
    // 188: dp3 r4.y, v7.xyzx, v7.xyzx
    r4.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 189: rsq r4.y, r4.y
    r4.y = (rsqrt(r4.yyyy)).y;
    // 190: mul r10.xyz, r4.yyyy, v7.xyzx
    r10.xyz = ((r4.yyyy)*(v7.xyzx)).xyz;
    // 191: dp3 r4.y, r10.xyzx, r5.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 192: mad r11.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r11.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 193: mul r11.xy, r11.xyxx, r11.xyxx
    r11.xy = ((r11.xyxx)*(r11.xyxx)).xy;
    // 194: mul r11.yzw, r11.yyyy, cb0[35].xxyz
    r11.yzw = ((r11.yyyy)*(source[35].xxyz)).yzw;
    // 195: mad r11.xyz, r11.xxxx, cb0[34].xyzx, r11.yzwy
    r11.xyz = ((r11.xxxx)*(source[34].xyzx)+(r11.yzwy)).xyz;
    // 196: mul r11.xyz, r11.xyzx, cb0[36].wwww
    r11.xyz = ((r11.xyzx)*(source[36].wwww)).xyz;
    // 197: mul r11.xyz, r8.xyzx, r11.xyzx
    r11.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 198: dp3 r4.y, r10.xyzx, r6.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 199: mad r12.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 200: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 201: mul r12.yzw, r12.yyyy, cb0[35].xxyz
    r12.yzw = ((r12.yyyy)*(source[35].xxyz)).yzw;
    // 202: mad r12.xyz, cb0[34].xyzx, r12.xxxx, r12.yzwy
    r12.xyz = ((source[34].xyzx)*(r12.xxxx)+(r12.yzwy)).xyz;
    // 203: mul r12.xyz, r12.xyzx, cb0[36].wwww
    r12.xyz = ((r12.xyzx)*(source[36].wwww)).xyz;
    // 204: dp3 r4.y, -r10.xyzx, r5.xyzx
    r4.y = (dot((-(r10.xyzx)).xyz,(r5.xyzx).xyz).xxxx).y;
    // 205: mad r10.xy, r4.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r4.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 206: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 207: mul r10.yzw, r10.yyyy, cb0[35].xxyz
    r10.yzw = ((r10.yyyy)*(source[35].xxyz)).yzw;
    // 208: mad r10.xyz, r10.xxxx, cb0[34].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[34].xyzx)+(r10.yzwy)).xyz;
    // 209: mul r10.xyz, r10.xyzx, cb0[36].wwww
    r10.xyz = ((r10.xyzx)*(source[36].wwww)).xyz;
    // 210: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 211: mul r9.xyz, r8.xyzx, r9.xyzx
    r9.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 212: mul r4.y, r8.w, l(0.080000)
    r4.y = ((r8.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 213: mad r10.xyz, -r8.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r8.xyzx
    r10.xyz = ((-(r8.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r8.xyzx)).xyz;
    // 214: mad r10.xyz, r4.wwww, r10.xyzx, r4.yyyy
    r10.xyz = ((r4.wwww)*(r10.xyzx)+(r4.yyyy)).xyz;
    // 215: deriv_rtx_coarse r13.x, r0.w
    r13.x = (ddx_coarse(r0.wwww)).x;
    // 216: deriv_rty_coarse r13.y, r0.w
    r13.y = (ddy_coarse(r0.wwww)).y;
    // 217: dp2 r4.y, r13.xyxx, r13.xyxx
    r4.y = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).y;
    // 218: sqrt r4.y, r4.y
    r4.y = (sqrt(r4.yyyy)).y;
    // 219: mad r4.y, r4.y, l(0.300000), r4.z
    r4.y = ((r4.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz)).y;
    // 220: min r13.y, r4.y, l(1.000000)
    r13.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 221: add r4.y, r6.z, l(1.000000)
    r4.y = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 222: min r4.y, r4.y, l(1.000000)
    r4.y = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 223: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 224: add_sat r13.x, -r4.y, r0.w
    r13.x = (saturate((-(r4.yyyy))+(r0.wwww))).x;
    // 225: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 226: add r0.w, -r13.y, l(1.000000)
    r0.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 227: max r14.xyz, r10.xyzx, r0.wwww
    r14.xyz = (max(r10.xyzx,r0.wwww)).xyz;
    // 228: add r14.xyz, -r10.xyzx, r14.xyzx
    r14.xyz = ((-(r10.xyzx))+(r14.xyzx)).xyz;
    // 229: mul_sat r0.w, r10.y, l(50.000000)
    r0.w = (saturate((r10.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 230: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 231: mul r15.xyz, r10.xyzx, r13.wwww
    r15.xyz = ((r10.xyzx)*(r13.wwww)).xyz;
    // 232: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 233: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r0.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 234: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 235: mad r15.xyz, r10.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r10.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 236: mul r16.xyz, r14.xyzx, r15.xyzx
    r16.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 237: dp3 r17.x, r1.xyzx, r6.xyzx
    r17.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 238: dp3 r17.y, r2.xyzx, r6.xyzx
    r17.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 239: dp3 r6.y, r0.xyzx, r6.xyzx
    r6.y = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 240: mul r0.w, r13.y, l(5.000000)
    r0.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 241: mul r13.zw, cb0[25].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r13.zw = ((source[25].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 242: dp2 r6.x, r17.xyxx, r13.zwzz
    r6.x = (dot((r17.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 243: dp2 r6.z, r17.xyxx, cb0[25].xyxx
    r6.z = (dot((r17.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 244: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r6.xyzx, t6.xyzw, s5, r0.w
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r0.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 245: mul r6.xyz, r6.xyzx, r6.wwww
    r6.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 246: mul r6.xyz, r6.xyzx, cb0[24].xyzx
    r6.xyz = ((r6.xyzx)*(source[24].xyzx)).xyz;
    // 247: mul r6.xyz, r6.xyzx, cb0[25].zzzz
    r6.xyz = ((r6.xyzx)*(source[25].zzzz)).xyz;
    // 248: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[24].wwww)).xyz;
    // 249: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 250: add r6.xyz, -r0.wwww, r6.xyzx
    r6.xyz = ((-(r0.wwww))+(r6.xyzx)).xyz;
    // 251: mad r6.xyz, r6.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r0.wwww
    r6.xyz = ((r6.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r0.wwww)).xyz;
    // 252: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 253: mad r4.y, r13.y, l(2.000000), l(2.000000)
    r4.y = ((r13.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).y;
    // 254: div r0.w, r0.w, r4.y
    r0.w = ((r0.wwww)/(r4.yyyy)).w;
    // 255: dp3 r5.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 256: mad r0.w, r5.w, l(5.000000), r0.w
    r0.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.wwww)).w;
    // 257: add_sat r0.w, r4.w, r0.w
    r0.w = (saturate((r4.wwww)+(r0.wwww))).w;
    // 258: mad r6.w, r0.w, l(-2.000000), l(3.000000)
    r6.w = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 259: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 260: mul r0.w, r0.w, r6.w
    r0.w = ((r0.wwww)*(r6.wwww)).w;
    // 261: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 262: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 263: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 264: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 265: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 266: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 267: dp3 r0.y, r0.xyzx, r5.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 268: dp2 r0.x, r1.xyxx, r13.zwzz
    r0.x = (dot((r1.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 269: dp2 r0.z, r1.xyxx, cb0[25].xyxx
    r0.z = (dot((r1.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 270: mov r0.w, l(1.000000)
    r0.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 271: dp4 r2.x, cb0[26].xyzw, r0.xyzw
    r2.x = (dot((source[26].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 272: dp4 r2.y, cb0[27].xyzw, r0.xyzw
    r2.y = (dot((source[27].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).y;
    // 273: dp4 r2.z, cb0[28].xyzw, r0.xyzw
    r2.z = (dot((source[28].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).z;
    // 274: mul r17.xyzw, r0.yzzx, r0.xyzz
    r17.xyzw = ((r0.yzzx)*(r0.xyzz)).xyzw;
    // 275: dp4 r5.x, cb0[29].xyzw, r17.xyzw
    r5.x = (dot((source[29].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).x;
    // 276: dp4 r5.y, cb0[30].xyzw, r17.xyzw
    r5.y = (dot((source[30].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).y;
    // 277: dp4 r5.z, cb0[31].xyzw, r17.xyzw
    r5.z = (dot((source[31].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).z;
    // 278: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 279: mad r0.x, r0.x, r0.x, -r0.z
    r0.x = ((r0.xxxx)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 280: add r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 281: mad r0.xzw, cb0[32].xxyz, r0.xxxx, r2.xxyz
    r0.xzw = ((source[32].xxyz)*(r0.xxxx)+(r2.xxyz)).xzw;
    // 282: max r0.xzw, r0.xxzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xzw = (max(r0.xxzw,float4(0.000000,0.000000,0.000000,0.000000))).xzw;
    // 283: mul r0.xzw, r0.xxzw, cb0[24].xxyz
    r0.xzw = ((r0.xxzw)*(source[24].xxyz)).xzw;
    // 284: mul r0.xzw, r0.xxzw, cb0[25].zzzz
    r0.xzw = ((r0.xxzw)*(source[25].zzzz)).xzw;
    // 285: mad r0.xzw, r0.xxzw, l(3.141593, 0.000000, 3.141593, 3.141593), cb0[24].wwww
    r0.xzw = ((r0.xxzw)*(float4(3.141593,0.000000,3.141593,3.141593))+(source[24].wwww)).xzw;
    // 286: dp3 r2.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 287: add r0.xzw, r0.xxzw, -r2.xxxx
    r0.xzw = ((r0.xxzw)+(-(r2.xxxx))).xzw;
    // 288: mad r0.xzw, r0.xxzw, l(0.800000, 0.000000, 0.800000, 0.800000), r2.xxxx
    r0.xzw = ((r0.xxzw)*(float4(0.800000,0.000000,0.800000,0.800000))+(r2.xxxx)).xzw;
    // 289: dp3 r2.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 290: div r2.x, r2.x, r4.y
    r2.x = ((r2.xxxx)/(r4.yyyy)).x;
    // 291: mad r2.x, r5.w, l(5.000000), r2.x
    r2.x = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.xxxx)).x;
    // 292: add_sat r2.x, r4.w, r2.x
    r2.x = (saturate((r4.wwww)+(r2.xxxx))).x;
    // 293: mad r2.y, r2.x, l(-2.000000), l(3.000000)
    r2.y = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 294: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 295: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 296: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 297: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 298: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 299: mul r0.xzw, r0.xxzw, r2.xxxx
    r0.xzw = ((r0.xxzw)*(r2.xxxx)).xzw;
    // 300: mul r2.x, r13.y, r13.y
    r2.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 301: dp3 r2.y, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 302: add r2.z, r3.w, r13.x
    r2.z = ((r3.wwww)+(r13.xxxx)).z;
    // 303: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 304: mul r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)*(r2.xxxx)).x;
    // 305: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 306: add r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)+(r2.xxxx)).x;
    // 307: add_sat r2.x, r2.x, l(-1.000000)
    r2.x = (saturate((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 308: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 309: mad r2.y, r2.x, r5.x, r5.y
    r2.y = ((r2.xxxx)*(r5.xxxx)+(r5.yyyy)).y;
    // 310: mad r2.y, r2.y, r2.x, r5.z
    r2.y = ((r2.yyyy)*(r2.xxxx)+(r5.zzzz)).y;
    // 311: mul r2.y, r2.x, r2.y
    r2.y = ((r2.xxxx)*(r2.yyyy)).y;
    // 312: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 313: mad r5.xyz, r8.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r8.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 314: mad r10.xyz, r8.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r8.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 315: mad r13.xyz, r8.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r8.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 316: mad r5.xyz, r3.wwww, r5.xyzx, r10.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(r10.xyzx)).xyz;
    // 317: mad r5.xyz, r5.xyzx, r3.wwww, r13.xyzx
    r5.xyz = ((r5.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 318: mul r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 319: max r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = (max(r3.wwww,r5.xyzx)).xyz;
    // 320: mul r10.xyz, r2.xxxx, r12.xyzx
    r10.xyz = ((r2.xxxx)*(r12.xyzx)).xyz;
    // 321: mul r11.xyz, r5.xyzx, r11.xyzx
    r11.xyz = ((r5.xyzx)*(r11.xyzx)).xyz;
    // 322: mul r10.xyz, r6.xyzx, r10.xyzx
    r10.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 323: mul r10.xyz, r10.xyzx, r16.xyzx
    r10.xyz = ((r10.xyzx)*(r16.xyzx)).xyz;
    // 324: mul r12.xyz, r10.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r12.xyz = ((r10.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 325: mad r13.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 326: mul r11.xyz, r11.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 327: mul r14.xyz, r0.xzwx, r13.xyzx
    r14.xyz = ((r0.xzwx)*(r13.xyzx)).xyz;
    // 328: mul r11.xyz, r11.xyzx, r14.xyzx
    r11.xyz = ((r11.xyzx)*(r14.xyzx)).xyz;
    // 329: mad r11.xyz, -r11.xyzx, r4.wwww, r11.xyzx
    r11.xyz = ((-(r11.xyzx))*(r4.wwww)+(r11.xyzx)).xyz;
    // 330: mul r6.xyz, r6.xyzx, r16.xyzx
    r6.xyz = ((r6.xyzx)*(r16.xyzx)).xyz;
    // 331: mul r13.xyz, r8.xyzx, r13.xyzx
    r13.xyz = ((r8.xyzx)*(r13.xyzx)).xyz;
    // 332: mul r13.xyz, r4.xxxx, r13.xyzx
    r13.xyz = ((r4.xxxx)*(r13.xyzx)).xyz;
    // 333: mul r0.xzw, r0.xxzw, r13.xxyz
    r0.xzw = ((r0.xxzw)*(r13.xxyz)).xzw;
    // 334: mul r0.xzw, r5.xxyz, r0.xxzw
    r0.xzw = ((r5.xxyz)*(r0.xxzw)).xzw;
    // 335: mad r0.xzw, r6.xxyz, r2.xxxx, r0.xxzw
    r0.xzw = ((r6.xxyz)*(r2.xxxx)+(r0.xxzw)).xzw;
    // 336: mad r0.xzw, r0.xxzw, l(0.400000, 0.000000, 0.400000, 0.400000), r11.xxyz
    r0.xzw = ((r0.xxzw)*(float4(0.400000,0.000000,0.400000,0.400000))+(r11.xxyz)).xzw;
    // 337: mad r2.xyz, r9.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r2.xyz = ((r9.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 338: mad r0.xzw, r10.xxyz, l(0.600000, 0.000000, 0.600000, 0.600000), r0.xxzw
    r0.xzw = ((r10.xxyz)*(float4(0.600000,0.000000,0.600000,0.600000))+(r0.xxzw)).xzw;
    // 339: add r2.xyz, r0.xzwx, r2.xyzx
    r2.xyz = ((r0.xzwx)+(r2.xyzx)).xyz;
    // 340: mad r2.xyz, r8.xyzx, cb0[36].xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)*(source[36].xyzx)+(r2.xyzx)).xyz;
    // 341: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 342: mad r3.x, r3.x, l(-0.250000), l(0.400000)
    r3.x = ((r3.xxxx)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).x;
    // 343: eq r3.y, cb0[37].x, l(0.000000)
    r3.y = (asfloat((uint4)((source[37].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 344: not r3.z, r3.y
    r3.z = (asfloat(~asuint(r3.yyyy))).z;
    // 345: lt r4.x, r2.w, r3.x
    r4.x = (asfloat((uint4)((r2.wwww)<(r3.xxxx)) * 0xffffffffu)).x;
    // 346: and r3.z, r3.z, r4.x
    r3.z = (asfloat(asuint(r3.zzzz) & asuint(r4.xxxx))).z;
    // 347: discard_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 348: ge r3.x, r2.w, r3.x
    r3.x = (asfloat((uint4)((r2.wwww)>=(r3.xxxx)) * 0xffffffffu)).x;
    // 349: mad r1.w, r1.w, cb0[2].x, l(-0.900000)
    r1.w = ((r1.wwww)*(source[2].xxxx)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 350: mul_sat r1.w, r1.w, l(9.999998)
    r1.w = (saturate((r1.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 351: mad r3.z, r1.w, l(-2.000000), l(3.000000)
    r3.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 352: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 353: mul r1.w, r1.w, r3.z
    r1.w = ((r1.wwww)*(r3.zzzz)).w;
    // 354: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 355: movc r1.w, r3.x, r1.w, r2.w
    r1.w = ((asuint(r3.xxxx) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 356: movc o0.w, r3.y, r1.w, r2.w
    output.targets[0].w = ((asuint(r3.yyyy) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 357: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 358: mov r1.z, r0.y
    r1.z = (r0.yyyy).z;
    // 359: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 360: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 361: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 362: dp3 r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r0.y = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).y;
    // 363: div r1.xy, r1.xyxx, r0.yyyy
    r1.xy = ((r1.xyxx)/(r0.yyyy)).xy;
    // 364: ge r0.y, l(0.000000), r1.z
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).y;
    // 365: ge r1.zw, r1.xxxy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.zw = (asfloat((uint4)((r1.xxxy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).zw;
    // 366: movc r1.zw, r1.zzzw, l(0,0,1.000000,1.000000), l(0,0,-1.000000,-1.000000)
    r1.zw = ((asuint(r1.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),1.000000,1.000000)) : (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000))).zw;
    // 367: mad r1.zw, -|r1.yyyx|, r1.zzzw, r1.zzzw
    r1.zw = ((-(abs(r1.yyyx)))*(r1.zzzw)+(r1.zzzw)).zw;
    // 368: movc r1.xy, r0.yyyy, r1.zwzz, r1.xyxx
    r1.xy = ((asuint(r0.yyyy) != 0u) ? (r1.zwzz) : (r1.xyxx)).xy;
    // 369: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 370: dp3 o4.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 371: dp3 o4.y, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 372: mul r0.xyz, r7.xyzx, cb0[23].xxxx
    r0.xyz = ((r7.xyzx)*(source[23].xxxx)).xyz;
    // 373: ftou r0.w, cb0[33].z
    r0.w = (asfloat((uint4)(source[33].zzzz))).w;
    // 374: and r0.w, r0.w, l(31)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(31u,31u,31u,31u))).w;
    // 375: utof r0.w, r0.w
    r0.w = ((float4)(asuint(r0.wwww))).w;
    // 376: mul o5.w, r0.w, l(0.003922)
    output.targets[5].w = ((r0.wwww)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 377: dp3_sat o5.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 378: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 379: mov o3.xyzw, r8.xyzw
    output.targets[3].xyzw = (r8.xyzw).xyzw;
    // 380: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 381: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 382: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 383: ret
    return output;
}

// source.character.classic-armor-skin-masked.v1 / source program 0b3be796bcf5b144a1761c47690de70a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase112(SOURCE_CHARACTER_NATIVE_INPUT input)
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


// source.character.equipment-native-160.v1 / source program d33da204b9a7a84189d02d8ac3544fd8
#else // SOURCE_CHARACTER_BASE_DISPATCH_CASES
    case 108u: return SourceCharacterBase108(input);
    case 109u: return SourceCharacterBase109(input);
    case 110u: return SourceCharacterBase110(input);
    case 111u: return SourceCharacterBase111(input);
    case 112u: return SourceCharacterBase112(input);
#endif // SOURCE_CHARACTER_BASE_DISPATCH_CASES
